#ifndef __PARSER_H__
#define __PARSER_H__

#include "linklist.h"

// 保存http请求内容的结构体
typedef struct Request
{
    char method[10];
    char path[236];
    char header_raw[1024];
    char body_raw[1024];
    char query_string[236];
    char request_uri[256];
    char port[6];
    char ip[16];
    char http_version[16];
    ListHead *header;
    ListHead *post;
    ListHead *get;
    ListHead *cookie;
} Request;

// 复制字符串里的某一段到另一个字符串里
int copy_cahr (char *src, char *dst, int start, int end);
// 删除字符串里的某个字符
void del_chr(char *s, char ch);

Request* requset_init();
void request_print(Request *request);
void request_free(Request *request);
void set_param(char *item, ListHead* request_body, char *separator);
void get_request_param(char *request_body_raw, ListHead* request_body);
void get_header(char *request_raw, Request *request);
void get_request_body_raw(char *request_raw, char *request_body);
void parser_request(Request *request, char *request_raw);
void parser_kv(char *src, ListHead* kv, char *item_separator, char *value_separator);
void char_replace(char search, char replace, char *subject);
char* is_cgi_script(char *paht, char *cgi_script[], int script_count);
void get_script_interpreter(char *path, char *first_line);
#endif