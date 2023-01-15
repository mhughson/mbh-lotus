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
#include "NES_ST/meta_enemies.h"

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
#define CHR_TD 12
#define CHR_SKEL 14
#define CHR_BIRD 14

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

const anim_def skel_walk_right	= { 20, 2, { 0, 1 }, CHR_SKEL};
const anim_def skel_squished	= { 255, 1, { 2 }, CHR_SKEL};

const anim_def bird_fly_right	= { 20, 2, { 3, 4 }, CHR_BIRD};
const anim_def bird_fall_right	= { 5, 3, { 3, 5, 6 }, CHR_BIRD};

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

	&skel_walk_right,
	&skel_squished,

	&bird_fly_right,
	&bird_fall_right,
};

const char wobble_table[4] = { -1, 0, 1, 0 };

unsigned char update_anim()
{
	static const struct anim_def* cur_anim;
    static unsigned char local_tmp1;
    static unsigned char local_tmp2;
    
	cur_anim = sprite_anims[animation_data.anim_current[in_working_anim_index]];

	// Note: In WnW this was done in each draw function manually, and I'm not sure why. Perhaps
	//		 to ensure that the first frame got played before advancing?
	++animation_data.anim_ticks[in_working_anim_index];

	// optimization to avoid int compare.
    local_tmp1 = animation_data.anim_ticks[in_working_anim_index];
    local_tmp2 = cur_anim->ticks_per_frame;
    if (local_tmp1 >= local_tmp2)
	{
		animation_data.anim_ticks[in_working_anim_index] = 0;
		++animation_data.anim_frame[in_working_anim_index];

		// optimization to avoid int compare.
        local_tmp1 = animation_data.anim_frame[in_working_anim_index];
        local_tmp2 = cur_anim->anim_len;
        if (local_tmp1 >= local_tmp2)
		{
			animation_data.anim_frame[in_working_anim_index] = 0;
			return 1;
		}
	}
	return 0;
}

void draw_player()
{
	in_working_anim_index = player1.anim_data_index;
	update_anim();

	draw_player_static();
}

void draw_player_static()
{
	// At the start of the next frame, when this oam data will be display, swap to the correct
	// sprite VRAM data.
	chr_index_queued = sprite_anims[animation_data.anim_current[player1.anim_data_index]]->chr_index;

	// When jumping UP past the top of the screen, the rendering loops into the attr table (240-256).
	// That gives us 16 pixels where the sprite can continue to draw offscreen, without popping up
	// on the bottom of the screen.
	// NOTE: Had to switch from -16 to -13 to avoid a few pixels spilling over. Not sure why.
	if (high_2byte(player1.pos_y) < 240 || high_2byte(player1.pos_y) > (0xffff - 13))
	{
		if (player1.dir_x < 0)
		{
			SPR_FLIP_META = 1;
		}
		else
		{
			SPR_FLIP_META = 0;
		}

		SPR_OFFSCREEN_META = 0;

		oam_meta_spr(
			high_2byte(player1.pos_x) - cam.pos_x, 
			high_2byte(player1.pos_y) - 1 - cam.pos_y,
			meta_player_list[sprite_anims[animation_data.anim_current[player1.anim_data_index]]->frames[animation_data.anim_frame[player1.anim_data_index]]]);
	}
}

void draw_player_td()
{
	in_working_anim_index = player1.anim_data_index;
	update_anim();

	// At the start of the next frame, when this oam data will be display, swap to the correct
	// sprite VRAM data.
	chr_index_queued = sprite_anims[animation_data.anim_current[player1.anim_data_index]]->chr_index;
	
	SPR_FLIP_META = 0;
	SPR_OFFSCREEN_META = 0;
	oam_meta_spr(
		high_2byte(player1.pos_x) - cam.pos_x, 
		high_2byte(player1.pos_y) - 1 - cam.pos_y,
		meta_player_td_list[sprite_anims[animation_data.anim_current[player1.anim_data_index]]->frames[animation_data.anim_frame[player1.anim_data_index]]]);
}

void draw_skeleton()
{
	in_working_anim_index = dynamic_objs.anim_data_index[in_dynamic_obj_index];
	update_anim();

	// At the start of the next frame, when this oam data will be display, swap to the correct
	// sprite VRAM data.

	// TODO: load into other slots.
	chr_3_index_queued = sprite_anims[animation_data.anim_current[in_working_anim_index]]->chr_index;

	if (dynamic_objs.dir_x[in_dynamic_obj_index] < 0)
	{
		SPR_FLIP_META = 1;
	}
	else
	{
		SPR_FLIP_META = 0;
	}

	SPR_OFFSCREEN_META = (dynamic_objs.pos_x[in_dynamic_obj_index]) < cam.pos_x || dynamic_objs.pos_x[in_dynamic_obj_index] >= (cam.pos_x + 256);

	oam_meta_spr(
		dynamic_objs.pos_x[in_dynamic_obj_index] - cam.pos_x,
		dynamic_objs.pos_y[in_dynamic_obj_index] - 1 - cam.pos_y,
		meta_enemies_list[sprite_anims[animation_data.anim_current[in_working_anim_index]]->frames[animation_data.anim_frame[in_working_anim_index]]]);
}

void draw_bird()
{
	in_working_anim_index = dynamic_objs.anim_data_index[in_dynamic_obj_index];
	update_anim();

	// At the start of the next frame, when this oam data will be display, swap to the correct
	// sprite VRAM data.

	// TODO: load into other slots.
	chr_3_index_queued = sprite_anims[animation_data.anim_current[in_working_anim_index]]->chr_index;

	if (dynamic_objs.dir_x[in_dynamic_obj_index] < 0)
	{
		SPR_FLIP_META = 1;
	}
	else
	{
		SPR_FLIP_META = 0;
	}

	SPR_OFFSCREEN_META = (dynamic_objs.pos_x[in_dynamic_obj_index]) < cam.pos_x || dynamic_objs.pos_x[in_dynamic_obj_index] >= (cam.pos_x + 256);

	oam_meta_spr(
		dynamic_objs.pos_x[in_dynamic_obj_index] - cam.pos_x,
		dynamic_objs.pos_y[in_dynamic_obj_index] - 1 - cam.pos_y + (wobble_table[(tick_count >> 3) % 4]),
		meta_enemies_list[sprite_anims[animation_data.anim_current[in_working_anim_index]]->frames[animation_data.anim_frame[in_working_anim_index]]]);
}