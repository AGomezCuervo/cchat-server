#include "main.h"
#include "err.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>
#include <syslog.h>

#define PORT 8000
#define BACK_LOG 4
#define BUFFER_SIZE 256

int server_fd, client_fd;

void print_received_data(struct Request *request)
{
  printf("Method_type: %d\n", request->method_type);
  printf("Message_type: %d\n", request->message_type);
  printf("Data_size: %d\n", request->data_size);
  printf("File_type: %d\n", request->file_type);
  printf("Filename_size: %d\n", request->filename_size);
  printf("Filename: %s\n", request->file_name);
  printf("Data: %s\n", request->data);
}

struct Request load_data(char *buffer, ssize_t bytes_received)
{
        struct Request request = {0};
        int offset = 0;

        request.method_type = buffer[offset];
        offset++;

        request.message_type = buffer[offset];
        offset++;

        request.data_size = (uint16_t)((buffer[offset] << 8) | buffer[offset+1]);
        offset += 2;

        request.file_type = buffer[offset];
        offset++;

        request.filename_size = buffer[offset];
        offset++;

        if(request.filename_size <= 50)
        {
          memcpy(request.file_name, buffer + offset, request.filename_size);
          request.file_name[request.filename_size] = '\0';
          offset += MAX_FILE_NAME_SIZE;
        }
        else
        {
                request.method_type = -1;
                printf("\nFilename size exceed the limit\n");
                return request;
        }
        size_t message_size = request.data_size;

        if(offset + message_size <= bytes_received)
                memcpy(request.data, buffer + offset, message_size);
        else
        {
                request.method_type = -1;
                printf("\n message exceed the limit\n");
                return request;
        }

        return request;
}

void handle_signal(int signal)
{
    if (signal == SIGINT)
    {
        printf("\nReceived SIGINT. Shutting down server...\n");
        if (server_fd != -1)
            close(server_fd);

        if (client_fd != -1)
          close(client_fd);

        exit(EXIT_SUCCESS);
    }
}

int main()
{
        // Setup variables and server
        struct sockaddr_in server_addr, client_addr;
        socklen_t client_len = sizeof(client_addr);

        if(signal(SIGINT, handle_signal) == SIG_ERR)
        {
          perror("error signal");
          exit(EXIT_FAILURE);
        }

        server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if(server_fd == -1)
                err_sys("Creating socket failed");

        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(PORT);
        server_addr.sin_addr.s_addr = INADDR_ANY;

        if(bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
        {
                close(server_fd);
                err_sys("Bind failed");
        }

        if(listen(server_fd, BACK_LOG))
        {
                close(server_fd);
                err_sys("listen failed");
        }
        printf("server listening on port %d\n", PORT);

        client_fd = accept(server_fd, (struct sockaddr *) &client_addr, &client_len);
        printf("client connected\n");

        // Reading data from client
        char buffer[BUFFER_SIZE] = {0}; 
        ssize_t bytes_received;
        while((bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0)) > 0)
        {
                buffer[bytes_received] = '\0';
                struct Request request = load_data(buffer, bytes_received);
                print_received_data(&request);
        }

        if(bytes_received < 0)
        {
          perror("recv failed");
        }

        close(client_fd);
        close(server_fd);
        return 0;
}

