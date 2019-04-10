#include "game.h"
#include "util.h"
#include <string>

game game_data;

game::PLAYER_DATA *game::players;
game::list<game::CHARA_DATA> *game::object_list;
game::vector<game::freeze_data> *game::freeze_list;
game::camera_data *game::camera;
game::grd_data *game::grds;
game::timer_data *game::timers;
void **game::generic_char_data_set;

decltype(game::RunFrame) game::RunFrame;
decltype(game::UpdateCharaState) game::UpdateCharaState;
decltype(game::UpdateStatePointers) game::UpdateStatePointers;
decltype(game::DeleteObject) game::DeleteObject;
decltype(game::CreateObject) game::CreateObject;

decltype(game::allocate) game::allocate;
decltype(game::deallocate) game::deallocate;

game::game()
{
	players = *(PLAYER_DATA**)(util::sigscan(
		"unist.exe",
		"\x7D\x23\x69\xC6\xFC\x08",
		"xxxxxx") + 0x9);

	object_list = *(list<CHARA_DATA>**)(util::sigscan(
		"unist.exe",
		"\x74\x29\x8B\x0D",
		"xxxx") + 0x4);

	freeze_list = *(vector<freeze_data>**)(util::sigscan(
		"unist.exe",
		"\xF7\xE9\x57\x0F\xB7\xBE",
		"xxxxxx") - 0x4);

	generic_char_data_set = *(void***)(util::sigscan(
		"unist.exe",
		"\x6A\x38\x8D\x44\x24\x20",
		"xxxxxx") + 0x55);

	camera = *(camera_data**)(util::sigscan(
		"unist.exe",
		"\x75\xD0\x8D\x45\xF8",
		"xxxxx") + 0x6);

	grds = *(grd_data**)(util::sigscan(
		"unist.exe",
		"\x83\xFF\x0A\x75\x4D",
		"xxxxx") - 0x10);

	timers = *(timer_data**)(util::sigscan(
		"unist.exe",
		"\x8D\x4F\xAC\xC7\x05",
		"xxxxx") - 0x34);

	RunFrame = (decltype(RunFrame))(util::sigscan(
		"unist.exe",
		"\x83\xE8\x02\x75\x10\xB9",
		"xxxxxx") - 0x11);

	UpdateCharaState = (decltype(UpdateCharaState))(util::sigscan(
		"unist.exe",
		"\x8B\x4E\x20\x0F\xBF\x56\x02",
		"xxxxxxx") - 0xB);

	UpdateStatePointers = (decltype(UpdateStatePointers))(util::sigscan(
		"unist.exe",
		"\xC6\x46\x38\x00\x85\xFF",
		"xxxxxx") - 0xA);

	DeleteObject = (decltype(DeleteObject))(util::sigscan(
		"unist.exe",
		"\x0F\x45\xF9\x85\xFF\x74\x11",
		"xxxxxxx") - 0x44);

	CreateObject = (decltype(CreateObject))(util::sigscan(
		"unist.exe",
		"\x83\x7F\x24\x00\x74\x17",
		"xxxxxx") - 0x10);

	allocate = (decltype(allocate))(util::sigscan(
		"unist.exe",
		"\x8B\x75\x08\x83\xFE\xE0",
		"xxxxxx") - 0x6);

	deallocate = (decltype(deallocate))(util::sigscan(
		"unist.exe",
		"\x83\x7D\x08\x00\x74\x2D",
		"xxxxxx") - 0x5);
}