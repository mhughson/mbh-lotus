#include "PRG2.h"
#include "PRG0.h"
#include "PRG1.h"
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
	static unsigned char local_index;
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
	
	// TODO: Confirm that sizeof() is returning the size of all elements.
	memfill(&trig_objs, 0, sizeof(trig_objs));

	// track the index of the last added object to fill the array up.
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
				}
				break;
			}

			case TRIG_TRANS_POINT:
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
	loaded_obj_index = rooms_maps_a[cur_room_index][cur_room_size_tiles + in_obj_index + NUM_CUSTOM_PROPS];
	++in_obj_index;
	loaded_obj_id = rooms_maps_a[cur_room_index][cur_room_size_tiles + in_obj_index + NUM_CUSTOM_PROPS];
	++in_obj_index;
}

void get_next_object_a()
{
	loaded_obj_id = rooms_maps_a[cur_room_index][cur_room_size_tiles + in_obj_index + NUM_CUSTOM_PROPS];
	++in_obj_index;
	loaded_obj_x = rooms_maps_a[cur_room_index][cur_room_size_tiles + in_obj_index + NUM_CUSTOM_PROPS];
	++in_obj_index;
	loaded_obj_y = rooms_maps_a[cur_room_index][cur_room_size_tiles + in_obj_index + NUM_CUSTOM_PROPS];
	++in_obj_index;
	loaded_obj_payload = rooms_maps_a[cur_room_index][cur_room_size_tiles + in_obj_index + NUM_CUSTOM_PROPS];
	++in_obj_index;
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

void get_next_object()
{
	MAP_BANK_SWAP(get_next_object_a, get_obj_id_b, get_obj_id_c)
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


void try_stream_in_next_level()
{
	/*
	How to support scrolling in levels not divisible by 512:
	eg. A B A | A | A B A B A

	1) Halt - Stream in next level into *next* nametable. (A is visible)
	2) Scroll into view. (B is now visible)
	-- If scrolling B -> A skip to #5
	3) Halt - Stream same level data into nametable A (B is still visible)
	4) Pop camera to Nametable A
	5) Pop Player to proper starting position.
	6) Resume gameplay

	For player moving right to left, I think ti would work by getting the 
	width of the level and figuring out if it is an odd sized room.

	Up/Down transitions should copy megaman: keep mirroring the same and just
	stream in next area at the edge of the screen. At the end, pop up to top,
	and then if needed halt and stream into Nametable A. Figuring out if you
	want to go to Nametable A or B might be a challenge.
*/

	static unsigned int local_i16;

	// TODO: This should be from hitting a trigger, not hard coded like this.
	if (high_2byte(player1.pos_x) > cur_room_width_pixels - 24)
	{
		// TODO: Destination should be data-drive not hard coded to ++.
		++cur_room_index;
		
#define SCROLL_SPEED (4)

		// Store the camera position as in a temp variable, as we don't want
		// affect the actual camera during this sequence, as the player rendering
		// will attemp to "offset" that camera.
		index16 = cam.pos_x;

		// Load the next room. NOTE: This is loading OVER top of the room in RAM 
		// that will be visible during scroll, but does NOT override VRAM. This is
		// a point of not return though, and the old remove must be scrolled out of
		// view before giving control back to the player.
		in_is_streaming = 1;
		copy_bg_to_current_room();

		// We need to do this once before entering the loop so that the 
		// first frame is not missing the player.
		banked_call(BANK_1, draw_player_static);

		// We know that the camera is right at the edge of the screen, just by the
		// nature of the camera system, and so as a result, we know that we need to
		// scroll 256 pixels to scroll the next room fully into view.
		for (local_i16 = 0; local_i16 < 256; local_i16+=SCROLL_SPEED)
		{
			// Load in a full column of tile data. Don't time slice in this case
			// as perf shouldn't be an issue, and time slicing would furth complicate
			// this sequence.
			in_x_tile = local_i16 / 16;
			in_x_pixel = local_i16;
			in_flip_nt = 0;
			banked_call(BANK_0, vram_buffer_load_column_full);

			// Wait for the frame to be drawn, clear out the sprite data and vram buffer
			// for the next frame, all within this tight loop.
			ppu_wait_nmi();
			oam_clear();
			clear_vram_buffer();

			// Start moving stuff after streaming in 1 column so that we don't see 
			// the first column appear after the camera has already moved.

			// desired distance (216) / 64 steps = 3.375
			// Is dependant on SCROLL_SPEED being 4. If that changes,
			// then the number of "steps" should be re-calculated.
			player1.pos_x -= (FP_WHOLE(3) + FP_0_18 + FP_0_18 + FP_0_05);

			// Draw the player without updating the animation, as it looks
			// weird if they "moon walk" across the screen.
			banked_call(BANK_1, draw_player_static);

			index16 += SCROLL_SPEED;
			// Scroll the camera without affecting "cam" struct.
			scroll(index16,0);			
		}

		// This point we have loaded the first nametable of content from the new level,
		// and scrolled it into view.
		// The chunk of code does the same thing, but for the OTHER nametable, and does
		// NOT scroll the camera.
		for (local_i16 = 0; local_i16 < 256; local_i16+=8)
		{
			// Load in a full column of tile data. Don't time slice in this case
			// as perf shouldn't be an issue, and time slicing would furth complicate
			// this sequence.
			in_x_tile = local_i16 / 16;
			in_x_pixel = local_i16;
			in_flip_nt = 1;
			banked_call(BANK_0, vram_buffer_load_column_full);

			// Wait for the frame to be drawn, clear out the sprite data and vram buffer
			// for the next frame, all within this tight loop.
			ppu_wait_nmi();
			oam_clear();
			clear_vram_buffer();

			// Draw the player without updating the animation, as it looks
			// weird if they "moon walk" across the screen.
			banked_call(BANK_1, draw_player_static);
		}

		// Move the cam now we we don't see the extra 3 tiles pop in.
		player1.pos_x = FP_WHOLE(16);
		cam.pos_x = 0;
		// Both nametables are identicle so this camera pop should be completely
		// unnoticed.
		scroll(cam.pos_x,0);	

		// Load in 3 extra columns, as the default scrolling logic will miss those.
		for (local_i16 = 256; local_i16 < (256+32); local_i16+=8)
		{
			in_x_tile = local_i16 / 16;
			in_x_pixel = local_i16;
			in_flip_nt = 0;
			banked_call(BANK_0, vram_buffer_load_column_full);
			banked_call(BANK_1, draw_player_static);
			ppu_wait_nmi();
			oam_clear();
			clear_vram_buffer();
		}	
	}
	else if (high_2byte(player1.pos_x) < (10))
	{
		// TODO: Destination should be data-drive not hard coded to ++.
		--cur_room_index;
		
#define SCROLL_SPEED (4)

		// Store the camera position as in a temp variable, as we don't want
		// affect the actual camera during this sequence, as the player rendering
		// will attemp to "offset" that camera.
		index16 = cam.pos_x;

		// Load the next room. NOTE: This is loading OVER top of the room in RAM 
		// that will be visible during scroll, but does NOT override VRAM. This is
		// a point of not return though, and the old remove must be scrolled out of
		// view before giving control back to the player.
		in_is_streaming = 1;
		copy_bg_to_current_room();

		// We need to do this once before entering the loop so that the 
		// first frame is not missing the player.
		banked_call(BANK_1, draw_player_static);

		// We know that the camera is right at the edge of the screen, just by the
		// nature of the camera system, and so as a result, we know that we need to
		// scroll 256 pixels to scroll the next room fully into view.
		for (local_i16 = cur_room_width_pixels; local_i16 > (cur_room_width_pixels - 256); local_i16-=SCROLL_SPEED)
		{
			// Load in a full column of tile data. Don't time slice in this case
			// as perf shouldn't be an issue, and time slicing would furth complicate
			// this sequence.
			in_x_tile = local_i16 / 16;
			in_x_pixel = local_i16;
			in_flip_nt = 0;
			banked_call(BANK_0, vram_buffer_load_column_full);

			// Wait for the frame to be drawn, clear out the sprite data and vram buffer
			// for the next frame, all within this tight loop.
			ppu_wait_nmi();
			oam_clear();
			clear_vram_buffer();

			// Start moving stuff after streaming in 1 column so that we don't see 
			// the first column appear after the camera has already moved.

			// desired distance (216) / 64 steps = 3.375
			// Is dependant on SCROLL_SPEED being 4. If that changes,
			// then the number of "steps" should be re-calculated.
			player1.pos_x += (FP_WHOLE(3) + FP_0_18 + FP_0_05 + FP_0_05 + FP_0_05 + FP_0_05 - FP_0_01);

			// Draw the player without updating the animation, as it looks
			// weird if they "moon walk" across the screen.
			banked_call(BANK_1, draw_player_static);

			index16 -= SCROLL_SPEED;
			// Scroll the camera without affecting "cam" struct.
			scroll(index16,0);			
		}

		// This point we have loaded the first nametable of content from the new level,
		// and scrolled it into view.
		// The chunk of code does the same thing, but for the OTHER nametable, and does
		// NOT scroll the camera.
		for (local_i16 = cur_room_width_pixels; local_i16 > (cur_room_width_pixels - 256); local_i16-=8)
		{
			// Load in a full column of tile data. Don't time slice in this case
			// as perf shouldn't be an issue, and time slicing would furth complicate
			// this sequence.
			in_x_tile = local_i16 / 16;
			in_x_pixel = local_i16;
			in_flip_nt = 1;
			banked_call(BANK_0, vram_buffer_load_column_full);

			// Wait for the frame to be drawn, clear out the sprite data and vram buffer
			// for the next frame, all within this tight loop.
			ppu_wait_nmi();
			oam_clear();
			clear_vram_buffer();

			// Draw the player without updating the animation, as it looks
			// weird if they "moon walk" across the screen.
			banked_call(BANK_1, draw_player_static);
		}

		// Move the cam now we we don't see the extra 3 tiles pop in.
		player1.pos_x = FP_WHOLE(cur_room_width_pixels - 32);
		cam.pos_x = cur_room_width_pixels - 256;
		// Both nametables are identicle so this camera pop should be completely
		// unnoticed.
		scroll(cam.pos_x,0);

		// We don't need the 3 extra columns for the left side, because natural
		// scrolling doesn't have the issue with missing columns.
	}
}