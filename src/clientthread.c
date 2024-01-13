#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "user.h"
#include "network/network.h"
#include "core/logger.h"
#include "clientthread.h"
#include "network/broadcastagent.h"
#include "application.h"

User* self = 0;
int16 connection_status = 0;
CombinedMessage* msg;
u16* msg_length;
client_disconnect_code disconnect_reason = CD_communication_error;


bool8 f_Name_has_forbidden_char(char name[NET_NAME_LEN]);

void* client_Thread(void* arg) {

	CL_LOG_FUNC_START("Thread-ID: %ld", pthread_self());

	self = create_User(pthread_self(), (u16)(uintptr_t)arg);
	
	msg = malloc(sizeof(CombinedMessage));
	memset(msg, 0, sizeof(CombinedMessage));

	msg_length = malloc(sizeof(u16));
	memset(msg_length, 0, sizeof(u16));
	
	disconnect_reason = CD_communication_error;

	login_response_code response_Code = LR_unknown_server_error;
	network_validation_code ValidationCode;

	while (TRUE) {
		
		CL_LOG(Error, "Dumb shit");
		memset(msg, 0, sizeof(CombinedMessage));

		connection_status = Check_for_Message(self->sock, &msg->header);
		if (connection_status <= 0) {
			break;
		}

		CL_SEPARATOR()
		*msg_length = ntohs(msg->header.length);
		CL_LOG(Info, "   [Client: %ld] received a message [length: %d]", self->sock, *msg_length)
		
		ValidationCode = network_Receive(self->sock, msg);
		CL_LOG(Info, "network_Receive() finished")
		CL_VALIDATE(ValidationCode >= 0, "network_Receive was a success [Code: %d]", "network_Receive FAILED [Code: %d]", , ValidationCode)

		switch (ValidationCode) {

			case NV_Received_login: {
				
				response_Code = LR_success;
				char login_name[NET_NAME_LEN] = "";
				strncpy(login_name, msg->Authentication.name, *msg_length - 5);
				
				// ---------------  Validate info of Users ---------------
				CL_VALIDATE(is_name_free(login_name), "   Name free", "   Name already taken", response_Code = LR_name_taken)
				CL_VALIDATE(!f_Name_has_forbidden_char(login_name), "   Name has only valid charater", "   Name has invalid charater", response_Code = LR_name_invalid)
					
				CL_LOG(Info, "   make login response [response_Code: %d]", response_Code)
				build_login_response(msg, response_Code);
				network_Send(self->sock, msg);
				memset(msg, 0, sizeof(CombinedMessage));
				
				if (response_Code == LR_success) {

					login_User(self, login_name, *msg_length - 5) ? LR_success : LR_unknown_server_error;
					register_thread_log_under_Name(pthread_self(), get_User_name(self));
					memset(msg, 0, sizeof(CombinedMessage));
				}
				else {
					// user_should_shutdown(self);
					break;
				}
				
				// ---------------  Inform all user about new user ---------------
				CL_LOG(Info, "make [User added] for group [name: %s/%d]", get_User_name(self), get_User_name_length(self))
				build_User_Added(msg, get_User_name(self), get_User_name_length(self), TRUE);
				broadcast_message(*msg, NULL);
				memset(msg, 0, sizeof(CombinedMessage));

				// ---------------  Inform about old Users ---------------
				CL_LOG(Info, "make [User added] for new user...")
				ITERATE_OVER_USERS	(	if(target != self && is_User_LoggedIn(target)) {

											build_User_Added(msg, get_User_name(target), get_User_name_length(target), FALSE);
											network_Send(self->sock, msg);
											memset(msg, 0, sizeof(CombinedMessage));
										}
									)
				break;
			}
			
			// -----------------------------------------  broadcast messages  -----------------------------------------
			case NV_Received_Message: {

				CL_LOG(Info, "   Received message")
				build_Server_to_client(msg, *msg_length, get_User_name(self), get_User_name_length(self));
				CL_VALIDATE(broadcast_message(*msg, NULL), "", "   failed to send message to broadcast queue",
					build_Server_response(msg, "Message Queue is sadly full, wait for admin to resume chat"); network_Send(self->sock, msg);)
				break;
			}

			// -----------------------------------------  handle Server-Command  -----------------------------------------
			case NV_Received_Server_command: {

				CL_VALIDATE(!strcmp(get_User_name(self), get_admin_name()), "   User is admin", "   User is not admin",
					build_Server_response(msg, "Caution! You dont have required permissions for [server_commands]");
					network_Send(self->sock, msg);
					break;
				)

				admin_command_code admin_code = get_server_command(msg->client2Server.text);
				switch (admin_code) {

					case AC_Kick: {

						char name_to_kick[NET_NAME_LEN];
						if(sscanf(msg->client2Server.text, "%*s %31[^\n]", name_to_kick) == 1)
							CL_LOG(Trace, "   Name: %s", name_to_kick)
						
						bool8 found_name = FALSE;
						ITERATE_OVER_USERS (	if(!strcmp(get_User_name(target), name_to_kick) && is_User_LoggedIn(target)) {
													close(target->sock);
													user_should_shutdown(target);
													found_name = TRUE;
													break;
												}
											)
						CL_VALIDATE(found_name, "   [%s] successfully kicked", "   [name: %s] is not in user list",
							build_Server_response(msg, "[name: %s] is not in user list", name_to_kick); network_Send(self->sock, msg); , 
							name_to_kick)
							
					} break;

					case AC_Pause: {

						CL_VALIDATE(!Is_broadcast_paused(), "   Pausing Server", "   Server is already paused", 
							build_Server_response(msg, "Server is already paused"); network_Send(self->sock, msg); break;)

						pause_broadcast();
						build_Server_response(msg, "admin has paused the server");
						ITERATE_OVER_USERS	(	if(is_User_LoggedIn(target))
														network_Send(target->sock, msg); 
											)

					} break;

					case AC_Resume: {
						
						CL_VALIDATE(Is_broadcast_paused(), "   Resuming Server", "   Server is NOT paused", 
							build_Server_response(msg, "Server is not paused, [/resume] makes no sense"); network_Send(self->sock, msg); break;)

						build_Server_response(msg, "admin has resumed the server");
						ITERATE_OVER_USERS	(	if(is_User_LoggedIn(target))
														network_Send(target->sock, msg); 
											)
						resume_broadcast();

					} break;

					case AC_Help: {

						ITERATE_OVER_ADMIN_COMMANDS	(	build_Server_response_Special(msg, current_AC_Data.syntax, current_AC_Data.description);
														network_Send(self->sock, msg);
														memset(msg, 0, sizeof(CombinedMessage));
													)
					} break;

					case AC_No_Valid_Command:
					default: {

						build_Server_response(msg, "Caution! detected invalid [server_commands] %s", msg->client2Server.text);
						network_Send(self->sock, msg);
						
					} break;
				}
				break;
			}

			case NV_invalid_length:
			case NV_Magic_num_invalid:
			case NV_Version_invalid:
			case NV_Name_length_invalid: {

				user_should_shutdown(self);
			} break;

			default:
				break;
		}
		
		CL_SEPARATOR()
	}


	logout_user(self);
	CL_LOG(Info, "   [Client: %d] disconnected", self->sock)
	
	if (connection_status == CS_Closed_Connection)
		disconnect_reason = CD_connection_closed_by_client;
	else if(should_User_shutdown(self)){

		disconnect_reason = CD_kicked_from_the_server;
	}					

	if (get_User_name_length(self) != 0) {
		// Notify all clients that the thread closed
		build_User_Removed(msg, disconnect_reason, get_User_name(self), get_User_name_length(self));
		ITERATE_OVER_USERS	(	if(is_User_LoggedIn(target) && (target != self)) 
									network_Send(target->sock, msg); 
							)
	}
	
	close(self->sock);
	remove_User(self->sock);
	free(msg);
	free(msg_length);

	CL_LOG_FUNC_END("");
	CL_SEPARATOR()
	return NULL;
}


//
bool8 f_Name_has_forbidden_char(char name[NET_NAME_LEN]) {

	CL_LOG_FUNC_START("");

	bool8 is_valid = FALSE;
	const u16 loc_name_len = strlen(name);

	for(u16 x = 0; x < loc_name_len; x++)	{

		if(name[x] < 33 || name[x] == 34 || name[x] == 39 || name[x] == 96 || name[x] >= 127) {

			CL_LOG(Error, "%c", name[x])
			//use_Formatting_Backup();
			is_valid = TRUE;
			break;
		}
	}

	CL_LOG_FUNC_END("");
	return is_valid;
}