//David Rodríguez y Alejandro Ortega
#include "Chat.h"

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

void ChatMessage::to_bin()
{
    alloc_data(MESSAGE_SIZE);

    memset(_data, 0, MESSAGE_SIZE);

    //Serializar los campos type, nick y message en el buffer _data
    char* datos = _data;
    memcpy(datos, &type, sizeof(uint8_t));
    datos += sizeof(uint8_t);
    memcpy(datos, nick.c_str(), 10 * sizeof(char));
    datos += 10 * sizeof(char);
    memcpy(datos, message.c_str(), 80 * sizeof(char));
}

int ChatMessage::from_bin(char * bobj)
{
    alloc_data(MESSAGE_SIZE);

    memcpy(static_cast<void *>(_data), bobj, MESSAGE_SIZE);

    char datosNick [10];
    char datosMensaje [80];

    //Reconstruir la clase usando el buffer _data
    char* datos = _data;
    memcpy(&type, datos, sizeof(uint8_t));
    datos += sizeof(uint8_t);
    memcpy(&datosNick, datos, 10 * sizeof(char));
    datos += 10 * sizeof(char);
    memcpy(&datosMensaje, datos, 80 * sizeof(char));
    nick = datosNick;
    message = datosMensaje;

    return 0;
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

void ChatServer::do_messages()
{
    while (true)
    {
        /*
         * NOTA: los clientes están definidos con "smart pointers", es necesario
         * crear un unique_ptr con el objeto socket recibido y usar std::move
         * para añadirlo al vector
         */
        Socket* clientSd;
        ChatMessage msg;
        socket.recv(msg, clientSd);
        //Recibir Mensajes en y en función del tipo de mensaje
        // - LOGIN: Añadir al vector clients
        // - LOGOUT: Eliminar del vector clients
        // - MESSAGE: Reenviar el mensaje a todos los clientes (menos el emisor)
        if (msg.type == ChatMessage::MessageType::LOGIN) {
            std::cout << msg.nick << " se ha unido\n";
            clients.push_back(std::unique_ptr<Socket>(std::move(clientSd)));
        }
        else if (msg.type == ChatMessage::MessageType::LOGOUT) {
            std::cout << msg.nick << " se ha desconectado\n";
            auto it = clients.begin();
            while (it != clients.end() && !(*(*it).get() == *clientSd)) it++;
            clients.erase(it);
        }
        else if (msg.type == ChatMessage::MessageType::MESSAGE) {
            std::cout << msg.nick << " ha enviado un mensaje\n";
            for (int i = 0; i < clients.size(); ++i) {
                if (!(*(clients[i].get()) == *clientSd))
                    socket.send(msg, (*clients[i].get()));
            }
        }
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
    std::string msg;

    ChatMessage em(nick, msg);
    em.type = ChatMessage::LOGOUT;

    socket.send(em, socket);
}

void ChatClient::input_thread()
{
    while (true)
    {
        // Leer stdin con std::getline
        // Enviar al servidor usando socket
        std::string msg;
        std::getline(std::cin, msg);
        if (msg == "q") {
            logout();
            break;
        }
        ChatMessage chatMsg(nick, msg);
        chatMsg.type = ChatMessage::MessageType::MESSAGE;
        socket.send(chatMsg, socket);
    }
}

void ChatClient::net_thread()
{
    while (true)
    {
        //Recibir Mensajes de red
        //Mostrar en pantalla el mensaje de la forma "nick: mensaje"
        ChatMessage chatMsg;
        socket.recv(chatMsg);
        std::cout << chatMsg.nick << ": " << chatMsg.message << "\n";
    }
}
