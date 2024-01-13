#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "network.h"
#include "core/logger.h"
#include "application.h"

void Print_message_Info(CombinedMessage* message);
uint64_t htonll(uint64_t host_longlong);
uint64_t ntohll(uint64_t net_longlong);

// receive [messages] from [connection :file_desc]
network_validation_code network_Receive(u16 file_desc, CombinedMessage* message) {

	// Validate length	
	u64 msg_len = ntohs(message->header.length);
	CL_LOG_FUNC_START("socket: %d, length: %d", file_desc, msg_len);

	CL_VALIDATE(msg_len > 0, "", "[msg_len] <= 0", return NV_invalid_length)
	
	network_validation_code return_val = 0;	
	ssize_t bytes_Read = 0;
	switch (message->header.type) {
		
		case MT_LoginRequest: {

			return_val = NV_Received_login;
			bytes_Read = read(file_desc, &message->Authentication, msg_len);
			CL_VALIDATE(ntohl(message->Authentication.magic) == 0x0badf00d, "   [Magic] valid", "   [Magic] invalid 0x%08x", return_val = NV_Magic_num_invalid, ntohl(message->Authentication.magic))
			CL_VALIDATE(message->Authentication.version == 0, "   [Version] valid", "   [Version] invalid", return_val = NV_Version_invalid)
			CL_VALIDATE(bytes_Read > 5 && bytes_Read < 37, "   [Name length] valid", "   [Name length] invalid - [bytes_Read: %d]", return_val = NV_Name_length_invalid, bytes_Read)			
			break;
		} 

		case MT_ClientToServer: {

			bytes_Read = read(file_desc, &message->client2Server.text, msg_len);
			CL_VALIDATE(bytes_Read > 0 && bytes_Read < NET_TEXT_LEN, "   [text length] valid", "   [text length] invalid - [bytes_Read: %d]", return_val = NV_Name_length_invalid, bytes_Read)

			if (message->client2Server.text[0] == '/')
				return_val = NV_Received_Server_command;
			else
				return_val = NV_Received_Message;			
			
			break;
		}
		
		default: {
	
			CL_LOG(Error, "Server received invalid message type - [type: %d]", message->header.type)
			break;
		}
	}
	CL_LOG_FUNC_END("");
	return return_val;
}

// send [message] to [file_desc]
int16 network_Send(u16 file_desc, CombinedMessage* message) {

	CL_LOG_FUNC_START("Sending to Socked: %d", file_desc);

	Print_message_Info(message);
	CL_VALIDATE(send(file_desc, message, sizeof(message->header) + ntohs(message->header.length), 0) >= 0, "", "send() => %s" , return FAILURE, ERROR_STR)

	CL_LOG_FUNC_END("");
	return SUCCESS;
}

int16 Check_for_Message(u16 file_desc, CombinedMessage* message) {

	return (int16)recv(file_desc, &message->header, sizeof(message->header), 0);
}

//
void build_login_response(CombinedMessage* loc_msg, login_response_code code) {

	CL_LOG_FUNC_START("")
	memset(loc_msg, 0, sizeof(CombinedMessage));
	
	loc_msg->header.type = MT_LoginResponse;
	loc_msg->Authentication.magic = htonl(0xc001c001);
	loc_msg->Authentication.version = code;
	strcpy(loc_msg->Authentication.name, get_server_name());
	u16 nameLength = strlen(get_server_name());

	loc_msg->header.length = htons(5 + nameLength);
	Print_message_Info(loc_msg);
	CL_LOG_FUNC_END("")
}

//
void build_User_Added(CombinedMessage* loc_msg, char name[NET_NAME_LEN_PLUS], u16 size, bool8 use_time_stamp) {

	CL_LOG_FUNC_START("[size: %d] [Use time: %s]", size, bool_To_String(use_time_stamp))
	loc_msg->header.type = MT_UserAdd;

    time_t timestamp = time(NULL);
	CL_VALIDATE(timestamp != -1, "   [timestamp] valid", "   [time] %s", ,ERROR_STR)

	loc_msg->userAdded.timestamp = (use_time_stamp && timestamp != -1) ? htonll(timestamp) : 0;
	CL_LOG(Trace, "   [name size: %d]", size)
	strncpy(loc_msg->userAdded.name, name, size);

	loc_msg->header.length = htons(8 + strlen(loc_msg->userAdded.name));
	Print_message_Info(loc_msg);
	CL_LOG_FUNC_END("")
}

//
void build_User_Removed(CombinedMessage* loc_msg, client_disconnect_code Code, char name[NET_NAME_LEN_PLUS], u16 size) {

	CL_LOG_FUNC_START("")
	loc_msg->header.type = MT_UserRemove;

    time_t timestamp = time(NULL);
	CL_VALIDATE(timestamp != -1, "   [timestamp] valid", "   [time] %s", ,ERROR_STR)

	loc_msg->userRemoved.timestamp = (timestamp != -1) ? htonll(timestamp) : 0;
	loc_msg->userRemoved.code = Code;
	CL_LOG(Trace, "   [size: %d]", size)
	strncpy(loc_msg->userRemoved.name, name, size);

	loc_msg->header.length = htons(9 + size);
	Print_message_Info(loc_msg);
	CL_LOG_FUNC_END("")
}

//
void build_Server_to_client(CombinedMessage* loc_msg, u16 msg_length, char originalSender[NET_NAME_LEN_PLUS], u16 name_size) {

	CL_LOG_FUNC_START("")
	loc_msg->header.type = MT_ServerToClient;

	char loc_text[NET_TEXT_LEN];
	memcpy(loc_text, loc_msg->client2Server.text, sizeof(loc_text));
	memset(loc_msg->client2Server.text, 0, sizeof(loc_msg->client2Server.text));
	time_t timestamp = time(NULL);
	CL_VALIDATE(timestamp != -1, "   [timestamp] valid", "   [time] %s", ,ERROR_STR)

	CL_LOG(Trace, "   [name_size: %d]", name_size)

	loc_msg->server2Client.timestamp = (timestamp != -1) ? htonll(timestamp) : 0;
	strncpy(loc_msg->server2Client.originalSender, originalSender, name_size);
	strncpy(loc_msg->server2Client.text, loc_text, msg_length);

	loc_msg->header.length = htons(40 + msg_length);
	Print_message_Info(loc_msg);
	CL_LOG_FUNC_END("")
}

//
void build_Server_response(CombinedMessage* loc_msg, const char* text, ...) {

	CL_LOG_FUNC_START("")
	loc_msg->header.type = MT_ServerToClient;

	time_t timestamp = time(NULL);
	CL_VALIDATE(timestamp != -1, "   [timestamp] valid", "   [time] %s", ,ERROR_STR)

    // write all arguments in to [message_formatted]
    char text_formatted[NET_TEXT_LEN];
        memset(text_formatted, 0, sizeof(text_formatted));
    __builtin_va_list args_ptr;
    va_start(args_ptr, text);
        vsnprintf(text_formatted, NET_TEXT_LEN, text, args_ptr);
    va_end(args_ptr);

	loc_msg->server2Client.timestamp = (timestamp != -1) ? htonll(timestamp) : 0;
	strncpy(loc_msg->server2Client.originalSender, get_server_name(), strlen(get_server_name()));
	strncpy(loc_msg->server2Client.text, text_formatted, strlen(text_formatted));

	loc_msg->header.length = htons(40 + strlen(text_formatted));
	Print_message_Info(loc_msg);
	CL_LOG_FUNC_END("")
}

//
void build_Server_response_Special(CombinedMessage* loc_msg, const char* name, const char* text, ...) {

	CL_LOG_FUNC_START("")
	loc_msg->header.type = MT_ServerToClient;

	time_t timestamp = time(NULL);
	CL_VALIDATE(timestamp != -1, "   [timestamp] valid", "   [time] %s", ,ERROR_STR)

    // write all arguments in to [message_formatted]
    char text_formatted[NET_TEXT_LEN];
        memset(text_formatted, 0, sizeof(text_formatted));
    __builtin_va_list args_ptr;
    va_start(args_ptr, text);
        vsnprintf(text_formatted, NET_TEXT_LEN, text, args_ptr);
    va_end(args_ptr);

	loc_msg->server2Client.timestamp = (timestamp != -1) ? htonll(timestamp) : 0;
	strncpy(loc_msg->server2Client.originalSender, name, strlen(name));
	strncpy(loc_msg->server2Client.text, text_formatted, strlen(text_formatted));

	loc_msg->header.length = htons(40 + strlen(text_formatted));
	Print_message_Info(loc_msg);
	CL_LOG_FUNC_END("")
}

// ----------------------------------------------------------------------------  private messages  ----------------------------------------------------------------------------

// convert a u64 from host to network byte-order
uint64_t htonll(uint64_t host_longlong) {

    // Check the endianness of the system
    union {
        uint64_t ull;
        uint8_t  c[8];
    } test = {0x0102030405060708ULL};

    if (test.c[0] == 0x01) 
        return host_longlong;	// Big-endian system
    else 
        return ((uint64_t)htonl(host_longlong & 0xFFFFFFFF) << 32) | htonl(host_longlong >> 32);	// Little-endian system, swap bytes
    
}

// convert a u64 from network to host byte-order
uint64_t ntohll(uint64_t net_longlong) {

    // Check the endianness of the system
    union {
        uint64_t ull;
        uint8_t  c[8];
    } test = {0x0102030405060708ULL};

    if (test.c[0] == 0x01) 
        return net_longlong;		// Big-endian system
	else
        return ((uint64_t)ntohl(net_longlong & 0xFFFFFFFF) << 32) | ntohl(net_longlong >> 32);	// Little-endian system, swap bytes
}


void Print_message_Info(CombinedMessage* message) {

	const char* types[] = {"MT_LoginRequest", "MT_LoginResponse", "MT_ClientToServer", "MT_ServerToClient", "MT_UserAdd", "MT_UserRemove"};

	switch(message->header.type) {

		case MT_LoginRequest: {

			CL_LOG(Trace, "\n     [type: %d => %s]\n     [length: %d]\n     [magic: 0x%08x]\n     [version: %d]\n     [name: %s]", 
				message->header.type, types[message->header.type], ntohs(message->header.length),
				message->Authentication.magic, message->Authentication.version, message->Authentication.name)
			break;
		}

		case MT_LoginResponse: {

			CL_LOG(Trace, "\n     [type: %d => %s]\n     [length: 5 + %d] \n     [magic: 0x%08x]\n     [version: %d]\n     [name: %s]", 
				message->header.type, types[message->header.type], ntohs(message->header.length) - 5,
				message->Authentication.magic, message->Authentication.version, message->Authentication.name)
			break;
		}

		case MT_ClientToServer: {

			CL_LOG(Trace, "\n     [type: %d => %s]\n     [length: %d]\n     [text: %s]", 
				message->header.type, types[message->header.type], ntohs(message->header.length),
				message->client2Server.text)
			break;
		}

		case MT_ServerToClient: {

			CL_LOG(Trace, "\n     [type: %d => %s]\n     [length: 40 + %d]\n     [time: %lld]\n     [originalSender: %s]\n     [text: %s]", 
				message->header.type, types[message->header.type], ntohs(message->header.length) - 40,
				ntohll(message->server2Client.timestamp), message->server2Client.originalSender, message->server2Client.text)
			break;
		}

		case MT_UserAdd: {

			CL_LOG(Trace, "\n     [type: %d => %s]\n     [length: 8 + %d]\n     [time: %lld]\n     [name: %s]", 
				message->header.type, types[message->header.type], ntohs(message->header.length) - 8,
				ntohll(message->server2Client.timestamp), message->userAdded.name)
			break;
		}		

		case MT_UserRemove: {

			CL_LOG(Trace, "\n     [type: %d => %s]\n     [length: %d]\n     [time: %lld]\n     [code: %d]\n     [name: %s]", 
				message->header.type, types[message->header.type], ntohs(message->header.length),
				ntohll(message->server2Client.timestamp), message->userRemoved.code, message->userRemoved.name)
			break;		
		}

		default: {

			break;
		}
	}
}