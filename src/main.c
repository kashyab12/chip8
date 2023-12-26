//
// Created by kashyab on 12/25/23.
//
#include <stdio.h>
#include <SDL2/SDL.h>
#include <stdbool.h>

// Constants
const uint16_t SCREEN_WIDTH = 640;
const uint16_t SCREEN_HEIGHT = 480;
const uint16_t MEMORY_SIZE = 4096;
const uint8_t GENERAL_PURPOSE_REGS = 16;
const uint8_t STACK_DEPTH = 16;
const uint8_t NUM_KEYS = 16;
const uint8_t VIEWPORT_WIDTH = 64;
const uint8_t VIEWPORT_HEIGHT = 32;

struct chip8 {
    uint8_t mmap[MEMORY_SIZE], v[GENERAL_PURPOSE_REGS], keyboard[NUM_KEYS], display[
            VIEWPORT_HEIGHT * VIEWPORT_WIDTH], sound_timer, delay_timer, sp;
    uint16_t pc, stack[STACK_DEPTH], opcode;
};


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
    SDL_FillRect(screen_surface, NULL, SDL_MapRGB(screen_surface->format, 0xFF, 0xFF, 0xFF));
    // Updating surface
    SDL_UpdateWindowSurface(window);
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