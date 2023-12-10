#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define SERVER_IP "127.0.0.1"
#define PORT 8000
#define BUFFERSIZE 1024

int createSocket()
{
    int clientSocket;
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) 
    {
        printf("Error al crear el socket");
        exit(0);
    }
    return clientSocket;
}

int connectToServer(int clientSocket)
{
    struct sockaddr_in server_addr;

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_addr.sin_port = htons(PORT);

    if (connect(clientSocket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) 
    {
        printf("Error al conectar al servidor");
        close(clientSocket);
        exit(0);
    }
}

void *receiveMessages(void *arg) {
    int clientSocket = *(int *)arg;
    char buffer[BUFFERSIZE];

    while (1) {
        int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesRead <= 0) {
            printf("El servidor ha cerrado la conexión.\n");
            close(clientSocket);
            exit(0);
        }

        printf("%s", buffer);
        memset(buffer, 0, sizeof(buffer));
    }

    return NULL;
}

int main(int argc, char *argv[]) {

    int clientSocket;
    //struct sockaddr_in serverAddr;

    char nombreClientes[10];
    strcpy(nombreClientes, argv[1]);

    //Configurar cliente
    clientSocket = createSocket();

    connectToServer(clientSocket);    

    //send(clientSocket, nombreClientes, strlen(nombreClientes), 0);

    printf("Conectado al servidor de chat. Ingresa 'BYE' para salir.\n");

    // Crear hilo para recibir mensajes
    pthread_t receiveThread;
    if (pthread_create(&receiveThread, NULL, receiveMessages, (void *)&clientSocket) != 0) {
        perror("Error al crear el hilo de recepción de mensajes");
        close(clientSocket);
        exit(1);
    }

    // Enviar mensajes al servidor
    char message[BUFFERSIZE];
    char messageCom[BUFFERSIZE] = {0};
    
    
    while (1) 
    {
        
        fgets(message, sizeof(message), stdin);

        strcat(messageCom, nombreClientes);
        strcat(messageCom, " Dijo ->");
        strcat(messageCom, message);

        if (strcmp(message, "BYE\n") == 0) {
            send(clientSocket, message, strlen(message), 0);
            break;
        } 
        else 
        {
            send(clientSocket, messageCom, strlen(messageCom), 0);
            memset(messageCom, 0, sizeof(messageCom));
        }
        
        
    }

    // Esperar a que el hilo de recepción termine
    pthread_join(receiveThread, NULL);

    // Cerrar el socket del cliente
    close(clientSocket);

    return 0;
}
