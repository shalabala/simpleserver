#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../types/sb.h"
#include "../types/smap.h"
#include "../types/cmap.h"
#include "../com/request.h"
#include "../com/response.h"
#include "../com/coordination.h"
#include "../utility/error.h"
#include "../utility/functions.h"
/**
 * Run the test.
 */
#define TEST(func)                                                             \
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
      error *err = geterr();
      fprintf(stderr,
              "[FAILED] Error has occured whilst evaluating expression [%s]: "
              "%sn",
              repr,
              err->description);
    } else {
      fprintf(stderr, "[FAILED] Expression [%s] evaluated to false", repr);
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
    check(strneq(key, strlen(key), node->key, node->keylen));
    check(strneq(value, strlen(value), node->value, node->vallen));
    check(map.size == i + 1);

    // UPDATE
    snprintf(value, 64, "UpdatedValue_%ld", i);
    check_success(smap_upsert(&map, key, strlen(key), value, strlen(value)));
    check_notnull(node = smap_get(&map, key, strlen(key)));
    check(strneq(key, strlen(key), node->key, node->keylen));
    check(strneq(value, strlen(value), node->value, node->vallen));
    check(map.size == i + 1);
  }
  // test empty value
  strncpy(key, "newkey", 16);
  check_success(smap_upsert(&map, key, strlen(key), "", 0));
  check_notnull(node = smap_get(&map, key, strlen(key)));
  check(strneq(node->value, node->vallen, "", 0));

  //test empty key
  check_fail(smap_upsert(&map, "", 0, value, strlen(value) ));
  smap_free(&map);
}

int user_ctrl(request *req, smap *urlparams, response *resp){
  snode *id = smap_get(urlparams, "id", 2);
  check_notnull(id);
  return atoi(id->value);
}

int user1_ctrl(request *req, smap *urlparams, response *resp){
  return 42;
}



void test_cmap(){
  cmap map;  
  centry *ctrl;
  smap urlargs;
  check_success(smap_init(urlargs));
  check_success(cmap_init(&map, 10));
  check_success(cmap_reg(&map, "users/{id}", user_ctrl ));
  check_notnull(ctrl = cmap_match(&map, "/users/123", 10));
  check()




}

int main() {
  TEST(test_sb);
  TEST(test_smap);
  TEST(test_cmap);
}