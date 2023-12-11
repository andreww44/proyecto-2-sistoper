#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
//#include 


//Define el tamaño de los mensajes ingresados
#define BUFFER_S 1000

//Bandera que indica cuando cerrar el hilo encargado de recibir mensajes
int flag = 0;

//Funcion para crear el socket del usuario
int createSocket()
{
    //Define un tipo de dato entero donde se guarda el socket del cliente
    int clientSocket;
    //Se inicia el socket el primer parametro indica el dominio del socket en este caso IPV4, Luego se pasa el tipo de socket Sock_strean para TCP
    //por ultimo el protocolo
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    
    //Si clientSocket es menos que cero indica que hubo un error en su creacion
    if (clientSocket < 0) {
        printf("Error al crear el socket");
        exit(0);
    }
    return clientSocket;
}

//Funcion para conectar al server
int connectToServer(int clientSocket, char *SERVER_IP, short PORT)
{
    //Estructura de datos que contiene la direccion, el protocolo y el puerto donde se conectara el cliente
    struct sockaddr_in server_addr;

    //Se asigna el protocolo 
    server_addr.sin_family = AF_INET;
    //Se asigna la direccion IP
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    //Se asigna el Puerto
    server_addr.sin_port = htons(PORT);

    //Connect establece la conexion con el servidor
    if (connect(clientSocket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        printf("Error al conectar al servidor");
        close(clientSocket);
        exit(0);
    }
}

//Hebra que se mantiene escuchando para ver si llegan mensajes por parte del servidor

void *receiveMessages(void *arg) {
    
    //Recibe por parametro el argumento enviado donde se crea la hebra y se castea a entero ya que necesitamos el descriptor del socket
    int clientSocket = *(int *)arg;
    
    //String para guardar el mensaje que venga desde el sevidor
    char buffer[BUFFER_S];

    //Mientras la bandera sea distinto de 0 la hebra seguira trabajando
    while (!flag) {

        //bytes indica la cantidad de bytes recibidos si es mayor que 0 recibio bien, si es -1 hay un error y 0 se cerro por motivos externos
        int bytes = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytes <= 0) {
            printf("El servidor ha cerrado la conexión.\n");
            close(clientSocket);
            exit(0);
        }

        //Imprime por pantalla el mensaje recibido 
        printf("%s", buffer);
        //Se libera el buffer
        memset(buffer, 0, sizeof(buffer));
    }

    return NULL;
}

int main(int argc, char *argv[]) {

    if(argc < 3){
        printf("Por favor ingrese los argumentos\n");
        return 0;
    }

    //Descriptor del socket
    int clientSocket;

    //String que guarda el nombre del cliente
    char nombreClientes[10];
    //Direccion que guarda la IP
    char *IP = argv[1];
    //String que guarda el puerto
    char PORT[4];

    //Se copia el tercer argumento en el puerto
    strcpy(PORT, argv[2]);
    //Se copia el cuarto argumento en el nombre
    strcpy(nombreClientes, argv[3]);
    //Se castea el puerto a short despues de transformarlo de string a int
    short _port = (short)atoi(PORT);

    //Funcion que crea el socket
    clientSocket = createSocket();
    //Se conecta al server
    connectToServer(clientSocket, IP, _port);    

    printf("Conectado al servidor de chat. Ingresa '/salir' para salir.\n");

    //Se arroja la hebra de recibir mensajes
    pthread_t receiveThread;
    pthread_create(&receiveThread, NULL, receiveMessages, (void *)&clientSocket);

    //Gestion de mensajes
    //Mensaje de saludo
    char messageHi[BUFFER_S] = {0};
    //Lectura por pantalla
    char message[BUFFER_S] = {0};
    //Mensaje a enviar
    char messageCom[BUFFER_S] = {0};
    //Mensaje de despedida
    char messageBye[BUFFER_S] = {0};

    strcat(messageHi, nombreClientes);
    strcat(messageHi, " se ha unido a la sala de chat \n");

    strcat(messageBye, nombreClientes);
    strcat(messageBye, " Ha abandonado la sala \n");

    //Funcion que envia el mensaje
    send(clientSocket, messageHi, strlen(messageHi), 0);
    
    while (1) 
    {
        //Lee por pantalla y limpia el mensaje ingresado
        fgets(message, sizeof(message), stdin);
        printf("\033[1A");
        printf("\033[K");

        strcat(messageCom, nombreClientes);
        strcat(messageCom, " Dijo -> ");
        strcat(messageCom, message);

        if (strcmp(message, "/salir\n") == 0) {
            //Sale si el mensaje ingresado es /salir
            break;
        } 
        else 
        {
            //Envia el mensaje 
            send(clientSocket, messageCom, strlen(messageCom), 0);
            //Limpia el buffer
            memset(messageCom, 0, sizeof(messageCom));
        }
    }

    send(clientSocket, messageBye, strlen(messageBye), 0);
    
    //indica que termina de recibir mensajes
    flag = 1;
    
    sleep(1);
    //Cierra el hilo 
    pthread_join(receiveThread, NULL);

    close(clientSocket);

    return 0;
}
