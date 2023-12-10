#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define BUFFERSIZE 1024

int flag = 0;
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

int connectToServer(int clientSocket, char *SERVER_IP, short PORT)
{
    struct sockaddr_in server_addr;

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_addr.sin_port = htons(PORT);
    
    printf("%d ---- %d ---- %s", clientSocket, PORT, SERVER_IP);

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

    while (!flag) {
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

    char nombreClientes[10];
    char *IP = argv[1];
    char PORT[4];

    strcpy(PORT, argv[2]);
    strcpy(nombreClientes, argv[3]);

    short _port = (short)atoi(PORT);

    clientSocket = createSocket();

    connectToServer(clientSocket, IP, _port);    

    printf("Conectado al servidor de chat. Ingresa 'BYE' para salir.\n");

    pthread_t receiveThread;
    if (pthread_create(&receiveThread, NULL, receiveMessages, (void *)&clientSocket) != 0) {
        printf("Error al crear el hilo de recepción de mensajes");
        close(clientSocket);
        exit(1);
    }

    char message[BUFFERSIZE] = {0};
    char messageCom[BUFFERSIZE] = {0};
    
    
    while (1) 
    {
        
        fgets(message, sizeof(message), stdin);
        printf("\033[1A");
        printf("\033[K");

        strcat(messageCom, nombreClientes);
        strcat(messageCom, " Dijo -> ");
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
    
    flag = 1;

    pthread_join(receiveThread, NULL);

    close(clientSocket);

    return 0;
}
