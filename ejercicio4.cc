//David Rodriguez y Alejandro Ortega

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <string.h>
#include <unistd.h>
#include <time.h>

int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        printf("Necesita mas argumentos");
        return 1;
    }

    struct addrinfo* info;
    struct addrinfo hints;
    memset(&hints, 0, sizeof(addrinfo));

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    int a = getaddrinfo(argv[1], argv[2], &hints, &info);

    if (a != 0)
    {
        fprintf(stderr, "Error getaddrinfo: %s\n", gai_strerror(a));
        return -1;
    }

    int tcp = socket(info->ai_family, info->ai_socktype, 0);

    if (tcp == -1)
    {
        fprintf(stderr, "Error socket %s\n", gai_strerror(tcp));
        return -1;
    }

    bind(tcp, info->ai_addr, info->ai_addrlen);

    freeaddrinfo(info);

    //Lo ponemos en estado LISTEN
    listen(tcp, 16);

    sockaddr client;
    socklen_t clientlen = sizeof(sockaddr);

    //Accept
    int client_sd = accept(tcp, &client, &clientlen);

    char host[NI_MAXHOST];
    char port[NI_MAXSERV];
    a = getnameinfo(&client, clientlen, host, NI_MAXHOST, port, NI_MAXSERV, NI_NUMERICHOST | NI_NUMERICSERV);

    if (a != 0)
    {
        fprintf(stderr, "Error getnameinfo: %s\n", gai_strerror(a));
        return -1;
    }

    printf("Conexion desde %s %s\n", host, port);

    while (true)
    {
        char buffer[80] = {};

        int bytes = recv(client_sd, buffer, 80, 0);

        if (bytes <= 0) //Si bytes es 0, salimos
        {
            printf("Conexión terminada\n");
            break;
        }

        send(client_sd, buffer, bytes, 0);
    }

    close(tcp);

    return 0;
}