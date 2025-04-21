#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <ifaddrs.h>
#include <net/if.h>
#define MAX_INPUT_LENGTH 30 // While taking input from the stdin stream, \n and \0 needs to be facilitated.

void sigalrm_handlar(int ret) {
  exit(0);
}

int main(int argc, char* argv[]) {
  if(argc != 3) {
    printf("Please try again with parameters \n-------------------------------- \nThe format is:                   \nremotecmdc.bin <ip address of server> <port of server>  \n-------------------------------- \nExiting....                      \n");
    return 0;
  }
  
  int client_port = atoi(argv[2]);
  char client_addr[50];
  strcpy(client_addr, argv[1]);

  int socket_return = socket(AF_INET6, SOCK_STREAM, 0);
  if(socket_return < 0) {
    printf("Error in allocting the socket. Please try again! \n");
  }
  
  // printf("Client IP: %s and client port: %d\n", client_addr, client_port);
  struct sockaddr_in6 server_addr;
  server_addr.sin6_family = AF_INET6;
  server_addr.sin6_port = htons(client_port);
  inet_pton(AF_INET6, client_addr, &server_addr.sin6_addr);
  server_addr.sin6_scope_id = if_nametoindex("eth0");

  int connect_return = connect(socket_return, (struct sockaddr *) &server_addr, sizeof(server_addr));
  if(connect_return < 0) {
    perror("Could not connect");
    return 0;
  }
  else {
    printf("Successfully connected with the server!\n");
  }
  char buffer[MAX_INPUT_LENGTH + 1];

  fprintf(stdout, "? ");
  if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
    if (strchr(buffer, '\n') == NULL) {
        int c;
        while ((c = getchar()) != '\n') {
        }
        printf("Error: More than 30 characters entered. Please try again!\n");
        return 0;
    }
  }
  int write_return = write(socket_return, buffer, sizeof(buffer));
  signal(SIGALRM, sigalrm_handlar);
  ualarm(500000, 0);
  char return_buffer[MAX_INPUT_LENGTH];
  int read_return = read(socket_return,  return_buffer, sizeof(return_buffer));
  if(read_return > 0) {
    printf("%s\n", return_buffer);
  }
  /*  Have to close socket      */
  /*  Have to close connection  */
  exit(0);
}