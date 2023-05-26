#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "game_engine.h"
#include "common.h"
#include "save_load.h"

#define ASCII_SHIFT     ('A' - 2)
#define HEADERS_SIZE    (2)
#define DIR_UP          (1)
#define DIR_DOWN        (2)
#define DIR_LEFT        (3)
#define DIR_RIGH        (4)
#define DIR_UP_DOWN     (5)
#define DIR_LEFT_RIGHT  (6)
#define SAVE_N_QUIT     (7)
#define QUIT            (8)
#define USER_MOVE       (9)

#define refreshScreen(session)  \
    do {                        \
        usleep(200000);         \
        clearScreen();          \
        printGrid(session);     \
    } while (0)

char symboles_table[MAX_SYBLS] = 
{
    'Q',
    'X',
    'U',
    'I',
    'V',
    'O',
};

static void printColoredSymb(char symbl)
{
    for (int i = 0; i < MAX_SYBLS; i++)
    {
        if (symbl == symboles_table[i])
        {
            switch (i)
            {
                case 0:
                    printf("\e[1;32m" "%-*c", 2, symbl);
                    break;
                case 1:
                    printf("\e[1;34m" "%-*c", 2, symbl);
                    break;
                case 2:
                    printf("\e[1;31m" "%-*c", 2, symbl);
                    break;
                case 3:
                    printf("\e[1;36m" "%-*c", 2, symbl);
                    break;
                case 4:
                    printf("\e[1;35m" "%-*c", 2, symbl);
                    break;
                case 5:
                    printf("\e[1;33m" "%-*c", 2, symbl);
                    break;
            }
            printf("\e[0m");
            return;
        }
    }
    printf("  ");
}

static void printGrid(game_session_t *session)
{
    for (int i = 0; i < session->lines + HEADERS_SIZE; i++)
    {
        for (int j = 0; j < session->cls + HEADERS_SIZE; j++)
        {
            if (i == 0)
            {
                if (j == 0 || j == 1) 
                    printf("  ");
                else
                    printf("%-*c", 2, ASCII_SHIFT + j);
            }
            else if (i == 1) 
            {
                if (j == 0 || j == 1)
                    printf("  ");
                else
                    printf("%-*c", 2, '-');
            }
            else 
            {
                if (j == 0)
                    printf("%-*d", 2, i - 1);
                else if (j == 1)
                    printf("%-*c", 2, '|');
                else
                    printColoredSymb(session->grid[i - HEADERS_SIZE][j - HEADERS_SIZE]);
            }
        }
        printf("\n");
    }
    printf("SCORE: %d\n", session->score);
}

static void fillGridPos(game_session_t *session, int line, int col)
{
    int rand_symbole = 0;
    char skip_symbole_x = 0, skip_symbole_y;
    int prev_pos = -1, prev_prev_pos = -1;
    int next_pos = -1, next_next_pos = -1;

    /* Set symbols nav for colums */
    prev_pos = col == 0 ? session->cls - 1 : col - 1;
    prev_prev_pos = prev_pos == 0 ? session->cls - 1 : prev_pos - 1;

    next_pos = (col + 1) % session->cls;
    next_next_pos = (next_pos + 1) % session->cls;

    /* Check if any symbol to ignore */
    if (session->grid[line][prev_pos] == session->grid[line][prev_prev_pos])
    {
        skip_symbole_x = session->grid[line][prev_pos];
    }
    if (skip_symbole_x == 0 && session->grid[line][prev_pos] == session->grid[line][next_pos])
    {
        skip_symbole_x = session->grid[line][prev_pos];
    }
    if (skip_symbole_x == 0 && session->grid[line][next_pos] == session->grid[line][next_next_pos])
    {
        skip_symbole_x = session->grid[line][next_pos];
    }

    /* Set symbols nav for lines */
    prev_pos = line == 0 ? session->lines - 1 : line - 1;
    prev_prev_pos = prev_pos == 0 ? session->lines - 1 : prev_pos - 1;

    next_pos = (line + 1) % session->lines;
    next_next_pos = (next_pos + 1) % session->lines;

    /* Check if any symbol to ignore */
    if (session->grid[prev_pos][col] == session->grid[prev_prev_pos][col])
    {
        skip_symbole_y = session->grid[prev_pos][col];
    }
    if (skip_symbole_y == 0 && session->grid[prev_pos][col] == session->grid[next_pos][col])
    {
        skip_symbole_y = session->grid[prev_pos][col];
    }
    if (skip_symbole_y == 0 && session->grid[next_pos][col] == session->grid[next_next_pos][col])
    {
        skip_symbole_y = session->grid[next_pos][col];
    }

    /* Get a valid symbole */
    while (1)
    {
        rand_symbole = rand() % session->symboles_count;
        if (symboles_table[rand_symbole] != skip_symbole_x && symboles_table[rand_symbole] != skip_symbole_y) 
            break;
    }
    session->grid[line][col] = symboles_table[rand_symbole];
}

static void buildGrid(game_session_t *session)
{
    assert(session != NULL && session->grid == NULL);
    /* Create lines */
    session->grid = (char **) malloc(sizeof(char *) * session->lines);
    /* Create colums */
    for (int i = 0; i < session->lines; i++)
    {
        session->grid[i] = (char *) malloc(session->cls);
        memset(session->grid[i], 0, session->cls);
    }
    /* Fill the grid */
    for (int i = 0; i < session->lines; i++)
    {
        for (int j = 0; j < session->cls; j++)
        {
            fillGridPos(session, i, j);
        }
    }
}

static game_session_t *buildGameSession(size_t lines, size_t cols, size_t symboles_count, char username[USER_MAX_BUFF])
{
    game_session_t *g_session = (game_session_t *) malloc(sizeof(game_session_t));
    assert(g_session != NULL);
    time_t temp = time(NULL);
    /* Copy data */
    g_session->grid = NULL;
    g_session->cls = cols;
    g_session->lines = lines;
    g_session->symboles_count = symboles_count;
    g_session->game_over = 0;
    g_session->exit_game = 0;
    g_session->start_time = mktime(localtime(&temp));
    memcpy(g_session->username, username, USER_MAX_BUFF);
    /* Build the grid */
    buildGrid(g_session);    
    return g_session;
}

static int checKallignedSymboles(game_session_t *session, int current_line, int current_colum, char move_direction, int *alligned_direction)
{
    assert(session != NULL && alligned_direction != NULL);

    int prev_pos = -1, prev_prev_pos = -1;
    int next_pos = -1, next_next_pos = -1;
    int next_col = current_colum, next_line = current_line;

    /* Check for valid args */
    if (current_colum < 0 || current_colum >= session->cls || current_line < 0 || current_line >= session->lines) 
        return 0;
    /* Check if the move worth points */
    if (move_direction == 'G')
    {
        /* Chcek if the move is possible */
        if (current_colum == 0) return 0;
        /* Set nav indexes for colums */
        next_col = current_colum - 1;
        prev_pos = next_col == 0 ? session->cls - 1 : next_col - 1;
        prev_prev_pos = prev_pos == 0 ? session->cls - 1 : prev_pos - 1;

        /* Check if the move woth points for lines */
        if (session->grid[current_line][prev_pos] == session->grid[current_line][prev_prev_pos] && session->grid[current_line][current_colum] == session->grid[current_line][prev_pos])
        {
            *alligned_direction = DIR_LEFT;
            return DIR_LEFT;
        }
        /* Set nav indexes for lines */
        prev_pos = current_line == 0 ? session->lines - 1 : current_line - 1;
        prev_prev_pos = prev_pos == 0 ? session->lines - 1 : prev_pos - 1;
        next_pos = (current_line + 1) % session->lines;
        next_next_pos = (next_pos + 1) % session->lines;
        if (session->grid[prev_pos][next_col] == session->grid[prev_prev_pos][next_col] && session->grid[current_line][current_colum] == session->grid[prev_pos][next_col])
        {
            *alligned_direction = DIR_UP;
            return DIR_LEFT;
        }
        if (session->grid[prev_pos][next_col] == session->grid[next_pos][next_col] && session->grid[current_line][current_colum] == session->grid[prev_pos][next_col])
        {
            *alligned_direction = DIR_UP_DOWN;
            return DIR_LEFT;
        }
        if (session->grid[next_pos][next_col] == session->grid[next_next_pos][next_col] && session->grid[current_line][current_colum] == session->grid[next_pos][next_col])
        {
            *alligned_direction = DIR_DOWN;
            return DIR_LEFT;
        }
    }

    if (move_direction == 'D')
    {
        /* Chcek if the move is possible */
        if (current_colum == session->cls - 1) return 0;
        /* Set nav indexes for colums */
        next_col = current_colum + 1;
        next_pos = (next_col + 1) % session->lines;
        next_next_pos = (next_pos + 1) % session->lines;

        /* Check if the move woth points for lines */
        if (session->grid[current_line][next_pos] == session->grid[current_line][next_next_pos] && session->grid[current_line][current_colum] == session->grid[current_line][next_pos])
        {
            *alligned_direction = DIR_RIGH;
            return DIR_RIGH;
        }
        /* Set nav indexes for lines */
        prev_pos = current_line == 0 ? session->lines - 1 : current_line - 1;
        prev_prev_pos = prev_pos == 0 ? session->lines - 1 : prev_pos - 1;
        next_pos = (current_line + 1) % session->lines;
        next_next_pos = (next_pos + 1) % session->lines;
        if (session->grid[prev_pos][next_col] == session->grid[prev_prev_pos][next_col] && session->grid[current_line][current_colum] == session->grid[prev_pos][next_col])
        {
            *alligned_direction = DIR_UP;
            return DIR_RIGH;
        }
        if (session->grid[prev_pos][next_col] == session->grid[next_pos][next_col] && session->grid[current_line][current_colum] == session->grid[prev_pos][next_col])
        {
            *alligned_direction = DIR_UP_DOWN;
            return DIR_RIGH;
        }
        if (session->grid[next_pos][next_col] == session->grid[next_next_pos][next_col] && session->grid[current_line][current_colum] == session->grid[next_pos][next_col])
        {
            *alligned_direction = DIR_DOWN;
            return DIR_RIGH;
        }
    }

    if (move_direction == 'H')
    {
        /* Chcek if the move is possible */
        if (current_line == 0) return 0;
        /* Set nav indexes for colums */
        next_line = current_line - 1;
        prev_pos = next_line == 0 ? session->cls - 1 : next_line - 1;
        prev_prev_pos = prev_pos == 0 ? session->cls - 1 : prev_pos - 1;

        /* Check if the move woth points for lines */
        if (session->grid[prev_pos][current_colum] == session->grid[prev_prev_pos][current_colum] && session->grid[current_line][current_colum] == session->grid[prev_pos][current_colum])
        {
            *alligned_direction = DIR_UP;
            return DIR_UP;
        }
        /* Set nav indexes for colums */
        prev_pos = current_colum == 0 ? session->cls - 1 : current_colum - 1;
        prev_prev_pos = prev_pos == 0 ? session->cls - 1 : prev_pos - 1;
        next_pos = (current_colum + 1) % session->cls;
        next_next_pos = (next_pos + 1) % session->cls;
        
        if (session->grid[next_line][prev_pos] == session->grid[next_line][prev_prev_pos] && session->grid[current_line][current_colum] == session->grid[next_line][prev_pos])
        {
            *alligned_direction = DIR_LEFT;
            return DIR_UP;
        }
        if (session->grid[next_line][prev_pos] == session->grid[next_line][next_pos] && session->grid[current_line][current_colum] == session->grid[next_line][prev_pos])
        {
            *alligned_direction = DIR_LEFT_RIGHT;
            return DIR_UP;
        }
        if (session->grid[next_line][next_pos] == session->grid[next_line][next_next_pos] && session->grid[current_line][current_colum] == session->grid[next_line][next_pos])
        {
            *alligned_direction = DIR_RIGH;
            return DIR_UP;
        }
    }

    if (move_direction == 'B')
    {
        /* Chcek if the move is possible */
        if (current_line == session->lines - 1) return 0;
        /* Set nav indexes for colums */
        next_line = current_line + 1;
        next_pos = (next_line + 1) % session->lines;
        next_next_pos = (next_pos + 1) % session->lines;

        /* Check if the move woth points for lines */
        if (session->grid[next_pos][current_colum] == session->grid[next_next_pos][current_colum] && session->grid[current_line][current_colum] == session->grid[next_pos][current_colum])
        {
            *alligned_direction = DIR_DOWN;
            return DIR_DOWN;
        }
        /* Set nav indexes for colums */
        prev_pos = current_colum == 0 ? session->cls - 1 : current_colum - 1;
        prev_prev_pos = prev_pos == 0 ? session->cls - 1 : prev_pos - 1;
        next_pos = (current_colum + 1) % session->cls;
        next_next_pos = (next_pos + 1) % session->cls;
        
        if (session->grid[next_line][prev_pos] == session->grid[next_line][prev_prev_pos] && session->grid[current_line][current_colum] == session->grid[next_line][prev_pos])
        {
            *alligned_direction = DIR_LEFT;
            return DIR_DOWN;
        }
        if (session->grid[next_line][prev_pos] == session->grid[next_line][next_pos] && session->grid[current_line][current_colum] == session->grid[next_line][prev_pos])
        {
            *alligned_direction = DIR_LEFT_RIGHT;
            return DIR_DOWN;
        }
        if (session->grid[next_line][next_pos] == session->grid[next_line][next_next_pos] && session->grid[current_line][current_colum] == session->grid[next_line][next_pos])
        {
            *alligned_direction = DIR_RIGH;
            return DIR_DOWN;
        }
    }
    /* Move is not possible */
    return 0;
}

static void updateGrid(game_session_t *session, int line, int col, int move, int parse_direc)
{
    int parser = 0;
    char temp_symbl = 0;
    int temp_line = line, temp_col = col;
    int empty_s = -1, empty_e = -1, cpy_step = 0, left_sbl = -1;

    /* Get the temp symbole location */
    switch (move)
    {
        case DIR_UP:
            temp_line--;
            break;
        case DIR_DOWN:
            temp_line++;
            break;
        case DIR_RIGH:
            temp_col++;
            break;
        case DIR_LEFT:
            temp_col--;
            break;
    }
    /* Swap the choosen symbole */
    temp_symbl = session->grid[temp_line][temp_col];
    session->grid[temp_line][temp_col] = session->grid[line][col];
    session->grid[line][col] = temp_symbl;
    temp_symbl = session->grid[temp_line][temp_col];

    refreshScreen(session);
    
    /* Distroy the matched symboles and count the points */
    if (parse_direc == DIR_UP)
    {
        parser = temp_line;
        while (1)
        {
            if (session->grid[parser][temp_col] == temp_symbl)
            {
                session->grid[parser][temp_col] = 0;
                session->score++;
            }
            else
                break;
            parser =  parser == 0 ? session->lines - 1 : parser - 1;
        }
    }
    if (parse_direc == DIR_DOWN)
    {
        parser = temp_line;
        while (1)
        {
            if (session->grid[parser][temp_col] == temp_symbl)
            {
                session->grid[parser][temp_col] = 0;
                session->score++;
            }
            else
                break;
            parser =  (parser + 1) % session->lines;
        }
    }
    if (parse_direc == DIR_LEFT)
    {
        parser = temp_col;
        while (1)
        {
            if (session->grid[temp_line][parser] == temp_symbl)
            {
                session->grid[temp_line][parser] = 0;
                session->score++;
            }
            else
                break;
            parser =  parser == 0 ? session->cls - 1 : parser - 1;
        }
    }
    if (parse_direc == DIR_RIGH)
    {
        parser = temp_col;
        while (1)
        {
            if (session->grid[temp_line][parser] == temp_symbl)
            {
                session->grid[temp_line][parser] = 0;
                session->score++;
            }
            else
                break;
            parser =  (parser + 1) % session->cls;
        }
    }
    if (parse_direc == DIR_LEFT_RIGHT)
    {
        int init_col = temp_col;
        parser = (init_col + 1) % session->cls;
        /* Going right first */
        while (1)
        {
            if (session->grid[temp_line][parser] == temp_symbl)
            {
                session->grid[temp_line][parser] = 0;
                session->score++;
            }
            else
                break;
            parser =  (parser + 1) % session->cls;
        }
        /* Then going left */
        parser = init_col;
        while (1)
        {
            if (session->grid[temp_line][parser] == temp_symbl)
            {
                session->grid[temp_line][parser] = 0;
                session->score++;
            }
            else
                break;
            parser =  parser == 0 ? session->cls - 1 : parser - 1;
        }
    }
    if (parse_direc == DIR_UP_DOWN)
    {
        int init_line = temp_line;
        parser = (init_line + 1) % session->lines;
        /* Going down first */
        while (1)
        {
            if (session->grid[parser][temp_col] == temp_symbl)
            {
                session->grid[parser][temp_col] = 0;
                session->score++;
            }
            else
                break;
            parser =  (parser + 1) % session->lines;
        }
        /* Then going up */
        parser = init_line;
        while (1)
        {
            if (session->grid[parser][temp_col] == temp_symbl)
            {
                session->grid[parser][temp_col] = 0;
                session->score++;
            }
            else
                break;
            parser =  parser == 0 ? session->lines - 1 : parser - 1;
        }
    }

    refreshScreen(session);

    /* Fill empty spaces in the grid */
    /* Gravity is making symbols shift to the top */
    for (int i = 0; i < session->cls; i++)
    {
        /* Get the empty zone delimiters */
        for (int j = 0; j < session->lines; j++)
        {
            if (session->grid[j][i] == 0 && empty_s == -1)
                empty_s = j;
            if (session->grid[j][i] != 0 && empty_s != -1 && empty_e == -1)
                empty_e = j - 1;
            if (session->grid[j][i] == 0 && empty_e != -1)
            {
                left_sbl = j - empty_e - 1;
                break;
            }
        }

        if (empty_s == -1) continue;
        if (empty_e == - 1) empty_e = session->lines - 1;
        if (empty_e && left_sbl == -1) left_sbl = session->lines - empty_e - 1;
        
        cpy_step = empty_e - empty_s + 1;
        int x = empty_s;
        
        /* Shift symboles */
        for (; x < empty_s + left_sbl; x++)
        {
            session->grid[x][i] = session->grid[x + cpy_step][i];
            session->grid[x + cpy_step][i] = 0;
            refreshScreen(session);
        }
        /* Place new symboles */
        for (int j = x; j < session->lines - 1; j++)
        {
            session->grid[j][i] = 0;
        }
        refreshScreen(session);
        for (; x < session->lines; x++)
        {
            fillGridPos(session, x, i);
            refreshScreen(session);
        }
        empty_e = -1;
        empty_s = -1;
    }
}

static void handleUserMove(game_session_t *session, char user_buffer[USER_MAX_BUFF], int *parse_direc, int *user_line, int *user_colum)
{   
    int alligned_direction = 0;
    int move = 0;
    char *cell_details = strtok(user_buffer, " ");
    char *direction = strtok(NULL, " ");

    if (cell_details == NULL || direction == NULL) 
        return;

    int col = cell_details[0] - 'A';
    int line = atoi(&cell_details[1]) - 1;

    /* Check if the user enties are good */
    if (col < 0 || col >= session->cls || line < 0 || line >= session->lines)
        return;
    
    /* Check if the move is possible in the grid */
    move = checKallignedSymboles(session, line, col, direction[0], &alligned_direction);
    if (move == 0) return;
    /* Update the grid */
    updateGrid(session, line, col, move, alligned_direction);
}

static int handleUserEntry(char user_buffer[USER_MAX_BUFF])
{
    if (strcmp(user_buffer, "save") == 0) return SAVE_N_QUIT;
    if (strcmp(user_buffer, "exit") == 0) return QUIT;
    return USER_MOVE;
}

static int isGameOver(game_session_t *session)
{
    /* unused is used only to provid all args for checKallignedSymboles */ 
    int unused;
    for (int i = 0; i < session->lines; i++)
    {
        for (int j = 0; j < session->cls; j++)
        {
            if (checKallignedSymboles(session, i, j, 'H', &unused) != 0) return 0;
            if (checKallignedSymboles(session, i, j, 'B', &unused) != 0) return 0;
            if (checKallignedSymboles(session, i, j, 'D', &unused) != 0) return 0;
            if (checKallignedSymboles(session, i, j, 'G', &unused) != 0) return 0;
        }
    }
    /* Game over */
    return 1;
}

static void mainGameLoop(game_session_t *session)
{
    char user_buffer[USER_MAX_BUFF];
    int parse_direc = 0, user_move = 0, line = 0, col = 0;

    while (1)
    {
        clearScreen();
        printGrid(session);
        /* Handle user move */
        while (1)
        {
            /* Get user choice and move direction */
            printf("Pour enregister la parite, tapez: save\n");
            printf("Pour quitter la partie, tapez: exit\n");
            printf("Quelle case voulez vous déplacer (ex: C8 H) ? : ");
            userSafeRead(user_buffer, USER_MAX_BUFF);
            user_move = handleUserEntry(user_buffer);
            /* Handle moves */ 
            if (user_move == SAVE_N_QUIT)
            {
                saveSession(session);
                return;
            }
            if (user_move == QUIT)
            {
                session->exit_game = 1;
                return;
            }
            handleUserMove(session, user_buffer, &parse_direc, &line, &col);
            if (isGameOver(session))
            {
                refreshScreen(session);
                session->game_over = 1;
                printf("Fin du jeu, plus aucun mouvement n'est possibe\n");
                printf("Typez une touche...");
                fgetc(stdin);
                return;
            }
        }
    }
}

static void getGridSize(size_t *lines, size_t *cols, size_t *symbls_count, char username[USER_MAX_BUFF])
{
    assert(lines != NULL && cols != NULL);
    char user_entry[USER_MAX_BUFF];

    /* Get grid lines count */
    do 
    {
        printf("Lines:\t");
        userSafeRead(user_entry, USER_MAX_BUFF);
        if ((*lines = atoi(user_entry)) > 0) break;
        printf("Invalid entry, please try again !\n");
    } while (1);

    /* Get grid colums count */
    do 
    {
        printf("Colums:\t");
        userSafeRead(user_entry, USER_MAX_BUFF);
        if ((*cols = atoi(user_entry)) > 0) break;
        printf("Invalid entry, please try again !\n");
    } while (1);

    /* Get symbols count */
    do 
    {
        printf("Symbols (%d-%d):\t", MAX_SYBLS, MIN_SYBLS);
        userSafeRead(user_entry, USER_MAX_BUFF);
        *symbls_count = atoi(user_entry);
        if (*symbls_count >= MIN_SYBLS && *symbls_count <= MAX_SYBLS) break;
        printf("Invalid entry, please try again !\n");
    } while (1);

    /* Get username */
    printf("Username: ");
    userSafeRead(username, USER_MAX_BUFF);
}

game_session_t *gameEngine_LoadGame(void)
{
    /* Load the game */
    clearScreen();
    printf("\t\t *** Chargement du jeu ***\n");
    game_session_t *new_session = NULL;
    char username[USER_MAX_BUFF];
    do
    {
        printf("Username: ");
        userSafeRead(username, USER_MAX_BUFF);
        new_session = loadSession(username);
    } while (new_session == NULL);
    mainGameLoop(new_session);
    return new_session;
}

game_session_t *gameEngine_startGame(void)
{
    clearScreen();
    printf("\t\t *** Lancement du jeu ***\n");
    size_t lines = 0, cols = 0, symb_count = 0;
    char username[USER_MAX_BUFF];
    /* Get session params */
    getGridSize(&lines, &cols, &symb_count, username);
    /* Build new session */
    game_session_t *new_session = buildGameSession(lines, cols, symb_count, username);
    mainGameLoop(new_session);
    return new_session;
}