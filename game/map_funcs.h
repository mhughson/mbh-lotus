#define GENERATE_MAP_FUNCS_PROTOTYPE(MAP_VAL) \
void copy_bg_to_current_room_ ## MAP_VAL (); \
void copy_original_room_to_current_ ## MAP_VAL (); \
void get_obj_id_ ## MAP_VAL (); \
void get_next_object_ ## MAP_VAL (); \
void get_cur_room_palettes_ ## MAP_VAL (); \
void get_cur_room_metatile_set_ ## MAP_VAL (); \
void get_cur_room_bit_flags_ ## MAP_VAL (); \
void get_cur_room_special_type_ ## MAP_VAL (); \
void get_cur_room_music_track_ ## MAP_VAL (); \
void get_cur_room_world_and_level_ ## MAP_VAL (); \
void get_cur_room_dimensions_ ## MAP_VAL ();


#define GENERATE_MAP_FUNCS(MAP_VAL) \
void copy_bg_to_current_room_ ## MAP_VAL () \
{ \
	/* NUM_CUSTOM_PROPS because the level data starts after the custom props */ \
	memcpy(current_room, &rooms_maps_ ## MAP_VAL [cur_room_index][NUM_CUSTOM_PROPS], cur_room_size_tiles); \
} \
 \
 \
void copy_original_room_to_current_ ## MAP_VAL () \
{ \
	static unsigned char tile_index_param_copy; \
 \
	tile_index_param_copy = tile_index_param; \
 \
	current_room[tile_index_param_copy] = rooms_maps_ ## MAP_VAL [cur_room_index][tile_index_param+NUM_CUSTOM_PROPS]; \
} \
 \
void get_obj_id_ ## MAP_VAL () \
{ \
	loaded_obj_index = rooms_maps_ ## MAP_VAL [cur_room_index][cur_room_size_tiles + in_obj_index + NUM_CUSTOM_PROPS]; \
	++in_obj_index; \
	loaded_obj_id = rooms_maps_ ## MAP_VAL [cur_room_index][cur_room_size_tiles + in_obj_index + NUM_CUSTOM_PROPS]; \
	++in_obj_index; \
} \
 \
void get_next_object_ ## MAP_VAL () \
{ \
	loaded_obj_id = rooms_maps_ ## MAP_VAL [cur_room_index][cur_room_size_tiles + in_obj_index + NUM_CUSTOM_PROPS]; \
	++in_obj_index; \
	loaded_obj_x = rooms_maps_ ## MAP_VAL [cur_room_index][cur_room_size_tiles + in_obj_index + NUM_CUSTOM_PROPS]; \
	++in_obj_index; \
	loaded_obj_y = rooms_maps_ ## MAP_VAL [cur_room_index][cur_room_size_tiles + in_obj_index + NUM_CUSTOM_PROPS]; \
	++in_obj_index; \
	loaded_obj_payload = rooms_maps_ ## MAP_VAL [cur_room_index][cur_room_size_tiles + in_obj_index + NUM_CUSTOM_PROPS]; \
	++in_obj_index; \
} \
 \
void get_cur_room_palettes_ ## MAP_VAL () \
{ \
	index = rooms_maps_ ## MAP_VAL [cur_room_index][0]; \
	index2 = rooms_maps_ ## MAP_VAL [cur_room_index][1]; \
} \
 \
void get_cur_room_metatile_set_ ## MAP_VAL () \
{ \
	cur_room_metatile_index = rooms_maps_ ## MAP_VAL [cur_room_index][2]; \
} \
 \
void get_cur_room_bit_flags_ ## MAP_VAL () \
{ \
	index = rooms_maps_ ## MAP_VAL [cur_room_index][3]; \
} \
 \
void get_cur_room_special_type_ ## MAP_VAL () \
{ \
	index = rooms_maps_ ## MAP_VAL [cur_room_index][4]; \
} \
 \
void get_cur_room_music_track_ ## MAP_VAL () \
{ \
	index = rooms_maps_ ## MAP_VAL [cur_room_index][5]; \
} \
 \
void get_cur_room_world_and_level_ ## MAP_VAL () \
{ \
	index = rooms_maps_ ## MAP_VAL [cur_room_index][6]; \
	index2 = rooms_maps_ ## MAP_VAL [cur_room_index][7]; \
} \
\
void get_cur_room_dimensions_ ## MAP_VAL () \
{ \
    index = rooms_maps_ ## MAP_VAL [cur_room_index][8]; \
	index2 = rooms_maps_ ## MAP_VAL [cur_room_index][9]; \
} \
