#ifndef A5B8504C_1172_4833_9D38_027FF7BA3DBB
#define A5B8504C_1172_4833_9D38_027FF7BA3DBB

// There should be no globals here; just function prototypes.
// Non constant (RAM) variables go in common header.
// Constant (ROM) variables go in this c file, so that they are no accessed by accident by other files.

// OUT: index   : cur world number
// OUT: index2  : cur sub level
void get_cur_room_world_and_level();

void stream_in_next_level();

// Copy over the metatile set to RAM.
void set_metatile_set();

#endif /* A5B8504C_1172_4833_9D38_027FF7BA3DBB */
