#include "PRG2.h"
#include "PRG0.h"
#include "PRG1.h"
#include "PRG5.h"
#include "main.h"
#include "LIB/neslib.h"
#include "LIB/nesdoug.h"
#include "mmc3\mmc3_code.h"
#include "../include/stdlib.h"
#include "map_defs.h"

#pragma rodata-name ("BANK2")
#pragma code-name ("BANK2")

//#include "maps_a.h"
#include "meta_tiles_temp.h"
#include "meta_tiles_overworld.h"

// List of all the meta tiles sets. When a new tileset is added, this list
// should be extended.
// NOTE: NEW_TILESET_CHANGE_REQUIRED
const unsigned char (* const metatile_sets[9])[128*META_TILE_NUM_BYTES] =
{
	&metatiles_temp,
	&metatiles_overworld,
};

void stream_in_next_level()
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
	//if (high_2byte(player1.pos_x) > cur_room_width_pixels - 24)
	if (in_stream_direction == 1)
	{
	
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
		banked_call(BANK_5, copy_bg_to_current_room);

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
			player1.pos_x -= (FP_WHOLE(3) + FP_0_18 + FP_0_15);

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
	else if (in_stream_direction == 0)
	{
	
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
		banked_call(BANK_5, copy_bg_to_current_room);

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
			player1.pos_x += (FP_WHOLE(3) + FP_0_18 + FP_0_05 + FP_0_05);

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

void set_metatile_set()
{
	banked_call(BANK_5, get_cur_room_metatile_set);
	memcpy(cur_metatiles, metatile_sets[cur_room_metatile_index], META_TILE_SET_NUM_BYTES);
}