#pragma once

#include "core/defines.h"
#include "network.h"
#include "user.h"


bool8 Is_broadcast_paused(void);

int16 broadcast_agent_init(void);
void broadcast_agent_cleanup(void);
bool8 broadcast_message(CombinedMessage msg, User* User_to_Ignore);

void pause_broadcast (void);
void resume_broadcast (void);