#include <pthread.h>
#include <mqueue.h>

#include "core/util.h"
#include "core/logger.h"
#include "broadcastagent.h"
#include "network/network.h"

#include <stdio.h>
#include <semaphore.h>


typedef struct queue_message {
	User* sender;
	CombinedMessage msg;
} queue_message;

static pthread_t threadId = 0;
static const char* queue_name = "/broadcast_queue";
static mqd_t broadcast_queue;
static volatile bool8 running = TRUE;
static u64 Max_msg_size = sizeof(queue_message);
static struct mq_attr attr;
static bool8 broadcast_Paused = FALSE;

static sem_t broadcast_sem;
pthread_mutex_t broadcast_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t broadcast_resume_cond = PTHREAD_COND_INITIALIZER;


//
static void *broadcast_agent(void *arg) {

	CL_LOG_FUNC_START("");
	register_thread_log_under_Name(pthread_self(), "broadcast_agent");

    queue_message buffer;
    ssize_t bytes_read;
    while (TRUE) {
		
		bytes_read = mq_receive(broadcast_queue, (char*)&buffer, Max_msg_size, NULL);
		sem_wait(&broadcast_sem);

		CL_VALIDATE(bytes_read >= 0, "   received a message", "mq_receive() => %s", running = FALSE, ERROR_STR)
		ITERATE_OVER_USERS(		CL_LOG(Trace, "   current target: %d", target->sock)
								if(target != buffer.sender && is_User_LoggedIn(target)) 
									network_Send(target->sock, &buffer.msg);
		)

		sem_post(&broadcast_sem);

    }

	CL_LOG_FUNC_END("");
	return arg;
}

//
int16 broadcast_agent_init(void) {
	
	CL_LOG_FUNC_START("");
	
    // initialize the queue attributes
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = Max_msg_size;
    attr.mq_curmsgs = 0;
	
    // create the message queue
   	broadcast_queue = mq_open(queue_name, O_CREAT | O_RDWR, 0644, &attr);
	CL_VALIDATE((mqd_t)-1 != broadcast_queue, "   [broadcast_queue] created", "mq_open() => %s", return -1, ERROR_STR)

	//TODO: start thread
	CL_VALIDATE(!pthread_create(&threadId, NULL, broadcast_agent, (void*)NULL), "   Tread created successfully", "pthread_create() => %s", return -1, ERROR_STR)
	sem_init(&broadcast_sem, 0, 1);

	CL_LOG_FUNC_END("");
	return SUCCESS;
}

//
void broadcast_agent_cleanup(void) {

	CL_LOG_FUNC_START("Shutting down [broadcast agent]");

	//resume_broadcast();
	//pthread_join(threadId, NULL);

    // cleanup message queue
	CL_VALIDATE((mqd_t)-1 != mq_close(broadcast_queue), "", "mq_close() => %s", return, ERROR_STR)
	CL_VALIDATE((mqd_t)-1 != mq_unlink(queue_name), "", "mq_unlink() => %s", return, ERROR_STR)

	CL_LOG_FUNC_END("");
}

// send the message(msg) to [broadcast_queue] 
bool8 broadcast_message(CombinedMessage msg, User* User_to_Ignore) {

	queue_message loc_msg = {User_to_Ignore, msg};
	mq_getattr(broadcast_queue, &attr);
	CL_VALIDATE(attr.mq_curmsgs < attr.mq_maxmsg, "   [broadcast_queue] not full", "[broadcast_queue] full", return FALSE)
	CL_VALIDATE(0 <= mq_send(broadcast_queue, (const char*)&loc_msg, sizeof(queue_message), 0), "Message send to [broadcast_queue]", "mq_send() %s", return FALSE, ERROR_STR)
	return TRUE;
}

//
void pause_broadcast (void) {

	CL_LOG_FUNC_START("");
	sem_wait(&broadcast_sem);
	broadcast_Paused = TRUE;
	CL_LOG_FUNC_END("");
}

//
void resume_broadcast (void) {

	CL_LOG_FUNC_START("");
	sem_post(&broadcast_sem);
	broadcast_Paused = FALSE;
	CL_LOG_FUNC_END("");
}

bool8 Is_broadcast_paused(void) {
	
	return broadcast_Paused;
}