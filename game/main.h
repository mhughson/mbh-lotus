/** (C) Matt Hughson 2020 */

// Note: all variables need extern and should be defined in main.c

#ifndef ONCE_MAIN_H
#define ONCE_MAIN_H

#define DEBUG_ENABLED 0

#if DEBUG_ENABLED
#define PROFILE_POKE(val) POKE((0x2001),(val));
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

// Used as both a potential versioning system, and also the value
// padded at the start of XRAM to validate a valid save game.
#define SAVE_VERSION 1
// The number of bytes that should contain the SAVE_VERSION at the 
// start of XRAM to confirm that this is valid save data.
#define NUM_SAVE_VERSION_VALIDATION 4

#define CELL_SIZE (16)
#define META_TILE_NUM_BYTES (8)
#define ROOM_WIDTH_PAGES (4)
#define ROOM_WIDTH_PIXELS (256*ROOM_WIDTH_PAGES)
#define ROOM_WIDTH_TILES (16*ROOM_WIDTH_PAGES)
#define GRID_XY_TO_ROOM_INDEX(x,y) (((y) * ROOM_WIDTH_TILES) + (x))

#define META_TILE_FLAGS_OFFSET (5)
#define GET_META_TILE_FLAGS(room_table_index) metatiles_temp[current_room[(room_table_index)] * META_TILE_NUM_BYTES + META_TILE_FLAGS_OFFSET]
// TODO: These should be needed anymore.
//#define GET_META_TILE_FLAGS_NEXT(room_table_index) GET_META_TILE_FLAGS(room_table_index) //metatiles_temp[next_room[(room_table_index)] * META_TILE_NUM_BYTES + META_TILE_FLAGS_OFFSET]
//#define GET_META_TILE_FLAGS_SCROLL(room_number, room_table_index) metatiles_temp[rooms[(room_number)][(room_table_index)] * META_TILE_NUM_BYTES + META_TILE_FLAGS_OFFSET]

// Constants
#define HALF_POS_BIT_COUNT (16ul)
#define FP_0_05 ((unsigned long)3277ul) // approx
#define FP_0_18 ((unsigned long)11796ul) // aprrox
#define FP_0_25 ((unsigned long)16384ul)
#define FP_0_5 ((unsigned long)32768ul)
#define FP_0_75 ((unsigned long)49152ul)
#define FP_WHOLE(x) ((unsigned long)(x)<<HALF_POS_BIT_COUNT)

// Tunables
#define JUMP_VEL (FP_WHOLE(3ul) + FP_0_5)
#define JUMP_HOLD_MAX (15ul)
#define GRAVITY (FP_0_25)
#define GRAVITY_LOW (FP_0_05)
#define WALK_SPEED (FP_WHOLE(1ul) + FP_0_5)
#define JUMP_COYOTE_DELAY (8)
#define ATTACK_LEN (5)

// Induvidual flag meanings.
#define FLAG_SOLID (1 << 0)
#define FLAG_KILL  (1 << 1)

enum
{
	ANIM_PLAYER_IDLE_RIGHT = 0,
	ANIM_PLAYER_IDLE_LEFT = 1, 
	ANIM_PLAYER_RUN_RIGHT = 2,
	ANIM_PLAYER_RUN_LEFT = 3, 
	ANIM_PLAYER_JUMP_RIGHT = 4,
	ANIM_PLAYER_JUMP_LEFT = 5, 
	ANIM_PLAYER_FALL_RIGHT = 6,
	ANIM_PLAYER_FALL_LEFT = 7, 
	ANIM_PLAYER_IDLE_CROUCH_RIGHT = 8,
	ANIM_PLAYER_IDLE_CROUCH_LEFT = 9,
	// ANIM_PLAYER_IDLE_ATTACK_RIGHT = 10,
	// ANIM_PLAYER_JUMP_ATTACK_RIGHT = 11,
	// ANIM_PLAYER_WALK_ATTACK_RIGHT = 12,

	NUM_ANIMS,
};


enum { STATE_BOOT, STATE_TITLE, STATE_GAME, STATE_OVER };

enum { BANK_0 = 0, BANK_1, BANK_2, BANK_3 };

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

// data speciic to player game objects.
typedef struct animated_sprite
{
	// Was the last horizontal move *attempted* in the left direction?
	unsigned char facing_left;

	// Stores all of the active animation info.
	anim_info anim;

} animated_sprite;

typedef struct game_actor
{
	animated_sprite sprite;

	unsigned long pos_x;
	unsigned long pos_y;

	signed long vel_y;

	unsigned char facing_left;
} game_actor;


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
extern unsigned long temp32;
extern unsigned char tempFlags;
extern unsigned char ticks_since_attack;
// temp used for working with a single vertical row of tiles.
extern unsigned char nametable_col[30];
extern unsigned char nametable_col_b[30];

// At some point I think I will need to keep the current 2 nametables of level data in RAM, so that I can
// edit it on the fly (eg. destructable blocks). Reserving it for now, since this is a large chunk of memory
// that will be challenging to peel back after its used.
#define ROOM_PAGE_SIZE (16 * 15 * 2)
extern unsigned char current_room_page_reserve[ROOM_PAGE_SIZE];


extern unsigned int in_x_tile;
extern unsigned int in_y_tile;

// Used by the anim functions to avoid passing in a parameter.
extern anim_info* global_working_anim;

extern game_actor player1;
extern camera cam;

extern unsigned char cur_state;

// XRAM
//

extern unsigned char save_version_validation[NUM_SAVE_VERSION_VALIDATION];

// Functions
//

void queue_next_anim(unsigned char next_anim_index);
void commit_next_anim();

#define FADE_DELAY 2
void fade_to_black();
void fade_from_black();
void fade_to_white();
void fade_from_white();

#endif // ONCE_MAIN_H