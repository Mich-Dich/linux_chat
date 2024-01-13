#pragma once

#include <stdint.h>
#include <time.h>
#include <arpa/inet.h>

#include "../core/defines.h"

typedef enum { 

    MT_LoginRequest = 0,
    MT_LoginResponse = 1,
    MT_ClientToServer = 2,
    MT_ServerToClient = 3,
    MT_UserAdd = 4,
    MT_UserRemove = 5

} message_type;

typedef enum {

	LR_success = 0,
	LR_name_taken = 1,
	LR_name_invalid = 2,
	LR_version_mismatch = 3,
	LR_unknown_server_error = 255

} login_response_code;

typedef enum {

	CS_waiting = 0,
	CS_received_msg = 1,
	CS_Closed_Connection = 2,

} connection_status_code;

typedef enum {

    // Positive => Success
    NV_Received_login = 1,
    NV_Received_Message = 2,
    NV_Received_Server_command = 3,
    
    // negative => Error
    NV_invalid_length = -1,
    NV_Magic_num_invalid = -2,
    NV_Version_invalid = -3,
    NV_Name_length_invalid = -4, 

} network_validation_code;

typedef enum {

	CD_connection_closed_by_client = 0,
    CD_kicked_from_the_server = 1,
	CD_communication_error = 2,

} client_disconnect_code; 

#pragma pack(push, 1)
typedef struct network_message {
	
	u16 len;
	char text[NET_TEXT_LEN];	
	
} network_message;
#pragma pack(pop)

// ------------------------------------- Define a packed structure for the header of all messages -------------------------------------
#pragma pack(push, 1)
typedef struct MessageHeader {

    u8 type;
    u16 length;

} MessageHeader;
#pragma pack(pop)

// ------------------------------------- Define packed structures for each message type -------------------------------------

// Login & Logout Request
#pragma pack(push, 1)
typedef struct Authentication {

    u32 magic;
    u8 version;
    char name[NET_NAME_LEN];

} Authentication;
#pragma pack(pop)

// -------------- Client2Server -------------- 
#pragma pack(push, 1)
typedef struct Client2Server {

    char text[NET_TEXT_LEN];

} Client2Server;
#pragma pack(pop)

// -------------- Server2Client -------------- 
#pragma pack(push, 1)
typedef struct Server2Client {

    u64 timestamp;
    char originalSender[NET_NAME_LEN_PLUS];
    char text[NET_TEXT_LEN];

} Server2Client;
#pragma pack(pop)

// -------------- UserAdded --------------
#pragma pack(push, 1)
typedef struct UserAdded {

    u64 timestamp;
    char name[NET_NAME_LEN];

} UserAdded;
#pragma pack(pop)

// -------------- UserRemoved --------------
#pragma pack(push, 1)
typedef struct UserRemoved {

    u64 timestamp;
    u8 code;
    char name[NET_NAME_LEN];

} UserRemoved;
#pragma pack(pop)

// ------------------------------------- Combine all structs into one packed struct -------------------------------------
#pragma pack(push, 1)
typedef struct CombinedMessage {

    MessageHeader header;
    union {
        Authentication Authentication;
        Client2Server client2Server;
        Server2Client server2Client;
        UserAdded userAdded;
        UserRemoved userRemoved;
    };

} CombinedMessage;
#pragma pack(pop)

//------------------------ Fuctions here ------------------------------------------------
network_validation_code network_Receive(u16 file_desc, CombinedMessage* message);
int16 network_Send(u16 file_desc, CombinedMessage* message);
int16 Check_for_Message(u16 file_desc, CombinedMessage* message);
void build_login_response(CombinedMessage* loc_msg, login_response_code code);
void build_User_Added(CombinedMessage* loc_msg, char name[NET_NAME_LEN_PLUS], u16 size, bool8 use_time_stamp);
void build_User_Removed(CombinedMessage* loc_msg, client_disconnect_code Code, char name[NET_NAME_LEN_PLUS], u16 size);
void build_Server_to_client(CombinedMessage* loc_msg, u16 msg_length, char originalSender[NET_NAME_LEN_PLUS], u16 name_size);
void build_Server_response(CombinedMessage* loc_msg, const char* text, ...);
void build_Server_response_Special(CombinedMessage* loc_msg, const char* name, const char* text, ...);
