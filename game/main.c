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

// Initalized RAM variables
//

unsigned char tick_count;
unsigned char pad_all;
unsigned char pad_all_new;

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

		if (pad_all & PAD_A)
		{
			oam_meta_spr(128, 127, meta_player_list[3]);
		}		
		else if (pad_all & PAD_B)
		{
			oam_meta_spr(128, 127, meta_player_list[4]);
		}
		else if (pad_all & PAD_RIGHT)
		{
			oam_meta_spr(128, 127, meta_player_list[((tick_count >> 3) % 6) + 5]);
		}
		else
		{
			oam_meta_spr(128, 127, meta_player_list[(tick_count >> 3) % 3]);
		}


	}
}