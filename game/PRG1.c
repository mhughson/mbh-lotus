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

const anim_def idle_right = { 5, 3, { 0, 1, 2 } };
const anim_def walk_right = { 5, 6, { 5, 6, 7, 8, 9, 10 } };
const anim_def jump_right = { 60, 1, { 3 } };
const anim_def fall_right = { 60, 1, { 4 } };
const anim_def idle_left = { 5, 3, { 11, 12, 13 } };
const anim_def walk_left = { 5, 6, { 16, 17, 18, 19, 20, 21 } };
const anim_def jump_left = { 60, 1, { 14 } };
const anim_def fall_left = { 60, 1, { 15 } };
const anim_def idle_crouch_right = { 60, 1, { 22 } };
const anim_def idle_crouch_left = { 60, 1, { 23 } };
// const anim_def idle_attack_right = { 60, 1, { 24 } };
// const anim_def jump_attack_right = { 60, 1, { 25 } };
// const anim_def walk_attack_right = { 5, 6, { 26, 27, 28, 29, 30, 31 } };

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

	&idle_crouch_right,
	&idle_crouch_left,

	// &idle_attack_right,
	// &jump_attack_right,
	// &walk_attack_right,
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

	//cur_cam_x = high_2byte(player1.pos_x) - 128;

	if (high_2byte(player1.pos_y) < 240 || high_2byte(player1.pos_y) > (0xffff - 16))
	{
		oam_meta_spr(
			high_2byte(player1.pos_x) - cam.pos_x, 
			high_2byte(player1.pos_y) - 1 - cam.pos_y,
			meta_player_list[sprite_anims[player1.sprite.anim.anim_current]->frames[player1.sprite.anim.anim_frame]]);
	}

	// Update animation.
	//++player1.sprite.anim.anim_ticks;

}