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

#define MAX_THREADS 5

class MessageThread {
private:
    int socket_;

public:

    void do_message()
    {
        while (true)
        {
            char buffer[80] = {};

            sockaddr client;
            socklen_t clientlen = sizeof(sockaddr);

            int bytes = recvfrom(socket_, buffer, 80, 0, &client, &clientlen);

            if (bytes < 0)
            {
                fprintf(stderr, "Error recvfrom: %s\n", gai_strerror(bytes));
                continue;
            }

            char hostname[NI_MAXHOST];
            char serv[NI_MAXSERV];
            int a = getnameinfo(&client, clientlen, hostname, NI_MAXHOST, serv, NI_MAXSERV, NI_NUMERICHOST | NI_NUMERICSERV);

            if (a != 0)
            {
                fprintf(stderr, "Error getnameinfo: %s\n", gai_strerror(a));
                continue;
            }

            sleep(3);

            std::cout << "thread " << std::this_thread::get_id() << ": " << bytes << " bytes de " << hostname << ":" << serv << std::endl;

            time_t rawTime;
            tm* timeInfo;

            time(&rawTime);

            timeInfo = localtime(&rawTime);

            char* format;

            switch (buffer[0])
            {
            case 't': //Hora
                format = "%r";
                break;
            case 'd': //Fecha
                format = "%F";
                break;
            default:
                printf("Comando no soportado %s\n", buffer);
                sendto(socket_, "Comando no soportado\n", 21, 0, &client, clientlen);
                continue;
            }

            char send[80];
            int timeBytes = strftime(send, 80, format, timeInfo);

            sendto(socket_, send, timeBytes + 1, 0, &client, clientlen);
        }
    }

    MessageThread(int socket) : socket_(socket) { }
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
    hints.ai_socktype = SOCK_DGRAM;

    int a = getaddrinfo(argv[1], argv[2], &hints, &info);

    if (a != 0)
    {
        fprintf(stderr, "Error getaddrinfo: %s\n", gai_strerror(a));
        return -1;
    }

    int udp = socket(info->ai_family, info->ai_socktype, 0);

    if (udp == -1)
    {
        fprintf(stderr, "Error socket %s\n", gai_strerror(udp));
        return -1;
    }

    bind(udp, info->ai_addr, info->ai_addrlen);

    freeaddrinfo(info);

    for (int i = 0; i < MAX_THREADS; ++i) {
        MessageThread* mthread = new MessageThread(udp);

        std::thread([mthread]() {
            mthread->do_message();
            delete mthread;
            }).detach();
    }

    char c;
    while (std::cin >> c)
    {
        if (c == 'q')
            break;
    }

    close(udp);

    return 0;
}