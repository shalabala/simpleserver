#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../com/coordination.h"
#include "../com/request.h"
#include "../com/response.h"
#include "../template/template.h"
#include "../types/cmap.h"
#include "../types/sb.h"
#include "../types/smap.h"
#include "../utility/error.h"
#include "../utility/functions.h"

/**
 * Run the test.
 */
#define TEST(func)                                                             \
  cleargloberr();                                                              \
  printf("starting test: [%s]\n", #func);                                      \
  func();                                                                      \
  printf("finished successfully: [%s]\n", #func);

/**
 * Test assert
 */
#define check(expr) do_assert(expr, #expr)
#define check_success(expr) check((expr) == 0)
#define check_fail(expr) check((expr) != 0)
#define check_notnull(expr) check((expr) != NULL)
#define check_null(expr) check((expr) == NULL)

void do_assert(bool expr, char *repr) {
  if (!expr) {
    if (haserr()) {
      fprintf(stderr,
              "[FAILED] Error has occured whilst evaluating expression [%s]: "
              "%s\n",
              repr,
              globerr->description);
    } else {
      fprintf(stderr, "[FAILED] Expression [%s] evaluated to false\n", repr);
    }
    abort();
  }
}

void test_sb() {
  // test basic functionality
  sb buff;
  static char teststr[] =
      "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed euismod, "
      "urna eu tincidunt consectetur, nisi nisl aliquam enim, eget facilisis "
      "sapien sapien nec erat. Pellentesque habitant morbi tristique senectus "
      "et netus et malesuada fames ac turpis egestas. Suspendisse "
      "potenti.";
  check(sbinit(&buff, 12) == 0);
  check(buff.capacity == 16);
  size_t len = strlen(teststr), i;
  for (i = 0; i + 8 < len; i += 8) {
    check(sbappend(&buff, teststr + i, 8) == 0);
    check(buff.data[buff.size] == 0);
  }

  if (i < len) {
    check(sbappend(&buff, teststr + i, len - i) == 0);
  }
  check(buff.data[buff.size] == 0);

  check(buff.capacity >= buff.size);
  check(buff.size == len);
  check(strncmp(teststr, buff.data, len) == 0);
  check(sbinsert(&buff, 3, "HelloWorld", 10) == 0);
  check(buff.size == len + 10);
  check(buff.data[buff.size] == 0);
  check(strncmp("HelloWorld", buff.data + 3, 10) == 0);
  sbfree(&buff);
}

void test_smap() {
  smap map;
  snode *node;

  smap_init(&map, 63);
  check(map.bucketsnum == 64);
  check(map.mask == 63);
  check(map.size == 0);

  // INSERT & UPDATE
  char key[16];
  char value[64];
  for (size_t i = 0; i < 200; ++i) {
    // INSERT

    // keyformat would be Hello_1 etc
    snprintf(key, 16, "Hello_%ld", i);
    // longer or shorter keys
    snprintf(
        value, 64, "World_%s_%ld", i % 2 == 0 ? "longlonglongvalue" : "sv", i);
    // we havent inserted - should be null
    check_null(smap_get(&map, key, strlen(key)));
    check_success(smap_upsert(&map, key, strlen(key), value, strlen(value)));
    check_notnull(node = smap_get(&map, key, strlen(key)));
    check(streq(key, strlen(key), node->key, node->keylen));
    check(streq(value, strlen(value), node->value, node->vallen));
    check(map.size == i + 1);

    // UPDATE
    snprintf(value, 64, "UpdatedValue_%ld", i);
    check_success(smap_upsert(&map, key, strlen(key), value, strlen(value)));
    check_notnull(node = smap_get(&map, key, strlen(key)));
    check(streq(key, strlen(key), node->key, node->keylen));
    check(streq(value, strlen(value), node->value, node->vallen));
    check(map.size == i + 1);
  }
  // test empty value
  strncpy(key, "newkey", 16);
  check_success(smap_upsert(&map, key, strlen(key), "", 0));
  check_notnull(node = smap_get(&map, key, strlen(key)));
  check(streq(node->value, node->vallen, "", 0));

  check_success(smap_del(&map, key, strlen(key)));
  check_null(node = smap_get(&map, key, strlen(key)));

  check_fail(smap_del(&map, EXP_LEN("KEYNOTFOUND")));
  check(geterrcode() == ITEM_NOTFOUND);

  // test empty key
  check_fail(smap_upsert(&map, "", 0, value, strlen(value)));
  smap_free(&map);
}

int user_ctrl(request *req, smap *urlparams, response *resp, smap *context) {
  snode *id = smap_get(urlparams, "id", 2);
  check_notnull(id);
  return atoi(id->value);
}

int user123_ctrl(request *req, smap *urlparams, response *resp, smap *context) {
  return 42;
}

void create_dummy_request(request *req, char *resource) {
  if (!req->resource.data) {
    if (sbinit(&req->resource, 16)) {
      fprintf(stderr, "Could not create dummy request");
      abort();
    }
  } else {
    sbclear(&req->resource);
  }
  if (sbappend(&req->resource, resource, strlen(resource))) {
    fprintf(stderr, "Could not create dummy request");
    abort();
  }
}

void test_cmap() {
  cmap map = {0};
  centry *ctrl;
  smap urlargs = {0};
  request dummyreq = {0};

  check_success(smap_init(&urlargs, 64));
  check_success(cmap_init(&map, 10));
  check_success(cmap_reg(&map, "users/{id}", user_ctrl));
  check_notnull(ctrl = cmap_match(&map, "/users/123", 10));
  check_success(
      parseurl("/users/123", 10, ctrl->path, ctrl->pathlen, &urlargs));
  create_dummy_request(&dummyreq, "/users/123");
  check(ctrl->c(&dummyreq, &urlargs, NULL, NULL) == 123);

  smap_clear(&urlargs);
  check_success(cmap_reg(&map, "users/123", user123_ctrl));
  check_notnull(ctrl = cmap_match(&map, "/users/123", 10));
  check_success(
      parseurl("/users/123", 10, ctrl->path, ctrl->pathlen, &urlargs));
  check(ctrl->c(&dummyreq, &urlargs, NULL, NULL) == 42);

  smap_clear(&urlargs);
  create_dummy_request(&dummyreq, "/users/456");
  check_notnull(ctrl = cmap_match(&map, "/users/456", 10));
  check_success(
      parseurl("/users/456", 10, ctrl->path, ctrl->pathlen, &urlargs));
  check(ctrl->c(&dummyreq, &urlargs, NULL, NULL) == 456);

  ctrl = NULL;
  reqfree(&dummyreq);
  cmap_free(&map);
  smap_free(&urlargs);
}

int index_ctrl_specific(request *req,
                        smap *urlparams,
                        response *resp,
                        smap *context) {
  return 52;
}

int index_ctrl_anything(request *req,
                        smap *urlparams,
                        response *resp,
                        smap *context) {
  return 62;
}
void test_cmap_emptypath() {
  cmap map = {0};
  centry *ctrl;
  smap urlargs = {0};
  request req = {0};

  check_success(smap_init(&urlargs, 64));
  check_success(cmap_init(&map, 10));
  check_success(cmap_reg(&map, "{anything}", index_ctrl_anything));
  create_dummy_request(&req, "/");
  check_notnull(ctrl = cmap_match(&map, req.resource.data, req.resource.size));
  check_success(parseurl(req.resource.data,
                         req.resource.size,
                         ctrl->path,
                         ctrl->pathlen,
                         &urlargs));
  check(ctrl->c(&req, &urlargs, NULL, NULL) == 62);

  check_success(cmap_reg(&map, "/", index_ctrl_specific));
  create_dummy_request(&req, "/");
  check_notnull(ctrl = cmap_match(&map, req.resource.data, req.resource.size));
  check_success(parseurl(req.resource.data,
                         req.resource.size,
                         ctrl->path,
                         ctrl->pathlen,
                         &urlargs));
  check(ctrl->c(&req, &urlargs, NULL, NULL) == 52);

  ctrl = NULL;
  reqfree(&req);
  cmap_free(&map);
  smap_free(&urlargs);
}

int glob_ctrl(request *req, smap *urlparams, response *resp, smap *context) {
  return 72;
}
int var_ctrl(request *req, smap *urlparams, response *resp, smap *context) {
  return 82;
}
int match_any(request *req, smap *urlparams, response *resp, smap *context) {
  return 92;
}

void test_cmap_globbing() {
  cmap map = {0};
  centry *ctrl = NULL;
  request req = {0};
  smap urlargs = {0};

  check_success(smap_init(&urlargs, 64));
  check_success(cmap_init(&map, 10));

  // pattern: '*' should match '/' '/foo' but not '/foo/bar'
  check_success(cmap_reg(&map, "*", glob_ctrl));
  create_dummy_request(&req, "/");
  check_notnull(ctrl = cmap_match(&map, req.resource.data, req.resource.size));
  check_success(parseurl(req.resource.data,
                         req.resource.size,
                         ctrl->path,
                         ctrl->pathlen,
                         &urlargs));
  check(ctrl->c(&req, &urlargs, NULL, NULL) == 72);
  create_dummy_request(&req, "/foo");
  check_notnull(ctrl = cmap_match(&map, req.resource.data, req.resource.size));
  check_success(parseurl(req.resource.data,
                         req.resource.size,
                         ctrl->path,
                         ctrl->pathlen,
                         &urlargs));
  check(ctrl->c(&req, &urlargs, NULL, NULL) == 72);
  create_dummy_request(&req, "/foo/bar");
  check_null(ctrl = cmap_match(&map, req.resource.data, req.resource.size));

  // should have same meaning as previous one
  cmap_clear(&map);
  check_success(cmap_reg(&map, "/*", glob_ctrl));
  create_dummy_request(&req, "/");
  check_notnull(ctrl = cmap_match(&map, req.resource.data, req.resource.size));
  check_success(parseurl(req.resource.data,
                         req.resource.size,
                         ctrl->path,
                         ctrl->pathlen,
                         &urlargs));
  check(ctrl->c(&req, &urlargs, NULL, NULL) == 72);
  create_dummy_request(&req, "/foo");
  check_notnull(ctrl = cmap_match(&map, req.resource.data, req.resource.size));
  check_success(parseurl(req.resource.data,
                         req.resource.size,
                         ctrl->path,
                         ctrl->pathlen,
                         &urlargs));
  check(ctrl->c(&req, &urlargs, NULL, NULL) == 72);
  create_dummy_request(&req, "/foo/bar");
  check_null(ctrl = cmap_match(&map, req.resource.data, req.resource.size));

  // should match '/' '/asd' but not /foo/bar.
  // should be more specific than /*
  check_success(cmap_reg(&map, "{any}", var_ctrl));
  create_dummy_request(&req, "/");
  check_notnull(ctrl = cmap_match(&map, req.resource.data, req.resource.size));
  check_success(parseurl(req.resource.data,
                         req.resource.size,
                         ctrl->path,
                         ctrl->pathlen,
                         &urlargs));
  check(ctrl->c(&req, &urlargs, NULL, NULL) == 82);
  create_dummy_request(&req, "/foo/bar");
  check_null(ctrl = cmap_match(&map, req.resource.data, req.resource.size));

  // should match anything that does not
  // otherwise have a mapping
  check_success(cmap_reg(&map, "**", match_any));
  create_dummy_request(&req, "/foo");
  check_notnull(ctrl = cmap_match(&map, req.resource.data, req.resource.size));
  check_success(parseurl(req.resource.data,
                         req.resource.size,
                         ctrl->path,
                         ctrl->pathlen,
                         &urlargs));
  check(ctrl->c(&req, &urlargs, NULL, NULL) == 82);

  create_dummy_request(&req, "/foo/bar");
  check_notnull(ctrl = cmap_match(&map, req.resource.data, req.resource.size));
  check_success(parseurl(req.resource.data,
                         req.resource.size,
                         ctrl->path,
                         ctrl->pathlen,
                         &urlargs));
  check(ctrl->c(&req, &urlargs, NULL, NULL) == 92);

  reqfree(&req);
  smap_free(&urlargs);
  cmap_free(&map);
}

char *template =
    "{{if falsevar1}}\r\n"
    "THIS should not be visible!\r\n"
    "{{elif falsevar2}}\r\n"
    "THIS should not be visible EITHER!\r\n"
    "{{else}}\r\n"
    "{{if truevar}}\r\n"
    "Hello!{{for e in 1..4}}{{e}} is a nice number! {{users[e]}} is a stupid "
    "user!{{endfor}}\r\n"
    "{{for user in users}}{{user}} is my favourite user, {{endfor}}\r\n"
    "\\{{this should get rendered normally}}\r\n"
    "{{if truevar}} \\{{elif should get rendered normally as well}}  "
    "{{endif}}\r\n"
    "{{else}} this should not render \r\n"
    "{{endif}}{{endif}}\r\n";

char *rendered =
    "\r\n\r\nHello!1 is a nice number! user1 is a stupid user!2 is a nice "
    "number! user2 is a stupid user!3 is a nice number! user3 is a stupid "
    "user!\r\nuserA is my favourite user, userB is my favourite user, userC is "
    "my favourite user, \r\n{{this should get rendered normally}}\r\n {{elif "
    "should get rendered normally as well}}  \r\n\r\n";

void test_template() {
  smap context = {0};
  sb dest = {0};
  check_success(smap_init(&context, 32));
  check_success(sbinit(&dest, 512));
  check_success(smap_upsert(&context, EXP_LEN("users[1]"), EXP_LEN("user1")));
  check_success(smap_upsert(&context, EXP_LEN("users[2]"), EXP_LEN("user2")));
  check_success(smap_upsert(&context, EXP_LEN("users[3]"), EXP_LEN("user3")));
  check_success(
      smap_upsert(&context, EXP_LEN("users"), EXP_LEN("userA;userB;userC")));
  check_success(smap_upsert(&context, EXP_LEN("falsevar1"), EXP_LEN("false")));
  check_success(smap_upsert(&context, EXP_LEN("falsevar2"), EXP_LEN("false")));
  check_success(smap_upsert(&context, EXP_LEN("truevar"), EXP_LEN("true")));

  check_success(render_template(&dest, template, strlen(template), &context));

  check(streq(dest.data, dest.size, rendered, strlen(rendered)));

  sbfree(&dest);
  smap_free(&context);
}

void test_path_validation() {
  // cmap map = {0};
  // centry *ctrl = NULL;
  // request req = {0};
  // smap urlargs = {0};

  // see if only alphanumerical names, {}*/ are allowed
  // make sure duplicate entries raise exceptions -> TODO
  // make sure full cmap also raises exception
}

int main() {
  TEST(test_sb);
  TEST(test_smap);
  TEST(test_cmap);
  TEST(test_cmap_emptypath);
  TEST(test_cmap_globbing);
  TEST(test_template);
}