#include "main.h"
#include <libxml/parser.h>
#include <libxml/xmlreader.h>
#include <libxml/tree.h>
#include <sys/prctl.h>
#include <string.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>

#define PORT 5220
#define BACK_LOG 10
#define BUFFER_SIZE 1500
#define DELIMITER "\r\t"

// TODO: handle error on connection lost

enum Message_type {
        STREAM_STREAM,
        MESSAGE,
        ERROR = -1,
};

int child_pid;
int server_fd;

int connection;

// Prototypes
void hdl_connection(int client_fd, char *buffer, struct stream_conf *stream_conf);
void create_stream_id(struct stream_conf *stream_conf, size_t len);
int init_connection(int fd, struct stream_conf *stream_conf);
enum Message_type hdl_stanza( int fd, char *stanza, size_t stanza_len, struct stream_conf *stream_conf);
int hdl_stream(struct stream_conf *stream_conf, xmlTextReaderPtr reader);

int main(void)
{
        prctl(PR_SET_NAME, "xmpp_server", 0, 0, 0);
        struct sockaddr_in server_addr, client_addr;
        socklen_t client_len;

        client_len = sizeof(client_addr);

        memset(&server_addr, 0, sizeof(server_addr));
        memset(&server_addr, 0, sizeof(server_addr));
        Signal(SIGINT, sig_exit);

        server_fd = Socket(AF_INET, SOCK_STREAM, 0);

        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(PORT);
        server_addr.sin_addr.s_addr = INADDR_ANY;

        Bind(server_fd, (struct sockaddr *) &server_addr, sizeof(server_addr));

        Listen(server_fd, BACK_LOG);
        Signal(SIGCHLD, sig_child);
        printf("Server listening on port %d\n", PORT);

        while (1)
        {
                int client_fd;
                client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
                if(client_fd < 0)
                {
                        if(errno == EINTR)
                                continue;
                        else
                                err_sys("Failed accept");
                }

                if( (child_pid = fork()) == 0)
                {
                        Close(server_fd);
                        struct stream_conf stream_conf = {0}; 
                        char buffer[BUFF_LEN];

                        printf("Client connected\n");
                        hdl_connection(client_fd, buffer, &stream_conf);
                        Close(client_fd);
                        exit(0);
                }
                Close(client_fd);
        }
        return 0;
}

void hdl_connection(int fd, char *buffer, struct stream_conf *stream_conf)
{
        ssize_t bytes_read;
        char stanza_buffer[BUFF_LEN];

        while ( (bytes_read = recv(fd, buffer, BUFF_LEN - 1, 0)) > 0)
        {
                memcpy(stanza_buffer, buffer, bytes_read );
                stanza_buffer[bytes_read] = '\0';

                char *stanza_end;
                if( (stanza_end = strstr(stanza_buffer, "<stream:stream")) == NULL)
                {
                        if( (stanza_end = strstr(stanza_buffer, "</message>")) == NULL)
                        {
                                if( (stanza_end = strstr(stanza_buffer, "</stream:stream>")) == NULL)
                                {
                                }
                        }
                }


                if(stanza_end != NULL) {
                        hdl_stanza(fd, stanza_buffer, bytes_read, stream_conf);
                } else {
                        Send(fd, stanza_buffer, bytes_read);
                }
        }
}


enum Message_type hdl_stanza(int fd, char *stanza, size_t stanza_len, struct stream_conf *stream_conf)
{
        xmlTextReaderPtr reader;
        int ret;

        reader = xmlReaderForMemory(stanza, stanza_len, NULL, NULL, 0);

        if(reader == NULL)
        {
                err_buf("failed to create reader");
                return ERROR;
        }

        while( (ret = xmlTextReaderRead(reader)) == 1)
        {
                const char *name = (const char *)xmlTextReaderConstName(reader);
                if(name == NULL) return ERROR;

                int node_type = xmlTextReaderNodeType(reader);
                switch (node_type) {
                        case XML_READER_TYPE_ELEMENT:
                                if(strcmp(name, "stream:stream") == 0)
                                {
                                        if(hdl_stream(stream_conf, reader) < 0)
                                        {
                                                err_buf("stream:stream bad format");
                                                return ERROR;
                                        } else
                                        {
                                                init_connection(fd, stream_conf);
                                        }
                                        return STREAM_STREAM;
                                }
                                else
                                {
                                        Send(fd, stanza, stanza_len);
                                }
                                break;

                        case XML_READER_TYPE_END_ELEMENT:
                                if(strcmp(name, "stream:stream") == 0)
                                break;

                        default:
                                break;
                }
        }
        return 0;
}

int hdl_stream(struct stream_conf *stream_conf, xmlTextReaderPtr reader)
{
        if(xmlTextReaderHasAttributes(reader))
        {
                while (xmlTextReaderMoveToNextAttribute(reader))
                {
                        uint attr_len;
                        const char *attr_name = (const char *)xmlTextReaderConstName(reader);
                        const char *attr_value = (const char *)xmlTextReaderConstValue(reader);
                        attr_len = strlen(attr_value) + 1;

                        if(attr_len > MAX_VALUE - 1)
                                attr_len = MAX_VALUE - 1 ;

                        if((strcmp(attr_name, "from")) == 0)
                        {
                                memcpy(stream_conf->from, attr_value, attr_len);
                                stream_conf->from[MAX_VALUE - 1] = '\0';
                                continue;
                        }

                        if((strcmp(attr_name, "to")) == 0)
                        {
                                memcpy(stream_conf->to, attr_value, attr_len);
                                stream_conf->to[MAX_VALUE - 1] = '\0';
                                continue;
                        }

                        if((strcmp(attr_name, "version")) == 0)
                        {
                                memcpy(stream_conf->version, attr_value, attr_len);
                                stream_conf->version[16 - 1] = '\0';
                                continue;
                        }

                        if((strcmp(attr_name, "xml:lang")) == 0)
                        {
                                memcpy(stream_conf->xml_lang, attr_value, attr_len);
                                stream_conf->xml_lang[16 - 1] = '\0';
                                continue;
                        }

                        if((strcmp(attr_name, "xmlns")) == 0)
                        {
                                memcpy(stream_conf->xmlns, attr_value, attr_len);
                                stream_conf->xmlns[MAX_VALUE - 1] = '\0';
                                continue;
                        }

                        if((strcmp(attr_name, "xmlns:stream")) == 0)
                        {
                                memcpy(stream_conf->xmlns_stream, attr_value, attr_len);
                                stream_conf->xmlns_stream[MAX_VALUE - 1] = '\0';
                                continue;
                        }

                        return ERROR;
                }
        }
        create_stream_id(stream_conf, MAX_VALUE - 1);
        return 0;
}

void create_stream_id(struct stream_conf *stream_conf, size_t len)
{
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);

        snprintf(stream_conf->id, len, "%s_%ld%09ld", stream_conf->from, ts.tv_sec, ts.tv_nsec);
}

int init_connection(int fd, struct stream_conf *stream_conf)
{
    char stream_header[2048];
    int bytes_write;

    snprintf(stream_header, sizeof(stream_header),
        "<stream:stream "
        "from='%s' "
        "id='%s' "
        "to='%s' "
        "version='%s' "
        "xml:lang='%s' "
        "xmlns='%s' "
        "xmlns:stream='%s'>",
        stream_conf->from, stream_conf->id, stream_conf->to,
        stream_conf->version, stream_conf->xml_lang, stream_conf->xmlns,
        stream_conf->xmlns_stream
    );

    bytes_write = Send(fd, stream_header, strlen(stream_header));

    if(bytes_write < 0)
    {
            connection = false;
            return -1;
    }
    else
    {
            connection = true;
            return 0;
    }
}


