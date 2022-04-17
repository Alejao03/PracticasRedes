//David Rodriguez y Alejandro Ortega

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <string.h>
#include <unistd.h>
#include <time.h>

#include <thread>
#include <iostream>

class MessageThreadTCP
{
private:
    int socket_;

public:

    void do_message()
    {
        while (true)
        {
            char buffer[80] = {};

            int bytes = recv(socket_, buffer, 80, 0);

            if (bytes <= 0) //Si bytes es 0, salimos
            {
                printf("Conexión terminada\n");
                break;
            }

            send(socket_, buffer, bytes, 0);
        }
        close(socket_);
    }

    MessageThreadTCP(int socket) : socket_(socket) {}
};

int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        printf("Necesita mas argumentos\n");
        return 1;
    }

    addrinfo* info;
    addrinfo hints;
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

    char host[NI_MAXHOST];
    char port[NI_MAXSERV];

    while (true)
    {
        //Accept
        int client_sd = accept(tcp, &client, &clientlen);
        a = getnameinfo(&client, clientlen, host, NI_MAXHOST, port, NI_MAXSERV, NI_NUMERICHOST | NI_NUMERICSERV);

        if (a != 0)
        {
            fprintf(stderr, "Error getnameinfo: %s\n", gai_strerror(a));
            return -1;
        }

        printf("Conexion desde %s %s\n", host, port);

        MessageThreadTCP* mthreadTCP = new MessageThreadTCP(client_sd);

        std::thread([mthreadTCP]() {
            mthreadTCP->do_message();
            delete mthreadTCP;
            }).detach();

            char* msg = "Conexión aceptada\n";
            send(client_sd, msg, strlen(msg), 0);
    }

    close(tcp);

    return 0;
}