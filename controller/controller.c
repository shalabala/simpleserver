#include "controller.h"
#include "context.h"

#include "../com/request.h"
#include "../com/response.h"
#include "../dal/dal.h"
#include "../template/template.h"
#include "../types/cmap.h"
#include "../types/sb.h"
#include "../types/smap.h"
#include "../utility/error.h"
#include "../utility/functions.h"

int ctrl_index(request *request,
               smap *urlargs,
               response *response,
               smap *context) {
  error_checked(smap_upsert(context, VERSION_K, VERSION_V) ||
                create_html(response, "login.html", context));

  return OK;
}

int ctrl_welcome(request *request,
                 smap *urlargs,
                 response *response,
                 smap *context) {
  return -1; // not implemented
}

int ctrl_not_found(request *request,
                   smap *urlargs,
                   response *response,
                   smap *context) {
  return -1; // not implemented
}

int ctrl_list_users(request *request,
                    smap *urlargs,
                    response *response,
                    smap *context) {
  return -1; // not implemented
}

int ctrl_login(request *request,
               smap *urlargs,
               response *response,
               smap *context) {
  snode *username = smap_get(&request->query, EXP_LEN("username"));
  snode *passwd = smap_get(&request->query, EXP_LEN("password"));
  if (!username || !passwd) {
    create_html(response, "not_found", context);
    return geterrcode();
  }

  user *user =
      login(username->value, username->vallen, passwd->value, passwd->vallen);
  if (!user) {
    create_html(response, "not_found", context);
    set_resp_code(response, "Not Found", 404);
    return geterrcode();
  }

  sb urlbuilder = {0};
  char buffer[16];
  sbinit(&urlbuilder, 32);
  sbappend(&urlbuilder, EXP_LEN("/welcome/"));
  snprintf(buffer, 16, "%d", user->id);
  buffer[15] = 0;
  sbappend(&urlbuilder, buffer, strlen(buffer));
  redirect(response, urlbuilder.data);
  sbfree(&urlbuilder);
  return geterrcode();
}