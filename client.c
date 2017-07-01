#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "./id.c"

void die(char *msg, int status) {
    if(!errno)
        errno = ENOTRECOVERABLE;
    perror(msg);
    exit(status);
}

void clean_disconnect(int serverfd) {
    char ack;
    send(serverfd, "E", 1, 0);
    recv(serverfd, &ack, 1, 0);
    close(serverfd);
}

void send_line(int serverfd, char *line) {
    //NOTE line is 0 terminated, send message is not
    char buffer[1024];
    char ack;
    int plsize = strlen(line) + 1;
    sprintf(buffer, "L%03d%s", (int) strlen(line), line);
    send(serverfd, buffer, strlen(line) + 3, 0);
    if(recv(serverfd, &ack, 1, 0) < 0)
        die("Did not get ACK", 7);
    //TODO maybe check value of ack ?
}

void send_log(int serverfd, int clientid, char *filename) {
    char buffer[1024];
    FILE *logfile = fopen(filename, "r");
    while(fgets(buffer, sizeof(buffer), logfile)) {
        send_line(serverfd, buffer);
        sleep(1);
    }
    fclose(logfile);
}

int main(int argc, char *argv[]) {
    if(argc < 2)
        die("logfile required", 6);
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0)
        die("Can't start server", 1);
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8109);
    if(inet_pton(AF_INET, "127.0.0.1", &address.sin_addr) <= 0)
        die("Can't understand localhost IP", 4);
    if(connect(sockfd, (struct sockaddr *) &address, sizeof(address)) < 0)
        die("Can't connect to server", 5);
    unsigned long clientid = stouid(argv[1]);
    printf("I am %lu, sending %s\n", clientid, argv[1]);
    send_log(sockfd, clientid, argv[1]);
    clean_disconnect(sockfd);
    return 0;
}
