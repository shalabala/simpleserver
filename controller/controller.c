#include "controller.h"
#include "context.h"

#include "../com/request.h"
#include "../com/response.h"
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
                create_html(response, "index.html", context));

  return OK;
}

int ctrl_about(request *request,
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

int ctrl_user(request *request,
              smap *urlargs,
              response *response,
              smap *context) {
  return -1; // not implemented
}

int ctrl_list_user(request *request,
                   smap *urlargs,
                   response *response,
                   smap *context) {
  return -1; // not implemented
}
