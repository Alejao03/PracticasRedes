//David Rodríguez y Alejandro Ortega



#include "Chat.h"

#include <cstring>



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



int ChatMessage::from_bin(char* bobj)

{

    alloc_data(MESSAGE_SIZE);

    memcpy(static_cast<void*>(_data), bobj, MESSAGE_SIZE);



    char datosNick[10];

    char datosMensaje[80];



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

        Socket* clientSd;

        ChatMessage msg;

        socket.recv(msg, clientSd);



        //Recibir Mensajes en y en función del tipo de mensaje



        // - LOGIN: Añadir al vector clients

        // - LOGOUT: Eliminar del vector clients

        // - TRY: Intento de una letra (comprueba repetición, lista usadas, acierto...)

        // - ERROR: Letra repetida

        // - USADAS: Mensaje con la lista de letras usadas

        // - TUTURNO: Mensaje que indica que es tu turno

        // - NOTURNO: Mensaje que sale al escribir cuando no es tu turno

        // - SUTURNO: Mensaje que indica de quien es el turno

        // - OCULTA: Muestra la palabra oculta (se va actualizando según acertamos letras)

        // - VICTORIA: Mensaje de victoria

        // - DERROTA: Mensaje de derrota (dice quien ha ganado)

        // - START: Se empieza la partida (generamos palabra)

        // - WAIT: Te conectas en medio de una partida (te quedas en espera)

        // - LOBBY: Tras terminar partida, la lista de espera se añade en la lista clientes

        // - NOSTART: La partida no esta iniciada



        if (msg.type == ChatMessage::MessageType::LOGIN) {

            if (!empezado)

            {

                std::cout << msg.nick << " se ha unido\n";



                clients.push_back(std::unique_ptr<Socket>(std::move(clientSd)));

                nicksClient.push_back(msg.nick);

                int a = clients.size();

                std::cout << "Jugadores: " << a << "\n";

                if (clients.size() >= 2)

                {

                    msg.message = std::to_string(a);

                    socket.send(msg, (*clients[0].get()));

                }

            }

            else

            {

                //Ponemos a jugador en espera

                std::cout << msg.nick << " esta waiting\n";

                waiting.push_back(std::unique_ptr<Socket>(std::move(clientSd)));

                nicksLobby.push_back(msg.nick);

                msg.type = ChatMessage::MessageType::WAIT;

                for (int i = 0; i < waiting.size(); ++i) {

                    if ((*(waiting[i].get()) == *clientSd))

                        socket.send(msg, (*waiting[i].get()));

                }

            }

        }

        else if (msg.type == ChatMessage::MessageType::LOBBY) {

            //Los jugadores waiting pasan a la lobby

            while (waiting.size() > 0)

            {

                std::cout << nicksLobby[nicksLobby.size() - 1] << " se ha unido\n";





                clients.push_back(std::unique_ptr<Socket>(std::move(waiting.back())));

                waiting.pop_back();

                nicksClient.push_back(nicksLobby.back());

                nicksLobby.pop_back();

            }



            int a = clients.size();

            msg.message = std::to_string(a);

            msg.type = ChatMessage::MessageType::LOGIN;

            socket.send(msg, (*clients[0].get()));



            usadas.clear();

            turno = 0;

        }

        else if (msg.type == ChatMessage::MessageType::START)

        {

            empezado = true;



            char buff[15];



            FILE* f = fopen("diccionario.dat", "r");

            srand(time(NULL));

            int a = rand() % 100;

            int vuelta = 0;



            fgets(buff, 1024, f);



            while (a != vuelta)

            {

                fgets(buff, 15, f);

                vuelta++;

            }



            int tam = 0;

            int i = 0;

            while (buff[i] != '\n' && buff[i] != ' ' && buff[i] != '\0')

            {

                tam++;

                i++;

            }



            std::string palabra = buff;

            std::string oculta = palabra;



            for (int i = 0; i < tam; i++)

            {

                oculta[i] = '_';

            }



            ocultas = oculta;

            letrasOcultas = tam;

            std::cout << letrasOcultas << "\n";



            // std::cout << "Oculta: " << oculta << "\n";

            palabras = palabra;

            std::cout << palabras;

            // std::cout << "TAM: " << palabras.size();



            for (int i = 0; i < oculta.size(); i++)

            {

                std::cout << oculta[i];

            }



            for (int i = 0; i < clients.size(); ++i) {

                msg.message = ocultas;

                msg.type = ChatMessage::MessageType::OCULTA;

                socket.send(msg, (*clients[i].get()));

            }



            //Para 2 o mas jugadores

            for (int i = 0; i < clients.size(); ++i) {

                if (i == 0)

                    msg.type = ChatMessage::MessageType::TUTURNO;

                else

                {

                    msg.type = ChatMessage::MessageType::SUTURNO;

                    msg.nick = nicksClient[turno];

                }

                socket.send(msg, (*clients[i].get()));

            }



            //Para 2 jugadores

            // //Al primer jugador

            // msg.type=ChatMessage::MessageType::TUTURNO;

            // socket.send(msg, (*clients[0].get()));



            // //Al segundo jugador

            // msg.type=ChatMessage::MessageType::SUTURNO;

            // msg.nick = nicksClient[0];

            // socket.send(msg, (*clients[1].get()));



            std::cout << "\n";



            fclose(f);

        }

        else if (msg.type == ChatMessage::MessageType::LOGOUT) {

            std::cout << msg.nick << " se ha desconectado\n";



            int jugador;

            for (int i = 0; i < nicksClient.size(); i++)

            {

                if (msg.nick == nicksClient[i]) jugador = i;

            }



            auto it = clients.begin();

            auto it2 = nicksClient.begin();

            // 0 1 x

            while (it != clients.end() && !(*(*it).get() == *clientSd))

            {

                it++;

                it2++;

            }



            if (it == clients.end()) //El que sale, esta en sala de espera

            {

                // 0

                it = waiting.begin();

                it2 = nicksLobby.begin();

                while (it != waiting.end() && !(*(*it).get() == *clientSd))

                {

                    it++;

                    it2++;

                }



                std::cout << "borrando lista de espera\n";

                waiting.erase(it);

                nicksLobby.erase(it2);

            }

            else //El que sale esta en el juego

            {

                if (turno == jugador)

                {

                    int pos = jugador;

                    jugador++;

                    if (jugador >= clients.size())

                        jugador -= clients.size();



                    for (int i = 0; i < clients.size(); ++i) {

                        if (i == jugador)

                        {

                            msg.type = ChatMessage::MessageType::TUTURNO;

                            socket.send(msg, (*clients[i].get()));

                        }

                        else if (i != jugador && i != pos)

                        {

                            msg.type = ChatMessage::MessageType::SUTURNO;

                            msg.nick = nicksClient[jugador];

                            socket.send(msg, (*clients[i].get()));

                        }

                    }

                }

                std::cout << "borrando lista de clientes\n";

                clients.erase(it);

                nicksClient.erase(it2);



                int a = clients.size();

                msg.message = std::to_string(a);

                msg.type = ChatMessage::MessageType::LOGIN;

                socket.send(msg, (*clients[0].get()));

            }

        }

        else if (msg.type == ChatMessage::MessageType::TRY) {

            if (empezado)

            {

                int jugador;

                for (int i = 0; i < nicksClient.size(); i++)

                {

                    if (msg.nick == nicksClient[i]) jugador = i;

                }



                if (turno == jugador)

                {

                    std::cout << msg.nick << " ha intentado la letra " << msg.message << "\n";



                    //REPETIDO

                    for (std::list<std::string>::iterator it = usadas.begin();

                        it != usadas.end(); ++it)

                    {

                        if (*it == msg.message)

                        {

                            msg.type = ChatMessage::MessageType::ERROR;

                        }

                    }

                    if (msg.type != ChatMessage::MessageType::ERROR)

                    {

                        usadas.push_back(msg.message);

                        for (int i = 0; i < clients.size(); ++i) { //TRY

                            socket.send(msg, (*clients[i].get()));

                        }



                        //Comprobar si la letra es correcta

                        bool acierto = false;

                        for (int i = 0; i < palabras.length(); i++)

                        {

                            if (palabras[i] == msg.message[0])

                            {

                                ocultas[i] = palabras[i];

                                acierto = true;

                                letrasOcultas--;

                            }

                        }



                        //Termina la partida

                        if (letrasOcultas <= 0)

                        {

                            empezado = false;

                            for (int i = 0; i < clients.size(); ++i) {

                                if ((*(clients[i].get()) == *clientSd))

                                {

                                    msg.type = ChatMessage::MessageType::VICTORIA;

                                    msg.message = palabras;

                                    socket.send(msg, (*clients[i].get()));

                                }

                                else

                                {

                                    msg.type = ChatMessage::MessageType::DERROTA;

                                    msg.message = palabras;

                                    msg.nick = nicksClient[jugador];

                                    socket.send(msg, (*clients[i].get()));

                                }

                            }



                            msg.type = ChatMessage::LOBBY;



                            // std::cout << "ME HE LOGEADO";

                            socket.send(msg, socket);

                        }

                        else

                        {

                            msg.message = ocultas;

                            msg.type = ChatMessage::MessageType::OCULTA;

                            for (int i = 0; i < clients.size(); ++i) { //PALABRA OCULTA

                                socket.send(msg, (*clients[i].get()));

                            }



                            std::string a;

                            for (std::list<std::string>::iterator it = usadas.begin();

                                it != usadas.end(); ++it)

                            {

                                a += *it + " ";

                            }

                            msg.message = a;

                            msg.type = ChatMessage::MessageType::USADAS;

                            for (int i = 0; i < clients.size(); ++i) { //LISTA LETRAS

                                socket.send(msg, (*clients[i].get()));

                            }



                            //Cambiamos turno

                            if (!acierto)

                            {

                                jugador++;

                                if (jugador >= clients.size())

                                    jugador -= clients.size();



                                turno = jugador;

                                std::cout << "Turno: " << turno;



                                for (int i = 0; i < clients.size(); ++i) {

                                    if (i == jugador)

                                    {

                                        msg.type = ChatMessage::MessageType::TUTURNO;

                                        socket.send(msg, (*clients[i].get()));

                                    }

                                    else

                                    {

                                        msg.type = ChatMessage::MessageType::SUTURNO;

                                        msg.nick = nicksClient[jugador];

                                        socket.send(msg, (*clients[i].get()));

                                    }

                                }

                            }

                            else

                            {

                                for (int i = 0; i < clients.size(); ++i) {

                                    if ((*(clients[i].get()) == *clientSd))

                                    {

                                        msg.type = ChatMessage::MessageType::TUTURNO;

                                        socket.send(msg, (*clients[i].get()));

                                    }

                                    else

                                    {

                                        msg.type = ChatMessage::MessageType::SUTURNO;

                                        msg.nick = nicksClient[jugador];

                                        socket.send(msg, (*clients[i].get()));

                                    }

                                }

                            }

                        }

                    }

                    else

                    {

                        for (int i = 0; i < clients.size(); ++i) {

                            if ((*(clients[i].get()) == *clientSd))

                                socket.send(msg, (*clients[i].get()));

                        }

                    }

                }

                else

                {

                    msg.type = ChatMessage::MessageType::NOTURNO;

                    for (int i = 0; i < clients.size(); ++i) {

                        if ((*(clients[i].get()) == *clientSd))

                            socket.send(msg, (*clients[i].get()));

                    }

                }

            }

            else

            {

                msg.type = ChatMessage::MessageType::NOSTART;

                socket.send(msg, (*clients[0].get()));

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



    // std::cout << "ME HE LOGEADO";

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



        while (msg != "SI" && msg != "exit" && msg.length() > 1)

        {

            std::cout << "Escriba solo una letra\n";

            std::getline(std::cin, msg);

        }



        ChatMessage chatMsg(nick, msg);

        if (msg.length() == 1) chatMsg.type = ChatMessage::MessageType::TRY;

        else if (msg == "SI") chatMsg.type = ChatMessage::MessageType::START;

        else chatMsg.type = ChatMessage::MessageType::LOGOUT;

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



        switch (chatMsg.type)

        {

        case ChatMessage::MessageType::TRY:

            std::cout << "\x1B[2J\x1B[H"; //Limpia consola (Gracias lolito)

            std::cout << "Se ha intentado la letra " << chatMsg.message << "\n";

            break;

        case ChatMessage::MessageType::PALABRA:

            std::cout << "Palabra Oculta: " << chatMsg.message;

            break;

        case ChatMessage::MessageType::USADAS:

            std::cout << "Letras Usadas: " << chatMsg.message << "\n";

            break;

        case ChatMessage::MessageType::LOGIN:

            std::cout << "Sois " << chatMsg.message << " jugadores\n";

            std::cout << "Para empezar partida escriba SI\n";

            break;

        case ChatMessage::MessageType::ERROR:

            std::cout << "ERROR: letra repetida\n";

            break;

        case ChatMessage::MessageType::NOTURNO:

            std::cout << "No es tu turno, espera\n";

            break;

        case ChatMessage::MessageType::TUTURNO:

            std::cout << "Es tu turno\n";

            break;

        case ChatMessage::MessageType::SUTURNO:

            std::cout << "Es turno de " + chatMsg.nick + "\n";

            break;

        case ChatMessage::MessageType::OCULTA:

            std::cout << "Palabra Oculta: " << chatMsg.message;

            break;

        case ChatMessage::MessageType::VICTORIA:

            std::cout << "Has ganado!!!\nLa palabra era " << chatMsg.message << "\n\n";

            break;

        case ChatMessage::MessageType::DERROTA:

            std::cout << "Has perdido :(\nHa ganado " << chatMsg.nick << "\nLa palabra era " << chatMsg.message;

            break;

        case ChatMessage::MessageType::NOSTART:

            std::cout << "No se ha iniciado la partida\nPara iniciarla escriba SI\n";

            break;

        case ChatMessage::MessageType::WAIT:

            std::cout << "Ya hay una partida en curso\nPor favor espere hasta que esta termine\n";

            break;



        default:

            break;

        }

    }

}