#include <stdlib.h>
#include <string.h>

#include "dal.h"

#include "../utility/functions.h"

user users[NO_OF_USERS] = {0};

static void createuser(int id, char *username, char *passwd, char *role) {
  user *u = users + id;
  u->id = id;

  size_t len = strlen(passwd);
  u->password = malloc(len + 1);
  if (!u->password) {
    abort();
  }
  strncpy(u->password, passwd, len);
  u->password[len] = 0;
  u->passlen = len;

  len = strlen(username);
  u->username = malloc(len + 1);
  if (!u->username) {
    abort();
  }
  strncpy(u->username, username, len);
  u->username[len] = 0;
  u->namelen = len;

  len = strlen(role);
  u->role = malloc(len + 1);
  if (!u->role) {
    abort();
  }
  strncpy(u->role, role, len);
  u->role[len] = 0;
  u->rolelen = len;
}

void init_users() {
  createuser(0, "coolguy1", "password", "user");
  createuser(1, "theM4N", "1234", "user");
  createuser(2, "anakin69", "theforce", "user");
  createuser(3, "shregged", "yadonkey1", "user");
  createuser(4, "meouwu", "drowssap", "user");
  createuser(5, "IHateMondays", "lasagna", "user");
  createuser(6, "frodo42", "asdqwe", "user");
  createuser(7, "admin", "admin", "admin");
  createuser(8, "idk", "neverguessthis", "user");
  createuser(9, "whyme", "NotABadPasswordAtAll", "user");
}

user *login(char *username, size_t namelen, char *passwd, size_t passlen) {
  for (int i = 0; i < NO_OF_USERS; ++i) {
    user *u = users + i;
    if (streq(u->password, u->passlen, passwd, passlen) &&
        streq(u->username, u->namelen, username, namelen)) {
      return u;
    }
  }
  return NULL;
}

user *getbyid(int id){
    return users + id;
}