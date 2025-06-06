#include <stdlib.h>

#include "template.h"

#include "../types/sb.h"
#include "../types/smap.h"
#include "../utility/error.h"
#include "../utility/functions.h"

#define ENDFOR "{{endfor}}"
#define IF "{{if "
#define FOR "{{for "
#define ENDIF "{{endif}}"
#define ELIF "{{elif "
#define ELSE "{{else}}"
// Two big TODOs: escaping and indexing
#define isendfor(src, offset)                                                  \
  streq(src + offset, STRLEN(ENDFOR), ENDFOR, STRLEN(ENDFOR))

#define isif(src, offset) streq(src + offset, STRLEN(IF), IF, STRLEN(IF))

#define isendif(src, offset)                                                   \
  streq(src + offset, STRLEN(ENDIF), ENDIF, STRLEN(ENDIF))

#define iselif(src, offset)                                                    \
  streq(src + offset, STRLEN(ELIF), ELIF, STRLEN(ELIF))

#define iselse(src, offset)                                                    \
  streq(src + offset, STRLEN(ELSE), ELSE, STRLEN(ELSE))

#define isvarnamechar(ch) (isalphanumeric(ch) || ch == '[' || ch == ']')

static int render_template_inner(
    sb *dest, char *src, size_t slen, smap *context, sb *varnamebuff);

static int get_varname(sb *varnamebuff, char *src, size_t len, smap *context);

static int render_template_inner(
    sb *dest, char *src, size_t slen, smap *context, sb *varnamebuff);

static int
handle_for(sb *dest, char *src, size_t slen, smap *context, sb *varnamebuff);

static int
handle_if(sb *dest, char *src, size_t slen, smap *context, sb *varnamebuff);
//--------------------definitions----------------------------------------------
// returns the end tag location or slen if not found.
static int find_end_tag(char *src,
                        size_t i,
                        size_t slen,
                        char *begin,
                        size_t beginlen,
                        char *end,
                        size_t endlen) {
  int num_of_begins = 0;
  while (true) {
    if (i >= slen) {
      return i;
    }
    if (i + endlen <= slen && src[i - 1] != '\\' &&
        streq(end, endlen, src + i, endlen)) {
      if (num_of_begins == 0) {
        break;
      }
      --num_of_begins;

    } else if ((i + beginlen) <= slen && src[i - 1] != '\\' &&
               streq(begin, beginlen, src + i, beginlen)) {
      ++num_of_begins;
    }
    ++i;
  }
  return i;
}

static int flush_ws(char *src, size_t len, size_t i) {
  while (i < len && (src[i] == ' ' || src[i] == '\t')) {
    ++i;
  }
  return i;
}

static int get_varname(sb *varnamebuff, char *src, size_t len, smap *context) {
  int error;
  if ((error = sbclear(varnamebuff))) {
    return error;
  }

  // size_t indexer_begin, indexer_len;
  size_t var_i = 0;
  while (true) {
    if (var_i >= len) {
      return OK;
    }
    if (src[var_i] == '[') {
      ++var_i;
      size_t indexer_begin = var_i, indexer_len;
      while (true) {
        if (var_i >= len) {
          return RAISE_INVALIDTEMPLATE("template error: in variable name there "
                                       "is opening [ without closing ].");
        }
        if (src[var_i] == ']') {
          indexer_len = var_i - indexer_begin;
          break;
        }
        if (!isalphanumeric(src[var_i])) {
          return RAISE_INVALIDTEMPLATE(
              "template error: in variable indexer name only alphanumeric "
              "chars are allowed");
        }
        var_i++;
      }
      snode *indexernode = smap_get(context, src + indexer_begin, indexer_len);
      if (!indexernode) {
        return RAISE_INVALIDTEMPLATE(
            "template error: variable indexer name was not found in context");
      }
      if ((error =
               sbappend(varnamebuff, "[", 1) ||
               sbappend(varnamebuff, indexernode->value, indexernode->vallen) ||
               sbappend(varnamebuff, "]", 1))) {
        return error;
      }
    } else if (!isalphanumeric(src[var_i])) {
      return RAISE_INVALIDTEMPLATE(
          "template error: in variable name only alphanumeric "
          "chars are allowed");
    } else {
      sbappend(varnamebuff, src + var_i, 1);
    }
    ++var_i;
  }
}

static int render_template_inner(
    sb *dest, char *src, size_t slen, smap *context, sb *varnamebuff) {
  size_t i = 0;
  int error;
  while (i < slen) {
    if (src[i] == '{' && i + 1 < slen && src[i+1] == '{' &&
        (i == 0 || src[i - 1] != '\\')) {
      i += 2; // i points for the first charachter after {{
      if (i + 4 <= slen && streq(src + i, 4, "for ", 4)) {
        // HANDLE FOR
        // find closing {{endfor}}
        size_t beginfor = i + 4;
        i = find_end_tag(src, i, slen, EXP_LEN(FOR), EXP_LEN(ENDFOR));

        if (i + STRLEN(ENDFOR) > slen) {
          return RAISE_INVALIDTEMPLATE("invalid template: found {{for ..}} at "
                                       "%lu without closing {{endfor}}",
                                       beginfor);
        }
        if ((error = handle_for(
                 dest, src + beginfor, i - beginfor, context, varnamebuff))) {
          return error;
        }
        i += STRLEN(ENDFOR);
        continue;
      } else if (i + 3 <= slen && streq(src + i, 3, "if ", 3)) {
        // HANDLE IF
        size_t beginif = i + 3;
        i = find_end_tag(src, i, slen, EXP_LEN(IF), EXP_LEN(ENDIF));

        if (i + STRLEN(ENDIF) > slen) {
          return RAISE_INVALIDTEMPLATE("invalid template: found {{if ..}} at "
                                       "%lu without closing {{endif}}",
                                       beginif);
        }
        if ((error = handle_if(
                 dest, src + beginif, i - beginif, context, varnamebuff))) {
          return error;
        }
        i += STRLEN(ENDIF);
        continue;
      } else {
        size_t varbegin = i;
        while (i + 2 <= slen && !streq(src + i, 2, "}}", 2)) {
          if (!isvarnamechar(src[i])) {
            return RAISE_INVALIDTEMPLATE(
                "invalid variable name in template at %lu, only alphanumeric "
                "characters are allowed",
                varbegin);
          }
          ++i;
        }

        if (i + 2 >= slen) {
          return RAISE_INVALIDTEMPLATE(
              "invalid template: found {{ without closing }} at %lu", varbegin);
        }

        if ((error = get_varname(
                 varnamebuff, src + varbegin, i - varbegin, context))) {
          return error;
        }

        snode *resolve =
            smap_get(context, varnamebuff->data, varnamebuff->size);
        if (!resolve) {
          return RAISE_INVALIDTEMPLATE(
              "template context error: could not resolve variable at index %lu",
              varbegin);
        }
        if ((error = sbappend(dest, resolve->value, resolve->vallen))) {
          return error;
        }

        i += 2;
        continue;
      }
    } else {
      if (src[i] != '\\' || i + 1 >= slen ||
          (src[i + 1] != '{' && src[i + 1] != '\\')) {
        if ((error = sbappend(dest, src + i, 1))) {
          return error;
        }
      }
      ++i;
      continue;
    }
  }
  return OK;
}

static int
handle_for(sb *dest, char *src, size_t slen, smap *context, sb *varnamebuff) {
  int error;
  // src will start with for
  // syntax is either "e in var" or "e in 1..10" or "e in 1..10/2"
  size_t varname_begin = 0, i = 0;
  i = flush_ws(src, slen, i);
  while (i < slen && src[i] != ' ') {
    if (!isvarnamechar(src[i])) {
      return RAISE_INVALIDTEMPLATE("template error: template variables can "
                                   "only have alphanumeric characters");
    }
    ++i; // todo add indexer support
  }
  if (i >= slen) {
    return RAISE_INVALIDTEMPLATE("template error: malformed for");
  }
  size_t varname_len = i - varname_begin;
  // string after varname should be " in "
  i = flush_ws(src, slen, i);
  if (src[i] != 'i' || src[++i] != 'n') {
    return RAISE_INVALIDTEMPLATE("template error: malformed for");
  }
  ++i;
  i = flush_ws(src, slen, i);

  size_t collectionvar_begin = i, collectionvar_len, endvar_begin, endvar_len,
         stepvar_begin, stepvar_len, for_body_begin, for_body_len;
  bool is_coll_numeric = false;
  bool has_step_var = false;
  while (true) {
    if (i >= slen) {
      return RAISE_INVALIDTEMPLATE("template error: malformed for");
    }
    if (src[i] == '.') {
      if (++i >= slen || src[i] != '.' || ++i >= slen || !isnumeric(src[i])) {
        return RAISE_INVALIDTEMPLATE("template error: malformed for");
      }
      is_coll_numeric = true;
      collectionvar_len = i - 2 - collectionvar_begin;
      endvar_begin = i;
      continue;
    }
    if (src[i] == '/') {
      if (!is_coll_numeric || ++i >= slen || !isnumeric(src[i])) {
        return RAISE_INVALIDTEMPLATE("template error: malformed for");
      }
      has_step_var = true;
      endvar_len = i - 1 - endvar_begin;
      stepvar_begin = i;
      continue;
    }
    if (src[i] == '}') {
      if (++i >= slen || src[i] != '}') {
        return RAISE_INVALIDTEMPLATE("template error: malformed for");
      }
      if (has_step_var) {
        stepvar_len = i - 1 - stepvar_begin;
      } else if (is_coll_numeric) {
        endvar_len = i - 1 - endvar_begin;
      } else {
        collectionvar_len = i - 1 - collectionvar_begin;
      }
      for_body_begin = ++i;
      for_body_len = slen - for_body_begin;
      break;
    }
    if (!isvarnamechar(src[i]) || (is_coll_numeric && !isnumeric(src[i]))) {
      return RAISE_INVALIDTEMPLATE("template error: malformed for");
    }
    ++i;
  }
  if (is_coll_numeric) {
    const int buffsize = 12;
    // calculate start, end, step for foreach
    char buffer[buffsize];
    memset(buffer, 0, buffsize);
    strncpy(buffer, src + collectionvar_begin, collectionvar_len);
    buffer[buffsize-1] = 0;
    int start = atoi(buffer);
    memset(buffer, 0, buffsize);
    strncpy(buffer, src + endvar_begin, endvar_len);
    buffer[buffsize-1] = 0;
    int end = atoi(buffer);
    int step = 1;
    if (has_step_var) {
      memset(buffer, 0, buffsize);
      strncpy(buffer, src + stepvar_begin, stepvar_len);
      buffer[buffsize-1] = 0;
      step = atoi(buffer);
    }
    for (int i = start; i < end; i += step) {
      // putting loop variable into context
      memset(buffer, 0, buffsize);
      snprintf(buffer, buffsize, "%d", i);
      buffer[buffsize-1] = 0;
      if ((error = smap_upsert(context,
                               src + varname_begin,
                               varname_len,
                               buffer,
                               strlen(buffer)))) {
        return error;
      };
      if ((error = render_template_inner(dest,
                                         src + for_body_begin,
                                         for_body_len,
                                         context,
                                         varnamebuff))) {
        return error;
      }
    }
    smap_del(context, src + varname_begin, varname_len);
    return OK;
  } else {
    // get varname
    if ((error = get_varname(varnamebuff,
                             src + collectionvar_begin,
                             collectionvar_len,
                             context))) {
      return error;
    }
    snode *collnode = smap_get(context, varnamebuff->data, varnamebuff->size);
    if (!collnode) {
      return RAISE_INVALIDTEMPLATE("template error: malformed for");
    }
    // we got our value that will serve as our collection.
    // we will tokenize along every ';' character (making sure
    // excaped "\;"-s are left together) and run the for block for each value.
    char *collvalue = collnode->value;
    size_t coll_i = 0, collen = collnode->vallen, token_start, token_end;

    while (coll_i < collen) {
      token_start = coll_i;
      token_end = coll_i;
      while (true) {
        if (coll_i >= collen) {
          token_end = coll_i;
          break;
        }
        if (collvalue[coll_i] == ';') {
          if (coll_i == 0 || collvalue[coll_i - 1] != '\\') {
            token_end = coll_i;
            ++coll_i;
            break;
          }
        }
        ++coll_i;
      }
      // upserting value into context
      if ((error = smap_upsert(context,
                               src + varname_begin,
                               varname_len,
                               collvalue + token_start,
                               token_end - token_start))) {
        return error;
      }
      // rendering for block with the calculated value
      if ((error = render_template_inner(dest,
                                         src + for_body_begin,
                                         for_body_len,
                                         context,
                                         varnamebuff))) {
        return error;
      }
    }
    smap_del(context, src + varname_begin, varname_len);
    return OK;
  }
}

static int
handle_if(sb *dest, char *src, size_t slen, smap *context, sb *varnamebuff) {
  int error;
  // get variable name
  // src starts with "if ..."
  size_t varname_start = 0, varname_len, i = 0;
  while (true) {
    if (i >= slen) {
      return RAISE_INVALIDTEMPLATE("invalid template: malformed if");
    } else if (src[i] == '}') {
      if (++i >= slen || src[i] != '}') {
        return RAISE_INVALIDTEMPLATE("invalid template: malformed if");
      }
      varname_len = i - 1 - varname_start;
      break;
    } else if (!isvarnamechar(src[i])) {
      return RAISE_INVALIDTEMPLATE("invalid template: malformed if");
    }
    ++i;
  }
  // get truth value
  if ((error = get_varname(
           varnamebuff, src + varname_start, varname_len, context))) {
    return error;
  }

  snode *varvalue_node =
      smap_get(context, varnamebuff->data, varnamebuff->size);
  if (!varvalue_node) {
    return RAISE_INVALIDTEMPLATE(
        "invalid template: if variable not in context");
  }
  // find end of this if block (either elif or nothing)
  if (++i >= slen) {
    return OK; // empty if block
  }
  size_t startofblock = i, endofblock;
  bool noelif = false;
  int nest_count = 0;
  while (true) {
    if (i + STRLEN(IF) <= slen && isif(src, i) && src[i - 1] != '\\') {
      ++nest_count;
    } else if (i + STRLEN(ENDIF) <= slen && isendif(src, i) &&
               src[i - 1] != '\\') {
      --nest_count;
    } else if (i >= slen) {
      if (nest_count == 0) {
        endofblock = slen;
        break;
      } else {
        return RAISE_INVALIDTEMPLATE(
            "invalid template: found {{if ..}} without closing {{endif}}");
      }
    } else if (i + STRLEN(ELIF) <= slen && iselif(src, i) &&
               src[i - 1] != '\\') {
      if (nest_count == 0) {
        endofblock = i;
        break;
      }
    } else if (i + STRLEN(ELSE) <= slen && iselse(src, i) &&
               src[i - 1] != '\\') {
      if (nest_count == 0) {
        endofblock = i;
        noelif = true;
        break;
      }
    }
    ++i;
  }
  if (streq(varvalue_node->value, varvalue_node->vallen, EXP_LEN("true"))) {
    return render_template_inner(dest,
                                 src + startofblock,
                                 endofblock - startofblock,
                                 context,
                                 varnamebuff);
  } else if (streq(varvalue_node->value,
                   varvalue_node->vallen,
                   EXP_LEN("false"))) {
    if (endofblock >= slen) {
      return OK; // no elif block
    }
    size_t next_block;

    if (noelif) {
      next_block = endofblock + STRLEN(ELSE);
      return render_template_inner(
          dest, src + next_block, slen - next_block, context, varnamebuff);
    } else {
      next_block =
          endofblock +
          STRLEN(ELIF); // points to start of variable name inside the elif
      return handle_if(
          dest, src + next_block, slen - next_block, context, varnamebuff);
    }

  } else {
    return RAISE_INVALIDTEMPLATE("invalid template: unrecognizable variable in "
                                 "context for if (should be true or false)");
  }
}

int render_template(sb *dest, char *src, size_t len, smap *context) {
  if(geterrcode()){
    return geterrcode();
  }
  if (!dest || !src || !context) {
    return RAISE_ARGERR("invalid arguments for render_template");
  }
  sb varnamebuff = {0};
  sbinit(&varnamebuff, 32);
  int res = render_template_inner(dest, src, len, context, &varnamebuff);
  sbfree(&varnamebuff);
  return res;
}