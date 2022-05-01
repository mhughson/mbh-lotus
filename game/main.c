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
unsigned char nametable_col[30];

unsigned int in_x_tile;
unsigned int in_y_tile;

anim_info* global_working_anim;
anim_info player_anim;

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