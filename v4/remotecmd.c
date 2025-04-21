// Concurrent server example: simple shell using fork() and execlp().

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
#include <net/if.h>

// #include <stdio.h>
// #include <string.h>
// #include <stdlib.h>
// #include <sys/socket.h>
#include <sys/time.h>
// #include <netinet/in.h>
// #include <arpa/inet.h>
#include <error.h>
#include <ifaddrs.h>
// #include <net/if.h>

#define SAFE_IP "128.10.112."
#define LS "ls"
#define DATE "date" 
#define MAX_COMMAND_LENGTH 30
char** parse_token(char arg[])
{
  /*  The arg string does not provide permission to modify
   *  Thus, it is necessary to duplicate it with the necessary permissions
   */
  char* duplicate_string = malloc(strlen(arg) + 1);
  strcpy(duplicate_string, arg);
  char* token = strtok(duplicate_string, " ");
  char** token_array = (char **)malloc(30 * sizeof(char *));
  int i = 0;
  while(token != NULL) {
    token_array[i] = token;
    i++;
    token = strtok(NULL, " ");
  }
  token_array[i] = NULL;
  /*  The execvp mathod takes an string array that is terminated by a NULL pointer
   */
  return token_array;
}

int main(int argc, char* argv[])
{
  if(argc != 3) {
    printf("Please try again with parameters \n-------------------------------- \nThe format is:                   \nremotecmd.bin <ip address> <port>  \n-------------------------------- \nExiting....                      \n");
    return 0;
  }
  int status;
  int len, self_port = atoi(argv[2]);
  char self_addr[40];
  strcpy(self_addr, argv[1]);

  int socket_return = socket(AF_INET6, SOCK_STREAM, 0);
  if(socket_return < 0) {
    perror("Error in allocting the socket");
  }
  
  struct sockaddr_in6 myaddr;
  myaddr.sin6_family = AF_INET6;
  myaddr.sin6_port = htons(self_port);
  int inet_pton_result = inet_pton(AF_INET6, self_addr, &myaddr.sin6_addr);
  // printf("Inet result is: %d\n", inet_pton_result);
  myaddr.sin6_scope_id = if_nametoindex("eth0");

  int bind_return = bind(socket_return, (struct sockaddr *) &myaddr, sizeof(myaddr));
  if(bind_return < 0) {
    printf("Binding failed for the following specifications \nIP Address: %s                                  \nPort: %d                                        \nPlease try again.                               \n", self_addr, self_port);
    return 0;
  }
  else {
    char address_str[INET6_ADDRSTRLEN]; // Buffer to store human-readable address 
    const char *result = inet_ntop(AF_INET6, &myaddr.sin6_addr, address_str, INET6_ADDRSTRLEN);
    printf("Binded on IP address: %s and port %d \n", address_str, self_port);
  }
  int listen_return = listen(socket_return, 5);
  while(1) {
    struct sockaddr_in6 addr;
    int addr_len = sizeof(addr);
    addr.sin6_family = AF_INET6;
    printf("Waiting for connection...\n");
    int accept_return = accept(socket_return, (struct sockaddr *)&addr, &addr_len);
    getpeername(accept_return, (struct sockaddr *)&addr, &addr_len);
    int client_port = ntohs(addr.sin6_port);
    char client_addr[50];
    inet_ntop(AF_INET6, &addr.sin6_addr, client_addr, INET6_ADDRSTRLEN);
    printf("The client details is as follows:\n---------------------------------\nIP Address: %s\nPort: %d\n---------------------------------\n", client_addr, client_port);

    // if(strstr(client_addr, SAFE_IP) != NULL) {
    //   if(strcmp(strstr(client_addr, SAFE_IP), client_addr) == 0) {
    //     printf("The client IP is safe!\n");
    //   }
    //   else {
    //     printf("Unsafe IP detected. Avoiding...\n");
    //     continue;
    //   }
    // }
    // else {
    //   printf("Unsafe IP detected. Avoiding...\n");
    //   continue;
    // }
    char buf[MAX_COMMAND_LENGTH + 1];
    read(accept_return, buf, sizeof(buf));
    len = strlen(buf);
    if(len == 1) 				// case: only return key pressed
      continue;
    buf[len-1] = '\0';			// case: command entered
    if(strlen(buf) > 30) {
      continue;
    }
    printf("User: %s\n", buf);
    char** token = parse_token(buf);
    int invalid_token = 0;
    if(strcmp(token[0], LS) != 0 && strcmp(token[0], DATE) != 0) {
      invalid_token++;
    }
    int token_it = 0;
    while (token[token_it] != NULL)
    {
      if(strlen(token[token_it]) > 2 && strcmp(token[0], DATE) != 0) {
        // printf("More than 2\n");
        invalid_token++;
      }
      // printf("The token is %s and size is: %ld\n", token[token_it], strlen(token[token_it]));
      token_it++;
    }
    if(token_it > 3 && strcmp(token[0], DATE) != 0) {
      // printf("More tokens!\n");
      invalid_token++;
    }
    if(token_it != 1 && strcmp(token[0], DATE) == 0) {
      invalid_token++;
    }
    if(invalid_token != 0) {
      printf("The user input was not valid!\n");
      continue;
    }
    
    int k = fork();
    if (k==0) {
    // child code
      if(execvp(token[0], token) == -1) {
        char return_string[] = "command failed\0";
        write(accept_return, return_string, sizeof(return_string));
      }
    }
    else {
      waitpid(k, &status, 0);		// block until child process terminates
      sleep(1);
    }
  }

}