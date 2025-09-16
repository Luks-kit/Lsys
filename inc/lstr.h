#ifndef LSTR_H
#define LSTR_H

#include <stddef.h>

size_t lstrlen(const char* s);
int lstrcmp(const char* a, const char* b);
int lstrncmp(const char* a, const char* b, size_t len);
void lstrcpy(char* dest, const char* src);
void lstrcat(char* dest, const char* src);
void litoa(int value, char* buf);

#endif

