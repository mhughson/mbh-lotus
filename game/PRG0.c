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

const unsigned char palette[16]={ 0x0f,0x05,0x23,0x37,0x0f,0x01,0x21,0x31,0x0f,0x06,0x16,0x26,0x0f,0x09,0x19,0x29 };

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
const anim_def jump_right = { 60, 2, { 3, 4 } };

const struct anim_def* sprite_anims[] =
{
	&idle_right,
	&walk_right,

	&jump_right,
};

const unsigned char level_data[ROOM_WIDTH_TILES * 15] = 
{
	5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5,    5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5,    5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5,   5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 2, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,    1, 2, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,    3, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,   3, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	5, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 5,    5, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 5,    5, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 5,   5, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 5,
};



// Functions
//

void main_real()
{
	unsigned int old_cam_x;

    ppu_off(); // screen off

	set_vram_buffer(); // do at least once, sets a pointer to a buffer
	clear_vram_buffer();

	set_chr_bank_0(0);
	bank_bg(0);
	bank_spr(1);

	pal_bg(palette);
	pal_spr(palette);

	load_current_map(NAMETABLE_A, NULL);

	ppu_on_all(); // turn on screen

	pal_bright(4);

	// Horizontal scrolling...
	set_mirror_mode(MIRROR_MODE_VERT);

	player.pos_x = 128;
	player.pos_y = 128;

	// infinite loop
	while (1)
	{
		++tick_count;

		ppu_wait_nmi(); // wait till beginning of the frame

		oam_clear();

		pad_all = pad_poll(0) | pad_poll(1); // read the first controller
		pad_all_new = get_pad_new(0) | get_pad_new(1); // newly pressed button. do pad_poll first

		clear_vram_buffer(); // do at the beginning of each frame

		// store the camera position at the start of the framem, so that
		// we can detect if it moved by the end of the frame.
		old_cam_x = cam.pos_x;

		if(pad_all & PAD_RIGHT && (player.pos_x <= ROOM_WIDTH_PIXELS-2))
		{
			player.pos_x += 2;
		}
		else if (pad_all & PAD_LEFT)
		{
			if (player.pos_x >= ((cam.pos_x / 256) * 256) + 2)
			{
				player.pos_x -= 2;
			}
		}

		global_working_anim = &player.anim;

		if (pad_all & PAD_A)
		{
			//oam_meta_spr(128, 127, meta_player_list[3]);
			queue_next_anim(2);
		}		
		// else if (pad_all & PAD_B)
		// {
		// 	oam_meta_spr(128, 127, meta_player_list[4]);
		// }
		else if (pad_all & PAD_RIGHT)
		{
			//oam_meta_spr(128, 127, meta_player_list[((tick_count >> 3) % 6) + 5]);
			queue_next_anim(1);
		}
		else
		{
			//oam_meta_spr(128, 127, meta_player_list[(tick_count >> 3) % 3]);
			queue_next_anim(0);
		}

        // move the camera to the player if needed.
        if (player.pos_x > cam.pos_x + 128 && cam.pos_x < ROOM_WIDTH_PIXELS-256)
        {
            cam.pos_x = player.pos_x - 128;
        }
        else if (player.pos_x < cam.pos_x + 64 && cam.pos_x % 256 != 0)
        {
            cam.pos_x = player.pos_x - 64;
        }

		// This should really be >> 4 (every 16 pixels) but that will miss the initial
		// row loading. Could update "load_map" but the nametable logic is kind of annoying
		// for the non-vram version. Will dig in more later.
		if ((old_cam_x >> 3) < (cam.pos_x >> 3))
		{
			in_x_tile = (cam.pos_x + 256) / 16;
			vram_buffer_load_column();
		}

		global_working_anim = &player.anim;
		commit_next_anim();

		global_working_anim = &player.anim;
		update_anim();

		oam_meta_spr(
			player.pos_x - cam.pos_x, 
			player.pos_y - 1 - cam.pos_y,
			meta_player_list[sprite_anims[player.anim.anim_current]->frames[player.anim.anim_frame]]);

		// cur_col is the last column to be loaded, aka the right
		// hand side of the screen. The scroll amount is relative to the 
		// left hand side of the screen, so offset by 256.
		set_scroll_x(cam.pos_x);
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

void load_current_map(unsigned int nt, unsigned char* _current_room)
{
	// "const_cast"
	_current_room = (unsigned char*)(level_data);
	
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

    const unsigned char* current_room = level_data;

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

    const unsigned char* current_room = level_data;

    nametable_index = (in_x_tile / 16) % 2;

	// TILES

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
}
