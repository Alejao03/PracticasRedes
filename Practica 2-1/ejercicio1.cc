//David Rodriguez y Alejandro Ortega

#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>


int main(int argc, char* argv[]) {

    if (argc < 2)
    {
        printf("Please write. %s domain.xx or %s IP", argv[0], argv[0]);
        return 1;
    }

    struct addrinfo* info;
    int a = getaddrinfo(argv[1], argv[2], NULL, &info);

    if (a != 0) {
        fprintf(stderr, "Error getaddrinfo: %s\n", gai_strerror(a));
        return -1;
    }

    struct addrinfo* it;

    for (it = info; it != NULL; it = it->ai_next) {
        char hostname[NI_MAXHOST];
        a = getnameinfo(it->ai_addr, it->ai_addrlen, hostname, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
        if (a != 0) {
            fprintf(stderr, "Error getnameinfo: %s\n", gai_strerror(a));
            continue;
        }
        if (hostname != "\0") {
            printf("%s    %d    %d\n", hostname, it->ai_family, it->ai_socktype);
        }
    }

    freeaddrinfo(info);

    return 0;
}