//David Rodríguez y Alejandro Ortega
#include "Chat.h"

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

void ChatMessage::to_bin()
{
    alloc_data(MESSAGE_SIZE);

    memset(_data, 0, MESSAGE_SIZE);

    //Serializar los campos type, nick y message en el buffer _data
    char* punteroBuffer = _data;

    memcpy(punteroBuffer, &type, sizeof(uint8_t));
    punteroBuffer += sizeof(uint8_t);

    memcpy(punteroBuffer, nick.c_str(), 8 * sizeof(char));
    punteroBuffer += 8 * sizeof(char);

    memcpy(punteroBuffer, message.c_str(), 80 * sizeof(char));
}

int ChatMessage::from_bin(char* bobj)
{
    alloc_data(MESSAGE_SIZE);

    memcpy(static_cast<void>(_data), _obj, MESSAGE_SIZE);

    //Reconstruir la clase usando el buffer _data
    char punteroBuffer = _data;

    memcpy(&type, punteroBuffer, sizeof(uint8_t));
    punteroBuffer += sizeof(uint8_t);

    char nombre[8];
    mempcpy(&nombre, punteroBuffer, 8 * sizeof(char));
    punteroBuffer += 8 * sizeof(char);
    nick = nombre;

    char mensaje[80];
    memcpy(&mensaje, punteroBuffer, 80 * sizeof(char));
    message = mensaje;

    return 0;
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

void ChatServer::do_messages()
{
    while (true)
    {
        /*
            *NOTA: los clientes están definidos con "smart pointers", es necesario
            * crear un unique_ptr con el objeto socket recibido y usar std::move
            * para añadirlo al vector
            * */

            //Recibir Mensajes en y en función del tipo de mensaje
            // - LOGIN: Añadir al vector clients
            // - LOGOUT: Eliminar del vector clients
            // - MESSAGE: Reenviar el mensaje a todos los clientes (menos el emisor)
    }
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

void ChatClient::login()
{
    std::string msg;

    ChatMessage em(nick, msg);
    em.type = ChatMessage::LOGIN;

    socket.send(em, socket);
}

void ChatClient::logout()
{
    // Completar
}

void ChatClient::input_thread()
{
    while (true)
    {
        // Leer stdin con std::getline
        // Enviar al servidor usando socket
    }
}

void ChatClient::net_thread()
{
    while (true)
    {
        //Recibir Mensajes de red
        //Mostrar en pantalla el mensaje de la forma "nick: mensaje"
    }
}