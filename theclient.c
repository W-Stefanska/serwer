#include <stdio.h>
#include <stdlib.h> // exit()
#include <string.h> // strlen()
#include <stdbool.h>

#include <sys/socket.h> // socket()
#include <netinet/in.h> // struct sockaddr_in
#include <arpa/inet.h> // inet_pton()
#include <netdb.h> // gethostbyname()

#define MAX_MSG_LEN 256

void pad_name(char *name) // fukcja dodaje padding do name
{
    int len = strlen(name);
    for (int i = len; i < 8; i++) 
    {
        name[i] = '_';
    }
    name[8] = '\0';
}

void fill_buffer(char *packet)
{
    int len = strlen(packet);
    for (int i = (len-1); i < 1000; i++) 
    {
        packet[i] = '0';
    }
    packet[999] = '#';
}

int main(int argc, char *argv[])
{
    if (argc != 4) {
        fprintf(stderr, "Wpisz: %s <nazwa klienta> <adres serwera> <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char name[9];
    strncpy(name, argv[1], 8);
    name[8] = '\0'; 
    
    if( strlen(argv[1]) > 8 )
    {
        fprintf(stderr, "Nazwa klienta nie powinna przekraczac 8 znakow\n");
        exit(EXIT_FAILURE);
    }

    if( strlen(name) < 8 ) 
    {
        pad_name(name);
        fprintf(stderr, "\nTest: %s\nZnak:%s\n",name,name[8]);
        fprintf(stderr, "Zmieniona nazwa: %s\n", name);
    }



    int sockfd;
    struct sockaddr_in serv_addr;
    char buffer[MAX_MSG_LEN];
    char *opis_bledu = "c";
    int bitent[15];
    char *adres = argv[2]; // adres serwera
    int port = atoi(argv[3]); // numer portu

    struct sockaddr_in serwer =
    {
        .sin_family = AF_INET,
        .sin_port = htons(port)
    };

    if( inet_pton( AF_INET, adres, &serwer.sin_addr ) <= 0 )
    {
        perror( "inet_pton() ERROR" );
        exit( 1 );
    }
   
    const int s = socket( serwer.sin_family, SOCK_STREAM, 0 ); // 3 arg - 0 nieblokujace, zmienic w razie potrzeby
    if( s < 0 )
    {
        perror( "socket() ERROR" );
        exit( 2 );
    }

    if( connect( s, (struct sockaddr * ) &serwer, sizeof( serwer ) ) < 0 )
    {
        perror( "connect() ERROR" );
        close(s);
        exit( 3 );
    }
    


    char packetA1[1000];
    char packetA2[1000];
    char packetA3[1000];

    // Przygotowanie pakietow
    snprintf(packetA1, sizeof(packetA1), "@%s0!N:0#", name);
    fill_buffer(packetA1);
    snprintf(packetA2, sizeof(packetA2), "@%s0!R:%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d0#", name, bitent[0],bitent[1],bitent[2],bitent[3],bitent[4],bitent[5],bitent[6],bitent[7],bitent[8],bitent[9],bitent[10],bitent[11],bitent[12],bitent[13],bitent[14],bitent[15]);

    snprintf(packetA3, sizeof(packetA3), "@%s0!E:%s0#", name, opis_bledu);

    // Wysyłanie pakietu A1
    if (send(s, packetA1, strlen(packetA1), 0) < 0) {
        perror("send() failed");
        close(s);
        exit(EXIT_FAILURE);
    }

    printf("Pakiet A1 wysłany: %.13s\n", packetA1);
    printf("NAzwa: %s \n DLugosc: %ld", name, strlen(name));
    while( recv( s, buffer, sizeof( buffer ), 0 ) > 0 )
    {
        puts( buffer );
    }
   
    shutdown( s, SHUT_RDWR );
    close(s);
   
    return 0;
}
