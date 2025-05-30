#ifndef REQUEST_H
#define REQUEST_H

#include "../types/sb.h"
#include "../types/smap.h"

/**
 * Represents the HTTP methods supported by the server.
 */
typedef enum _method { GET } method;

/**
 * Represents a parsed HTTP request.
 */
typedef struct _request {
  /**
   * The HTTP method of the request.
   */
  method method;

  /**
   * The requested resource. like /users?name=JohnDoe. 
   * Includes the query string as well.
   */
  sb resource;

  /**
   * The headers of the request.
   */
  smap header;

  /**
   * The query parameters of the request.
   */
  smap query;
} request;


/**
 * Initializes and parses the request from the string buffer.
 * @param req The request structure to fill with parsed data.
 * @param str The string buffer containing the request data.
 */
int parse(request *req, sb *str);

/**
 * Frees the memory allocated for the request structure.
 * @param req The request structure to free.
 */
void reqfree(request *req);

/**
 * Prints the request as a string.
 */
void reqprint(request *req);
#endif // REQUEST_H