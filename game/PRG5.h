#ifndef DFEECF2C_29C5_4E62_89A6_4759D563ACA5
#define DFEECF2C_29C5_4E62_89A6_4759D563ACA5

void copy_bg_to_current_room();

// cur_room_index
// tile_index_param
// OUT: current_room[tile_index_param]
void copy_original_room_to_current();

// cur_room_index
// IN:  in_obj_index
// OUT: loaded_obj_id
void get_obj_id();

// IN:  in_obj_index
//      cur_room_index
// OUT: loaded_obj_id
//      loaded_obj_x;
//      loaded_obj_y;
//      loaded_obj_payload;
void get_next_object();

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

#endif /* DFEECF2C_29C5_4E62_89A6_4759D563ACA5 */
