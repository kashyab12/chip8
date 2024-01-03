## Chip 8

A simple first emulator! Following the references below for the implementation:
- [Chip 8 Technical Reference 1](https://github.com/mattmikolay/chip-8/wiki/CHIP%E2%80%908-Technical-Reference)
- [Chip 8 Technical Reference 2](http://devernay.free.fr/hacks/chip8/C8TECH10.HTM)
- [Chip 8 Instruction Set](https://github.com/mattmikolay/chip-8/wiki/CHIP%E2%80%908-Instruction-Set)
- [SDL2 Reference](https://lazyfoo.net/tutorials/SDL/02_getting_an_image_on_the_screen/index.php)

Follow me working on this live: "[emudev - chip8](https://www.youtube.com/watch?v=9Zi7X2YB_Is&list=PLUjxEbbWODdNA5sLttq8u1X5EA7lIa3M7)".

### todos
- [ ] Ensure drawing is working correctly and complete implementation of `Dxyn`. Ensure chip8 struct display which we cache syncs it's state with actual display.
 - [ ] Figure out loading of instruction file. According to sources, seems to be of `.8o` format. However, where are the list of tokens specified?
 - [ ] Audio i.e. producing tones in accordance with the sound timer and delay timer.
 - [ ] small todos and optimizations (some mentioned within main.c)  
 - [ ] Better makefile
 - [ ] c -> wasm; lesgo to the web ;D