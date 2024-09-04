#include "schip.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>


/*
BINARY CODE NOTES:

-> binary mask:
    new_var = (var & 0x00FF)
    var = 1101 1001 1010 1110
    new_bar = 0000 0000 1010 1110
This can be used to select the last 8 bit of a value and transform
it to a 8 bit variable

-> binary shift:
    new_var = var >> 8
    var = 1101 1001 1010 1110
    new_var = 0000 0000 1101 1001

    new_var = var >> 4
    var = 1101 1001 1010 1110
    new_bar = 0000 1101 1001 1010

-> binary mask (AND operator) + binary shift:
    new_var = (var & 0x0F00) >> 8
    var = 1101 1001 1010 1110
    (var & 0x0F00) = 0000 1001 0000 0000
    new_var = 0000 0000 0000 1001

This can select specific bits of a 16 bit value and transform it
into a 4 bit value (like an hex digit)

-> high/low byte:
    0x0FF0 (0000 1111 1111 0000)
    high byte = 0F (0000 1111)
    low byte = F0 (0000 1111)

CHIP INSTRUCTIONS application:

nnn or addr (12-bit, lowest 12 bits of instruction) -> instruction & 0x0FFF
n or nibble (4-bit, lowest 4 bits of instruction ) -> instruction & 0x000F
x (4 bit, lower 4 bits of high byte) -> (instruction & 0x0F00) >> 8
y (4 bit, upper 4 bits of low byte) -> (instruction & 0x00F0) >> 4
kk or byte (8-bit, lowest 8 bits of instruction) -> instruction & 0x00FF

*/
void OP_00E0(){
    memset(current_screen, 0, sizeof(current_screen));
    drawflag = 1;
}

void OP_00EE(){
    stack_pointer--;
    program_counter = stack[stack_pointer];
}

void OP_1nnn(){                                                 //Jump to location nnn.
    uint16_t addr = (op_code & 0x0FFF);                         //nnn -> Lowest 12bits of op
    program_counter = addr;
}

void OP_2nnn(){
    uint16_t addr = (op_code & 0x0FFF);
    stack[stack_pointer] = program_counter;
    stack_pointer++;
    program_counter = addr;
}

void OP_3xkk(){
    uint8_t Vx = (op_code & 0x0F00) >> 8;                           //shift binary to right 8 times(0000 0010 0000 0000 -> 0000 0000 0000 0010)
    uint8_t byte = (op_code & 0x00FF);                              //byte -> lowest 8bits of instruction

    if(registers[Vx] == byte) program_counter += 2;
}

void OP_4xkk(){
    uint8_t Vx = (op_code & 0x0F00) >> 8;
    uint8_t byte = op_code & 0x00FF;

    if (registers[Vx] != byte) program_counter += 2;
}


void OP_5xy0(){
    uint8_t Vx = (op_code & 0x0F00) >> 8;
    uint8_t Vy = (op_code & 0x00F0) >> 4;

    if (registers[Vx] == registers[Vy]) program_counter += 2;
}

void OP_6xkk(){
    uint8_t Vx = (op_code & 0x0F00) >> 8;
    uint8_t byte = op_code & 0x00FF;

    registers[Vx] = byte;
}

void OP_7xkk(){
    uint8_t Vx = (op_code & 0x0F00) >> 8;
    uint8_t byte = op_code & 0x00FF;

    registers[Vx] += byte;
}

void OP_8xy0(){
    uint8_t Vx = (op_code & 0x0F00) >> 8;
    uint8_t Vy = (op_code & 0x00F0) >> 4;

    registers[Vx] = registers[Vy];
}

/*
BITEWISE OR
    new_var = var1 | var2
    var1 = 1101 1001 1010 1110
    var2 = 0100 1101 0011 1111
    new_var = 1101 1101 1011 1111


*/
void OP_8xy1(){
    uint8_t Vx = (op_code & 0x0F00) >> 8;
    uint8_t Vy = (op_code & 0x00F0) >> 4;

    registers[Vx] |= registers[Vy];
}

void OP_8xy2(){
    uint8_t Vx = (op_code & 0x0F00) >> 8;
    uint8_t Vy = (op_code & 0x00F0) >> 4;

    registers[Vx] &= registers[Vy];
}

/*
BITEWISE XOR
    new_var = var1 ^ var2
    var1 = 1101 1001 1010 1110
    var2 = 0100 1101 0011 1111
    new_var = 1001 0101 1001 0001


*/
void OP_8xy3(){
    uint8_t Vx = (op_code & 0x0F00) >> 8;
    uint8_t Vy = (op_code & 0x00F0) >> 4;

    registers[Vx] ^= registers[Vy];
}

void OP_8xy4(){
    uint8_t Vx = (op_code & 0x0F00) >> 8;
    uint8_t Vy = (op_code & 0x00F0) >> 4;

    uint16_t total = registers[Vx] + registers[Vy];

    if (total > 255) registers[0xF] = 1;
    else registers[0xF] = 0;

    registers[Vx] = total & 0xFF;
}

void OP_8xy5(){
    uint8_t Vx = (op_code & 0x0F00) >> 8;
    uint8_t Vy = (op_code & 0x00F0) >> 4;

    if (registers[Vx] > registers[Vy]) registers[0xF] = 1;
    else registers[0xF] = 0;

    registers[Vx] -= registers[Vy];
}

void OP_8xy6(){
    uint8_t Vx = (op_code & 0x0F00) >> 8;

    registers[0xF] = (registers[Vx] & 0x1);
    registers[Vx] = registers[Vx] >> 1;
}

void OP_8xy7(){
    uint8_t Vx = (op_code & 0x0F00) >> 8;
    uint8_t Vy = (op_code & 0x00F0) >> 4;

    if (registers[Vy] > registers[Vx]) registers[0xF] = 1;
    else registers[0xF] = 0;

    registers[Vx] = registers[Vy] - registers[Vx];
}

void OP_8xyE(){
    uint8_t Vx = (op_code & 0x0F00) >> 8;

    registers[0xF] = (registers[Vx] & 0x80) >> 7;
    registers[Vx] <<= 1;
}

void OP_9xy0(){
    uint8_t Vx = (op_code & 0x0F00) >> 8;
    uint8_t Vy = (op_code & 0x00F0) >> 4;

    if(registers[Vx] != registers[Vy]) program_counter += 2;
}

void OP_Annn(){
    uint16_t addr = op_code & 0x0FFF;

    I_index = addr;
}

void OP_Bnnn(){
    uint16_t addr = op_code & 0x0FFF;

    program_counter = addr + registers[0x0];
}

void OP_Cxkk(){
    uint8_t Vx = (op_code & 0x0F00) >> 8;
    uint8_t kk = op_code & 0x00FF;

    uint8_t byte = rand() % (256);
    registers[Vx] = (byte & kk);
}


// VIDEO CODING
void OP_Dxyn(){
    uint8_t nibble = (op_code & 0x000F);                        //sprite height
    uint8_t Vx = (op_code & 0x0F00) >> 8;
    uint8_t Vy = (op_code & 0x00F0) >> 4;
    uint8_t pixel;

    uint8_t x_position = registers[Vx] % current_screen_length;
    uint8_t y_position = registers[Vy] % current_screen_height;

    registers[0xF] = 0;
    for (int row=0; row < nibble; row++){
        uint8_t sprite_byte = ram_memory[I_index+row];

        for (int column = 0; column < 8; column++){             //sprites are 8 bytes wide
            pixel = sprite_byte & (0x80 >> column);
            if (pixel){
                if (current_screen[x_position+column+(y_position+row)*current_screen_length] == 0xFFFFFFFF){
                    registers[0xF] = 1;
                }
                current_screen[x_position+column+(y_position+row)*current_screen_length] ^= 0xFFFFFFFF;
            }
        } 
    }
    drawflag = 1;

}

void OP_Ex9E(){
    uint8_t Vx = (op_code & 0x0F00) >> 8;
    
    uint8_t key = registers[Vx];
    if (keyboard[key]) program_counter += 2;
}

void OP_ExA1(){
    uint8_t Vx = (op_code & 0x0F00) >> 8;
    uint8_t key = registers[Vx];

    if (!keyboard[key]) program_counter += 2;
}

void OP_Fx07(){
    uint8_t Vx = (op_code & 0x0F00) >> 8;

    registers[Vx] = delay_timer;
}

void OP_Fx0A(){
    uint8_t Vx = (op_code & 0x0F00) >> 8;
    if (keyboard[0]) registers[Vx] = 0;
    else if (keyboard[1]) registers[Vx] = 1;
    else if (keyboard[2]) registers[Vx] = 2;
    else if (keyboard[3]) registers[Vx] = 3;
    else if (keyboard[4]) registers[Vx] = 4;
    else if (keyboard[5]) registers[Vx] = 5;
    else if (keyboard[6]) registers[Vx] = 6;
    else if (keyboard[7]) registers[Vx] = 7;
    else if (keyboard[8]) registers[Vx] = 8;
    else if (keyboard[9]) registers[Vx] = 9;
    else if (keyboard[10]) registers[Vx] = 10;
    else if (keyboard[11]) registers[Vx] = 11;
    else if (keyboard[12]) registers[Vx] = 12;
    else if (keyboard[13]) registers[Vx] = 13;
    else if (keyboard[14]) registers[Vx] = 14;
    else if (keyboard[15]) registers[Vx] = 15;
    else program_counter -= 2;                                      //jumps 2 instructions before == the same instruction repeats
}

void OP_Fx15(){
    uint8_t Vx = (op_code & 0x0F00) >> 8;

    delay_timer = registers[Vx];
}

void OP_Fx18(){
    uint8_t Vx = (op_code & 0x0F00) >> 8;

    sound_timer = registers[Vx];
} 

void OP_Fx1E(){
    uint8_t Vx = (op_code & 0x0F00) >> 8;

    I_index += registers[Vx];
}

void OP_Fx29(){
    uint8_t Vx = (op_code & 0x0F00) >> 8;

    I_index = SPRITE_START_ADDR + (registers[Vx] * 5);
}

void OP_Fx33(){
    uint8_t Vx = (op_code & 0x0F00) >> 8;
    uint8_t value = registers[Vx];

    ram_memory[I_index] = (value - (value % 100))/100;
    value = value - (ram_memory[I_index]*100);
    ram_memory[I_index + 1] = (value - (value % 10))/10;
    value = value - (ram_memory[I_index+1]*10);
    ram_memory[I_index + 2] = value;
}

void OP_Fx55(){
    uint8_t Vx = (op_code & 0x0F00) >> 8;

    for (int i = 0; i <= Vx; ++i) ram_memory[I_index+i] = registers[i];
}

void OP_Fx65(){
    uint8_t Vx = (op_code & 0x0F00) >> 8;

    for (int i = 0; i <= Vx; ++i) registers[i] = ram_memory[I_index + i];
}

void OP_NULL(){ fprintf(stderr, "Opcode error : %x\n", op_code);}

//Functions that call other functions inside the function table

void Function_0(){
    (*function_0[op_code & 0x00FF])();
}

void Function_8(){
    (*function_8[op_code & 0x000F])();
}

void Function_E(){
    (*function_E[op_code & 0x000F])();
}

void Function_F(){
    (*function_F[op_code & 0x00FF])();
}

void Chip8_Loop(){
    if (sound_timer > 0) sound_timer--;
    
    op_code = (ram_memory[program_counter] << 8) | ram_memory[program_counter+1];
    program_counter += 2;

    (*function_table[(op_code & 0xF000) >> 12])();
    
    
}

void Chip8_init(){
    current_screen_height = LOW_RES_HEIGHT;
    current_screen_length = LOW_RES_LENGTH;
    current_screen = low_res_screen;

    program_counter = PROGRAM_START_ADDR;
    for(int i = 0; i < SPRITE_SIZE; i++){                           //charge the fontset into memory
        ram_memory[SPRITE_START_ADDR + i] = fontSprites[i];
    }

    //Load funtions into tables as pointers
    function_table[0x0] = &Function_0;
    function_table[0x1] = &OP_1nnn;
    function_table[0x2] = &OP_2nnn;
    function_table[0x3] = &OP_3xkk;
    function_table[0x4] = &OP_4xkk;
    function_table[0x5] = &OP_5xy0;
    function_table[0x6] = &OP_6xkk;
    function_table[0x7] = &OP_7xkk;
    function_table[0x8] = &Function_8;
    function_table[0x9] = &OP_9xy0;
    function_table[0xA] = &OP_Annn;
    function_table[0xB] = &OP_Bnnn;
    function_table[0xC] = &OP_Cxkk;
    function_table[0xD] = &OP_Dxyn;
    function_table[0xE] = &Function_E;
    function_table[0xF] = &Function_F;

    function_0[0xE0] = &OP_00E0;
    for (uint8_t i=0x01; i < 0xEE; i++) function_0[i] = &OP_NULL;
    function_0[0xEE] = &OP_00EE;

    function_8[0x0] = &OP_8xy0;
    function_8[0x1] = &OP_8xy1;
    function_8[0x2] = &OP_8xy2;
    function_8[0x3] = &OP_8xy3;
    function_8[0x4] = &OP_8xy4;
    function_8[0x5] = &OP_8xy5;
    function_8[0x6] = &OP_8xy6;
    function_8[0x7] = &OP_8xy7;
    for (uint8_t i=0x8; i < 0xE; i++) function_8[i] = &OP_NULL;
    function_8[0xE] = &OP_8xyE;

    function_E[0x0] = &OP_NULL;
    function_E[0x1] = &OP_ExA1;
    for (uint8_t i=0x2; i<0xE; i++) function_E[i] = &OP_NULL;
    function_E[0xE] = &OP_Ex9E;

    for (uint8_t i=0x0; i<=0x65; i++) function_F[i] = &OP_NULL;
    function_F[0x07] = OP_Fx07;
    function_F[0x0A] = OP_Fx0A;
    function_F[0x15] = OP_Fx15;
    function_F[0x18] = OP_Fx18;
    function_F[0x1E] = OP_Fx1E;
    function_F[0x29] = OP_Fx29;
    function_F[0x33] = OP_Fx33;
    function_F[0x55] = OP_Fx55;
    function_F[0x65] = OP_Fx65;
    delay_timer = 0;
    sound_timer = 0;
}

// LOAD ROM TO MEMORY
void Load_ROM(char filename[]){
    FILE *rom_file = fopen(filename, "rb");                         //open file
    if (rom_file == NULL){                                          //chek if file exists
        fprintf(stderr,"Error handling file: %s", errno);
        fclose(rom_file);
        exit(EXIT_FAILURE);
    }
    int file_read_return_code = fseek(rom_file, 0L, SEEK_END);      //goto end of file
    if (file_read_return_code != 0){                                //check if file is not corrupt
        fprintf(stderr, "Error reading file: %s", errno);
        fclose(rom_file);
        exit(EXIT_FAILURE);
    }
    long file_size = ftell(rom_file);                               //get rom byte length
    char buffer[file_size];                                         //create a buffer of that length
    fseek(rom_file, 0L, SEEK_SET);                                  //go back to the start of the rom
    fread(buffer, file_size, 1, rom_file);                          //put rom data into buffer
    for(int i=0; i<file_size; i++){
        ram_memory[PROGRAM_START_ADDR+i] = buffer[i];
    }
}

void Window_Manager_Init(char *rom_file){
    window = SDL_CreateWindow(rom_file, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 320, SDL_WINDOW_SHOWN);
    if (window == NULL){
        fprintf(stderr, "Error with window creation");
        exit(3);
    }
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
        if (window == NULL){
        fprintf(stderr, "Error with renderer creation");
        exit(4);
    }

    SDL_RenderSetLogicalSize(renderer, current_screen_length, current_screen_height);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, current_screen_length, current_screen_height);
}

void CleanUp_Window(){
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

}

void Screen_Draw(){
    uint32_t pixels[current_screen_length*current_screen_height];

    if(drawflag){
        memset(pixels, 0, (62*32)*4);
        for(int x=0; x<current_screen_length; x++){
            for (int y=0; y<current_screen_height;y++){
                if (current_screen[x+(y*current_screen_length)] == 0xFFFFFFFF){
                    pixels[x+(y*current_screen_length)] = UINT32_MAX;
                }
            }
        }


    SDL_UpdateTexture(texture, NULL, pixels, current_screen_length*sizeof(uint32_t));
    SDL_Rect pos;
    pos.x = 0;
    pos.y = 0;

    pos.w = current_screen_length;
    pos.h = current_screen_height;
    SDL_RenderCopy(renderer, texture, NULL, &pos);
    SDL_RenderPresent(renderer);
    }
    drawflag = 0;

}

void print_state(int refresh_date){
    printf("----------STATE----------\n");
    printf("Refresh Date: %d\n\n", refresh_date);
    printf("Index: %d\nOpCode: %x\nProgram Counter: %x\nStack Pointer: %x\n", I_index, op_code, program_counter, stack_pointer);
    printf("----------STATE----------\n");
 
}

int main(int argc, char *argv[]){
    if (argc != 2){
        fprintf(stderr, "How to use: %s *ROM*", argv[0]);
        exit(EXIT_FAILURE);
    }
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0){
        fprintf(stderr, "ERROR SDL2 Init: %s\n", SDL_GetError());
        return 1;
    }
    char *rom_file = argv[1];

    SDL_Event e;

    Chip8_init();
    Window_Manager_Init(rom_file);
    Load_ROM(rom_file);



    uint32_t quit = 0;
    int refresh_rate = 5;
    while (!quit){
        while(SDL_PollEvent(&e))
        {
        switch (e.type)
        {
            case SDL_QUIT:
                quit = 1;
                break;
            case SDL_KEYDOWN:
            
                switch (e.key.keysym.sym)
                {
                case SDLK_ESCAPE:
                    quit = 1;
                    break;
                case SDLK_F6:
                    refresh_rate += 1;
                    break;
                case SDLK_F5:
                    refresh_rate -= 1;
                    break;
                case SDLK_F12:
                    print_state(refresh_rate);
                    break;
                case SDLK_1:
                    keyboard[1] = 1;
                    break;
                case SDLK_2:
                    keyboard[2] = 1;
                    break;
                case SDLK_3:
                    keyboard[3] = 1;
                    break;
                case SDLK_4:
                    keyboard[0xC] = 1;
                    break;
                case SDLK_q:
                    keyboard[4] = 1;
                    break;
                case SDLK_w:
                    keyboard[5] = 1;
                    break;
                case SDLK_e:
                    keyboard[6] = 1;
                    break;
                case SDLK_r:
                    keyboard[0xD] = 1;
                    break;
                case SDLK_a:
                    keyboard[7] = 1;
                    break;
                case SDLK_s:
                    keyboard[8] = 1;
                    break;
                case SDLK_d:
                    keyboard[9] = 1;
                    break;
                case SDLK_f:
                    keyboard[0xE] = 1;
                    break;
                case SDLK_z:
                    keyboard[0xA] = 1;
                    break;
                case SDLK_x:
                    keyboard[0] = 1;
                    break;
                case SDLK_c:
                    keyboard[0xB] = 1;
                    break;
                case SDLK_v:
                    keyboard[0xF] = 1;
                    break;
                }
            break;
            case SDL_KEYUP:
                switch (e.key.keysym.sym)
                {
                case SDLK_1:
                    keyboard[1] = 0;
                    break;
                case SDLK_2:
                    keyboard[2] = 0;
                    break;
                case SDLK_3:
                    keyboard[3] = 0;
                    break;
                case SDLK_4:
                    keyboard[0xC] = 0;
                    break;
                case SDLK_q:
                    keyboard[4] = 0;
                    break;
                case SDLK_w:
                    keyboard[5] = 0;
                    break;
                case SDLK_e:
                    keyboard[6] = 0;
                    break;
                case SDLK_r:
                    keyboard[0xD] = 0;
                    break;
                case SDLK_a:
                    keyboard[7] = 0;
                    break;
                case SDLK_s:
                    keyboard[8] = 0;
                    break;
                case SDLK_d:
                    keyboard[9] = 0;
                    break;
                case SDLK_f:
                    keyboard[0xE] = 0;
                    break;
                case SDLK_z:
                    keyboard[0xA] = 0;
                    break;
                case SDLK_x:
                    keyboard[0] = 0;
                    break;
                case SDLK_c:
                    keyboard[0xB] = 0;
                    break;
                case SDLK_v:
                    keyboard[0xF] = 0;
                    break;
                }            
            break;
        }
        break;
        }
        if (refresh_rate < 0) refresh_rate = 0;
        else SDL_Delay(refresh_rate);

        if (delay_timer > 0) delay_timer--;

        Chip8_Loop();
        
        Screen_Draw();
    }
 
    CleanUp_Window();
    return 0;
}