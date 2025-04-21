#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
// #include <netinet/in.h>
#define IPV4_BINARY_LEN 4
#define IPV4_ADDR_STR_LEN 16
#define PORT_BINARY_LEN 2
#define NO_INDEX_LEFT -10

#define MAX_PUBKEYS 9
#define NO_PUBKEY_FOUND -11

typedef struct forwardtab {
   unsigned long srcaddress;
   unsigned short srcport;
   unsigned long dstaddress;
   unsigned short dstport;
   unsigned short tunnelsport; 
} forwardtab_t;

typedef struct pubkey {
    char IP[IPV4_ADDR_STR_LEN];
    unsigned long long int pub;
} pubkey_t;

// pubkey_t * all_pubkeys;

pubkey_t * initiate_pubkeys(pubkey_t * pubkeys) {
    pubkeys = malloc(sizeof(pubkey_t) * 9);
    for(int i = 0; i<9; i++) {
        sprintf(pubkeys[i].IP, "128.10.112.13%d", i+1);
        pubkeys[i].pub = 0xAAAAAAAAAAAAAAAA; // Equivalent to 101010...10 (64 bits of alternating 1 and 0)
    }
    return pubkeys;
}

int search_peer_IP(pubkey_t * pubkeys, char * key) {
    for(int i = 0; i<MAX_PUBKEYS; i++) {
        if(strcmp(pubkeys[i].IP, key) == 0) {
            return i;
        }
    }
    return NO_PUBKEY_FOUND;
}

void binary_to_ip(const char *binary_ip, char *ip_str) {
    unsigned int ip_addr;
 
    memcpy(&ip_addr, binary_ip, IPV4_BINARY_LEN); 

    inet_ntop(AF_INET, &ip_addr, ip_str, IPV4_ADDR_STR_LEN);
}

unsigned short binary_to_port(const char *binary_port) {
    unsigned short port_nbo;

    memcpy(&port_nbo, binary_port, PORT_BINARY_LEN);  

    return ntohs(port_nbo);
}

void ip_to_binary(const char *ip_str, char *binary_ip) {
    unsigned int ip_addr;

    if (inet_pton(AF_INET, ip_str, &ip_addr) != 1) {
        fprintf(stderr, "Invalid IP address\n");
        return;
    }

    memset(binary_ip, 0, IPV4_BINARY_LEN); 

    memcpy(binary_ip, &ip_addr, IPV4_BINARY_LEN);
}

void port_to_binary(unsigned short port, char *binary_port) {
    unsigned short port_nbo = htons(port);

    memset(binary_port, 0, PORT_BINARY_LEN); 

    memcpy(binary_port, &port_nbo, PORT_BINARY_LEN);
}

int find_empty_index(forwardtab_t *forward) {
    for(int i = 0; i<6; i++) {
        if(forward[i].srcaddress == 0) {
            return i;
        }
    }
    return NO_INDEX_LEFT;
}

void set_to_zero(forwardtab_t * tabentry, int index) {
    tabentry[index].srcaddress = 0;
    tabentry[index].srcport = 0;
    tabentry[index].dstaddress = 0;
    tabentry[index].dstport = 0;
    tabentry[index].tunnelsport = 0;
}

unsigned long long encodesimp(unsigned long long x, unsigned long long pub) {
  return x ^ pub;
}

void main(int argc, char * argv[]) {
    forwardtab_t tabentry[6];
    for(int i = 0; i<6; i++) {
        tabentry[i].srcaddress = 0;
    }
    pubkey_t * pubkeys = initiate_pubkeys(pubkeys);
    // for(int i = 0; i<9; i++) {
    //     printf("Pubkeys: %s\n", pubkeys[i].IP);
    // }
    char self_addr[50], secrect[50];
    int self_port;
    // printf("Number of inputs: %d\n", argc);
    if(argc != 4) {
        printf("Please try again with parameters \n-------------------------------- \nThe format is:                   \tunnels <ip address> <port> <secret> \n-------------------------------- \nExiting....                      \n");
        return;
    }

    strcpy(self_addr, argv[1]);
    strcpy(secrect, argv[3]);
    self_port = atoi(argv[2]);

    int socket_return = socket(AF_INET, SOCK_STREAM, 0);
    if(socket_return < 0) {
        perror("Error occured while creating socket\n");
        return;
    }
    struct sockaddr_in myaddr;
    myaddr.sin_family = AF_INET;
    myaddr.sin_port = htons(self_port);
    inet_aton(self_addr, &myaddr.sin_addr);
    
    int bind_return = bind(socket_return, (struct sockaddr *) &myaddr, sizeof(myaddr));
    if(bind_return < 0) {
        printf("Binding failed for the following specifications \nIP Address: %s                                  \nPort: %d                                        \nPlease try again.                               \n", self_addr, self_port);
        perror("Error");
        return;
    }
    else {
        printf("Binded on port %d \n", self_port);
    }

    int listen_return = listen(socket_return, 6);
    // printf("Listen return: %d\n", listen_return);
    while(1) {
        printf("(Parent Process) Waiting for a connection....\n");
        struct sockaddr_in client_socket;
        socklen_t client_socklen = sizeof(client_socket);
        int accept_return = accept(socket_return, (struct sockaddr*) &client_socket, &client_socklen);
        
        if(accept_return < 0) {
            perror("(Parent Process) Could not accept the connection");
            continue;
        }
        else {
            printf("(Parent Process) Connected!\n");
        }
        char peer_addr_IP[17];
        inet_ntop(AF_INET, &client_socket.sin_addr, peer_addr_IP, INET_ADDRSTRLEN);
        // printf("Peer IP is: %s\n", peer_addr_IP);
        int client_index = search_peer_IP(pubkeys, peer_addr_IP); 
        if(client_index == NO_PUBKEY_FOUND) {
            printf("(Parent Process) IP not trusted. Terminating connection...\n");
            close(accept_return);
            continue;
        }

        char init, dest_addr[4];
        int read_return = read(accept_return,  &init, sizeof(init));
        // printf("The first character is %c\n", init);
        if(init != 'c') {
            printf("(Parent Process) TCP Connection did not send their first data to be 'c'\n");
            continue;
            // Something needs to happen.
        }

        unsigned long long tunnelc_secret; 
        read_return = read(accept_return, &tunnelc_secret, 8);

        unsigned long long client_addr_encoded = encodesimp(tunnelc_secret, pubkeys[client_index].pub);
        char tunnnelc_IP_BIN[IPV4_BINARY_LEN * 2], tunnnelc_IP_decoded[IPV4_ADDR_STR_LEN];
        memcpy(tunnnelc_IP_BIN, &client_addr_encoded, IPV4_BINARY_LEN);
        binary_to_ip(tunnnelc_IP_BIN, tunnnelc_IP_decoded);
        printf("The tunnelc IP is: %s\n", tunnnelc_IP_decoded);

        if(strcmp(tunnnelc_IP_decoded, pubkeys[client_index].IP) == 0) {
            printf("(Parent Process) Authenticated.\n");
        }
        else {
            // printf("")
            printf("(Parent Process) Could not succesfully authenticate. Closing connection...\n");
            close(accept_return);
            continue;
        }
    }
}