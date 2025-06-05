#ifndef TEMPLATE_H
#define TEMPLATE_H

#include "../types/smap.h"
#include "../types/sb.h"

/**
 * Renders the template into the specified string buffer. 
 * Supported syntax:
 *  - {{varname}} will be matched against the provided context, and if
 *                  it contains a mapping it will be resolved
 *  - {{if var1}} [A] {{elif var2}} [B] {{else}} [C] {{endif}} if var is in the context,
 *                  and its value is not false nor empty, [A] will be rendered, otherwise
 *                  the same checks are conducted on var2 and if they are true [B] is rendered,
 *                  else [C] is rendered
 *  - {{for e in var}} ..{{e or var[e]}}.. {{endfor}} takes the value in var, tokenizes it along ';'
 *                  (\; is escaped) and renders the template for each value in string. For numerical
 *                  loops 1..10 or 1..10/2 if stride is also specified
 * @param dest The string buffer where the contents are rendered
 * @param src The template
 * @param len The length of the template
 * @param context The context against which the template variables are matched
 * @return 0 if the rendering was successful, error code if
 *  - function is called with invalid arguments like NULL
 *  - illegal value between {{ and }} NOTE to escape use \{{..}}
 *  - no closing }}
 */
int render_template(sb *dest,char *src, size_t len, smap *context);
#endif