#ifndef DAL_H
#define DAL_H

#include <string.h>

#define NO_OF_USERS 10

typedef struct _user {
  int id;
  char *username;
  size_t namelen;
  char *password;
  size_t passlen;
  char *role;
  size_t rolelen;
} user;

extern user users[];

void init_users();

void free_users();

user *login(char *username, size_t namelen, char *passwd, size_t passlen);

user *getbyid(int id);

#endif