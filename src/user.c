#include <stdlib.h>

#include "core/logger.h"
#include "clientthread.h"
#include "user.h"

static pthread_mutex_t userLock = PTHREAD_MUTEX_INITIALIZER;
static User *userFront = NULL;
static User *userBack = NULL;

User* f_find_User(u16 sock);

void lock_user_mutex(void)      { pthread_mutex_lock(&userLock); }
void unlock_user_mutex(void)    { pthread_mutex_unlock(&userLock); }

//
User* create_User(pthread_t threadID, u16 sock) {

    CL_LOG_FUNC_START("thread: %ld, socket: %d", threadID, sock);

    // Safeguard / multiple declarations
    CL_VALIDATE(!f_find_User(sock), "socket unique", "Socked already in use", return NULL)
    pthread_mutex_lock(&userLock);

    User *newUser = malloc(sizeof(User));
    CL_VALIDATE(newUser != NULL, "", "   malloc() => %s", return NULL, ERROR_STR)
    memset(newUser, 0, sizeof(User));
    newUser->thread = threadID;
    newUser->sock = sock;

    // list is Empty
    if (userFront == NULL) {

        userFront = newUser;
        userBack = newUser;
        CL_LOG(Trace, "   Added to list - was empty");
    }

    // Add to List-End
    else {

        userBack->next = newUser;
        newUser->prev = userBack;
        userBack = newUser;
        CL_LOG(Trace, "   Added to list - at end");
    }

    pthread_mutex_unlock(&userLock);

    CL_LOG_FUNC_END("");
    return newUser;
}

// removes user from doubly linked list, if found
bool8 remove_User(u16 sock) {

    CL_LOG_FUNC_START("");

    // Find User
    User *locPointer = f_find_User(sock);
    CL_VALIDATE(locPointer != NULL, "Deleting User", "User NOT found", return FALSE)

    // locPointer is found User
    pthread_mutex_lock(&userLock);

    // List has only one element
    if (userFront == userBack) {

        userFront = NULL;
        userBack = NULL;
        CL_LOG(Trace, "   front == back => only one element");
    }

    // List has more than one element
    else {

        // Is First?
        if (locPointer == userFront){

            userFront = userFront->next;
            userFront->prev = NULL;
        }

        // Is Last?
        else if (locPointer == userBack) {

            userBack = userBack->prev;
            userBack->next = NULL;
        }

        // is in middle
        else {

            locPointer->prev->next = locPointer->next;
            locPointer->next->prev = locPointer->prev;
        }
    }

    free(locPointer);
    pthread_mutex_unlock(&userLock);

    CL_LOG_FUNC_END("User found and deleted");
    return TRUE;
}


// Iterate through list and try to find user by socket // Returns NULL if not found
User* f_find_User(u16 sock) {

    CL_LOG_FUNC_START("");

    pthread_mutex_lock(&userLock);
    bool8 locFound = FALSE;
    User *locPointer = userFront;
    int32 locIndex = -1;
    while (locFound == FALSE && locPointer != NULL) {

        if (locPointer->sock == sock)
            locFound = TRUE;
        else
            locPointer = locPointer->next;

        locIndex++;
    }

    if (locFound)
        CL_LOG_FUNC_END("[User socket: %d] found at [index: %d]", sock, locIndex)
    else
        CL_LOG_FUNC_END("[User socket: %d] NOT found", sock)

    pthread_mutex_unlock(&userLock);
    return locFound ? locPointer : NULL;
}

bool8 is_name_free(char name[NET_NAME_LEN]) {

    CL_LOG_FUNC_START("[name: %s]", name);

    // Safeguard / empty list
    User *locPointer = userFront;
    CL_VALIDATE(locPointer != NULL, "   found valid list, starting iteration", "   List is Empty ==> ABORT", return TRUE)

    // Iterate over list with break
    pthread_mutex_lock(&userLock);
    while (locPointer != NULL) {

        CL_LOG(Trace, "   User [socket: %d] [name: %s]", locPointer->sock, locPointer->name);

        if (!strcmp(name, locPointer->name) && is_User_LoggedIn(locPointer)) {

            CL_LOG(Debug, "   Name already exits")
            pthread_mutex_unlock(&userLock);
            return FALSE;
        }

        locPointer = locPointer->next;
    }

    pthread_mutex_unlock(&userLock);
    CL_LOG_FUNC_END("");
    return TRUE;
}

//
bool8 login_User(User *target, char name[NET_NAME_LEN], u16 size) {

    CL_LOG_FUNC_START(" Size: %d", size);
    pthread_mutex_lock(&userLock);

    strncpy(target->name, name, size);
    target->logged_in = TRUE;
    target->name_length = size;

    pthread_mutex_unlock(&userLock);
    CL_LOG_FUNC_END("");
    return TRUE;
}

void user_should_shutdown(User *target) {

    target->should_shutdown = TRUE;
}

//
User *get_First_User(void) {

    return userFront;
}





User* iterate_User(User* target) {

    return target->next; 
}
    
char* get_User_name(User* target) {
    
    return target->name;
}

u16 get_User_name_length(User* target) {
    
    return target->name_length;
}

bool8 is_User_LoggedIn(User* target) {

    return target->logged_in; 
}

bool8 should_User_shutdown(User* target) {
    
    return target->should_shutdown;
}

void logout_user(User* target) {
    
    target->logged_in = FALSE;
}
