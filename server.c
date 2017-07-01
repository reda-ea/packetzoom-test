#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

// assumptions:
// lines are under 1024
// lines are received completely in one send/read

void die(char *msg, int status) {
    if(!errno)
        errno = ENOTRECOVERABLE;
    perror(msg);
    exit(status);
}

void process_line(char *line) {
    printf("%s\n", line);
}

int read_line(clientfd) {
    char buffer[1024] = {0};
    int readlen = recv(clientfd , buffer, 1024, 0);
    if(readlen <= 0)
        return 1;
    int plsize = buffer[1] * 100 + buffer[2] * 10 + buffer[3];
    // 0 terminate to process as string
    buffer[4 + plsize] = 0;
    process_line(buffer + 4);
    send(clientfd, "K", 1, 0);
    return 0;
}

void process_client(int serverfd) {
    struct sockaddr_in address;
    int addrlen;
    int clientfd = accept(serverfd, (struct sockaddr *) &address, &addrlen);
    if(clientfd < 0)
        die("Can't accept client", 3);
    if(fork() > 0)
        return;

    while(read_line(clientfd) == 0);
    //FIXME this is reached both on error and disconnect, process differently
    close(clientfd);
    exit(0);
}

int main(int argc, char *argv[]) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0)
        die("Can't start server", 1);
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8109);
    if(bind(sockfd, (struct sockaddr *) &address, sizeof(address)) < 0)
        die("Can't listen on 8109", 2);
    listen(sockfd, 5);
    for(;;)
        process_client(sockfd);
    return 0;
}
