#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "json.h"
#include "hashtable.h"
#include "arraylist.h"
#include "mempool.h"

static lky_mempool mempool;

void *con_number(double d)
{
    double *m = malloc(sizeof(double));
    *m = d;

    pool_add(&mempool, m);

    return m;
}

void *con_string(char *s)
{
    char *m = strdup(s);
    pool_add(&mempool, m);
    return m;
}

void *con_singleton(js_singleton_type type)
{
    unsigned *m = malloc(sizeof(unsigned));
    *m = type;
    pool_add(&mempool, m);

    return m;
}

void *con_object()
{
    hashtable *ht = malloc(sizeof(hashtable));
    hashtable src = hst_create();
    memcpy(ht, &src, sizeof(hashtable));

    ht->duplicate_keys = 1;

    return ht;
}

void *con_array()
{
    arraylist *lst = malloc(sizeof(arraylist));
    arraylist list = arr_create(10);
    memcpy(lst, &list, sizeof(arraylist));

    return lst;
}

void add_object(void *obj, char *text, void *val)
{
    hashtable *ht = (hashtable *)obj;
    hst_put(ht, text, val, NULL, NULL);
}

void add_array(void *obj, void *val)
{
    arraylist *list = (arraylist *)obj;
    arr_append(list, val);
}

int main(int argc, char *argv[])
{
    mempool = pool_create();
    json_parser *parser = js_make_parser("[1, 5, \"Hello\", \"World\", [11], [true, {\"name\" : \"Sam\"}] ]");
    js_set_number_constructor(parser, con_number);
    js_set_string_constructor(parser, con_string);
    js_set_singleton_constructor(parser, con_singleton);
    js_set_object_constructor(parser, con_object);
    js_set_array_constructor(parser, con_array);

    js_set_object_adder(parser, add_object);
    js_set_array_adder(parser, add_array);

    arraylist *arr = js_parse(parser);

    printf("Got %p in return.\n", arr); 
    printf("Values:\n\t0 -> %lf\n\t1 -> %lf\n", *(double *)arr_get(arr, 0), *(double *)arr_get(arr, 1));
    printf("\t2 -> '%s'\n", (char *)arr_get(arr, 2));
    printf("\t3 -> '%s'\n", (char *)arr_get(arr, 3));
    printf("Nested first value: %lf\n", *(double *)(arr_get((arraylist *)arr_get(arr, 4), 0)));

    hashtable *ht = (hashtable *)arr_get((arraylist *)arr_get(arr, 5), 1);
    printf("\n\n{\n\tname: %s\n}\n", (char *)hst_get(ht, "name", NULL, NULL));

    return 0;
}

