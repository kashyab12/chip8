const MEMORY_SIZE: usize = 4096;
const NUM_GENERAL_PURPOSE_REGS: usize = 16;
const CALL_STACK_DEPTH: usize = 16;
const KEYBOARD_CHARS: usize = 16;
const DISPLAY_HEIGHT: usize = 32;
const DISPLAY_WIDTH: usize = 64;

struct Chip8 {
    memory: [u8; MEMORY_SIZE],
    v: [u8; NUM_GENERAL_PURPOSE_REGS],
    delay_timer: u8,
    sound_timer: u8,
    pc: u16,
    stack: [u16; CALL_STACK_DEPTH],
    sp: u8,
    opcode: u16,
    keyboard: [u8; KEYBOARD_CHARS],
    display: [u8; DISPLAY_HEIGHT * DISPLAY_WIDTH]
}