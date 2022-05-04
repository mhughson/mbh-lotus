/** (C) Matt Hughson 2020 */

#include "LIB/neslib.h"
#include "LIB/nesdoug.h"
#include "A53/bank_helpers.h"
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
unsigned char i;
unsigned long temp32;
unsigned char nametable_col[30];

unsigned int in_x_tile;
unsigned int in_y_tile;

anim_info* global_working_anim;
game_actor player1;
camera cam;

unsigned char cur_state;

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