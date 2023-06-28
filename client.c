#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define SIZE 1048576
#define SERVERPORT 7799
#define MAX_THREADS 10

pthread_t tid[MAX_THREADS];
char** global_argv;

void* clientFunc(void* arg)
{//서버에 연결하고 전송받은 파일을 새로운 파일로 만듦.
    int indent = *(int*)arg;
    int c_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t c_addr_size;
    char buf[SIZE] = { 0 };
    char hello[] = "Hello! I'm Client\n";

    //printf("%d\n", indent);

    c_sock = socket(AF_INET, SOCK_STREAM, 0);

    bzero(&server_addr, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVERPORT);
    server_addr.sin_addr.s_addr = htons(INADDR_ANY);

    if (connect(c_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("Can't connect to a Server");
        exit(1);
    }

    printf("Check: c_sock = %d\n", c_sock);

    char filename[SIZE];
    strcpy(filename, global_argv[indent + 1]);

    printf("Sending filename %s\n", filename);
    //sleep(1);
    if (send(c_sock, filename, strlen(filename) + 1, 0) == -1)
    {
        perror("Can't send filename\n");
        exit(1);
    }

    int rlen = 0;
    if ((rlen = recv(c_sock, buf, SIZE, 0)) == -1)
    {
        perror("Can't receive data\n");
        exit(1);
    }
    printf("received data : %s\n", buf);

    char received_filename[100] = "received_";
    strcat(received_filename, filename);
    //printf("received filename : %s\n", received_filename);

    FILE* fp = fopen(received_filename, "w");

    fwrite(buf, 1, rlen, fp);

    printf("%s received\n", filename);

    fclose(fp);
    close(c_sock);

    int* retval = (int*)malloc(sizeof(int));
    *retval=indent;
    pthread_exit(retval);
    //printf("Thread Finished : %d\n", *retval);
}

int main(int argc, char* argv[])
{
    int threads, *status, tcounts = 0;

    //printf("%d\n", argc);
    threads = atoi(argv[argc - 1]);

    if (argc != threads + 2)
    {
        printf("Usage: ./client <a number of thread>\n");
        return 1;
    }

    if (threads >= 1 && threads > MAX_THREADS)
    {
        printf("MAX. number of thread is %d! Your input: %d\n", MAX_THREADS, threads);
        return 2;
    }

    printf("Your input: %d\n", threads);

    global_argv = argv;

    int args[MAX_THREADS];

    for (int i = 0; i < threads; i++, tcounts++)
    {
        args[i] = i;
        if (pthread_create(&tid[i], NULL, clientFunc, &args[i]) != 0)
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

    return 0;
}