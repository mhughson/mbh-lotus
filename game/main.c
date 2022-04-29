/** (C) Matt Hughson 2020 */

#include "LIB/neslib.h"
#include "LIB/nesdoug.h"
#include "A53/bank_helpers.h"
#include "../include/stdlib.h"

#include "main.h"
#include "PRG0.h"

#pragma rodata-name ("CODE")
#pragma code-name ("CODE")

#include "NES_ST/meta_player.h"
#include "NES_ST/screen_temp.h"

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

// Initalized RAM variables
//

unsigned char tick_count;
unsigned char pad_all;
unsigned char pad_all_new;

anim_info* global_working_anim;
anim_info player_anim;

#pragma rodata-name ("CODE")
#pragma code-name ("CODE")


// It seems like main() MUST be in fixed back!
void main (void) 
{
	ppu_off(); // screen off

	set_vram_buffer(); // do at least once, sets a pointer to a buffer
	clear_vram_buffer();

	set_chr_bank_0(0);
	bank_bg(0);
	bank_spr(1);

	pal_bg(palette);
	pal_spr(palette);

	vram_unrle(screen_temp);

	ppu_on_all(); // turn on screen

	pal_bright(4);

	// infinite loop
	while (1)
	{
		++tick_count;

		ppu_wait_nmi(); // wait till beginning of the frame

		oam_clear();

		pad_all = pad_poll(0) | pad_poll(1); // read the first controller
		pad_all_new = get_pad_new(0) | get_pad_new(1); // newly pressed button. do pad_poll first

		clear_vram_buffer(); // do at the beginning of each frame


		global_working_anim = &player_anim;
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


		global_working_anim = &player_anim;
		commit_next_anim();

		global_working_anim = &player_anim;
		update_anim();

		oam_meta_spr(
			128, 
			128 - 1,
			meta_player_list[sprite_anims[player_anim.anim_current]->frames[player_anim.anim_frame]]);

	}
}

void queue_next_anim(unsigned char next_anim_index)
{
	global_working_anim->anim_queued = next_anim_index;
}

void commit_next_anim()
{
	if (global_working_anim->anim_queued != 0xff && global_working_anim->anim_queued != global_working_anim->anim_current)
	{
		global_working_anim->anim_current = global_working_anim->anim_queued;
		global_working_anim->anim_frame = 0;
		global_working_anim->anim_ticks = 0;
	}

	global_working_anim->anim_queued = 0xff;
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