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

            nicksClient.push_back(msg.nick);

            if (clients.size() == 2)

            {

                char buff[15];



                // char buff2 [1024];



                FILE* f = fopen("diccionario.dat", "r");

                srand(time(NULL));

                int a = rand() % 60;

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

                // std::cout << "TAM: " + palabra.size();





                std::string oculta = palabra;



                for (int i = 0; i < (tam - 1); i++)

                {

                    oculta[i] = '_';

                }



                ocultas = oculta;



                // std::cout << "Oculta: " << oculta << "\n";

                palabras = palabra;

                std::cout << palabras;



                // std::cout << "B";

                for (int i = 0; i < oculta.size(); i++)

                {

                    std::cout << oculta[i];

                }



                for (int i = 0; i < clients.size(); ++i) {

                    // std::cout << "A";

                    socket.send(msg, (*clients[i].get()));

                }



                for (int i = 0; i < clients.size(); ++i) {

                    msg.message = ocultas;

                    msg.type = ChatMessage::MessageType::OCULTA;

                    // std::cout << "A";

                    socket.send(msg, (*clients[i].get()));

                }



                //Al primer juagdor

                msg.type = ChatMessage::MessageType::TUTURNO;

                socket.send(msg, (*clients[0].get()));



                //Al segundo jugador

                msg.type = ChatMessage::MessageType::SUTURNO;

                msg.nick = nicksClient[0];

                socket.send(msg, (*clients[1].get()));



                std::cout << "\n";



                // std::cout << "oculta tAM: " << sizeof(oculta)<<"\n";



                fclose(f);



            }



        }



        else if (msg.type == ChatMessage::MessageType::LOGOUT) {



            std::cout << msg.nick << " se ha desconectado\n";



            auto it = clients.begin();



            while (it != clients.end() && !(*(*it).get() == *clientSd)) it++;



            clients.erase(it);



        }



        else if (msg.type == ChatMessage::MessageType::MESSAGE) {



            std::cout << msg.nick << " ha enviado un mensaje\n";



            std::cout << msg.nick << " ha escrito " << msg.message << "\n";



            for (int i = 0; i < clients.size(); ++i) {



                if (!(*(clients[i].get()) == *clientSd))



                    socket.send(msg, (*clients[i].get()));



            }



        }



        else if (msg.type == ChatMessage::MessageType::TRY) {

            int jugador;

            if (msg.nick == nicksClient[0]) jugador = 0;

            else jugador = 1;



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

                    turno = !turno;



                    for (int i = 0; i < clients.size(); ++i) {

                        if (!(*(clients[i].get()) == *clientSd))

                        {

                            msg.type = ChatMessage::MessageType::TUTURNO;

                            socket.send(msg, (*clients[i].get()));

                        }

                        else

                        {

                            msg.type = ChatMessage::MessageType::SUTURNO;

                            if (jugador == 0) msg.nick = nicksClient[1];

                            else msg.nick = nicksClient[0];

                            socket.send(msg, (*clients[i].get()));

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

    // std::cout << "ME HE LOGEADO2";

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



        while (msg != "exit" && msg.length() > 1)

        {

            std::cout << "Escriba solo una letra\n";

            std::getline(std::cin, msg);

        }



        ChatMessage chatMsg(nick, msg);

        if (msg.length() == 1) chatMsg.type = ChatMessage::MessageType::TRY;

        else chatMsg.type = ChatMessage::MessageType::LOGOUT;

        socket.send(chatMsg, socket);



    }



}







void ChatClient::net_thread()



{



    while (true)



    {

        // std::cout << "RUBICOCK";

        //Recibir Mensajes de red

        //Mostrar en pantalla el mensaje de la forma "nick: mensaje"

        ChatMessage chatMsg;

        socket.recv(chatMsg);



        if (chatMsg.type == ChatMessage::MessageType::TRY)

            std::cout << "\n\nSe ha intentado la letra " << chatMsg.message << "\n";

        else if (chatMsg.type == ChatMessage::MessageType::PALABRA)

            std::cout << "Palabra Oculta: " << chatMsg.message;

        else if (chatMsg.type == ChatMessage::MessageType::USADAS)

            std::cout << "Letras Usadas: " << chatMsg.message << "\n";

        else if (chatMsg.type == ChatMessage::MessageType::LOGIN)

            std::cout << "Empieza la partida\n";

        else if (chatMsg.type == ChatMessage::MessageType::ERROR)

            std::cout << "ERROR: letra repetida\n";

        else if (chatMsg.type == ChatMessage::MessageType::NOTURNO)

            std::cout << "No es tu turno, espera\n";

        else if (chatMsg.type == ChatMessage::MessageType::TUTURNO)

            std::cout << "Es tu turno\n";

        else if (chatMsg.type == ChatMessage::MessageType::SUTURNO)

            std::cout << "Es turno de " + chatMsg.nick + "\n";

        else if (chatMsg.type == ChatMessage::MessageType::OCULTA)

            std::cout << "Palabra Oculta: " << chatMsg.message;



        // std::cout << "TERMINO MSG";







    }



}



