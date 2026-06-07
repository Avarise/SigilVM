#pragma once
#include <iostream>
#include <string>
#include <unistd.h>
#include <termios.h>

namespace sigil::tty {

// Store original terminal state
struct term_state_t {
    termios original {};
    bool valid = false;
};

// Enable raw mode (no canonical input, no echo)
inline term_state_t enable_raw_mode() {
    term_state_t state;

    if (tcgetattr(STDIN_FILENO, &state.original) == 0) {
        termios raw = state.original;

        raw.c_lflag &= ~(ECHO | ICANON);   // no echo, no line buffering
        raw.c_cc[VMIN]  = 1;               // read 1 byte
        raw.c_cc[VTIME] = 0;

        tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
        state.valid = true;
    }

    return state;
}

inline void restore_terminal(const term_state_t& state) {
    if (state.valid) {
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &state.original);
    }
}

// Clear entire screen
inline void clear() {
    std::cout << "\033[2J\033[H";
}

// Move cursor
inline void move(int row, int col) {
    std::cout << "\033[" << row << ";" << col << "H";
}

// Hide / show cursor
inline void hide_cursor() {
    std::cout << "\033[?25l";
}

inline void show_cursor() {
    std::cout << "\033[?25h";
}

// Flush output explicitly
inline void flush() {
    std::cout.flush();
}

// Read single char (blocking)
inline char read_char() {
    char c = 0;
    if (::read(STDIN_FILENO, &c, 1) <= 0) return 0;
    return c;
}

} // namespace sigil::tty