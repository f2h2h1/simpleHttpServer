#undef UNICODE
#define FD_SETSIZE 128

#include <stdio.h>
#include <winsock2.h>
#include <windows.h>
#include <string.h>
#include <locale.h>

#include "linklist.h"
#include "parser.h"

#pragma comment (lib, "ws2_32.lib")


#define CONNECT_NUM_MAX FD_SETSIZE
#define MAX_BUF 8192

void close_socket(fd_set *socketSet, SOCKET fd_arr)
{
    printf("close:%d\n", fd_arr);
    closesocket(fd_arr);
    // fd_arr = INVALID_SOCKET;
    FD_CLR(fd_arr, socketSet);
}

char tohex(int n)
{

    if (n >= 10 && n <= 15)
    {
        return 'A' + n - 10;
    }
    return '0' + n;
}
void dec2hex(int n, char *buf)
{
    int i = 0;
    int mod;
    while (n)
    {
        mod = n % 16;
        buf[i++] = tohex(mod);
        n = n / 16;
    }
    //得进行反序。
    int j, k;
    for (j = 0, k = i - 1; j < i / 2; j++, k--)
    {
        char temp;
        temp = buf[j];
        buf[j] = buf[k];
        buf[k] = temp;
    }
    buf[i] = '\0';
}

int main(int argc, char *argv[])
{

    int iResult = 0;
    SetConsoleOutputCP(65001);
    /*
     * windows下使用socket必须用WSAStartup初始化，否则不能调用
     */
    WSADATA wsaData;
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) // 启动winsock，指定winsock库的主板本和副版本
    {
        printf("WSAStartup failed: %d\n", iResult);
        return 1;
    }

    printf("wVersion: %d.%d\n", LOBYTE(wsaData.wVersion), HIBYTE(wsaData.wVersion));

    // 创建socket套接字
    SOCKET sockfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd == INVALID_SOCKET)
    {
        printf("Error at socket(): %ld\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // 初始化套接字
    struct sockaddr_in sockAddr;
    memset(&sockAddr, 0, sizeof(sockAddr)); // 每个字节都用0填充
    sockAddr.sin_family = PF_INET; // 使用IPv4地址
    sockAddr.sin_addr.s_addr = inet_addr("127.0.0.1"); // 具体的IP地址
    sockAddr.sin_port = htons(2047); // 端口
    // 绑定
    iResult = bind(sockfd, (SOCKADDR*)&sockAddr, sizeof(SOCKADDR));
    if (iResult == SOCKET_ERROR)
    {
        printf("bind function failed with error %d\n", WSAGetLastError());
        iResult = closesocket(sockfd);
        if (iResult == SOCKET_ERROR)
        {
            printf("closesocket function failed with error %d\n", WSAGetLastError());
        }
        WSACleanup();
        return 1;
    }
    // 监听
    iResult = listen(sockfd, CONNECT_NUM_MAX); // 设置最大连接数
    if (iResult == SOCKET_ERROR)
    {
        printf("listen function failed with error: %d\n", WSAGetLastError());
    }

    /*
        为了方便使用，windows sockets提供了下列宏，用来对fd_set进行一系列操作。使用以下宏可以使编程工作简化。
        FD_CLR(s,*set);从set集合中删除s套接字。
        FD_ISSET(s,*set);检查s是否为set集合的成员。
        FD_SET(s,*set);将套接字加入到set集合中。
        FD_ZERO(*set);将set集合初始化为空集合。
    */

    // 1)初始化一个套接字集合fdSocket，并将监听套接字放入
    fd_set socketSet;
    FD_ZERO(&socketSet);
    FD_SET(sockfd, &socketSet);

    fd_set readSet;
    fd_set writeSet;

    TIMEVAL time = {1, 0};

    printf("FD_SETSIZE=%d\n", FD_SETSIZE);

    char buf[MAX_BUF];
    char sendBuf[MAX_BUF];

    while (1)
    {
        // 2）将fdSocket的一个拷贝fdRead传给select函数
        FD_ZERO(&readSet);
        FD_ZERO(&writeSet);
        // FD_SET(sockfd, &readSet);
        // FD_SET(sockfd, &writeSet);
        readSet = socketSet;
        writeSet = socketSet;

        // 同时检查套接字的可读可写性。
        int nRetAll = select(0, &readSet, &writeSet, NULL, &time); // 若不设置超时则select为阻塞
        if (nRetAll >0)
        {
            // 是否存在客户端的连接请求。
            if (FD_ISSET(sockfd ,&readSet)) // 在readset中会返回已经调用过listen的套接字。
            {

                if (socketSet.fd_count < FD_SETSIZE)
                {
                    SOCKADDR_IN addrRemote;
                    int nAddrLen = sizeof(addrRemote);
                    SOCKET sClient = accept(sockfd, (SOCKADDR*)&addrRemote, &nAddrLen);
                    if (sClient != INVALID_SOCKET)
                    {
                        FD_SET(sClient, &socketSet);
                        printf("接收到连接：(%s)\n",inet_ntoa(addrRemote.sin_addr));
                        printf("a client %d connect.\t\n", sClient);
                        // sprintf(sendBuf, "Welcome %s", inet_ntoa(addrRemote.sin_addr));
                        // send(sClient, sendBuf, strlen(sendBuf) + 1, 0);
                        FD_CLR(sockfd, &readSet);
                    }
                }
                else
                {
                    printf("连接数量已达上限！\n");
                    continue;
                }
            }

            for (int i = 0; i < readSet.fd_count; i++)
            {
                if (FD_ISSET(readSet.fd_array[i], &readSet))
                {
                    int sock_flg = 0;
                    int sock_i;
                    for (int j = 0; j < socketSet.fd_count; j++)
                    {
                        if (readSet.fd_array[i] == socketSet.fd_array[j])
                        {
                            sock_i = j;
                            sock_flg = 1;
                            break;
                        }
                    }
                    if (sock_flg == 0)
                    {
                        break;
                    }
                    //调用recv，接收数据。
                    int nRecv = recv(readSet.fd_array[i], buf, MAX_BUF, 0);
                    if (nRecv > 0)
                    {
                        buf[nRecv] = 0;

                        Request *request = requset_init();
                        parser_request(request, buf);
                        request_print(request);

                        char response[8192];
                        char header[2048];
                        char length[4];
                        char html[1024];
                        char *cgi_script[] = {".cgi", ".pl", ".py", ".php", ".rb"};
                        char *ext;
                        char temp_path[128];
                        copy_cahr(request->path, temp_path, 1, strlen(request->path) + 1);
                        ext = is_cgi_script(temp_path, cgi_script, (sizeof(cgi_script)/sizeof(cgi_script[0])));

                        // 动态请求
                        if (ext != NULL)
                        {
                            printf("ext=%s\n", ext);
                            char cstrNewDosCmd[MAX_PATH];
                            memset(cstrNewDosCmd, 0, sizeof(char)*MAX_PATH);

                            strcpy(header, "HTTP/1.1 500 Internal Server Error\r\nServer: plusplus123/0.1\r\nContent-Type: text/html;charset=UTF-8\r\nContent-Length: ");
                            strcpy(html, "<html><head><title>500</title></head><body><h1>500 Internal Server Error</h1></body></html");

                            // 可执行文件
                            if (strcmp(ext, ".cgi") == 0)
                            {
                                strcat(cstrNewDosCmd, temp_path);
                            }
                            // 脚本
                            else
                            {
                                // 解释脚本第一行，获取脚本的解释器
                                char interpreter[256];
                                memset(interpreter, 0, sizeof(char)*256);
                                get_script_interpreter(temp_path, interpreter);
                                printf("interpreter=%s\n", interpreter);
                                if (interpreter[0] == '\0')
                                {
                                    if (strcmp(ext, ".php") == 0)
                                    {
                                        strcpy(interpreter, "php");
                                    }
                                    else if (strcmp(ext, ".python") == 0)
                                    {
                                        strcpy(interpreter, "python");
                                    }
                                    else
                                    {
                                        // 解释失败
                                        printf("解释失败\n");
                                        sprintf(length, "%d", strlen(html));
                                        strcpy(response, header);
                                        strcat(response, length);
                                        strcat(response, "\r\n\r\n");
                                        strcat(response, html);
                                        send(readSet.fd_array[i], response, strlen(response) + 1, 0);
                                        goto finish;
                                    }
                                }

                                strcpy(cstrNewDosCmd, interpreter);
                                strcat(cstrNewDosCmd, " ");
                                strcat(cstrNewDosCmd, temp_path);
                            }
                            printf("cstrNewDosCmd=%s\n", cstrNewDosCmd);

                            /***************************调用cgi start**************************/

                            HANDLE hRead = NULL;
                            HANDLE hWrite = NULL;
                            char szBuffer[1024] = {0};
                            char chunked[2048] = {0};
                            DWORD dwRead = 0;

                            SECURITY_ATTRIBUTES sa;
                            sa.nLength = sizeof(SECURITY_ATTRIBUTES);
                            sa.lpSecurityDescriptor = NULL;
                            // 新创建的进程继承管道读写句柄
                            sa.bInheritHandle = TRUE;
                            if (!CreatePipe(&hRead, &hWrite, &sa, 0))
                            {
                                printf("管道创建失败\n");
                                sprintf(length, "%d", strlen(html));
                                strcpy(response, header);
                                strcat(response, length);
                                strcat(response, "\r\n\r\n");
                                strcat(response, html);
                                send(readSet.fd_array[i], response, strlen(response) + 1, 0);
                                goto finish;
                            }

                            if (NULL == hRead || NULL == hWrite)
                            {
                                printf("管道创建失败\n");
                                printf("error=%d\n", GetLastError());
                                sprintf(length, "%d", strlen(html));
                                strcpy(response, header);
                                strcat(response, length);
                                strcat(response, "\r\n\r\n");
                                strcat(response, html);
                                send(readSet.fd_array[i], response, strlen(response) + 1, 0);
                                goto finish;
                            }

                            STARTUPINFO si;
                            si.cb = sizeof(STARTUPINFO);
                            GetStartupInfo(&si);
                            si.hStdError = hWrite;  // 把创建进程的标准错误输出重定向到管道输入
                            si.hStdOutput = hWrite; // 把创建进程的标准输出重定向到管道输入
                            printf("request->body_raw=%s\n", request->body_raw);
                            if (strcmp(request->method, "POST") == 0 && request->body_raw != NULL)
                            {
                                // 用WriteFile，把post数据写入管道
                                DWORD dwWritten;
                                if (!WriteFile(hRead, request->body_raw, strlen(request->body_raw), &dwWritten, NULL))
                                {
                                    printf("管道创建失败1\n");
                                    printf("error=%d\n", GetLastError());
                                    sprintf(length, "%d", strlen(html));
                                    strcpy(response, header);
                                    strcat(response, length);
                                    strcat(response, "\r\n\r\n");
                                    strcat(response, html);
                                    send(readSet.fd_array[i], response, strlen(response) + 1, 0);
                                    goto finish;
                                }
                            }
                            else
                            {
                                si.hStdInput = hWrite;
                            }

                            si.wShowWindow = SW_HIDE;

                            si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;

                            PROCESS_INFORMATION pi;

                            // 设置环境变量
                            ListHead *my_envp = request->header;
                            link_add(my_envp, "SYSTEMROOT", getenv("SYSTEMROOT"));
                            link_add(my_envp, "COMSPEC", getenv("COMSPEC"));
                            link_add(my_envp, "PATH", getenv("PATH"));
                            link_add(my_envp, "PATHEXT", getenv("PATHEXT"));
                            link_add(my_envp, "WINDIR", getenv("WINDIR"));
                            link_add(my_envp, "GATEWAY_INTERFACE", "CGI/1.1");
                            link_add(my_envp, "REMOTE_ADDR", request->ip);
                            link_add(my_envp, "SCRIPT_NAME", request->path);
                            link_add(my_envp, "REQUEST_URI", request->request_uri);
                            link_add(my_envp, "QUERY_STRING", request->query_string);
                            link_add(my_envp, "REQUEST_METHOD", request->method);
                            link_add(my_envp, "SERVER_PROTOCOL", request->http_version);

                            char temp_env[16384];
                            char my_envp_item_temp[4096];
                            int i5 = 0;
                            int temp_env_len = 0;
                            ListStruct *my_envp_item;
                            my_envp_item = my_envp->first;
                            int start1 = 0;
                            do
                            {
                                strcpy(my_envp_item_temp, my_envp_item->name);
                                strcat(my_envp_item_temp, "=");
                                strcat(my_envp_item_temp, my_envp_item->value);
                                int my_env_item_len = strlen(my_envp_item_temp);
                                start1 = temp_env_len;
                                for (int i4 = 0; i4 < my_env_item_len; i4++)
                                {
                                    temp_env[start1] = my_envp_item_temp[i4];
                                    start1++;
                                }
                                temp_env_len += my_env_item_len;
                                temp_env[temp_env_len] = '\0';
                                temp_env_len++;
                                my_envp_item = my_envp_item->next;
                            } while (my_envp_item != NULL);
                            temp_env[temp_env_len] = '\0';

                            // 启动进程
                            DWORD dwCreationFlag = NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW;
                            BOOL bSuc = CreateProcessA(NULL, cstrNewDosCmd, NULL, NULL, TRUE, dwCreationFlag, (LPVOID)temp_env, NULL, &si, &pi);
                            if (bSuc == 0)
                            {
                                DWORD err = GetLastError();
                                printf("cgi启动失败 ");
                                printf("error=%d\n", err);

                                printf("cstrNewDosCmd=%s\n", cstrNewDosCmd);
                                printf("temp_env=\n");
                                link_print(my_envp);

                                sprintf(length, "%d", strlen(html));
                                strcpy(response, header);
                                strcat(response, length);
                                strcat(response, "\r\n\r\n");
                                strcat(response, html);
                                send(readSet.fd_array[i], response, strlen(response) + 1, 0);
                                goto finish;
                            }
                            // 在读取管道内容前，关闭写管道
                            if (NULL != hWrite)
                            {
                                CloseHandle(hWrite);
                                hWrite = NULL;
                            }

                            // 发送http头
                            strcpy(response, "HTTP/1.1 200 OK\r\n");
                            strcpy(header, "Server: plusplus123/0.1\r\n");
                            strcat(response, header);
                            send(readSet.fd_array[i], response, strlen(response), 0);
                            printf("header=%s\n", header);
                            printf("response=%s\n", response);

                            // 读取管道内的所有内容
                            while (ReadFile(hRead, szBuffer, 1024, &dwRead, NULL))
                            {
                                // printf("%s", szBuffer);
                                send(readSet.fd_array[i], szBuffer, strlen(szBuffer) + 1, 0);
                                memset(szBuffer, 0, 1024);
                            }

                            // 等待一毫秒，等待浏览器接收完数据才关闭链接
                            Sleep(100);

                            CloseHandle(hRead);
                            CloseHandle(pi.hProcess);
                            CloseHandle(pi.hThread);

                            /***************************调用cgi end**************************/
                        }
                        // 静态请求
                        else
                        {
                            FILE *fp = NULL;
                            fp = fopen(temp_path, "r");

                            if (fp == NULL)
                            {
                                strcpy(header, "HTTP/1.1 404 Not Found\r\nServer: plusplus123/0.1\r\nContent-Type: text/html;charset=UTF-8\r\nContent-Length: ");
                                strcpy(html, "<html><head><title>404</title></head><body><h1>404 Not Found</h1></body></html");
                            }
                            else
                            {
                                strcpy(header, "HTTP/1.1 200 OK\r\nServer: plusplus123/0.1\r\nContent-Type: text/html;charset=UTF-8\r\nContent-Length: ");
                                char ch;
                                int i2 = 0;
                                while((ch = fgetc(fp)) != EOF)
                                {
                                    html[i2] = ch;
                                    i2++;
                                }
                                html[i2] = 0;
                            }

                            sprintf(length, "%d", strlen(html));
                            strcpy(response, header);
                            strcat(response, length);
                            strcat(response, "\r\n\r\n");
                            strcat(response, html);
                            send(readSet.fd_array[i], response, strlen(response) + 1, 0);
                            fp = NULL;
                        }

                        finish:

                        response[0] = '\0';
                        request_free(request);
                        request = NULL;

                        close_socket(&socketSet, socketSet.fd_array[sock_i]);
                    }
                    else if (nRecv == 0)
                    {
                        close_socket(&socketSet, socketSet.fd_array[sock_i]);
                        printf("a client %d disconnect.\n", readSet.fd_array[sock_i]);
                        continue;
                    }
                    else if (nRecv == SOCKET_ERROR)
                    {
                        close_socket(&socketSet, socketSet.fd_array[sock_i]);
                        printf("a client %d failed disconnect.\t\n", readSet.fd_array[sock_i]);
                        continue;
                    }
                }
            }
        }
        else if (nRetAll == 0)
        {
            // printf("time out!\n");
            continue;
        }
        else
        {
            printf("select error!%d\n", WSAGetLastError());
            Sleep(1000);
            continue;
        }
    }

    WSACleanup(); // 中止Windows Sockets在所有线程上的操作

    return 0;
}