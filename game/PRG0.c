#include "PRG0.h"
#include "main.h"
#include "LIB/neslib.h"
#include "LIB/nesdoug.h"
#include "A53/bank_helpers.h"
#include "../include/stdlib.h"

#pragma rodata-name ("BANK0")
#pragma code-name ("BANK0")

// Const data
//

#include "NES_ST/meta_player.h"
#include "meta_tiles_temp.h"
#include "NES_ST/screen_title.h"

const unsigned char palette[16]={ 0x0f,0x05,0x23,0x37,0x0f,0x01,0x21,0x31,0x0f,0x06,0x16,0x26,0x0f,0x09,0x19,0x29 };
const unsigned char palette_title[16]={ 0x0f,0x15,0x25,0x30,0x0f,0x13,0x25,0x30,0x0f,0x06,0x16,0x26,0x0f,0x09,0x19,0x29 };



#define NUM_Y_COLLISION_OFFSETS 3
const unsigned char y_collision_offsets[NUM_Y_COLLISION_OFFSETS] = { 1, 12, 23 };
#define NUM_X_COLLISION_OFFSETS 3
const unsigned char x_collision_offsets[NUM_X_COLLISION_OFFSETS] = { 2, 8, 14 };

typedef struct anim_def
{
	// how many frames to hold on each frame of animation.
	unsigned char ticks_per_frame;

	// how many frames are used in frames array.
	// TODO: would it be better to just use a special value (eg. 0xff)
	//		 to signify the end of the anim?
	unsigned char anim_len;

	// index into meta_sprites array
	unsigned char frames[17];
} anim_def;

const anim_def idle_right = { 5, 3, { 0, 1, 2 } };
const anim_def walk_right = { 10, 5, { 5, 6, 7, 8, 9 } };
const anim_def jump_right = { 60, 1, { 3 } };
const anim_def fall_right = { 60, 1, { 4 } };
const anim_def idle_left = { 5, 3, { 11, 12, 13 } };
const anim_def walk_left = { 10, 5, { 16, 17, 18, 19, 20 } };
const anim_def jump_left = { 60, 1, { 14 } };
const anim_def fall_left = { 60, 1, { 15 } };


enum
{
	ANIM_PLAYER_IDLE_RIGHT = 0,
	ANIM_PLAYER_IDLE_LEFT = 1, 
	ANIM_PLAYER_RUN_RIGHT = 2,
	ANIM_PLAYER_RUN_LEFT = 3, 
	ANIM_PLAYER_JUMP_RIGHT = 4,
	ANIM_PLAYER_JUMP_LEFT = 5, 
	ANIM_PLAYER_FALL_RIGHT = 6,
	ANIM_PLAYER_FALL_LEFT = 7, 

	NUM_ANIMS,
};

const struct anim_def* sprite_anims[] =
{
	&idle_right,
	&idle_left,

	&walk_right,
	&walk_left,

	&jump_right,
	&jump_left,

	&fall_right,
	&fall_left,
};

const unsigned char current_room[ROOM_WIDTH_TILES * 15] = 
{
	5, 1, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 5,    5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5,    5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0,    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0,    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,    1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,    0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   3, 2, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1,    1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,    3, 2, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1,   3, 2, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0,    0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,    1, 1, 1, 1, 3, 3, 3, 3, 3, 3, 3, 3, 1, 1, 1, 1,   1, 1, 1, 1, 0, 0, 0, 0, 2, 2, 0, 0, 0, 0, 0, 0,
	1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    0, 0, 3, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1,    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1,    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1,    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 0, 0,
	5, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 5,    5, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 5,    5, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 5,   5, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};


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


// Functions
//

void update_player();
void draw_player();

void main_real()
{
	unsigned int old_cam_x;

    ppu_off(); // screen off

	set_vram_buffer(); // do at least once, sets a pointer to a buffer
	clear_vram_buffer();

	bank_bg(0);
	bank_spr(1);

	//ppu_on_all(); // turn on screen

	//pal_bright(4);

	// Horizontal scrolling...
	set_mirror_mode(MIRROR_MODE_VERT);

	player1.pos_x = FP_WHOLE(128);
	player1.pos_y = FP_WHOLE(128);

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
				// store the camera position at the start of the framem, so that
				// we can detect if it moved by the end of the frame.
				old_cam_x = cam.pos_x;

				update_player();

				// move the camera to the player if needed.
				if (high_2byte(player1.pos_x) > cam.pos_x + 128 && cam.pos_x < ROOM_WIDTH_PIXELS-256)
				{
					cam.pos_x = high_2byte(player1.pos_x) - 128;
				}
				else if (high_2byte(player1.pos_x) < cam.pos_x + 64 && (cam.pos_x / 256) == ((high_2byte(player1.pos_x) - 64) / 256))
				{
					cam.pos_x = high_2byte(player1.pos_x) - 64;
				}

				// This should really be >> 4 (every 16 pixels) but that will miss the initial
				// row loading. Could update "load_map" but the nametable logic is kind of annoying
				// for the non-vram version. Will dig in more later.
				if ((old_cam_x >> 3) < (cam.pos_x >> 3))
				{
					in_x_tile = (cam.pos_x + 256) / 16;
					vram_buffer_load_column();
				}

				draw_player();

				// cur_col is the last column to be loaded, aka the right
				// hand side of the screen. The scroll amount is relative to the 
				// left hand side of the screen, so offset by 256.
				set_scroll_x(cam.pos_x);				
				break;
			}
		}


		PROFILE_POKE(PROF_CLEAR)
	}
}

unsigned char update_anim()
{
	static const struct anim_def* cur_anim;
	cur_anim = sprite_anims[global_working_anim->anim_current];

	// Note: In WnW this was done in each draw function manually, and I'm not sure why. Perhaps
	//		 to ensure that the first frame got played before advancing?
	++global_working_anim->anim_ticks;

	if (global_working_anim->anim_ticks >= cur_anim->ticks_per_frame)
	{
		global_working_anim->anim_ticks = 0;
		++global_working_anim->anim_frame;
		// todo: don't always loop.
		if (global_working_anim->anim_frame >= cur_anim->anim_len)
		{
			global_working_anim->anim_frame = 0;
			return 1;
		}
	}
	return 0;
}

void draw_player()
{
	global_working_anim = &player1.sprite.anim;
	update_anim();

	//cur_cam_x = high_2byte(player1.pos_x) - 128;

	oam_meta_spr(
		high_2byte(player1.pos_x) - cam.pos_x, 
		high_2byte(player1.pos_y) - 1 - cam.pos_y,
		meta_player_list[sprite_anims[player1.sprite.anim.anim_current]->frames[player1.sprite.anim.anim_frame]]);


	// Update animation.
	//++player1.sprite.anim.anim_ticks;

}

void update_player()
{
	if (pad_all & PAD_LEFT && (cam.pos_x / 256) <= (( high_2byte((player1.pos_x)) - (WALK_SPEED >> 16)) / 256) && player1.pos_x >= WALK_SPEED)
	{
		// move the player left.
		player1.pos_x -= (unsigned long)WALK_SPEED;// + FP_0_5;

		for (i = 0; i < NUM_Y_COLLISION_OFFSETS; ++i)
		{
			// take the player position, offset it a little
			// so that the arms overlap the wall a bit (+8),
			// convert to metatile space (16x16) (>>4).
			// The position is stored using fixed point math, 
			// where the high byte is the whole part, and the
			// low byte is the fraction.
			x = (high_2byte(player1.pos_x)) >> 4;

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

				// Check if that point is in a solid metatile
				if (GET_META_TILE_FLAGS(index16) & FLAG_SOLID)
				{
					// Hit a wall, shift back to the edge of the wall.
					player1.pos_x = (unsigned long)((x << 4) + 16) << HALF_POS_BIT_COUNT;

					break;
				}
			}
		}
	}
	// Is the right side of the sprite, after walking, going to be passed the end of the map?
	if (pad_all & PAD_RIGHT && (player1.pos_x + WALK_SPEED + FP_WHOLE(16) ) <= FP_WHOLE(ROOM_WIDTH_PIXELS))
	{

		temp32 = player1.pos_x;
		player1.pos_x += WALK_SPEED;// + FP_0_5;

		for (i = 0; i < NUM_Y_COLLISION_OFFSETS; ++i)
		{
			// take the player position, offset it a little
			// so that the arms overlap the wall a bit (+16),
			// convert to metatile space (16x16) (>>4).
			// The position is stored using fixed point math, 
			// where the high byte is the whole part, and the
			// low byte is the fraction.
			x = (high_2byte(player1.pos_x) + 16) >> 4;

			// Player is 24 pixels high, so +12 checks middle of them.
			// >>4 to put the position into metatile-space (16x16 chunks).
			y = (high_2byte(player1.pos_y) + y_collision_offsets[i]) >> 4;

			if (y < 15)
			{
				// Convert the x,y into an index into the room data array.
				//temp16 = GRID_XY_TO_ROOM_NUMBER(x, y);
				index16 = GRID_XY_TO_ROOM_INDEX(x, y);

				// Check if that point is in a solid metatile
				if (GET_META_TILE_FLAGS(index16) & FLAG_SOLID)
				{
					// Hit a wall, shift back to the edge of the wall.
					player1.pos_x = temp32; //(unsigned long)((x << 4) - 17) << HALF_POS_BIT_COUNT;

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
		for (i = 0; i < NUM_X_COLLISION_OFFSETS; ++i)
		{
			x = (high_2byte(player1.pos_x) + x_collision_offsets[i]) >> 4;
			y = (high_2byte(player1.pos_y) + 32) >> 4; // feet
			if (y < 15)
			{
				index16 = GRID_XY_TO_ROOM_INDEX(x, y);
				if (GET_META_TILE_FLAGS(index16) & FLAG_SOLID)
				{
					jump_count = 0;
					grounded = 1;
					player1.pos_y = (unsigned long)((y << 4) - 32) << HALF_POS_BIT_COUNT;
					player1.vel_y = 0;
					airtime = 0;
					break;
				}
			}
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
	else
	{					
		anim_index = player1.facing_left ? ANIM_PLAYER_IDLE_LEFT : ANIM_PLAYER_IDLE_RIGHT;
		global_working_anim = &player1.sprite.anim;
		queue_next_anim(anim_index);
		commit_next_anim();
	}

	//draw_player();
}

void load_current_map(unsigned int nt, unsigned char* _current_room)
{
	// "const_cast"
	_current_room = (unsigned char*)(current_room);
	
	//shake_remaining = 0;

	//banked_call(BANK_5, copy_and_process_map_data);

	for (y = 0; y < 15; ++y)
	{
		for (x = 0; x < 16; ++x)
		{
			index16 = GRID_XY_TO_ROOM_INDEX(x, y);
			index16 = _current_room[index16] * META_TILE_NUM_BYTES;
			vram_adr(NTADR(nt,x*2,y*2));	
			vram_write(&metatiles_temp[index16], 2);
			vram_adr(NTADR(nt,x*2,(y*2)+1));	
			vram_write(&metatiles_temp[index16+2], 2);
		}
	}

	index = 0;
	for (y = 0; y < 15; y+=2)
	{
		for (x = 0; x < 16; x+=2)
		{
			i = 0;

			// room index.
			index16 = (y * ROOM_WIDTH_TILES) + (x);
			// meta tile palette index.
			index16 = (_current_room[index16] * META_TILE_NUM_BYTES) + 4;
			// bit shift amount
			i |= (metatiles_temp[index16]);

			index16 = (y * ROOM_WIDTH_TILES) + (x + 1);
			index16 = (_current_room[index16] * META_TILE_NUM_BYTES) + 4;
			i |= (metatiles_temp[index16]) << 2;

			index16 = ((y + 1) * ROOM_WIDTH_TILES) + (x);
			index16 = (_current_room[index16] * META_TILE_NUM_BYTES) + 4;
			i |= (metatiles_temp[index16]) << 4;

			index16 = ((y + 1) * ROOM_WIDTH_TILES) + (x + 1);
			index16 = (_current_room[index16] * META_TILE_NUM_BYTES) + 4;
			i |= (metatiles_temp[index16]) << 6;	

			vram_adr(nt + 960 + index);	
			vram_write(&i, 1);
			++index;
		}
	}
}

// LARGELY UNTESTED!
// ATTRIBUTES LOOKS LIKE THEY WOULD NOT WORK. ASSUMES 16 TILE WIDE LEVELS
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

void vram_buffer_load_column()
{
	static unsigned char local_x;
	static unsigned char local_y;
	static unsigned char local_i;
	static unsigned int local_index16;
	static unsigned int local_att_index16;
    static unsigned char nametable_index;

    nametable_index = (in_x_tile / 16) % 2;

	// TILES

	PROFILE_POKE(PROF_G)

    // left column
    for (local_i = 0; local_i < 30; local_i+=2)
    {

        local_index16 = GRID_XY_TO_ROOM_INDEX(in_x_tile, (local_i / 2));
        local_att_index16 = current_room[local_index16] * META_TILE_NUM_BYTES;
        nametable_col[local_i] = metatiles_temp[local_att_index16];
        nametable_col[local_i + 1] = metatiles_temp[local_att_index16 + 2];
    }

    multi_vram_buffer_vert(nametable_col, 30, get_ppu_addr(nametable_index, in_x_tile * CELL_SIZE, 0));

    // right column
    for (local_i = 0; local_i < 30; local_i+=2)
    {

        local_index16 = GRID_XY_TO_ROOM_INDEX(in_x_tile, (local_i / 2));
        local_att_index16 = current_room[local_index16] * META_TILE_NUM_BYTES;
        nametable_col[local_i] = metatiles_temp[local_att_index16 + 1];
        nametable_col[local_i + 1] = metatiles_temp[local_att_index16 + 3];
    }

    multi_vram_buffer_vert(nametable_col, 30, get_ppu_addr(nametable_index, (in_x_tile * CELL_SIZE) + 8, 0));


	// ATTRIBUTES

	PROFILE_POKE(PROF_B)

	// Attributes are in 2x2 meta tile chunks, so we need to round down to the nearest,
	// multiple of 2 (eg. if you pass in index 5, we want to start on 4).
	local_x = (in_x_tile / 2) * 2;

    // TODO: This could potentially be batched into a single 
    //       vertical vram write.
    for (local_y = 0; local_y < 15; local_y+=2)
    {
        local_i = 0;

        // room index.
        local_index16 = GRID_XY_TO_ROOM_INDEX(local_x, local_y);
        // meta tile palette index.
        local_att_index16 = (current_room[local_index16] * META_TILE_NUM_BYTES) + 4;
        // bit shift amount
        local_i |= (metatiles_temp[local_att_index16]);

        local_index16 = local_index16 + 1; //(local_y * 16) + (local_x + 1);
        local_att_index16 = (current_room[local_index16] * META_TILE_NUM_BYTES) + 4;
        local_i |= (metatiles_temp[local_att_index16]) << 2;

        local_index16 = local_index16 + ROOM_WIDTH_TILES - 1; //((local_y + 1) * 16) + (local_x);
        local_att_index16 = (current_room[local_index16] * META_TILE_NUM_BYTES) + 4;
        local_i |= (metatiles_temp[local_att_index16]) << 4;

        local_index16 = local_index16 + 1; //((local_y + 1) * 16) + (local_x + 1);
        local_att_index16 = (current_room[local_index16] * META_TILE_NUM_BYTES) + 4;
        local_i |= (metatiles_temp[local_att_index16]) << 6;	

        one_vram_buffer(local_i, get_at_addr(nametable_index, (local_x) * CELL_SIZE, (local_y) * CELL_SIZE));
    }

	PROFILE_POKE(PROF_R)
}

void go_to_state(unsigned char new_state)
{
	cur_state = new_state;

	switch(cur_state)
	{
		case STATE_TITLE:
		{
			ppu_off();
			set_chr_bank_0(2);	
			pal_bg(palette_title);
			pal_spr(palette_title);		
			vram_adr(NTADR_A(0,0));
			vram_unrle(screen_title);
			ppu_on_all();
			fade_from_black();
			break;
		}

		case STATE_GAME:
		{
			fade_to_black();
			ppu_off();
			set_chr_bank_0(0);
			pal_bg(palette);
			pal_spr(palette);		
			load_current_map(NAMETABLE_A, NULL);

			ppu_on_all();
			fade_from_black();
			break;
		}
	}
}