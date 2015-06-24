#ifndef JSON_H
#define JSON_H

typedef enum {
    JSS_TRUE,
    JSS_FALSE,
    JSS_NULL
} js_singleton_type;

typedef struct json_parser_ json_parser;

typedef void *(*js_number_constructor)(double);
typedef void *(*js_string_constructor)(char *);
typedef void *(*js_object_constructor)();
typedef void (*js_object_add)(void *, char *, void *);
typedef void *(*js_array_constructor)();
typedef void (*js_array_add)(void *, void *);
typedef void *(*js_singleton_constructor)(js_singleton_type type);

json_parser *js_make_parser(char *raw);
void *js_parse(json_parser *parser);
void js_set_number_constructor(json_parser *parser, js_number_constructor constructor);
void js_set_string_constructor(json_parser *parser, js_string_constructor constructor);
void js_set_object_constructor(json_parser *parser, js_object_constructor constructor);
void js_set_object_adder(json_parser *parser, js_object_add adder);
void js_set_array_constructor(json_parser *parser, js_array_constructor constructor);
void js_set_array_adder(json_parser *parser, js_array_add adder);
void js_set_singleton_constructor(json_parser *parser, js_singleton_constructor constructor);

#endif
