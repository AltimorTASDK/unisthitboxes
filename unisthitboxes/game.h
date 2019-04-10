#pragma once

#include <string>
#include <vector>

class game
{
public:
	// Object flags
	static constexpr auto OF3_DORMANT = 0x400;

	// Counterhit flags
	static constexpr auto CH_HIGH_COUNTER = 1;
	static constexpr auto CH_COUNTER = 2;

	// Object creation flags
	static constexpr auto CREATE_PIXEL_COORDS = 2;

	static constexpr auto COORD_TO_PIXEL = 128;
	static constexpr auto CAMERA_HEIGHT = 280;

	// Game's STL allocator
	template<typename T>
	class allocator
	{
	public:
		using value_type = T;

		T *allocate(size_t n)
		{
			return (T*)(game::allocate(n));
		}

		void deallocate(T *p, size_t n)
		{
			game::deallocate(p, n);
		}
	};

	// STL types with game's allocator
	using string = std::basic_string<char, std::char_traits<char>, allocator<char>>;
	template<typename T>
	using vector = std::vector<T, allocator<T>>;

	template<typename T>
	struct list_entry
	{
		T *item;
		int flags;
	};

	template<typename T>
	struct list
	{
		list_entry<T> *elements;
		int max;
		int count;

		list_entry<T> *begin()
		{
			return &elements[0];
		}

		list_entry<T> *end()
		{
			return &elements[count];
		}
	};

	struct hitbox
	{
		// Top left, bottom right
		short x1, y1, x2, y2;
	};

	class state_cmd
	{
		char pad00[0xB6];
	public:
		unsigned char hurtbox_count;
		unsigned char hitbox_count;
	private:
		char pad01[0xC0 - 0xB6 - 2];
	public:
		hitbox **hurtboxes;
		hitbox **hitboxes;
	};

	class state
	{
		char pad00[0x30];
	public:
		state_cmd *cmds;
		state_cmd *cmds_end;
	};

	class chara_type
	{
		char pad00[0x4];
	public:
		state *states;
		state *states_end;
	};

	class chara_state_block
	{
	public:
		unsigned short state_id;
		unsigned short cmd_id;
		short cmd_duration;
		short cmd_frames;
		short speed_progress; // cmd_frames increments when >= 128
	private:
		short align01;
	public:
		short speed_factor; // 128 = 1.0x
		unsigned short next_state_id;
		unsigned short next_cmd_id;
	private:
		short align02;
	public:
		float cmd_fraction;
		int state_updates;
		int last_state_change;
		chara_type *chara_type_info;
	};

	class CHARA_DATA
	{
		char pad00[0x14 - 4];
	public:
		chara_state_block state_block;
	private:
		char pad01[0x3C - 0x14 - sizeof(chara_state_block)];
	public:
		int pos_x;
		int pos_y;
		int unknown;
		int parent_pos_x;
		int parent_pos_y;
	private:
		char pad02[0x54 - 0x3C - 20];
	public:
		// EF1_*
		int flags1;
		// EF2_*
		int flags2;
		// EF3_*
		int flags3;
	private:
		char pad03[0x64 - 0x54 - 12];
	public:
		int health;
		int max_health;
	private:
		char pad04[0x84 - 0x64 - 8];
	public:
		int meter;
	private:
		char pad05[0xDC - 0x84 - 4];
	public:
		int vel_x;
		int accel_x;
		int vel_y;
		int accel_y;
	private:
		char pad06[0xF4 - 0xDC - 16];
	public:
		// Attack pushback
		int push_vel_x;
		int push_accel_x;
		int push_vel_y;
		int push_accel_y;
	private:
		char pad07[0x140 - 0xF4 - 16];
	public:
		// Dash sliding, carries into jumps
		int slide_vel;
		int slide_accel;
	private:
		char pad08[0x180 - 0x140 - 8];
	public:
		short dash_speed;
	private:
		char pad09[0x19F - 0x180 - 2];
	public:
		char active;
	private:
		char pad10[0x1A4 - 0x19F - 1];
	public:
		// CH_*
		char ch_flags;
	private:
		char pad11[0x1A8 - 0x1A4 - 1];
	public:
		char invuln_frames;
	private:
		char pad12[0x231 - 0x1A8 - 1];
	public:
		char tobi_param[10];
	private:
		char pad13[0x378 - 0x231 - 10];
	public:
		CHARA_DATA *owner;
	private:
		char pad14[0x3B8 - 0x378 - 4];
	public:
		int effect_id;
		int moveable_flags;
	private:
		char pad15[0x434 - 0x3B8 - 8];
	public:
		char flipped;
	private:
		char pad16[0x440 - 0x434 - 1];
	public:
		state *state_info;
		state_cmd *state_cmd_info;
	private:
		char pad17[0x450 - 0x440 - 8];
	public:
		void *char_data_set;
	private:
		char pad18[0x459 - 0x450 - 4];
	public:
		unsigned char air_count[18];
		unsigned char dash_count;
	private:
		char pad19[0x48C - 0x459 - 19];
	public:
		string move_name;
	private:
		char pad20[0x534 - 0x48C - sizeof(string)];
	public:
		string last_move_name;
	private:
		char pad21[0x584 - 0x534 - sizeof(string)];

	public:
		virtual bool IsValid();
	};

	class PLAYER_DATA : public CHARA_DATA
	{
		char pad00[0x8FC - sizeof(CHARA_DATA)];
	};

	class CBtlCamera_Element
	{
		char pad00[0x08];
	public:
		int pos_x;
		int pos_y;
	private:
		char pad01[0x14 - 0x08 - 8];
	public:
		float zoom_factor;
	};

	class camera_data
	{
	public:
		CBtlCamera_Element final;
		CBtlCamera_Element interpolated;
		CBtlCamera_Element target;
	};

	class grd_data
	{
		char pad00[0xE8];
	public:
		int blocks;
		int block_progress;
	private:
		char pad01[0x13C - 0xE8 - 8];
	};

	struct state_update_info
	{
		char pad00[28];
	};

	struct create_object_info
	{
		CHARA_DATA *owner;
		int x;
		int y;
		int datatype;
		// CREATE_*
		int flags;
		string mvname;
		int id;
		int unknown;
		char pad[0x20];
	};

	class timer_data
	{
		char pad00[0x1D8];
	public:
		int game_time;
	private:
		char pad01[0x210 - 0x1D8 - 4];
	};

	class freeze_data
	{
		int a;
		int b;
		int c;
		int d;
		int e;
	};

	static PLAYER_DATA *players;
	static list<CHARA_DATA> *object_list;
	static vector<freeze_data> *freeze_list;
	static camera_data *camera;
	static grd_data *grds;
	static timer_data *timers;
	static void **generic_char_data_set;

	static int(__thiscall *RunFrame)(void*);
	static bool(__thiscall *UpdateCharaState)(chara_state_block*, state_update_info*);
	static void(__thiscall *UpdateStatePointers)(CHARA_DATA*);
	static void(__thiscall *DeleteObject)(CHARA_DATA*);
	static CHARA_DATA*(__thiscall *CreateObject)(create_object_info*);

	// Use the game's std::allocator functions to prevent heap corruption with STL types
	static void*(*allocate)(size_t);
	static void(*deallocate)(void*, size_t);

	game();
};

template<typename T, typename U>
inline bool operator==(const game::allocator<T> &a, const game::allocator<U> &b)
{
	return true;
}

template<typename T, typename U>
inline bool operator!=(const game::allocator<T> &a, const game::allocator<U> &b)
{
	return !(a == b);
}

extern game game_data;