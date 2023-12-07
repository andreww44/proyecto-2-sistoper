#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

////
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

////
#include <pthread.h>

#define port 8000
#define BUFFERSIZE 1024

void crearSocket(int *sock){
    if ((*sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {        
        printf("Error Creación de Socket\n");
        exit(0);
    }
}

void configurarServidor(int socket, struct sockaddr_in *conf){
    conf->sin_family = AF_INET;		   		    // Dominio
    conf->sin_addr.s_addr = htonl(INADDR_ANY);	// Enlazar con cualquier dirección local
    conf->sin_port = htons(port);				// Puerto donde escucha

    if ((bind(socket, (struct sockaddr *) conf, sizeof(*conf))) < 0) { // bind!
        printf("Error de enlace\n");
        exit(0);
    }
}

void escucharClientes(int sock, int n){
    if (listen(sock, n) < 0) {		
        printf("Error listening\n");
        exit(0);
    }
}

void aceptarConexion(int *sockNuevo, int sock, struct sockaddr_in *conf ){
    socklen_t  tamannoConf = sizeof(*conf);

    if ((*sockNuevo = accept(sock, (struct sockaddr *) conf, &tamannoConf)) < 0) {
        printf("Error accepting\n");
        exit(0);
    }
}

void reemplazarCaracter(char *buffer, char caracter1, char caracter2){
    for(int j=0; j < strlen(buffer); j++){
        if (buffer[j] == caracter1){
            buffer[j] = caracter2;
        }
    }              
}

void invertirPalabra(char *palabra){  
    int largo = strlen(palabra);
    char palabra2[largo];
    
    for (int i = 0; i < largo; i++)  {  
        palabra2[i] = palabra[largo - i - 1];  
    }  
    
    palabra2[largo] = '\0';
    strcpy(palabra, palabra2);
}  

void *Servidor(void *arg){
   int *sockCliente = (int *)arg;
   /////
   int primerMensaje = 1;
   while(1){
        char nombre[BUFFERSIZE]={0};
        char buffer[BUFFERSIZE]={0};
        char buffer2[BUFFERSIZE]={0};
        int valread;
        
        valread = read(*sockCliente, buffer, BUFFERSIZE);

        if (primerMensaje){
            strcpy(buffer2, "Hola ");
            strcat(buffer2, buffer);
            strcat(buffer2, "\n");
            strcpy(nombre, buffer);

            printf("%s", buffer2);
            send(*sockCliente, buffer2, strlen(buffer2), 0);
            primerMensaje = 0;
        }else{
            if (strcmp(buffer, "BYE\n")==0){
                strcpy(buffer2, buffer);
                strcat(buffer2, nombre);
                send(*sockCliente, buffer2, strlen(buffer2), 0);

                printf("%s", buffer2);                    
                break;
            }

            printf("%s", buffer);
            
            ////Separar en palabras el mensaje recibido
            char buffer2[1024]={0}, *palabra;
            palabra = strtok (buffer, " ");
            int primeraPalabra = 1;

            while (palabra != NULL){
                //Invertir palabra
                if (palabra[strlen(palabra)-1]=='\n'){
                    palabra[strlen(palabra)-1]='\0';
                }
                    
                invertirPalabra(palabra);     
                
                if (primeraPalabra){
                    strcpy(buffer2, palabra);
                    primeraPalabra = 0;
                }else{
                    strcat(buffer2, " "); 
                    strcat(buffer2, palabra);
                }

                palabra = strtok (NULL, " ");
            }

            send(*sockCliente, buffer2, strlen(buffer2), 0);
        } 
   }  
}

int main(int argc, char *argv[]){
    if (argc < 2)
        return 0;
    
    int nClientes = strtol(argv[1], NULL, 10);
    
    if (nClientes < 1)
    
       return 0;

    ///1. Configuración del Socket
    int sockServidor;
    crearSocket(&sockServidor);

    ///2. Vinculación
    struct sockaddr_in confServidor;
    configurarServidor(sockServidor, &confServidor);

    ///3. Escuchando conexiones entrantes
    escucharClientes(sockServidor, nClientes);

    ///4. Aceptar conexión
    struct sockaddr_in confCliente;
    int sockCliente[nClientes];

    for(int i=0; i < nClientes; i++){
        aceptarConexion(&sockCliente[i], sockServidor, &confCliente);

        pthread_t hebra;
        pthread_create(&hebra, NULL, Servidor, (void *)&sockCliente[i]);       
    }

    return 0;
}