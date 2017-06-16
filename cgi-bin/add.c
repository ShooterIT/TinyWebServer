/*
* cgi测试
*/

#include <stdio.h>
#include <stdlib.h>

int main()
{
    char *buf, *p;
    char arg1[1024],arg2[1024],content[1024];
    int n1 = 0, n2 = 0;

    //获取参数
    if((buf = getenv("QUERY_STRING")) != NULL){
        p = strchr(buf, '&');
        *p = 0;
        strcpy(arg1,buf);
        strcpy(arg2,p+1);
        n1 = atoi(arg1);
        n2 = atoi(arg2);
    }


    sprintf(content, "<p>Welcome to add: %d + %d = %d\r\n</p>\
        <p>Thanks for visiting!\r\n</p>",n1,n2,n1+n2);
    //HTTP响应报文
    printf("Connection: close\r\n");
    printf("Content-type: text/html\r\n");
    printf("Content-length: %d\r\n", (int)strlen(content));
    //报文结束
    printf("\r\n");
    //HTTP响应实体
    printf("%s", content);
    fflush(stdout);

    return 0;
}
