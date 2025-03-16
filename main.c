#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "str.h"
#include "utils.h"
#include "dynamic_array.h"

#define UNREACHABLE(message) do { fprintf(stderr, "%s:%d: UNREACHABLE: %s\n", __FILE__, __LINE__, message); abort(); } while(0)

// GLOBAL VARIABLES
typedef struct
{
    darray_tt da;
    int cursorX;
    int cursorY;
    int offsetX;
    int offsetY;
    int screen_rows;
    int screen_columns;
} Controller;
static Controller ct = {NULL, 0, 0, 0, 0, 0, 0};
static struct termios original_termios;

void terminal_raw_mode(void)
{
    struct termios settings;
    int result;
    if ( (result = tcgetattr(STDIN_FILENO, &original_termios)) < 0 ) handle_error("ERROR: Failed to tcgetattr");

    settings = original_termios;

    // ECHO - Prevents terminal from displaying what is typed
    // ICANON - Unbuffer input.
    settings.c_lflag &= ~(ECHO | ICANON);
    if ( (result = tcsetattr(STDIN_FILENO, TCSAFLUSH, &settings)) < 0 ) handle_error("ERROR: Failed to tcsetattr");
}

void terminal_cooked_mode(void)
{
    if ( tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_termios) < 0 ) handle_error("ERROR: Failed to tcsetattr");
}

void get_terminal_size(void)
{
    struct winsize ws;
    if ( ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 ) handle_error("ERROR: Failed to ioctl");

    ct.screen_rows = ws.ws_row;
    ct.screen_columns = ws.ws_col;
}

void draw_content(void)
{
    printf("\x1b[H");

    int num_show_lines = ( ct.screen_rows > (int)(ct.da->count) ) ? (int)(ct.da->count): ct.screen_rows - 1;

    for ( int i = 0; i < num_show_lines; i++ )
    {
        printf("\x1b[K");
        // TODO: Properly handle the lines id here
        // printf("%d: ", i + ct.offsetY);
        string_print(ct.da->items[i + ct.offsetY]);
    }
}

void reposition_cursor(void)
{
    printf("\x1b[%d;%dH", ct.cursorY, ct.cursorX);
    fflush(stdout);
}

void redraw_screen(void)
{
    draw_content();
    reposition_cursor();
}

// TODO: make move_cursor scrollable
void handle_cursor(char read_arrow)
{
    bool redraw = false;
    // TODO: treat ESC case
    // TODO: treat OTHER cases (shift-ARROWS, cntrl-ARROWS, alt-ARROWS ~ anything that might be combinable with ARROW-Keys)
    //       maybe there is a flag to be used at tcsetattr()
    // TODO: Finish SCROLL Left-Right -> Maybe word-wrap
    // TODO: Screen resize not being treated
    switch ( read_arrow )
    {
        case 'A': // UP
            if ( ct.cursorY > 0 ) ct.cursorY--;
            else if ( ct.offsetY > 0 )
            {
                redraw = true;
                ct.offsetY--; // Scroll
            }
            break;
        case 'B': // DOWN

            // Preventing to go beyond the total number of strings (max_column)
            if ( ct.cursorY < (int) (ct.da->count) )
            {
                ct.cursorY++;
                if ( ct.cursorY >= ct.screen_rows + ct.offsetY )
                {
                    redraw = true;
                    ct.offsetY++; // Scroll
                }
            }
            break;
        case 'C': // RIGHT

            // Preventing to go beyond the string content (max_column)
            int curr_pointerY = (ct.cursorY > 0) ? ct.cursorY - 1 : 0;
            int max_column = (int) ((string_tt) ct.da->items[curr_pointerY + ct.offsetY])->content_len;
            if ( ct.cursorX < max_column )
            {
                ct.cursorX++;
                if ( ct.cursorX >= ct.screen_columns + ct.offsetX ) ct.offsetX++; // Scroll
            }
            break;
        case 'D': // LEFT
            if ( ct.cursorX > 0 ) ct.cursorX--;
            else if ( ct.offsetX > 0 ) ct.offsetX--; // Scroll
            break;
        default: UNREACHABLE("Arrows");
    }
    if ( redraw ) redraw_screen();
    else reposition_cursor();
}

void handle_typing(char read_char)
{
    int curr_pointerY = (ct.cursorY > 0) ? ct.cursorY - 1 : 0;
    int curr_pointerX = (ct.cursorX > 0) ? ct.cursorX - 1 : 0;
    string_append(ct.da->items[curr_pointerY + ct.offsetY], read_char, curr_pointerX);

    // Rewrite current line
    printf("\x1b[%d;0H", ct.cursorY);
    printf("\x1b[2K");
    printf("\x1b[%d;0H", ct.cursorY);
    string_print(ct.da->items[curr_pointerY + ct.offsetY]);

    ct.cursorY = curr_pointerY + 1;
    ct.cursorX = curr_pointerX + 2;
    reposition_cursor();
}

bool handle_keyboard_input(char read_char)
{
    // TODO: Allow <ENTER> to create a newly complete line
    switch ( read_char )
    {
        case 'a':
            // TODO: curses.h has a database of terminals types and their escape sequences, might be better than this
            printf("\x1b[2J");
            printf("Leaving...\n");
            return true;
        case '\x1b':
            char seq[2];
            if ( (read(STDIN_FILENO, &seq[0], 1) == 1) &&
                 (read(STDIN_FILENO, &seq[1], 1) == 1)    )
            {
                if ( seq[0] == '[' ) handle_cursor(seq[1]);
            }
            break;
        default:
            handle_typing(read_char);
            break;
    }
    return false;
}

int main(int argc, char** argv)
{
    if ( argc != 2 )
    {
        printf("Usage: ./buid/main <file_name>\n");
        exit(EXIT_FAILURE);
    }

    ct.da = darray_create(string_free, sizeof(string_tt), true);
    terminal_raw_mode();
    atexit(terminal_cooked_mode);

    // Terminal info
    get_terminal_size();

    FILE *fp = fopen(argv[1], "r");
    if ( fp == NULL ) handle_error("ERROR: Unnable to open file\n");
    char *line;
    size_t len = 0;
    while ( (getline(&line, &len, fp)) != -1 )
    {
        if ( line != NULL ) darray_insert(ct.da, string_create(line));
    }
    fclose(fp);

    draw_content();
    reposition_cursor();

    char read_char;
    while ( (read(STDIN_FILENO, &read_char, 1)) == 1 )
    {
        if (handle_keyboard_input(read_char)) break;
    }

    darray_destroy(ct.da);
    return 0;
}
