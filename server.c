#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

// assumptions:
// lines are under 1024
// lines are received completely in one send/read

void die(char *msg, int status) {
    if(!errno)
        errno = ENOTRECOVERABLE;
    perror(msg);
    exit(status);
}

struct client_info {
    int socket;
    char clientid[25];
    FILE *destfile;
};

void initiate_client(struct client_info *client, char *clientid) {
    strncpy(client->clientid, clientid, 24);
    char filename[50];
    sprintf(filename, "c_%s.log", clientid);
    client->destfile = fopen(filename, "a");
}

void end_client(struct client_info *client) {
    fclose(client->destfile);
}

void process_line(struct client_info *client, char *line) {
    fprintf(client->destfile, "%s\n", line);
}

int read_line(struct client_info *client) {
    char buffer[1024] = {0};
    int readlen = recv(client->socket, buffer, 1024, 0);
    if(readlen <= 0)
        return 1; // some error / unclean disconnect
    if(buffer[0] == 'E') {
        end_client(client);
        send(client->socket, "K", 1, 0);
        return 2; // clean disconnect
    }
    if(buffer[0] == 'I') {
        buffer[20] = 0;
        initiate_client(client, buffer + 1);
        send(client->socket, "K", 1, 0);
        return 0; // still working
    }
    if(buffer[0] == 'L') {
        int plsize = buffer[1] * 100 + buffer[2] * 10 + buffer[3];
        // 0 terminate to process as string
        buffer[4 + plsize] = 0;
        process_line(client, buffer + 4);
        send(client->socket, "K", 1, 0);
        return 0; // still working
    }
    return 3; // unknown data
}

void process_client(int serverfd) {
    struct sockaddr_in address;
    int addrlen;
    int clientfd = accept(serverfd, (struct sockaddr *) &address, &addrlen);
    if(clientfd < 0)
        die("Can't accept client", 3);
    if(fork() > 0)
        return;
    struct client_info client;
    client.socket = clientfd;
    client.clientid[0] = 0;
    client.destfile = 0;
    while(read_line(&client) == 0);
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
