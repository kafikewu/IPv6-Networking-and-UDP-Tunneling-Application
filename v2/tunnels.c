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
#include <sys/select.h>
#include <sys/shm.h>

#define IPV4_BINARY_LEN 4
#define IPV4_ADDR_STR_LEN 16
#define PORT_BINARY_LEN 2
#define NO_INDEX_LEFT -10


typedef struct forwardtab {
   unsigned long srcaddress;
   unsigned short srcport;
   unsigned long dstaddress;
   unsigned short dstport;
   unsigned short tunnelsport; 
} forwardtab_t;

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
        printf("Invalid IP address\n");
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

short find_empty_index(forwardtab_t *forward) {
    for(short i = 0; i<6; i++) {
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

void main(int argc, char * argv[]) {
    int shm_id = shmget(IPC_PRIVATE, sizeof(forwardtab_t) * 6, IPC_CREAT | 0666);
    forwardtab_t * tabentry = shmat(shm_id, NULL, 0);

    for(int i = 0; i<6; i++) {
        tabentry[i].srcaddress = 0;
    }

    char self_addr[50], secrect[50];
    int self_port;

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
        char init, dest_addr[4], tunnelc_secret[9];
        int read_return = read(accept_return,  &init, sizeof(init));

        if(init != 'c') {
            printf("(Parent Process) TCP Connection did not send their first data to be 'c'\n");
            close(accept_return);
            continue;
        }
        read_return = read(accept_return, tunnelc_secret, 8);
        tunnelc_secret[8] = '\0';
        if(strcmp(tunnelc_secret, secrect) != 0) {
            printf("(Parent Process) Secret don't match\n");
            close(accept_return);
            continue;
        }

        char pings_addr_bin[4], pings_addr[30];
        read_return = read(accept_return, pings_addr_bin, sizeof(pings_addr_bin));

        binary_to_ip(pings_addr_bin, pings_addr);


        char pings_port_bin[PORT_BINARY_LEN];
        read_return = read(accept_return, pings_port_bin, sizeof(pings_port_bin));
        unsigned short pings_port;
        pings_port = binary_to_port(pings_port_bin);


        char pingc_addr_bin[4], pingc_addr[30];
        read_return = read(accept_return, pingc_addr_bin, sizeof(pingc_addr_bin));

        binary_to_ip(pingc_addr_bin, pingc_addr);
        // printf("The pingc IP address is: %s\n", pingc_addr);
        // printf("Finding for empty index\n");
        short index = find_empty_index(tabentry);
        if(index == NO_INDEX_LEFT) {
            printf("(Parent Process) No index left. Closing connection..\n");
            close(accept_return);
            continue;
        }
        // printf("Empty index found\n");
        // tabentry[index].dstaddress
        memcpy(&tabentry[index].dstaddress, pings_addr_bin, IPV4_BINARY_LEN);
        memcpy(&tabentry[index].srcaddress, pingc_addr_bin, IPV4_BINARY_LEN);
        // memcpy(&tabentry[index].dstport, &pings_port_bin, PORT_BINARY_LEN);
        tabentry[index].dstport = pings_port;
        // printf("DOne\n");
        int fork_return = fork();
        // printf("the fork return is: %d\n", fork_return);
        if(fork_return == 0) {
            // printf("Child Code\n");
            // printf("CHild propcess\n");;
            // Child process
            int socket_1 = socket(AF_INET, SOCK_DGRAM, 0);
            struct sockaddr_in socket_1_addr;
            unsigned short socket_1_port = 60000;
            // printf("Tryint to connect with :%s and port: %u\n", self_addr, socket_1_port);
            while(1) {
                socket_1_addr.sin_family = AF_INET;
                socket_1_addr.sin_port = htons(socket_1_port);
                inet_aton(self_addr, &socket_1_addr.sin_addr);
                int socket_1_bind = bind(socket_1, (struct sockaddr *) &socket_1_addr, sizeof(socket_1_addr));
                // printf("Trying with port: %u and return: %d\n", socket_1_port, socket_1_bind);
                if(socket_1_bind == 0) {
                    // printf("Binded on port: %u\n", socket_1_port);
                    break;
                }
                else {
                    socket_1_port++;
                }
            }
            // tabentry[index].tunnelsport = socket_1_port;
            int write_return = write(accept_return, &socket_1_port, sizeof(socket_1_port));
            // close(socket_1);
            
            int socket_2 = socket(AF_INET, SOCK_DGRAM, 0);
            struct sockaddr_in socket_2_addr;
            unsigned short socket_2_port = 64000;
            // printf("The write return is: %d\n", write_return);
            while(1) {
                socket_2_addr.sin_family = AF_INET;
                socket_2_addr.sin_port = htons(socket_2_port);
                inet_aton(self_addr, &socket_2_addr.sin_addr);
                int socket_2_bind = bind(socket_2, (struct sockaddr *) &socket_2_addr, sizeof(socket_2_addr));
                // printf("Trying with port: %u and return: %d\n", socket_1_port, socket_1_bind);
                if(socket_2_bind == 0) {
                    // printf("Binded on port: %u\n", socket_2_port);
                    break;
                }
                else {
                    socket_2_port++;
                }
            }
            while(1) {
                // printf("checkpoint 1");
                fd_set readfds;
                FD_ZERO(&readfds);
                FD_SET(accept_return, &readfds);
                FD_SET(socket_1, &readfds); 
                FD_SET(socket_2, &readfds); 
                int ready_fds = select(FD_SETSIZE, &readfds, NULL, NULL, NULL); 
                // close(socket_2);
                // printf("Checkpoint 3\n");
                int pingc_data_received = 0;
                // printf("Ready fds: %d\n", ready_fds);
                if (ready_fds == -1) {
                    perror("select");
                    exit(1);
                }
                else {
                    // Check which socket has data ready to read
                    if (FD_ISSET(accept_return, &readfds)) {
                        char termination_text[9];
                        // printf("There is data at TCP server\n");
                        int termination = read(accept_return, termination_text, 8);
                        // printf("The data I just read is: %s\n", termination_text);
                        if(strncmp(termination_text, secrect, 8) == 0) {
                            printf("(Child Process) A connection has been terminated\n");
                        }
                        else {
                            continue;
                        }
                        set_to_zero(tabentry, index);
                        // printf("The local value is: %u\n", tabentry[index].srcport);
                        close(accept_return);
                        close(socket_1);
                        close(socket_2);
                        // printf("retuning\n");
                        exit(1);
                        // printf("I have data at TCP\n");
                    }
                    if (FD_ISSET(socket_1, &readfds)) {
                        // printf("I have data from client\n");
                        char forward_packet[100];
                        struct sockaddr_in pingc_socket;
                        int length_of_pingc_socket = sizeof(pingc_socket);
                        int recvfrom_result = recvfrom(socket_1, forward_packet, sizeof(forward_packet), 0, (struct sockaddr *) &pingc_socket, &length_of_pingc_socket);
                        // if(recvfrom_result < 1) {
                        //     printf("The pingc app might be closed! Please try again.\n");
                        // }
                        pingc_data_received = 1;
                        
                        tabentry[index].srcport = ntohs(pingc_socket.sin_port);
                        char pings_IPv4_addr_bin[IPV4_BINARY_LEN], pings_IPv4_addr[IPV4_ADDR_STR_LEN];

                        memcpy(pings_IPv4_addr_bin, &tabentry[index].dstaddress, IPV4_BINARY_LEN);
                        binary_to_ip(pings_IPv4_addr_bin, pings_IPv4_addr);
                        short int pings_port_;

                        struct sockaddr_in pings_socket_to_send;
                        pings_socket_to_send.sin_family = AF_INET;
                        pings_socket_to_send.sin_port = htons(tabentry[index].dstport);
                        inet_aton(pings_IPv4_addr, &pings_socket_to_send.sin_addr);
                        int send_to_pings = sendto(socket_2, forward_packet, sizeof(forward_packet), 0, (struct sockaddr *) &pings_socket_to_send, sizeof(pings_socket_to_send));
                    }
                    if (FD_ISSET(socket_2, &readfds)) {
                        
                        char backward_packet[100];
                        struct sockaddr_in pings_socket_to_receive;
                        int length_of_pings_socket = sizeof(pings_socket_to_receive);
                        int recvfrom_result = recvfrom(socket_2, backward_packet, sizeof(backward_packet), 0, (struct sockaddr *) &pings_socket_to_receive, &length_of_pings_socket);

                        struct sockaddr_in pingc_socket_to_send;
                        pingc_socket_to_send.sin_family = AF_INET;
                        pingc_socket_to_send.sin_port = htons(tabentry[index].srcport);
                        
                        char pingc_socket_to_send_addr[IPV4_ADDR_STR_LEN], pingc_socket_to_send_addr_bin[IPV4_BINARY_LEN];
                        memcpy(pingc_socket_to_send_addr_bin, &tabentry[index].srcaddress, IPV4_BINARY_LEN);
                        binary_to_ip(pingc_socket_to_send_addr_bin, pingc_socket_to_send_addr);

                        inet_aton(pingc_socket_to_send_addr, &pingc_socket_to_send.sin_addr);

                        int send_to_pingc = sendto(socket_1, backward_packet, sizeof(backward_packet), 0, (struct sockaddr *) &pingc_socket_to_send, sizeof(pingc_socket_to_send));
                    }
                }
            }
            return;
        }
    }
}