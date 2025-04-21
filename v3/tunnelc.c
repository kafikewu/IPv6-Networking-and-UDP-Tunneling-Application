#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#define IPV4_BINARY_LEN 4
#define PORT_BINARY_LEN 2
#define IPV4_ADDR_STR_LEN 16

#define PRIVKEY 0xAAAAAAAAAAAAAAAA
// #define PRIVKEY 0xCCCCCCCCCCCCCCCC

unsigned long long decodesimp(unsigned long long x, unsigned long long priv) {
  return x ^ priv;
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

int main(int argc, char *argv[]) {
    /*  tunnels ->  128.10.112.133 55550
     *  secret  ->  abcdABCD
     *  pingc   ->  128.10.112.134
     *  pings   ->  128.10.112.135 56001
     */

    char tunnels_addr[20], pingc_addr[20], pings_addr[20], secret[20];
    int tunnels_port;
    unsigned short pings_port;
    strcpy(tunnels_addr, argv[1]);
    tunnels_port = atoi(argv[2]);
    strcpy(secret, argv[3]);
    // strcpy(pingc_addr, argv[4]);
    // strcpy(pings_addr, argv[5]);
    // pings_port = (unsigned short) strtoul(argv[6], NULL, 0);

    int socket_return = socket(AF_INET, SOCK_STREAM, 0);
    if(socket_return == -1) {
        return 0;
    }

    struct sockaddr_in tunnels_sockaddr;
    tunnels_sockaddr.sin_family = AF_INET;
    tunnels_sockaddr.sin_port = htons(tunnels_port);
    if(!inet_pton(AF_INET, tunnels_addr, &tunnels_sockaddr.sin_addr)) {
        printf("Please enter valid IP address!\n");
        return 0;
    }
    printf("Attempting to connect to the the tunnels with IP: %s and port %d\n", tunnels_addr, tunnels_port);

    int connect_return = connect(socket_return, (struct sockaddr *) &tunnels_sockaddr, sizeof(tunnels_sockaddr));

    char stream[8], ini = 'c';

    struct sockaddr_in local_addr;
    socklen_t addrlen = sizeof(local_addr);

    if (getsockname(socket_return, (struct sockaddr*)&local_addr, &addrlen) < 0) {
        perror("Error getting socket name");
        exit(1);
    }
    char * local_addr_IP = inet_ntoa(local_addr.sin_addr), local_addr_IP_bin[IPV4_BINARY_LEN];
    printf("My Address is: %s\n", local_addr_IP);
    ip_to_binary(local_addr_IP, local_addr_IP_bin);

    char secret_string[IPV4_BINARY_LEN * 2];
    strncpy(secret_string, local_addr_IP_bin, IPV4_BINARY_LEN);
    strncpy(secret_string + IPV4_BINARY_LEN, local_addr_IP_bin, IPV4_BINARY_LEN);
    
    unsigned long long int secret_long;
    memcpy(&secret_long, &secret_string, IPV4_BINARY_LEN * 2);

    secret_long = decodesimp(secret_long, PRIVKEY);
    
    strncpy(stream, secret, 8);
    int send_return = send(socket_return, &ini, sizeof(ini), 0);
    if(send_return < 0) {
        perror("Send Return is less than zero");
    }
    send_return = send(socket_return, &secret_long, 8, 0);

    char pings_addr_bin[IPV4_BINARY_LEN];
    ip_to_binary(pings_addr, pings_addr_bin);
    send_return = send(socket_return, pings_addr_bin, sizeof(pings_addr_bin), 0);

    char pings_port_bin[PORT_BINARY_LEN];
    port_to_binary(pings_port, pings_port_bin);
    send_return = send(socket_return, pings_port_bin, sizeof(pings_port_bin), 0);

    char pingc_addr_bin[IPV4_BINARY_LEN];
    ip_to_binary(pingc_addr, pingc_addr_bin);
    send_return = send(socket_return, pingc_addr_bin, sizeof(pingc_addr_bin), 0);
 
 
    printf("Press enter to send termination signal to tunnel server....");
    getchar();
    send_return = send(socket_return, stream, 8, 0);    
    printf("Socket is terminated\n");
    close(socket_return);
}