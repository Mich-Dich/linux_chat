#pragma once 

#include <arpa/inet.h>

#include "core/defines.h"

typedef enum admin_command_code{
    AC_Kick = 0,
    AC_Pause,
    AC_Resume,
    AC_Shutdown,
    AC_Help,
    AC_No_Valid_Command,
    AC_MAX
} admin_command_code;

typedef struct AC_Command_Data {
    const char* command;
    const char* syntax;
    const char* description;
    admin_command_code code;
} AC_Command_Data;


void application_Startup(int argc, char **argv);
void application_Run();
int16 application_Shutdown();


char* get_server_name();
char* get_admin_name();
in_port_t get_port();

admin_command_code get_server_command(const char* command);
AC_Command_Data Get_Admin_Command_Data(u16 index);
size_t get_length_of_AC_Data(void);

// Loops over all an array that holds all [AC_Command_Data]
// variable in loop is[current_AC_Data]
#define ITERATE_OVER_ADMIN_COMMANDS(LoopCommand)	do {size_t array_length = get_length_of_AC_Data();                                  \
                                                        for (u16 x = 0; x < array_length; x++) {                                        \
                                                            AC_Command_Data current_AC_Data = Get_Admin_Command_Data(x);                \
                                                            LoopCommand                                                                 \
                                                        }} while(0);				                                                    \

