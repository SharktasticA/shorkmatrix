/*
    ######################################################
    ##            SHORKTAINMENT - SHORKMATRIX           ##
    ######################################################
    ## A vertical scrolling text screensaver. It        ##
    ## provides "digital rain", inspired by the 1999    ## 
    ## film "The Matrix" and Abishek V Ashok's CMatrix. ##
    ######################################################
    ## Licence: GNU GENERAL PUBLIC LICENSE Version 3    ##
    ######################################################
    ## Kali (links.sharktastica.co.uk)                  ##
    ######################################################
*/



#include <fcntl.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>



typedef struct
{
    int col;
    int row;
    int len;
    int vis;
} Droplet;



#define COL_BAK_BLACK           "40"
#define COL_BAK_BLUE            "44"
#define COL_BAK_CYAN            "46"
#define COL_BAK_GREEN           "42"
#define COL_BAK_GREY            "40"
#define COL_BAK_MAGENTA         "45"
#define COL_BAK_RED             "41"
#define COL_BAK_WHITE           "47"
#define COL_BAK_YELLOW          "43"

#define COL_FOR_BLACK           "0;30"
#define COL_FOR_BLUE            "0;34"
#define COL_FOR_BOLD_BLUE       "1;34"
#define COL_FOR_BOLD_CYAN       "1;36"
#define COL_FOR_BOLD_GREEN      "1;32"
#define COL_FOR_BOLD_MAGENTA    "1;35"
#define COL_FOR_BOLD_RED        "1;31"
#define COL_FOR_BOLD_WHITE      "1;37"
#define COL_FOR_BOLD_YELLOW     "1;33"
#define COL_FOR_CYAN            "0;36"
#define COL_FOR_GREEN           "0;32"
#define COL_FOR_GREY            "1;30"
#define COL_FOR_MAGENTA         "0;35"
#define COL_FOR_RED             "0;31"
#define COL_FOR_WHITE           "0;37"
#define COL_FOR_YELLOW          "0;33"

#define COL_RESET               "0"
#define COL_FOR_RESET           "39"
#define COL_BAK_RESET           "49"

#define MAX_WORDS               27
#define MAX_WORD_LEN            16
#define MIN_HEIGHT              12
#define MIN_WIDTH               32



static char *COL_TRAIL = COL_FOR_BLUE;
static char *COL_HEAD = COL_FOR_BOLD_CYAN;
static int DOUBLE_HEAD = 1;
static Droplet *DROPLETS;
static int MONO = 0;
static struct termios OLD_TERMIOS;
const char PREDEFINED_WORDS[MAX_WORDS][MAX_WORD_LEN] = {
    "ADMIRALSHARK",
    "BEAMSPRING",
    "BUCKLINGSLEEVE",
    "BUCKLINGSPRING",
    "CHIMAERA",
    "CHONDRICHTHYES",
    "ELASMOBRANCHII",
    "HOLOCEPHALI",
    "KALI",
    "LINUX",
    "MODEL1A",
    "MODEL1B",
    "MODELB",
    "MODELF",
    "MODELG",
    "MODELM",
    "RAY",
    "SAWFISH",
    "SHARK",
    "SHARKTASTICA",
    "SHORK",
    "SHORK486",
    "SHORKDISKETTE",
    "SHORKMATRIX",
    "SHORKTAINMENT",
    "SHORKUTILS",
    "SKATE"
};
static int SCREEN_HEIGHT;
static int SCREEN_WIDTH;
static struct winsize TERM_SIZE;



/**
 * Moves the cursor to topleft-most position and clears below cursor.
 */
void clearScreen(void)
{
    printf("\033[H\033[J");
}

/**
 * Creates a droplet.
 * @param col The droplet's column
 * @param vis Flags if this droplet should be visible
 * @returns An initialised Droplet struct
 */
Droplet createDroplet(int col, int vis)
{
    Droplet l;
    l.col = col;
    l.len = 6 + rand() % (SCREEN_HEIGHT / 2 - 6);
    l.row = -l.len;
    l.vis = vis;
    return l;
}

/**
 * Enables the terminal's canonical input. Used only when the program exits.
 */
void disableRawMode(void)
{
    tcsetattr(STDIN_FILENO, TCSANOW, &OLD_TERMIOS);
}

/**
 * Disables the terminal's canonical input so that things like getchar do not
 * wait until enter is pressed.
 */
void enableRawMode(void)
{
    struct termios newTERMIO;
    tcgetattr(STDIN_FILENO, &OLD_TERMIOS);
    newTERMIO = OLD_TERMIOS;
    newTERMIO.c_lflag &= ~(ICANON | ECHO);
    newTERMIO.c_cc[VMIN] = 1;
    newTERMIO.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &newTERMIO);
}

/**
 * Files in our screen matrix with random ASCII characters and some predefined words, and initialises the flags for first if first droplet has passed.
 * @param screen The screen matrix
 * @param firstDropletDone The flags for if first droplet has passed
 */
void fillScreen(char screen[SCREEN_HEIGHT][SCREEN_WIDTH], int firstDropletDone[SCREEN_WIDTH])
{
    // Start by randomly filling in the screen with ASCII characters
    for (int i = 0; i < SCREEN_HEIGHT; i++)
        for (int j = 0; j < SCREEN_WIDTH; j++)
            screen[i][j] = (char)(32 + rand() % (126 - 32 + 1));

    // Then lets add some of our predefined words as "easter eggs"
    // Also initialise firstDropletDone whilst we're at it
    for (int i = 0; i < SCREEN_WIDTH; i++)
    {
        firstDropletDone[i] = 0;

        // 1/3 chance of proceeding
        if (rand() % 3 != 0) continue;

        // Pick a word
        int wordI = rand() % MAX_WORDS;
        const char *word = PREDEFINED_WORDS[wordI];

        // Make sure it fits out height
        int len = 0;
        while (len < MAX_WORD_LEN && word[len] != '\0') len++;
        if (len > SCREEN_HEIGHT) continue;

        // Pick the starting row
        int startRow = rand() % (SCREEN_HEIGHT - len + 1);

        // Insert the word
        for (int j = 0; word[j] != '\0'; j++)
            screen[startRow + j][i] = word[j];
    }
}

/**
 * Adds new lines to a given string based on the requested line width.
 * @param buffer Input string
 * @param width Characters per line
 * @param indent Indent to include after newly inserted new line
 * @return Number of lines in the string
 */
int formatNewLines(char *buffer, int width, char *indent)
{
    if (!buffer || width < 1) return 0;

    size_t bufferStrLen = strlen(buffer);
    size_t indentLen = indent ? strlen(indent) : 0;
    int lines = 1;
    int lastSpace = -1;
    int widthCount = 1;

    for (int i = 0; i < bufferStrLen; i++)
    {
        if (buffer[i] == '\033')
        {
            while (i < bufferStrLen && buffer[i] != 'm') i++;
            if (i >= bufferStrLen) break;
            continue; 
        }
        
        if (buffer[i] == ' ') lastSpace = i;
        else if (buffer[i] == '\n')
        {
            lines++;
            widthCount = 0;
            continue;
        }

        if (widthCount == width)
        {
            if (lastSpace != -1)
            {
                buffer[lastSpace] = '\n';
                lines++;

                if (indent && indentLen > 0)
                {
                    memmove(buffer + lastSpace + 1 + indentLen, buffer + lastSpace + 1, bufferStrLen - lastSpace);
                    memcpy(buffer + lastSpace + 1, indent, indentLen);
                    bufferStrLen += indentLen;
                    if (lastSpace <= i) i += indentLen;
                }
            }
            widthCount = i - lastSpace;
        }

        widthCount++;
    }

    return lines;
}

/**
 * @return winsize struct containing the current terminal size in columns and rows
 */
struct winsize getTerminalSize(void)
{
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0)
    {
        ws.ws_col = 80;
        ws.ws_row = 24;
    }
    return ws;
}

/**
 * Prints a frame of current droplets without colour support.
 * @param screen The screen matrix
 * @param firstDropletDone The flags for if first droplet has passed
 */
void printFrameNoCol(char screen[SCREEN_HEIGHT][SCREEN_WIDTH], int firstDropletDone[SCREEN_WIDTH])
{
    for (int i = 0; i < SCREEN_WIDTH; i++)
    {
        Droplet *dl = &DROPLETS[i];

        // Check if we need a new droplet
        if (dl->len == 0)
        {
            int spawnChance = 2;

            // This helps prevent the a massive burst of droplets at the beginning
            if (!firstDropletDone[i])
                spawnChance = (30 > 0 ? 30 : 10);
            
            if (rand() % spawnChance == 0)
            {
                *dl = createDroplet(i, 1);
                firstDropletDone[i] = 1;
            }

            continue;
        }

        //if (dl->vis)
        {
            // Erase the character above the droplet's head
            if (dl->row > 0)
            {
                int eraseRow = dl->row - 1;
                if (eraseRow >= 0 && eraseRow < SCREEN_HEIGHT)
                {
                    printf("\033[%d;%dH ", eraseRow + 1, dl->col + 1);
                }
            }

            // Draw the droplet
            for (int j = 0; j < dl->len; j++)
            {
                int row = dl->row + j;
                if (row >= 0 && row < SCREEN_HEIGHT)
                {
                    char ch = screen[row][dl->col];
                    printf("\033[%d;%dH%c", row + 1, dl->col + 1, ch);
                }
            }

            fflush(stdout);
        }

        dl->row++;

        //if (dl->vis)
        {
            // See if the droplet may be out of the screen
            if (dl->row >= SCREEN_HEIGHT)
            {
                // Flags this column can have a new droplet
                dl->len = 0;

                // Erase the last trial
                int lastBottom = dl->row + dl->len - 1;
                if (lastBottom >= 0 && lastBottom < SCREEN_HEIGHT)
                {
                    printf("\033[%d;%dH ", lastBottom + 1, dl->col + 1);
                }
            }
        }
    }
}

/**
 * Prints a frame of current droplets with colour support.
 * @param screen The screen matrix
 * @param firstDropletDone The flags for if first droplet has passed
 */
void printFrameCol(char screen[SCREEN_HEIGHT][SCREEN_WIDTH], int firstDropletDone[SCREEN_WIDTH])
{
    for (int i = 0; i < SCREEN_WIDTH; i++)
    {
        Droplet *dl = &DROPLETS[i];

        // Check if we need a new droplet
        if (dl->len == 0)
        {
            int spawnChance = 2;

            // This helps prevent the a massive burst of droplets at the beginning
            if (!firstDropletDone[i])
                spawnChance = (30 > 0 ? 30 : 10);
            
            if (rand() % spawnChance == 0)
            {
                *dl = createDroplet(i, 1);
                firstDropletDone[i] = 1;
            }

            continue;
        }

        //if (dl->vis)
        {
            // Erase the character above the droplet's head
            if (dl->row > 0)
            {
                int eraseRow = dl->row - 1;
                if (eraseRow >= 0 && eraseRow < SCREEN_HEIGHT)
                {
                    printf("\033[%d;%dH ", eraseRow + 1, dl->col + 1);
                }
            }

            // Draw the bottom of the head
            int bottomMost = dl->row + dl->len - 1;
            if (bottomMost < SCREEN_HEIGHT)
            {
                char ch = screen[bottomMost][dl->col];
                printf("\033[%d;%dH\033[%sm%c\033[%sm", bottomMost + 1, dl->col + 1, COL_HEAD, ch, COL_RESET);
            }

            // Draw a second character of the head (if flagged to do so)
            int bottomSecond = bottomMost - 1;
            if (DOUBLE_HEAD && bottomSecond >= 0 && bottomSecond < SCREEN_HEIGHT)
            {
                char ch = screen[bottomSecond][dl->col];
                printf("\033[%d;%dH\033[%sm%c\033[%sm", bottomSecond + 1, dl->col + 1, COL_HEAD, ch, COL_RESET);
            }

            // Draw the trail
            int bodyRow = DOUBLE_HEAD ? (bottomSecond - 1) : (bottomMost - 1);
            if (bodyRow >= 0 && bodyRow < SCREEN_HEIGHT)
            {
                char ch = screen[bodyRow][dl->col];
                printf("\033[%d;%dH\033[%sm%c\033[%sm", bodyRow + 1, dl->col + 1, COL_TRAIL, ch, COL_RESET);
            }
            
            fflush(stdout);
        }

        dl->row++;

        //if (dl->vis)
        {
            // See if the droplet may be out of the screen
            if (dl->row >= SCREEN_HEIGHT)
            {
                // Flags this column can have a new droplet
                dl->len = 0;

                // Erase the last trial
                int lastBottom = dl->row + dl->len - 1;
                if (lastBottom >= 0 && lastBottom < SCREEN_HEIGHT)
                {
                    printf("\033[%d;%dH ", lastBottom + 1, dl->col + 1);
                }
            }
        }
    }
}

/**
 * Prints the entire screen matrix on screen. Intended for debugging.
 * @param screen The screen matrix
 */
void printMatrix(char screen[SCREEN_HEIGHT][SCREEN_WIDTH])
{
    printf("\033[H");
    for (int i = 0; i < SCREEN_HEIGHT; i++)
    {
        fwrite(screen[i], 1, SCREEN_WIDTH, stdout);
        if (i < SCREEN_HEIGHT - 1)
            putchar('\n');
    }
    fflush(stdout);
}

/**
 * Makes the terminal cursor visible again, resets the terminal's colours and clears the screen
 * upon exiting.
 */
void onExit(void)
{
    free(DROPLETS);
    disableRawMode();
    printf("\033[?25h");
    printf("\033[%sm", COL_RESET);
    clearScreen();
}

/**
 * Used to handle exiting controllably if SIGINT is received (Ctrl+C).
 */
void onSigInt(int sig)
{
    exit(0);
}

void showHelp(void)
{
    char desc[150] = "A \"digital rain\" vertical scrolling text screensaver, inspired by the 1999 film \"The Matrix\" and Abishek V Ashok's CMatrix.\n";
    formatNewLines(desc, TERM_SIZE.ws_col, NULL);
    printf("%s\n", desc);

    char usage[40] = "Usage: shorkmatrix [OPTIONS]\n\n";
    formatNewLines(usage, TERM_SIZE.ws_col, NULL);
    printf("%s", usage);

    char options[670] = "\
Options:\n\
-h, --help            Displays help information and exits\n\
-g, --green           Changes the droplet colour to green\n\
-ma, --magenta        Changes the droplet colour to magenta\n\
-mo, --mono           Disables colour support and lighter heads\n\
-nc, --no-clear       Prevents clearing the terminal before starting\n\
-r, --red             Changes the droplet colour to red\n\
-sh, --single-head    Makes the lighter head of the droplets one character long instead of two\n\
-u, --update          Custom draw update control value (be must positive whole number)\n\
-y, --yellow          Changes the droplet colour to yellow\n\n";
    formatNewLines(options, TERM_SIZE.ws_col, "                      ");
    printf("%s", options);

    char notes[80];
    snprintf(notes, 80, "Notes:\nThe host terminal size must be %dx%d before starting.\n", MIN_WIDTH, MIN_HEIGHT);
    formatNewLines(notes, TERM_SIZE.ws_col, NULL);
    printf("%s", notes);
}



int main(int argc, char *argv[])
{
    TERM_SIZE = getTerminalSize();
    int noClear = 0;
    int update = 40000;

    for (int i = 1; i < argc; i++)
    {
        if ((strcmp(argv[i], "-h") == 0) || (strcmp(argv[i], "--help") == 0))
        {
            showHelp();
            return 0;
        }
        else if ((strcmp(argv[i], "-g") == 0) || (strcmp(argv[i], "--green") == 0))
        {
            COL_TRAIL = COL_FOR_GREEN;
            COL_HEAD = COL_FOR_WHITE;
        }
        else if ((strcmp(argv[i], "-ma") == 0) || (strcmp(argv[i], "--magenta") == 0))
        {
            COL_TRAIL = COL_FOR_MAGENTA;
            COL_HEAD = COL_FOR_BOLD_MAGENTA;
        }
        else if ((strcmp(argv[i], "-mo") == 0) || (strcmp(argv[i], "--mono") == 0))
        {
            MONO = 1;
            COL_TRAIL = COL_FOR_RESET;
            COL_HEAD = COL_FOR_RESET;
        }
        else if ((strcmp(argv[i], "-nc") == 0) || (strcmp(argv[i], "--no-clear") == 0))
        {
            noClear = 1;
        }
        else if ((strcmp(argv[i], "-r") == 0) || (strcmp(argv[i], "--red") == 0))
        {
            COL_TRAIL = COL_FOR_RED;
            COL_HEAD = COL_FOR_BOLD_RED;
        }
        else if ((strcmp(argv[i], "-sh") == 0) || (strcmp(argv[i], "--single-head") == 0))
        {
            DOUBLE_HEAD = 0;
        }
        else if ((strcmp(argv[i], "-u") == 0) || (strcmp(argv[i], "--update") == 0))
        {
            if (i + 1 >= argc)
            {
                printf("ERROR: update value is missing\n");
                return 1;
            }

            char *endptr = NULL;
            long val = strtol(argv[i + 1], &endptr, 10);

            if (*endptr != '\0' || val <= 0)
            {
                printf("ERROR: update value must be a positive whole number\n");
                return 1;
            }

            update = (int)val;
            continue;
        }
        else if ((strcmp(argv[i], "-y") == 0) || (strcmp(argv[i], "--yellow") == 0))
        {
            COL_TRAIL = COL_FOR_YELLOW;
            COL_HEAD = COL_FOR_BOLD_YELLOW;
        }
    }

    if (TERM_SIZE.ws_col < MIN_WIDTH || TERM_SIZE.ws_row < MIN_HEIGHT)
    {
        printf("ERROR: terminal size too small (must be %dx%d or more)\n", MIN_WIDTH, MIN_HEIGHT);
        return 1;
    }



    atexit(onExit);
    signal(SIGINT, onSigInt);
    srand(time(NULL));
    printf("\033[?25l");
    enableRawMode();
    
    SCREEN_WIDTH = TERM_SIZE.ws_col;
    SCREEN_HEIGHT = TERM_SIZE.ws_row;
    DROPLETS = calloc(SCREEN_WIDTH, sizeof(Droplet));

    char screen[SCREEN_HEIGHT][SCREEN_WIDTH];
    int firstDropletDone[SCREEN_WIDTH]; 
    fillScreen(screen, firstDropletDone);
    if (!noClear) clearScreen();

    if (MONO)
    {
        while (1)
        {
            printFrameNoCol(screen, firstDropletDone);
            usleep(update);
        }
    }
    else
    {
        while (1)
        {
            printFrameCol(screen, firstDropletDone);
            usleep(update);
        }
    }

    return 0;
}
