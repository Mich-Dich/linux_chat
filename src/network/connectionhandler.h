#pragma once

#include <netinet/in.h>

#include "core/defines.h"

void connection_handler(in_port_t port);
void connection_handler_shutdown();

bool8 is_port_available(int port);