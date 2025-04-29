#include "main.h"
#include <sys/epoll.h>
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

#ifdef XML_LARGE_SIZE
#  define XML_FMT_INT_MOD "ll"
#else
#  define XML_FMT_INT_MOD "l"
#endif

#ifdef XML_UNICODE_WCHAR_T
#  define XML_FMT_STR "ls"
#else
#  define XML_FMT_STR "s"
#endif

// TODO: handle error on connection lost
// TODO: Implement TLS before XMPP
// TODO: Implement SASL before XMPP
// TODO: Implement error Handling
// TODO: Implement schema validation

int server_fd;
int epoll_fd;
Pool pool = {0};
Cnode *clients_list;

// Prototypes
int init_xmpp(int fd, struct XMPP_Client *client);
void send_xml_response(struct XMPP_Client *client);
void create_stream_id(struct XMPP_Client *client, size_t len);
void disconnect_client(int fd, struct XMPP_Client *client);
size_t get_response_len(const char *s, enum tag_type tag_type, struct XMPP_Client *client);
int hdl_xml_element(struct XMPP_Client *client, const XML_Char *el_name, const XML_Char **atts);
void hdl_xmpp_data(struct XMPP_Client *client);
int init_client(int client_fd, struct XMPP_Client *client);

static void XMLCALL text_element(void *userData, const XML_Char *string, int len)
{
	struct XMPP_Client *client = (struct XMPP_Client*) userData;
	struct XMPP_Message *message = &client->message;
	if(message->tag_type == BODY)
	{
		if(client->message.body_len + len < MAX_BODY)
		{
			memcpy(message->body + message->body_len, string, len);
			message->body_len += len;
		}

	}
}

static void XMLCALL start_element(void *userData, const XML_Char *name, const XML_Char **atts)
{
	struct XMPP_Client *client = (struct XMPP_Client*) userData;
	struct XMPP_Message *message = &client->message;
	message->depth++;

	if(strcmp(name, "message") == 0)
		message->tag_type = MESSAGE;
	
	else if(strcmp(name, "stream:stream") == 0)
		message->tag_type = STREAM;
	
	else if(strcmp(name, "body") == 0)
		message->tag_type = BODY;

	hdl_xml_element(client, name, atts);
}


static void XMLCALL end_element(void *userData, const XML_Char *name)
{
	struct XMPP_Client *client = (struct XMPP_Client*) userData;
	client->message.depth--;
	
	if(strcmp("message", name) == 0)
		client->message.tag_type = MESSAGE;
	
	else if(strcmp("stream", name) == 0)
		client->message.tag_type = STREAM;
	
	else
	client->message.tag_type = NONE;
}

int main(void)
{
	prctl(PR_SET_NAME, "xmpp_server", 0, 0, 0);
	struct sockaddr_in server_addr;

	memset(&server_addr, 0, sizeof(server_addr));
	Signal(SIGINT, sig_exit);

	server_fd = Socket(AF_INET, SOCK_STREAM, 0);

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT);
	server_addr.sin_addr.s_addr = INADDR_ANY;

	Bind(server_fd, (struct sockaddr *) &server_addr, sizeof(server_addr));

	Listen(server_fd, BACK_LOG);
	printf("Server listening on port %d\n", PORT);

	// Create epoll
	struct epoll_event ev, events[MAX_CLIENTS];

	epoll_fd = EpollCreate(0);
	ev.events = EPOLLIN;
	ev.data.fd = server_fd;

	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev) == -1)
	err_sys("epoll_error: listen_sock");

	while(1)
	{
		int n;
		n = epoll_wait(epoll_fd, events, MAX_CLIENTS, -1 );

		for (int i = 0; i < n; i++)
		{
			// TODO: Change data.fd for data.ptr for the server
			if(events[i].data.fd == server_fd && (events[i].events & EPOLLIN))
			{
				struct XMPP_Client *client;
				int client_fd;
				int r;
				
				client = NULL;
				
				client_fd = Accept(server_fd, NULL, NULL);
				if(client_fd < 0) continue;
				
				r = init_client(client_fd, client);
				if(r == -1)
					break;
				
				
				// Todo: Handle initial Stream
				hdl_xmpp_data(client);
				printf("Client connected successfully\n");

				client->status = true;

				struct epoll_event client_ev = {0};

				client_ev.events = EPOLLIN | EPOLLERR | EPOLLHUP;
				client_ev.data.ptr = client;
				epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &client_ev);
			}

			else if(events[i].events & EPOLLIN)
			{
				struct XMPP_Client* client = (struct XMPP_Client*)events[i].data.ptr;
				hdl_xmpp_data(client);
			}

			else if(events[i].events & (EPOLLERR | EPOLLHUP))
			{
				struct XMPP_Client *client = (struct XMPP_Client*)events[i].data.ptr;
				disconnect_client(epoll_fd, client);
			}
		}
	}

	return 0;
}

void hdl_xmpp_data(struct XMPP_Client *client)
{
	ssize_t bytes_read;
	char *stack_memory;
	uint64_t stack_len;
	uint64_t stack_size;
	int error;

	stack_memory = client->arena->stack_memory;
	stack_len = ArenaGetPos(client->arena);
	stack_size = ArenaMaxSize();

	if(stack_len >= stack_size)
	{
		ArenaClear(client->arena);
		stack_len = ArenaGetPos(client->arena);
	}
	bytes_read = recv(client->fd, stack_memory + stack_len, stack_size - stack_len, 0);
	if(bytes_read > 0)
	{
		stack_memory = ArenaPush(client->arena, bytes_read);
		if (XML_Parse(client->parser, stack_memory, (int)bytes_read, 0) == XML_STATUS_ERROR)
		{
			err_buf(
			"Parse error at line %" XML_FMT_INT_MOD "u:\n%" XML_FMT_STR "\n",
			XML_GetCurrentLineNumber(client->parser),
			XML_ErrorString(XML_GetErrorCode(client->parser)));

			disconnect_client(epoll_fd, client);
		}
		
		error =  XML_GetErrorCode(client->parser);
		
		if(error == XML_ERROR_NONE)
		send_xml_response(client);
	}
	else
	{
		disconnect_client(epoll_fd, client);
	}
}

int hdl_xml_element(struct XMPP_Client *client, const XML_Char *el_name, const XML_Char **atts)
{
	(void)el_name;
	enum tag_type flag;
	struct XMPP_Client *xmpp_client;
	struct XMPP_Message *xmpp_message;

	xmpp_client = client;
	xmpp_message = &client->message;
	flag = xmpp_message->tag_type;

	// handle attributes
	for(int i = 0; atts[i] && (flag == MESSAGE || flag == STREAM); i+=2)
	{
		const char *attr_name = atts[i];
		const char *attr_value = atts[i+1];

		int attr_len = strlen(attr_value);

		if(attr_len > MAX_VALUE - 1)
		continue;

		if((strcmp(attr_name, "from")) == 0)
		{
			if(flag == MESSAGE)
			{
				memcpy(xmpp_message->from, attr_value, attr_len);
				xmpp_message->from[attr_len] = '\0';
			}
			else if(flag == STREAM)
			{
				memcpy(xmpp_client->from, attr_value, attr_len);
				xmpp_client->from[attr_len] = '\0';
			}
		}

		else if((strcmp(attr_name, "to")) == 0)
		{
			if(flag == MESSAGE)
			{
				memcpy(xmpp_message->to, attr_value, attr_len);
				xmpp_message->to[attr_len] = '\0';
			}
			else if(flag == STREAM)
			{
				memcpy(xmpp_client->to, attr_value, attr_len);
				xmpp_client->to[attr_len] = '\0';
			}
		}

	}

	return 0;
}

void disconnect_client(int fd, struct XMPP_Client *client)
{
	err_conn("Client %d disconnected\n", client->fd);

	epoll_ctl(fd, EPOLL_CTL_DEL, client->fd, NULL);
	Close(client->fd);
	XML_ParseBuffer(client->parser, 0, 1);
	XML_ParserFree(client->parser);
	ArenaRelease(client->arena, &pool);
	free(client);

}

void send_xml_response(struct XMPP_Client *client)
{
	char *message;
	int bytes_send;
	size_t len;
	
	client->message.body[client->message.body_len] = '\0';
	bytes_send = 0;
	
	if(client->message.depth != 0)
	return;

	const char message_schema[] =
	"<message from='%s' to='%s' type='%s' xml:lang='en'> <body>%s</body> </message>";

	const char stream_schema[] =
	"<stream:stream from='%s' to='%s'>";

	switch(client->message.tag_type)
	{
		case MESSAGE:
			len = get_response_len(message_schema, MESSAGE, client);
			message = malloc(len);

			snprintf(message, len, message_schema,
			client->message.from,
			client->message.to,
			client->message.msg_type,
			client->message.body);
			break;
		case STREAM:
			len = get_response_len(stream_schema, STREAM, client);
			message = malloc(len);

			snprintf(message, len, stream_schema,
			client->message.from,
			client->message.to);
			break;
		default:
			return;
		}
		
		bytes_send = Send(client->fd, message, len);
		free(message);
		
		if(bytes_send > 0)
		{
			ArenaClear(client->arena);
			client->message.body_len = 0;
		}
	}

	size_t get_response_len(const char *s, enum tag_type tag_type, struct XMPP_Client *client)
	{
		int message_offset;
		int stream_offset;

	stream_offset = 4;
	message_offset = 6;

	switch(tag_type)
	{
		case MESSAGE:
			return strlen(s) + strlen(client->message.from) +
			strlen(client->message.to) + strlen(client->message.msg_type) + client->message.body_len - message_offset;

		case STREAM:
			return strlen(s) + strlen(client->from) + strlen(client->to) - stream_offset;
		default:
			return 0;
	}
}

int init_client(int client_fd, struct XMPP_Client *client)
{

	client = (struct XMPP_Client*)Calloc(1, sizeof(struct XMPP_Client));

	client->fd = client_fd;
	XML_Parser parser = XML_ParserCreate(NULL);
	if (!parser)
	{
		err_buf("Couldn't allocate memory for parser\n");
		return -1;
	}
	client->parser = parser;

	XML_SetElementHandler(client->parser, start_element, end_element);
	XML_SetCharacterDataHandler(client->parser, text_element);
	XML_SetUserData(client->parser, client);
	client->arena = ArenaAlloc(&pool);

	ArenaPush(clients);
	
	return 0;
}

void create_stream_id(struct XMPP_Client *client, size_t len)
{
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);

	snprintf(client->id, len, "%s_%ld%09ld", client->from, ts.tv_sec, ts.tv_nsec);
}
