#include "PRG1.h"
#include "main.h"
#include "LIB/neslib.h"
#include "LIB/nesdoug.h"
#include "mmc3\mmc3_code.h"
#include "../include/stdlib.h"

#pragma rodata-name ("BANK1")
#pragma code-name ("BANK1")

#include "NES_ST/meta_player.h"
#include "NES_ST/meta_player_td.h"

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

	// The CHR ROM to use for this animation.
	unsigned char chr_index;
} anim_def;

// Helpers to make things easier to move around.
#define CHR_SIDE 4
#define CHR_SIDE_DASH 6
#define CHR_TD 13

const anim_def idle_right = { 20, 4, { 0, 1, 2, 3 }, CHR_SIDE };
const anim_def walk_right = { 7, 4, { 4, 5, 6, 5 }, CHR_SIDE };
const anim_def jump_right = { 255, 1, { 7 }, CHR_SIDE };
const anim_def fall_right = { 255, 1, { 8 }, CHR_SIDE };
const anim_def dash_right = { 5, 2, { 9, 10 }, CHR_SIDE_DASH };

const anim_def idle_down_td 	= { 60, 2, { 2, 3 }, CHR_TD };
const anim_def idle_right_td 	= { 60, 2, { 7, 8 }, CHR_TD };
const anim_def idle_up_td 		= { 60, 2, { 12, 13 }, CHR_TD };
const anim_def idle_left_td 	= { 60, 2, { 17, 18 }, CHR_TD };

const anim_def walk_down_td 	= { 20, 2, { 0, 1 }, CHR_TD };
const anim_def walk_right_td 	= { 20, 2, { 5, 6 }, CHR_TD };
const anim_def walk_up_td 		= { 20, 2, { 10, 11 }, CHR_TD };
const anim_def walk_left_td 	= { 20, 2, { 15, 16 }, CHR_TD };
const struct anim_def* sprite_anims[] =
{
	&idle_right,

	&walk_right,

	&jump_right,

	&fall_right,

	&dash_right,

	&idle_down_td ,
	&idle_right_td,
	&idle_up_td ,
	&idle_left_td ,
	&walk_down_td ,
	&walk_right_td,
	&walk_up_td ,
	&walk_left_td ,
};

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

	draw_player_static();
}

void draw_player_static()
{
	// At the start of the next frame, when this oam data will be display, swap to the correct
	// sprite VRAM data.
	chr_index_queued = sprite_anims[player1.sprite.anim.anim_current]->chr_index;

	if (high_2byte(player1.pos_y) < 240 || high_2byte(player1.pos_y) > (0xffff - 16))
	{
		if (player1.dir_x < 0)
		{
			in_oam_x = high_2byte(player1.pos_x) - cam.pos_x;
			in_oam_y = high_2byte(player1.pos_y) - 1 - cam.pos_y;
			in_oam_data = meta_player_list[sprite_anims[player1.sprite.anim.anim_current]->frames[player1.sprite.anim.anim_frame]];
			c_oam_meta_spr_flipped();
		}
		else // NOTE: This assumes left/right only.
		{		
			oam_meta_spr(
				high_2byte(player1.pos_x) - cam.pos_x, 
				high_2byte(player1.pos_y) - 1 - cam.pos_y,
				meta_player_list[sprite_anims[player1.sprite.anim.anim_current]->frames[player1.sprite.anim.anim_frame]]);
		}
	}
}

void draw_player_td()
{
	global_working_anim = &player1.sprite.anim;
	update_anim();

	// At the start of the next frame, when this oam data will be display, swap to the correct
	// sprite VRAM data.
	chr_index_queued = sprite_anims[player1.sprite.anim.anim_current]->chr_index;
	
	oam_meta_spr(
		high_2byte(player1.pos_x) - cam.pos_x, 
		high_2byte(player1.pos_y) - 1 - cam.pos_y,
		meta_player_td_list[sprite_anims[player1.sprite.anim.anim_current]->frames[player1.sprite.anim.anim_frame]]);
}