#ifndef MINI_CJSON_H
#define MINI_CJSON_H

/*
 * Minimal, self-contained JSON parser exposing the small subset of the
 * cJSON API this project relies on (object/string parsing). Vendored
 * in-tree so the shell builds standalone without an external -lcjson
 * dependency.
 */

typedef enum {
    cJSON_False = 0,
    cJSON_True,
    cJSON_NULL,
    cJSON_Number,
    cJSON_String,
    cJSON_Array,
    cJSON_Object
} cJSON_Type;

typedef struct cJSON {
    struct cJSON *next;
    struct cJSON *child;
    int type;
    char *valuestring;
    double valuedouble;
    char *string;
} cJSON;

cJSON *cJSON_Parse(const char *value);
void cJSON_Delete(cJSON *item);
cJSON *cJSON_GetObjectItem(const cJSON *object, const char *string);
int cJSON_IsString(const cJSON *item);
int cJSON_IsObject(const cJSON *item);

#define cJSON_ArrayForEach(pos, head) \
    for ((pos) = ((head) != NULL) ? (head)->child : NULL; (pos) != NULL; (pos) = (pos)->next)

#endif /* MINI_CJSON_H */
