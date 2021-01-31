#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

// Global structure of original terminal
struct termios orig_termios;

// disableRawMode when the editor exits
void disableRawMode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

// enableRawMode when the editor starts
void enableRawMode() {
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disableRawMode);
    
    // Create a copy of the original terminal to edit
    struct termios raw = orig_termios;

    // Fix Control-M and Disable Control-S and Control-Q
    // I == input, XON == "X ON"
    raw.c_iflag &= ~(ICRNL | IXON);

    // Turn off echo, canonical (read bytes instead of lines), Control-V, and SIGINT/SIGSTOP commands
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

int main() {
    enableRawMode();

    char c;
    while (read(STDIN_FILENO, &c, 1) == 1 && c!= 'q') {

        // If we have a control byte like ENTER
        if (iscntrl(c)) {
            printf("%d\n", c);

        // Print the byte as a number and character
        } else {
            printf("%d ('%c')\n", c, c);
        }
    }
    return 0;
}
