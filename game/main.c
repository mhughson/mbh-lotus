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

anim_info* global_working_anim;
game_actor player1;
camera cam;

unsigned char cur_state;
unsigned char cur_room_index;
unsigned char tile_index_param;
unsigned char loaded_obj_id;
unsigned char loaded_obj_index;
unsigned int cur_room_metatile_index;

unsigned int cur_room_width_pixels;
unsigned char cur_room_width_tiles;
unsigned int cur_room_size_tiles;
unsigned char cur_room_width_tiles_shift_factor;

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