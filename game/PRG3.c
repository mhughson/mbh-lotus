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

#define NUM_Y_COLLISION_OFFSETS_TD 2
const unsigned char y_collision_offsets_TD[NUM_Y_COLLISION_OFFSETS_TD] = { 8, 15 };
#define NUM_X_COLLISION_OFFSETS_TD 2
const unsigned char x_collision_offsets_TD[NUM_X_COLLISION_OFFSETS_TD] = { 5, 11 };

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
		//high_2byte(player1.pos_x) - cam.pos_x >= (high_walk_speed + 8) &&
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
			y = (high_2byte(player1.pos_y) + y_collision_offsets_TD[0]) >> 4; // head
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
		(player1.pos_y + WALK_SPEED_TD + FP_WHOLE(y_collision_offsets_TD[NUM_Y_COLLISION_OFFSETS_TD-1]) ) <= FP_WHOLE(cur_room_height_pixels))
	{
		temp32 = player1.pos_y;
		player1.pos_y += WALK_SPEED_TD;// + FP_0_5;

		for (i = 0; i < NUM_X_COLLISION_OFFSETS_TD; ++i)
		{
			x = (high_2byte(player1.pos_x) + x_collision_offsets_TD[i]) >> 4;
			y = (high_2byte(player1.pos_y) + y_collision_offsets_TD[NUM_Y_COLLISION_OFFSETS_TD-1]) >> 4; // foot
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
	old_cam_x = cam.pos_x;
	old_cam_y = cam.pos_y;

	cam.pos_x = high_2byte(player1.pos_x);
	if (cam.pos_x < 128) cam.pos_x = 0;
	else if (cam.pos_x > (cur_room_width_pixels - 128)) cam.pos_x = cur_room_width_pixels - 256;
	else cam.pos_x -= 128;
	cam.pos_y = high_2byte(player1.pos_y);
	if (cam.pos_y < 120) cam.pos_y = 0;
	else if (cam.pos_y > (cur_room_height_pixels - 120)) cam.pos_y = cur_room_height_pixels - 240;
	else cam.pos_y -= 120;	


	vram_buffer_load_inner_frame();

	banked_call(BANK_1, draw_player_td);


	scroll(cam.pos_x, cam.pos_y % 480);
}


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
	if (local_i < 36)
	{
		++nametable_index;
		cam_x = nametable_index * 256;
		dist_to_nt_edge = 36 - local_i;

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

void draw_row_attr()
{
	static unsigned int local_att_index16;
	static unsigned int local_index16;
	static unsigned char local_i;

	static unsigned char dist_to_nt_edge;
	static unsigned char nametable_index;

	static unsigned char local_x;
	static unsigned char local_y;

	static unsigned char cam_y_mod_240;

	//static unsigned char cam_x_mod_256;

	static unsigned int attr_address;

	cam_x = (cam_x / 32) * 32;
	//cam_y = (cam_y / 32) * 32;

	cam_y_mod_240 = cam_y % 240;

	// Is the camera in the 2nd half of a 32x32 attribute entry?
	// If so, we need to adjust the camera so that the top most
	// meta tile visible to the "camera" is also the top of an attribute
	// cluster.
	if ((cam_y_mod_240 % 32) >= 16)
	{
		// Clamp the camera back to the closest 32x32 cluster.
		cam_y = ((cam_y) / 32) * 32;

		// However, the screen is not divisible by 32 (it's 240) which means
		// for every other vertical screen, the logic for determining if a 
		// meta tile is the top or bottom of a attribute cluster flips!
		if (cam_y % 480 >= 240)
		{
			// odd screen. The first meta tile is going to be offset by 16 pixels
			cam_y -= 16;
		}

		// Recalculate based on new camera position.
		cam_y_mod_240 = cam_y % 240;
	}

	nametable_index = ((cam_x / 256) % 2);

	// Attributes are in 2x2 meta tile chunks, so we need to round down to the nearest,
	// multiple of 2 (eg. if you pass in index 5, we want to start on 4).
	local_y = (cam_y / 16);

	

	dist_to_nt_edge = (256 - (cam_x % 256)) / 16;

	attr_address = get_at_addr(nametable_index, cam_x, cam_y % 240);

	for (local_x = 0; local_x < dist_to_nt_edge; local_x+=2)
	{
		local_i = 0;

		// room index.
		local_index16 = GRID_XY_TO_ROOM_INDEX(local_x + (cam_x / 16), local_y);
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

		one_vram_buffer(local_i, attr_address);
		++attr_address;
	}

	if (local_x < 17)
	{
		++nametable_index;
		// Move the camera to the edge of the next nametable.
		cam_x += (256 - (cam_x % 256));
		dist_to_nt_edge = 17 - local_x;

		attr_address = get_at_addr(nametable_index, cam_x, cam_y % 240);

		for (local_x = 0; local_x < dist_to_nt_edge; local_x+=2)
		{
			local_i = 0;

			// room index.
			local_index16 = GRID_XY_TO_ROOM_INDEX(local_x + (cam_x / 16), local_y);
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

			one_vram_buffer(local_i, attr_address);
			++attr_address;
		}
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

void draw_col_attr()
{
	static unsigned int local_att_index16;
	static unsigned int local_index16;
	static unsigned char local_i;

	static unsigned char dist_to_nt_edge;
	static unsigned char nametable_index;

	static unsigned char local_x;
	static unsigned char local_y;

	static unsigned char cam_y_mod_240;
	static unsigned char is_bottom_half;

	static unsigned int attr_address;

	cam_x = (cam_x / 32) * 32;

	nametable_index = ((cam_x / 256) % 2);

	// % 240 then % 32 >= 16 detect second half

	// put camera in 32x32 space (to match attribute table). This allows the following
	// logic to all assume that the tiles are coming in at correct location.
	//cam_y = ((cam_y) / 32) * 32;

	// Cache for quick reference.
	// cam_y is the top of the camera in world space.
	cam_y_mod_240 = cam_y % 240;

	// Is the camera in the 2nd half of a 32x32 attribute entry?
	// If so, we need to adjust the camera so that the top most
	// meta tile visible to the "camera" is also the top of an attribute
	// cluster.
	is_bottom_half = 0;
	if ((cam_y_mod_240 % 32) >= 16)
	{
		// Clamp the camera back to the closest 32x32 cluster.
		cam_y = ((cam_y) / 32) * 32;

		// However, the screen is not divisible by 32 (it's 240) which means
		// for every other vertical screen, the logic for determining if a 
		// meta tile is the top or bottom of a attribute cluster flips!
		if (cam_y % 480 >= 240)
		{
			// odd screen. The first meta tile is going to be offset by 16 pixels
			cam_y -= 16;
		}

		// Recalculate based on new camera position.
		cam_y_mod_240 = cam_y % 240;
	}

	// Attributes are in 2x2 meta tile chunks, so we need to round down to the nearest,
	// multiple of 2 (eg. if you pass in index 5, we want to start on 4).
	local_x = ((cam_x / 16) & 0xFFFE); // / 2) * 2;//local_x = (in_x_tile / 2) * 2;

	//dist_to_nt_edge = (256 - (cam_y % 256)) / 16;

	dist_to_nt_edge = (240 - ((cam_y / 16 * 16) % 240)) / 16;

	attr_address = get_at_addr(nametable_index, (local_x) * 16, cam_y_mod_240);

	for (local_y = 0; local_y < dist_to_nt_edge; local_y+=2)
	{
		local_i = 0;

		// room index.
		local_index16 = GRID_XY_TO_ROOM_INDEX(local_x, local_y + (cam_y / 16));
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

		one_vram_buffer(local_i, attr_address);
		attr_address += 8;
	}

	// TODO: I think we need to go further than 15.
	if (local_y < 16)
	{
		nametable_index += 2;
		// Move the camera to the edge of the next nametable.
		cam_y += (240 - (cam_y_mod_240));
		cam_y_mod_240 = cam_y % 240;
		dist_to_nt_edge = 16 - local_y;

		//local_index16 = GRID_XY_TO_ROOM_INDEX(cam_x / 16, cam_y / 16);

		attr_address = get_at_addr(nametable_index, (local_x) * 16, cam_y_mod_240);

		for (local_y = 0; local_y < dist_to_nt_edge; local_y+=2)
		{
			local_i = 0;

			// room index.
			local_index16 = GRID_XY_TO_ROOM_INDEX(local_x, local_y + (cam_y / 16));
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

			one_vram_buffer(local_i, attr_address);
			attr_address += 8;
		}
	}
}

void vram_buffer_load_inner_frame()
{

	if (pad_all & PAD_UP && ((old_cam_y / 8) != (cam.pos_y / 8)))
	{
		DEBUG_ASSERT(1 == 2)
		// cam_x = cam.pos_x;
		// cam_y = cam.pos_y;
		// draw_row();

		draw_queue[draw_queue_index] = QUEUE_DRAW_ROW;
		++draw_queue_index;
		draw_queue[draw_queue_index] = (cam.pos_x >= 16) ? cam.pos_x - 16 : 0;
		++draw_queue_index;
		draw_queue[draw_queue_index] = cam.pos_y;
		++draw_queue_index;
		++draw_queue_index;
		draw_queue_index = draw_queue_index % DRAW_QUEUE_SIZE;
		DEBUG_ASSERT(draw_queue_index != draw_dequeue_index);


		// cam_x = cam.pos_x;
		// cam_y = cam.pos_y;
		// draw_row_attr();

		draw_queue[draw_queue_index] = QUEUE_DRAW_ROW_ATTR;
		++draw_queue_index;
		draw_queue[draw_queue_index] = cam.pos_x;
		++draw_queue_index;
		draw_queue[draw_queue_index] = cam.pos_y;
		++draw_queue_index;
		++draw_queue_index;
		draw_queue_index = draw_queue_index % DRAW_QUEUE_SIZE;
		DEBUG_ASSERT(draw_queue_index != draw_dequeue_index);
	}
	else if (pad_all & PAD_DOWN && (((old_cam_y + 232) / 8) != ((cam.pos_y + 232) / 8)))
	{
		// cam_x = cam.pos_x;
		// cam_y = cam.pos_y + 232;
		// draw_row();

		draw_queue[draw_queue_index] = QUEUE_DRAW_ROW;
		++draw_queue_index;
		draw_queue[draw_queue_index] = (cam.pos_x >= 16) ? cam.pos_x - 16 : 0;;
		++draw_queue_index;
		draw_queue[draw_queue_index] = cam.pos_y + 232;
		++draw_queue_index;
		++draw_queue_index;
		draw_queue_index = draw_queue_index % DRAW_QUEUE_SIZE;

		DEBUG_ASSERT(draw_queue_index != draw_dequeue_index);

		// cam_x = cam.pos_x;
		// cam_y = cam.pos_y + 239;
		// draw_row_attr();

		draw_queue[draw_queue_index] = QUEUE_DRAW_ROW_ATTR;
		++draw_queue_index;
		draw_queue[draw_queue_index] = cam.pos_x;
		++draw_queue_index;
		draw_queue[draw_queue_index] = cam.pos_y + 239;
		++draw_queue_index;
		++draw_queue_index;
		draw_queue_index = draw_queue_index % DRAW_QUEUE_SIZE;

		DEBUG_ASSERT(draw_queue_index != draw_dequeue_index);
	}

	if (pad_all & PAD_LEFT && (((old_cam_x - 8) / 8) != ((cam.pos_x - 8) / 8)))
	{
		// cam_x = cam.pos_x - 8;
		// cam_y = cam.pos_y;
		// draw_col();

		draw_queue[draw_queue_index] = QUEUE_DRAW_COL;
		++draw_queue_index;
		draw_queue[draw_queue_index] = cam.pos_x - 8;
		++draw_queue_index;
		draw_queue[draw_queue_index] = cam.pos_y;
		++draw_queue_index;
		++draw_queue_index;
		draw_queue_index = draw_queue_index % DRAW_QUEUE_SIZE;
		DEBUG_ASSERT(draw_queue_index != draw_dequeue_index);

		// cam_x = cam.pos_x - 8;
		// cam_y = cam.pos_y;
		// draw_col_attr();

		draw_queue[draw_queue_index] = QUEUE_DRAW_COL_ATTR;
		++draw_queue_index;
		draw_queue[draw_queue_index] = cam.pos_x - 8;
		++draw_queue_index;
		draw_queue[draw_queue_index] = cam.pos_y;
		++draw_queue_index;
		++draw_queue_index;
		draw_queue_index = draw_queue_index % DRAW_QUEUE_SIZE;
		DEBUG_ASSERT(draw_queue_index != draw_dequeue_index);	
	}
	else if (pad_all & PAD_RIGHT && (((old_cam_x + 255) / 8) != ((cam.pos_x + 255) / 8)))
	{
		// cam_x = cam.pos_x + 256;
		// cam_y = cam.pos_y;
		// draw_col();


		draw_queue[draw_queue_index] = QUEUE_DRAW_COL;
		++draw_queue_index;
		draw_queue[draw_queue_index] = cam.pos_x + 255;
		++draw_queue_index;
		draw_queue[draw_queue_index] = cam.pos_y;
		++draw_queue_index;
		++draw_queue_index;
		draw_queue_index = draw_queue_index % DRAW_QUEUE_SIZE;
		DEBUG_ASSERT(draw_queue_index != draw_dequeue_index);

		// cam_x = cam.pos_x + 256;
		// cam_y = cam.pos_y;
		// draw_col_attr();

		draw_queue[draw_queue_index] = QUEUE_DRAW_COL_ATTR;
		++draw_queue_index;
		draw_queue[draw_queue_index] = cam.pos_x + 255;
		++draw_queue_index;
		draw_queue[draw_queue_index] = cam.pos_y;
		++draw_queue_index;
		++draw_queue_index;
		draw_queue_index = draw_queue_index % DRAW_QUEUE_SIZE;
		DEBUG_ASSERT(draw_queue_index != draw_dequeue_index);	
	}

	if (draw_dequeue_index != draw_queue_index)
	{
		switch (draw_queue[draw_dequeue_index])
		{
			case QUEUE_DRAW_COL:
			{
				++draw_dequeue_index;
				cam_x = draw_queue[draw_dequeue_index];
				++draw_dequeue_index;
				cam_y = draw_queue[draw_dequeue_index];
				++draw_dequeue_index;
				++draw_dequeue_index;
				draw_dequeue_index = draw_dequeue_index % DRAW_QUEUE_SIZE;
				draw_col();
				break;
			}
			case QUEUE_DRAW_ROW:
			{
				++draw_dequeue_index;
				cam_x = draw_queue[draw_dequeue_index];
				++draw_dequeue_index;
				cam_y = draw_queue[draw_dequeue_index];
				++draw_dequeue_index;
				++draw_dequeue_index;
				draw_dequeue_index = draw_dequeue_index % DRAW_QUEUE_SIZE;
				draw_row();
				break;
			}
			case QUEUE_DRAW_COL_ATTR:
			{
				++draw_dequeue_index;
				cam_x = draw_queue[draw_dequeue_index];
				++draw_dequeue_index;
				cam_y = draw_queue[draw_dequeue_index];
				++draw_dequeue_index;
				++draw_dequeue_index;
				draw_dequeue_index = draw_dequeue_index % DRAW_QUEUE_SIZE;
				draw_col_attr();
				break;				
			}
			case QUEUE_DRAW_ROW_ATTR:
			{
				++draw_dequeue_index;
				cam_x = draw_queue[draw_dequeue_index];
				++draw_dequeue_index;
				cam_y = draw_queue[draw_dequeue_index];
				++draw_dequeue_index;
				++draw_dequeue_index;
				draw_dequeue_index = draw_dequeue_index % DRAW_QUEUE_SIZE;
				draw_row_attr();
				break;				
			}
		
		default:
			break;
		}
	}

	
}