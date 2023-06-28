#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/wait.h>

#define SIZE 1048576
#define SERVERPORT 7799

int main()
{// 서버 연결
    int s_sock, c_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t c_addr_size;
    char hello[] = "Hello! I'm Server\n";

    s_sock = socket(AF_INET, SOCK_STREAM, 0);

    int option = 1;
    setsockopt(s_sock, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

    bzero(&server_addr, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVERPORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(s_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1)
        {
            perror("Can't bind a socket\n");
            exit(1);
        }

    listen(s_sock, 1);

    c_addr_size = sizeof(struct sockaddr);

    for (int i = 0; i < 3; i++) // 3개의 프로세스 생성 후 수행
    {
        char buf[SIZE] = { 0 };

        printf("Waiting for a Client..#%02d\n", i);
        c_sock = accept(s_sock, (struct sockaddr*)&client_addr, &c_addr_size);

        if (c_sock == -1)
        {
            perror("Can't accept a connection\n");
            exit(1);
        }

        int pid = fork();

        if (pid < 0)
        {
            perror("Fork Failed");
            return 1;
        }
        else if (pid == 0) //child
        {
            printf("I'm Child#%02d %d\n", i, getpid());
            printf("Connected: Client IP addr = %s port = %d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

            if (send(c_sock, hello, sizeof(hello) + 1, 0) == -1)
            {
                perror("Can't send message");
                exit(1);
            }

            printf("I said Hello to Client\n");

            if (recv(c_sock, buf, SIZE, 0) == -1)
            {
                perror("Can't receive message\n");
                exit(1);
            }

            printf("Client Says : %s\n", buf);

            close(c_sock);
        }
        else //parent
        {
            int wstatus;
            wait(&wstatus);

            printf("Process#%02d End %d\n", i, wstatus);

            close(s_sock);

            return 0;
        }
    }
}