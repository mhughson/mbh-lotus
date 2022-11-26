#include "PRG0.h"
#include "PRG1.h"
#include "PRG2.h"
#include "PRG3.h"
#include "main.h"
#include "LIB/neslib.h"
#include "LIB/nesdoug.h"
#include "mmc3\mmc3_code.h"
#include "../include/stdlib.h"

#pragma rodata-name ("BANK3")
#pragma code-name ("BANK3")

#define NUM_Y_COLLISION_OFFSETS_TD 3
const unsigned char y_collision_offsets_TD[NUM_Y_COLLISION_OFFSETS_TD] = { 1, 10, 19 };
#define NUM_X_COLLISION_OFFSETS_TD 2
const unsigned char x_collision_offsets_TD[NUM_X_COLLISION_OFFSETS_TD] = { 0, 12 };

void update_player_td()
{
	static const unsigned int high_walk_speed = (WALK_SPEED >> 16);

PROFILE_POKE(PROF_G);

	// high_x = high_2byte(player1.pos_x);
	// high_y = high_2byte(player1.pos_y);

	if (pad_all & PAD_LEFT && 
		//((cam.pos_x) / 256) <= (( high_2byte((player1.pos_x)) - (WALK_SPEED >> 16)) / 256) && 
		high_2byte(player1.pos_x) - cam.pos_x >= (high_walk_speed + 8) &&
		player1.pos_x >= WALK_SPEED + FP_WHOLE(8))
	{
		temp32 = player1.pos_x;

		// move the player left.
		player1.pos_x -= (unsigned long)WALK_SPEED;// + FP_0_5;

		for (i = 0; i < NUM_Y_COLLISION_OFFSETS_TD; ++i)
		{
			// take the player position, offset it a little
			// so that the arms overlap the wall a bit (+8),
			// convert to metatile space (16x16) (>>4).
			// The position is stored using fixed point math, 
			// where the high byte is the whole part, and the
			// low byte is the fraction.
			x = (high_2byte(player1.pos_x) + x_collision_offsets_TD[0] - 1) >> 4;

			// Player is 24 pixels high, so +12 checks middle of them.
			// >>4 to put the position into metatile-space (16x16 chunks).
			y = (high_2byte(player1.pos_y) + y_collision_offsets_TD[i]) >> 4;

			// Prevent player for colliding with data outside the map, but still
			// allow them to travel above (and below) the top of the map.
			// This hard coded 15 will obviously need to change if we add vertical
			// levels.
			if (y < 15)
			{
				// Convert the x,y into an index into the room data array.
				index16 = GRID_XY_TO_ROOM_INDEX(x, y);

				tempFlags = GET_META_TILE_FLAGS(index16);

				// Check if that point is in a solid metatile
				if (tempFlags & FLAG_SOLID)
				{
					// Hit a wall, shift back to the edge of the wall.
					player1.pos_x = temp32;
					break;
				}
			}
		}
	}
	// Is the right side of the sprite, after walking, going to be passed the end of the map?
	if (pad_all & PAD_RIGHT && (player1.pos_x + WALK_SPEED + FP_WHOLE(16) ) <= FP_WHOLE(cur_room_width_pixels))
	{

		temp32 = player1.pos_x;
		player1.pos_x += WALK_SPEED;// + FP_0_5;

		for (i = 0; i < NUM_Y_COLLISION_OFFSETS_TD; ++i)
		{
			// take the player position, offset it a little
			// so that the arms overlap the wall a bit (+16),
			// convert to metatile space (16x16) (>>4).
			// The position is stored using fixed point math, 
			// where the high byte is the whole part, and the
			// low byte is the fraction.
			x = (high_2byte(player1.pos_x) + x_collision_offsets_TD[NUM_X_COLLISION_OFFSETS_TD - 1] + 1) >> 4;

			// Player is 24 pixels high, so +12 checks middle of them.
			// >>4 to put the position into metatile-space (16x16 chunks).
			y = (high_2byte(player1.pos_y) + y_collision_offsets_TD[i]) >> 4;

			if (y < 15)
			{
				// Convert the x,y into an index into the room data array.
				//temp16 = GRID_XY_TO_ROOM_NUMBER(x, y);
				index16 = GRID_XY_TO_ROOM_INDEX(x, y);

				tempFlags = GET_META_TILE_FLAGS(index16);

				// Check if that point is in a solid metatile.
				// Treat spikes like walls if the player is on the floor. It feels lame to 
				// walk into a spike and die; you should need to fall into them.
				if (tempFlags & FLAG_SOLID)
				{
					// Hit a wall, shift back to the edge of the wall.
					player1.pos_x = temp32; //(unsigned long)((x << 4) - 17) << HALF_POS_BIT_COUNT;

					break;
				}
			}
		}
	}

	if (pad_all & PAD_UP &&
		player1.pos_y >= WALK_SPEED)
	{
		
		temp32 = player1.pos_y;
		player1.pos_y -= WALK_SPEED;// + FP_0_5;

		for (i = 0; i < NUM_X_COLLISION_OFFSETS_TD; ++i)
		{
			x = (high_2byte(player1.pos_x) + x_collision_offsets_TD[i]) >> 4;
			y = (high_2byte(player1.pos_y)) >> 4; // head
			if (y < 15)
			{
				index16 = GRID_XY_TO_ROOM_INDEX(x, y);
				if (GET_META_TILE_FLAGS(index16) & FLAG_SOLID)
				{
					player1.pos_y = temp32;
					player1.vel_y = 0;
					break;
				}
			}
		}					
	}

	if (pad_all & PAD_DOWN &&
		(player1.pos_y + WALK_SPEED + FP_WHOLE(y_collision_offsets_TD[2]) ) <= FP_WHOLE(240))
	{
		temp32 = player1.pos_y;
		player1.pos_y += WALK_SPEED;// + FP_0_5;

		for (i = 0; i < NUM_X_COLLISION_OFFSETS_TD; ++i)
		{
			x = (high_2byte(player1.pos_x) + x_collision_offsets_TD[i]) >> 4;
			y = (high_2byte(player1.pos_y) + y_collision_offsets_TD[2]) >> 4; // foot
			if (y < 15)
			{
				index16 = GRID_XY_TO_ROOM_INDEX(x, y);
				if (GET_META_TILE_FLAGS(index16) & FLAG_SOLID)
				{
					player1.pos_y = temp32;
					player1.vel_y = 0;
					break;
				}
			}
		}		
	}

	if (pad_all & PAD_RIGHT)
	{
		player1.facing_left = 0;
	}
	else if (pad_all & PAD_LEFT)
	{
		player1.facing_left = 1;
	}

	anim_index = ANIM_PLAYER_IDLE;
	global_working_anim = &player1.sprite.anim;
	queue_next_anim(anim_index);
	commit_next_anim();

	//draw_player();

PROFILE_POKE(PROF_R);	
}