#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SIZE 1048576
#define SERVERPORT 7799

int main()
{
    int c_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t c_addr_size;
    char buf[SIZE] = { 0 };
    char hello[] = "Hello! I'm Client\n";

    c_sock = socket(AF_INET, SOCK_STREAM, 0);

    bzero(&server_addr, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVERPORT);
    server_addr.sin_addr.s_addr = htons(INADDR_ANY);

    printf("Connecting...\n");

    if (connect(c_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("Can't connect to a Server");
        exit(1);
    }

    printf("Connected!\n");

    if (recv(c_sock, buf, SIZE, 0) == -1)
    {
        perror("Can't receive message\n");
        exit(1);
    }

    printf("Server says: %s\n", buf);

    if (send(c_sock, hello, sizeof(hello) + 1, 0) == -1)
    {
        perror("Can't send message\n");
        exit(1);
    }

    printf("I said Hi to Server\n");

    //printf("I am going to sleep\n");
    //sleep(10);

    close(c_sock);

    return 0;
}