//
// Created by kashyab on 12/25/23.
//
#include <stdio.h>
#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

// Constants
#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 640
#define MEMORY_SIZE 4096
#define GENERAL_PURPOSE_REGS 16
#define STACK_DEPTH 16
#define NUM_KEYS 16
#define VIEWPORT_WIDTH 64
#define VIEWPORT_HEIGHT 32
#define SPRITE_DATA_MEM_START 0
// Amount of space reserved for CHIP8 programs and data according to memory diagram
#define MAX_PROG_SIZE 133 // bytes

struct chip8 {
    uint8_t mmap[MEMORY_SIZE], v[GENERAL_PURPOSE_REGS], keyboard[NUM_KEYS], display[
            VIEWPORT_WIDTH][VIEWPORT_HEIGHT], sound_timer, delay_timer, sp;
    uint16_t pc, stack[STACK_DEPTH], opcode, index;
};

typedef void (*instruction)(struct chip8*);

void op_sys(struct chip8 *chip_ate); // SYS instruction -> ignore
void op_cls(struct chip8 *chip_ate); // CLS instruction -> clears the display
void op_ret(struct chip8 *chip_ate); // RET instruction -> return from subroutine
void op_jp_nnn(struct chip8 *chip_ate); // JP instruction -> jump to location nnn
void op_jp_nnn_v0(struct chip8 *chip_ate); // JP instruction -> jump to location nnn + v0
void op_call(struct chip8 *chip_ate); // CALL instruction -> call subroutine at nnn
void op_skip_eq_kk(struct chip8 *chip_ate); // SKIP EQ instruction -> skips next instruction if Vx = kk
void op_skip_eq_vy(struct chip8 *chip_ate); // SKIP EQ instruction -> skips next instruction if Vx = Vy
void op_skip_eq_kb(struct chip8 *chip_ate); // SKIP EQ instruction -> skips next instruction if kb char Vx is pressed
void op_skip_neq_kk(struct chip8 *chip_ate); // SKIP NEQ instruction -> skips next instruction if Vx != kk
void op_skip_neq_vy(struct chip8 *chip_ate); // SKIP NEQ instruction -> skips next instruction if Vx != Vy
void op_skip_neq_kb(struct chip8 *chip_ate); // SKIP NEQ instruction -> skips next instruction if kb char Vx is not pressed
void op_ld_kk(struct chip8 *chip_ate); // LD instruction -> loads kk into Vx
void op_ld_vy(struct chip8 *chip_ate); // LD instruction -> loads Vy into Vx
void op_ld_index(struct chip8 *chip_ate); // LD instruction -> loads nnn into I
void op_ld_delay_timer(struct chip8 *chip_ate); // LD instruction -> loads delay_timer val into Vx
void op_ld_kb_press(struct chip8 *chip_ate); // LD instruction -> loads pressed key into Vx
void op_ld_delay_timer_val(struct chip8 *chip_ate); // LD instruction -> loads Vx into delay timer
void op_ld_sound_timer_val(struct chip8 *chip_ate); // LD instruction -> loads Vx into sound timer
void op_ld_index_sprite(struct chip8 *chip_ate); // LD instruction -> loads memory address of value of Vx into index register
void op_ld_index_vx_bcd(struct chip8 *chip_ate); // LD instruction -> loads BCD representation of Vx into I, I+1, and I+2
void op_ld_index_gen_regs(struct chip8 *chip_ate); // LD instruction -> loads v0 through vx into mmap[i..i+x]
void op_ld_gen_regs_index(struct chip8 *chip_ate); // LD instruction -> loads mmap[i..i+x] into v0 through vx
void op_add_wo_carry(struct chip8 *chip_ate); // ADD instruction -> Vx = Vx + kk
void op_add_w_carry(struct chip8 *chip_ate); // ADD instruction -> Vx = Vx + kk and VF = carry
void op_add_index(struct chip8 *chip_ate); // ADD instruction -> I = I + Vx
void op_or(struct chip8 *chip_ate); // OR instruction -> Vx = Vx OR Vy
void op_and(struct chip8 *chip_ate); // AND instruction -> Vx = Vx AND Vy
void op_xor(struct chip8 *chip_ate); // AND instruction -> Vx = Vx XOR Vy
void op_sub(struct chip8 *chip_ate); // SUB instruction -> Vx = Vx - Vy
void op_subn(struct chip8 *chip_ate); // SUBN instruction -> Vx  = Vy - Vx
void op_shr(struct chip8 *chip_ate); // SHR instruction -> Vx/=2
void op_shl(struct chip8 *chip_ate); // SHL instruction -> Vx*=2
void op_rand(struct chip8 *chip_ate); // RND instruction -> Vx = rand(0,255) & kk
void op_drw(struct chip8 *chip_ate); // DRW instruction -> display n-byte sprite starting at mem location I at (Vx, Vy), set VF = collision

void init_sprite_data(struct chip8 *chip_ate);
void ld_instructions_file(const char* file_path, struct chip8 *chip_ate);
instruction opcode_to_instruction(uint16_t opcode);
void update_real_display(struct chip8 *chip_ate, SDL_Window* window, SDL_Surface *screen_surface); // Update the SDL display as per what's held in the chip8 display array
size_t twod_to_oned_arr_idx(size_t twod_arr_max_rows, size_t twod_row, size_t twod_col);

// TODO: Switch to local variable later
SDL_Window *window = NULL;
SDL_Surface *screen_surface = NULL;

int main(int argc, char *args[]) {
    // Initalizations - ChipAte and SDL
    struct chip8 chip_ate;
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    printf("Init didn't work!");
    }
    const char *title = "Test out SDL";
    window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT,
                                SDL_WINDOW_SHOWN);
    if (window == NULL) {
        printf("Window creation failed!");
    }
    screen_surface = SDL_GetWindowSurface(window);
    // TODO: Load .8o file here?
    // TODO: Accept as command line arg or part of some config
    ld_instructions_file("./tests/ibm.ch8", &chip_ate);
    // TODO: Need to reference the PC to know when we are done? Or maybe return it from ld_instructions
    // bool final_instruction = false;
    // Fetch-Decode-Execute cycle
    while (true) {
        // Fetch Step - instruction which pc is pointing to
        // uint8_t pc_high = (chip_ate.pc & 0xFF00) >> 8; 
        // uint8_t pc_low = (chip_ate.pc & 0x00FF); 
        chip_ate.opcode = (chip_ate.mmap[chip_ate.pc] << 8) + chip_ate.mmap[chip_ate.pc + 1];
        chip_ate.pc += 2;
        // Decode Step - Obtain pointer to relevant instruction function 
        instruction func = opcode_to_instruction(chip_ate.opcode);
        if (func == NULL) {
            abort(); // Something is wrong!
        }
        // Execute Step - execute the relevant instruction
        (*func)(&chip_ate);
        // TODO: Optimize to call when required!
        update_real_display(&chip_ate, window, screen_surface);
    }
    // Keeping the window up
    SDL_Event event;
    bool quit = false;
    while (quit == false) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || (event.type == SDL_TEXTINPUT && tolower(event.text.text[0]) == 'x')) {
                quit = true;
            }
        }
    }
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

void ld_instructions_file(const char* file_path, struct chip8 *chip_ate) {
    FILE *f_ptr = fopen(file_path, "rb");
    fread((chip_ate->mmap + 0x200), 1, MAX_PROG_SIZE, f_ptr);
    // Set the PC to the start of the program
    chip_ate->pc = 0x200;
    init_sprite_data(chip_ate);
    memset(chip_ate->v, 0, GENERAL_PURPOSE_REGS);
    memset(chip_ate->keyboard, 0, NUM_KEYS);
    memset(chip_ate->display, 0, VIEWPORT_HEIGHT * VIEWPORT_WIDTH);
    memset(chip_ate->stack, 0, STACK_DEPTH);
}

instruction opcode_to_instruction(uint16_t opcode) {
    // Exact Match Case
    switch (opcode) {
        case 0x00E0: return &op_cls;
        case 0x00EE: return &op_ret;
        default: break;
    }
    switch (opcode & 0xF000) {
        case 0x0000: return &op_sys;
        case 0x1000: return &op_jp_nnn;
        case 0x2000: return &op_call;
        case 0x3000: return &op_skip_eq_kk;
        case 0x4000: return &op_skip_neq_kk;
        case 0x6000: return &op_ld_kk;
        case 0x7000: return &op_add_wo_carry;
        case 0xA000: return &op_ld_index;
        case 0xB000: return &op_jp_nnn_v0;
        case 0xC000: return &op_rand;
        case 0xD000: return &op_drw;
        default: break;
    }
    switch (opcode & 0xF00F) {
        case 0x5000: return &op_skip_eq_vy;
        case 0x8000: return &op_ld_vy;
        case 0x8001: return &op_or;
        case 0x8002: return &op_and;
        case 0x8003: return &op_xor;
        case 0x8004: return &op_add_w_carry;
        case 0x8005: return &op_sub;
        case 0x8006: return &op_shr;
        case 0x8007: return &op_subn;
        case 0x800E: return &op_shl;
        case 0x9000: return &op_skip_neq_vy;
        default: break;
    }

    switch (opcode & 0xF0FF) {
        case 0xE09E: return &op_skip_eq_kb;
        case 0xE0A1: return &op_skip_neq_kb; 
        case 0xF007: return &op_ld_delay_timer;
        case 0xF00A: return &op_ld_kb_press;
        case 0xF015: return &op_ld_delay_timer_val;
        case 0xF018: return &op_ld_sound_timer_val;
        case 0xF01E: return &op_add_index;
        case 0xF029: return &op_ld_index_sprite;
        case 0xF033: return &op_ld_index_vx_bcd;
        case 0xF055: return &op_ld_index_gen_regs;
        case 0xF065: return &op_ld_gen_regs_index;
        default: break;
    }
    return NULL;
}

void init_sprite_data(struct chip8 *chip_ate) {
    // TODO: Do some assertion for mem-start value and ensure no spill over to idx 512 or above of mem
    uint8_t mem_start = SPRITE_DATA_MEM_START;
    // Sprite Bytes for 0
    chip_ate->mmap[mem_start] = 0xF0;
    chip_ate->mmap[++mem_start] = 0x90;
    chip_ate->mmap[++mem_start] = 0x90;
    chip_ate->mmap[++mem_start] = 0x90;
    chip_ate->mmap[++mem_start] = 0xF0;
    // Sprite Bytes for 1
    chip_ate->mmap[++mem_start] = 0x20;
    chip_ate->mmap[++mem_start] = 0x60;
    chip_ate->mmap[++mem_start] = 0x20;
    chip_ate->mmap[++mem_start] = 0x20;
    chip_ate->mmap[++mem_start] = 0x70;
    // Sprite Bytes for 2
    chip_ate->mmap[++mem_start] = 0xF0;
    chip_ate->mmap[++mem_start] = 0x10;
    chip_ate->mmap[++mem_start] = 0xF0;
    chip_ate->mmap[++mem_start] = 0x80;
    chip_ate->mmap[++mem_start] = 0xF0;
    // Sprite Bytes for 3
    chip_ate->mmap[++mem_start] = 0xF0;
    chip_ate->mmap[++mem_start] = 0x10;
    chip_ate->mmap[++mem_start] = 0xF0;
    chip_ate->mmap[++mem_start] = 0x10;
    chip_ate->mmap[++mem_start] = 0xF0;
    // Sprite Bytes for 4
    chip_ate->mmap[++mem_start] = 0x90;
    chip_ate->mmap[++mem_start] = 0x90;
    chip_ate->mmap[++mem_start] = 0xF0;
    chip_ate->mmap[++mem_start] = 0x10;
    chip_ate->mmap[++mem_start] = 0x10;
    // Sprite Bytes for 5
    chip_ate->mmap[++mem_start] = 0xF0;
    chip_ate->mmap[++mem_start] = 0x80;
    chip_ate->mmap[++mem_start] = 0xF0;
    chip_ate->mmap[++mem_start] = 0x10;
    chip_ate->mmap[++mem_start] = 0xF0;
    // Sprite Bytes for 6
    chip_ate->mmap[++mem_start] = 0xF0;
    chip_ate->mmap[++mem_start] = 0x80;
    chip_ate->mmap[++mem_start] = 0xF0;
    chip_ate->mmap[++mem_start] = 0x90;
    chip_ate->mmap[++mem_start] = 0xF0;
    // Sprite Bytes for 7
    chip_ate->mmap[++mem_start] = 0xF0;
    chip_ate->mmap[++mem_start] = 0x10;
    chip_ate->mmap[++mem_start] = 0x20;
    chip_ate->mmap[++mem_start] = 0x40;
    chip_ate->mmap[++mem_start] = 0x40;
    // Sprite Bytes for 8
    chip_ate->mmap[+mem_start] = 0xF0;
    chip_ate->mmap[+mem_start] = 0x90;
    chip_ate->mmap[+mem_start] = 0xF0;
    chip_ate->mmap[+mem_start] = 0x90;
    chip_ate->mmap[+mem_start] = 0xF0;
    // Sprite Bytes for 9
    chip_ate->mmap[+mem_start] = 0xF0;
    chip_ate->mmap[+mem_start] = 0x90;
    chip_ate->mmap[+mem_start] = 0xF0;
    chip_ate->mmap[+mem_start] = 0x10;
    chip_ate->mmap[+mem_start] = 0xF0;
    // Sprite Bytes for A/10
    chip_ate->mmap[+mem_start] = 0xF0;
    chip_ate->mmap[+mem_start] = 0x90;
    chip_ate->mmap[+mem_start] = 0xF0;
    chip_ate->mmap[+mem_start] = 0x90;
    chip_ate->mmap[+mem_start] = 0x90;
    // Sprite Bytes for B/11
    chip_ate->mmap[+mem_start] = 0xE0;
    chip_ate->mmap[+mem_start] = 0x90;
    chip_ate->mmap[+mem_start] = 0xE0;
    chip_ate->mmap[+mem_start] = 0x90;
    chip_ate->mmap[+mem_start] = 0xE0;
    // Sprite Bytes for C/12
    chip_ate->mmap[+mem_start] = 0xF0;
    chip_ate->mmap[+mem_start] = 0x90;
    chip_ate->mmap[+mem_start] = 0xF0;
    chip_ate->mmap[+mem_start] = 0x90;
    chip_ate->mmap[+mem_start] = 0xF0;
    // Sprite Bytes for 8
    chip_ate->mmap[+mem_start] = 0xF0;
    chip_ate->mmap[+mem_start] = 0x90;
    chip_ate->mmap[+mem_start] = 0xF0;
    chip_ate->mmap[+mem_start] = 0x90;
    chip_ate->mmap[+mem_start] = 0xF0;
    // Sprite Bytes for 8
    chip_ate->mmap[+mem_start] = 0xF0;
    chip_ate->mmap[+mem_start] = 0x90;
    chip_ate->mmap[+mem_start] = 0xF0;
    chip_ate->mmap[+mem_start] = 0x90;
    chip_ate->mmap[+mem_start] = 0xF0;
    // Sprite Bytes for 8
    chip_ate->mmap[+mem_start] = 0xF0;
    chip_ate->mmap[+mem_start] = 0x90;
    chip_ate->mmap[+mem_start] = 0xF0;
    chip_ate->mmap[+mem_start] = 0x90;
    chip_ate->mmap[+mem_start] = 0xF0;
}

void update_real_display(struct chip8 *chip_ate, SDL_Window* window, SDL_Surface *screen_surface) {
    const int pixel_width = SCREEN_WIDTH / VIEWPORT_WIDTH;
    const int pixel_height = SCREEN_HEIGHT / VIEWPORT_HEIGHT;
    size_t scaled_y = 0;
    for (size_t row_idx=0; row_idx < VIEWPORT_HEIGHT; row_idx+=1) {
        size_t scaled_x = 0;
        for (size_t col_idx=0; col_idx < VIEWPORT_WIDTH; col_idx+=1) {
            uint8_t color_val = chip_ate->display[col_idx][row_idx];
            SDL_Rect pixel_fill_rect = { .x = scaled_x, .y = scaled_y, .w = pixel_width, .h = pixel_height};
            if (color_val == 0) {
                SDL_FillRect(screen_surface, &pixel_fill_rect, SDL_MapRGB(screen_surface->format, 0x0, 0x0, 0x0));
            } else if (color_val == 1) {
                SDL_FillRect(screen_surface, &pixel_fill_rect, SDL_MapRGB(screen_surface->format, 0xFF, 0xFF, 0xFF));
            } else {
                // Should not happen!
                abort();
            }
            SDL_UpdateWindowSurface(window);
            scaled_x += pixel_width;
        }
        scaled_y += pixel_height;
    } 
}

size_t twod_to_oned_arr_idx(size_t twod_arr_max_rows, size_t twod_row, size_t twod_col) {
    return (twod_arr_max_rows * twod_row + twod_col);
}

void op_sys(struct chip8 *chip_ate) {
    return;
}

void op_cls(struct chip8 *chip_ate) {
    memset(chip_ate->display, 0, VIEWPORT_HEIGHT * VIEWPORT_WIDTH);
}

void op_ret(struct chip8 *chip_ate) {
    chip_ate->pc = chip_ate->stack[chip_ate->sp];
    chip_ate->sp-=1;
}

void op_jp_nnn(struct chip8 *chip_ate) {
    uint16_t nnn = (chip_ate->opcode & 0x0FFF);
    chip_ate->pc = nnn;
}

void op_jp_nnn_v0(struct chip8 *chip_ate) {
    uint16_t nnn = (chip_ate->opcode & 0x0FFF);
    chip_ate->pc = nnn + chip_ate->v[0];
}

void op_call(struct chip8 *chip_ate) {
    uint16_t nnn = (chip_ate->opcode & 0x0FFF);
    chip_ate->stack[++chip_ate->sp] = chip_ate->pc;
    chip_ate->pc = nnn;
}

void op_skip_eq_kk(struct chip8 *chip_ate) {
    uint8_t kk = (chip_ate->opcode & 0x00FF);
    uint8_t x = (chip_ate->opcode & 0x0F00) >> 8;
    if (chip_ate->v[x] == kk) {
        chip_ate->pc+=2;
    }
}

void op_skip_eq_vy(struct chip8 *chip_ate) {
    uint8_t y = (chip_ate->opcode & 0x00F0) >> 4;
    uint8_t x = (chip_ate->opcode & 0x0F00) >> 8;
    if (chip_ate->v[x] == chip_ate->v[y]) {
        chip_ate->pc+=2;
    }
}

void op_skip_eq_kb(struct chip8 *chip_ate) {
    uint8_t x = (chip_ate->opcode & 0x0F00) >> 8;
    // TODO: We shouldn't assume that Vx will hold the index to the correct key in our keyboard arr, need to define mapping
    if (chip_ate->keyboard[chip_ate->v[x]] == 1) {
        chip_ate->pc+=2;
    }
}

void op_skip_neq_kk(struct chip8 *chip_ate) {
    uint8_t kk = (chip_ate->opcode & 0x00FF);
    uint8_t x = (chip_ate->opcode & 0x0F00) >> 8;
    if (chip_ate->v[x] != kk) {
        chip_ate->pc+=2;
    }
}

void op_skip_neq_vy(struct chip8 *chip_ate) {
    uint8_t y = (chip_ate->opcode & 0x00F0) >> 4;
    uint8_t x = (chip_ate->opcode & 0x0F00) >> 8;
    if (chip_ate->v[x] != chip_ate->v[y]) {
        chip_ate->pc+=2;
    }
}

void op_skip_neq_kb(struct chip8 *chip_ate) {
    uint8_t x = (chip_ate->opcode & 0x0F00) >> 8;
    // TODO: We shouldn't assume that Vx will hold the index to the correct key in our keyboard arr, need to define mapping
    if (chip_ate->keyboard[chip_ate->v[x]] == 0) {
        chip_ate->pc+=2;
    }
}

void op_ld_kk(struct chip8 *chip_ate) {
    uint8_t kk = (chip_ate->opcode & 0x00FF);
    uint8_t x = (chip_ate->opcode & 0x0F00) >> 8;
    chip_ate->v[x] = kk;
}

void op_ld_vy(struct chip8 *chip_ate) {
    uint8_t y = (chip_ate->opcode & 0x00F0) >> 4;
    uint8_t x = (chip_ate->opcode & 0x0F00) >> 8;
    chip_ate->v[x] = chip_ate->v[y];
}

void op_ld_index(struct chip8 *chip_ate) {
    uint16_t nnn = (chip_ate->opcode & 0x0FFF);
    chip_ate->index = nnn;
}

void op_ld_delay_timer(struct chip8 *chip_ate) {
    uint8_t x = (chip_ate->opcode & 0x0F00) >> 8;
    chip_ate->v[x] = chip_ate->delay_timer;
}

void op_ld_kb_press(struct chip8 *chip_ate) {
    uint8_t x = (chip_ate->opcode & 0x0F00) >> 8;
    bool key_press = false;
    // TODO: We need to define a mapping b/w keys on keyboard and what val Vx stores? Look at test roms at example usages
    while (!key_press) {
        for (size_t kb_char=0; kb_char < NUM_KEYS; kb_char+=1) {
            if (chip_ate->keyboard[kb_char] == 1) {
                chip_ate->v[x] = kb_char;
                key_press = true;
                break;
            }
        }
    }
}

void op_ld_delay_timer_val(struct chip8 *chip_ate) {
    uint8_t x = (chip_ate->opcode & 0x0F00) >> 8;
    chip_ate->delay_timer = chip_ate->v[x];
}

void op_ld_sound_timer_val(struct chip8 *chip_ate) {
    uint8_t x = (chip_ate->opcode & 0x0F00) >> 8;
    chip_ate->sound_timer = chip_ate->v[x];
}


void op_ld_index_sprite(struct chip8 *chip_ate) {
    uint8_t x = (chip_ate->opcode & 0x0F00) >> 8;
    // Multiple by 5 since all symbols are 8x5 sprites
    uint8_t sprite_start_addr = chip_ate->v[x] * 5;
    chip_ate->index = chip_ate->mmap[sprite_start_addr];
}

void op_ld_index_vx_bcd(struct chip8 *chip_ate) {
    uint8_t x = (chip_ate->opcode & 0x0F00) >> 8;
    uint8_t vx = chip_ate->v[x];
    // Init to 3 representing the hundredths place digit
    size_t index_register_offset = 0;
    uint8_t digit_place = 100;
    while (vx != 0) {
        chip_ate->mmap[chip_ate->index + index_register_offset] = (uint8_t)(vx / digit_place);
        vx%=digit_place;
        digit_place = (uint8_t) (digit_place / 10);
        index_register_offset+=1;
    }
}

void op_ld_index_gen_regs(struct chip8 *chip_ate) {
    uint8_t x = (chip_ate->opcode & 0x0F00) >> 8;
    // TODO: Is it an inclusive  for Vx as well?
    for (size_t loop_idx=0; loop_idx < x+1; loop_idx+=1) {
        chip_ate->mmap[chip_ate->index + loop_idx] = chip_ate->v[loop_idx];
    }
}

void op_ld_gen_regs_index(struct chip8 *chip_ate) {
    uint8_t x = (chip_ate->opcode & 0x0F00) >> 8;
    // TODO: Is it an inclusive copy for Vx as well?
    for (size_t loop_idx=0; loop_idx < x+1; loop_idx+=1) {
        chip_ate->v[loop_idx] = chip_ate->mmap[chip_ate->index + loop_idx];
    }
}

void op_add_wo_carry(struct chip8 *chip_ate) {
    uint8_t kk = (chip_ate->opcode & 0x00FF);
    uint8_t x = (chip_ate->opcode & 0x0F00) >> 8;
    chip_ate->v[x]+=kk;
}

void op_add_w_carry(struct chip8 *chip_ate) {
    uint8_t y = (chip_ate->opcode & 0x00F0) >> 4;
    uint8_t x = (chip_ate->opcode & 0x0F00) >> 8;
    uint16_t sum = chip_ate->v[x] + chip_ate->v[y];
    if (sum > 0xFF) {
        chip_ate->v[0xF] = 1;
    } else {
        chip_ate->v[0xF] = 0;
    }
    chip_ate->v[x] = (sum & 0x00FF);
}

void op_add_index(struct chip8 *chip_ate) {
    uint8_t x = (chip_ate->opcode & 0x0F00) >> 8;
    chip_ate->index+=chip_ate->v[x];
}

void op_or(struct chip8 *chip_ate) {
    uint8_t y = (chip_ate->opcode & 0x00F0) >> 4;
    uint8_t x = (chip_ate->opcode & 0x0F00) >> 8;
    chip_ate->v[x]|=chip_ate->v[y];
}

void op_and(struct chip8 *chip_ate) {
    uint8_t y = (chip_ate->opcode & 0x00F0) >> 4;
    uint8_t x = (chip_ate->opcode & 0x0F00) >> 8;
    chip_ate->v[x]&=chip_ate->v[y];
}

void op_xor(struct chip8 *chip_ate) {
    uint8_t y = (chip_ate->opcode & 0x00F0) >> 4;
    uint8_t x = (chip_ate->opcode & 0x0F00) >> 8;
    chip_ate->v[x]^=chip_ate->v[y];
}

void op_sub(struct chip8 *chip_ate) {
    uint8_t y = (chip_ate->opcode & 0x00F0) >> 4;
    uint8_t x = (chip_ate->opcode & 0x0F00) >> 8;
    if (chip_ate->v[x] > chip_ate->v[y]) {
        chip_ate->v[0xF] = 1;
    } else {
        chip_ate->v[0xF] = 0;
    }
    chip_ate->v[x]-=chip_ate->v[y];
}

void op_subn(struct chip8 *chip_ate) {
    uint8_t y = (chip_ate->opcode & 0x00F0) >> 4;
    uint8_t x = (chip_ate->opcode & 0x0F00) >> 8;
    if (chip_ate->v[x] > chip_ate->v[y]) {
        chip_ate->v[0xF] = 0;
    } else {
        chip_ate->v[0xF] = 1;
    }
    chip_ate->v[x] = chip_ate->v[y] - chip_ate->v[x];
}

void op_shr(struct chip8 *chip_ate) {
    uint8_t y = (chip_ate->opcode & 0x00F0) >> 4;
    uint8_t x = (chip_ate->opcode & 0x0F00) >> 8;
    if ((chip_ate->v[x] & 0x0001) == 0x0001) {
        chip_ate->v[0xF] = 1;
    } else {
        chip_ate->v[0xF] = 0;
    }
    chip_ate->v[x]>>=1;
}

void op_shl(struct chip8 *chip_ate) {
    uint8_t y = (chip_ate->opcode & 0x00F0) >> 4;
    uint8_t x = (chip_ate->opcode & 0x0F00) >> 8;
    if ((chip_ate->v[x] & 0x1000) == 0x1000) {
        chip_ate->v[0xF] = 1;
    } else {
        chip_ate->v[0xF] = 0;
    }
    chip_ate->v[x]<<=1;
}

void op_rand(struct chip8 *chip_ate) {
    uint8_t kk = (chip_ate->opcode & 0x00FF);
    uint8_t x = (chip_ate->opcode & 0x0F00) >> 8;
    uint8_t rand_num = rand() % 0xFF;
    chip_ate->v[x] = rand_num & kk;
}

void op_drw(struct chip8 *chip_ate) {
    uint8_t n = (chip_ate->opcode & 0x000F);
    uint8_t x = (chip_ate->opcode & 0x0F00) >> 8;
    uint8_t y = (chip_ate->opcode & 0x00F0) >> 4;
    uint16_t start_mem_idx = chip_ate->index;
    int current_y = chip_ate->v[y];
    for (size_t idx = 0; idx < n; idx+=1) {
        int current_x = chip_ate->v[x];
        uint8_t sprite_byte = chip_ate->mmap[start_mem_idx + idx];
        current_y %= VIEWPORT_HEIGHT;
        // Choosing 8 since every sprite will be a byte i.e 8 bits wide
        short nth_bit = 7;
        while (nth_bit >= 0) {
            // Obtaining the individual bit
            uint8_t pixel_color_val = (sprite_byte & ( 1 << nth_bit )) >> nth_bit;
            current_x %= VIEWPORT_WIDTH;
            if (chip_ate->display[current_x][current_y] ^ pixel_color_val == 1) {
                chip_ate->v[0xF] = 1;
                chip_ate->display[current_x][current_y] = pixel_color_val;
            }
            current_x += 1;
            nth_bit -= 1;
        }
        current_y+=1;
    }
}