#include "rollback.h"
#include "game.h"
#include <memory>

rollback_manager rollback;

const std::vector<rollback_manager::stored_field> rollback_manager::object_fields =
{
	&game::CHARA_DATA::state_block,
	&game::CHARA_DATA::pos_x,
	&game::CHARA_DATA::pos_y,
	&game::CHARA_DATA::parent_pos_x,
	&game::CHARA_DATA::parent_pos_y,
	&game::CHARA_DATA::flags1,
	&game::CHARA_DATA::flags2,
	&game::CHARA_DATA::flags3,
	&game::CHARA_DATA::health,
	&game::CHARA_DATA::max_health,
	&game::CHARA_DATA::meter,
	&game::CHARA_DATA::vel_x,
	&game::CHARA_DATA::accel_x,
	&game::CHARA_DATA::vel_y,
	&game::CHARA_DATA::accel_y,
	&game::CHARA_DATA::push_vel_x,
	&game::CHARA_DATA::push_accel_x,
	&game::CHARA_DATA::push_vel_y,
	&game::CHARA_DATA::push_accel_y,
	&game::CHARA_DATA::slide_vel,
	&game::CHARA_DATA::slide_accel,
	&game::CHARA_DATA::dash_speed,
	&game::CHARA_DATA::active,
	&game::CHARA_DATA::ch_flags,
	&game::CHARA_DATA::invuln_frames,
	&game::CHARA_DATA::tobi_param,
	&game::CHARA_DATA::moveable_flags,
	&game::CHARA_DATA::flipped,
	&game::CHARA_DATA::air_count,
	&game::CHARA_DATA::dash_count,
	{ 0x182, 0x106 }, // Hit data
	{ 0x294, 0x10 } // Last hit position data
};

const size_t rollback_manager::object_state_size = std::accumulate(
	object_fields.begin(),
	object_fields.end(),
	0,
	[](auto a, const auto &b) { return a + b.size; });

auto rollback_manager::save_object_state(const game::CHARA_DATA *object) -> object_state
{
	object_state state;
	state.data = std::make_unique<char[]>(object_state_size);
	size_t offset = 0;

	for (const auto &field : object_fields)
	{
		memcpy(state.data.get() + offset, (char*)(object) + field.offset, field.size);
		offset += field.size;
	}

	state.move_name = object->move_name;
	state.last_move_name = object->last_move_name;

	return state;
}

__declspec(noinline) void rollback_manager::load_object_state(game::CHARA_DATA *object, const object_state &state)
{
	size_t offset = 0;

	for (const auto &field : object_fields)
	{
		memcpy((char*)(object) + field.offset, state.data.get() + offset, field.size);
		offset += field.size;
	}

	object->move_name = state.move_name;
	object->last_move_name = state.last_move_name;
	game::UpdateStatePointers(object);
}

auto rollback_manager::save_camera_state(const game::CBtlCamera_Element *camera) -> camera_state
{
	camera_state state;
	state.x = camera->pos_x;
	state.y = camera->pos_y;
	state.zoom_factor = camera->zoom_factor;
	return state;
}

void rollback_manager::load_camera_state(game::CBtlCamera_Element *camera, const camera_state &state)
{
	camera->pos_x = state.x;
	camera->pos_y = state.y;
	camera->zoom_factor = state.zoom_factor;
}

auto rollback_manager::save_grd_state(const game::grd_data *grd) -> grd_state
{
	grd_state state;
	state.blocks = grd->blocks;
	state.block_progress = grd->block_progress;
	return state;
}

void rollback_manager::load_grd_state(game::grd_data *grd, const grd_state &state)
{
	grd->blocks = state.blocks;
	grd->block_progress = state.block_progress;
}

void rollback_manager::save_game_state()
{
	test_state.cam_interpolated = save_camera_state(&game::camera->interpolated);
	test_state.cam_target = save_camera_state(&game::camera->target);

	memcpy(&test_state.timers, game::timers, sizeof(game::timer_data));

	test_state.freezes = *game::freeze_list;

	test_state.grds[0] = save_grd_state(&game::grds[0]);
	test_state.grds[1] = save_grd_state(&game::grds[1]);

	test_state.players[0] = save_object_state(&game::players[0]);
	test_state.players[1] = save_object_state(&game::players[1]);

	test_state.effects.clear();

	for (auto i = 2; i < game::object_list->count; i++)
	{
		const auto *object = game::object_list->elements[i].item;
		if (object == nullptr)
			continue;

		effect_state effect;

		if (object->owner == &game::players[0])
			effect.owner_idx = 0;
		else if (object->owner == &game::players[1])
			effect.owner_idx = 1;
		else
			continue;

		effect.datatype = object->char_data_set == *game::generic_char_data_set ? 1 : 0;
		effect.mvname = object->move_name;
		effect.id = object->effect_id;
		effect.state = save_object_state(object);

		test_state.effects.push_back(std::move(effect));
	}
}

void rollback_manager::load_game_state()
{
	load_camera_state(&game::camera->interpolated, test_state.cam_interpolated);
	load_camera_state(&game::camera->target, test_state.cam_target);

	memcpy(game::timers, &test_state.timers, sizeof(game::timer_data));

	*game::freeze_list = test_state.freezes;

	load_grd_state(&game::grds[0], test_state.grds[0]);
	load_grd_state(&game::grds[1], test_state.grds[1]);

	load_object_state(&game::players[0], test_state.players[0]);
	load_object_state(&game::players[1], test_state.players[1]);

	for (auto i = 2; i < game::object_list->count; i++)
	{
		auto *object = game::object_list->elements[i].item;
		if (object != nullptr && !(game::object_list->elements[i].flags & 4) && object->IsValid())
			game::DeleteObject(object);
	}

	for (auto &effect : test_state.effects)
	{
		game::create_object_info info = { 0 };
		info.owner = &game::players[effect.owner_idx];
		info.datatype = effect.datatype;
		info.mvname = effect.mvname;
		info.id = effect.id;

		auto *object = game::CreateObject(&info);
		load_object_state(object, effect.state);
	}
}