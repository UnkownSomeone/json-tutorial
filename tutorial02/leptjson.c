#include "leptjson.h"
#include <assert.h>  /* assert() */
#include <stdlib.h>  /* NULL, strtod() */
#include <errno.h>
#include <math.h>    /* HUGE_VAL */


#define EXPECT(c, ch)       do { assert(*c->json == (ch)); c->json++; } while(0)
#define ISDIGIT1TO9(ch)     ((ch) >= '1' && (ch) <= '9')
#define ISDIGIT(ch)         ((ch) >= '0' && (ch) <= '9')


typedef struct {
    const char* json;
}lept_context;

static void lept_parse_whitespace(lept_context* c) {
    const char *p = c->json;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        p++;
    c->json = p;
}

/** 重构合并 `lept_parse_null()`、`lept_parse_false()`、`lept_parse_true()` 为 `lept_parse_literal()` **/
static int lept_parse_literal(lept_context* c, lept_value* v, const char* literal, lept_type type){
    EXPECT(c, literal[0]);
    size_t i;
    for (i = 0; literal[i+1]; i++) {
        if (c->json[i] != literal[i+1])
            return LEPT_PARSE_INVALID_VALUE;
    }
    c->json += i;
    v->type = type;
    return LEPT_PARSE_OK;
}

/** static int lept_parse_true(lept_context* c, lept_value* v) 

static int lept_parse_false(lept_context* c, lept_value* v) {
    EXPECT(c, 'f');
    if (c->json[0] != 'a' || c->json[1] != 'l' || c->json[2] != 's' || c->json[3] != 'e')
        return LEPT_PARSE_INVALID_VALUE;
    c->json += 4;
    v->type = LEPT_FALSE;
    return LEPT_PARSE_OK;
}

static int lept_parse_null(lept_context* c, lept_value* v) {
    EXPECT(c, 'n');
    if (c->json[0] != 'u' || c->json[1] != 'l' || c->json[2] != 'l')
        return LEPT_PARSE_INVALID_VALUE;
    c->json += 3;
    v->type = LEPT_NULL;
    return LEPT_PARSE_OK;
} **/

/* 按 JSON number 的语法在 lept_parse_number() 校验，不符合标准的情况返回 `LEPT_PARSE_INVALID_VALUE` 错误码。 */
static int lept_parse_number(lept_context* c, lept_value* v) {
    /* \TODO validate number */
    const char* p = c->json;
    /* 负号 跳过 */
    if (*p == '-') p++;
    /* 整数: 只能是单个 0；而由 1-9 开始的话，可以加任意数量的数字（0-9)*/
    if (*p == '0') p++;
    else {
        if (!ISDIGIT1TO9(*p)) return LEPT_PARSE_INVALID_VALUE; /* 不是0也不是1-9 */
        for (p++; ISDIGIT(*p); p++);
    }      
    /* 小数 : 跳过该小数点，然后检查它至少应有一个 digit，不是 digit 就返回错误码。跳过首个 digit，
    我们再检查有没有 digit，有多少个跳过多少个。这里用了 for 循环技巧来做这件事。 */
    if (*p == '.') {
        p++;
        if (!ISDIGIT(*p)) return LEPT_PARSE_INVALID_VALUE;
        for (p++; ISDIGIT(*p); p++);
    }
    /* 指数 : 后一个可能是 +/-，然后是 digit */
    if (*p == 'e' || *p == 'E') {
        p++;
        if (*p == '+' || *p == '-') p++;
        if (!ISDIGIT(*p)) return LEPT_PARSE_INVALID_VALUE;
        for (p++; ISDIGIT(*p); p++);
    }

    /* strtod的返回值，成功时返回str内容对应的浮点值。如果转换后的值不在相应返回类型的范围内，
    则发生范围错误(errno设置为ERANGE)，并返回HUGE_VAL、HUGE_VALF或HUGE_VALL。如果不能进行转换，则返回0。 */

    errno = 0;
    v->n = strtod(c->json, NULL);
    if (errno == ERANGE && (v->n == HUGE_VAL || v->n == -HUGE_VAL)) return LEPT_PARSE_NUMBER_TOO_BIG;
    c->json = p;
    v->type = LEPT_NUMBER;
    return LEPT_PARSE_OK;
}

static int lept_parse_value(lept_context* c, lept_value* v) {
    switch (*c->json) {
        case 't':  return lept_parse_literal(c, v, "true", LEPT_TRUE);;
        case 'f':  return lept_parse_literal(c, v, "false", LEPT_FALSE);;
        case 'n':  return lept_parse_literal(c, v, "null", LEPT_NULL);;
        default:   return lept_parse_number(c, v);
        case '\0': return LEPT_PARSE_EXPECT_VALUE;
    }
}

int lept_parse(lept_value* v, const char* json) {
    lept_context c;
    int ret;
    assert(v != NULL);
    c.json = json;
    v->type = LEPT_NULL;
    lept_parse_whitespace(&c);
    if ((ret = lept_parse_value(&c, v)) == LEPT_PARSE_OK) {
        lept_parse_whitespace(&c);
        if (*c.json != '\0') {
            v->type = LEPT_NULL;
            ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
        }
    }
    return ret;
}

lept_type lept_get_type(const lept_value* v) {
    assert(v != NULL);
    return v->type;
}

double lept_get_number(const lept_value* v) {
    assert(v != NULL && v->type == LEPT_NUMBER);
    return v->n;
}
