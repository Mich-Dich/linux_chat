
#include "application.h"
#include "network/broadcastagent.h"

#include <signal.h>
#include <getopt.h>
#include <stdio.h>

// DEV-ONLY incudes
#include "network/connectionhandler.h"
#include "core/defines.h"
#include "core/logger.h"

// Server name
static char server_name[32] = "Server_04";
static char admin_name[32] = "Admin";
static in_port_t port = 8111;
static AC_Command_Data All_Admin_Commands_Data[] = {
	{"kick", "/kick", "Kick a user from the chat", AC_Kick},
	{"pause", "/pause", "Pause new incoming messages", AC_Pause},
	{"resume", "/resume", "Resume receiving incoming messages", AC_Resume},
	{"shutdown", "/shutdown", "Disconnect all clients and shutdown the server", AC_Shutdown},
	{"help", "/help", "Display a list of available commands and their usage", AC_Help},
};

void print_help_and_exit(void);
void print_Help(void);
void sigint_Handler(int signum);

// ----------------------------------------------------------------------  Main Functions  ----------------------------------------------------------------------

// standard code to start application
void application_Startup(int argc, char **argv) {

	//Initialize all subsystems
	log_init("main", "[$B$L$X$E $T:$J] [$B$F:$E $G] - $B$C$E$Z", pthread_self(), 1);
	Set_Format_For_Specific_Log_Level(Fatal, "[$B$L$X$E $T:$J] [$B$I: $F:$E $G] - $B$C$E$Z");
	Set_Format_For_Specific_Log_Level(Error, "[$B$L$X$E $T:$J] [$B$I: $F:$E $G] - $B$C$E$Z");
	Set_Format_For_Specific_Log_Level(Warn, "[$B$L$X$E $T:$J] [$B$I: $F:$E $G] - $B$C$E$Z");
    signal(SIGINT, sigint_Handler);		// Registering the signal handler

	if (argc > 1) {

		CL_SEPARATOR()

		int option;
		int option_index = 0;
		static struct option command_arguments[] = {
			{"admin",     	required_argument	, 0, 'a'},
			{"monochrome",	no_argument			, 0, 'm'},
			{"debugLevel",  required_argument	, 0, 'd'},
			{"serverName",  required_argument	, 0, 'n'},
			{"port",    	required_argument	, 0, 'p'},
			{"logFormat",   required_argument	, 0, 'l'},
			{"help",     	no_argument			, 0, 'h'},
			{0, 0, 0, 0}  // Required at the end of the array
		};
		
		u64 opt_buffer = 0;
		// Evaluate command line arguments
		while ((option = getopt_long(argc, argv, "a:d:mn:p:l:h", command_arguments, &option_index)) != -1) {
			switch (option) {
				case 'a': {

					opt_buffer = strlen(optarg);
					CL_VALIDATE((opt_buffer > 0 && opt_buffer < 32), "New [admin name] is valid", "New [admin name] to big [%d/32]", log_shutdown(); exit(EXIT_SUCCESS);, opt_buffer)
					strcpy(admin_name, optarg);
					CL_LOG(Info, "Admin name: %s", admin_name);
					
				} break;

				case 'm': {

					CL_LOG(Info, "monochrome");
					set_Formatting("[$L$X] [$F: $G] - $C$Z");
					
				} break;

				case 'd': {

					opt_buffer = atoi(optarg);
					set_log_level(opt_buffer);
					
				} break;

				case 'n': {

					opt_buffer = strlen(optarg);
					CL_VALIDATE((opt_buffer > 0 && opt_buffer < 32), "New [server name] is valid", "New [server name] to big [%d/32]", log_shutdown(); exit(EXIT_FAILURE);, opt_buffer)
					strcpy(server_name, optarg);
					CL_LOG(Info, "Server name: %s", server_name);
					
				} break;

				case 'p': {

					opt_buffer = atoi(optarg);
					CL_VALIDATE((opt_buffer > 0 && opt_buffer < 65535), "[port: %d] in valid range", "[port: %d] not in valid range (0 < port < 65535)", log_shutdown(); exit(EXIT_FAILURE);, opt_buffer)
					CL_VALIDATE(is_port_available(opt_buffer), "selected port is available", "port not available", log_shutdown(); exit(EXIT_FAILURE);)
					port = opt_buffer;
					CL_LOG(Info, "Port: %d", port);
					
					
				} break;

				case 'l': {

					CL_LOG(Info, "adjusting of log formatting is not implemented yet");
					print_help_and_exit();
					
				} break;

				case '?':
				case 'h': 
				default: {

					print_help_and_exit();
				}
			}
		}	
		
		// Process non-option arguments (if any)
		for (int i = optind; i < argc; i++) {
			CL_LOG(Warn, "Non-option argument: %s\n", argv[i]);
			print_help_and_exit();
		}
	}

	broadcast_agent_init();
	CL_LOG_FUNC_END("Basic SubSystems functional => [server: %s] online", server_name);
	CL_SEPARATOR()
}

// MAIN RUN
void application_Run() {

	CL_LOG_FUNC_START("");
	connection_handler(port);
	CL_LOG_FUNC_END("");
}

// standard code to stop application
int16 application_Shutdown() {

	// perform cleanup
	CL_LOG(Info, "befor cleanup")
	broadcast_agent_cleanup();
	CL_LOG(Info, "after cleanup")
	log_shutdown();
    return EXIT_SUCCESS;
}

// ----------------------------------------------------------------------  Helper Functions  ----------------------------------------------------------------------

void print_help_and_exit(void) {

	set_log_level(5);
	print_Help();
	log_shutdown();
	exit(EXIT_SUCCESS);
}

//
void print_Help(void) {

	set_Formatting("  $C$Z");

	CL_LOG(Trace, "\n")
	CL_LOG(Trace, "Usage:  build/server [-a ADMIN] [-m] [-d LEVEL] [-n SERVERNAME] [-p PORT]")
	CL_LOG(Trace, "\n")
	CL_LOG(Trace, "Shortcut | Parameter   | Description")
	CL_LOG(Trace, "===========================================================================================")
	CL_LOG(Trace, " -a      | ADMIN       | set name of the administrator     (default: Admin)")
	CL_LOG(Trace, " -m      |             | do not use colors for output")
	CL_LOG(Trace, " -d      | LEVEL       | set debug level                   (default: 5)")
	CL_LOG(Trace, "         |             |    1 => FATAL + ERROR")
	CL_LOG(Trace, "         |             |    2 => FATAL + ERROR + WARN")
	CL_LOG(Trace, "         |             |    3 => FATAL + ERROR + WARN + INFO")
	CL_LOG(Trace, "         |             |    4 => FATAL + ERROR + WARN + INFO + DEBUG")
	CL_LOG(Trace, "         |             |    5 => FATAL + ERROR + WARN + INFO + DEBUG + TRACE")
	CL_LOG(Trace, " -n      | SERVNAME    | set server name                   (default: %s)", server_name)
	CL_LOG(Trace, " -p      | PORT        | set TCP port to use               (default: 8111)")
	CL_LOG(Trace, " -l      | LogFormat   | Set the format of Log messages    (NOT IMPLEMENTED YET)")
	CL_LOG(Trace, "         |             |   Formatting the LogMessages can be customized with the following tags")
	CL_LOG(Trace, "         |             |   e.g. $B[$T] $L [$F]  $C$E  or $BTime:[$M $S] $L $E ==> $C")
	CL_LOG(Trace, "         |             |   $T  Time              hh:mm:ss")
	CL_LOG(Trace, "         |             |   $H  Hour              hh")
	CL_LOG(Trace, "         |             |   $M  Minute            mm")
	CL_LOG(Trace, "         |             |   $S  Second           ss")
	CL_LOG(Trace, "         |             |   $J  MilliSecond      mm")
	CL_LOG(Trace, "         |             |   ")
	CL_LOG(Trace, "         |             |   $N  Date              yyyy:mm:dd:")
	CL_LOG(Trace, "         |             |   $Y  Date Year         yyyy")
	CL_LOG(Trace, "         |             |   $O  Date Month        mm")
	CL_LOG(Trace, "         |             |   $D  Date Day          dd")
	CL_LOG(Trace, "         |             |   ")
	CL_LOG(Trace, "         |             |   $F  Func. Name        main, foo")
	CL_LOG(Trace, "         |             |   $A  File Name         C:/Project/main.c C:/Project/foo.c")
	CL_LOG(Trace, "         |             |   $I  short File Name   .c foo.c")
	CL_LOG(Trace, "         |             |   $G  Line              1, 42")
	CL_LOG(Trace, "         |             |   $P  thread id         155642640")
	CL_LOG(Trace, "         |             |   ")
	CL_LOG(Trace, "         |             |   $L  LogLevel          [TRACE], [DEBUG] … [FATAL]")
	CL_LOG(Trace, "         |             |   $X  Alinement         add space for 'INFO' & 'WARN'")
	CL_LOG(Trace, "         |             |   $B  Color Begin       from here the color starts")
	CL_LOG(Trace, "         |             |   $E  Color End         from here the color ends")
	CL_LOG(Trace, "         |             |   $C  Text              Formate Message with variables")
	CL_LOG(Trace, "         |             |   $Z  New Line          Adds a new Line to the log")
	CL_LOG(Trace, "\n")

	use_Formatting_Backup();
}

// Handle Ctrl+C gracefully
void sigint_Handler(int signum) {
	
	UNUSED(signum);
	set_Formatting("$C");
	CL_LOG(Trace, "\b\r")
	use_Formatting_Backup();

	CL_SEPARATOR()
	CL_LOG(Warn, "Detected Shutdown signal: %d", signum)
	CL_LOG(Info, "Shutting down SubSystems");
	connection_handler_shutdown();
}

//
admin_command_code get_server_command(const char* command){

	char first_word[32];
	CL_VALIDATE(sscanf(command, "%31s", first_word) == 1, "   Found first word", "   Unable to find first word", return AC_No_Valid_Command)
	
	size_t array_length = sizeof(All_Admin_Commands_Data) / sizeof(All_Admin_Commands_Data[0]);
	for (u16 x = 0; x < array_length; x++) {

		if (!strcmp(first_word, All_Admin_Commands_Data[x].syntax)) {

			CL_LOG(Debug, "   command: %s", All_Admin_Commands_Data[x].command)
			return All_Admin_Commands_Data[x].code;
		}
		else
			CL_LOG(Trace, "   [command: %s] != %s", first_word, All_Admin_Commands_Data[x].command)
 	}	
	CL_LOG(Debug, "   [command: %s] NOT found", first_word)
	return AC_No_Valid_Command;
}




//
char* get_server_name() { return server_name; }
//
char* get_admin_name() { return admin_name; }
//
in_port_t get_port() { return port; }

AC_Command_Data Get_Admin_Command_Data(u16 index) { return All_Admin_Commands_Data[index]; }

size_t get_length_of_AC_Data(void) { return (sizeof(All_Admin_Commands_Data) / sizeof(All_Admin_Commands_Data[0])); }