#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>

void create_UDP_packet(char* packet, short int sequence, char control)
{
    *(short int*)packet = sequence;
    memset(packet + sizeof(short int), control, sizeof(control));
}

int main(int argc, char *argv[]) {
    char enth0_addr[20], server_addr[20];
    int bind_return, port = 0, server_port = atoi(argv[2]);
    strcpy(server_addr, argv[1]);
    strcpy(enth0_addr, argv[3]);
    int socket_return = socket(AF_INET, SOCK_DGRAM, 0);
    if(socket_return == -1) {
        return 0;
    }
    /*  Assigns a address to the socket with appropriate information    */
    struct sockaddr_in socket_address;
    socket_address.sin_family = AF_INET;
    inet_aton(enth0_addr, &socket_address.sin_addr);
    socket_address.sin_port = htons(port);
    bind_return = bind(socket_return, (struct sockaddr *) &socket_address, sizeof(socket_address));
    if(bind_return == 0) {
        printf("Binded on port: %d!\n", port);
    }
    else {
        printf("Failed!. Exiting..");
        return 0;
    }
    /*  Creates address for delivery    */
    struct sockaddr_in server_socket;
    server_socket.sin_family = AF_INET;
    inet_aton(server_addr, &server_socket.sin_addr);
    server_socket.sin_port = htons(server_port);
    /*  Creates a UDP Packet with 100 bytes.
        Each char takes up 1 byte.
    */
    char packet[100];
    create_UDP_packet(packet, 20, '1');
    /*  gettimeofday() preparation  */
    struct timeval start;
	struct timeval end;
    unsigned long e_usec;
    /*  Calls sendto()  */
    gettimeofday(&start, 0);
    int sendto_result = sendto(socket_return, packet, sizeof(packet), 0, (struct sockaddr *)&server_socket, sizeof(server_socket));
    // printf("The result from the client is: %d\n", sendto_result);
    struct sockaddr_in client_socket;
    int length_of_client_socket = sizeof(client_socket);
    
    /*  Creates a UDP Packet with 100 bytes.
        Each char takes up 1 byte.
    */
    char recvpacket[100];
    
    /*  Calls sendto()  */
    int recvfrom_result = recvfrom(socket_return, recvpacket, sizeof(recvpacket), 0, (struct sockaddr *) &client_socket, &length_of_client_socket);
    if(recvfrom_result > 0) {
        gettimeofday(&end, 0);
    }
    short int sequence;
    memcpy(&sequence, recvpacket, sizeof(sequence));
    if(sequence != 20) {
        printf("The sequence number do not match.\n");
    }
    else {
        e_usec = ((end.tv_sec * 1000000) + end.tv_usec) - ((start.tv_sec * 1000000) + start.tv_usec);
        printf("The difference was: %.3fms\n", e_usec / 1000.000);
    }
}