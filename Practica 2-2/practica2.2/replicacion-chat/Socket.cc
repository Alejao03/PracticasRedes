//David Rodríguez y Alejandro Ortega
#include <string.h>

#include "Serializable.h"
#include "Socket.h"

Socket::Socket(const char * _direccion, const char * _puerto):sd(-1)
{
    //Construir un socket de tipo AF_INET y SOCK_DGRAM usando getaddrinfo.
    //Con el resultado inicializar los miembros sd, sa y sa_len de la clase
    struct addrinfo hints;
    struct addrinfo *res;

    memset((void*) &hints, 0, sizeof(struct addrinfo));

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    int a = getaddrinfo(_direccion, _puerto, &hints, &res);

    if(a != 0){
        perror("getaddrinfo\n");
        return;
    }

    sd = socket(res->ai_family, res->ai_socktype, 0);

    sa = *res->ai_addr;
    sa_len = res->ai_addrlen;

    if(a != 0){
        printf("[bind]\n");
        return;
    }

    freeaddrinfo(res);
}

int Socket::recv(Serializable &_obj, Socket * &_socket)
{
    struct sockaddr sa;
    socklen_t sa_len = sizeof(struct sockaddr);

    char buffer[MAX_MESSAGE_SIZE];

    ssize_t bytes = ::recvfrom(sd, buffer, MAX_MESSAGE_SIZE, 0, &sa, &sa_len);

    if ( bytes <= 0 )
    {
        return -1;
    }

    if ( _socket != 0 )
    {
        _socket = new Socket(&sa, sa_len);
    }

    _obj.from_bin(buffer);

    return 0;
}

int Socket::send(Serializable& _obj, const Socket& _socket)
{
    //Serializar el objeto
    //Enviar el objeto binario a sock usando el socket sd
    _obj.to_bin();

    sendto(sd, _obj.data(), _obj.size(), 0, &_socket.sa, _socket.sa_len);
}

bool operator== (const Socket &_s1, const Socket &_s2)
{
    //Comparar los campos sin_family, sin_addr.s_addr y sin_port
    //de la estructura sockaddr_in de los Sockets s1 y s2
    //Retornar false si alguno difiere
    struct sockaddr_in* socket1;
    struct sockaddr_in* socket2;

    socket1 = (struct sockaddr_in *) &_s1.sa;
    socket2 = (struct sockaddr_in *) &_s2.sa;

    return (socket1->sin_family == socket2->sin_family && socket1->sin_addr.s_addr == socket2->sin_addr.s_addr && socket1->sin_port == socket2->sin_port);
};

std::ostream& operator<<(std::ostream& _ostream, const Socket& s)
{
    char hostname[NI_MAXHOST];
    char server[NI_MAXSERV];

    getnameinfo((struct sockaddr *) &(s.sa), s.sa_len, hostname, NI_MAXHOST, server,
                NI_MAXSERV, NI_NUMERICHOST);

    _ostream << hostname << ":" << server;

    return _ostream;
};