//David Rodríguez y Alejandro Ortega
//¿Qué hace el comando od? - El comando od muestra el contenido byte a byte del archivo en octal.
//¿Qué relación hay entre la salida del comando y la serialización? - El tamaño de los datos serializados es el mismo que si lo comprobamos con el comando od.

#include "Serializable.h"

#include <iostream>
#include <string>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

class Jugador : public Serializable
{
public:
    Jugador(const char* _n, int16_t _x, int16_t _y) :pos_x(_x), pos_y(_y)
    {
        strncpy(name, _n, MAX_NAME);
    };

    virtual ~Jugador() {};

    void to_bin()
    {
        int32_t data_size = MAX_NAME * sizeof(char) + 2 * sizeof(int16_t);
        alloc_data(data_size);

        char* d = _data;

        memcpy(d, name, MAX_NAME * sizeof(char));
        d += MAX_NAME * sizeof(char);

        memcpy(d, &pos_x, sizeof(int16_t));
        d += sizeof(int16_t);

        memcpy(d, &pos_y, sizeof(int16_t));
    }

    int from_bin(char* data)
    {
        char* d = data;

        memcpy(name, d, MAX_NAME * sizeof(char));
        d += MAX_NAME * sizeof(char);

        memcpy(&pos_x, d, sizeof(int16_t));
        d += sizeof(int16_t);

        memcpy(&pos_y, d, sizeof(int16_t));
        d += sizeof(int16_t);
        return 0;
    }


public:
    int16_t pos_x;
    int16_t pos_y;

    static const size_t MAX_NAME = 80;

    char name[MAX_NAME];
};

int main(int argc, char** argv)
{
    Jugador one_w("Player_ONE", 12, 345);
    Jugador one_r("", 0, 0);

    // 1. Serializar el objeto one_w
    printf("Serializando jugador one_w\n");
    one_w.to_bin();

    // 2. Escribir la serialización en un fichero
    char* fileName = "./jugadorSerializado";
    int fd = open(fileName, O_CREAT | O_TRUNC | O_RDWR, 0666);
    write(fd, one_w.data(), one_w.size());
    close(fd);

    // 3. Leer el fichero
    printf("Leyendo archivo serializado\n");
    fd = open(fileName, O_RDONLY, 0666);
    char data[one_w.size()];
    read(fd, data, one_w.size());
    close(fd);

    // 4. "Deserializar" en one_r
    printf("Deserializando datos en one_r\n");
    one_r.from_bin(data);

    // 5. Mostrar el contenido de one_r
    printf("Player name: %s\n", one_r.name);
    printf("Player x pos: %d\n", one_r.pos_x);
    printf("Player y pos: %d\n", one_r.pos_y);

    return 0;
}

