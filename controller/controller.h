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
int ctrl_index(request *request, smap *urlargs, response *response, smap *context);

/**
 * Controller mapped to "/about"
 */
int ctrl_about(request *request, smap *urlargs, response *response, smap *context);

/**
 * Controller mapped to "**" - matches everything that does not have a more
 * specific controller
 */
int ctrl_not_found(request *request, smap *urlargs, response *response, smap *context);

/**
 * Controller mapped to /user/{id}
 */
int ctrl_user(request *request, smap *urlargs, response *response, smap *context);

/**
 * Controller mapped to /users?name=
 */
int ctrl_list_user(request *request, smap *urlargs, response *response, smap *context);

inline int reg_controllers(cmap *controllers) {
  return 0 || cmap_reg(controllers, "/", ctrl_index) ||
         cmap_reg(controllers, "/about", ctrl_about) ||
         cmap_reg(controllers, "/users", ctrl_list_user) ||
         cmap_reg(controllers, "/user/{id}", ctrl_user) ||
         cmap_reg(controllers, "**", ctrl_not_found);
}

#endif