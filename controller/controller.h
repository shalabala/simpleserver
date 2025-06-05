#ifndef CONTROLLER_H
#define CONTROLLER_H
#include "../com/request.h"
#include "../com/response.h"
#include "../types/cmap.h"
#include "../types/sb.h"
#include "../types/smap.h"
#include "../utility/functions.h"

/**
 * Controller mapped to "/"
 */
int ctrl_index(request *request,
               smap *urlargs,
               response *response,
               smap *context);

/**
 * Controller mapped to "/welcome/{id}"
 */
int ctrl_welcome(request *request,
                 smap *urlargs,
                 response *response,
                 smap *context);

/**
 * Controller mapped to "**" - matches everything that does not have a more
 * specific controller
 */
int ctrl_not_found(request *request,
                   smap *urlargs,
                   response *response,
                   smap *context);

/**
 * Controller mapped to /users
 */
int ctrl_list_users(request *request,
                    smap *urlargs,
                    response *response,
                    smap *context);

/**
 *Controller mapped to /login
 */
int ctrl_login(request *request,
               smap *urlargs,
               response *response,
               smap *context);

inline int reg_controllers(cmap *controllers) {
  if (cmap_reg(controllers, "/", ctrl_index) ||
      cmap_reg(controllers, "/welcome/{id}", ctrl_welcome) ||
      cmap_reg(controllers, "/users", ctrl_list_users) ||
      cmap_reg(controllers, "/login", ctrl_login) ||
      cmap_reg(controllers, "**", ctrl_not_found)) {
    return RAISE_INVALID_CTRLCONF("Invalid controller configuration");
  }
  return 0;
}

#endif