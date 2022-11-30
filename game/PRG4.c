
#include "main.h"
#include "LIB/neslib.h"
#include "LIB/nesdoug.h"
#include "mmc3\mmc3_code.h"
#include "../include/stdlib.h"

#pragma rodata-name ("BANK4")
#pragma code-name ("BANK4")

#include "NES_ST/screen_title.h"
#include "NES_ST/screen_gameover.h"

void load_screen_title()
{
	vram_adr(NTADR_A(0,0));
    vram_unrle(screen_title);
}

void load_screen_gameover()
{
	vram_adr(NTADR_A(0,0));
    vram_unrle(screen_gameover);
}