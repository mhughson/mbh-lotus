#include "PRG0.h"
#include "PRG1.h"
#include "PRG2.h"
#include "PRG3.h"
#include "PRG4.h"
#include "PRG5.h"
#include "main.h"
#include "LIB/neslib.h"
#include "LIB/nesdoug.h"
#include "mmc3\mmc3_code.h"
#include "../include/stdlib.h"

#pragma rodata-name ("BANK0")
#pragma code-name ("BANK0")

// Const data
//

const unsigned char BG_palettes[][16] =
{
	{ 0x0f,0x14,0x24,0x35,0x0f,0x0c,0x1c,0x3c,0x0f,0x07,0x17,0x3d,0x0f,0x09,0x19,0x29 },
	{ 0x0f,0x28,0x1a,0x2a,0x0f,0x28,0x18,0x38,0x0f,0x1c,0x0c,0x2c,0x0f,0x28,0x00,0x30 },
};

const unsigned char SPR_palettes[][16] =
{
	{ 0x24,0x04,0x23,0x38,0x24,0x04,0x27,0x38,0x24,0x13,0x23,0x33,0x24,0x14,0x24,0x34 },
};

const unsigned char palette_title[16]={ 0x0f,0x15,0x25,0x30,0x0f,0x13,0x25,0x30,0x0f,0x06,0x16,0x26,0x0f,0x09,0x19,0x29 };

#define NUM_Y_COLLISION_OFFSETS 3
const unsigned char y_collision_offsets[NUM_Y_COLLISION_OFFSETS] = { 1, 10, 19 };
#define NUM_X_COLLISION_OFFSETS 2
const unsigned char x_collision_offsets[NUM_X_COLLISION_OFFSETS] = { 0, 12 };

//const unsigned char bg_banks[4] = { 3, 8, 9, 10 };

//const unsigned char bg_banks[4] = { 27, 28, 29, 30 };

const unsigned char bg_bank_sets[NUM_BG_BANK_SETS][NUM_BG_BANKS] =
{
	{ 3, 8, 10, 20 },
	{ 26, 28, 30, 32 },
};

unsigned char irq_array[32];
unsigned char irq_array_buffer[32];

//unsigned char oam_base;

#if DEBUG_ENABLED
// Debug hack to test teleporting around a map.
unsigned int debug_pos_start;
#endif // DEBUG_ENABLED

// Functions
//

void update_player();

void main_real()
{
	unsigned int local_old_cam_x;
	unsigned char cur_bg_bank;
	unsigned char local_i;
	signed char local_inc;

	set_mirroring(MIRROR_VERTICAL);

	irq_array[0] = IRQ_END; // end of data
	irq_array_buffer[0] = IRQ_END;
	set_irq_ptr(irq_array); // point to this array
	
	
	// SAVE_VERSION is a magic number which means that this is valid save data.
	// Any other value is assumed to be garbage and therefore this is
	// a first time booting.
	// We check multiple entries in an array to reduce the chance of a fluke where the
	// random piece of data happens to match the version number.
	for (i = 0; i < NUM_SAVE_VERSION_VALIDATION; ++i)
	{
		if (save_version_validation[i] != SAVE_VERSION)
		{
			// clear all of WRAM.
			memfill(save_version_validation, 0, 0x2000);

			for (i = 0; i < NUM_SAVE_VERSION_VALIDATION; ++i)
			{
				// Store the save version as our magic number to check next time.
				save_version_validation[i] = SAVE_VERSION;
			}

			// SET NON-ZERO DEFAULT VALUES HERE.
			
			checkpoint_room_index = 0xff;
			checkpoint_spawn_id = 0xff;

			// Clear all the game options to a default value. Currently
			// using 1 because that is ON for the only 2 settings: audio.
			// As more settings are added, this might need to change.
			memfill(game_option_list_values, 1, OPTION_LIST_LEN);

			break;
		}
	}

	memfill(current_room, 0, MAX_ROOM_NUM_TILES);

    ppu_off(); // screen off

	set_vram_buffer(); // do at least once, sets a pointer to a buffer
	clear_vram_buffer();

	bank_bg(0);
	bank_spr(1);

	//ppu_on_all(); // turn on screen

	//pal_bright(4);

	// Horizontal scrolling...
//	set_mirror_mode(MIRROR_MODE_VERT);

	//music_play(1);

	// If player is always 0, does it really need to store this index?
	player1.anim_data_index = 0;
	animation_data_count = 1;

	go_to_state(STATE_TITLE);

	// infinite loop
	while (1)
	{
		++tick_count;
		++tick_count16;
		++ticks_in_state16;

		ppu_wait_nmi(); // wait till beginning of the frame
PROFILE_POKE(PROF_W)

		oam_clear();

		//oam_base += (39 * 4);
		//oam_set(oam_base);

		// When a VRAM update is queued, we can't do it while the screen is
		// drawing or it will change the visuals for the *previous* frame, which
		// are in the middle of being rendered (as these changes take place instantly).
		if (chr_index_queued != 0xff)
		{
			// TODO: Eventually we might want to support setting the other parts of
			// 		 VRAM as well.
			set_chr_mode_2(chr_index_queued);
			// HACK: Currently the sprites assume 2k of VRAM. Eventually
			//		 we will want to redo the metasprites to operate on
			//	 	 1k slices to make better use of the space.
			set_chr_mode_3(chr_index_queued + 1);
			chr_index_queued = 0xff;
		}

		if (chr_3_index_queued != 0xff)
		{
			set_chr_mode_4(chr_3_index_queued);
			chr_3_index_queued = 0xff;
		}

		pad_all = pad_poll(0) | pad_poll(1); // read the first controller
		pad_all_new = get_pad_new(0) | get_pad_new(1); // newly pressed button. do pad_poll first

		clear_vram_buffer(); // do at the beginning of each frame

		switch(cur_state)
		{
			case STATE_TITLE:
			{
				if (pad_all_new & PAD_ANY_CONFIRM_BUTTON)
				{
					if (checkpoint_room_index != 0xff && checkpoint_spawn_id != 0xff)
					{
						cur_room_index = checkpoint_room_index;
						in_destination_spawn_id = checkpoint_spawn_id;
					}
					go_to_state(STATE_GAME);
				}
				else
				{
					IRQ_CMD_BEGIN;
					
					// All these command swill run at the start of the frame.
					// Swap the first 4k section of graphics with the 16th chunk of graphics.
					IRQ_CMD_CHR_MODE_0(16);

					// All the commands after this point will run after scanline 166 is drawn.
					IRQ_CMD_SCANLINE(166);
					// Swap the first 4k section of graphics with the 36th chunk of graphics.
					IRQ_CMD_CHR_MODE_0(36);
					// Scroll the screen by 1 pixel every frame (looping every 256 pixels).
					IRQ_CMD_H_SCROLL(ticks_in_state16 % 256);
					// Switch to the correct nametable.
					IRQ_CMD_WRITE_2000(0x88 | (ticks_in_state16 % 512 < 256 ? 0 : 1));
					// Tint the screen "yellowish".
					IRQ_CMD_WRITE_2001(PROF_R_TINT | PROF_G_TINT);
					
					// Signal the end of the commands.
					IRQ_CMD_END;
				}
				break;
			}

			case STATE_GAME:
			{

				if (tick_count % 32 == 0)
				{
					cur_bg_bank = (cur_bg_bank + 1) % 4;
					// The second half of the background CHR is 
					// used for animations.
					set_chr_mode_1(bg_bank_sets[cur_room_metatile_index][cur_bg_bank]);
				}

				// store the camera position at the start of the frame, so that
				// we can detect if it moved by the end of the frame.
				local_old_cam_x = cam.pos_x;

PROFILE_POKE(PROF_G);
				if (cur_room_type == 1)
				{
					banked_call(BANK_3, update_player_td);
				}
				else
				{
					update_player();
				}
PROFILE_POKE(PROF_W);

				// update the trigger objects.
				for (local_i = 0; local_i < MAX_TRIGGERS; ++local_i)
				{
					if (trig_objs.type[local_i] != TRIG_UNUSED)
					{
						switch (trig_objs.type[local_i])
						{
							case TRIG_TRANS_POINT:
							{
								// For side scrolling levels, we only care about the X collision, so that the
								// transition happens for the entire height of the screen.
								// This means that we want to add "doors" again, they will need to restore the
								// Y check, or be a new TRIG type (which also require UP to be pressed).
								if ((high_2byte(player1.pos_x) + 8) / 16 == trig_objs.pos_x_tile[local_i] &&
									(cur_room_type == 0 ||
									(high_2byte(player1.pos_y) + 8) / 16 == trig_objs.pos_y_tile[local_i]))
								{
									cur_state = 0xff;
									// right 5 bits are destination level (max value 31)
									cur_room_index = (trig_objs.payload[local_i]) & 0b00011111;
									// left 3 bits are the destination spawn point (max value 7)
									in_destination_spawn_id = (trig_objs.payload[local_i] >> 5);
									go_to_state(STATE_GAME);
									// avoid trigger a transition back from the newly loaded level.
									goto skip_remaining;
								}
								break;
							}

							case TRIG_TRANS_EDGE:
							{
								// 1 for right size, 0 for left side.
								// Left side is different from right because the left most pixels
								// are hidden so it needs to collide sooner.
								index = 1;
								if (pad_all & PAD_LEFT) index = 0;
								// NOTE: Only checking for X collision so that this spans the height of the screen.
								if ((high_2byte(player1.pos_x) + y_collision_offsets[index] ) / 16 == trig_objs.pos_x_tile[local_i])
								{
									// Figure out if we are headed left or right based on the position in world.
									if (trig_objs.pos_x_tile[local_i] > 8)
									{
										in_stream_direction = 1;
									}
									else if (trig_objs.pos_x_tile[local_i] <= 8)
									{
										in_stream_direction = 0;
									}

									// Right 5 bits are the destination level.
									// Left 3 bits are currently unused.
									cur_room_index = (trig_objs.payload[local_i]) & 0b00011111;

									banked_call(BANK_2, stream_in_next_level);
									// Avoid triggering additional things in the newly loaded level.
									goto skip_remaining;
								}
								break;
							}
						}
					}
				}

				// Handle case where the state changed in update_player()
				// We want to make sure that the state changes hold (eg. scroll)
				// and don't get overwritten through the rest of this state.
				if (cur_state != STATE_GAME)
				{
					clear_vram_buffer();
					break;
				}

				if (cur_room_type == 1)
				{
					banked_call(BANK_3, update_cam_td);
				}
				else
				{
				#define CAM_DEAD_ZONE 16

				// move the camera to the player if needed.
				if (high_2byte(player1.pos_x) > cam.pos_x + (128 + CAM_DEAD_ZONE) && cam.pos_x < cur_room_width_pixels-256)
				{
					cam.pos_x = MIN(cur_room_width_pixels-256, high_2byte(player1.pos_x) - (128 + CAM_DEAD_ZONE));
				}
				else if (high_2byte(player1.pos_x) < cam.pos_x + (128 - CAM_DEAD_ZONE) && cam.pos_x > 0 ) //(cam.pos_x / 256) == ((high_2byte(player1.pos_x) - 64) / 256))
				{
					// TODO: There is probably a chance the rolls over to -1 if the player doesn't start at 0,0. Can't clamp
					//		 because pos_x is unsigned. Need to detect roll over and clamp to 0.
					local_old_cam_x = cam.pos_x;
					cam.pos_x = high_2byte(player1.pos_x) - (128 - CAM_DEAD_ZONE);
					if (local_old_cam_x < cam.pos_x) cam.pos_x = 0;
				}

				// This should really be >> 4 (every 16 pixels) but that will miss the initial
				// row loading. Could update "load_map" but the nametable logic is kind of annoying
				// for the non-vram version. Will dig in more later.
				if ((local_old_cam_x >> 3) < (cam.pos_x >> 3) || cur_nametable_y_right != 0)
				{
					in_x_tile = 1 + (cam.pos_x + 256) / 16;
					in_x_pixel = 16 + (cam.pos_x + 256);
					cur_nametable_y = cur_nametable_y_right;
					vram_buffer_load_column();

					cur_nametable_y_right += NAMETABLE_TILES_8_UPDATED_PER_FRAME;
					if (cur_nametable_y_right >= 30) cur_nametable_y_right = 0;					
				}
				if ((local_old_cam_x >> 3) > (cam.pos_x >> 3) || cur_nametable_y_left != 0)
				{
					in_x_tile = (cam.pos_x) / 16;
					in_x_pixel = (cam.pos_x);
					cur_nametable_y = cur_nametable_y_left;
					vram_buffer_load_column();

					cur_nametable_y_left += NAMETABLE_TILES_8_UPDATED_PER_FRAME;
					if (cur_nametable_y_left >= 30) cur_nametable_y_left = 0;		
				}

				// update the freeze and thaw values at the end of the frame.
				cam.freeze_left = (cam.pos_x >= FROZEN_OFFSET ? cam.pos_x - FROZEN_OFFSET : 0);
				cam.freeze_right = (cam.pos_x <= cur_room_width_pixels - 256 - FROZEN_OFFSET ? cam.pos_x + 256 + FROZEN_OFFSET : cur_room_width_pixels);
				cam.thaw_left = (cam.pos_x >= THAW_OFFSET ? cam.pos_x - THAW_OFFSET : 0);
				cam.thaw_right = (cam.pos_x <= cur_room_width_pixels - 256 - THAW_OFFSET ? cam.pos_x + 256 + THAW_OFFSET : cur_room_width_pixels);	

				// cur_col is the last column to be loaded, aka the right
				// hand side of the screen. The scroll amount is relative to the 
				// left hand side of the screen, so offset by 256.
				set_scroll_x(cam.pos_x);

				banked_call(BANK_1, draw_player);

				// flip flop the order that objects are drawn to create "flickering".
				if (tick_count % 2 == 0)
				{
					local_i = 0;
					local_inc = 1;
				}
				else
				{
					local_i = MAX_DYNAMIC_OBJS - 1;
					local_inc = -1;
				}

				// update the trigger objects.
				for (; local_i < MAX_DYNAMIC_OBJS; local_i += local_inc)
				{
					if (dynamic_objs.type[local_i] != TRIG_UNUSED)
					{
						if ((dynamic_objs.state[local_i] & DYNAMIC_STATE_FROZEN) == 0)
						{
							// Only advance when thawed, that way things like lookup offsets don't jump
							// after a thaw.
							++dynamic_objs.time_in_state[local_i];

							switch (dynamic_objs.type[local_i])
							{
								case TRIG_BIRD:
								{
									in_dynamic_obj_index = local_i;
									banked_call(BANK_4, update_bird);

									// The update call might have killed them.
									if (dynamic_objs.type[local_i] != TRIG_UNUSED && (dynamic_objs.state[local_i] & DYNAMIC_STATE_FROZEN) == 0)
									{
										commit_next_anim();
										in_dynamic_obj_index = local_i;
										banked_call(BANK_1, draw_bird);
									}
									break;									
								}
								case TRIG_SKELETON:
								{
									in_dynamic_obj_index = local_i;
									banked_call(BANK_4, update_skeleton);

									// The update call might have killed them.
									if (dynamic_objs.type[local_i] != TRIG_UNUSED && (dynamic_objs.state[local_i] & DYNAMIC_STATE_FROZEN) == 0)
									{
										commit_next_anim();
										in_dynamic_obj_index = local_i;
										banked_call(BANK_1, draw_skeleton);
									}
									break;
								}
							}
						}
						else
						{
							// try to wake it up.
							switch (dynamic_objs.type[local_i])
							{
								case TRIG_BIRD:
								case TRIG_SKELETON:
								{
									if (dynamic_objs.pos_x[local_i] > cam.thaw_left &&
										dynamic_objs.pos_x[local_i] < cam.thaw_right)
									{
										dynamic_objs.state[local_i] ^= DYNAMIC_STATE_FROZEN;
									}
									break;
								}
							}							
						}
					}
				}					

#if DEBUG_ENABLED
				if (pad_all_new & PAD_SELECT)
				{
					cur_state = 0xff;
					++cur_room_index;
					go_to_state(STATE_GAME);
				}
#endif // DEBUG_ENABLED
				}
				
				// Moved this down here because I was getting a hang when transitioning
				// from TD level to side scrolling level (leaving sprites in bad state).
				// I suspect it has something to do with chaning game modes mid-update, 
				// so I moved this here.
				// It seemed be crashing inside a call to draw_player->oam_spr. The call
				// stack looked suspect.
				skip_remaining:
				break;
			}

			case STATE_OVER:
			{
				if (pad_all_new & PAD_ANY_CONFIRM_BUTTON)
				{
					// Go back to the last checkpoint.
					if (checkpoint_room_index != 0xff && checkpoint_spawn_id != 0xff)
					{
						cur_room_index = checkpoint_room_index;
						in_destination_spawn_id = checkpoint_spawn_id;
					}
					go_to_state(STATE_GAME);
				}
				else if (pad_all_new & PAD_B)
				{
					go_to_state(STATE_TITLE);
				}
				break;
			}
		}

//gray_line();

PROFILE_POKE(PROF_CLEAR)

		// wait till the irq system is done before changing it
		// this could waste a lot of CPU time, so we do it last
		while(!is_irq_done() ){ // have we reached the 0xff, end of data
							// is_irq_done() returns zero if not done
			// do nothing while we wait
		}

		// copy from double_buffer to the irq_array
		// memcpy(void *dst,void *src,unsigned int len);
		memcpy(irq_array, irq_array_buffer, sizeof(irq_array)); 
	}
}

void kill_player()
{
	go_to_state(STATE_OVER);
}



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
			kill_player();
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

void copy_current_map_to_nametable()
{
	static unsigned int start_x;
	static unsigned int start_y;
	static unsigned int nt;
	static unsigned char local_loop;
	static unsigned char local_offset;
	static unsigned int cam_y;

	//shake_remaining = 0;

	//banked_call(BANK_5, copy_and_process_map_data);

	// Figure out where the start of the nametable the left
	// side of the camera is in. This is where we will start
	// loading in tile data, and will determine which nametable
	// to load data into.
	start_x = ((cam.pos_x / 256) * 256) / 16; // start of Nametable A

	// Y position does not clamp to nametables, is it will need to 
	// start in (potentially) the vertical middle of the name tables
	// and loop back on itself. This is because we use vertical mirroring.
	start_y = cam.pos_y / 16;

	// Figure out which nametable the camera is starting in.
	if (cam.pos_x % 512 < 256)
	{
		nt = NAMETABLE_A;
	}
	else
	{
		nt = NAMETABLE_B;
	}

	for (y = 0; y < 15; ++y)
	{
		for (x = 0; x < 16; ++x)
		{
			// Find the tile, offset by by the start x and y.
			index16 = GRID_XY_TO_ROOM_INDEX(start_x + x, start_y + y);
			index16 = current_room[index16] * META_TILE_NUM_BYTES;
			// The Y position needs to loop around on itself at the 
			// bootom of the nametable (tile 30).
			vram_adr(NTADR(nt,x*2,(((y*2) + (cam.pos_y / 8)) % 30)));	
			vram_write(&cur_metatiles[index16], 2);
			vram_adr(NTADR(nt,x*2,((y*2) + ((cam.pos_y / 8)) + 1) % 30));	
			vram_write(&cur_metatiles[index16+2], 2);
		}
	}
 	
	// Because nametable data is duplicated in memory, and
	// because we only support vertical mirroring atm, this
	// logic works. For example, incrementing from table B
	// to table C, is ok because table C is a copy of A.
	nt += 0x400;
	for (y = 0; y < 15; ++y)
	{
		for (x = 0; x < 16; ++x)
		{
			index16 = GRID_XY_TO_ROOM_INDEX(start_x + x + 16, start_y + y);
			index16 = current_room[index16] * META_TILE_NUM_BYTES;
			vram_adr(NTADR(nt,x*2,(((y*2) + (cam.pos_y / 8)) % 30)));	
			vram_write(&cur_metatiles[index16], 2);
			vram_adr(NTADR(nt,x*2,((y*2) + ((cam.pos_y / 8)) + 1) % 30));	
			vram_write(&cur_metatiles[index16+2], 2);
		}
	}

	// Go back to the start of the first nametable.
	nt-=0x400;

	// Used to offset for left/right nametables.
	local_offset = 0;
	cam_y = ((cam.pos_y / 32) * 32);
	if (cam_y % 480 > 240) cam_y += 16;
	index = ((cam_y % 240) / 32) * 8;
	start_y = cam_y / 16;

	// Loop through left and right nametables.
	for (local_loop = 0; local_loop < 2; ++local_loop)
	{
		for (y = 0; y < 15; y+=2)
		{
			for (x = 0; x < 16; x+=2)
			{
				i = 0;

				// room index.
				index16 = GRID_XY_TO_ROOM_INDEX(start_x + x + local_offset, start_y + y);
				// meta tile palette index.
				index16 = (current_room[index16] * META_TILE_NUM_BYTES) + 4;
				// bit shift amount
				i |= (cur_metatiles[index16]);

				index16 =  GRID_XY_TO_ROOM_INDEX(start_x + x + 1 + local_offset, start_y + y);
				index16 = (current_room[index16] * META_TILE_NUM_BYTES) + 4;
				i |= (cur_metatiles[index16]) << 2;

				index16 =  GRID_XY_TO_ROOM_INDEX(start_x + x + local_offset, start_y + y + 1);
				index16 = (current_room[index16] * META_TILE_NUM_BYTES) + 4;
				i |= (cur_metatiles[index16]) << 4;

				index16 =  GRID_XY_TO_ROOM_INDEX(start_x + x + 1 + local_offset, start_y + y + 1);
				index16 = (current_room[index16] * META_TILE_NUM_BYTES) + 4;
				i |= (cur_metatiles[index16]) << 6;	

				vram_adr(nt + 960 + index);	
				vram_write(&i, 1);

				++index;

				// Handle the case where the Y position goes past the bottom of the
				// current nametable, in which case it should loop back up to the
				// top of the current nametable.
				if (index % 64 == 0)
				{
					index = 0;

					// For the last row of attributes, the logic above will *think*
					// that it row to 2 rows of meta tiles, but the bottom row will
					// not have done anything because that data is not displayed.
					y -= 1;
				}
			}
		}

		// Move to the next horizontal nametable, and reset the tile data
		// back to the top.
		nt+=0x400;
		index = ((cam_y % 240) / 32) * 8;
		local_offset += 16;
	}

	return;
}

// LARGELY UNTESTED!
// ATTRIBUTES LOOKS LIKE THEY WOULD NOT WORK. ASSUMES 16 TILE WIDE LEVELS
/*
void vram_buffer_load_2x2_metatile()
{
	// Function gets called from a lot of places, so not safe to use globals.
	static unsigned char local_x;
	static unsigned char local_y;
	static unsigned char local_i;
	static unsigned int local_index16;
	static unsigned int local_att_index16;
    static unsigned char nametable_index;

    nametable_index = (in_x_tile / 16) % 2;

	// TILES

	local_index16 = GRID_XY_TO_ROOM_INDEX(in_x_tile, in_y_tile);
	local_att_index16 = current_room[local_index16] * META_TILE_NUM_BYTES;
	multi_vram_buffer_horz(&cur_metatiles[local_att_index16], 2, get_ppu_addr(nametable_index, in_x_tile * CELL_SIZE, in_y_tile * CELL_SIZE));
	multi_vram_buffer_horz(&cur_metatiles[local_att_index16+2], 2, get_ppu_addr(nametable_index, in_x_tile * CELL_SIZE, (in_y_tile * CELL_SIZE) + 8));

	// ATTRIBUTES

	// Attributes are in 2x2 meta tile chunks, so we need to round down to the nearest,
	// multiple of 2 (eg. if you pass in index 5, we want to start on 4).
	local_x = (in_x_tile / 2) * 2;
	local_y = (in_y_tile / 2) * 2;

	local_i = 0;

    // TODO: DONT ASSUME 16 TILE WIDE LEVELS

	// room index.
	local_index16 = GRID_XY_TO_ROOM_INDEX(local_x, local_y);
	// meta tile palette index.
	local_att_index16 = (current_room[local_index16] * META_TILE_NUM_BYTES) + 4;
	// bit shift amount
	local_i |= (cur_metatiles[local_att_index16]);

	local_index16 = local_index16 + 1; //(local_y * 16) + (local_x + 1);
	local_att_index16 = (current_room[local_index16] * META_TILE_NUM_BYTES) + 4;
	local_i |= (cur_metatiles[local_att_index16]) << 2;

	local_index16 = local_index16 + 15; //((local_y + 1) * 16) + (local_x);
	local_att_index16 = (current_room[local_index16] * META_TILE_NUM_BYTES) + 4;
	local_i |= (cur_metatiles[local_att_index16]) << 4;

	local_index16 = local_index16 + 1; //((local_y + 1) * 16) + (local_x + 1);
	local_att_index16 = (current_room[local_index16] * META_TILE_NUM_BYTES) + 4;
	local_i |= (cur_metatiles[local_att_index16]) << 6;	

	one_vram_buffer(local_i, get_at_addr(nametable_index, (local_x) * CELL_SIZE, (local_y) * CELL_SIZE));
}
*/

void vram_buffer_load_column()
{
	// TODO: Remove int is a significant perf improvment.
	static unsigned char local_y;
	static unsigned char local_i;
	static unsigned int local_index16;
	static unsigned int local_att_index16;
	static unsigned char nametable_index;
	static unsigned char tile_offset;
	static unsigned char tile_offset2;
	static unsigned char local_cur_nametable_y_pixel;
	static unsigned int nametable_attr_address;

	static unsigned char array_temp8;

PROFILE_POKE(PROF_R)

	nametable_index = (in_x_tile / 16) % 2;

	local_cur_nametable_y_pixel = cur_nametable_y * 8;

	// TILES

	tile_offset = (in_x_pixel % 16) / 8;
	tile_offset2 = tile_offset+2;

	local_index16 = GRID_XY_TO_ROOM_INDEX(in_x_tile, cur_nametable_y/2);

	for (local_i = 0; local_i < NAMETABLE_TILES_8_UPDATED_PER_FRAME; )
	{
		local_att_index16 = current_room[local_index16] * META_TILE_NUM_BYTES;

		// single column of tiles
		array_temp8 = cur_metatiles[local_att_index16 + tile_offset];
		nametable_col[local_i] = array_temp8;
		local_i++;
		array_temp8 = cur_metatiles[local_att_index16 + tile_offset2];
		nametable_col[local_i] = array_temp8;
		local_i++;

		local_index16 += cur_room_width_tiles;
	}

	// The screen is only 30 tiles high, which doesn't always divide nicely into out metatile updates, so map sure we don't
	// update beyond the bottom of the screen.
	array_temp8 = MIN(NAMETABLE_TILES_8_UPDATED_PER_FRAME, 30 - cur_nametable_y);

	multi_vram_buffer_vert(nametable_col, (unsigned char)array_temp8, get_ppu_addr(nametable_index, in_x_pixel, local_cur_nametable_y_pixel));

PROFILE_POKE(PROF_G)

	// ATTRIBUTES

	// Convert tile index to attribute group index.
	local_att_index16 = in_x_tile / 2;

	// Cache the nametable address which can be manually incremented in loop.
	nametable_attr_address = get_at_addr(nametable_index, local_att_index16 * 32, (local_cur_nametable_y_pixel));	

	// The index into the attribute table.
	local_att_index16 += ((cur_nametable_y / 4) * cur_room_width_attributes);

	for (local_y = 0; local_y < NAMETABLE_ATTRIBUTES_16_UPDATED_PER_FRAME; ++local_y)
	{
		// Get the attribute for this 2x2 chunk of metatiles.
		local_i = current_room_attr[local_att_index16];
		one_vram_buffer(local_i, nametable_attr_address);

		// Increment to the next row of attribute chunks.
		local_att_index16 += cur_room_width_attributes;
		nametable_attr_address += 8;
	}
PROFILE_POKE(PROF_W)
}

// TODO: Can the full and timesliced versions of these functions be combined
//       to avoid duplicate code?
void vram_buffer_load_column_full()
{
	// TODO: Remove int is a significant perf improvment.
	static unsigned char local_x;
	static unsigned char local_y;
	static unsigned char local_i;
	static unsigned int local_index16;
	static unsigned int local_att_index16;
	static unsigned char nametable_index;
	static unsigned char tile_offset;
	static unsigned char tile_offset2;

	static unsigned char array_temp8;

	// Slightly hacky logic to determine which nametable to load into
	// and potentially REVERSE the logic, if asked to. Very specific
	// to the level transition logic.
	if (cam.pos_x % 512 < 256)
	{
		nametable_index = in_flip_nt ? 0 : 1; // in A, show B
	}
	else
	{
		nametable_index = in_flip_nt ? 1 : 0; // in B, show A
	}

	// TILES

	tile_offset = (in_x_pixel % 16) / 8;
	tile_offset2 = tile_offset+2;

	local_index16 = GRID_XY_TO_ROOM_INDEX(in_x_tile, 0);

	for (local_i = 0; local_i < 30; )
	{
		local_att_index16 = current_room[local_index16] * META_TILE_NUM_BYTES;

		// single column of tiles
		array_temp8 = cur_metatiles[local_att_index16 + tile_offset];
		nametable_col[local_i] = array_temp8;
		local_i++;
		array_temp8 = cur_metatiles[local_att_index16 + tile_offset2];
		nametable_col[local_i] = array_temp8;
		local_i++;

		local_index16 += cur_room_width_tiles;
	}

	multi_vram_buffer_vert(nametable_col, (unsigned char)30, get_ppu_addr(nametable_index, in_x_pixel, 0));


	// ATTRIBUTES

	// Attributes are in 2x2 meta tile chunks, so we need to round down to the nearest,
	// multiple of 2 (eg. if you pass in index 5, we want to start on 4).
	local_x = (in_x_tile & 0xFFFE);//local_x = (in_x_tile / 2) * 2;
	//local_x = (in_x_tile / 2) * 2;

	for (local_y = 0; local_y < (15); local_y+=2)
	{
		local_i = 0;

		// room index.
		local_index16 = GRID_XY_TO_ROOM_INDEX(local_x, local_y);
		// meta tile palette index.
		local_att_index16 = (current_room[local_index16] * META_TILE_NUM_BYTES);
		local_att_index16+=4;
		// bit shift amount
		local_i |= (cur_metatiles[local_att_index16]);

		local_index16++;//local_index16 = local_index16 + 1; //(local_y * 16) + (local_x + 1);
		local_att_index16 = (current_room[local_index16] * META_TILE_NUM_BYTES);
		local_att_index16+=4;
		local_i |= (cur_metatiles[local_att_index16]) << 2;

		local_index16 = local_index16 + cur_room_width_tiles; //((local_y + 1) * 16) + (local_x);
		local_index16--;
		local_att_index16 = (current_room[local_index16] * META_TILE_NUM_BYTES);
		local_att_index16+=4;
		local_i |= (cur_metatiles[local_att_index16]) << 4;

		local_index16++;// = local_index16 + 1; //((local_y + 1) * 16) + (local_x + 1);
		local_att_index16 = (current_room[local_index16] * META_TILE_NUM_BYTES);
		local_att_index16+=4;
		local_i |= (cur_metatiles[local_att_index16]) << 6;	

		one_vram_buffer(local_i, get_at_addr(nametable_index, (local_x) * CELL_SIZE, ((local_y * CELL_SIZE))));
	}
}

void go_to_state(unsigned char new_state)
{
	cur_state = new_state;

	ticks_in_state16 = 0;

	switch(cur_state)
	{
		case STATE_TITLE:
		{
			fade_to_black();
			ppu_off();
			scroll(0,0);		

			// Title screen graphics into the BG VRAM.
			set_chr_mode_0(16);
			set_chr_mode_1(18);

			cur_room_index = 0;

			pal_bg(palette_title);
			pal_spr(palette_title);
			banked_call(BANK_4, load_screen_title);

			IRQ_CMD_BEGIN;
			IRQ_CMD_CHR_MODE_0(16);
			IRQ_CMD_SCANLINE(166);
			IRQ_CMD_CHR_MODE_0(36);
			IRQ_CMD_END;
			IRQ_CMD_FLUSH;

			ppu_on_all();
//			music_play(0);
			fade_from_black();
			MUSIC_PLAY_WRAPPER(0);
			break;
		}

		case STATE_GAME:
		{
			fade_to_black();

			// I am able to comment this out and the irq stuff still gets cleared out properly
			// but I think that is just luck. Not sure though.
			while(!is_irq_done()) {} 
			
			ppu_off();

			// Clear out any IRQ tasks before kicking off the next section.
			irq_array[0] = 0xff;
			irq_array_buffer[0] = 0xff;

			music_stop();
			scroll(0,0);
			cam.pos_x = 0;

			// Clear the queue to load sprite data.
			chr_index_queued = 0xff;
			chr_3_index_queued = 0xff;

// #if DEBUG_ENABLED
// 			player1.pos_x = FP_WHOLE(debug_pos_start);
// 			debug_pos_start += 128;
// 			if (debug_pos_start >= cur_room_width_pixels)
// 			{
// 				debug_pos_start = 0;
// 			}
// #else
			// TODO: This should come from the map or from
			//       a destination when coming through a 
			//		 door.
			player1.pos_x = FP_WHOLE(24);
//#endif // DEBUG_ENABLED
			player1.pos_y = FP_WHOLE((6<<4));
			player1.vel_y16 = 0;

			// Load the room first so that we know it's size.
			in_is_streaming = 0;
			banked_call(BANK_5, load_and_process_map);

			// Move the camera to the player, but clamp to the edges.
			if (high_2byte(player1.pos_x) < 128)
			{
				cam.pos_x = 0;
			}
			else if (high_2byte(player1.pos_x) > (cur_room_width_pixels - 128))
			{
				cam.pos_x = cur_room_width_pixels - 256;
			}
			else
			{
				cam.pos_x = high_2byte(player1.pos_x) - 128;
			}
			if (high_2byte(player1.pos_y) < 120)
			{
				cam.pos_y = 0;
			}
			else if (high_2byte(player1.pos_y) > (cur_room_height_pixels - 120))
			{
				cam.pos_y = cur_room_height_pixels - 240;
			}
			else
			{
				cam.pos_y = high_2byte(player1.pos_y) - 120;
			}

			// Copy the map data to nametables based on the player position.
			copy_current_map_to_nametable();
			

			ppu_on_all();
	
			// pre-adjust scroll, and wait for that to get 
			// pushed to the ppu.
			scroll(cam.pos_x, cam.pos_y);	
			ppu_wait_nmi();	

			fade_from_black();
			break;
		}

		case STATE_OVER:
		{
			// bit of a hack to keep the player visible during the fade to
			// white.
			music_stop();
			banked_call(BANK_1, draw_player);
			fade_to_white();
			ppu_off();
			scroll(0,0);		
//			set_chr_bank_0(0);	
			pal_bg(palette_title);
			pal_spr(palette_title);		
			banked_call(BANK_4, load_screen_gameover);
			// we draw the player sprites at the start of the white fade,
			// so now we need to clear it before fading back in or else you
			// will see her for a few frames.
			oam_clear();
			ppu_on_all();
			fade_from_white();
		}
	}
}

void set_chr_bank_for_current_room()
{
	switch(cur_room_metatile_index)
	{
		case 0:
		{
			set_chr_mode_0(0);
			set_chr_mode_1(2);
			break;
		}

		case 1:
		{
			set_chr_mode_0(24);
			set_chr_mode_1(26);
			break;
		}
	}	
}

void load_level_pal()
{
	banked_call(BANK_5, get_cur_room_palettes);

	pal_bg(BG_palettes[index]);
	pal_spr(SPR_palettes[index2]);
}