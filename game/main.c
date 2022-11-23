/** (C) Matt Hughson 2020 */

#include "LIB/neslib.h"
#include "LIB/nesdoug.h"
#include "mmc3\mmc3_code.h"
#include "../include/stdlib.h"

#include "main.h"
#include "PRG0.h"
#include "PRG1.h"
#include "PRG2.h"

#pragma rodata-name ("CODE")
#pragma code-name ("CODE")


// Initalized RAM variables
//

unsigned char tick_count;
unsigned char pad_all;
unsigned char pad_all_new;
unsigned int cur_col;
unsigned int index16;
unsigned char x;
unsigned char y;
unsigned char index;
unsigned char index2;
unsigned char i;
unsigned char temp;
unsigned long temp32;
unsigned char tempFlags;
unsigned char ticks_since_attack = 0xff;
unsigned char nametable_col[30];
unsigned char nametable_col_b[30];

unsigned char current_room_page_reserve[ROOM_PAGE_SIZE];

unsigned int in_x_tile;
unsigned int in_y_tile;
unsigned int in_x_pixel;
unsigned char in_flip_nt;

unsigned char in_oam_x;
unsigned char in_oam_y;
const unsigned char *in_oam_data;

anim_info* global_working_anim;
game_actor player1;
camera cam;
trigger_object trig_objs[MAX_TRIGGERS];

unsigned char cur_state;
unsigned char cur_room_index;
unsigned char tile_index_param;
unsigned char loaded_obj_id;
unsigned char loaded_obj_index;
unsigned char loaded_obj_x;
unsigned char loaded_obj_y;
unsigned char loaded_obj_payload;
unsigned char in_obj_index;
unsigned int cur_room_metatile_index;

unsigned int cur_room_width_pixels;
unsigned char cur_room_width_tiles;
unsigned int cur_room_size_tiles;
unsigned char cur_room_width_tiles_shift_factor;
unsigned char cur_nametable_y;
unsigned char cur_nametable_y_left;
unsigned char cur_nametable_y_right;

#pragma bss-name(push, "XRAM")
// extra RAM at $6000-$7fff
//unsigned char wram_array[0x2000];
unsigned char save_version_validation[NUM_SAVE_VERSION_VALIDATION];
unsigned char current_room[MAX_ROOM_NUM_TILES];
#pragma bss-name(pop)

#pragma rodata-name ("CODE")
#pragma code-name ("CODE")


// It seems like main() MUST be in fixed back!
void main (void) 
{
	banked_call(BANK_0, main_real);
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

void c_oam_meta_spr_flipped(void)
{
	static const unsigned char* _elem;
	static unsigned char _x;
	static unsigned char _y;
	static unsigned char _sprite;
	static unsigned char _att;

	if (in_oam_data == NULL)
	{
		return;
	}

	_elem = in_oam_data;

	while (_elem[0] != END_OF_META)
	{
		_x = 8 - _elem[0];
		_y = _elem[1];
		_sprite = _elem[2];
		_att = _elem[3] | OAM_FLIP_H;
		_elem += 4;
		oam_spr(in_oam_x + _x, in_oam_y + _y, _sprite, _att);
	}
}

#define FADE_DELAY 2
void fade_to_black()
{
	pal_bright(3);
	delay(FADE_DELAY);
	pal_bright(2);
	delay(FADE_DELAY);
	pal_bright(1);
	delay(FADE_DELAY);
	pal_bright(0);
//	delay(FADE_DELAY);
}

void fade_from_black()
{
	pal_bright(1);
	delay(FADE_DELAY);
	pal_bright(2);
	delay(FADE_DELAY);
	pal_bright(3);
	delay(FADE_DELAY);
	pal_bright(4);
//	delay(FADE_DELAY);
}

#define FADE_DELAY_SLOW 10
void fade_to_white()
{
	pal_bright(5);
	delay(FADE_DELAY_SLOW);
	pal_bright(6);
	delay(FADE_DELAY_SLOW);
	pal_bright(7);
	delay(FADE_DELAY_SLOW);
	pal_bright(8);
//	delay(FADE_DELAY);
}

void fade_from_white()
{
	pal_bright(7);
	delay(FADE_DELAY);
	pal_bright(6);
	delay(FADE_DELAY);
	pal_bright(5);
	delay(FADE_DELAY);
	pal_bright(4);
//	delay(FADE_DELAY);
}