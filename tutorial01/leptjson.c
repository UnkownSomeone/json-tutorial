#include "leptjson.h"
#include <assert.h>  /* assert() */
#include <stdlib.h>  /* NULL */

#define EXPECT(c, ch)       do { assert(*c->json == (ch)); c->json++; } while(0)

typedef struct {
    const char* json;
}lept_context;

static void lept_parse_whitespace(lept_context* c) {
    const char *p = c->json;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        p++;
    c->json = p;
}


static int lept_parse_null(lept_context* c, lept_value* v) {
    EXPECT(c, 'n');
    if (c->json[0] != 'u' || c->json[1] != 'l' || c->json[2] != 'l')
        return LEPT_PARSE_INVALID_VALUE;
    c->json += 3;
    v->type = LEPT_NULL;
    return LEPT_PARSE_OK;
}

/* 参考 `lept_parse_null()` 的实现和调用方，解析 true 和 false 值。 */
static int lept_parse_true(lept_context* c, lept_value* v) {
    EXPECT(c,'t');
    if (c->json[0] != 'r' || c->json[1] != 'u' || c->json[2] != 'e')
        return LEPT_PARSE_INVALID_VALUE;
    c->json+=3;
    v->type=LEPT_TRUE;
    return LEPT_PARSE_OK;
}

static int lept_parse_false(lept_context* c, lept_value* v) {
    EXPECT(c,'f');
    if (c->json[0] != 'a' || c->json[1] != 'l' || c->json[2] != 's' || c->json[3] != 'e')
        return LEPT_PARSE_INVALID_VALUE;
    c->json+=4;
    v->type=LEPT_FALSE;
    return LEPT_PARSE_OK;
}

static int lept_parse_value(lept_context* c, lept_value* v) {
    switch (*c->json) {
        case 'n':  return lept_parse_null(c, v);
        case '\0': return LEPT_PARSE_EXPECT_VALUE; /* '\0'就是代表NULL字符 */
        case 't': return lept_parse_true(c, v); /* 增加true */
        case 'f': return lept_parse_false(c, v); /* 增加true */
        default:   return LEPT_PARSE_INVALID_VALUE;
    }
}


/* 练习1：若 json 在一个值之后，空白之后还有其它字符，则要返回 LEPT_PARSE_ROOT_NOT_SINGULAR  */
int lept_parse(lept_value* v, const char* json) {
    lept_context c;
    assert(v != NULL);
    c.json = json;
    v->type = LEPT_NULL;
    lept_parse_whitespace(&c);/* 1.处理ws  */

    int ret;
    /* 2.处理value  */
    if((ret = lept_parse_value(&c,v)) == LEPT_PARSE_OK) {
        /* 3.处理ws  */
        lept_parse_whitespace(&c);
        /* ws后面还有还有其它字符，返回 LEPT_PARSE_ROOT_NOT_SINGULAR */
        if(*c.json != '\0')
            return LEPT_PARSE_ROOT_NOT_SINGULAR;
    }

    return ret;
}

lept_type lept_get_type(const lept_value* v) {
    assert(v != NULL);
    return v->type;
}
