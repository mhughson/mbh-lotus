#include "PRG0.h"
#include "PRG1.h"
#include "PRG2.h"
#include "main.h"
#include "LIB/neslib.h"
#include "LIB/nesdoug.h"
#include "mmc3\mmc3_code.h"
#include "../include/stdlib.h"

#pragma rodata-name ("BANK0")
#pragma code-name ("BANK0")

// Const data
//

#include "meta_tiles_temp.h"
#include "NES_ST/screen_title.h"
#include "NES_ST/screen_gameover.h"

const unsigned char palette[16]={ 0x0f,0x05,0x23,0x37,0x0f,0x01,0x21,0x31,0x0f,0x06,0x16,0x26,0x0f,0x19,0x2a,0x3a };
const unsigned char palette_title[16]={ 0x0f,0x15,0x25,0x30,0x0f,0x13,0x25,0x30,0x0f,0x06,0x16,0x26,0x0f,0x09,0x19,0x29 };



#define NUM_Y_COLLISION_OFFSETS 3
const unsigned char y_collision_offsets[NUM_Y_COLLISION_OFFSETS] = { 1, 16, 31 };
#define NUM_X_COLLISION_OFFSETS 2
const unsigned char x_collision_offsets[NUM_X_COLLISION_OFFSETS] = { 4, 12 };

const unsigned char bg_banks[4] = { 3, 8, 9, 10 };

// TODO: Move to player
unsigned char anim_index;
unsigned char grounded;
unsigned char jump_held_count;
unsigned char can_jump;
unsigned char airtime;
unsigned char ticks_down;
unsigned char jump_count;
unsigned char on_ground;
unsigned char new_jump_btn;
unsigned int scroll_y;

unsigned char irq_array[32];
unsigned char double_buffer[32];

#if DEBUG_ENABLED
// Debug hack to test teleporting around a map.
unsigned int debug_pos_start;
#endif // DEBUG_ENABLED

// Functions
//

void kill_player();
void update_player();

void main_real()
{
	unsigned int old_cam_x;
	unsigned char cur_bg_bank;

	set_mirroring(MIRROR_VERTICAL);

	irq_array[0] = 0xff; // end of data
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

	go_to_state(STATE_TITLE);

	// infinite loop
	while (1)
	{
		++tick_count;

		ppu_wait_nmi(); // wait till beginning of the frame
		PROFILE_POKE(PROF_R)

		oam_clear();

		pad_all = pad_poll(0) | pad_poll(1); // read the first controller
		pad_all_new = get_pad_new(0) | get_pad_new(1); // newly pressed button. do pad_poll first

		clear_vram_buffer(); // do at the beginning of each frame

		switch(cur_state)
		{
			case STATE_TITLE:
			{
				if (pad_all_new & PAD_ANY_CONFIRM_BUTTON)
				{
					go_to_state(STATE_GAME);
				}
				break;
			}

			case STATE_GAME:
			{

				if (tick_count % 32 == 0)
				{
					cur_bg_bank = (cur_bg_bank + 1) % 4;
					set_chr_mode_5(bg_banks[cur_bg_bank]);
				}

				// store the camera position at the start of the framem, so that
				// we can detect if it moved by the end of the frame.
				old_cam_x = cam.pos_x;

				update_player();

				// Handle case where the state changed in update_player()
				// We want to make sure that the state changes hold (eg. scroll)
				// and don't get overwritten through the rest of this state.
				if (cur_state != STATE_GAME)
				{
					clear_vram_buffer();
					break;
				}

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
					old_cam_x = cam.pos_x;
					cam.pos_x = high_2byte(player1.pos_x) - (128 - CAM_DEAD_ZONE);
					if (old_cam_x < cam.pos_x) cam.pos_x = 0;
				}

				// This should really be >> 4 (every 16 pixels) but that will miss the initial
				// row loading. Could update "load_map" but the nametable logic is kind of annoying
				// for the non-vram version. Will dig in more later.
				if ((old_cam_x >> 3) < (cam.pos_x >> 3) || cur_nametable_y_right != 0)
				{
					in_x_tile = 1 + (cam.pos_x + 256) / 16;
					in_x_pixel = 16 + (cam.pos_x + 256);
					cur_nametable_y = cur_nametable_y_right;
					vram_buffer_load_column();

					cur_nametable_y_right += NAMETABLE_TILES_8_UPDATED_PER_FRAME;
					if (cur_nametable_y_right >= 30) cur_nametable_y_right = 0;					
				}
				if ((old_cam_x >> 3) > (cam.pos_x >> 3) || cur_nametable_y_left != 0)
				{
					in_x_tile = (cam.pos_x) / 16;
					in_x_pixel = (cam.pos_x);
					cur_nametable_y = cur_nametable_y_left;
					vram_buffer_load_column();

					cur_nametable_y_left += NAMETABLE_TILES_8_UPDATED_PER_FRAME;
					if (cur_nametable_y_left >= 30) cur_nametable_y_left = 0;		
				}

				banked_call(BANK_1, draw_player);

				// cur_col is the last column to be loaded, aka the right
				// hand side of the screen. The scroll amount is relative to the 
				// left hand side of the screen, so offset by 256.
				set_scroll_x(cam.pos_x);

#if DEBUG_ENABLED
				if (pad_all_new & PAD_SELECT)
				{
					cur_state = 0xff;
					++cur_room_index;
					go_to_state(STATE_GAME);
				}
#endif // DEBUG_ENABLED
				break;
			}

			case STATE_OVER:
			{
				if (pad_all_new & PAD_ANY_CONFIRM_BUTTON)
				{
					go_to_state(STATE_GAME);
				}
				else if (pad_all_new & PAD_B)
				{
					go_to_state(STATE_TITLE);
				}
				break;
			}
		}


		PROFILE_POKE(PROF_CLEAR)
	}
}

void kill_player()
{
	go_to_state(STATE_OVER);
}

void update_player()
{
	static unsigned char hit_kill_box;
	// static unsigned int high_x;
	// static unsigned int high_y;
	static const unsigned int high_walk_speed = (WALK_SPEED >> 16);
	static unsigned int local_i16;

//	PROFILE_POKE(PROF_G);

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
			x = (high_2byte(player1.pos_x) + x_collision_offsets[0] - 1) >> 4;

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
					player1.pos_x = temp32;
					hit_kill_box = 0;
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

		hit_kill_box = 0;

		for (i = 0; i < NUM_Y_COLLISION_OFFSETS; ++i)
		{
			// take the player position, offset it a little
			// so that the arms overlap the wall a bit (+16),
			// convert to metatile space (16x16) (>>4).
			// The position is stored using fixed point math, 
			// where the high byte is the whole part, and the
			// low byte is the fraction.
			x = (high_2byte(player1.pos_x) + x_collision_offsets[NUM_X_COLLISION_OFFSETS - 1] + 1) >> 4;

			// Player is 24 pixels high, so +12 checks middle of them.
			// >>4 to put the position into metatile-space (16x16 chunks).
			y = (high_2byte(player1.pos_y) + y_collision_offsets[i]) >> 4;

			if (y < 15)
			{
				// Convert the x,y into an index into the room data array.
				//temp16 = GRID_XY_TO_ROOM_NUMBER(x, y);
				index16 = GRID_XY_TO_ROOM_INDEX(x, y);

				tempFlags = GET_META_TILE_FLAGS(index16);

				// Check if that point is in a solid metatile.
				// Treat spikes like walls if the player is on the floor. It feels lame to 
				// walk into a spike and die; you should need to fall into them.
				if (tempFlags & FLAG_SOLID || (grounded && tempFlags & FLAG_KILL))
				{
					// Hit a wall, shift back to the edge of the wall.
					player1.pos_x = temp32; //(unsigned long)((x << 4) - 17) << HALF_POS_BIT_COUNT;
					hit_kill_box = 0;

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
				player1.vel_y = -(JUMP_VEL);//keep going up while held
			}
		}
		else if ((on_ground && new_jump_btn && jump_count == 0))
		{
			++jump_held_count;
			++jump_count;
			player1.vel_y = -(JUMP_VEL);
			sfx_play(5,0);
		}
		else if (jump_count < 1 && pad_all_new & PAD_A)
		{
			++jump_held_count;
			++jump_count;
			player1.vel_y = -(JUMP_VEL);
		}
	}
	else
	{
		// if (ticks_down > 0 && ticks_down < 5)
		// {
		// 	player1.vel_y >>= 2;
		// }
		ticks_down = 0;
		jump_held_count = 0;
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

	player1.vel_y += GRAVITY;
	player1.pos_y += player1.vel_y;

	// Assume not on the ground each frame, until we detect we hit it.
	grounded = 0;

	// Roof check
	if (player1.vel_y < 0)
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
					player1.vel_y = 0;
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
			y = (high_2byte(player1.pos_y) + 32) >> 4; // feet

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
					player1.pos_y = (unsigned long)((y << 4) - 32) << HALF_POS_BIT_COUNT;
					player1.vel_y = 0;
					airtime = 0;
					hit_kill_box = 0;
					break;
				}

				// We want the kill check to be more forgiving.
				y = (high_2byte(player1.pos_y) + 24) >> 4; // feet
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

	// Store the camera position as in a temp variable, as we don't want
	// affect the actual camera during this sequence, as the player rendering
	// will attemp to "offset" that camera.
	index16 = cam.pos_x;

	// TODO: This should be from hitting a trigger, not hard coded like this.
	if (high_2byte(player1.pos_x) > cur_room_width_pixels - 24)
	{
		// TODO: Destination should be data-drive not hard coded to ++.
		++cur_room_index;
		
#define SCROLL_SPEED (4)

		// Load the next room. NOTE: This is loading OVER top of the room in RAM 
		// that will be visible during scroll, but does NOT override VRAM. This is
		// a point of not return though, and the old remove must be scrolled out of
		// view before giving control back to the player.
		banked_call(BANK_2, copy_bg_to_current_room);

		// We know that the camera is right at the edge of the screen, just by the
		// nature of the camera system, and so as a result, we know that we need to
		// scroll 256 pixels to scroll the next room fully into view.
		for (local_i16 = 0; local_i16 < 256; local_i16+=SCROLL_SPEED)
		{
			// desired distance (216) / 64 steps = 3.375
			// Is dependant on SCROLL_SPEED being 4. If that changes,
			// then the number of "steps" should be re-calculated.
			player1.pos_x -= (FP_WHOLE(3) + FP_0_18 + FP_0_18);

			// Draw the player without updating the animation, as it looks
			// weird if they "moon walk" across the screen.
			banked_call(BANK_1, draw_player_static);

			index16 += SCROLL_SPEED;
			// Scroll the camera without affecting "cam" struct.
			scroll(index16,0);

			// Load in a full column of tile data. Don't time slice in this case
			// as perf shouldn't be an issue, and time slicing would furth complicate
			// this sequence.
			in_x_tile = local_i16 / 16;
			in_x_pixel = local_i16;
			vram_buffer_load_column_full();

			// Wait for the frame to be drawn, clear out the sprite data and vram buffer
			// for the next frame, all within this tight loop.
			ppu_wait_nmi();
			oam_clear();
			clear_vram_buffer();
		}

		// Load in 3 extra columns, as the default scrolling logic will miss those.
		for (i = 0; i < 3; ++i)
		{
			in_x_tile = local_i16 / 16;
			in_x_pixel = local_i16;
			vram_buffer_load_column_full();
			banked_call(BANK_1, draw_player_static);
			ppu_wait_nmi();
			oam_clear();
			clear_vram_buffer();

			// NOTE: local_i16 starts with the value it was left with at the end of the 
			//       loop above this intentionally.
			local_i16+=8;
		}	

		// Put the player just outside the door they entered.
		player1.pos_x = FP_WHOLE(16);
		cam.pos_x = 0;
	}

	if (pad_all & PAD_RIGHT)
	{
		player1.facing_left = 0;
	}
	else if (pad_all & PAD_LEFT)
	{
		player1.facing_left = 1;
	}

	if (!grounded)
	{
		if (player1.vel_y > 0)
		{
			anim_index = player1.facing_left ? ANIM_PLAYER_FALL_LEFT : ANIM_PLAYER_FALL_RIGHT;
			global_working_anim = &player1.sprite.anim;
			queue_next_anim(anim_index);
			commit_next_anim();
		}
		else
		{
			anim_index = player1.facing_left ? ANIM_PLAYER_JUMP_LEFT : ANIM_PLAYER_JUMP_RIGHT;
			global_working_anim = &player1.sprite.anim;
			queue_next_anim(anim_index);
			commit_next_anim();
		}
	}
	else if (pad_all & PAD_RIGHT)
	{
		//player1.facing_left = 0;
		anim_index = ANIM_PLAYER_RUN_RIGHT;
		global_working_anim = &player1.sprite.anim;
		queue_next_anim(anim_index);
		commit_next_anim();
	}
	else if (pad_all & PAD_LEFT)
	{
		//player1.facing_left = 1;
		anim_index = ANIM_PLAYER_RUN_LEFT;
		global_working_anim = &player1.sprite.anim;
		queue_next_anim(anim_index);
		commit_next_anim();
	}
	else if (pad_all & PAD_DOWN)
	{
		anim_index = player1.facing_left ? ANIM_PLAYER_IDLE_CROUCH_LEFT : ANIM_PLAYER_IDLE_CROUCH_RIGHT;
		global_working_anim = &player1.sprite.anim;
		queue_next_anim(anim_index);
		commit_next_anim();
	}
	else
	{		
		//anim_index = (ticks_since_attack < ATTACK_LEN) ? ANIM_PLAYER_IDLE_ATTACK_RIGHT : (player1.facing_left ? ANIM_PLAYER_IDLE_LEFT : ANIM_PLAYER_IDLE_RIGHT);
		anim_index = (player1.facing_left ? ANIM_PLAYER_IDLE_LEFT : ANIM_PLAYER_IDLE_RIGHT);
		global_working_anim = &player1.sprite.anim;
		queue_next_anim(anim_index);
		commit_next_anim();
	}

	//draw_player();

	PROFILE_POKE(PROF_R);
}

void copy_current_map_to_nametable()
{
	static unsigned int start_x;
	static unsigned int nt;

	//shake_remaining = 0;

	//banked_call(BANK_5, copy_and_process_map_data);

	// Figure out where the start of the nametable the left
	// side of the camera is in. This is where we will start
	// loading in tile data, and will determine which nametable
	// to load data into.
	start_x = ((cam.pos_x / 256) * 256) / 16; // start of Nametable A

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
			index16 = GRID_XY_TO_ROOM_INDEX(start_x + x, y);
			index16 = current_room[index16] * META_TILE_NUM_BYTES;
			vram_adr(NTADR(nt,x*2,y*2));	
			vram_write(&metatiles_temp[index16], 2);
			vram_adr(NTADR(nt,x*2,(y*2)+1));	
			vram_write(&metatiles_temp[index16+2], 2);
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
			index16 = GRID_XY_TO_ROOM_INDEX(start_x + x + 16, y);
			index16 = current_room[index16] * META_TILE_NUM_BYTES;
			vram_adr(NTADR(nt,x*2,y*2));	
			vram_write(&metatiles_temp[index16], 2);
			vram_adr(NTADR(nt,x*2,(y*2)+1));	
			vram_write(&metatiles_temp[index16+2], 2);
		}
	}

	// Go back to the start of the first nametable.
	nt-=0x400;
	index = 0;
	for (y = 0; y < 15; y+=2)
	{
		for (x = 0; x < 16; x+=2)
		{
			i = 0;

			// room index.
			index16 = (y * cur_room_width_tiles) + (start_x + x);
			// meta tile palette index.
			index16 = (current_room[index16] * META_TILE_NUM_BYTES) + 4;
			// bit shift amount
			i |= (metatiles_temp[index16]);

			index16 = (y * cur_room_width_tiles) + (start_x + x + 1);
			index16 = (current_room[index16] * META_TILE_NUM_BYTES) + 4;
			i |= (metatiles_temp[index16]) << 2;

			index16 = ((y + 1) * cur_room_width_tiles) + (start_x + x);
			index16 = (current_room[index16] * META_TILE_NUM_BYTES) + 4;
			i |= (metatiles_temp[index16]) << 4;

			index16 = ((y + 1) * cur_room_width_tiles) + (start_x + x + 1);
			index16 = (current_room[index16] * META_TILE_NUM_BYTES) + 4;
			i |= (metatiles_temp[index16]) << 6;	

			vram_adr(nt + 960 + index);	
			vram_write(&i, 1);
			++index;
		}
	}

	nt+=0x400;
	index = 0;
	for (y = 0; y < 15; y+=2)
	{
		for (x = 0; x < 16; x+=2)
		{
			i = 0;

			// room index.
			index16 = (y * cur_room_width_tiles) + (start_x + x + 16);
			// meta tile palette index.
			index16 = (current_room[index16] * META_TILE_NUM_BYTES) + 4;
			// bit shift amount
			i |= (metatiles_temp[index16]);

			index16 = (y * cur_room_width_tiles) + (start_x + x + 1 + 16);
			index16 = (current_room[index16] * META_TILE_NUM_BYTES) + 4;
			i |= (metatiles_temp[index16]) << 2;

			index16 = ((y + 1) * cur_room_width_tiles) + (start_x + x + 16);
			index16 = (current_room[index16] * META_TILE_NUM_BYTES) + 4;
			i |= (metatiles_temp[index16]) << 4;

			index16 = ((y + 1) * cur_room_width_tiles) + (start_x + x + 1 + 16);
			index16 = (current_room[index16] * META_TILE_NUM_BYTES) + 4;
			i |= (metatiles_temp[index16]) << 6;	

			vram_adr(nt + 960 + index);	
			vram_write(&i, 1);
			++index;
		}
	}
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
	multi_vram_buffer_horz(&metatiles_temp[local_att_index16], 2, get_ppu_addr(nametable_index, in_x_tile * CELL_SIZE, in_y_tile * CELL_SIZE));
	multi_vram_buffer_horz(&metatiles_temp[local_att_index16+2], 2, get_ppu_addr(nametable_index, in_x_tile * CELL_SIZE, (in_y_tile * CELL_SIZE) + 8));

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
	local_i |= (metatiles_temp[local_att_index16]);

	local_index16 = local_index16 + 1; //(local_y * 16) + (local_x + 1);
	local_att_index16 = (current_room[local_index16] * META_TILE_NUM_BYTES) + 4;
	local_i |= (metatiles_temp[local_att_index16]) << 2;

	local_index16 = local_index16 + 15; //((local_y + 1) * 16) + (local_x);
	local_att_index16 = (current_room[local_index16] * META_TILE_NUM_BYTES) + 4;
	local_i |= (metatiles_temp[local_att_index16]) << 4;

	local_index16 = local_index16 + 1; //((local_y + 1) * 16) + (local_x + 1);
	local_att_index16 = (current_room[local_index16] * META_TILE_NUM_BYTES) + 4;
	local_i |= (metatiles_temp[local_att_index16]) << 6;	

	one_vram_buffer(local_i, get_at_addr(nametable_index, (local_x) * CELL_SIZE, (local_y) * CELL_SIZE));
}
*/
void vram_buffer_load_column()
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

	nametable_index = (in_x_tile / 16) % 2;

	// TILES

PROFILE_POKE(PROF_G)

	tile_offset = (in_x_pixel % 16) / 8;
	tile_offset2 = tile_offset+2;

	local_index16 = GRID_XY_TO_ROOM_INDEX(in_x_tile, cur_nametable_y/2);

	for (local_i = 0; local_i < NAMETABLE_TILES_8_UPDATED_PER_FRAME; )
	{
		local_att_index16 = current_room[local_index16] * META_TILE_NUM_BYTES;

		// single column of tiles
		array_temp8 = metatiles_temp[local_att_index16 + tile_offset];
		nametable_col[local_i] = array_temp8;
		local_i++;
		array_temp8 = metatiles_temp[local_att_index16 + tile_offset2];
		nametable_col[local_i] = array_temp8;
		local_i++;

		local_index16 += cur_room_width_tiles;
	}

	// The screen is only 30 tiles high, which doesn't always divide nicely into out metatile updates, so map sure we don't
	// update beyond the bottom of the screen.
	array_temp8 = MIN(NAMETABLE_TILES_8_UPDATED_PER_FRAME, 30 - cur_nametable_y);

	multi_vram_buffer_vert(nametable_col, (unsigned char)array_temp8, get_ppu_addr(nametable_index, in_x_pixel, cur_nametable_y * 8));


	// ATTRIBUTES

PROFILE_POKE(PROF_B)

	// Attributes are in 2x2 meta tile chunks, so we need to round down to the nearest,
	// multiple of 2 (eg. if you pass in index 5, we want to start on 4).
	local_x = (in_x_tile & 0xFFFE);//local_x = (in_x_tile / 2) * 2;
	//local_x = (in_x_tile / 2) * 2;

	for (local_y = 0; local_y < (NAMETABLE_ATTRIBUTES_16_UPDATED_PER_FRAME); local_y+=2)
	{
		local_i = 0;

		// room index.
		local_index16 = GRID_XY_TO_ROOM_INDEX(local_x, local_y + (cur_nametable_y / 2));
		// meta tile palette index.
		local_att_index16 = (current_room[local_index16] * META_TILE_NUM_BYTES);
		local_att_index16+=4;
		// bit shift amount
		local_i |= (metatiles_temp[local_att_index16]);

		local_index16++;//local_index16 = local_index16 + 1; //(local_y * 16) + (local_x + 1);
		local_att_index16 = (current_room[local_index16] * META_TILE_NUM_BYTES);
		local_att_index16+=4;
		local_i |= (metatiles_temp[local_att_index16]) << 2;

		local_index16 = local_index16 + cur_room_width_tiles; //((local_y + 1) * 16) + (local_x);
		local_index16--;
		local_att_index16 = (current_room[local_index16] * META_TILE_NUM_BYTES);
		local_att_index16+=4;
		local_i |= (metatiles_temp[local_att_index16]) << 4;

		local_index16++;// = local_index16 + 1; //((local_y + 1) * 16) + (local_x + 1);
		local_att_index16 = (current_room[local_index16] * META_TILE_NUM_BYTES);
		local_att_index16+=4;
		local_i |= (metatiles_temp[local_att_index16]) << 6;	

		one_vram_buffer(local_i, get_at_addr(nametable_index, (local_x) * CELL_SIZE, ((local_y * CELL_SIZE) + (cur_nametable_y * 8))));
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

	nametable_index = (in_x_tile / 16) % 2;

	// TILES

PROFILE_POKE(PROF_G)

	tile_offset = (in_x_pixel % 16) / 8;
	tile_offset2 = tile_offset+2;

	local_index16 = GRID_XY_TO_ROOM_INDEX(in_x_tile, 0);

	for (local_i = 0; local_i < 30; )
	{
		local_att_index16 = current_room[local_index16] * META_TILE_NUM_BYTES;

		// single column of tiles
		array_temp8 = metatiles_temp[local_att_index16 + tile_offset];
		nametable_col[local_i] = array_temp8;
		local_i++;
		array_temp8 = metatiles_temp[local_att_index16 + tile_offset2];
		nametable_col[local_i] = array_temp8;
		local_i++;

		local_index16 += cur_room_width_tiles;
	}

	multi_vram_buffer_vert(nametable_col, (unsigned char)30, get_ppu_addr(nametable_index, in_x_pixel, 0));


	// ATTRIBUTES

PROFILE_POKE(PROF_B)

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
		local_i |= (metatiles_temp[local_att_index16]);

		local_index16++;//local_index16 = local_index16 + 1; //(local_y * 16) + (local_x + 1);
		local_att_index16 = (current_room[local_index16] * META_TILE_NUM_BYTES);
		local_att_index16+=4;
		local_i |= (metatiles_temp[local_att_index16]) << 2;

		local_index16 = local_index16 + cur_room_width_tiles; //((local_y + 1) * 16) + (local_x);
		local_index16--;
		local_att_index16 = (current_room[local_index16] * META_TILE_NUM_BYTES);
		local_att_index16+=4;
		local_i |= (metatiles_temp[local_att_index16]) << 4;

		local_index16++;// = local_index16 + 1; //((local_y + 1) * 16) + (local_x + 1);
		local_att_index16 = (current_room[local_index16] * META_TILE_NUM_BYTES);
		local_att_index16+=4;
		local_i |= (metatiles_temp[local_att_index16]) << 6;	

		one_vram_buffer(local_i, get_at_addr(nametable_index, (local_x) * CELL_SIZE, ((local_y * CELL_SIZE))));
	}
PROFILE_POKE(PROF_W)
}

void go_to_state(unsigned char new_state)
{
	cur_state = new_state;

	switch(cur_state)
	{
		case STATE_TITLE:
		{
			fade_to_black();
			ppu_off();
			scroll(0,0);		
//			set_chr_bank_0(2);	

/*
;mode 2 changes $0000-$03FF
;mode 3 changes $0400-$07FF
;mode 4 changes $0800-$0BFF
;mode 5 changes $0C00-$0FFF
*/	
			set_chr_mode_2(16);
			set_chr_mode_3(17);
			set_chr_mode_4(18);
			set_chr_mode_5(19);

			cur_room_index = 0;

			pal_bg(palette_title);
			pal_spr(palette_title);		
			vram_adr(NTADR_A(0,0));
			vram_unrle(screen_title);
			ppu_on_all();
//			music_play(0);
			fade_from_black();
			music_play(0);
			break;
		}

		case STATE_GAME:
		{
			fade_to_black();
			ppu_off();
			music_stop();
			scroll(0,0);
			cam.pos_x = 0;
//			set_chr_bank_0(0);

			set_chr_mode_2(0);
			set_chr_mode_3(1);
			set_chr_mode_4(2);
			set_chr_mode_5(3);

			pal_bg(palette);
			pal_spr(palette);
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
			player1.pos_x = FP_WHOLE(0);
//#endif // DEBUG_ENABLED
			player1.pos_y = FP_WHOLE((6<<4));
			player1.vel_y = 0;
			player1.facing_left = 0;

			// Load the room first so that we know it's size.
			banked_call(BANK_2, copy_bg_to_current_room);

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

			// Copy the map data to nametables based on the player position.
			copy_current_map_to_nametable();

			ppu_on_all();
	
			// pre-adjust scroll, and wait for that to get 
			// pushed to the ppu.
			scroll(cam.pos_x,0);	
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
			vram_adr(NTADR_A(0,0));
			vram_unrle(screen_gameover);
			// we draw the player sprites at the start of the white fade,
			// so now we need to clear it before fading back in or else you
			// will see her for a few frames.
			oam_clear();
			ppu_on_all();
			fade_from_white();
		}
	}
}