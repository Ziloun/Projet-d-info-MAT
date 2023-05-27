#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "game_engine.h"
#include "common.h"

#define MAX_SYBLS       (6)
#define MIN_SYBLS       (4)

typedef struct
{
    char username[USER_MAX_BUFF];
    int score;
    time_t play_time;
} live_record_t;

void clearScreen(void)
{
    system("clear");
}

void userSafeRead(char *buffer, size_t buffer_size)
{
    char *eol_parser = NULL;
    if (buffer == NULL) return;
    memset(buffer, 0, buffer_size);
    /* Read data from user */
    if (fgets(buffer, buffer_size, stdin) == NULL)
    {
        fprintf(stderr, "Error while reading from user\n");
        exit(EXIT_FAILURE);
    }
    eol_parser = strchr(buffer, '\n');
    /* Remove \n from user data if exist */
    if (eol_parser)
    {
        *eol_parser = '\0';
        return;
    }
    /* clean stdin buffer */
    while (fgetc(stdin) != '\n');
}


static void distroySession(game_session_t **session)
{
    /* Destroy colums */
    for (int i = 0; i < (*session)->lines; i++)
        free((*session)->grid[i]);
    /* Destroy lines */
    free((*session)->grid);
    /* Destroy the session pointer */
    free(*session);
}

static void displayRecords(live_record_t *list, int size)
{
    if (list == NULL) return;
    int max_score = 0;
    int index = 0;
    time_t current_time = time(NULL);
    current_time = mktime(localtime(&current_time));
    printf("\t\t *** Le meilleur scores ***\n");
    for (int i = 0; i < size; i++)
    {
        if (list[i].score > max_score) 
        {
            max_score = list[i].score;
            index = i;
        }
    }
    printf("SCORE : %d\n", max_score);
    printf("TIME  : %.0f seconds\n", difftime(current_time, list[index].play_time));
}

int main(int argc, char **argv)
{
    char buffer[USER_MAX_BUFF];
    int start = 0, load = 0, record_list_size = 0;
    live_record_t *records_list = NULL;
    game_session_t *session = NULL;

    /* Init random */
    srand(time(NULL));
    while (1)
    {
        clearScreen();
        printf("\t\t *** Welcom to CY-Crush saga ***\n");
    
        do
        {
            printf("To start a game type : start\n");
            printf("To load a game type  : load\n");
            printf("> ");
            userSafeRead(buffer, USER_MAX_BUFF);

            if (strcmp(buffer, "start") == 0) start = 1;
            if (strcmp(buffer, "load") == 0) load = 1;

        } while (start == 0 && load == 0);
        
        printf("Please type any key to enter to the game\n");
        fgetc(stdin);

        if (load == 1) 
            session = gameEngine_LoadGame();
        else 
            session = gameEngine_startGame();
        
        /* End of game, store the score and username if gameover */
        if (session->game_over)
        {
            clearScreen();
            record_list_size++;
            records_list = (live_record_t *) realloc(records_list, record_list_size * sizeof(live_record_t));
            records_list[record_list_size - 1].score = session->score;
            records_list[record_list_size - 1].play_time = session->start_time;
            memcpy(records_list[record_list_size - 1].username, session->username, USER_MAX_BUFF);
            displayRecords(records_list, record_list_size);
        }
        if (session->exit_game)
            break;
        /* Destroy the session */
        distroySession(&session);
    }
    return 0;
}
