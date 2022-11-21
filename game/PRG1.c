#include "PRG1.h"
#include "main.h"
#include "LIB/neslib.h"
#include "LIB/nesdoug.h"
#include "mmc3\mmc3_code.h"
#include "../include/stdlib.h"

#pragma rodata-name ("BANK1")
#pragma code-name ("BANK1")

#include "NES_ST/meta_player.h"

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

const anim_def idle_right = { 20, 4, { 0, 1, 2, 3 } };
const anim_def walk_right = { 7, 4, { 4, 5, 6, 5 } };
const anim_def jump_right = { 255, 1, { 7 } };
const anim_def fall_right = { 255, 1, { 8 } };

const struct anim_def* sprite_anims[] =
{
	&idle_right,

	&walk_right,

	&jump_right,

	&fall_right,
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
	if (high_2byte(player1.pos_y) < 240 || high_2byte(player1.pos_y) > (0xffff - 16))
	{
		if (player1.facing_left == 1)
		{
			in_oam_x = high_2byte(player1.pos_x) - cam.pos_x;
			in_oam_y = high_2byte(player1.pos_y) - 1 - cam.pos_y;
			in_oam_data = meta_player_list[sprite_anims[player1.sprite.anim.anim_current]->frames[player1.sprite.anim.anim_frame]];
			c_oam_meta_spr_flipped();
		}
		else
		{		
			oam_meta_spr(
				high_2byte(player1.pos_x) - cam.pos_x, 
				high_2byte(player1.pos_y) - 1 - cam.pos_y,
				meta_player_list[sprite_anims[player1.sprite.anim.anim_current]->frames[player1.sprite.anim.anim_frame]]);
		}
	}
}