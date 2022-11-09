#ifndef A5B8504C_1172_4833_9D38_027FF7BA3DBB
#define A5B8504C_1172_4833_9D38_027FF7BA3DBB

// There should be no globals here; just function prototypes.
// Non constant (RAM) variables go in common header.
// Constant (ROM) variables go in this c file, so that they are no accessed by accident by other files.

void copy_bg_to_current_room();

// cur_room_index
// tile_index_param
// OUT: current_room[tile_index_param]
void copy_original_room_to_current();

// cur_room_index
// x
// OUT: loaded_obj_id
void get_obj_id();

//void build_room_delta();

// cur_room_index
// OUT: index  : BG
// OUT: index2 : SPR
void get_cur_room_palettes();

// OUT: index  : set number
void get_cur_room_metatile_set();

// OUT: index  : bit flags
void get_cur_room_bit_flags();

// OUT: index : special room type (0 - NONE)
void get_cur_room_special_type();

// OUT: index : music track to use for this level
void get_cur_room_music_track();

// OUT: index   : cur world number
// OUT: index2  : cur sub level
void get_cur_room_world_and_level();

#endif /* A5B8504C_1172_4833_9D38_027FF7BA3DBB */
