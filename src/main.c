//
// Created by kashyab on 12/25/23.
//
#include <stdio.h>
#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

// Constants
#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define MEMORY_SIZE 4096
#define GENERAL_PURPOSE_REGS 16
#define STACK_DEPTH 16
#define NUM_KEYS 16
#define VIEWPORT_WIDTH 64
#define VIEWPORT_HEIGHT 32
#define SPRITE_DATA_MEM_START 0

struct chip8 {
    uint8_t mmap[MEMORY_SIZE], v[GENERAL_PURPOSE_REGS], keyboard[NUM_KEYS], display[
            VIEWPORT_HEIGHT * VIEWPORT_WIDTH], sound_timer, delay_timer, sp;
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
instruction opcode_to_instruction(uint16_t opcode);
void update_display(struct chip8 *chip_ate, SDL_Window* window, SDL_Surface *screen_surface); // Update the SDL display as per what's held in the chip8 display array
void draw_sprite(struct chip8 *chip_ate, SDL_Window* window, SDL_Surface *screen_surface, int start_x, int start_y, const uint8_t sprite_bytes[], size_t num_sprite_bytes);
size_t twod_to_oned_arr_idx(size_t twod_arr_max_rows, size_t twod_row, size_t twod_col);

int main(int argc, char *args[]) {
    struct chip8 *chip_ate = {};
    bool final_instruction = false;
    // Fetch-Decode-Execute cycle
    while (!final_instruction) {
        // Fetch Step - instruction which pc is pointing to
        uint8_t pc_high = (chip_ate->pc & 0xFF00) >> 8; 
        uint8_t pc_low = (chip_ate->pc & 0x00FF); 
        uint16_t fetched_op = (chip_ate->mmap[pc_high] << 8) + chip_ate->mmap[pc_low];
        // Decode Step - Obtain pointer to relevant instruction function 
        instruction func = opcode_to_instruction(fetched_op);
        // Execute Step - execute the relevant instruction
        (*func)(chip_ate);
    }

    SDL_Window *window = NULL;
    SDL_Surface *screen_surface = NULL;
    // Init SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Init didn't work!");
    }
    const char *title = "Test out SDL";
    window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT,
                              SDL_WINDOW_SHOWN);
    if (window == NULL) {
        printf("Window creation failed!");
    }
    // Getting the window surface
    screen_surface = SDL_GetWindowSurface(window);
    // Color the surface
    int start_x = (int)(SCREEN_WIDTH / 2);
    int start_y = (int)(SCREEN_HEIGHT / 2);
    uint8_t zero_sprite[] = {0xF0, 0x90, 0x90, 0x90, 0xF0};
    struct chip8 chip_ate;
    draw_sprite(&chip_ate, window, screen_surface, start_x, start_y, zero_sprite, 5);
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

instruction opcode_to_instruction(uint16_t opcode) {
    switch (opcode) {
        case 0x00E0: return &op_cls;
        case 0x00EE: return &op_ret;
        
    }
    switch (opcode & 0x1000) {
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

    }
    switch (opcode & 0x1001) {
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
    }

    switch (opcode & 0x1011) {
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
    }
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

// Draft function - some of this behavior will be transferred to the DRW instruction
void
draw_sprite(struct chip8 *chip_ate, SDL_Window* window, SDL_Surface *screen_surface, int start_x, int start_y, const uint8_t sprite_bytes[], size_t num_sprite_bytes) {
    int current_x, current_y;
    int pixel_width = SCREEN_WIDTH / VIEWPORT_WIDTH;
    int pixel_height = SCREEN_HEIGHT / VIEWPORT_HEIGHT;
    current_x = start_x;
    current_y = start_y;
    // TODO: Need to incorporate the display array via the chip_ate struct here
    for (size_t idx = 0; idx < num_sprite_bytes; idx+=1) {
        uint8_t sprite_byte = sprite_bytes[idx];
        // TODO: Need to be using viewport height and width (64 x 32 is too small, so need to allow resizing?)
        if (current_y >= SCREEN_HEIGHT) {
            current_y %= SCREEN_HEIGHT;
        }
        // TODO: How would clipping occur if we are using modulo?
        // Choosing 8 since every sprite will be a byte i.e 8 bits wide
        short nth_bit = 7;
        while (nth_bit >= 0) {
            // Obtaining the individual bit
            size_t nth_bit_val = (sprite_byte & ( 1 << nth_bit )) >> nth_bit;
            // TODO: Need to be using viewport height and width (64 x 32 is too small, so need to allow resizing?)
            if (current_x >= SCREEN_WIDTH) {
                current_x %= SCREEN_WIDTH;
            }
            SDL_Rect pixel_fill_rect = { .x = current_x, .y = current_y, .w = pixel_width, .h = pixel_height};
            if (nth_bit_val == 0) {
                SDL_FillRect(screen_surface, &pixel_fill_rect, SDL_MapRGB(screen_surface->format, 0x0, 0x0, 0x0));
            } else {
                SDL_FillRect(screen_surface, &pixel_fill_rect, SDL_MapRGB(screen_surface->format, 0xFF, 0xFF, 0xFF));
            }
            // Updating surface
            // TODO: Perhaps call it after every drawing an entire sprite?
            SDL_UpdateWindowSurface(window);
            current_x += pixel_width;
            nth_bit -= 1;
        }
        current_x = start_x;
        current_y += pixel_height;
    }
}

size_t twod_to_oned_arr_idx(size_t twod_arr_max_rows, size_t twod_row, size_t twod_col) {
    return (twod_arr_max_rows * twod_row + twod_col);
}

void op_cls(struct chip8 *chip_ate) {
    for (size_t pixel_idx=0; pixel_idx < VIEWPORT_WIDTH * VIEWPORT_HEIGHT; pixel_idx+=1) {
        chip_ate->display[pixel_idx] = 0;
    }
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
    chip_ate->v[x]&=chip_ate->v[y];
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
