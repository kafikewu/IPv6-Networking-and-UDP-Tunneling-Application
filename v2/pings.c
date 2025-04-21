#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc, char *argv[]) {
    char enth0_addr[20];
    strcpy(enth0_addr, argv[1]);
    int bind_return, port = atoi(argv[2]);
    int socket_return = socket(AF_INET, SOCK_DGRAM, 0);
    if(socket_return == -1) {
        return 0;
    }
    for(int i = 0; i<10; i++) {
        struct sockaddr_in socket_address;
        socket_address.sin_family = AF_INET;
        inet_aton(enth0_addr, &socket_address.sin_addr);
        socket_address.sin_port = htons(port + i);
        bind_return = bind(socket_return, (struct sockaddr *) &socket_address, sizeof(socket_address));
        if(bind_return == 0) {
            printf("Binded on port: %d!\n", port + i);
            break;
        }
    }
    if(bind_return != 0) {
        printf("Exiting!");
        return 0;
    }
    while(1) {
        printf("Waiting for recvfrom()\n");
        /*  Creates address for receiption    */
        struct sockaddr_in client_socket;
        int length_of_client_socket = sizeof(client_socket);
        
        /*  Creates a UDP Packet with 100 bytes.
            Each char takes up 1 byte.
        */
        char packet[100];
        
        /*  Calls sendto()  */
        int recvfrom_result = recvfrom(socket_return, packet, sizeof(packet), 0, (struct sockaddr *) &client_socket, &length_of_client_socket);
        // printf("The result from the server is: %d\n", recvfrom_result);
        if(recvfrom_result < 0) {
            printf("Error occured when receiving the packet. Please try again.\n");
            return 0;
        }
        
        /*  Accept the message  */
        short int sequence;
        char control;
        memcpy(&sequence, packet, sizeof(sequence));
        memcpy(&control, packet + sizeof(sequence), sizeof(control));
        printf("The sequence is: %u and control is: %c\n", sequence, control);
        int send_back = 0;
        if(control == '0') {
            send_back = 1;
        }
        else if(control == '1') {
            usleep(555 * 1000);
            send_back = 1;
        }
        else if(control == '2') {
            send_back = 0;
        }
        else {
            char* client_address = inet_ntoa(client_socket.sin_addr);
            int client_port = ntohs(client_socket.sin_port);
            printf("Client's IP address: %s, port: %d, command: %c\n", client_address, client_port, control);
        }

        if(send_back) {
            char* client_address = inet_ntoa(client_socket.sin_addr);
            int client_port = ntohs(client_socket.sin_port);
            printf("Sending it back to IP: %s and port: %d\n", client_address, client_port);
            int sendto_result = sendto(socket_return, packet, sizeof(packet), 0, (struct sockaddr *) &client_socket, sizeof(client_socket));
        }
    }
}