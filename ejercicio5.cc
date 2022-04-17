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
        printf("Necesita mas argumentos \n");
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
    if (tcp < 0)
    {
        fprintf(stderr, "Error socket %s\n", gai_strerror(tcp));
        return -1;
    }

    if (connect(tcp, info->ai_addr, info->ai_addrlen) < 0)
    {
        printf("Error direccion");
        return -1;
    }

    while (true)
    {
        char* buffer = NULL;
        size_t len = 80;
        ssize_t line = 0;

        line = getline(&buffer, &len, stdin);

        if (line == 2 && (buffer[0] == 'Q' || buffer[0] == 'q')) //Si escribimos "q" salimos
            break;

        send(tcp, buffer, line - 1, 0);

        int n = recv(tcp, buffer, 79, 0);

        buffer[n] = '\n';
        printf("%s", buffer);
    }
    close(tcp);
    return 0;
}