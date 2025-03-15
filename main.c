#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <stdbool.h>

#include "str.h"
#include "dynamic_array.h"

#define UNREACHABLE(message) do { fprintf(stderr, "%s:%d: UNREACHABLE: %s\n", __FILE__, __LINE__, message); abort(); } while(0)

int cursorX = 1, cursorY = 1;
void terminal_raw_mode(void)
{
    struct termios raw;
    tcgetattr(STDIN_FILENO, &raw);

    // ECHO - Prevents terminal from displaying what is typed
    // ICANON - Unbuffer input.
    raw.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void terminal_normal_mode(void)
{
    struct termios raw;
    tcgetattr(STDIN_FILENO, &raw);

    raw.c_lflag |= (ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

// TODO: make move_cursor scrollable
void move_cursor(char read_arrow)
{
    // TODO: treat ESC case
    // TODO: treat OTHER cases (shift-ARROWS, cntrl-ARROWS, alt-ARROWS ~ anything that might be combinable with ARROW-Keys)
    switch ( read_arrow )
    {
        case 'A':
            cursorY--; // UP
            break;
        case 'B':
            cursorY++; // DOWN
            break;
        case 'C':
            cursorX++; // LEFT
            break;
        case 'D':
            cursorX--; // RIGHT
            break;
        default: UNREACHABLE("Arrows");
     }

    printf("\x1b[%d;%dH", cursorY, cursorX);
    fflush(stdout);
}

bool handle_keyboard_input(char read_char)
{
    // TODO: put it into switch case to handle all
    switch ( read_char )
    {
        case 'a':
            printf("Leaving...\n");
            return true;
        case '\x1b':
            char seq[2];
            if ( (read(STDIN_FILENO, &seq[0], 1) == 1) &&
                 (read(STDIN_FILENO, &seq[1], 1) == 1)    )
            {
                if ( seq[0] == '[' ) move_cursor(seq[1]);
            }
            break;
        default:
            printf("%c\n", read_char);
            break;
    }
    return false;
}

int main(void)
{
    terminal_raw_mode();
    darray_tt da = darray_create(string_free, sizeof(string_tt), true);

    // TODO: make file to be read at execution time
    FILE *fp = fopen("str.c", "r");
    char *line = NULL;
    size_t len = 0;
    while ( (getline(&line, &len, fp)) != -1 )
    {
        darray_insert(da, string_create(line));
        printf("%s\n", line);
    }

    char read_char;
    while ( (read(STDIN_FILENO, &read_char, 1)) == 1 )
    {
        if (handle_keyboard_input(read_char)) break;
    }

    terminal_normal_mode();
    // TODO: clean terminal whenever code finishes.
    darray_destroy(da);
    return 0;
}
