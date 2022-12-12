
#include "PRG5.h"
#include "PRG0.h"
#include "PRG1.h"
#include "PRG2.h"
#include "main.h"
#include "LIB/neslib.h"
#include "LIB/nesdoug.h"
#include "mmc3\mmc3_code.h"
#include "../include/stdlib.h"
#include "map_defs.h"

#pragma rodata-name ("BANK5")
#pragma code-name ("BANK5")

#include "maps_a.h"

#include "map_funcs.h"
GENERATE_MAP_FUNCS(a)

GENERATE_MAP_FUNCS_PROTOTYPE(b)

///////////////////////////

unsigned char nested_call;

#define MAP_BANK_SWAP(func_name_a, func_name_b, func_name_c) 	\
	/* Protect against these map functions calling into themselves when that isn't safe because of cur_room_index getting altered. */ \
	DEBUG_ASSERT(nested_call == 0); \
	nested_call = 1; \
	if (cur_room_index < ROOM_SPLIT_INDEX_B) \
	{ \
		banked_call(BANK_5, func_name_a); \
	} \
	else if (cur_room_index < ROOM_SPLIT_INDEX_C) \
	{ \
		cur_room_index -= ROOM_SPLIT_INDEX_B; \
		banked_call(BANK_6, func_name_b); \
		cur_room_index += ROOM_SPLIT_INDEX_B; \
	} \
	nested_call = 0; \
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

void get_next_object()
{
	MAP_BANK_SWAP(get_next_object_a, get_next_object_b, get_next_object_c)
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

void get_cur_room_dimensions()
{
	MAP_BANK_SWAP(get_cur_room_dimensions_a, get_cur_room_dimensions_b, get_cur_room_dimensions_c)
}


void load_and_process_map()
{
	static unsigned char local_index;

	get_cur_room_dimensions();
    cur_room_width_tiles = index;
	cur_room_height_tiles = index2;

	get_cur_room_special_type();
	cur_room_type = index;

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
	cur_room_height_pixels = cur_room_height_tiles * 16;
    cur_room_size_tiles = cur_room_width_tiles * cur_room_height_tiles;

	banked_call(BANK_2, set_metatile_set);
	banked_call(BANK_0, set_chr_bank_for_current_room);

	banked_call(BANK_0, load_level_pal);

	//memcpy(current_room, &rooms_maps_ ## MAP_VAL [cur_room_index][NUM_CUSTOM_PROPS], cur_room_size_tiles);
	copy_bg_to_current_room();

	in_obj_index = 0;
	do
	{
		get_obj_id();

		switch (loaded_obj_id)
		{
			default:
			{

			}
		}

	} while (loaded_obj_id != 0xff);
	
	/* TODO: Confirm that sizeof() is returning the size of all elements. */
	memfill(&trig_objs, 0, sizeof(trig_objs));

	/* track the index of the last added object to fill the array up. */
	local_index = 0;
	do
	{
		get_next_object();

		switch (loaded_obj_id)
		{
			case TRIG_PLAYER_SPAWN_POINT:
			{
				if (!in_is_streaming)
				{
					if (loaded_obj_payload == in_destination_spawn_id)
					{
						player1.pos_x = FP_WHOLE(loaded_obj_x * 16);
						player1.pos_y = FP_WHOLE((loaded_obj_y * 16) - 4);
					}

					// For the overworld, we just keep the dir what it was, and that should
					// point the player in the right direction, but for the side scrolling
					// levels, they enter on the extreme ends of the levels and so we just
					// face them away from the edge that will exit.
					if (cur_room_type == 0)
					{
						player1.dir_x = 1;

						// We consider anything beyond the half way point of the first screen
						// to be on the right side of the level.
						if (player1.pos_x > FP_WHOLE(128))
						{
							player1.dir_x = -1;
						}

						dash_time = 0;
					}
					player1.dir_y = 1;
					player1.vel_x = 0;
					player1.vel_y = 0;
				}
				break;
			}

			case TRIG_TRANS_POINT:
			case TRIG_TRANS_EDGE:
			{
				trig_objs.type[local_index] = loaded_obj_id;
				trig_objs.pos_x_tile[local_index] = loaded_obj_x;
				trig_objs.pos_y_tile[local_index] = loaded_obj_y;
				trig_objs.payload[local_index]	= loaded_obj_payload;

				++local_index;
				break;
			}
			
			default:
				break;
		}
	} while (loaded_obj_id != 0xff);

	draw_queue_index = 0;
	draw_dequeue_index = 0;
	memfill(draw_queue, 0, DRAW_QUEUE_SIZE);
}