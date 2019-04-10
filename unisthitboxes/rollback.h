#pragma once

#include "game.h"
#include <vector>
#include <numeric>
#include <memory>

class rollback_manager
{
	struct stored_field
	{
		template<typename field_type, typename class_type>
		stored_field(field_type class_type::*member)
		{
			offset = (size_t)(&(((class_type*)(nullptr))->*member));
			size = sizeof(field_type);
		}

		stored_field(size_t offset, size_t size)
		{
			this->offset = offset;
			this->size = size;
		}

		size_t offset;
		size_t size;
	};

	struct object_state
	{
		std::unique_ptr<char[]> data;
		game::string move_name;
		game::string last_move_name;
	};

	struct effect_state
	{
		int owner_idx;
		game::string mvname;
		int datatype;
		int id;

		object_state state;
	};

	struct grd_state
	{
		int blocks;
		int block_progress;
	};

	struct camera_state
	{
		int x;
		int y;
		float zoom_factor;
	};

	struct frame_state
	{
		camera_state cam_interpolated;
		camera_state cam_target;
		game::timer_data timers;
		game::vector<game::freeze_data> freezes;
		grd_state grds[2];
		object_state players[2];
		std::vector<effect_state> effects;
	};

	frame_state test_state;

	static const std::vector<stored_field> object_fields;
	static const size_t object_state_size;

	object_state save_object_state(const game::CHARA_DATA *object);
	void load_object_state(game::CHARA_DATA *object, const object_state &state);

	camera_state save_camera_state(const game::CBtlCamera_Element *camera);
	void load_camera_state(game::CBtlCamera_Element *camera, const camera_state &state);

	grd_state save_grd_state(const game::grd_data *grd);
	void load_grd_state(game::grd_data *grd, const grd_state &state);

public:
	void save_game_state();
	void load_game_state();
};

extern rollback_manager rollback;