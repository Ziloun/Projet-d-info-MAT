#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <fcntl.h>
#include "save_load.h"

typedef struct
{
    size_t lines;
    size_t cls;
    size_t symboles_count;
    char username[USER_MAX_BUFF];
    int score;
} session_header_t;

void saveSession(game_session_t *session)
{
    assert(session != NULL);
    /* Set the header */
    session_header_t header = 
    {
        .lines = session->lines,
        .cls = session->cls,
        .symboles_count = session->symboles_count,
        .score = session->score
    };
    memcpy(header.username, session->username, USER_MAX_BUFF);
    int file = open(session->username, O_WRONLY | O_CREAT, 0666);
    /* Check the save file */
    if (file < 0)
    {
        fprintf(stderr, "Unable to save the game\n");
        return;
    }
    /* Write the header */
    write(file, &header, sizeof(session_header_t));
    /* Write the map */
    for (int i = 0; i < session->lines; i++)
    {
        write(file, session->grid[i], session->cls);
    }
    close(file);
}

game_session_t *loadSession(char *username)
{
    assert(username != NULL);
    int file = open(username, O_RDONLY);
    game_session_t *session = NULL;
    session_header_t header = {0};
    /* Open the file */
    if (file < 0)
    {
        fprintf(stderr, "Unable to load the game\n");
        return NULL;
    }
    /* Create the session */
    session = (game_session_t *) malloc(sizeof(game_session_t));
    memset(session, 0, sizeof(game_session_t));
    /* Load the header */
    memset(&header, 0, sizeof(session_header_t));
    read(file, &header, sizeof(session_header_t));
    session->lines = header.lines;
    session->cls = header.cls;
    session->symboles_count = header.symboles_count;
    session->score = header.score;
    memcpy(session->username, header.username, USER_MAX_BUFF);
    /* Create the grid */
    session->grid = (char **) malloc(sizeof(char *) * session->lines);
    for (int i = 0; i < session->lines; i++)
    {
        session->grid[i] = (char *) malloc(session->cls);
        read(file, session->grid[i], session->cls);
    }
    close(file);
    return session;
}