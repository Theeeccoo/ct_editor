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

#define BACKSPACE_KEY 127
#define DEL_KEY       '~'
#define ENTER_KEY     '\n'
#define ESCAPE_SEQ_INIT '\x1b'

#define UNREACHABLE(message) do { fprintf(stderr, "%s:%d: UNREACHABLE: %s\n", __FILE__, __LINE__, message); abort(); } while(0)

// GLOBAL VARIABLES
typedef struct
{
    // TODO: Maybe an list would be better (for removal/insert) than darray
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

// -----------------------------------------------------
// |                 TERMINAL SETTINGS                 |
// -----------------------------------------------------
void terminal_raw_mode(void)
{
    struct termios settings;
    int result;
    if ( (result = tcgetattr(STDIN_FILENO, &original_termios)) < 0 ) handle_error("ERROR: Failed to tcgetattr");

    settings = original_termios;

    // ECHO - Prevents terminal from displaying what is typed
    // ICANON - Unbuffer input.
    settings.c_lflag &= ~(ECHO | ICANON | ISIG);
    if ( (result = tcsetattr(STDIN_FILENO, TCSAFLUSH, &settings)) < 0 ) handle_error("ERROR: Failed to tcsetattr");
}

void terminal_cooked_mode(void)
{
    if ( tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_termios) < 0 ) handle_error("ERROR: Failed to tcsetattr");
}
void get_terminal_size(void)
{
    // TODO: Handle resize
    // FIX: When terminal is not fullscreen, it may occur some issues related the lines (terminal_size)
    //                              ^- not in fullscreen and the zoom creates right-side scrollbar
    struct winsize ws;
    if ( ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 ) handle_error("ERROR: Failed to ioctl");

    ct.screen_rows = ws.ws_row;
    ct.screen_columns = ws.ws_col;
}


// -----------------------------------------------------
// |                 TERMINAL DRAWING                  |
// -----------------------------------------------------
void draw_content(void)
{
    printf("\x1b[H");
    int num_lines_print = (ct.screen_rows > (int)ct.da->count)? (int)ct.da->count : ct.screen_rows - 1;

    // Change it to a while
    for ( int i = 0; i < num_lines_print; i++ )
    {
        printf("\x1b[K");
        // TODO: Properly handle the lines id here
        // printf("%d: ", i + ct.offsetY);
        string_print(ct.da->items[i + ct.offsetY]);
    }
}

void reposition_cursor(void)
{
    int curr_pointerY = ct.cursorY + ct.offsetY;
    if ( curr_pointerY < 0 ) curr_pointerY = 0;
    else if ( curr_pointerY >= (int) ct.da->count ) curr_pointerY = (int)ct.da->count - 1;

    int max_column = (int) ((string_tt) ct.da->items[curr_pointerY])->content_len;
    int curr_pointerX = ct.cursorX + ct.offsetX;
    if ( curr_pointerX < 0 ) curr_pointerX = 0;
    else if ( curr_pointerX >= max_column ) curr_pointerX = max_column;

    printf("\x1b[%d;%dH", ct.cursorY + 1, curr_pointerX + 1);
    fflush(stdout);
}

void redraw_whole_screen(void)
{
    draw_content();
    reposition_cursor();
}

void redraw_current_line(int line, int position_afterY, int position_afterX)
{
    printf("\x1b[%d;0H", ct.cursorY + 1);
    printf("\x1b[2K");
    string_print(ct.da->items[line]);
    ct.cursorY = position_afterY;
    ct.cursorX = position_afterX;
    reposition_cursor();
}
// -----------------------------------------------------
// |                     MOVEMENT                      |
// -----------------------------------------------------
void move_up()
{
    bool redraw = false;
    int top_visible_line = ct.cursorY + ct.offsetY;
    if ( top_visible_line > 0 )
    {
        if ( ct.cursorY == 0 ) { redraw = true; ct.offsetY--; }
        else if ( ct.offsetY > 0 ) ct.cursorY--;
        else if ( ct.offsetY == 0 ) ct.cursorY--;
    }
    if ( redraw ) redraw_whole_screen();
    else reposition_cursor();
}

void move_down()
{
    bool redraw = false;
    int total_lines = ct.da->count;
    int bottom_visible_line = ct.cursorY + ct.offsetY;

    if ( bottom_visible_line < total_lines )
    {
        if ( ct.cursorY == ct.screen_rows - 1)
        {
            redraw = true;
            ct.offsetY++;
        } else  ct.cursorY++;
    }
    if ( redraw ) redraw_whole_screen();
    else reposition_cursor();
}

void move_right()
{
    bool redraw = false;

    // Preventing from go beyond the string content (max_column)
    int curr_pointerY = ct.cursorY + ct.offsetY;
    int max_column = (int) ((string_tt) ct.da->items[curr_pointerY])->content_len;
    if ( (ct.cursorX + ct.offsetX) < max_column )
    {
        ct.cursorX++;
        if ( ct.cursorX > ct.screen_columns ) ct.cursorX = max_column; // Scroll
    }
    else
    {
        ct.cursorX = 0;
        move_down();
    }
    if ( redraw ) redraw_whole_screen();
    else reposition_cursor();
}

void move_left()
{
    // int curr_pointerX = ct.cursorX + ct.offsetX;
    if ( ct.cursorX > 0 )
    {
        ct.cursorX--;
        // if ( ct.cursorX < ct.screen_columns ) ct.offsetX--;
    }
    else
    {
        move_up();
        int curr_pointerY = ct.cursorY + ct.offsetY;
        int max_column = (int) ((string_tt) ct.da->items[curr_pointerY])->content_len;
        ct.cursorX = max_column;
    }
    reposition_cursor();
}


// -----------------------------------------------------
// |                     HANDLERS                      |
// -----------------------------------------------------

// TODO: Handle when trying to remove char[0].
//       If line above, append content curr_line to live_above and remove curr_line.
void handle_deletion(char read_char)
{
    int curr_pointerY = ct.cursorY + ct.offsetY;
    int curr_pointerX = ct.cursorX + ct.offsetX;
    if ( read_char == BACKSPACE_KEY )
    {
        if ( curr_pointerX == 0 )
        {
            string_tt removed_line = ((string_tt) ct.da->items[curr_pointerY]);
            darray_remove_at(ct.da, curr_pointerY);
            move_up();

            // FIX: IF "columns" is smaller than content_len, we must move it to columns
            string_append_string(ct.da->items[curr_pointerY - 1], removed_line->content);
            ct.cursorX = (int) ((string_tt) ct.da->items[curr_pointerY - 1])->content_len;
            free(removed_line->content);
            redraw_whole_screen();
        }
        else
        {
            string_delete_char_at(ct.da->items[curr_pointerY], curr_pointerX - 1);
            redraw_current_line(curr_pointerY, ct.cursorY, ct.cursorX - 1);
        }
    }
    else if ( read_char == DEL_KEY )
    {
        int max_columns = (int) ((string_tt) ct.da->items[curr_pointerY])->content_len;
        if ( curr_pointerX >= max_columns - 2 )
        {
            string_tt removed_line = ((string_tt) ct.da->items[curr_pointerY + 1]);
            darray_remove_at(ct.da, curr_pointerY + 1);

            // FIX: IF "columns" is smaller than content_len, we must move it to columns
            string_append_string(ct.da->items[curr_pointerY], removed_line->content);
            ct.cursorX = (int) ((string_tt) ct.da->items[curr_pointerY])->content_len;
            free(removed_line->content);
            redraw_whole_screen();
        }
        else
        {
            string_delete_char_at(ct.da->items[curr_pointerY], curr_pointerX);
            redraw_current_line(curr_pointerY, ct.cursorY, ct.cursorX);
        }
    }
}

void handle_typing(char read_char)
{
    int curr_pointerY = ct.cursorY + ct.offsetY;
    int curr_pointerX = ct.cursorX + ct.offsetX;
    string_append_char_at(ct.da->items[curr_pointerY], read_char, curr_pointerX);

    redraw_current_line(curr_pointerY, ct.cursorY, ct.cursorX + 1);
}

void handle_new_line(void)
{
    int curr_pointerY = ct.cursorY + ct.offsetY;
    int curr_pointerX = ct.cursorX + ct.offsetX;
    string_tt new_line = string_content_from(ct.da->items[curr_pointerY], curr_pointerX);
    darray_insert_at(ct.da, (curr_pointerY + 1), new_line);
    redraw_whole_screen();
}

void handle_cursor(char read_arrow)
{
    // TODO: treat ESC case
    // TODO: treat OTHER cases (shift-ARROWS, cntrl-ARROWS, alt-ARROWS ~ anything that might be combinable with ARROW-Keys)
    //       maybe there is a flag to be used at tcsetattr()
    // TODO: Finish SCROLL Left-Right -> Maybe word-wrap
    // TODO: Refactor movements into functions
    // TODO: Screen resize not being treated
    // TODO: curses.h has soem definitions, like "KEY_DC" that might be helpfull
    switch ( read_arrow )
    {
        case 'A': // UP
            move_up();
            break;
        case 'B': // DOWN
            move_down();
            break;
        case 'C': // RIGHT
            move_right();
            break;
        case 'D': // LEFT
            move_left();
            break;
        case '3':
            char seq;
            if ( (read(STDIN_FILENO, &seq, 1) == 1) )
            {
                if ( seq == DEL_KEY ) handle_deletion(seq);
                else UNREACHABLE("Arrows");
            }
            break;
        default: UNREACHABLE("Arrows");
    }
}

bool handle_keyboard_input(char read_char)
{
    // TODO: Allow <ENTER> to create a new line. If <ENTER> pressed in the middle of a line, the content AT cursor must be moved down
    switch ( read_char )
    {
        case '+':
            // TODO: curses.h has a database of terminals types and their escape sequences, might be better than this
            printf("\x1b[2J");
            printf("Leaving...\n");
            return true;
        case ESCAPE_SEQ_INIT:
            char seq[2];
            if ( (read(STDIN_FILENO, &seq[0], 1) == 1) &&
                 (read(STDIN_FILENO, &seq[1], 1) == 1)    )
            {
                if ( seq[0] == '[' ) handle_cursor(seq[1]);
            }
            break;
        case ENTER_KEY:
            handle_new_line();
            break;
            // Backspace
            // TODO: Check for backspace's char, not it's ascii value
        case BACKSPACE_KEY:
            handle_deletion(read_char);
            break;
        default:
            handle_typing(read_char);
            break;
    }
    return false;
}


// -----------------------------------------------------
// |                       MAIN                        |
// -----------------------------------------------------
int main(int argc, char** argv)
{
    if ( argc != 2 )
    {
        printf("Usage: ./buid/main <file_path>\n");
        exit(EXIT_FAILURE);
    }

    ct.da = darray_create(string_free, sizeof(string_tt), true);
    terminal_raw_mode();
    atexit(terminal_cooked_mode);

    // Terminal info
    get_terminal_size();

    // TODO: Ask to create file if not found
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
