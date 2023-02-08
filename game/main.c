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

anim_info animation_data; // +1 for player.
unsigned char animation_data_count; // counter as animation_data is used up.

unsigned char tick_count;
unsigned int tick_count16;
unsigned int ticks_in_state16;
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
unsigned char nametable_col[36];
unsigned char nametable_col_b[36];
unsigned char irq_index;
unsigned int cam_x;
unsigned int cam_y;
unsigned int old_cam_x;
unsigned int old_cam_y;
unsigned char draw_queue_index;
unsigned char draw_dequeue_index;
unsigned int draw_queue[DRAW_QUEUE_SIZE];

unsigned char rainy_day_ram[RAINY_DAY_RAM_SIZE];

unsigned int in_x_tile;
unsigned int in_y_tile;
unsigned int in_x_pixel;
unsigned int in_y_pixel;
unsigned char in_flip_nt;

unsigned char in_oam_x;
unsigned char in_oam_y;
const unsigned char *in_oam_data;
unsigned char in_is_streaming;
unsigned char in_destination_spawn_id;
unsigned char in_stream_direction;
unsigned char in_dynamic_obj_index;
unsigned char in_vert_dest_right;

unsigned char out_num_tiles;

unsigned char in_working_anim_index;
game_actor player1;
camera cam;
trigger_objects trig_objs;
dynamic_actors dynamic_objs;

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
unsigned char cur_room_width_attributes;
unsigned int cur_room_height_pixels;
unsigned char cur_room_height_tiles;
unsigned int cur_room_size_tiles;
unsigned char cur_room_width_tiles_shift_factor;
unsigned char cur_nametable_y;
unsigned char cur_nametable_y_left;
unsigned char cur_nametable_y_right;
unsigned char cur_nametable_x;
unsigned char cur_nametable_x_top;
unsigned char cur_nametable_x_bottom;
unsigned char cur_room_type;

unsigned char cur_sfx_chan;

unsigned char anim_index;
unsigned char grounded;
unsigned char jump_held_count;
unsigned char can_jump;
unsigned char airtime;
unsigned char ticks_down;
unsigned char jump_count;
unsigned char on_ground;
unsigned char new_jump_btn;
unsigned int scroll_y;
unsigned char dash_time;
unsigned char dash_count;

unsigned char chr_index_queued;
unsigned char chr_3_index_queued;

const unsigned char y_collision_offsets[NUM_Y_COLLISION_OFFSETS] = { 1, 10, 19 };
const unsigned char x_collision_offsets[NUM_X_COLLISION_OFFSETS] = { 0, 12 };
unsigned char irq_array[IRQ_BUFFER_LEN*2];
unsigned char irq_cur_buffer_index;

#pragma bss-name(push, "XRAM")
// extra RAM at $6000-$7fff
//unsigned char wram_array[0x2000];
unsigned char save_version_validation[NUM_SAVE_VERSION_VALIDATION];
unsigned char current_room[MAX_ROOM_NUM_TILES];
unsigned char current_room_attr[(MAX_ROOM_NUM_TILES) / 4];
//unsigned char current_room_tiles[(240 * 4) * 3];
unsigned char cur_metatiles[META_TILE_SET_NUM_BYTES];
unsigned char checkpoint_room_index;
unsigned char checkpoint_spawn_id;
unsigned char game_option_list_values[OPTION_LIST_LEN];
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
	animation_data.anim_queued[in_working_anim_index] = next_anim_index;
}

void commit_next_anim()
{
	if (animation_data.anim_queued[in_working_anim_index] != 0xff && animation_data.anim_queued[in_working_anim_index] != animation_data.anim_current[in_working_anim_index])
	{
		animation_data.anim_current[in_working_anim_index] = animation_data.anim_queued[in_working_anim_index];
		animation_data.anim_frame[in_working_anim_index] = 0;
		animation_data.anim_ticks[in_working_anim_index] = 0;
	}

	animation_data.anim_queued[in_working_anim_index] = 0xff;
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



unsigned char get_chr_mode_0()
{
	switch(cur_room_metatile_index)
	{
		case 0:
		{
			return (0);
		}

		case 1:
		{
			return (24);
		}
	}	
}

void set_chr_bank_for_current_room()
{
	switch(cur_room_metatile_index)
	{
		case 0:
		{
			set_chr_mode_0(0);
			set_chr_mode_1(2);
			break;
		}

		case 1:
		{
			set_chr_mode_0(24);
			set_chr_mode_1(26);
			break;
		}
	}	
}

void bounce_player()
{
	if (player1.vel_x16 < 0)
	{
		player1.vel_x16 = 448; // 1.75
	}
	else
	{
		player1.vel_x16 = -448;
	}
	player1.vel_y16 = -FP_WHOLE_16(5); // FP_WHOLE(-5);
	dash_time = 0;
	dash_count = 0;

	// needed for the 'hitting an enemy case' as the "check for being grounded"
	// logic won't run again, until the player update, but there is a check
	// earyly in player update that zero's out x velocity if grounded.
	grounded = 0;
	
	SFX_PLAY_WRAPPER(15);
}