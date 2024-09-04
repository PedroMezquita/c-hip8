#include<stdint.h>
#include <SDL2/SDL.h>
#define PROGRAM_START_ADDR 0x200
#define SPRITE_START_ADDR 0x050
#define SPRITE_SIZE 80
#define LOW_RES_HEIGHT 32
#define LOW_RES_LENGTH 64
//SuperChip-48 instructions
#define HIGH_RES_LENGTH 128
#define HIGH_RES_HEIGHT 64



typedef void (*Chip8Action)(void);

uint8_t registers[16];
uint8_t ram_memory[4096];
uint16_t I_index;
uint32_t low_res_screen[LOW_RES_LENGTH*LOW_RES_HEIGHT]; //screen 
uint32_t high_res_screen[HIGH_RES_LENGTH*HIGH_RES_HEIGHT]; //SuperChip-48 high resolution mode
uint8_t keyboard[16];
uint8_t stack_pointer; //stack position address
uint8_t delay_timer;
uint8_t sound_timer;
uint16_t program_counter; //address of next instruction
uint16_t stack[16];
uint16_t op_code; //code of the instruction


uint8_t fontSprites[SPRITE_SIZE] = {     //Create hex table
    0xF0, 0x90, 0x90, 0x90, 0xF0, //0
    0x20, 0x60, 0x20, 0x20, 0x70, //1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, //2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, //3
    0x90, 0x90, 0xF0, 0x10, 0x10, //4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, //5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, //6
    0xF0, 0x10, 0x20, 0x40, 0x40, //7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, //8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, //9
    0xF0, 0x90, 0xF0, 0x90, 0x90, //A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, //B
    0xF0, 0x80, 0x80, 0x80, 0xF0, //C
    0xE0, 0x90, 0x90, 0x90, 0xE0, //D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, //E
    0xF0, 0x80, 0xF0, 0x80, 0x80  //F
};

Chip8Action function_table[0xF+1];
Chip8Action function_0[0xFF+1];         //0xE + 0x1 = 0xF
Chip8Action function_8[0xF];
Chip8Action function_E[0xF];
Chip8Action function_F[0x65+1];

//SDL variables
SDL_Window *window;
SDL_Renderer *renderer;
SDL_Texture *texture;

uint_fast8_t drawflag;

// SuperChip-48 new resolution
int current_screen_height;
int current_screen_length;
uint32_t *current_screen;