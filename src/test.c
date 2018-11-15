#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[], char **envp)
{
    printf("Content-Type: text/html; charset=utf-8\n\n");
    printf("<pre>\n");
    printf("test");
    printf( "\nEnvironment variables:\n" );
    while( *envp != NULL )
        printf( "  %s\n", *(envp++) );

    printf("PATH=%s\n", getenv("PATH"));
    printf("THIS=%s\n", getenv("THIS"));

    printf("\n%s\n", envp);

    char poststr[4096];
    fgets(poststr, 4096, stdin);
    printf("\npoststr=%s\n", poststr);
    printf("</pre>\n");
    return 0;
}