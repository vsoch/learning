/*** includes ***/

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>


/*** defines ***/

// We want control W to quit
#define CTRL_KEY(k) ((k) & 0x1f)

/*** data ***/

struct editorConfig {
    struct termios orig_termios;
}

struct editorConfig E;

/*** terminal ***/

// Handle error
void die(const char *s) {
    perror(s);
    exit(1);
}

// disableRawMode when the editor exits
void disableRawMode() {
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) == -1)
        die("tcsetattr");
}

// enableRawMode when the editor starts
void enableRawMode() {
    if (tcgetattr(STDIN_FILENO, &E.orig_termios) == -1) die("tcgetattr");
    atexit(disableRawMode);
    
    // Create a copy of the original terminal to edit
    struct termios raw = E.orig_termios;

    // Traditional flag to disable during raw mode
    // Fix Control-M and Disable Control-S and Control-Q
    // I == input, XON == "X ON"
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_cflag |= (CS8);

    // Turn off output processing
    raw.c_oflag &= ~(OPOST);
    
    // Turn off echo, canonical (read bytes instead of lines), Control-V, and SIGINT/SIGSTOP commands
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

    // set a timeout so read returns if no input for a while
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) die("tcsetattr");
}

char editorReadKey() {
    int nread;
    char c;
    while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
        if (nread == -1 && errno != EAGAIN) die("read");
    }
    return c;
}

int getWindowSize(int *rows, int *cols) {
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1 || ws.ws_col = 0) {
        return -1;
    } else {
        *cols = ws.ws_col;
        *rows = ws.ws_row;
        return 0;
    }
}

/*** output ***/

void editorDrawRows() {
    int y;
    for (y = 0; y<24; y++){
        write(STDOUT_FILENO, "~\r\n", 3);
    }
}

void editorRefreshScreen() {
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);
    editorDrawRows();

    write(STDOUT_FILENO, "\x1b[H", 3);
}


void editorProcessKeypress() {
    char c = editorReadKey();

    switch (c) {
        case CTRL_KEY('q'):
            write(STDOUT_FILENO, "\x1b[2J", 4);
            write(STDOUT_FILENO, "\x1b[H", 3);
            exit(0);
            break;
    }
}

/*** init ***/

int main() {
    enableRawMode();

    while (1) {
        editorRefreshScreen();
        editorProcessKeypress();
    }
    return 0;
}
