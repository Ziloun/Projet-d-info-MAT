#ifndef __GAME_ENGINE_H__
#define __GAME_ENGINE_H__

#define MAX_SYBLS       (6)
#define MIN_SYBLS       (4)
#define USER_MAX_BUFF   (20)

extern char symboles_table[MAX_SYBLS];

typedef struct
{
    size_t lines;
    size_t cls;
    size_t symboles_count;
    char username[USER_MAX_BUFF];
    char **grid;
    int score;
    int game_over;
    int exit_game;
    time_t start_time;
} game_session_t;

game_session_t *gameEngine_startGame(void);
game_session_t *gameEngine_LoadGame(void);

#endif