CC 		= gcc
CFLAGS 	= -g -Wall -Werror
LDFLAGS =
SRC		= ui.c game_engine.c save_load.h
OBJ		= game


all: $(OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -c $^ -o $@

$(OBJ): ui.o game_engine.o save_load.o
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -rf $(OBJ) *.o