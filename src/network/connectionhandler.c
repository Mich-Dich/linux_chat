#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>

#include "core/util.h"
#include "core/logger.h"
#include "connectionhandler.h"
#include "user.h"
#include "clientthread.h"

static volatile bool8 accept_Connections = TRUE;
static int sock_server = -1;
static const int on = 1;


// static function to create socked (sockets start as passive)
// ErrorCode: [-1]: socket not crated [-2]: socket not bound [-3]: socket not listening
static int create_socket(in_port_t port) {

	CL_LOG_FUNC_START("port: %d", port);

	// AF_INET => IPv4, SOCK_STREAM => TCP, 0 => Standard protocol
	const int server_socket = socket(AF_INET, SOCK_STREAM, 0);
	const int backlog = 10;

	CL_VALIDATE(server_socket >= 0, "   [server socket] created", "Failed to create [server socket] [error: %d]", return -1);
	
	CL_VALIDATE(setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) >= 0, "[server socket] options are set", "unable to set options for [server socket] %s",
		close(server_socket); return -2, ERROR_STR)

	struct sockaddr_in address;
	memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_port = htons(port);
	address.sin_addr.s_addr = htonl(INADDR_ANY);

	CL_VALIDATE(bind(server_socket, (const struct sockaddr*) &address, (socklen_t) sizeof(struct sockaddr_in)) >= 0, "   [server socket] bound", "[server socket] not bound: %s", return -3, ERROR_STR)
	CL_VALIDATE(listen(server_socket, backlog) >= 0, "", "[server socket] not listening: %s", return -4, ERROR_STR)

	CL_LOG_FUNC_END("[server socket: %d] listening", server_socket)
	return server_socket;
}

// create socket on [port] and accept connections
void connection_handler(in_port_t port) {

	CL_LOG_FUNC_START("")


	const int server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (server_socket == -1) {
		perror("Socket creation failed");
		exit(EXIT_FAILURE);
	}
	struct sockaddr_in server_address;
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = INADDR_ANY;  // Use any available IP address of the machine
	server_address.sin_port = htons(8111);  // Convert port to network byte order

	if (bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == -1) {
		perror("Binding failed");
		exit(EXIT_FAILURE);
	}

	// [SOMAXCONN] maximum number of pending connections in listen-queue
	if (listen(server_socket, SOMAXCONN) == -1) {
		perror("Listening failed");
		exit(EXIT_FAILURE);
	}

	while (TRUE) {

		struct sockaddr_in client_address;
		socklen_t client_address_len = sizeof(client_address);

		int sock_client = accept(server_socket, (struct sockaddr*)&client_address, &client_address_len);
		if (sock_client == -1) {
			perror("Acceptance failed");
			break;
		}

		CL_SEPARATOR()
		CL_LOG(Debug, "   trying to accept connection from client on socket: %d", sock_client)
		pthread_t t_id;

		CL_VALIDATE(!pthread_create(&t_id, NULL, client_Thread, (void*)sock_client), "   Tread created successfully", "pthread_create() => %s",
				close(sock_client); continue; ,
				ERROR_STR)

		CL_LOG(Info, "   Client on [socket: %d] was connected successfully", sock_client)
		CL_SEPARATOR()

	}

	if (FALSE) {

		close(server_socket);
	}


/*
	sock_server = create_socket(port);
	CL_VALIDATE(sock_server > 0, "   created [server socket: %d]", "Unable to create [server socket] => [sock: %d] [error: %s]", return, sock_server, ERROR_STR)

	// wait to accept incoming connection
	CL_LOG(Debug, "   Waiting for connection")
	struct sockaddr incoming_addr;
	socklen_t incoming_addr_len = sizeof(incoming_addr);
	int64 sock_client = 0;
	while(TRUE)	{
		
		sock_client = (int64)accept(sock_server , &incoming_addr , &incoming_addr_len);
		if (sock_client == -1) {

			CL_LOG(Error, "Error accepting connection => %s", ERROR_STR);
			break;
		}

		CL_SEPARATOR()
		CL_LOG(Debug, "   trying to accept connection from client on socket: %d", sock_client)
		pthread_t t_id;

		CL_VALIDATE(!pthread_create(&t_id, NULL, client_Thread, (void*)sock_client), "   Tread created successfully", "pthread_create() => %s",
				close(sock_client); continue; ,
				ERROR_STR)

		CL_LOG(Info, "   Client on [socket: %d] was connected successfully", sock_client)
		CL_SEPARATOR()
	}

	return;
	*/
}

// Close the server socket
void connection_handler_shutdown() {

    CL_LOG(Trace, "shutdown")
    close(sock_server);
}

// attempts to create and then destroy a socket on given port
bool8 is_port_available(int port) {

	CL_LOG_FUNC_START("port: %d", port);
	bool8 result = TRUE;

	// AF_INET => IPv4, SOCK_STREAM => TCP, 0 => Standard protocol
	const int test_socket = socket(AF_INET, SOCK_STREAM | O_NONBLOCK, 0);
	CL_VALIDATE(test_socket >= 0, "   [test socket] created", "socket() => %s", return FALSE, ERROR_STR);

	struct sockaddr_in address = {
		.sin_family = AF_INET,
		.sin_port = htons(port),
		.sin_addr.s_addr = htonl(INADDR_ANY),
	};

	// Bind socket
	CL_VALIDATE(bind(test_socket, (const struct sockaddr*) &address, (socklen_t) sizeof(address)) >= 0, "   [test socket] bound", "bind() => %s", 
		close(test_socket); return FALSE, 
		ERROR_STR)
    
	CL_LOG_FUNC_END("")
    close(test_socket);

	return result;
}