
#include <stdio.h>
#include <errno.h>

#include "application.h"

int main(int argc, char **argv) {
	
	application_Startup(argc, argv);

	application_Run();

	int16 result = application_Shutdown();
	return result ? EXIT_SUCCESS : EXIT_FAILURE;
}
