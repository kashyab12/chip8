//
// Created by kashyab on 12/25/23.
//
#include <stdio.h>
#include <SDL2/SDL.h>
#include <stdbool.h>
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

struct chip8 {
    uint8_t mmap[MEMORY_SIZE], v[GENERAL_PURPOSE_REGS], keyboard[NUM_KEYS], display[
            VIEWPORT_HEIGHT * VIEWPORT_WIDTH], sound_timer, delay_timer, sp;
    uint16_t pc, stack[STACK_DEPTH], opcode;
};

void dxyn(struct chip8 chip_ate); // DRW instruction
void draw_sprite(struct chip8 *chip_ate, SDL_Window* window, SDL_Surface *screen_surface, int start_x, int start_y, const uint8_t sprite_bytes[], size_t num_sprite_bytes);
size_t twod_to_oned_arr_idx(size_t twod_arr_max_rows, size_t twod_row, size_t twod_col);

int main(int argc, char *args[]) {
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
        // TODO: Need to be using viewport height and width
        if (current_y >= SCREEN_HEIGHT) {
            current_y %= SCREEN_HEIGHT;
        }
        // TODO: How would clipping occur if we are using modulo?
        // Choosing 8 since every sprite will be a byte i.e 8 bits wide
        short nth_bit = 7;
        while (nth_bit >= 0) {
            // Obtaining the individual bit
            size_t nth_bit_val = (sprite_byte & ( 1 << nth_bit )) >> nth_bit;
            // TODO: Need to be using viewport height and width
            if (current_x >= SCREEN_WIDTH) {
                current_x %= SCREEN_WIDTH;
            }
            // TODO: Perhaps scale pixel width and height as per as factors of screen_width / viewport_width (repeat for height)
            SDL_Rect pixel_fill_rect = { .x = current_x, .y = current_y, .w = pixel_width, .h = pixel_height};
            if (nth_bit_val == 0) {
                SDL_FillRect(screen_surface, &pixel_fill_rect, SDL_MapRGB(screen_surface->format, 0x0, 0x0, 0x0));
            } else {
                SDL_FillRect(screen_surface, &pixel_fill_rect, SDL_MapRGB(screen_surface->format, 0xFF, 0xFF, 0xFF));
            }
            // Updating surface
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
