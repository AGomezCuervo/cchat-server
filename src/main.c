#include "main.h"
#include <arpa/inet.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <string.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <syslog.h>
#include <unistd.h>

#define PORT 5222
#define BACK_LOG 10
#define BUFFER_SIZE 1500

int server_fd;

// Prototypes
void hdl_signal(int signal);
int hdl_stream(char *buffer, struct initial_stream *stream, ssize_t *bytes_read );
void hdl_client(int client_fd);

void hdl_signal(int signal)
{
        if (signal == SIGINT)
        {
                printf("\nReceived SIGINT. Shutting down server....\n");
                if (server_fd != -1)
                        close(server_fd);

                exit(EXIT_SUCCESS);
        }
}

int main(void)
{
        struct sockaddr_in server_addr, client_addr;
        socklen_t client_len;

        client_len = sizeof(client_addr);

        memset(&server_addr, 0, sizeof(server_addr));
        memset(&server_addr, 0, sizeof(server_addr));

        if (signal(SIGINT, hdl_signal) == SIG_ERR)
        {
                perror("error signal");
                exit(EXIT_FAILURE);
        }

        server_fd = Socket(AF_INET, SOCK_STREAM, 0);

        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(PORT);
        server_addr.sin_addr.s_addr = INADDR_ANY;

        Bind(server_fd, (struct sockaddr *) &server_addr, sizeof(server_addr));

        Listen(server_fd, BACK_LOG);
        printf("Server listening on port %d\n", PORT);

        /*while (1)*/
        /*{*/
                int client_fd;
                client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
                if (client_fd == -1)
                {
                        err_sys("Failed accept");
                        /*continue;*/
                }
                printf("Client connected\n");

                hdl_client(client_fd);
        /*}*/
        close(server_fd);
        return 0;
}

int hdl_stream(char *buffer, struct initial_stream *stream, ssize_t *bytes_read)
{
        char *initial_st;
        initial_st = strstr(buffer, "<stream:stream");
        if(initial_st != NULL)
        {
                char *end_st = strstr(buffer, "</stream:stream>");
                if(end_st == NULL)
                {
                        size_t close_tag_len = 16;
                        size_t buffer_len = strlen(buffer);
                        if(BUFFER_SIZE - buffer_len >= close_tag_len + 1)
                        {
                                strncat(buffer, "</stream:stream>", close_tag_len);
                                *bytes_read = buffer_len + close_tag_len;
                        }
                        else
                        {
                                err_buf("Not enough space");
                                return -1;
                        }
                };
                xmlDocPtr doc = xmlReadMemory(buffer, *bytes_read, "noname.xml", NULL, 0);
                if (doc == NULL) {
                        fprintf(stderr, "Failed to parse XML\n");
                        return -1;
                }

                // Search attributes
                xmlNode *root = xmlDocGetRootElement(doc);
                if(parse_stream_attributes(root, stream) == 0)
                {
                        printf("from: %s\n", stream->from);
                        printf("to: %s\n", stream->to);
                        printf("version: %s\n", stream->version);
                        printf("xml:lang: %s\n", stream->xml_lang);
                        printf("xmlns: %s\n", stream->xmlns);
                        printf("xmlns:stream: %s\n", stream->xmlns_stream);
                }
                xmlFree(root);
                return 1;
        }
        else
        {
                err_buf("Not a xml stream");
                return -1;
        }
        return 0;
        
}
void hdl_client(int client_fd)
{
        int stream_status;
        char buffer[BUFFER_SIZE];
        ssize_t bytes_read;
        struct initial_stream stream = {0};
        xmlDocPtr doc;

        stream_status = 0;

        while ( (bytes_read = recv(client_fd, buffer, sizeof(buffer), 0)) > 0)
        {
                buffer[bytes_read] = '\0';
                if(!stream_status)
                        stream_status = hdl_stream(buffer, &stream, &bytes_read );
                doc = xmlReadMemory(buffer, bytes_read, "noname.xml", NULL, 0);
                if (doc == NULL) {
                        fprintf(stderr, "Failed to parse XML\n");
                        return;
                }
                printf("Received data: %s\n", buffer);
        }

        if (bytes_read == -1)
        {
                printf("No data received\n");
        }
        close(client_fd);
}
