#include "PRG0.h"
#include "PRG7.h"
#include "main.h"
#include "LIB/neslib.h"
#include "LIB/nesdoug.h"
#include "mmc3\mmc3_code.h"
#include "../include/stdlib.h"

#pragma rodata-name ("BANK7")
#pragma code-name ("BANK7")

void update_player()
{
	static unsigned char hit_kill_box;

	static unsigned long old_x;
	static unsigned long old_y;

	static signed int tempInt;

	// static unsigned int high_x;
	// static unsigned int high_y;

	// high_x = high_2byte(player1.pos_x);
	// high_y = high_2byte(player1.pos_y);

	old_x = player1.pos_x;
	old_y = player1.pos_y;



	// Only apply horizontal friction if the player is
	// NOT dashing.
	if (dash_time == 0)
	{
		if (grounded)
		{
			// When on the ground, stop dead. This isn't an
			// ice rink!
			player1.vel_x16 = 0;
		}
		else
		{
			// Apply air friction to the current velocity, which
			// will be added to position later.
			// If we are in range of 0, just right there to avoid
			// over shooting.
			/*
				x >> 4 = 0.94
				x >> 3 = 0.88
				x >> 2 = 0.75
				x >> 1 = 0.5 
			*/
		
			//tempFlags = player1.vel_x < 0 ? -1 : 1;
			tempInt = (ABS(player1.vel_x16) >> 4); // equivlent of * 0.94f
			if (tempInt > 0)
			{
				if (player1.vel_x16 > 0)
				{
					player1.vel_x16 -= (tempInt);// * tempFlags);
				}
				else
				{
					player1.vel_x16 += tempInt;
				}
			}
			else
			{
				player1.vel_x16 = 0;
			}
		}
	}

	// Only allow the player to walk left/right if they are
	// NOT dashing.
	if (dash_time == 0)
	{
		if (pad_all & PAD_LEFT)
		{
			player1.vel_x16 = -WALK_SPEED_16bit;
		}
		// Is the right side of the sprite, after walking, going to be passed the end of the map?
		else if (pad_all & PAD_RIGHT) // && (high_2byte(player1.pos_x) + high_byte(WALK_SPEED) + 16) <= cur_room_width_pixels)
		{
			player1.vel_x16 = WALK_SPEED_16bit;
		}
	}

	// If the player has a horizontal velecity, be it from
	// pressing the DPAD above, or because they are airborne,
	// move the position by that velocity now.
	if (player1.vel_x16 != 0)
	{
		// Not sure if blocking the player at the edge of the screen will actually
		// ever be needed. Can be solved in level design.
		
		// // Don't move the player if we are approaching an edge.
		// if ((player1.vel_x < 0 && player1.pos_x >= (ABS(player1.vel_x) + FP_WHOLE(4))) || 
		//     (player1.vel_x > 0 && (player1.pos_x + player1.vel_x + FP_WHOLE(16) ) <= FP_WHOLE(cur_room_width_pixels)) )
		// {
		// 	player1.pos_x += player1.vel_x;
		// }

		player1.pos_x += FP_16_TO_32(player1.vel_x16);

		// track if the player hit a spike.
		hit_kill_box = 0;

		for (i = 0; i < NUM_Y_COLLISION_OFFSETS; ++i)
		{
			// take the player position, offset it a little
			// so that the arms overlap the wall a bit (+8),
			// convert to metatile space (16x16) (>>4).
			// The position is stored using fixed point math, 
			// where the high byte is the whole part, and the
			// low byte is the fraction.
			if (player1.vel_x16 < 0)
			{
				x = (high_2byte(player1.pos_x) + x_collision_offsets[0]) >> 4;
			}
			else
			{
				x = (high_2byte(player1.pos_x) + x_collision_offsets[NUM_X_COLLISION_OFFSETS - 1] + 1) >> 4;
			}

			// Player is 24 pixels high, so +12 checks middle of them.
			// >>4 to put the position into metatile-space (16x16 chunks).
			y = (high_2byte(player1.pos_y) + y_collision_offsets[i]) >> 4;

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
				if (tempFlags & FLAG_SOLID || (grounded && tempFlags & FLAG_KILL))
				{
					// Hit a wall, shift back to the edge of the wall.
					player1.pos_x = old_x;
					hit_kill_box = 0;

					if (dash_time > 0)
					{
						if (player1.vel_x16 < 0)
						{
							player1.vel_x16 = 448; // 1.75
						}
						else
						{
							player1.vel_x16 = -448;
						}
						player1.vel_y16 = -FP_WHOLE_16(5); // FP_WHOLE(-5);
						dash_time = 0;
						dash_count = 0;
						SFX_PLAY_WRAPPER(15);
					}
					else
					{
						player1.vel_x16 = 0;
					}

					break;
				}
			}
		}
	}

	if (pad_all & PAD_A)
	{
		++ticks_down;

		//is player on ground recently.
		//allow for jump right after 
		//walking off ledge.
		on_ground = (grounded != 0 || airtime < JUMP_COYOTE_DELAY);
		//was btn presses recently?
		//allow for pressing right before
		//hitting ground.
		new_jump_btn = ticks_down < 10;

		//is player continuing a jump
		//or starting a new one?
		if (jump_held_count > 0)
		{
			++jump_held_count;
			//keep applying jump velocity
			//until max jump time.
			if (jump_held_count < JUMP_HOLD_MAX)
			{
				player1.vel_y16 = -(JUMP_VEL_16bit);//keep going up while held
			}
		}
		else if ((on_ground && new_jump_btn && jump_count == 0))
		{
			++jump_held_count;
			++jump_count;
			player1.vel_y16 = -(JUMP_VEL_16bit);
			// The moment you just, the dash is over, but
			// we don't reset the dash_count until you land.
            dash_time = 0;
			SFX_PLAY_WRAPPER(5);
		}
		else if (jump_count < 1 && pad_all_new & PAD_A)
		{
			++jump_held_count;
			++jump_count;
			player1.vel_y16 = -(JUMP_VEL_16bit);
		}
	}
	else
	{
		// if (ticks_down > 0 && ticks_down < 5)
		// {
		//  	player1.vel_y >>= 2;
		// }
		ticks_down = 0;
		jump_held_count = 0;
	}

	// Start a dash if you aren't already in one
	if (dash_time == 0 && dash_count == 0 && pad_all_new & PAD_B)
	{
		// Try to make dash go exactly 4 meta tiles. Note that if dash_speed is greater than
		// 1, it will not land exactly on point as we always move the full dash_speed every frame.
		dash_time = DASH_LENGTH_TICKS;
		player1.vel_x16 = player1.dir_x * FP_WHOLE_16(DASH_SPEED);
		dash_count = 1;

		// When we start a dash, end the current jump if there is
		// one. This prevents the player from HOLDING A, dashing,
		// and continuing to hold A, resulting in the "continue jump"
		// logic overwriting the upward velocity when they hit a wall.
		jump_held_count = 0;

		SFX_PLAY_WRAPPER(10);
	}
	else if (dash_time > 0)
	{
		--dash_time;
		if (dash_time == 0)
		{
			player1.vel_y16 = 0;
			player1.vel_x16 = player1.dir_x * FP_DASH_SPEED_EXIT_16bit;
		}
	}

	// if (ticks_since_attack >= ATTACK_LEN)
	// {
	// 	if (pad_all_new & PAD_B)
	// 	{
	// 		ticks_since_attack = 0;
	// 	}
	// }
	// else
	// {
	// 	++ticks_since_attack;
	// }

	// Only apply gravity if you are dashing.
	if (dash_time == 0)
	{
		player1.vel_y16 += GRAVITY_16bit;
		player1.pos_y += FP_16_TO_32(player1.vel_y16);
	}

	// Assume not on the ground each frame, until we detect we hit it.
	grounded = 0;

	// Roof check
	if (player1.vel_y16 < 0)
	{
		for (i = 0; i < NUM_X_COLLISION_OFFSETS; ++i)
		{
			x = (high_2byte(player1.pos_x) + x_collision_offsets[i]) >> 4;
			y = (high_2byte(player1.pos_y)) >> 4; // head
			if (y < 15)
			{
				index16 = GRID_XY_TO_ROOM_INDEX(x, y);
				if (GET_META_TILE_FLAGS(index16) & FLAG_SOLID)
				{
					player1.pos_y = (unsigned long)((y << 4) + CELL_SIZE) << HALF_POS_BIT_COUNT;
					player1.vel_y16 = 0;
					// prevent hovering against the roof.
					jump_held_count = JUMP_HOLD_MAX;
					break;
				}
			}
		}					
	}
	else // floor check
	{
		hit_kill_box = 0;
		for (i = 0; i < NUM_X_COLLISION_OFFSETS; ++i)
		{
			x = (high_2byte(player1.pos_x) + x_collision_offsets[i]) >> 4;
			// + 1 because we want to check collision just below where the Left/Right checks were to avoid
			// getting "stuck"
			y = (high_2byte(player1.pos_y) + (y_collision_offsets[2] + 1)) >> 4; // feet

			if (y > 15 && y < 20)
			{
				hit_kill_box = 1;
			}
			else if (y < 15)
			{
				index16 = GRID_XY_TO_ROOM_INDEX(x, y);
				tempFlags = GET_META_TILE_FLAGS(index16);
				if (tempFlags & FLAG_SOLID)
				{
					jump_count = 0;
					grounded = 1;
					player1.pos_y = (unsigned long)((y << 4) - (y_collision_offsets[2] + 1)) << HALF_POS_BIT_COUNT;
					player1.vel_y16 = 0;
					airtime = 0;
					hit_kill_box = 0;
					// Don't reset the dash count unless the player has stopped dashing,
					// since they will hit the floor while dashing along the ground.
					if (dash_time == 0)	dash_count = 0;
					break;
				}

				// We want the kill check to be more forgiving.
				y = (high_2byte(player1.pos_y) + 12) >> 4; // feet
				// Make sure the new y value is also on screen.
				if (y < 15)
				{
					index16 = GRID_XY_TO_ROOM_INDEX(x, y);
					tempFlags = GET_META_TILE_FLAGS(index16);
					if (tempFlags & FLAG_KILL)
					{
						hit_kill_box = 1;
					}
				}
			}
		}	

		if (hit_kill_box == 1)
		{
			banked_call(BANK_0, kill_player);
		}	
	}

	if (grounded == 0)
	{
		++airtime;
		if (airtime >= JUMP_COYOTE_DELAY && jump_count == 0)
		{
			// We fell of a ledge. eat a jump so that you can't fall->jump->jump to get further.
			jump_count++;
		}
	}

	if (dash_time == 0)
	{
		if (pad_all & PAD_LEFT)
		{
			player1.dir_x = -1;
		}
		else if (pad_all & PAD_RIGHT)
		{
			player1.dir_x = 1;
		}
	}

	if (dash_time > 0)
	{
		anim_index = ANIM_PLAYER_DASH;
		in_working_anim_index = player1.anim_data_index;
		queue_next_anim(anim_index);
		commit_next_anim();
	}
	else if (!grounded)
	{
		if (player1.vel_y16 > 0)
		{
			anim_index = ANIM_PLAYER_FALL;
			in_working_anim_index = player1.anim_data_index;
			queue_next_anim(anim_index);
			commit_next_anim();
		}
		else
		{
			anim_index = ANIM_PLAYER_JUMP;
			in_working_anim_index = player1.anim_data_index;
			queue_next_anim(anim_index);
			commit_next_anim();
		}
	}
	else if (pad_all & (PAD_RIGHT | PAD_LEFT))
	{
		anim_index = ANIM_PLAYER_RUN;
		in_working_anim_index = player1.anim_data_index;
		queue_next_anim(anim_index);
		commit_next_anim();
	}
	else
	{
		anim_index = ANIM_PLAYER_IDLE;
		in_working_anim_index = player1.anim_data_index;
		queue_next_anim(anim_index);
		commit_next_anim();
	}
}