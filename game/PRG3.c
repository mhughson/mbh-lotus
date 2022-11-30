#include "PRG0.h"
#include "PRG1.h"
#include "PRG2.h"
#include "PRG3.h"
#include "main.h"
#include "LIB/neslib.h"
#include "LIB/nesdoug.h"
#include "mmc3\mmc3_code.h"
#include "../include/stdlib.h"

#pragma rodata-name ("BANK3")
#pragma code-name ("BANK3")

#define NUM_Y_COLLISION_OFFSETS_TD 3
const unsigned char y_collision_offsets_TD[NUM_Y_COLLISION_OFFSETS_TD] = { 1, 8, 15 };
#define NUM_X_COLLISION_OFFSETS_TD 2
const unsigned char x_collision_offsets_TD[NUM_X_COLLISION_OFFSETS_TD] = { 0, 16 };

#define WALK_SPEED_TD (FP_WHOLE(1))

void vram_buffer_load_column_td();
void vram_buffer_load_row_td();
void vram_buffer_load_inner_frame();

void update_player_td()
{
	static const unsigned int high_walk_speed = (WALK_SPEED_TD >> 16);

PROFILE_POKE(PROF_G);

	// high_x = high_2byte(player1.pos_x);
	// high_y = high_2byte(player1.pos_y);

	if (pad_all & PAD_LEFT && 
		//((cam.pos_x) / 256) <= (( high_2byte((player1.pos_x)) - (WALK_SPEED_TD >> 16)) / 256) && 
		high_2byte(player1.pos_x) - cam.pos_x >= (high_walk_speed + 8) &&
		player1.pos_x >= WALK_SPEED_TD + FP_WHOLE(8))
	{
		temp32 = player1.pos_x;

		// move the player left.
		player1.pos_x -= (unsigned long)WALK_SPEED_TD;// + FP_0_5;

		for (i = 0; i < NUM_Y_COLLISION_OFFSETS_TD; ++i)
		{
			// take the player position, offset it a little
			// so that the arms overlap the wall a bit (+8),
			// convert to metatile space (16x16) (>>4).
			// The position is stored using fixed point math, 
			// where the high byte is the whole part, and the
			// low byte is the fraction.
			x = (high_2byte(player1.pos_x) + x_collision_offsets_TD[0] - 1) >> 4;

			// Player is 24 pixels high, so +12 checks middle of them.
			// >>4 to put the position into metatile-space (16x16 chunks).
			y = (high_2byte(player1.pos_y) + y_collision_offsets_TD[i]) >> 4;

			// Prevent player for colliding with data outside the map, but still
			// allow them to travel above (and below) the top of the map.
			// This hard coded 15 will obviously need to change if we add vertical
			// levels.
			//if (y < 15)
			{
				// Convert the x,y into an index into the room data array.
				index16 = GRID_XY_TO_ROOM_INDEX(x, y);

				tempFlags = GET_META_TILE_FLAGS(index16);

				// Check if that point is in a solid metatile
				if (tempFlags & FLAG_SOLID)
				{
					// Hit a wall, shift back to the edge of the wall.
					player1.pos_x = temp32;
					break;
				}
			}
		}
	}
	// Is the right side of the sprite, after walking, going to be passed the end of the map?
	if (pad_all & PAD_RIGHT && (player1.pos_x + WALK_SPEED_TD + FP_WHOLE(16) ) <= FP_WHOLE(cur_room_width_pixels))
	{

		temp32 = player1.pos_x;
		player1.pos_x += WALK_SPEED_TD;// + FP_0_5;

		for (i = 0; i < NUM_Y_COLLISION_OFFSETS_TD; ++i)
		{
			// take the player position, offset it a little
			// so that the arms overlap the wall a bit (+16),
			// convert to metatile space (16x16) (>>4).
			// The position is stored using fixed point math, 
			// where the high byte is the whole part, and the
			// low byte is the fraction.
			x = (high_2byte(player1.pos_x) + x_collision_offsets_TD[NUM_X_COLLISION_OFFSETS_TD - 1] + 1) >> 4;

			// Player is 24 pixels high, so +12 checks middle of them.
			// >>4 to put the position into metatile-space (16x16 chunks).
			y = (high_2byte(player1.pos_y) + y_collision_offsets_TD[i]) >> 4;

			//if (y < 15)
			{
				// Convert the x,y into an index into the room data array.
				//temp16 = GRID_XY_TO_ROOM_NUMBER(x, y);
				index16 = GRID_XY_TO_ROOM_INDEX(x, y);

				tempFlags = GET_META_TILE_FLAGS(index16);

				// Check if that point is in a solid metatile.
				// Treat spikes like walls if the player is on the floor. It feels lame to 
				// walk into a spike and die; you should need to fall into them.
				if (tempFlags & FLAG_SOLID)
				{
					// Hit a wall, shift back to the edge of the wall.
					player1.pos_x = temp32; //(unsigned long)((x << 4) - 17) << HALF_POS_BIT_COUNT;

					break;
				}
			}
		}
	}

	if (pad_all & PAD_UP &&
		player1.pos_y >= WALK_SPEED_TD)
	{
		
		temp32 = player1.pos_y;
		player1.pos_y -= WALK_SPEED_TD;// + FP_0_5;

		for (i = 0; i < NUM_X_COLLISION_OFFSETS_TD; ++i)
		{
			x = (high_2byte(player1.pos_x) + x_collision_offsets_TD[i]) >> 4;
			y = (high_2byte(player1.pos_y)) >> 4; // head
			//if (y < 15)
			{
				index16 = GRID_XY_TO_ROOM_INDEX(x, y);
				if (GET_META_TILE_FLAGS(index16) & FLAG_SOLID)
				{
					player1.pos_y = temp32;
					player1.vel_y = 0;
					break;
				}
			}
		}					
	}

	if (pad_all & PAD_DOWN &&
		(player1.pos_y + WALK_SPEED_TD + FP_WHOLE(y_collision_offsets_TD[2]) ) <= FP_WHOLE(cur_room_height_pixels))
	{
		temp32 = player1.pos_y;
		player1.pos_y += WALK_SPEED_TD;// + FP_0_5;

		for (i = 0; i < NUM_X_COLLISION_OFFSETS_TD; ++i)
		{
			x = (high_2byte(player1.pos_x) + x_collision_offsets_TD[i]) >> 4;
			y = (high_2byte(player1.pos_y) + y_collision_offsets_TD[2]) >> 4; // foot
			//if (y < 15)
			{
				index16 = GRID_XY_TO_ROOM_INDEX(x, y);
				if (GET_META_TILE_FLAGS(index16) & FLAG_SOLID)
				{
					player1.pos_y = temp32;
					player1.vel_y = 0;
					break;
				}
			}
		}		
	}

	if (pad_all & PAD_RIGHT)
	{
		player1.facing_left = 1;
		anim_index = ANIM_WALK_RIGHT_TD;
	}
	else if (pad_all & PAD_LEFT)
	{
		player1.facing_left = 3;
		anim_index = ANIM_WALK_LEFT_TD;
	}
	else if (pad_all & PAD_UP)
	{
		player1.facing_left = 2;
		anim_index = ANIM_WALK_UP_TD;
	}
	else if (pad_all & PAD_DOWN)
	{
		player1.facing_left = 0;
		anim_index = ANIM_WALK_DOWN_TD;
	}
	else
	{
		anim_index = ANIM_IDLE_DOWN_TD;
	}

	global_working_anim = &player1.sprite.anim;
	queue_next_anim(anim_index);
	commit_next_anim();

	//draw_player();

PROFILE_POKE(PROF_R);	
}

void update_cam_td()
{
	unsigned int old_cam_x = cam.pos_x;
	unsigned int old_cam_y = cam.pos_y;

#define CAM_DEAD_ZONE 16

	// move the camera to the player if needed.
	if (high_2byte(player1.pos_x) > cam.pos_x + (128 + CAM_DEAD_ZONE) && cam.pos_x < cur_room_width_pixels-256)
	{
		cam.pos_x = MIN(cur_room_width_pixels-256, high_2byte(player1.pos_x) - (128 + CAM_DEAD_ZONE));
	}
	else if (high_2byte(player1.pos_x) < cam.pos_x + (128 - CAM_DEAD_ZONE) && cam.pos_x > 0 ) //(cam.pos_x / 256) == ((high_2byte(player1.pos_x) - 64) / 256))
	{
		// TODO: There is probably a chance the rolls over to -1 if the player doesn't start at 0,0. Can't clamp
		//		 because pos_x is unsigned. Need to detect roll over and clamp to 0.
		old_cam_x = cam.pos_x;
		cam.pos_x = high_2byte(player1.pos_x) - (128 - CAM_DEAD_ZONE);
		if (old_cam_x < cam.pos_x) cam.pos_x = 0;
	}

	if (high_2byte(player1.pos_y) > cam.pos_y + (120 + CAM_DEAD_ZONE) && cam.pos_y < cur_room_height_pixels-240)
	{
		cam.pos_y = MIN(cur_room_height_pixels-240, high_2byte(player1.pos_y) - (120 + CAM_DEAD_ZONE));
	}
	else if (high_2byte(player1.pos_y) < cam.pos_y + (120 - CAM_DEAD_ZONE) && cam.pos_y > 0 ) //(cam.pos_x / 256) == ((high_2byte(player1.pos_x) - 64) / 256))
	{
		// TODO: There is probably a chance the rolls over to -1 if the player doesn't start at 0,0. Can't clamp
		//		 because pos_x is unsigned. Need to detect roll over and clamp to 0.
		old_cam_y = cam.pos_y;
		cam.pos_y = high_2byte(player1.pos_y) - (120 - CAM_DEAD_ZONE);
		if (old_cam_y < cam.pos_y) cam.pos_y = 0;
	}

	x = old_cam_x;
	y = cam.pos_x;


	vram_buffer_load_inner_frame();

#if 0
	// This should really be >> 4 (every 16 pixels) but that will miss the initial
	// row loading. Could update "load_map" but the nametable logic is kind of annoying
	// for the non-vram version. Will dig in more later.
	if (((old_cam_x + 7) >> 3) < ((cam.pos_x + 7) >> 3) || cur_nametable_y_right != 0)
	{
		in_x_tile = (cam.pos_x + 256) / 16;
		in_x_pixel = (cam.pos_x + 256);

		in_y_tile =((cam.pos_y) / 16);
		in_y_pixel = (cam.pos_y);

		cur_nametable_y = cur_nametable_y_right;
		vram_buffer_load_column_td();

		cur_nametable_y_right += out_num_tiles;
		if (cur_nametable_y_right >= 30) cur_nametable_y_right = 0;					
	}
	if ((old_cam_x >> 3) > (cam.pos_x >> 3) || cur_nametable_y_left != 0)
	{
		in_x_tile = (cam.pos_x) / 16;
		in_x_pixel = (cam.pos_x);

		in_y_tile =((cam.pos_y) / 16);
		in_y_pixel = (cam.pos_y);
		
		cur_nametable_y = cur_nametable_y_left;
		vram_buffer_load_column_td();

		cur_nametable_y_left += out_num_tiles;
		if (cur_nametable_y_left >= 30) cur_nametable_y_left = 0;		
	}

	if (((old_cam_y + 7) >> 3) < ((cam.pos_y + 7) >> 3) || cur_nametable_x_bottom != 0)
	{

		in_x_tile = (cam.pos_x) / 16;
		in_x_pixel = (cam.pos_x);
		in_y_tile =((cam.pos_y + 232) / 16);
		in_y_pixel = (cam.pos_y + 232);
		cur_nametable_x = cur_nametable_x_bottom;
		vram_buffer_load_row_td();

		in_y_tile =((cam.pos_y + 232 - 8) / 16);
		in_y_pixel = (cam.pos_y + 232 - 8);
		cur_nametable_x = cur_nametable_x_top;
		vram_buffer_load_row_td();								

		cur_nametable_x_bottom += out_num_tiles;
		if (cur_nametable_x_bottom >= 34) cur_nametable_x_bottom = 0;	

		// done inside load_row for now
		// cur_nametable_x_bottom += NAMETABLE_TILES_8_UPDATED_PER_FRAME;
		// if (cur_nametable_x_bottom >= 32) cur_nametable_x_bottom = 0;
	}

	if (((old_cam_y) >> 3) > ((cam.pos_y) >> 3) || cur_nametable_x_top != 0)
	{

		in_x_tile = (cam.pos_x) / 16;
		in_x_pixel = (cam.pos_x);
		in_y_tile =((cam.pos_y + 8) / 16);
		in_y_pixel = (cam.pos_y) + 8;
		cur_nametable_x = cur_nametable_x_top;
		vram_buffer_load_row_td();

		in_y_tile =((cam.pos_y) / 16);
		in_y_pixel = (cam.pos_y);
		cur_nametable_x = cur_nametable_x_top;
		vram_buffer_load_row_td();					

		cur_nametable_x_top += out_num_tiles;
		if (cur_nametable_x_top >= 34) cur_nametable_x_top = 0;	
	}							

	// if (pad_all & PAD_START)
	// {
	// 	in_x_tile = (cam.pos_x) / 16;
	// 	in_x_pixel = (cam.pos_x);
	// 	cur_nametable_y = 0;
	// 	//cur_nametable_y = cur_nametable_y_left;
	// 	vram_buffer_load_row();
	// }

#endif //0

	// if top down
	banked_call(BANK_1, draw_player_td);
	// else
	//banked_call(BANK_1, draw_player);

	// cur_col is the last column to be loaded, aka the right
	// hand side of the screen. The scroll amount is relative to the 
	// left hand side of the screen, so offset by 256.
	//set_scroll_x(cam.pos_x);
	//set_scroll_y(cam.pos_y);

	scroll(cam.pos_x, cam.pos_y % 480);
}

void vram_buffer_load_column_td()
{
	// TODO: Remove int is a significant perf improvment.
	static unsigned char local_x;
	static unsigned char local_y;
	static unsigned char local_i;
	static unsigned int local_index16;
	static unsigned int local_att_index16;
	static unsigned char nametable_index;
	static unsigned char tile_offset;
	static unsigned char tile_offset2;

	// Track the tile that was the starting point for this update pass.
	static unsigned int starting_y_tile;

	static unsigned char array_temp8;

	nametable_index = (in_x_tile / 16) % 2;

	// TILES

PROFILE_POKE(PROF_G)

	tile_offset = (in_x_pixel % 16) / 8;
	tile_offset2 = tile_offset+2;

	// Get the index of the tile based on the fixed x starting tile, and the 
	// time sliced y tile. cur_nametable_y is in 8x8 tiles, so /2 puts it into
	// 16/16 metatiles.
	local_index16 = GRID_XY_TO_ROOM_INDEX(in_x_tile, in_y_tile + (cur_nametable_y/2));

	// We will need the starting tile in order to calculate if we have gone over
	// a full screen height of data, to avoid wrapping back to the top.
	starting_y_tile = in_y_tile + (cur_nametable_y/2);

	for (local_i = 0; local_i < NAMETABLE_TILES_8_UPDATED_PER_FRAME_TD; )
	{
		local_att_index16 = current_room[local_index16] * META_TILE_NUM_BYTES;

		// single column of tiles
		array_temp8 = cur_metatiles[local_att_index16 + tile_offset];
		nametable_col[local_i] = array_temp8;
		local_i++;
		array_temp8 = cur_metatiles[local_att_index16 + tile_offset2];
		nametable_col[local_i] = array_temp8;
		local_i++;

		local_index16 += cur_room_width_tiles;
	}

	// The screen is only 30 tiles high, which doesn't always divide nicely into out metatile updates, so map sure we don't
	// update beyond the bottom of the screen.
	//array_temp8 = MIN(NAMETABLE_TILES_8_UPDATED_PER_FRAME, 30 - cur_nametable_y);

	// Only update to the edge of the nametable to avoid writing into the attribute table data.
	out_num_tiles = MIN(NAMETABLE_TILES_8_UPDATED_PER_FRAME_TD, (15 - ((starting_y_tile) % 15)) * 2 );
	// Also, make sure that we only write 30 tiles total. We could do more if the "updates per frame"
	// doesn't divide evenly into the number of meta tiles.
	out_num_tiles = MIN(out_num_tiles, 30 - (cur_nametable_y));

	multi_vram_buffer_vert(nametable_col, (unsigned char)out_num_tiles, get_ppu_addr(nametable_index, in_x_pixel, (((in_y_pixel / 16) * 16) + (cur_nametable_y * 8)) % 240));

// Not handling attributes yet.
return;

	// ATTRIBUTES

PROFILE_POKE(PROF_B)

	// Attributes are in 2x2 meta tile chunks, so we need to round down to the nearest,
	// multiple of 2 (eg. if you pass in index 5, we want to start on 4).
	local_x = (in_x_tile & 0xFFFE);//local_x = (in_x_tile / 2) * 2;
	//local_x = (in_x_tile / 2) * 2;

	for (local_y = 0; local_y < (NAMETABLE_ATTRIBUTES_16_UPDATED_PER_FRAME); local_y+=2)
	{
		local_i = 0;

		// room index.
		local_index16 = GRID_XY_TO_ROOM_INDEX(local_x, local_y + (cur_nametable_y / 2));
		// meta tile palette index.
		local_att_index16 = (current_room[local_index16] * META_TILE_NUM_BYTES);
		local_att_index16+=4;
		// bit shift amount
		local_i |= (cur_metatiles[local_att_index16]);

		local_index16++;//local_index16 = local_index16 + 1; //(local_y * 16) + (local_x + 1);
		local_att_index16 = (current_room[local_index16] * META_TILE_NUM_BYTES);
		local_att_index16+=4;
		local_i |= (cur_metatiles[local_att_index16]) << 2;

		local_index16 = local_index16 + cur_room_width_tiles; //((local_y + 1) * 16) + (local_x);
		local_index16--;
		local_att_index16 = (current_room[local_index16] * META_TILE_NUM_BYTES);
		local_att_index16+=4;
		local_i |= (cur_metatiles[local_att_index16]) << 4;

		local_index16++;// = local_index16 + 1; //((local_y + 1) * 16) + (local_x + 1);
		local_att_index16 = (current_room[local_index16] * META_TILE_NUM_BYTES);
		local_att_index16+=4;
		local_i |= (cur_metatiles[local_att_index16]) << 6;	

		one_vram_buffer(local_i, get_at_addr(nametable_index, (local_x) * CELL_SIZE, ((local_y * CELL_SIZE) + (cur_nametable_y * 8))));
	}
PROFILE_POKE(PROF_W)
}

void vram_buffer_load_row_td()
{
	// TODO: Remove int is a significant perf improvment.
	static unsigned char local_x;
	static unsigned char local_y;
	static unsigned char local_i;
	static unsigned int local_index16;
	static unsigned int local_att_index16;
	static unsigned char nametable_index;
	static unsigned char tile_offset;
	static unsigned char tile_offset2;

	// Track which tile we started updating horizontally.
	static unsigned int starting_x_tile;

	static unsigned char array_temp8;

	// The nametable is figured out by calculating the starting X tile,
	// dividing that into the nametable width (16 meta tiles). Odd number
	// means nametable 1, even nametable 0.
	nametable_index = ((in_x_tile + (cur_nametable_x / 2)) / 16) % 2;

	// TILES

PROFILE_POKE(PROF_G)

	// Figure out which index in the metatile data will be drawn on the left.
	// Is it the top slice, or the bottom slice?
	// Tiles go:
	// 0 1
	// 2 3
	// So if we are in the top half, this will return 0 * 2
	// Bottom half it will be 1 * 2
	tile_offset = ((in_y_pixel % 16) / 8) * 2;
	// The 2nd tile is always 1 to the right of the first.
	tile_offset2 = tile_offset+1;

	// Get the index into the current_room table. This accounts for y scroll, but also
	// x scroll and the timesliced "cur_gametable_x" sliding update.
	local_index16 = GRID_XY_TO_ROOM_INDEX((in_x_tile + (cur_nametable_x / 2)), in_y_tile);

	// Save this for later so that we can figure out how many tiles we actually want to 
	// write out, to keep it within the visible window width.
	starting_x_tile = local_index16;

	for (local_i = 0; local_i < NAMETABLE_TILES_8_UPDATED_PER_FRAME_TD; )
	{
		local_att_index16 = current_room[local_index16] * META_TILE_NUM_BYTES;

		// single column of tiles
		array_temp8 = cur_metatiles[local_att_index16 + tile_offset];
		nametable_col[local_i] = array_temp8;
		local_i++;
		array_temp8 = cur_metatiles[local_att_index16 + tile_offset2];
		nametable_col[local_i] = array_temp8;
		local_i++;

		// next tile in the row.
		++local_index16;
	}

	// Stop at the end of nametables. The next update will continue from here but on the next
	// nametable.
	out_num_tiles = MIN(NAMETABLE_TILES_8_UPDATED_PER_FRAME, (16 - ((starting_x_tile) % 16)) * 2 );
	// TODO: What about going past the edge?

	// The VRAM for the screen goes up to 240 in the y before switching to attribute data. %240 to move
	// to the next nametable rather than writing to attribute data.
	multi_vram_buffer_horz(nametable_col, out_num_tiles, get_ppu_addr(nametable_index, (cur_nametable_x * 8) + ((in_x_pixel / 16) * 16), in_y_pixel % 240));

return;

	// ATTRIBUTES

PROFILE_POKE(PROF_B)

	// Attributes are in 2x2 meta tile chunks, so we need to round down to the nearest,
	// multiple of 2 (eg. if you pass in index 5, we want to start on 4).
	local_x = (in_x_tile & 0xFFFE);//local_x = (in_x_tile / 2) * 2;
	//local_x = (in_x_tile / 2) * 2;

	for (local_y = 0; local_y < (NAMETABLE_ATTRIBUTES_16_UPDATED_PER_FRAME); local_y+=2)
	{
		local_i = 0;

		// room index.
		local_index16 = GRID_XY_TO_ROOM_INDEX(local_x, local_y + (cur_nametable_y / 2));
		// meta tile palette index.
		local_att_index16 = (current_room[local_index16] * META_TILE_NUM_BYTES);
		local_att_index16+=4;
		// bit shift amount
		local_i |= (cur_metatiles[local_att_index16]);

		local_index16++;//local_index16 = local_index16 + 1; //(local_y * 16) + (local_x + 1);
		local_att_index16 = (current_room[local_index16] * META_TILE_NUM_BYTES);
		local_att_index16+=4;
		local_i |= (cur_metatiles[local_att_index16]) << 2;

		local_index16 = local_index16 + cur_room_width_tiles; //((local_y + 1) * 16) + (local_x);
		local_index16--;
		local_att_index16 = (current_room[local_index16] * META_TILE_NUM_BYTES);
		local_att_index16+=4;
		local_i |= (cur_metatiles[local_att_index16]) << 4;

		local_index16++;// = local_index16 + 1; //((local_y + 1) * 16) + (local_x + 1);
		local_att_index16 = (current_room[local_index16] * META_TILE_NUM_BYTES);
		local_att_index16+=4;
		local_i |= (cur_metatiles[local_att_index16]) << 6;	

		one_vram_buffer(local_i, get_at_addr(nametable_index, (local_x) * CELL_SIZE, ((local_y * CELL_SIZE) + (cur_nametable_y * 8))));
	}
PROFILE_POKE(PROF_W)
}

unsigned int cam_x;
unsigned int cam_y;
void draw_row()
{
	static unsigned int local_att_index16;
	static unsigned int local_index16;
	static unsigned char local_i;
	static unsigned char array_temp8;

	static unsigned char tile_offset;
	static unsigned char tile_offset2;

	static unsigned char dist_to_nt_edge;
	static unsigned char nametable_index;

	// Figure out which index in the metatile data will be drawn on the left.
	// Is it the top slice, or the bottom slice?
	// Tiles go:
	// 0 1
	// 2 3
	// So if we are in the top half, this will return 0 * 2
	// Bottom half it will be 1 * 2
	tile_offset = (((cam_y % 16) / 8) * 2);
	// The 2nd tile is always 1 to the right of the first.
	tile_offset2 = tile_offset+1;

	local_index16 = GRID_XY_TO_ROOM_INDEX(cam_x / 16, cam_y / 16);

	local_i = 0;

	nametable_index = (cam_x / 256) % 2;

	// We are half way through a meta-tile.
	if (cam_x % 16 >= 8)
	{
		local_att_index16 = current_room[local_index16] * META_TILE_NUM_BYTES;

		// Just the second tile and move on.
		array_temp8 = cur_metatiles[local_att_index16 + tile_offset2];
		nametable_col[local_i] = array_temp8;
		local_i++;

		// next tile in the row.
		++local_index16;		
	}

	dist_to_nt_edge = (256 - (cam_x % 256)) / 8;

	while (local_i < dist_to_nt_edge)
	{
		local_att_index16 = current_room[local_index16] * META_TILE_NUM_BYTES;

		// single column of tiles
		array_temp8 = cur_metatiles[local_att_index16 + tile_offset];
		nametable_col[local_i] = array_temp8;
		local_i++;
		array_temp8 = cur_metatiles[local_att_index16 + tile_offset2];
		nametable_col[local_i] = array_temp8;
		local_i++;

		// next tile in the row.
		++local_index16;
	}

	multi_vram_buffer_horz(nametable_col, local_i, get_ppu_addr(nametable_index, cam_x, cam_y % 240));

	// Stopped at a nametable edge. Do the second half.
	if (local_i < 32)
	{
		++nametable_index;
		cam_x = nametable_index * 256;
		dist_to_nt_edge = 32 - local_i;

		local_index16 = GRID_XY_TO_ROOM_INDEX(cam_x / 16, cam_y / 16);

		local_i = 0;

		// We are half way through a meta-tile.
		// if (cam_x % 16 >= 8)
		// {
		// 	local_att_index16 = current_room[local_index16] * META_TILE_NUM_BYTES;

		// 	// Just the second tile and move on.
		// 	array_temp8 = cur_metatiles[local_att_index16 + tile_offset2];
		// 	nametable_col[local_i] = array_temp8;
		// 	local_i++;

		// 	// next tile in the row.
		// 	++local_index16;		
		// }

		while (local_i < dist_to_nt_edge)
		{
			local_att_index16 = current_room[local_index16] * META_TILE_NUM_BYTES;

			// single column of tiles
			array_temp8 = cur_metatiles[local_att_index16 + tile_offset];
			nametable_col[local_i] = array_temp8;
			local_i++;

			// TODO: Check for going over edge here too if needed for
			// 		 case where we started on an odd tile.
			array_temp8 = cur_metatiles[local_att_index16 + tile_offset2];
			nametable_col[local_i] = array_temp8;
			local_i++;

			// next tile in the row.
			++local_index16;
		}

		multi_vram_buffer_horz(nametable_col, local_i, get_ppu_addr(nametable_index, cam_x, cam_y % 240));
	}
}

void draw_col()
{
	static unsigned int local_att_index16;
	static unsigned int local_index16;
	static unsigned char local_i;
	static unsigned char array_temp8;

	static unsigned char tile_offset;
	static unsigned char tile_offset2;

	static unsigned char dist_to_nt_edge;
	static unsigned char nametable_index;

	// Figure out which index in the metatile data will be drawn on the left.
	// Is it the left slice, or the right slice?
	// Tiles go:
	// 0 1
	// 2 3
	// So if we are in the left half, it returns 0
	// Right half it will be 1
	tile_offset = ((cam_x % 16) / 8);
	// The 2nd tile is always 1 below (+2).
	tile_offset2 = tile_offset+2;

	local_index16 = GRID_XY_TO_ROOM_INDEX(cam_x / 16, cam_y / 16);

	local_i = 0;

	nametable_index = ((cam_x / 256) % 2);

	// We are half way through a meta-tile.
	if (cam_y % 16 >= 8)
	{
		local_att_index16 = current_room[local_index16] * META_TILE_NUM_BYTES;

		// Just the second tile and move on.
		array_temp8 = cur_metatiles[local_att_index16 + tile_offset2];
		nametable_col[local_i] = array_temp8;
		local_i++;

		// next tile in the col.
		local_index16 += cur_room_width_tiles;
	}

	dist_to_nt_edge = (240 - (cam_y % 240)) / 8;

	while (local_i < dist_to_nt_edge)
	{
		local_att_index16 = current_room[local_index16] * META_TILE_NUM_BYTES;

		// single column of tiles
		array_temp8 = cur_metatiles[local_att_index16 + tile_offset];
		nametable_col[local_i] = array_temp8;
		local_i++;
		array_temp8 = cur_metatiles[local_att_index16 + tile_offset2];
		nametable_col[local_i] = array_temp8;
		local_i++;

		// next tile in the col.
		local_index16 += cur_room_width_tiles;
	}

	// TODO: Is %240 needed since it's capped above?
	multi_vram_buffer_vert(nametable_col, local_i, get_ppu_addr(nametable_index, cam_x, cam_y % 240));

	// Stopped at a nametable edge. Do the second half.
	if (local_i < 30)
	{
		nametable_index += 2;
		// Move the camera to the edge of the next nametable.
		cam_y += (240 - (cam_y % 240));
		dist_to_nt_edge = 30 - local_i;

		local_index16 = GRID_XY_TO_ROOM_INDEX(cam_x / 16, cam_y / 16);

		local_i = 0;

		// We are half way through a meta-tile.
		// if (cam_x % 16 >= 8)
		// {
		// 	local_att_index16 = current_room[local_index16] * META_TILE_NUM_BYTES;

		// 	// Just the second tile and move on.
		// 	array_temp8 = cur_metatiles[local_att_index16 + tile_offset2];
		// 	nametable_col[local_i] = array_temp8;
		// 	local_i++;

		// 	// next tile in the row.
		// 	++local_index16;		
		// }

		while (local_i < dist_to_nt_edge)
		{
			local_att_index16 = current_room[local_index16] * META_TILE_NUM_BYTES;

			// single row of tiles
			array_temp8 = cur_metatiles[local_att_index16 + tile_offset];
			nametable_col[local_i] = array_temp8;
			local_i++;

			// TODO: Check for going over edge here too if needed for
			// 		 case where we started on an odd tile.
			if (local_i < dist_to_nt_edge)
			{
				array_temp8 = cur_metatiles[local_att_index16 + tile_offset2];
				nametable_col[local_i] = array_temp8;
				local_i++;
			}

			// next tile in the row.
			local_index16 += cur_room_width_tiles;
		}

		multi_vram_buffer_vert(nametable_col, local_i, get_ppu_addr(nametable_index, cam_x, cam_y % 240));
	}
}

void vram_buffer_load_inner_frame()
{

	if (pad_all & PAD_UP)
	{
		cam_x = cam.pos_x;
		cam_y = cam.pos_y;
		draw_row();
	}
	else if (pad_all & PAD_DOWN)
	{
		cam_x = cam.pos_x;
		cam_y = cam.pos_y + 232;
		draw_row();
	}

	if (pad_all & PAD_LEFT)
	{
		cam_x = cam.pos_x - 8;
		cam_y = cam.pos_y;
		draw_col();
	}
	else if (pad_all & PAD_RIGHT)
	{
		cam_x = cam.pos_x + 256;
		cam_y = cam.pos_y;
		draw_col();
	}
}