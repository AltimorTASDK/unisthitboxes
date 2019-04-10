#include "util.h"
#include "game.h"
#include "graphics.h"
#include "rollback.h"
#include <Windows.h>
#include <d3d9.h>
#include <detours.h>

void draw_pushbox(IDirect3DDevice9 *device, const game::CHARA_DATA *object)
{
	const auto *cmd_info = object->state_cmd_info;
	const auto flip = object->flipped ? -1 : 1;

	if (cmd_info == nullptr || cmd_info->hurtbox_count == 0)
		return;

	const auto pushbox = cmd_info->hurtboxes[0];

	if (pushbox == nullptr)
		return;

	graphics::draw_box(
		device,
		object->pos_x + object->parent_pos_x + pushbox->x1 * game::COORD_TO_PIXEL * flip,
		object->pos_y + object->parent_pos_y + pushbox->y1 * game::COORD_TO_PIXEL,
		object->pos_x + object->parent_pos_x + pushbox->x2 * game::COORD_TO_PIXEL * flip,
		object->pos_y + object->parent_pos_y + pushbox->y2 * game::COORD_TO_PIXEL,
		D3DCOLOR_ARGB(1, 255, 255, 0),
		D3DCOLOR_ARGB(255, 255, 255, 0));
}

void draw_hurtboxes(IDirect3DDevice9 *device, const game::CHARA_DATA *object)
{
	const auto *cmd_info = object->state_cmd_info;
	const auto flip = object->flipped ? -1 : 1;

	if (cmd_info == nullptr)
		return;

	// Hollow when invuln
	// Cyan when counter
	// Blue when high counter
	const auto alpha = object->invuln_frames != 0 ? 0 : 1;
	const auto red = 0;
	const auto green = (object->ch_flags & game::CH_HIGH_COUNTER) ? 0 : 255;
	const auto blue = object->ch_flags != 0 ? 255 : 0;

	for (auto i = 1; i < cmd_info->hurtbox_count; i++)
	{
		const auto hurtbox = cmd_info->hurtboxes[i];

		if (hurtbox == nullptr)
			continue;

		graphics::draw_box(
			device,
			object->pos_x + object->parent_pos_x + hurtbox->x1 * game::COORD_TO_PIXEL * flip,
			object->pos_y + object->parent_pos_y + hurtbox->y1 * game::COORD_TO_PIXEL,
			object->pos_x + object->parent_pos_x + hurtbox->x2 * game::COORD_TO_PIXEL * flip,
			object->pos_y + object->parent_pos_y + hurtbox->y2 * game::COORD_TO_PIXEL,
			D3DCOLOR_ARGB(alpha, red, green, blue),
			D3DCOLOR_ARGB(255, red, green, blue));
	}
}

void draw_hitboxes(IDirect3DDevice9 *device, const game::CHARA_DATA *object)
{
	const auto *cmd_info = object->state_cmd_info;
	const auto flip = object->flipped ? -1 : 1;

	if (cmd_info == nullptr)
		return;

	for (auto i = 0; i < cmd_info->hitbox_count; i++)
	{
		const auto hitbox = cmd_info->hitboxes[i];

		if (hitbox == nullptr)
			continue;

		graphics::draw_box(
			device,
			object->pos_x + object->parent_pos_x + hitbox->x1 * game::COORD_TO_PIXEL * flip,
			object->pos_y + object->parent_pos_y + hitbox->y1 * game::COORD_TO_PIXEL,
			object->pos_x + object->parent_pos_x + hitbox->x2 * game::COORD_TO_PIXEL * flip,
			object->pos_y + object->parent_pos_y + hitbox->y2 * game::COORD_TO_PIXEL,
			D3DCOLOR_ARGB(1, 255, 0, 0),
			D3DCOLOR_ARGB(255, 255, 0, 0));
	}
}

// IDirect3DDevice9::EndScene
using EndScene_t = HRESULT(__stdcall*)(IDirect3DDevice9*);
EndScene_t orig_EndScene;
constexpr auto EndScene_idx = 42;

HRESULT __stdcall hook_EndScene(IDirect3DDevice9 *device)
{
	graphics::render_start(device);

	// Draw hitboxes of each type for all entities.
	const auto iterate_objects = [&](auto func)
	{
		for (const auto &entry : *game::object_list)
		{
			if (entry.item == nullptr || (entry.flags & 4) || !entry.item->IsValid())
				continue;

			if (!(entry.item->flags3 & game::OF3_DORMANT))
				func(device, entry.item);
		}
	};

	iterate_objects(draw_pushbox);
	iterate_objects(draw_hurtboxes);
	iterate_objects(draw_hitboxes);

	graphics::render_end(device);

	return orig_EndScene(device);
}

using RunFrame_t = decltype(game::RunFrame);
RunFrame_t orig_RunFrame;

int __fastcall hook_RunFrame(void *thisptr)
{
	static auto pressed = false;

	if (GetAsyncKeyState(VK_F1) & 0x8000)
	{
		if (!pressed)
			rollback.save_game_state();

		pressed = true;
	}
	else if (GetAsyncKeyState(VK_F2) & 0x8000)
	{
		if (!pressed)
			rollback.load_game_state();

		pressed = true;
	}
	else if (GetAsyncKeyState(VK_F3) & 0x8000)
	{
		if (!pressed)
		{
			for (auto i = 0; i < 10; i++)
				orig_RunFrame(thisptr);
		}

		pressed = true;
	}
	else
	{
		pressed = false;
	}

	return orig_RunFrame(thisptr);
}

BOOL WINAPI DllMain(
	_In_ HINSTANCE hinstDLL,
	_In_ DWORD     fdwReason,
	_In_ LPVOID    lpvReserved
	)
{
	if (fdwReason != DLL_PROCESS_ATTACH)
		return FALSE;

	// Hook IDirect3DDevice9::EndScene.
	const auto **dev_vtable = *(const void***)(util::sigscan(
		"d3d9.dll",
		"\xC7\x06\x00\x00\x00\x00\x89\x86\x00\x00\x00\x00\x89\x86",
		"xx????xx????xx") + 0x2);

	orig_EndScene = (EndScene_t)(DetourFunction((BYTE*)(dev_vtable[EndScene_idx]), (BYTE*)(hook_EndScene)));

	//orig_RunFrame = (RunFrame_t)(DetourFunction((BYTE*)(game::RunFrame), (BYTE*)(hook_RunFrame)));

	return TRUE;
}