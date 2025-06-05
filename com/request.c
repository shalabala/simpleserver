#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "../configuration/const.h"
#include "../types/sb.h"
#include "../utility/error.h"
#include "../utility/functions.h"
#include "request.h"

static method getmethod(char *methodName, size_t len) {
  if (streq(methodName, len, "GET", 3)) {
    return GET;
  }

  return RAISE_UNSPMETH("Methods other than GET are not supported");
}

static int reqinit(request *req) {
  int error;

  if (!req) {
    return RAISE_ARGERR("cannot initialize null request");
  }

  if ((error = sbinit(&req->resource, 16))) {
    return error;
  }

  if ((error = smap_init(&req->header, 64))) {
    return error;
  }

  if ((error = smap_init(&req->query, 64))) {
    return error;
  }

  return OK;
}

static int parse_method(request *req, sb *str, size_t *cursor) {
  size_t i = 0;
  // getting the method
  while (i < str->size && str->data[i] != ' ') {
    ++i;
  }

  method m = getmethod(str->data, i);
  if (m < 0) {
    return m; // serves as errorcode as well
  }

  req->method = m;
  *cursor = i;
  return OK;
}

static int parse_resource(request *req, sb *str, size_t *cursor) {
  size_t i = *cursor;
  int error;
  // get the requested resource
  size_t res_start = ++i;
  while (i < str->size && str->data[i] != ' ') {
    ++i;
  }

  if ((error =
           sbappend(&req->resource, str->data + res_start, i - res_start))) {
    return error;
  }
  *cursor = i;
  return OK;
}

static void newline(sb *str, size_t *cursor) {
  size_t i = *cursor;
  while (i < str->size && str->data[i] != '\n') {
    ++i;
  }
  *cursor = ++i;
}

static void flushws(sb *str, size_t *cursor) {
  size_t i = *cursor;
  while (i < str->size && (str->data[i] == ' ' || str->data[i] == '\t')) {
    ++i;
  }
  *cursor = i;
}

static int parse_query(request *req) {
  size_t i = 0;
  int error;
  while (i < req->resource.size && req->resource.data[i] != '?') {
    ++i;
  }

  ++i;
  if (i >= req->resource.size) {
    return OK;
  }

  while (i < req->resource.size) {
    size_t keystart = i;
    while (i < req->resource.size && req->resource.data[i] != '=') {
      ++i;
    }
    size_t keylen = i - keystart;
    ++i;
    if (i >= req->resource.size) {
      if ((error = smap_upsert(
               &req->query, req->resource.data + keystart, keylen, "", 0))) {
        return error;
      }
      return OK;
    }
    size_t valstart = i;
    while (i < req->resource.size && req->resource.data[i] != '&') {
      ++i;
    }
    size_t vallen = i - valstart;
    if ((error = smap_upsert(&req->query,
                             req->resource.data + keystart,
                             keylen,
                             req->resource.data + valstart,
                             vallen))) {
      return error;
    }
    ++i;
  }
  return OK;
}

static int parse_header(request *req, sb *str, size_t *cursor) {
  size_t i = *cursor;
  int error;
  while (i < str->size) {
    size_t keystart = i;
    while (i < str->size && str->data[i] != ':') {
      if (str->data[i] == '\r' || str->data[i] == '\n') {
        if (i == keystart) {
          // empty line - reached end of header
          newline(str, &i);
          *cursor = i;
          return OK;
        } else {
          // no colon -- malformed header
          RAISE_MALFORMEDREQ("in header reached newline before reaching colon");
        }
      }
      ++i;
    }
    size_t keylen = i - keystart;
    ++i;
    flushws(str, &i);
    if (i >= str->size) {
      if ((error = smap_upsert(
               &req->header, str->data + keystart, keylen, "", 0))) {
        return error;
      }
    }
    size_t valstart = i;
    while (i < str->size && str->data[i] != '\n' && str->data[i] != '\r') {
      ++i;
    }
    size_t vallen = i - valstart;
    if ((error = smap_upsert(&req->header,
                             str->data + keystart,
                             keylen,
                             str->data + valstart,
                             vallen))) {
      return error;
    }
    newline(str, &i);
  }
  *cursor = i;
  return OK;
}

int parse(request *req, sb *str) {
  size_t i = 0;
  int error;

  if ((error = reqinit(req))) {
    return error;
  }

  // getting the method
  if ((error = parse_method(req, str, &i))) {
    return error;
  }

  if ((error = parse_resource(req, str, &i))) {
    return error;
  }

  if ((error = parse_query(req))) {
    return error;
  }

  // we ignore http version here,
  // this is just a stupid hobby project not a
  // full-fledged webserver
  newline(str, &i);

  if ((error = parse_header(req, str, &i))) {
    return error;
  }

  return OK;
}

void reqfree(request *req) {
  if (req) {
    smap_free(&req->header);
    smap_free(&req->query);
    sbfree(&req->resource);
  }
}

static void print_method(method m) {
  switch (m) {
  case GET:
    printf("GET");
    break;

  default:
    printf("[WARNING UNKNOWN METHOD %d]", m);
    break;
  }
}

void reqprint(request *req) {
  if (!req) {
    printf("Request is NULL\n");
    return;
  }
  printf("--------REQ---------\n");
  printf("Resource: %s\nMethod: ", req->resource.data);
  print_method(req->method);
  printf("\nQuery: \n");
  smap_print(&req->query);
  printf("Header: \n");
  smap_print(&req->header);
  printf("--------------------\n");
}
