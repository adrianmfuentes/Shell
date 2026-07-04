#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "cJSON.h"

typedef struct {
    const char *json;
    size_t pos;
} parse_ctx;

static cJSON *cJSON_New_Item(void) {
    cJSON *node = calloc(1, sizeof(cJSON));
    return node;
}

static void skip_whitespace(parse_ctx *ctx) {
    while (ctx->json[ctx->pos] && isspace((unsigned char)ctx->json[ctx->pos])) {
        ctx->pos++;
    }
}

/* Parses a quoted JSON string starting at the opening quote. Returns a
 * newly allocated, NUL-terminated buffer with escapes resolved, or NULL
 * on malformed input. Advances ctx past the closing quote. */
static char *parse_string_literal(parse_ctx *ctx) {
    if (ctx->json[ctx->pos] != '"') return NULL;
    ctx->pos++;

    size_t start = ctx->pos;
    size_t len = 0;
    while (ctx->json[ctx->pos] && ctx->json[ctx->pos] != '"') {
        if (ctx->json[ctx->pos] == '\\' && ctx->json[ctx->pos + 1]) ctx->pos++;
        ctx->pos++;
        len++;
    }
    if (ctx->json[ctx->pos] != '"') return NULL; /* unterminated string */

    char *out = malloc(len + 1);
    if (!out) return NULL;

    size_t i = start, o = 0;
    while (i < ctx->pos) {
        char c = ctx->json[i];
        if (c == '\\' && i + 1 < ctx->pos) {
            i++;
            switch (ctx->json[i]) {
                case '"': out[o++] = '"'; break;
                case '\\': out[o++] = '\\'; break;
                case '/': out[o++] = '/'; break;
                case 'n': out[o++] = '\n'; break;
                case 't': out[o++] = '\t'; break;
                case 'r': out[o++] = '\r'; break;
                case 'b': out[o++] = '\b'; break;
                case 'f': out[o++] = '\f'; break;
                case 'u': /* skip \uXXXX, not needed for shell config values */
                    i += 4;
                    break;
                default: out[o++] = ctx->json[i]; break;
            }
            i++;
        } else {
            out[o++] = c;
            i++;
        }
    }
    out[o] = '\0';
    ctx->pos++; /* consume closing quote */
    return out;
}

static cJSON *parse_value(parse_ctx *ctx);

static cJSON *parse_object(parse_ctx *ctx) {
    cJSON *object = cJSON_New_Item();
    if (!object) return NULL;
    object->type = cJSON_Object;

    ctx->pos++; /* consume '{' */
    skip_whitespace(ctx);

    cJSON *last_child = NULL;
    if (ctx->json[ctx->pos] == '}') {
        ctx->pos++;
        return object;
    }

    while (1) {
        skip_whitespace(ctx);
        if (ctx->json[ctx->pos] != '"') {
            cJSON_Delete(object);
            return NULL;
        }
        char *key = parse_string_literal(ctx);
        if (!key) {
            cJSON_Delete(object);
            return NULL;
        }

        skip_whitespace(ctx);
        if (ctx->json[ctx->pos] != ':') {
            free(key);
            cJSON_Delete(object);
            return NULL;
        }
        ctx->pos++;
        skip_whitespace(ctx);

        cJSON *value = parse_value(ctx);
        if (!value) {
            free(key);
            cJSON_Delete(object);
            return NULL;
        }
        value->string = key;

        if (last_child) {
            last_child->next = value;
        } else {
            object->child = value;
        }
        last_child = value;

        skip_whitespace(ctx);
        if (ctx->json[ctx->pos] == ',') {
            ctx->pos++;
            continue;
        }
        break;
    }

    skip_whitespace(ctx);
    if (ctx->json[ctx->pos] != '}') {
        cJSON_Delete(object);
        return NULL;
    }
    ctx->pos++;
    return object;
}

/* Arrays aren't needed by this project's config schema; skip and drop
 * them so an unexpected array value doesn't abort the whole parse. */
static cJSON *parse_array(parse_ctx *ctx) {
    cJSON *array = cJSON_New_Item();
    if (!array) return NULL;
    array->type = cJSON_Array;

    ctx->pos++; /* consume '[' */
    skip_whitespace(ctx);
    if (ctx->json[ctx->pos] == ']') {
        ctx->pos++;
        return array;
    }

    while (1) {
        skip_whitespace(ctx);
        cJSON *value = parse_value(ctx);
        if (!value) {
            cJSON_Delete(array);
            return NULL;
        }
        cJSON_Delete(value); /* not surfaced; we only need this to not derail parsing */
        skip_whitespace(ctx);
        if (ctx->json[ctx->pos] == ',') {
            ctx->pos++;
            continue;
        }
        break;
    }
    skip_whitespace(ctx);
    if (ctx->json[ctx->pos] != ']') {
        cJSON_Delete(array);
        return NULL;
    }
    ctx->pos++;
    return array;
}

static cJSON *parse_value(parse_ctx *ctx) {
    skip_whitespace(ctx);
    char c = ctx->json[ctx->pos];

    if (c == '"') {
        cJSON *item = cJSON_New_Item();
        if (!item) return NULL;
        item->type = cJSON_String;
        item->valuestring = parse_string_literal(ctx);
        if (!item->valuestring) {
            free(item);
            return NULL;
        }
        return item;
    }
    if (c == '{') return parse_object(ctx);
    if (c == '[') return parse_array(ctx);
    if (strncmp(&ctx->json[ctx->pos], "true", 4) == 0) {
        cJSON *item = cJSON_New_Item();
        item->type = cJSON_True;
        ctx->pos += 4;
        return item;
    }
    if (strncmp(&ctx->json[ctx->pos], "false", 5) == 0) {
        cJSON *item = cJSON_New_Item();
        item->type = cJSON_False;
        ctx->pos += 5;
        return item;
    }
    if (strncmp(&ctx->json[ctx->pos], "null", 4) == 0) {
        cJSON *item = cJSON_New_Item();
        item->type = cJSON_NULL;
        ctx->pos += 4;
        return item;
    }
    if (c == '-' || isdigit((unsigned char)c)) {
        char *end;
        double num = strtod(&ctx->json[ctx->pos], &end);
        if (end == &ctx->json[ctx->pos]) return NULL;
        cJSON *item = cJSON_New_Item();
        item->type = cJSON_Number;
        item->valuedouble = num;
        ctx->pos += (end - &ctx->json[ctx->pos]);
        return item;
    }
    return NULL;
}

cJSON *cJSON_Parse(const char *value) {
    if (!value) return NULL;
    parse_ctx ctx = { value, 0 };
    skip_whitespace(&ctx);
    if (ctx.json[ctx.pos] != '{') return NULL; /* only object roots are supported */
    return parse_object(&ctx);
}

void cJSON_Delete(cJSON *item) {
    while (item) {
        cJSON *next = item->next;
        if (item->child) cJSON_Delete(item->child);
        free(item->valuestring);
        free(item->string);
        free(item);
        item = next;
    }
}

cJSON *cJSON_GetObjectItem(const cJSON *object, const char *string) {
    if (!object || object->type != cJSON_Object) return NULL;
    cJSON *item = object->child;
    while (item) {
        if (item->string && strcmp(item->string, string) == 0) return item;
        item = item->next;
    }
    return NULL;
}

int cJSON_IsString(const cJSON *item) {
    return item != NULL && item->type == cJSON_String;
}

int cJSON_IsObject(const cJSON *item) {
    return item != NULL && item->type == cJSON_Object;
}
