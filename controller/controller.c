#include <stdlib.h>

#include "context.h"
#include "controller.h"

#include "../com/request.h"
#include "../com/response.h"
#include "../dal/dal.h"
#include "../template/template.h"
#include "../types/cmap.h"
#include "../types/sb.h"
#include "../types/smap.h"
#include "../utility/error.h"
#include "../utility/functions.h"

int notfound(response *resp, smap *context) {
  create_html(resp, "not_found.html", context);
  set_resp_code(resp, "Not Found", 404);
  return geterrcode();
}

int ctrl_index(request *request,
               smap *urlargs,
               response *response,
               smap *context) {
  smap_upsert(context, VERSION_K, VERSION_V);
  create_html(response, "login.html", context);
  set_resp_code(response, "OK", 200);

  return geterrcode();
}

int ctrl_welcome(request *request,
                 smap *urlargs,
                 response *response,
                 smap *context) {
  snode *id = smap_get(urlargs, EXP_LEN("id"));
  if (!id) {
    return notfound(response, context);
  }
  user *user = getbyid(atoi(id->value));
  smap_upsert(context, EXP_LEN("username"), user->username, user->namelen);
  char buffer[16];

  snprintf(buffer, 16, "%d", user->id);
  smap_upsert(context, EXP_LEN("userid"), buffer, strlen(buffer));

  if (streq(user->role, user->rolelen, EXP_LEN("admin"))) {
    smap_upsert(context, EXP_LEN("admin"), EXP_LEN("true"));
  } else {
    smap_upsert(context, EXP_LEN("admin"), EXP_LEN("false"));
  }
  create_html(response, "welcome.html", context);
  set_resp_code(response, "OK", 200);

  return geterrcode();
}

int ctrl_not_found(request *request,
                   smap *urlargs,
                   response *response,
                   smap *context) {
  return notfound(response, context);
}

int ctrl_list_users(request *request,
                    smap *urlargs,
                    response *response,
                    smap *context) {
  char buffer[32];
  sb userids = {0};
  sbinit(&userids, NO_OF_USERS * 2);
  for (int i = 0; i < NO_OF_USERS; ++i) {
    user *user = users + i;
    snprintf(buffer, 32, "usernames[%d]", i);
    smap_upsert(context, buffer, strlen(buffer), user->username, user->namelen);
    snprintf(buffer, 32, "roles[%d]", i);
    smap_upsert(context, buffer, strlen(buffer), user->role, user->rolelen);
    if (i + 1 == NO_OF_USERS) {
      snprintf(buffer, 32, "%d", i);
    } else {
      snprintf(buffer, 32, "%d;", i);
    }
    sbappend(&userids, buffer, strlen(buffer));
  }
  smap_upsert(context, EXP_LEN("userids"), userids.data, userids.size);

  // get requesting user
  snode *userid = smap_get(&request->query, EXP_LEN("user"));
  if (!userid) {
    return notfound(response, context);
  }
  user *user = getbyid(atoi(userid->value));

  smap_upsert(context, EXP_LEN("username"), user->username, user->namelen);

  create_html(response, "list_users.html", context);
  set_resp_code(response, "OK", 200);
  sbfree(&userids);
  return geterrcode();
}

int ctrl_profile(request *request,
                 smap *urlargs,
                 response *response,
                 smap *context) {
  // get user id
  snode *userid = smap_get(urlargs, EXP_LEN("id"));
  if (!userid) {
    return notfound(response, context);
  }
  user *user = getbyid(atoi(userid->value));
  if (!user) {
    return notfound(response, context);
  }
  smap_upsert(context, EXP_LEN("username"), user->username, user->namelen);
  smap_upsert(context, EXP_LEN("role"), user->role, user->rolelen);
  if (streq(user->role, user->rolelen, EXP_LEN("admin"))) {
    smap_upsert(context, EXP_LEN("admin"), EXP_LEN("true"));
  } else {
    smap_upsert(context, EXP_LEN("admin"), EXP_LEN("false"));
  }
  create_html(response, "profile.html", context);
  set_resp_code(response, "OK", 200);
  return geterrcode();
}

int ctrl_login(request *request,
               smap *urlargs,
               response *response,
               smap *context) {
  snode *username = smap_get(&request->query, EXP_LEN("username"));
  snode *passwd = smap_get(&request->query, EXP_LEN("password"));
  if (!username || !passwd) {
    return notfound(response, context);
  }

  user *user =
      login(username->value, username->vallen, passwd->value, passwd->vallen);
  if (!user) {
    return notfound(response, context);
  }

  char urlbuilder[64];
  snprintf(urlbuilder, 64, "/welcome/%d", user->id);
  redirect(response, urlbuilder);
  return geterrcode();
}

int reg_controllers(cmap *controllers) {
  if (cmap_reg(controllers, "/", ctrl_index) ||
      cmap_reg(controllers, "/welcome/{id}", ctrl_welcome) ||
      cmap_reg(controllers, "/users", ctrl_list_users) ||
      cmap_reg(controllers, "/login", ctrl_login) ||
      cmap_reg(controllers, "/profile/{id}", ctrl_profile) ||
      cmap_reg(controllers, "**", ctrl_not_found)) {
    return RAISE_INVALID_CTRLCONF("Invalid controller configuration");
  }
  return 0;
}