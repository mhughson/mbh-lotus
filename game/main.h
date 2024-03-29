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

#if DEBUG_ENABLED
#define DEBUG_ASSERT(x) if(!(x)) assert();
#else
#define DEBUG_ASSERT(x)
#endif

#define PROF_CLEAR (0x1e & 0b11111001) // none
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
#define SAVE_VERSION 3
// The number of bytes that should contain the SAVE_VERSION at the 
// start of XRAM to confirm that this is valid save data.
#define NUM_SAVE_VERSION_VALIDATION 1

#define NUM_BG_BANKS 4 // MUST BE POWER OF 2
#define NUM_BG_BANK_SETS 2

// Flag used to signify the end of a metasprites data.
#define END_OF_META (0x80)

#define CELL_SIZE (16)
#define META_TILE_NUM_BYTES (8)
#define GRID_XY_TO_ROOM_INDEX(x,y) (((y) << cur_room_width_tiles_shift_factor) + (x))
#define META_TILE_SET_NUM_BYTES (128*META_TILE_NUM_BYTES)

#define NUM_CUSTOM_PROPS (10)
#define META_TILE_FLAGS_OFFSET (5)
#define GET_META_TILE_FLAGS(room_table_index) cur_metatiles[current_room[(room_table_index)] * META_TILE_NUM_BYTES + META_TILE_FLAGS_OFFSET]
// TODO: These should be needed anymore.
//#define GET_META_TILE_FLAGS_NEXT(room_table_index) GET_META_TILE_FLAGS(room_table_index) //cur_metatiles[next_room[(room_table_index)] * META_TILE_NUM_BYTES + META_TILE_FLAGS_OFFSET]
//#define GET_META_TILE_FLAGS_SCROLL(room_number, room_table_index) cur_metatiles[rooms[(room_number)][(room_table_index)] * META_TILE_NUM_BYTES + META_TILE_FLAGS_OFFSET]


#define NAMETABLE_TILES_8_UPDATED_PER_FRAME ((unsigned char)12)
#define NAMETABLE_TILES_8_UPDATED_PER_FRAME_TD ((unsigned char)32)
#define NAMETABLE_ATTRIBUTES_16_UPDATED_PER_FRAME (NAMETABLE_TILES_8_UPDATED_PER_FRAME / (unsigned char)4)

// Constants
#define HALF_POS_BIT_COUNT (16ul)
#define QUART_POS_BIT_COUNT (8)
#define FP_0_01 ((unsigned long)655ul) // approx
#define FP_0_05 ((unsigned long)3277ul) // approx
#define FP_0_15 ((unsigned long)9831ul) // approx
#define FP_0_18 ((unsigned long)11796ul) // aprrox
#define FP_0_25 ((unsigned long)16384ul)
#define FP_0_5 ((unsigned long)32768ul)
#define FP_0_75 ((unsigned long)49152ul)
#define FP_WHOLE(x) ((unsigned long)(x)<<HALF_POS_BIT_COUNT)
#define FP_WHOLE_16(x) ((signed int)(x)<<QUART_POS_BIT_COUNT)
#define FP_16_TO_32(x) ((unsigned long)(x)<<QUART_POS_BIT_COUNT)

// Tunables
#define JUMP_VEL_16bit ( (2 << 8) + 128 ) //  (FP_WHOLE(2ul) + FP_0_5)
#define JUMP_HOLD_MAX (15ul) // 10 = 3 tiles, 12 = 3.5 tiles, 15 = 4 tiles
#define GRAVITY_16bit (46) // (FP_0_18)
#define GRAVITY_LOW (FP_0_05)
#define WALK_SPEED_16bit (384) //(FP_WHOLE(1ul) + FP_0_5)
#define JUMP_COYOTE_DELAY (8)
#define ATTACK_LEN (5)
#define DASH_SPEED ((unsigned char)3)
#define FP_DASH_SPEED_EXIT_16bit WALK_SPEED_16bit // feels weird with anything higher than walk speed
#define DASH_LENGTH_TICKS (((unsigned char)4 * (unsigned char)16) / DASH_SPEED)
#define FP_AIR_FRICTION FP_0_05

// Induvidual flag meanings.
#define FLAG_SOLID (1 << 0)
#define FLAG_KILL  (1 << 1)

// TODO: We should be able to reduce this size to the number of commands that can fire in a single frame
//		 because if it grows over time, we likely have a death spiral.
#define DRAW_QUEUE_SIZE 64
#define QUEUE_UNUSED 0
#define QUEUE_DRAW_ROW 1
#define QUEUE_DRAW_COL 2
#define QUEUE_DRAW_ROW_ATTR 3
#define QUEUE_DRAW_COL_ATTR 4

#define OPTION_INDEX_MUSIC 0
#define OPTION_INDEX_SOUND 1

// NOTE: cur_sfx_chan goes out of range of FT_SFX_STREAMS, but that seems to be fine.
#define SFX_PLAY_WRAPPER(id) if (game_option_list_values[OPTION_INDEX_SOUND]) { sfx_play((id), ++cur_sfx_chan); }
// play a sound effect that is treated like music to the user (jingles, etc).
#define SFX_MUSIC_PLAY_WRAPPER(id) if (game_option_list_values[OPTION_INDEX_MUSIC]) { sfx_play((id), ++cur_sfx_chan); }
#define MUSIC_PLAY_WRAPPER(id) if (game_option_list_values[OPTION_INDEX_MUSIC]) { music_play((id)); }

#define ROOM_TYPE_SIDE (0)
#define ROOM_TYPE_TOP (1)

// IRQ COMMANDS
// All IRQ operations should ideally be wrapped in a command to 
// enforce the byte formats.
#define IRQ_BUFFER_LEN (32)

#define IRQ_CMD(cmd) irq_array[irq_index++] = (cmd);

// Writes to the "off" part of the buffer.
// NOTE: Reuses irq_index as a temp to lookup the array offset.
#define IRQ_CMD_BEGIN 	    		__asm__ ("lda %v", irq_cur_buffer_index); \
									__asm__ ("eor #1"); \
									__asm__ ("and #1"); \
									__asm__ ("sta %v", irq_index); \
									irq_index = mmc3_irq_buffer_start_offsets[irq_index];

#define IRQ_CMD_SCANLINE(line_num) 	IRQ_CMD(IRQ_SCANLINE(line_num));

#define IRQ_CMD_WRITE_2000(val) 	IRQ_CMD(IRQ_WRITE_2000); \
									IRQ_CMD(val);

#define IRQ_CMD_WRITE_2001(val) 	IRQ_CMD(IRQ_WRITE_2001); \
									IRQ_CMD(val);

// See: https://www.nesdev.org/wiki/PPU_scrolling#Split_X/Y_scroll
#define IRQ_CMD_H_V_SCROLL(x_val, y_val, nt_id)	IRQ_CMD(IRQ_H_V_SCROLL); \
												IRQ_CMD((nt_id << 2)); \
												IRQ_CMD(y_val); \
												IRQ_CMD(x_val); \
												IRQ_CMD((unsigned char)(((y_val & 0xf8) << 2) | (x_val >> 3))); 

#define IRQ_CMD_H_SCROLL(scroll_val) IRQ_CMD(IRQ_H_SCROLL); \
								     IRQ_CMD(scroll_val);

#define IRQ_CMD_SET_PPU_ADDRESS(hi, lo) IRQ_CMD(IRQ_SET_PPU_ADDR); \
										IRQ_CMD((hi)); \
										IRQ_CMD((lo));

#define IRQ_CMD_CHR_MODE_0(bank_id) IRQ_CMD(IRQ_CHR_MODE_0); \
									IRQ_CMD(IRQ_CHR_BANK(bank_id));
#define IRQ_CMD_CHR_MODE_1(bank_id) IRQ_CMD(IRQ_CHR_MODE_1); \
									IRQ_CMD(IRQ_CHR_BANK(bank_id));
#define IRQ_CMD_CHR_MODE_2(bank_id) IRQ_CMD(IRQ_CHR_MODE_2); \
									IRQ_CMD(IRQ_CHR_BANK(bank_id));
#define IRQ_CMD_CHR_MODE_3(bank_id) IRQ_CMD(IRQ_CHR_MODE_3); \
									IRQ_CMD(IRQ_CHR_BANK(bank_id));
#define IRQ_CMD_CHR_MODE_4(bank_id) IRQ_CMD(IRQ_CHR_MODE_4); \
									IRQ_CMD(IRQ_CHR_BANK(bank_id));
#define IRQ_CMD_CHR_MODE_5(bank_id) IRQ_CMD(IRQ_CHR_MODE_5); \
									IRQ_CMD(IRQ_CHR_BANK(bank_id));

#define IRQ_CMD_END IRQ_CMD(IRQ_END);

// Swap the "active" buffer. The eor stuff just swaps 0->1 or 1->0
#define IRQ_CMD_FLUSH 		__asm__ ("lda %v", irq_cur_buffer_index); \
    						__asm__ ("eor #1"); \
    						__asm__ ("and #1"); \
    						__asm__ ("sta %v", irq_cur_buffer_index); \
    						mmc3_irq_buffer_offset = mmc3_irq_buffer_start_offsets[irq_cur_buffer_index];
							


enum
{
	ANIM_PLAYER_IDLE = 0,
	ANIM_PLAYER_RUN,
	ANIM_PLAYER_JUMP,
	ANIM_PLAYER_FALL,
	ANIM_PLAYER_DASH,

	ANIM_IDLE_DOWN_TD ,
	ANIM_IDLE_RIGHT_TD,
	ANIM_IDLE_UP_TD ,
	ANIM_IDLE_LEFT_TD ,
	ANIM_WALK_DOWN_TD ,
	ANIM_WALK_RIGHT_TD,
	ANIM_WALK_UP_TD ,
	ANIM_WALK_LEFT_TD ,

	ANIM_SKEL_WALK_RIGHT,
	ANIM_SKEL_SQUISHED,
	
	ANIM_BIRD_FLY_RIGHT,
	ANIM_BIRD_FALL_RIGHT,

	NUM_ANIMS,
};


enum { STATE_BOOT, STATE_TITLE, STATE_GAME, STATE_OVER };

enum { BANK_0 = 0, BANK_1, BANK_2, BANK_3, BANK_4, BANK_5, BANK_6, BANK_7 };

#pragma bss-name(push, "ZEROPAGE")
#pragma bss-name(pop)

// TODO: Where is non-zero page? Is this just starting at zero page?

// Structs.
//

#define MAX_ANIM_INFO (16)

typedef struct anim_info
{
	// index into sprite_anims array.
	unsigned char anim_current[MAX_ANIM_INFO];
	unsigned char anim_queued[MAX_ANIM_INFO];

	// how many ticks have passed since the last frame change.
	unsigned char anim_ticks[MAX_ANIM_INFO];

	// the currently displaying frame of the current anim.
	unsigned char anim_frame[MAX_ANIM_INFO];
} anim_info;

typedef struct camera
{
	unsigned int pos_x;
	unsigned int pos_y;

	// cached values for where offscreen objects should go into a
	// frozen state or a re-activate. This is done once at the end
	// of the frame, rather than per-object.
	unsigned int freeze_left;
	unsigned int freeze_right;
	unsigned int thaw_left;
	unsigned int thaw_right;
} camera;

typedef struct game_actor
{
	unsigned char anim_data_index;

	unsigned long pos_x;
	unsigned long pos_y;

	signed int vel_x16;
	signed int vel_y16;

	signed char dir_x;
	signed char dir_y;
} game_actor;


typedef enum TRIGGER_OBJECT_TYPES
{
	TRIG_PLAYER_SPAWN_POINT = 0,
	TRIG_TRANS_POINT,
	TRIG_TRANS_EDGE,
	TRIG_TRANS_EDGE_VERT,
	TRIG_SKELETON,
	TRIG_BIRD,

	TRIG_UNUSED = 0xff,
} TRIGGER_OBJECT_TYPES;

#define PAYLOAD_TRANS_EDGE_VERT_DEST_MASK 0b00100000

#define MAX_TRIGGERS (8)
typedef struct trigger_objects
{
	TRIGGER_OBJECT_TYPES type[MAX_TRIGGERS];
	unsigned char pos_x_tile[MAX_TRIGGERS];
	unsigned char pos_y_tile[MAX_TRIGGERS];
	unsigned char payload[MAX_TRIGGERS];	
} trigger_objects;


#define MAX_DYNAMIC_OBJS (8)
#define DYNAMIC_STATE_FROZEN 		(0b00000001)
#define DYNAMIC_STATE_DEAD 			(0b00000010)

#define FROZEN_OFFSET (64)
#define THAW_OFFSET (32)


typedef struct dynamic_actors
{
	unsigned char anim_data_index[MAX_DYNAMIC_OBJS];

	unsigned int pos_x[MAX_DYNAMIC_OBJS];
	unsigned int pos_y[MAX_DYNAMIC_OBJS];

	signed char dir_x[MAX_DYNAMIC_OBJS];
	signed char dir_y[MAX_DYNAMIC_OBJS];

	TRIGGER_OBJECT_TYPES type[MAX_DYNAMIC_OBJS];

	unsigned char payload[MAX_DYNAMIC_OBJS];	

	unsigned char time_in_state[MAX_DYNAMIC_OBJS];

	unsigned char state[MAX_DYNAMIC_OBJS];
} dynamic_actors;

// RAM
//

extern anim_info animation_data; // +1 for player.
extern unsigned char animation_data_count; // counter as animation_data is used up.

extern unsigned char tick_count;
extern unsigned int tick_count16;
extern unsigned int ticks_in_state16;
extern unsigned char pad_all;
extern unsigned char pad_all_new;
extern unsigned int index16;
extern unsigned char x;
extern unsigned char y;
extern unsigned char index;
extern unsigned char index2;
extern unsigned char i;
extern unsigned long temp32;
extern unsigned char tempFlags;
extern unsigned char ticks_since_attack;
// temp used for working with a single vertical row of tiles.
extern unsigned char nametable_col[36];
extern unsigned char nametable_col_b[36];
extern unsigned char irq_index;

// inputs for cached camera positions. see "cam" struct for *actual* camera.
extern unsigned int cam_x;
extern unsigned int cam_y;
extern unsigned int old_cam_x;
extern unsigned int old_cam_y;

extern unsigned char draw_queue_index;
extern unsigned char draw_dequeue_index;
extern unsigned int draw_queue[DRAW_QUEUE_SIZE];




// At some point I think I will need to keep the current 2 nametables of level data in RAM, so that I can
// edit it on the fly (eg. destructable blocks). Reserving it for now, since this is a large chunk of memory
// that will be challenging to peel back after its used.
#define RAINY_DAY_RAM_SIZE (128)
extern unsigned char rainy_day_ram[RAINY_DAY_RAM_SIZE];


extern unsigned int in_x_tile;
extern unsigned int in_y_tile;

extern unsigned int in_x_pixel;
extern unsigned int in_y_pixel;

extern unsigned char in_flip_nt;

extern unsigned char in_oam_x;
extern unsigned char in_oam_y;
extern const unsigned char *in_oam_data;

extern unsigned char in_is_streaming;
extern unsigned char in_destination_spawn_id;
extern unsigned char in_stream_direction;

extern unsigned char in_dynamic_obj_index;

extern unsigned char in_vert_dest_right;

extern unsigned char out_num_tiles;

// Used by the anim functions to avoid passing in a parameter.
extern unsigned char in_working_anim_index;

extern game_actor player1;
extern camera cam;
extern trigger_objects trig_objs;
extern dynamic_actors dynamic_objs;

extern unsigned char cur_state;

// The currently loading room index (or the one we want to load in the case of input to function).
extern unsigned char cur_room_index;
// Input to copy_original_room_to_current
extern unsigned char tile_index_param;
// Output from get_obj_id
extern unsigned char loaded_obj_id;
extern unsigned char loaded_obj_index;
// Output from get_next_object
extern unsigned char loaded_obj_x;
extern unsigned char loaded_obj_y;
extern unsigned char loaded_obj_payload;
extern unsigned char in_obj_index;
// Output from get_cur_room_metatile_set
extern unsigned int cur_room_metatile_index;

extern unsigned int cur_room_width_pixels;
extern unsigned char cur_room_width_tiles;
extern unsigned char cur_room_width_attributes;
extern unsigned int cur_room_height_pixels;
extern unsigned char cur_room_height_tiles;
extern unsigned char cur_room_width_tiles_shift_factor;
extern unsigned int cur_room_size_tiles;

extern unsigned char cur_nametable_y;
extern unsigned char cur_nametable_y_left;
extern unsigned char cur_nametable_y_right;
extern unsigned char cur_nametable_x;
extern unsigned char cur_nametable_x_top;
extern unsigned char cur_nametable_x_bottom;

extern unsigned char cur_room_type;

// Counter used to ensure each sound effect ends up on a different
// channel. (no regard to priority)
extern unsigned char cur_sfx_chan;

// Player
extern unsigned char anim_index;
extern unsigned char grounded;
extern unsigned char jump_held_count;
extern unsigned char can_jump;
extern unsigned char airtime;
extern unsigned char ticks_down;
extern unsigned char jump_count;
extern unsigned char on_ground;
extern unsigned char new_jump_btn;
extern unsigned int scroll_y;
// How long the dash has been active.
extern unsigned char dash_time;
// How many dashes have been executed since the last
// time the player landed on ground.
extern unsigned char dash_count;

extern unsigned char chr_index_queued;
extern unsigned char chr_3_index_queued;

#define NUM_Y_COLLISION_OFFSETS 3
extern const unsigned char y_collision_offsets[NUM_Y_COLLISION_OFFSETS];
#define NUM_X_COLLISION_OFFSETS 2
extern const unsigned char x_collision_offsets[NUM_X_COLLISION_OFFSETS];

// 2x 32 byte buffers. Ping Pong back and forth for double buffer.
extern unsigned char irq_array[IRQ_BUFFER_LEN * 2];
extern unsigned char irq_cur_buffer_index;
extern const unsigned char mmc3_irq_buffer_start_offsets[2];

// assembly exports.
extern unsigned char SPR_FLIP_META;
 #pragma zpsym ("SPR_FLIP_META"); // zero-page
extern unsigned char SPR_OFFSCREEN_META;
 #pragma zpsym ("SPR_OFFSCREEN_META");
 extern unsigned char mmc3_irq_buffer_offset;
 #pragma zpsym ("mmc3_irq_buffer_offset");



// XRAM
//

// Yup! Almost 4KB of RAM going to a copy of the current room!
// Max square room: 64x60 tiles
#define MAX_ROOM_NUM_TILES (240 * 16)
extern unsigned char save_version_validation[NUM_SAVE_VERSION_VALIDATION];
extern unsigned char current_room[MAX_ROOM_NUM_TILES];
extern unsigned char current_room_attr[MAX_ROOM_NUM_TILES / 4];
extern unsigned char cur_metatiles[META_TILE_SET_NUM_BYTES];
extern unsigned char checkpoint_room_index;
extern unsigned char checkpoint_spawn_id;
#define OPTION_LIST_LEN 2
extern unsigned char game_option_list_values[OPTION_LIST_LEN];

// Functions
//

void queue_next_anim(unsigned char next_anim_index);
void commit_next_anim();

#define FADE_DELAY 2
void fade_to_black();
void fade_from_black();
void fade_to_white();
void fade_from_white();


unsigned char get_chr_mode_0();
void set_chr_bank_for_current_room();

void bounce_player();

#endif // ONCE_MAIN_H