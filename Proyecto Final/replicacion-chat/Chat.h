#include <string>

#include <unistd.h>

#include <string.h>

#include <vector>

#include <memory>



#include "Serializable.h"

#include "Socket.h"

#include <list>



//------------------------------------------------------------------------------

//------------------------------------------------------------------------------



/**

 *  Mensaje del protocolo de la aplicación de Chat

 *

 *  +-------------------+

 *  | Tipo: uint8_t     | 0 (login), 1 (mensaje), 2 (logout)

 *  +-------------------+

 *  | Nick: char[8]     | Nick incluido el char terminación de cadena '\0'

 *  +-------------------+

 *  |                   |

 *  | Mensaje: char[80] | Mensaje incluido el char terminación de cadena '\0'

 *  |                   |

 *  +-------------------+

 *

 */

class ChatMessage : public Serializable

{

public:

    static const size_t MESSAGE_SIZE = sizeof(char) * 88 + sizeof(uint8_t);



    enum MessageType

    {

        LOGIN = 0,

        MESSAGE = 1,

        LOGOUT = 2,

        TRY = 3,

        ERROR = 4,

        PALABRA = 5,

        USADAS = 6,

        TUTURNO = 7,

        NOTURNO = 8,

        SUTURNO = 9,

        OCULTA = 10,

        VICTORIA = 11,

        DERROTA = 12,

        FIN = 13,

        START = 14,

        WAIT = 15,

        LOBBY = 16,

        NOSTART = 17

    };



    ChatMessage() {};



    ChatMessage(const std::string& n, const std::string& m) :nick(n), message(m) {};



    void to_bin();



    int from_bin(char* bobj);



    uint8_t type;



    std::string nick;

    std::string message;

};





// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------



/**

 *  Clase para el servidor de chat

 */

class ChatServer

{

public:

    ChatServer(const char* s, const char* p) : socket(s, p)

    {

        socket.bind();

    };



    /**

     *  Thread principal del servidor recive mensajes en el socket y

     *  lo distribuye a los clientes. Mantiene actualizada la lista de clientes

     */

    void do_messages();



private:

    /**

     *  Lista de clientes conectados al servidor de Chat, representados por

     *  su socket

     */

    std::vector<std::unique_ptr<Socket>> clients;

    std::vector<std::unique_ptr<Socket>> waiting;

    std::vector<std::string> nicksClient;

    std::vector<std::string> nicksLobby;

    /**

     * Socket del servidor

     */

    Socket socket;



public:

    std::string palabras;

    std::string ocultas;



    int letrasOcultas;



    std::list<std::string> usadas;



    int turno = 0;

    bool empezado = false;

};



// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------



/**

 *  Clase para el cliente de chat

 */

class ChatClient

{

public:

    /**

     * @param s dirección del servidor

     * @param p puerto del servidor

     * @param n nick del usuario

     */

    ChatClient(const char* s, const char* p, const char* n) :socket(s, p),

        nick(n) {};



    /**

     *  Envía el mensaje de login al servidor

     */

    void login();



    /**

     *  Envía el mensaje de logout al servidor

     */

    void logout();



    /**

     *  Rutina principal para el Thread de E/S. Lee datos de STDIN (std::getline)

     *  y los envía por red vía el Socket.

     */

    void input_thread();



    /**

     *  Rutina del thread de Red. Recibe datos de la red y los "renderiza"

     *  en STDOUT

     */

    void net_thread();



private:



    /**

     * Socket para comunicar con el servidor

     */

    Socket socket;



    /**

     * Nick del usuario

     */

    std::string nick;

};



