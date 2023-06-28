#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>

#define SIZE 1048576
#define SERVERPORT 7799
#define MAX_THREADS 10

pthread_t tid[MAX_THREADS];
int s_sock, c_sock[MAX_THREADS];
int sum = 0; // 메모리 사용량 측정

void* serverFunc(void* arg)
{//전송할 파일이름을 받아서 Client에게 해당 파일을 전송함.
    int indent = *((int*)arg), *ret;
    sum += sizeof(indent);
    int accp_sock = c_sock[indent];
    sum += sizeof(accp_sock);

    char filename[SIZE];
    sum += sizeof(filename);
    char buf[SIZE] = { 0 };
    sum += sizeof(buf);

    if (recv(accp_sock, filename, SIZE, 0) == -1)
    {
        perror("Can't receive filename\n");
        exit(1);
    }
    printf("Client wants: %s\n", filename);

    FILE* fp = fopen(filename, "r");
    sum += sizeof(fp);
    if (fp == NULL)
    {
        perror("file-open error");
        exit(1);
    }

    int len = 0;
    sum += sizeof(len);

    while ((len = fread(buf, 1, SIZE, fp)) > 0)
    {
        printf("Sending data to Client : %s\n", buf);
        //sleep(1);
        if (send(accp_sock, buf, len, 0) == -1)
        {
            perror("Can't send data\n");
            exit(1);
        }
    }

    fclose(fp);
    close(s_sock);
    close(accp_sock);

    //printf("%s\n", buf);

    ret = (int*)malloc(sizeof(int));
    sum += sizeof(ret);
    *ret = indent;
    pthread_exit(ret);
}

int main(int argc, char* argv[])
{
    clock_t start, end;
    sum = sum + sizeof(start) + sizeof(end);

    int threads, *status, tcounts = 0, args[MAX_THREADS];
    sum = sum + sizeof(threads) + sizeof(status) + sizeof(tcounts) + sizeof(args);

    // 소켓 연결 수행
    struct sockaddr_in server_addr, client_addr;
    sum = sum + sizeof(server_addr) + sizeof(client_addr);
    socklen_t c_addr_size;
    sum += sizeof(c_addr_size);

    if (argc != 2)
    {
        printf("Usage: ./server <a number of thread>\n");
        return 1;
    }

    threads = atoi(argv[1]);

    if (threads >= 1 && threads > MAX_THREADS)
    {
        printf("MAX. number of thread is %d! Your input: %d\n", MAX_THREADS, threads);
        return 2;
    }

    printf("Your input: %d\n", threads);

    s_sock = socket(AF_INET, SOCK_STREAM, 0);

    int option = 1;
    sum += sizeof(option);
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

    start = clock(); // 실행시간 측정 시작

    for (int i = 0; i < threads; i++, tcounts++)
    {//thread 생성 및 종료 수행
        args[i] = i;
        
        listen(s_sock, 1);

        c_addr_size = sizeof(struct sockaddr);
        c_sock[i] = accept(s_sock, (struct sockaddr*)&client_addr, &c_addr_size);

        printf("Check: s_sock = %d c_sock = %d\n", s_sock, c_sock[i]);
        printf("Check: client IP addr = %s port = %d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        if (c_sock[i] == -1)
        {
            perror("Can't accept a connection\n");
            exit(1);
        }

        if (pthread_create(&tid[i], NULL, serverFunc, &args[i]) != 0)
        {
            perror("Failed to create thread\n");
            goto exit;
        }
    }

exit:
    for (int i = 0; i < tcounts; i++)
    {
        pthread_join(tid[i], (void**)&status);
        printf("Thread no.%d ends: %d\n", i, *status);
    }

    end = clock(); // 실행시간 측정 종료

    printf("%f ms\n", (double)(end - start));
    printf("memory usage : %d\n", sum);

    return 0;
}