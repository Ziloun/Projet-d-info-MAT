#ifndef _SAVE_LOAD_H_
#define _SAVE_LOAD_H_

#include "game_engine.h"

void saveSession(game_session_t *session);
game_session_t *loadSession(char *username);

#endif