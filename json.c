#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "json.h"

#define NUM_LIMIT 100
#define ERROR ((void *)-1)
#define IS_WHITESPACE(c) (c == ' ' || c == '\t' || c == '\n')

void *jsp_parse_object(json_parser *parser);
void *jsp_parse_array(json_parser *parser);

struct json_parser_{
    char *raw;
    char *input;
    js_number_constructor    con_num;
    js_string_constructor    con_string;
    js_object_constructor    con_object;
    js_array_constructor     con_array;
    js_singleton_constructor con_single;
    
    js_object_add            add_object;
    js_array_add             add_array;
};

json_parser *js_make_parser(char *raw)
{
    json_parser *parser = malloc(sizeof(json_parser));
    size_t len = strlen(raw);

    parser->raw = malloc(len + 1);
    memcpy(parser->raw, raw, len);
    parser->raw[len] = '\0';

    parser->input = parser->raw;

    parser->con_num = NULL;
    parser->con_string = NULL;
    parser->con_object = NULL;
    parser->con_array = NULL;
    parser->con_single = NULL;
    parser->add_object = NULL;
    parser->add_array = NULL;

    return parser;
}

char jsp_peek(json_parser *parser)
{
    return parser->input[0];
}

void jsp_advance(json_parser *parser)
{
    parser->input++;
    while(parser->input[0] && IS_WHITESPACE(parser->input[0]))
        parser->input++;
}

void jsp_ff(json_parser *parser)
{
    while(parser->input[0] && IS_WHITESPACE(parser->input[0]))
        parser->input++;
}

void jsp_eat(json_parser *parser, char expect)
{
    jsp_ff(parser);
    if(parser->input[0] == expect)
        jsp_advance(parser);
    else
        printf("Unexpected character: %c (expected %c).\n", parser->input[0], expect);
}

int jsp_eat_all(json_parser *parser, char *expect)
{
    size_t len = strlen(expect);
    int i;
    for(i = 0; i < len && parser->input[i]; i++)
    {
        if(parser->input[0] == expect[0])
            parser->input++, expect++;
        else
            return 0;
    }

    return len == i;
}

char jsp_next(json_parser *parser)
{
    char c = parser->input[0];
    parser->input++;
    return c;
}

void *jsp_parse_number(json_parser *parser)
{
    char *rest = NULL;
    double d = strtod(parser->input, &rest);

    if(rest == NULL || rest == parser->input)
        return ERROR;

    parser->input = rest;

    return parser->con_num(d);

    /*
    int foundDecimal, foundExponent;
    foundDecimal = foundExponent = 0;

    char tmp[NUM_LIMIT];
    int i;
    for(i = 0; i < NUM_LIMIT; i++)
    {
        char c = jsp_peek(parser);    
        if(c >= '0' && c <= '9' || c == '-' && i == 0)
        {
            tmp[i] = c;
            jsp_eat(parser, c);
            continue;
        }
        
        if(c == '.' && i != 0 && !foundDecimalk)
        {
            tmp[i] = c;
            jsp_eat(parser, c);
            foundDecimal = 1;
            continue;
        }
    }*/
}

char *jsp_parse_string_raw(json_parser *parser)
{
    jsp_eat(parser, '"');

    char *tmp = parser->input;
    while(parser->input[0] && parser->input[0] != '"')
        parser->input++;

    int len = parser->input - tmp;
    char *nw = malloc(len + 1);
    memcpy(nw, tmp, len);
    nw[len] = 0;

    jsp_eat(parser, '"');

    return nw;
}

void *jsp_parse_string(json_parser *parser)
{
    char *raw = jsp_parse_string_raw(parser);
    void *obj = parser->con_string(raw);
    free(raw);
    return obj;
}

void *jsp_parse_value(json_parser *parser)
{
    char c = jsp_peek(parser);

    switch(c)
    {
        case '"':
            return jsp_parse_string(parser);
        case '{':
            return jsp_parse_object(parser);
        case '[':
            return jsp_parse_array(parser);
        case 't':
            jsp_eat_all(parser, "true");
            return parser->con_single(JSS_TRUE);
        case 'f':
            jsp_eat_all(parser, "false");
            return parser->con_single(JSS_FALSE);
        case 'n':
            jsp_eat_all(parser, "null");
            return parser->con_single(JSS_NULL);
        default:
            if(c == '-' || (c >= '0' && c <= '9'))
                return jsp_parse_number(parser);
    }

    return ERROR;
}

void jsp_parse_object_pair(json_parser *parser, void *obj)
{
    char *key = jsp_parse_string_raw(parser);
    jsp_eat(parser, ':');
    void *val = jsp_parse_value(parser);

    parser->add_object(obj, key, val);
    free(key);
}

void jsp_parse_object_pair_list(json_parser *parser, void *obj)
{
    jsp_parse_object_pair(parser, obj);
    if(jsp_peek(parser) == ',')
    {
        jsp_eat(parser, ',');
        jsp_parse_object_pair_list(parser, obj);
    }
}

void jsp_parse_value_list(json_parser *parser, void *arr)
{
    void *obj = jsp_parse_value(parser);
    parser->add_array(arr, obj);

    if(jsp_peek(parser) == ',')
    {
        jsp_eat(parser, ',');
        jsp_parse_value_list(parser, arr);
    }
}

void *jsp_parse_object(json_parser *parser)
{
    void *obj = parser->con_object();
    jsp_eat(parser, '{');
    if(jsp_peek(parser) == '}')
    {
        jsp_eat(parser, '}');
        return obj;
    }

    jsp_parse_object_pair_list(parser, obj);
    jsp_eat(parser, '}');
    return obj;
}

void *jsp_parse_array(json_parser *parser)
{
    void *arr = parser->con_array();
    jsp_eat(parser, '[');
    if(jsp_peek(parser) == ']')
    {
        jsp_eat(parser, ']');
        return arr;
    }

    jsp_parse_value_list(parser, arr);
    jsp_eat(parser, ']');
    return arr;
}


void *js_parse(json_parser *parser)
{
    char c = jsp_peek(parser);
    if(c != '{' && c != '[')
        return NULL;

    if(c == '{')
        return jsp_parse_object(parser);
    else
        return jsp_parse_array(parser);
}

void js_set_number_constructor(json_parser *parser, js_number_constructor constructor)
{
    parser->con_num = constructor;
}

void js_set_string_constructor(json_parser *parser, js_string_constructor constructor)
{
    parser->con_string = constructor;
}

void js_set_object_constructor(json_parser *parser, js_object_constructor constructor)
{
    parser->con_object = constructor;
}

void js_set_object_adder(json_parser *parser, js_object_add adder)
{
    parser->add_object = adder;
}

void js_set_array_constructor(json_parser *parser, js_array_constructor constructor)
{
    parser->con_array = constructor;
}

void js_set_array_adder(json_parser *parser, js_array_add adder)
{
    parser->add_array = adder;
}

void js_set_singleton_constructor(json_parser *parser, js_singleton_constructor constructor)
{
    parser->con_single = constructor;
}

