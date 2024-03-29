#ifndef BE4E8D12_51CB_4D34_A282_8D2D9C997AB4
#define BE4E8D12_51CB_4D34_A282_8D2D9C997AB4

// There should be no globals here; just function prototypes.
// Non constant (RAM) variables go in common header.
// Constant (ROM) variables go in this c file, so that they are no accessed by accident by other files.

void draw_player();
void draw_player_static();
void draw_player_td();
void draw_skeleton();
void draw_bird();

#endif /* BE4E8D12_51CB_4D34_A282_8D2D9C997AB4 */
