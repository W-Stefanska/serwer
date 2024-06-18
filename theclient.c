#include <stdio.h>
#include <stdlib.h> // exit()
#include <string.h> // strlen()
#include <stdbool.h>

#include <sys/socket.h> // socket()
#include <netinet/in.h> // struct sockaddr_in
#include <arpa/inet.h> // inet_pton()
#include <netdb.h> // gethostbyname()

#define MAX_MSG_LEN 1000
#define MAX_DATA 100000

void pad_name(char *name) // fukcja dodaje padding do name
{
    int len = strlen(name);
    for (int i = len; i < 8; i++) 
    {
        name[i] = '_';
    }
    //name[8] = '\0';
}

void fill_buffer(char *packet)
{
    int len = strlen(packet);
    for (int i = (len); i < 1000; i++) 
    {
        packet[i] = '0';
    }
    packet[999] = '#';
}

void process_data(uint32_t *data, size_t len, uint32_t *bitcnt) {
    for (size_t i = 0; i < len; i++) {
        if ((data[i] & 0xFFFF0000) == 0) {
            for (int j = 0; j < 16; j++) {
                if (data[i] & (1 << j)) {
                    bitcnt[j]++;
                }
            }
        }
    }
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
    }

    int sockfd;
    struct sockaddr_in serv_addr;
    char buffer[1000]; // nie ruszac
    char *opis_bledu = "c";
    uint32_t bitcnt[16] = {0};
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
    snprintf(packetA1, sizeof(packetA1), "@%s0!N:", name);
    fill_buffer(packetA1);
    snprintf(packetA2, sizeof(packetA2), "@%s0!R:%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", name, bitcnt[0],bitcnt[1],bitcnt[2],bitcnt[3],bitcnt[4],bitcnt[5],bitcnt[6],bitcnt[7],bitcnt[8],bitcnt[9],bitcnt[10],bitcnt[11],bitcnt[12],bitcnt[13],bitcnt[14],bitcnt[15]);

    snprintf(packetA3, sizeof(packetA3), "@%s0!E:%s", name, opis_bledu);

    // Wysyłanie pakietu A1
    if (send(s, packetA1, sizeof(packetA1), 0) < 0) {
        perror("send() failed");
        close(s);
        exit(EXIT_FAILURE);
    }

    printf("Pakiet A1 wysłany: %.13s\n", packetA1);
    
    
    int received_data = 0;
    while (received_data < MAX_DATA * sizeof(uint32_t)) {
        int bytes_received = recv(s, buffer, sizeof(buffer), 0);
        if (bytes_received < 0) {
            perror("recv() ERROR");
            break;
        }
        received_data += bytes_received;

        int values_received = bytes_received / sizeof(uint32_t);
        process_data((uint32_t *)buffer, values_received, bitcnt);
    }
    close(s);

    printf("Histogram bitow:\n");
    for (int i = 0; i < 16; i++) 
    {
        printf("Bit %d: %u\n", i, bitcnt[i]);
    }

    shutdown( s, SHUT_RDWR );
    close(s);
    
    return 0;
}
