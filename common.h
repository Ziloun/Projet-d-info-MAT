#ifndef __COMMON_H__
#define __COMMON_H__

#define MIN(a, b)   ((a) > (b) ? (b) : (a))
#define SLEEP_SEC   (2)

void userSafeRead(char *buffer, size_t buffer_size);
void clearScreen(void);

#endif