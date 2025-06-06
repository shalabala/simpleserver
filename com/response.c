#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/socket.h>

#include "response.h"

#include "../configuration/const.h"
#include "../template/template.h"
#include "../types/sb.h"
#include "../types/smap.h"
#include "../utility/error.h"
#include "../utility/functions.h"

#define BUFFLEN 64
#define BUFFLEN_BIG 1024

int respinit(response *resp) {
  if (geterrcode()) {
    return geterrcode();
  }
  if (!resp) {
    return RAISE_ARGERR("cannot initialize null response");
  }
  sbinit(&resp->body, 512);
  smap_init(&resp->header, 64);

  return geterrcode();
}

static int append_header(sb *respbuilder, smap *header) {
  for (size_t i = 0; i < header->bucketsnum; ++i) {
    snode *current = header->buckets + i;
    while (current && current->key) {
      sbappend(respbuilder, current->key, current->keylen);
      sbappend(respbuilder, ": ", 2);
      sbappend(respbuilder, current->value, current->vallen);
      sbappend(respbuilder, "\r\n", 2);
      current = current->next;
    }
  }
  return geterrcode();
}

static int resp_render(sb *respbuilder, response *resp) {
  char buffer[BUFFLEN] = {0};

  sbappend(respbuilder, EXP_LEN("HTTP/1.1 "));

  snprintf(buffer, BUFFLEN, "%hu", resp->respcode.code);
  buffer[BUFFLEN - 1] = 0;
  sbappend(respbuilder, buffer, strlen(buffer));

  sbappend(respbuilder, " ", 1);
  sbappend(respbuilder, resp->respcode.name, resp->respcode.namelen);
  sbappend(respbuilder, "\r\n", 2);
  append_header(respbuilder, &resp->header);
  sbappend(respbuilder, "\r\n", 2);
  sbappend(respbuilder, resp->body.data, resp->body.size);
  return geterrcode();
}

int respsend(response *resp, int socket) {
  sb respbuilder = {0};

  sbinit(&respbuilder, 512);
  resp_render(&respbuilder, resp);

  send(socket, respbuilder.data, respbuilder.size, 0);

  sbfree(&respbuilder);
  return OK;
}

static int sb_from_file(sb *str, const char *fname) {
  FILE *f;
  f = fopen(fname, "r");
  if (!f) {
    return RAISE_FILE_NOTFOUND("Could not open file %s to read", fname);
  }

  char buff[BUFFLEN_BIG];
  while (fgets(buff, BUFFLEN_BIG, f)) {
    size_t len = strlen(buff);
    if (buff[len - 1] == '\n' && (len == 1 || buff[len - 2] != '\r')) {
      sbappend(str, buff, len - 1);
      sbappend(str, EXP_LEN("\r\n"));
    } else {
      sbappend(str, buff, len);
    }
  }
  return geterrcode();
}

static int get_full_filename(sb *fnamebuff, char *file) {
  sbappend(fnamebuff, EXP_LEN(FILEROOT));
  if (file[0] != '/') {
    sbappend(fnamebuff, EXP_LEN("/"));
  }
  sbappend(fnamebuff, file, strlen(file));
  return geterrcode();
}

int create_html(response *resp, char *file, smap *context) {
  int error;
  sb filename = {0}, template = {0};
  char buffer[BUFFLEN];

  if ((error = sbinit(&filename, 64))) {
    return error;
  }
  if ((error = sbinit(&template, 256))) {
    sbfree(&filename);
    return error;
  }

  get_full_filename(&filename, file);
  sb_from_file(&template, filename.data);

  render_template(&resp->body, template.data, template.size, context);
  smap_upsert(&resp->header,
              EXP_LEN("Content-Type"),
              EXP_LEN("text/html; charset=UTF-8"));
  snprintf(buffer, BUFFLEN, "%lu", resp->body.size);
  buffer[BUFFLEN - 1] = 0;

  smap_upsert(&resp->header, EXP_LEN("Content-Length"), buffer, strlen(buffer));

  sbfree(&filename);
  sbfree(&template);

  return geterrcode();
}

void respfree(response *resp) {
  if (resp) {
    sbfree(&resp->body);
    smap_free(&resp->header);
  }
}

int redirect(response *resp, char *whereto) {
  set_resp_code(resp, "Found", 302);
  smap_upsert(&resp->header, EXP_LEN("Location"), whereto, strlen(whereto));
  smap_upsert(&resp->header, EXP_LEN("Content-Length"), "0", 1);
  return geterrcode();
}

void set_resp_code(response *resp, char *name, uint16_t code) {
  resp->respcode.code = code;
  size_t len = MIN(15, strlen(name));
  strncpy(resp->respcode.name, name, len);
  resp->respcode.name[len] = 0;
  resp->respcode.namelen = len;
}

int respclear(response *resp) {
  smap_clear(&resp->header);
  sbclear(&resp->body);
  resp->respcode.code = 0;
  resp->respcode.namelen = 0;
  memset(resp->respcode.name, 0, RESP_CODE_NAME_LEN);
  return geterrcode();
}

int respprint(response *resp){
  sb respbuilder = {0};

  sbinit(&respbuilder, 512);
  resp_render(&respbuilder, resp);
  printf("------RESP----------\n");
  printf("%s\n", respbuilder.data);
  printf("--------------------\n");

  sbfree(&respbuilder);
  return OK;
}