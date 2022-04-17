//David Rodriguez y Alejandro Ortega

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <string.h>
#include <unistd.h>
#include <time.h>

//Direccion = primer argumento
//Puerto = segundo argumento
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

    while (true)
    {
        char buffer[80] = {};

        sockaddr client;
        socklen_t clientlen = sizeof(sockaddr);

        int bytes = recvfrom(udp, buffer, 80, 0, &client, &clientlen);

        char hostname[NI_MAXHOST];
        char serv[NI_MAXSERV];
        a = getnameinfo(&client, clientlen, hostname, NI_MAXHOST, serv, NI_MAXSERV, NI_NUMERICHOST | NI_NUMERICSERV);

        if (a != 0)
        {
            fprintf(stderr, "Error getnameinfo: %s\n", gai_strerror(a));
            continue;
        }

        printf("%d bytes de %s:%s\n", bytes, hostname, serv);

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
        case 'q': //Terminar proceso
            printf("Saliendo...\n");
            close(udp);
            return 0;
        default:
            printf("Comando no soportado %s\n", buffer);
            sendto(udp, "Comando no soportado\n", 21, 0, &client, clientlen);
            continue;
        }

        char send[80];
        int timeBytes = strftime(send, 80, format, timeInfo);

        sendto(udp, send, timeBytes + 1, 0, &client, clientlen);
    }

    close(udp);

    return 0;
}