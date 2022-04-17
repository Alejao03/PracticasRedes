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
    if (argc < 4)
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

    freeaddrinfo(info);

    char buffer[80];

    sendto(udp, argv[3], strlen(argv[3]), 0, info->ai_addr, info->ai_addrlen);

    //El tercer argumento q => salir
    if (argv[3][0] == 'q')
        return 0;

    int n = recvfrom(udp, buffer, 80, 0, info->ai_addr, &info->ai_addrlen);

    buffer[n] = '\n';
    printf("%s", buffer);
    close(udp);
    return 0;
}