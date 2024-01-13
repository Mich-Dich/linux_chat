#pragma once

#include "core/defines.h"

#include <pthread.h>

// Function prototype
typedef void (*func)(void);

typedef struct User {
	struct User *prev;
	struct User *next;
	pthread_t thread;	//thread ID of the client thread
	u16 sock;			//socket for client
	char name[NET_NAME_LEN_PLUS];
	u16 name_length;
	bool8 logged_in;
	bool8 should_shutdown;
} User;


User* iterate_User(User* target);
char* get_User_name(User* target);
u16 get_User_name_length(User* target);
bool8 is_User_LoggedIn(User* target);
bool8 should_User_shutdown(User* target);
void logout_user(User* target);

User* create_User(pthread_t threadID, u16 sock);
bool8 remove_User(u16 sock);
bool8 login_User(User* target, char name[NET_NAME_LEN], u16 size);
void user_should_shutdown(User* target);

bool8 is_name_free(char name[NET_NAME_LEN]);
User* get_First_User(void);   
void lock_user_mutex(void);
void unlock_user_mutex(void);

#define ITERATE_OVER_USERS(LoopCommand)		do {lock_user_mutex();										\
												CL_LOG(Trace, "Mutex locked")							\
												User* target = get_First_User();						\
												while (target != NULL) {								\
													LoopCommand											\
													target = iterate_User(target);						\
												}														\
												unlock_user_mutex();									\
												CL_LOG(Trace, "Mutex unlocked")} while(0);				\

