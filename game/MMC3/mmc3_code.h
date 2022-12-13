// Contains functions to help with working with multiple PRG/CHR banks
// For MMC3 code.


// format
// value < 0xf0, it's a scanline count
// zero is valid, it triggers an IRQ at the end of the current line

// if >= 0xf0...
// f0 = 2000 write, next byte is write value
// f1 = 2001 write, next byte is write value
// f2-f4 unused - future TODO ?
// f5 = 2005 write, next byte is H Scroll value
// f6 = 2006 write, next 2 bytes are write values

// f7 = change CHR mode 0, next byte is write value
// f8 = change CHR mode 1, next byte is write value
// f9 = change CHR mode 2, next byte is write value
// fa = change CHR mode 3, next byte is write value
// fb = change CHR mode 4, next byte is write value
// fc = change CHR mode 5, next byte is write value

// fd = very short wait, no following byte 
// fe = short wait, next byte is quick loop value
// (for fine tuning timing of things)

// ff = end of data set

// IRQ COMMANDS
#define IRQ_CHR_BANK(bank_num) (bank_num)
#define IRQ_SCANLINE(line_num) (line_num)
#define IRQ_WRITE_2000 (0xf0) 
#define IRQ_WRITE_2001 (0xf1) 
#define IRQ_H_SCROLL (0xf5)
#define IRQ_SET_PPU_ADDR (0xf6)
#define IRQ_CHR_MODE_0 (0xf7)
#define IRQ_CHR_MODE_1 (0xf8)
#define IRQ_CHR_MODE_2 (0xf9)
#define IRQ_CHR_MODE_3 (0xfa)
#define IRQ_CHR_MODE_4 (0xfb)
#define IRQ_CHR_MODE_5 (0xfc)
#define IRQ_END (0xff)

// Maximum level of recursion to allow with banked_call and similar functions.
#define MAX_BANK_DEPTH 10

extern unsigned char bankLevel;
extern unsigned char bankBuffer[MAX_BANK_DEPTH];




// Switch to another bank and call this function.
// Note: Using banked_call to call a second function from within  
// another banked_call is safe. This will break if you nest more  
// than 10 calls deep.
void banked_call(unsigned char bankId, void (*method)(void));


// Internal function used by banked_call(), don't call this directly.
// Switch to the given bank, and keep track of the current bank, so 
// that we may jump back to it as needed.
void bank_push(unsigned char bankId);


// Internal function used by banked_call(), don't call this directly.
// Go back to the last bank pushed on using bank_push.
void bank_pop(void);






// Switch to the given bank (at $8000-9fff). Your prior bank is not saved.
// Can be used for reading data with a function in the fixed bank.
// bank_id: The bank to switch to.
void __fastcall__ set_prg_8000(unsigned char bank_id);


// Get the current PRG bank at $8000-9fff.
// returns: The current bank.
unsigned char __fastcall__ get_prg_8000(void);


// WARNING, DON'T USE THIS IN THE CURRENT CFG, unless you
// really know what you're doing. El Crasho.
// Switch to the given bank (at $a000-bfff). Your prior bank is not saved.
// bank_id: The bank to switch to.
void __fastcall__ set_prg_a000(unsigned char bank_id);


// Changes a portion of the tilsets
// The plan was to NOT use these. Use the irq system instead
// You can, but make sure the IRQ system isn't changing CHR.
// See notes in README
void __fastcall__ set_chr_mode_0(unsigned char chr_id);
void __fastcall__ set_chr_mode_1(unsigned char chr_id);
void __fastcall__ set_chr_mode_2(unsigned char chr_id);
void __fastcall__ set_chr_mode_3(unsigned char chr_id);
void __fastcall__ set_chr_mode_4(unsigned char chr_id);
void __fastcall__ set_chr_mode_5(unsigned char chr_id);





#define MIRROR_VERTICAL 0
#define MIRROR_HORIZONTAL 1

// Set the current mirroring mode. Your options are 
// MIRROR_HORIZONTAL, and MIRROR_VERTICAL.
void __fastcall__ set_mirroring(unsigned char mirroring);


#define WRAM_OFF 0x40
#define WRAM_ON 0x80
#define WRAM_READ_ONLY 0xC0

// Set the WRAM mode. Your options are 
// WRAM_OFF, WRAM_ON, and WRAM_READ_ONLY.
// May not work in some emulators. Init code turns it ON.
void __fastcall__ set_wram_mode(unsigned char mode);


// Turns off MMC3 irqs, and changes the array pointer
// to point to a default 0xff
void disable_irq(void);


// This points an array to the IRQ system 
// Also turns ON the system
void set_irq_ptr(char * address);


// Check if it's safe to write to the irq array
// returns 0xff if done, zero if not done
// if the irq pointer is pointing to 0xff it is
// safe to edit.
unsigned char is_irq_done(void);














