/** (C) Matt Hughson 2020 */

// Note: all variables need extern and should be defined in main.c

#ifndef ONCE_MAIN_H
#define ONCE_MAIN_H

#define DEBUG_ENABLED 0

#if DEBUG_ENABLED
#define PROFILE_POKE(val) //POKE((0x2001),(val));
#else
#define PROFILE_POKE(val)
#endif

#define PROF_CLEAR 0x1e // none
#define PROF_R_TINT 0x3e // red
#define PROF_G_TINT 0x5e // green
#define PROF_B_TINT 0x9e // blue
#define PROF_W_TINT 0x1e // white
#define PROF_R 0x3f // red + grey
#define PROF_G 0x5f // green + grey
#define PROF_B 0x9f // blue + grey
#define PROF_W 0x1f // white + grey

#define CELL_SIZE (16)
#define META_TILE_NUM_BYTES (8)
#define ROOM_WIDTH_PAGES (4)
#define ROOM_WIDTH_PIXELS (256*ROOM_WIDTH_PAGES)
#define ROOM_WIDTH_TILES (16*ROOM_WIDTH_PAGES)
#define GRID_XY_TO_ROOM_INDEX(x,y) (((y) * ROOM_WIDTH_TILES) + (x))

#pragma bss-name(push, "ZEROPAGE")
#pragma bss-name(pop)

// TODO: Where is non-zero page? Is this just starting at zero page?

// Structs.
//

typedef struct anim_info
{
	// index into sprite_anims array.
	unsigned char anim_current;
	unsigned char anim_queued;

	// how many ticks have passed since the last frame change.
	unsigned char anim_ticks;

	// the currently displaying frame of the current anim.
	unsigned char anim_frame;
} anim_info;

typedef struct camera
{
	unsigned int pos_x;
	unsigned int pos_y;
} camera;

typedef struct object
{
	anim_info anim;
	unsigned int pos_x;
	unsigned int pos_y;
} object;


// RAM
//

extern unsigned char tick_count;
extern unsigned char pad_all;
extern unsigned char pad_all_new;
extern unsigned int index16;
extern unsigned char x;
extern unsigned char y;
extern unsigned char index;
extern unsigned char i;
// temp used for working with a single vertical row of tiles.
extern unsigned char nametable_col[30];

extern unsigned int in_x_tile;
extern unsigned int in_y_tile;

// Used by the anim functions to avoid passing in a parameter.
extern anim_info* global_working_anim;

extern object player;
extern camera cam;

// Functions
//

void queue_next_anim(unsigned char next_anim_index);
void commit_next_anim();

#endif // ONCE_MAIN_H