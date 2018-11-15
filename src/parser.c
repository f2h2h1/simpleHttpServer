#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"

Request* requset_init()
{
    Request *request; // 定义头节点
    request = (Request*)malloc(sizeof(Request)); // 分配地址
    memset(request->method, 0, sizeof(char)*10);
    memset(request->path, 0, sizeof(char)*236);
    memset(request->http_version, 0, sizeof(char)*16);
    memset(request->header_raw, 0, sizeof(char)*512);
    memset(request->body_raw, 0, sizeof(char)*512);
    memset(request->query_string, 0, sizeof(char)*236);
    memset(request->request_uri, 0, sizeof(char)*256);
    request->header = link_create();
    request->post = link_create();
    request->get = link_create();
    request->cookie = link_create();
    return request;
}
void request_print(Request *request)
{
    if (request == NULL)
    {
        printf("empty\n");
        return;
    }
    printf("%x\n", request);
    printf("method:\n");
    printf("%s\n", request->method);
    printf("path:\n");
    printf("%s\n", request->path);
    printf("http_version:\n");
    printf("%s\n", request->http_version);
    printf("header_raw:\n");
    printf("%s\n", request->header_raw);
    printf("body_raw:\n");
    printf("%s\n", request->body_raw);
    printf("query_string:\n");
    printf("%s\n", request->query_string);
    printf("request_uri:\n");
    printf("%s\n", request->request_uri);
    printf("port:\n");
    printf("%s\n", request->port);
    printf("ip:\n");
    printf("%s\n", request->ip);
    printf("header:\n");
    link_print(request->header);
    printf("post:\n");
    link_print(request->post);
    printf("get:\n");
    link_print(request->get);
    printf("cookie:\n");
    link_print(request->cookie);
}
void request_free(Request *request)
{
    link_free(request->header);
    link_free(request->post);
    link_free(request->get);
    free(request);
}
// 复制字符串里的某一段到另一个字符串里
int copy_cahr (char *src, char *dst, int start, int end)
{
    int i = start, k = 0;
    for (; i < end; k++, i++)
    {
        dst[k] = src[i];
    }
    dst[k] = 0;
}
// 删除字符串里的某个字符
void del_chr(char *s, char ch)
{
    char *t = s; // 目标指针先指向原串头
    while(*s != '\0') // 遍历字符串s
    {
        if (*s != ch) // 如果当前字符不是要删除的，则保存到目标串中
            *t++ = *s;
        s++; // 检查下一个字符
    }
    *t = '\0'; // 置目标串结束符。
}
// 替换单个字符
void char_replace(char search, char replace, char *subject)
{
    int i = 0;
    int len = strlen(subject);
    do {
        if (subject[i] == search)
        {
            subject[i] = replace;
        }
        i++;
    } while (i < len);
}
void set_param(char *item, ListHead* request_body, char *separator)
{
    char name[256];
    char value[256];
    int i = 0;
    int len = strlen(item);
    int start = i;
    int end;
    int separator_len = strlen(separator);
    char temp[64];
    do {
        copy_cahr(item, temp, i, i+separator_len);
        temp[separator_len] = '\0';
        if (strcmp(temp, separator) == 0)
        {
            end = i;
            copy_cahr(item, name, start, end);
            start = end + separator_len;
            end = len;
            copy_cahr(item, value, start, end);
            link_add(request_body, name, value);
            break;
        }
        i++;
    } while (i < len);
}
void parser_kv(char *src, ListHead* kv, char *item_separator, char *value_separator)
{
    int i;
    int len;
    int start;
    int end;
    int item_separator_len;
    char temp[64];
    char item[512];

    i = 0;
    start = i;
    len = strlen(src);
    item_separator_len = strlen(item_separator);
    do {
        copy_cahr(src, temp, i, i+item_separator_len);
        temp[item_separator_len] = '\0';
        if (strcmp(temp, item_separator) == 0) {
            end = i;
            copy_cahr(src, item, start, end);
            start = end + item_separator_len;
            set_param(item, kv, value_separator);
        }
        i++;
    } while (i < len);
}
void separate_request(char *request_raw, Request *request)
{
    char *method = request->method;
    char *request_uri = request->request_uri;
    char *http_version = request->http_version;
    char *path = request->path;
    char *query_string = request->query_string;
    char status_line[512];
    char file_name[64];
    char file_full_name[64];
    char file_ext_name[32];
    int start;
    int end;
    int i;
    int len;
    ListStruct *node;

    i = 0;
    len = strlen(request_raw);
    do {
        // 分离请求头和请求行
        if (request_raw[i] == '\r')
        {
            copy_cahr(request_raw, status_line, 0, i);
            i = i + 2;
            start = i;
            // 分离请求头和请求体
            do {
                if (request_raw[i] == '\r' && request_raw[i+1] == '\n' && request_raw[i+2] == '\r' && request_raw[i+3] == '\n')
                {
                    end = i + 2; // 把最后一个\r\n也包含进去
                    copy_cahr(request_raw, request->header_raw, start, end);
                    break;
                }
                i++;
            } while (i < len);
            break;
        }
        i++;
    } while (i < len);

    parser_kv(request->header_raw, request->header, "\r\n", ": ");

    // 分离ip，端口
    node = link_get(request->header, "Host");
    if (node != NULL)
    {
        i = 0;
        start = i;
        len = strlen(node->value);
        do {
            if (node->value[i] == ':')
            {
                end = i;
                copy_cahr(node->value, request->ip, start, end);
                start = i + 1;
                end = len;
                copy_cahr(node->value, request->port, start, end);
                break;
            }
            i++;
        } while(i < len);
    }
    node = NULL;

    // 分离请求行
    len = strlen(status_line);
    i = 0;
    start = i;
    end;
    do {
        // 分离请求方法
        if (status_line[i] == ' ' && method[0] == 0)
        {
            end = i;
            copy_cahr(status_line, method, start, end);
            start = end + 1;
        }
        // 分离url
        else if (status_line[i] == ' ' && request_uri[0] == 0)
        {
            end = i;
            copy_cahr(status_line, request_uri, start, end);
            start = end + 1;
        }
        // 分离http版本
        else if (status_line[i+1] == '\0' && http_version[0] == 0)
        {
            end = len;
            copy_cahr(status_line, http_version, start, end);
            break;
        }
        i++;
    } while (i < len);

    // 分离query_string参数
    len = strlen(request_uri);
    i = 0;
    start = i;
    do {
        if (request_uri[i] == '?')
        {
            end = i;
            copy_cahr(request_uri, path, start, end);
            start = end + 1;
            end = len;
            copy_cahr(request_uri, query_string, start, end);
            break;
        }
        i++;
    } while (i <= len);
    if (path[0] == 0)
    {
        copy_cahr(request_uri, path, 0, len);
    }

    // 分离cookie
    node = link_get(request->header, "Cookie");
    if (node != NULL)
    {
        len = strlen(node->value);
        node->value[len] = ';';
        node->value[len+1] = ' ';
        node->value[len+2] = '\0';
        parser_kv(node->value, request->cookie, "; ", "=");
    }
}
void get_request_param(char *request_body_raw, ListHead* request_body)
{
    int i = 0;
    int len = strlen(request_body_raw);
    int start = i;
    int end;
    char item[64];
    do {
        if (request_body_raw[i] == '&' || request_body_raw[i] == '\0')
        {
            end = i;
            copy_cahr(request_body_raw, item, start, end);
            start = end + 1;
            set_param(item, request_body, "=");
        }
        i++;
    } while (i <= len);
}
// 获取请求行
void get_header(char *request_raw, Request *request)
{
    char *method = request->method;
    char *request_uri = request->request_uri;
    char *path = request->path;
    char *query_string = request->query_string;
    char status_line[512];
    char header_raw[1024];
    int start;
    int end;
    int i = 0;
    int len = strlen(request_raw);
    do {
        if (request_raw[i] == '\r')
        {
            copy_cahr(request_raw, status_line, 0, i);
            i = i + 2;
            start = i;
            do {
                if (request_raw[i] == '\r' && request_raw[i+1] == '\n' && request_raw[i+2] == '\r' && request_raw[i+3] == '\n')
                {
                    end = i;
                    copy_cahr(request_raw, header_raw, start, end);
                    break;
                }
                i++;
            } while(i < len);
            break;
        }
        i++;
    } while(i < len);
    strcpy(request->header_raw, header_raw);
    i = 0;
    len = strlen(header_raw);
    start = i;
    end;
    char item[64];
    do {
        if ((header_raw[i] == '\r' && header_raw[i+1] == '\n') || header_raw[i] == '\0')
        {
            end = i;
            copy_cahr(header_raw, item, start, end);
            start = end + 2;
            set_param(item, request->header, ": ");
        }
        i++;
    } while (i <= len);

    ListStruct *node = link_get(request->header, "Host");
    if (node != NULL)
    {
        i = 0;
        start = i;
        len = strlen(node->value);
        do {
            if (node->value[i] == ':')
            {
                end = i;
                copy_cahr(node->value, request->ip, start, end);
                start = i + 1;
                end = len;
                copy_cahr(node->value, request->port, start, end);
                break;
            }
            i++;
        } while(i < len);
    }

    len = strlen(status_line);
    i = 0;
    start = i;
    end;
    do {
        if (status_line[i] == 32 && method[0] == 0)
        {
            end = i;
            copy_cahr(status_line, method, start, end);
            start = end + 1;
        }
        else if (status_line[i] == 32 && request_uri[0] == 0)
        {
            end = i;
            copy_cahr(status_line, request_uri, start, end);
            start = end + 1;
        }
        i++;
    } while (i < len);

    len = strlen(request_uri);
    i = 0;
    start = i;
    do {
        if (request_uri[i] == '?')
        {
            end = i;
            copy_cahr(request_uri, path, start, end);
            start = end + 1;
            end = len;
            copy_cahr(request_uri, query_string, start, end);
            break;
        }
        i++;
    } while (i <= len);
}
void get_request_body_raw(char *request_raw, char *request_body)
{
    int i = 0;
    int len = strlen(request_raw);
    do {
        if (request_raw[i] == '\r' && request_raw[i+1] == '\n' && request_raw[i+2] == '\r' && request_raw[i+3] == '\n')
        {
            i = i + 4;
            break;
        }
        i++;
    } while (i < len);
    copy_cahr(request_raw, request_body, i, len);
}
void parser_request(Request *request, char *request_raw)
{
    // get_header(request_raw, request);
    separate_request(request_raw, request);
    get_request_param(request->query_string, request->get);
    get_request_body_raw(request_raw, request->body_raw);
    get_request_param(request->body_raw, request->post);
}
char* is_cgi_script(char *path, char *cgi_script[], int script_count)
{
    int i;
    int j;
    int path_len;
    int script_len;
    char temp[64];

    path_len = strlen(path);
    for (i = 0; i < script_count; i++)
    {
        script_len = strlen(cgi_script[i]);
        copy_cahr(path, temp, path_len - script_len, path_len);

        if (strcmp(temp, cgi_script[i]) == 0) {
            return cgi_script[i];
        }
    }
    return NULL;
}
void get_script_interpreter(char *path, char *interpreter)
{
    FILE *fp = NULL;
    fp = fopen(path, "r");
    if (fp == NULL)
    {
        return;
    }

    char ch;
    int i2 = 0;
    char first_line[256];
    memset(first_line, 0, sizeof(char)*256);
    while(!((ch = fgetc(fp)) == '\n' || ch == '\r'))
    {
        first_line[i2] = ch;
        i2++;
    }

    if (first_line[0] != '#' || first_line[1] != '!')
    {
        interpreter[0] = '\0';
        return;
    }
    
    copy_cahr(first_line, interpreter, 2, strlen(first_line));
    char temp[256];
    memset(temp, 0, sizeof(char)*256);
    strcat(temp, "\"");
    strcat(temp, interpreter);
    strcat(temp, "\"");
    copy_cahr(temp, interpreter, 0, strlen(temp));
}