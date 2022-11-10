#include "PRG2.h"
#include "main.h"
#include "LIB/neslib.h"
#include "LIB/nesdoug.h"
#include "mmc3\mmc3_code.h"
#include "../include/stdlib.h"
#include "map_defs.h"

#pragma rodata-name ("BANK2")
#pragma code-name ("BANK2")

#include "maps_a.h"

void copy_bg_to_current_room_a()
{
    cur_room_width_tiles = rooms_maps_a[cur_room_index][8];

    switch (cur_room_width_tiles)
    {
    case 16:
        cur_room_width_tiles_shift_factor = 4;
        break;
    case 32:
        cur_room_width_tiles_shift_factor = 5;
        break;
    case 64:
        cur_room_width_tiles_shift_factor = 6;
        break;
    case 128:
        cur_room_width_tiles_shift_factor = 7;
        break;
    
    default:
        break;
    } 
    cur_room_width_pixels = cur_room_width_tiles * 16;
    cur_room_size_tiles = cur_room_width_tiles * rooms_maps_a[cur_room_index][9];

	// NUM_CUSTOM_PROPS because the level data starts after the custom props
	memcpy(current_room, &rooms_maps_a[cur_room_index][NUM_CUSTOM_PROPS], cur_room_size_tiles);	    
}

// tile_index_param
void copy_original_room_to_current_a()
{
	static unsigned char tile_index_param_copy;

	tile_index_param_copy = tile_index_param;

	current_room[tile_index_param_copy] = rooms_maps_a[cur_room_index][tile_index_param+NUM_CUSTOM_PROPS];
}

void get_obj_id_a()
{
	loaded_obj_index = rooms_maps_a[cur_room_index][cur_room_size_tiles + x + NUM_CUSTOM_PROPS];
	++x;
	loaded_obj_id = rooms_maps_a[cur_room_index][cur_room_size_tiles + x + NUM_CUSTOM_PROPS];
	++x;
}

void get_cur_room_palettes_a()
{
	index = rooms_maps_a[cur_room_index][0];
	index2 = rooms_maps_a[cur_room_index][1];
}

void get_cur_room_metatile_set_a()
{
	cur_room_metatile_index = rooms_maps_a[cur_room_index][2];
}

void get_cur_room_bit_flags_a()
{
	index = rooms_maps_a[cur_room_index][3];
}

void get_cur_room_special_type_a()
{
	index = rooms_maps_a[cur_room_index][4];
}

void get_cur_room_music_track_a()
{
	index = rooms_maps_a[cur_room_index][5];
}

void get_cur_room_world_and_level_a()
{
	index = rooms_maps_a[cur_room_index][6];
	index2 = rooms_maps_a[cur_room_index][7];
}


///////////////////////////

#define MAP_BANK_SWAP(func_name_a, func_name_b, func_name_c) 	if (cur_room_index < ROOM_SPLIT_INDEX_B) \
	{ \
		banked_call(BANK_2, func_name_a); \
	} \
	// else if (cur_room_index < ROOM_SPLIT_INDEX_C) \
	// { \
	// 	cur_room_index -= ROOM_SPLIT_INDEX_B; \
	// 	banked_call(BANK_3, func_name_b); \
	// 	cur_room_index += ROOM_SPLIT_INDEX_B; \
	// } \
	// else \
	// { \
	// 	cur_room_index -= ROOM_SPLIT_INDEX_C; \
	// 	banked_call(BANK_2, func_name_c); \
	// 	cur_room_index += ROOM_SPLIT_INDEX_C; \
	// }


void copy_bg_to_current_room()
{
	MAP_BANK_SWAP(copy_bg_to_current_room_a, copy_bg_to_current_room_b, copy_bg_to_current_room_c)
}

void copy_original_room_to_current()
{
	MAP_BANK_SWAP(copy_original_room_to_current_a, copy_original_room_to_current_b, copy_original_room_to_current_c);	
}

void get_obj_id()
{
	MAP_BANK_SWAP(get_obj_id_a, get_obj_id_b, get_obj_id_c)
}

void get_cur_room_palettes()
{
	MAP_BANK_SWAP(get_cur_room_palettes_a, get_cur_room_palettes_b, get_cur_room_palettes_c)
}

void get_cur_room_metatile_set()
{
	MAP_BANK_SWAP(get_cur_room_metatile_set_a, get_cur_room_metatile_set_b, get_cur_room_metatile_set_c)
}

void get_cur_room_bit_flags()
{
	MAP_BANK_SWAP(get_cur_room_bit_flags_a, get_cur_room_bit_flags_b, get_cur_room_bit_flags_c)
}

void get_cur_room_special_type()
{
	MAP_BANK_SWAP(get_cur_room_special_type_a, get_cur_room_special_type_b, get_cur_room_special_type_c)
}

void get_cur_room_music_track()
{
	MAP_BANK_SWAP(get_cur_room_music_track_a, get_cur_room_music_track_b, get_cur_room_music_track_c)
}

void get_cur_room_world_and_level()
{
	MAP_BANK_SWAP(get_cur_room_world_and_level_a, get_cur_room_world_and_level_b, get_cur_room_world_and_level_c)
}