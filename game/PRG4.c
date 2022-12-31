#include "main.h"
#include "PRG0.h"
#include "PRG1.h"
#include "PRG2.h"
#include "PRG3.h"
#include "PRG4.h"
#include "PRG5.h"
#include "LIB/neslib.h"
#include "LIB/nesdoug.h"
#include "mmc3\mmc3_code.h"
#include "../include/stdlib.h"

#pragma rodata-name ("BANK4")
#pragma code-name ("BANK4")

#include "NES_ST/screen_title.h"
#include "NES_ST/screen_gameover.h"

void load_screen_title()
{
	vram_adr(NTADR_A(0,0));
    vram_unrle(screen_title);
    // Load it into both nametables so that the h_scroll split
    // loops nicely.
	vram_adr(NTADR_B(0,0));
    vram_unrle(screen_title);
}

void load_screen_gameover()
{
	vram_adr(NTADR_A(0,0));
    vram_unrle(screen_gameover);
}

// NOTE: Very specific to player->skeleton collision right now
unsigned char intersects_box_box()
{
	static signed int _pos_delta;
	static signed int _dist_sum;
	static signed int _width_half = 8;
	static signed int _height_half = 8;

	_pos_delta = (high_2byte(player1.pos_x) + _width_half) - (dynamic_objs.pos_x[in_dynamic_obj_index] + _width_half);
	_dist_sum = _width_half + _width_half;
	if( abs(_pos_delta) >= _dist_sum ) return 0;

	_pos_delta = (high_2byte(player1.pos_y) + 12) - (dynamic_objs.pos_y[in_dynamic_obj_index] + _height_half);
	_dist_sum = 12 + _height_half;
	if( abs(_pos_delta) >= _dist_sum ) return 0;
	
	return 1;
}

void update_skeleton()
{
	static unsigned char local_offset;

    // Check if the Skeleton is dead.
    if (dynamic_objs.dead_time[in_dynamic_obj_index] > 0)
    {
        // Decrement the death timer. We want the skeleton
        // to stary visible in a squished state for a while.
        --dynamic_objs.dead_time[in_dynamic_obj_index];
        if (dynamic_objs.dead_time[in_dynamic_obj_index] == 0)
        {
            // Timers has completed. Free up this object slot.
            dynamic_objs.type[in_dynamic_obj_index] = TRIG_UNUSED;
        }

        // Skip the rest.
        return;
    }

	if (tick_count % 4 == 0)
	{
		if (dynamic_objs.dir_x[in_dynamic_obj_index] < 0)
		{
			--dynamic_objs.pos_x[in_dynamic_obj_index];
		}
		else
		{
			++dynamic_objs.pos_x[in_dynamic_obj_index];
		}
	}

	if (dynamic_objs.pos_x[in_dynamic_obj_index] < cam.freeze_left ||
		dynamic_objs.pos_x[in_dynamic_obj_index] > cam.freeze_right)
	{
		dynamic_objs.state[in_dynamic_obj_index] |= DYNAMIC_STATE_FROZEN;
		return;
	}

	local_offset = 0;
	if (dynamic_objs.dir_x[in_dynamic_obj_index] > 0)
	{
		local_offset = 1;
	}

	index16 = GRID_XY_TO_ROOM_INDEX((dynamic_objs.pos_x[in_dynamic_obj_index] / 16) + local_offset, dynamic_objs.pos_y[in_dynamic_obj_index] / 16);

	tempFlags = GET_META_TILE_FLAGS(index16);

	// Check if that point is in a solid metatile
	if (tempFlags & FLAG_SOLID)
	{
		dynamic_objs.dir_x[in_dynamic_obj_index] *= -1;
	}
	else
	{
		// Only check for a pit if we DIDN'T already hit a wall to avoid
		// flipping again since a lot of tiles below edges will actually
		// not be solid because you can't reach them.
		index16 += cur_room_width_tiles;

		tempFlags = GET_META_TILE_FLAGS(index16);

		// Check if that point is in a solid metatile
		if ((tempFlags & FLAG_SOLID) == 0)
		{
			dynamic_objs.dir_x[in_dynamic_obj_index] *= -1;
		}
	}

    // Is the skeleton overlapping the player *at all*.
	if (intersects_box_box())
	{
        // Check if the player's feet are above the feet of the Skeleton.
        // We will only consider this a "squish attack" if they are.
        // TODO: I'm not quite sure why the player feet needs to be at "21" rather than their actual position of "20".
		if ((dynamic_objs.pos_y[in_dynamic_obj_index] + 16) >= (high_2byte(player1.pos_y) + 21))
		{
			// if downward, bounce
			if (player1.vel_y16 > 0)
			{
                // Big or normal bounce?
                if (pad_all & PAD_A)
                {
				    player1.vel_y16 = -(JUMP_VEL_16bit) * 2;
                }
                else
                {
                    player1.vel_y16 = -(JUMP_VEL_16bit);
                }

                // Experimenting with zeroing out the X velocity
                // of the player, to make it feel less "out of control".
                player1.vel_x16 = 0;
				sfx_play(5,0);

                // Start the death timer for the skeleton.
                dynamic_objs.dead_time[in_dynamic_obj_index] = 60;
			}
		}
        // Only hurt the player if they are NOT jumping upward.
        // This is to make the game feel more forgiving.
		else if (player1.vel_y16 >= 0)
		{
			banked_call(BANK_0, kill_player);
		}
	}
}