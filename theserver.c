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

void fill_buffer(char *packet)
{
    int len = strlen(packet);
    for (int i = (len); i < 1000; i++) 
    {
        packet[i] = '0';
    }
    packet[999] = '#';
}

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

    int port = atoi(argv[1]);
    int server_socket, data_socket, client_socket[10];
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    // tu jest socket serwera
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("socket() ERROR");
        exit(EXIT_FAILURE);
    }

    // struktura
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);

    if( inet_pton( AF_INET, "127.0.0.1", &server_addr.sin_addr ) <= 0 )
    {
        perror( "inet_pton() ERROR" );
        exit( 1 );
    }
    
    // bindowanie
    socklen_t len = sizeof( server_addr );
    if (bind(server_socket, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        perror("bind() ERROR");
        close(server_socket);
        exit(EXIT_FAILURE);
    }
   
    // nasłuchiwanie
    if (listen(server_socket, MAX_CONNECTION) < 0) {
        perror("listen() ERROR");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Nasluchiwanie na porcie %d...\n", port);
   
    // tutaj mam 10 klientow
    for (int i = 0; i < 10; i++) 
    {
        client_socket[i] = 0;
    }

    // socket nr 2
    data_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (data_socket < 0) {
        perror("socket() ERROR");
        exit(EXIT_FAILURE);
    }

    // DO ZMIENIENIA
    struct sockaddr_in data_addr;
    memset(&data_addr, 0, sizeof(data_addr));
    data_addr.sin_family = AF_INET;
    data_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    data_addr.sin_port = htons(port + 1);

    // bind vol 2
    if (bind(data_socket, (struct sockaddr *) &data_addr, sizeof(data_addr)) < 0) 
    {
        perror("bind() ERROR");
        close(data_socket);
        exit(EXIT_FAILURE);
    }

    // tu listen vol 2 
    if (listen(data_socket, MAX_CONNECTION) < 0) 
    {
        perror("listen() ERROR");
        close(data_socket);
        exit(EXIT_FAILURE);
    }

    printf("JESLI CZZYTASZ TE WIAD TO PORT 2 DZIALA %d <--- a to jego nr\n", port + 1);

    fd_set readfds;
    int max_sd, activity;
    
    
    while( 1 )
    {
        char buffer[1000]; // nie ruszac
       
        FD_ZERO(&readfds);
        FD_SET(server_socket, &readfds);
        max_sd = server_socket;
        printf( "Oczekiwanie na polaczenie...\n" );
       
        // dodawanie deskryptorow
        for (int i = 0; i < 10; i++) 
        {
            int sd = client_socket[i];
            
            if (sd > 0)
                FD_SET(sd, &readfds);
            
            if (sd > max_sd)
                max_sd = sd;
        }


        struct sockaddr_in client = { };
       
        const int clientSocket = accept( server_socket, (struct sockaddr * ) &client, &len );
        if( clientSocket < 0 )
        {
            perror( "accept() ERROR" );
            continue;
        }
       
        if( recv( clientSocket, buffer, sizeof( buffer ), 0 ) <= 0 )
        {
            perror( "recv() ERROR" );
            close(clientSocket);
            continue;
        }
        printf( "|Message from client|: %s\n", buffer );

        char packetB1[1000];
        snprintf(packetB1, sizeof(packetB1), "@000000000!N:%s", "5005");
        fill_buffer(packetB1);
    
        socklen_t len = sizeof( server_addr );
    
        strcpy( buffer, "Message from server: Odpowiedz" );
        if( send( clientSocket, packetB1, strlen( packetB1 ), 0 ) <= 0 )
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
