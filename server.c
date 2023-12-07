#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>

#define PORT 8000
#define MAX_CLIENTS 10

// Estructura para almacenar la información de cada cliente
typedef struct {
    int socket;
    struct sockaddr_in address;
} ClientInfo;

ClientInfo clients[MAX_CLIENTS];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int numClients = 0;
int serverSocket;

void createSocket(int *sock){
    if ((*sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Error al crear el socket");
        exit(0);
    }
}

void configServer(int socket, struct sockaddr_in *conf){
    conf->sin_family = AF_INET;		   		    // Dominio
    conf->sin_addr.s_addr = htonl(INADDR_ANY);	// Enlazar con cualquier dirección local
    conf->sin_port = htons(PORT);				// Puerto donde escucha

    if ((bind(socket, (struct sockaddr *) conf, sizeof(*conf))) < 0) { // bind!
        printf("Error de enlace\n");
        exit(0);
    }
}

void listenClients(int serverSocket){
    if (listen(serverSocket, MAX_CLIENTS) < 0) {
        perror("Error al escuchar conexiones");
        close(serverSocket);
        exit(0);
    }
}

void *handleClient(void *arg) {
    ClientInfo *clientInfo = (ClientInfo *)arg;
    int clientSocket = clientInfo->socket;

    // Implementa la lógica para manejar la comunicación con este cliente
    // (puedes usar recv() y send() aquí)

    // Ejemplo: un bucle de lectura que retransmite mensajes a todos los clientes
    char buffer[1024];
    while (1) {
        int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesRead <= 0) {
            // El cliente ha cerrado la conexión
            pthread_mutex_lock(&mutex);
            // Eliminar al cliente del array
            for (int i = 0; i < numClients; i++) {
                if (clients[i].socket == clientSocket) {
                    for (int j = i; j < numClients - 1; j++) {
                        clients[j] = clients[j + 1];
                    }
                    numClients--;
                    break;
                }
            }
            pthread_mutex_unlock(&mutex);

            close(clientSocket);
            free(clientInfo);
            pthread_exit(NULL);
        }

        // Retransmitir el mensaje a todos los clientes
        pthread_mutex_lock(&mutex);
        for (int i = 0; i < numClients; i++) {
            send(clients[i].socket, buffer, bytesRead, 0);
        }
        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}


void *waitAndSleep(void *arg) {
    int T1 = *(int *)arg;

    
    while (1) 
    {

        printf("Esperando nuevas conexiones...\n");

        struct sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);

        int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientLen);

        if (clientSocket == -1) {
            perror("Error al aceptar la conexión del cliente");
            continue;
        }

        // Configurar información del cliente
        ClientInfo *clientInfo = (ClientInfo *)malloc(sizeof(ClientInfo));
        clientInfo->socket = clientSocket;
        clientInfo->address = clientAddr;

        // Agregar cliente al array
        pthread_mutex_lock(&mutex);
        if (numClients < MAX_CLIENTS) {
            clients[numClients] = *clientInfo;
            numClients++;
        } else {
            printf("Número máximo de clientes alcanzado. Rechazando nueva conexión.\n");
            close(clientSocket);
            free(clientInfo);
            continue;
        }
        pthread_mutex_unlock(&mutex);

        // Crear hilo para manejar al cliente
        pthread_t clientThread;
        if (pthread_create(&clientThread, NULL, handleClient, (void *)clientInfo) != 0) {
            perror("Error al crear el hilo del cliente");
            close(clientSocket);
            free(clientInfo);
            continue;
        }

        // Liberar el hilo hijo
        pthread_detach(clientThread);

        printf("Nuevo cliente conectado: %s:%d\n",
               inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
    }

    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Uso: %s <T1> <T2>\n", argv[0]);
        return 1;
    }

    int T1 = atoi(argv[1]);
    int T2 = atoi(argv[2]);

    //int serverSocket;
    struct sockaddr_in serverAddr;

    // Configurar el socket del servidor
    createSocket(&serverSocket);
    
    // Configurar la dirección del servidor
    configServer(serverSocket, &serverAddr);

    // Escuchar conexiones entrantes
    listenClients(serverSocket);

    printf("Servidor de chat en espera de conexiones...\n");

    // Crear hilo para esperar y dormir
    pthread_t waitAndSleepThread;
    if (pthread_create(&waitAndSleepThread, NULL, waitAndSleep, (void *)&T1) != 0) {
        perror("Error al crear el hilo de espera y dormir");
        close(serverSocket);
        exit(1);
    }

    // Esperar a que el hilo de espera y dormir termine
    pthread_join(waitAndSleepThread, NULL);

    // Cerrar el socket del servidor
    close(serverSocket);

    return 0;
}
