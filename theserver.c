#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>

#define MAX_MSG_LEN 4096
#define MAX_CONNECTION 1000
#define MAX_DATA 100000

struct CALCDATA
{
    uint32_t data[MAX_DATA];
};

struct timevalue
{
    int tv_sec; // ilosc sekund
    int tv_usec; // ilosc mikrosekund
};

int create_data(int idx, struct CALCDATA *cdata)
{
    if (cdata != NULL)
    {
        uint32_t i;
        uint32_t *value;
        uint32_t v;
        value = &cdata->data[0];
        for (i = 0; i < MAX_DATA; i++)
        {
            v = (uint32_t)rand() ^ (uint32_t)rand();
            printf("Creating value #%d v=%u addr=%lu \r", i, v, (unsigned long)value);
            *value = v & 0x0000FFFF;
            value++;
        }
        return (1);
    }
    return (0);
}

int main(int argc, char *argv[]) // tu ma byc tylko nr portu nasluchujacego tj. ./server <liczba>
{
    if (argc != 2) // sprawdza czy dobrze wywolano program
    {
        fprintf(stderr, "Wpisz: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    else
    {
        fprintf(stderr, "Nasluchiwanie na porcie %s\n", argv[1]);
    }

    int port = atoi(argv[1]);
    int server_socket, new_socket, client_socket[MAX_CONNECTION];
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    fd_set readfds;
    int max_sd, sd, activity, valread;
    char buffer[MAX_MSG_LEN];

    struct sockaddr_in serwer =
    {
        .sin_family = AF_INET,
        .sin_port = htons(port)
    };

    if( inet_pton( AF_INET, "127.0.0.1", &serwer.sin_addr ) <= 0 )
    {
        perror( "inet_pton() ERROR" );
        exit( 1 );
    }
    
    // socket klienta
    for (int i = 0; i < MAX_CONNECTION; i++) {
        client_socket[i] = 0;
    }

    // socket serwera
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if( server_socket < 0 )
    {
        perror( "socket() ERROR" );
        exit( 2 );
    }
   
    socklen_t len = sizeof( serwer );
    if( bind( server_socket, (struct sockaddr * ) &serwer, len ) < 0 )
    {
        perror( "bind() ERROR" );
        close(server_socket);
        exit( 3 );
    }
   
    if( listen( server_socket, MAX_CONNECTION ) < 0 )
    {
        perror( "listen() ERROR" );
        close(server_socket);
        exit( 4 );
    }
   
    while( 1 )
    {
        // Clear the socket set
        FD_ZERO(&readfds);

        // Add server socket to set
        FD_SET(server_socket, &readfds);
        max_sd = server_socket;
        printf( "Waiting for connection...\n" );
       
        struct sockaddr_in client = { };
       
        const int clientSocket = accept( server_socket, (struct sockaddr * ) &client, &len );
        if( clientSocket < 0 )
        {
            perror( "accept() ERROR" );
            continue;
        }
       
        char buffer[ MAX_MSG_LEN ] = { };
       
        if( recv( clientSocket, buffer, sizeof( buffer ), 0 ) <= 0 )
        {
            perror( "recv() ERROR" );
            close(clientSocket);
            continue;
        }
        printf( "|Message from client|: %s \n", buffer );
       
        strcpy( buffer, "Message from server: Odpowiedz" );
        if( send( clientSocket, buffer, strlen( buffer ), 0 ) <= 0 )
        {
            perror( "send() ERROR" );
            close(clientSocket);
            continue;
        }
       
        shutdown( clientSocket, SHUT_RDWR );
        close(clientSocket);
    }
   
    shutdown( server_socket, SHUT_RDWR );
    close(server_socket);
    return 0;
}
