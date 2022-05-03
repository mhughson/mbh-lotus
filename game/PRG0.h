#ifndef D12B32D3_39DD_4865_9236_3AEDEF9ACEDE
#define D12B32D3_39DD_4865_9236_3AEDEF9ACEDE

// There should be no globals here; just function prototypes.
// Non constant (RAM) variables go in common header.
// Constant (ROM) variables go in this c file, so that they are no accessed by accident by other files.

void main_real();
unsigned char update_anim();
void load_current_map(unsigned int nt, unsigned char* _current_room);
void vram_buffer_load_2x2_metatile();
void vram_buffer_load_column();
void go_to_state(unsigned char new_state);

#endif /* D12B32D3_39DD_4865_9236_3AEDEF9ACEDE */
