CMAKE_MINIMUM_REQUIRED(VERSION 3.1)
PROJECT(chat C)

SET(CMAKE_C_STANDARD 99)
SET(CMAKE_C_STANDARD_REQUIRED ON)

IF("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror=implicit")
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror=return-type")
	SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Og -ggdb")
ELSEIF("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang")
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror=implicit")
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror=return-type")
	SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -ggdb")
ENDIF()

SET(CMAKE_THREAD_PREFER_PTHREAD TRUE)
SET(THREADS_PREFER_PTHREAD_FLAG TRUE)
FIND_PACKAGE(Threads REQUIRED)

FIND_LIBRARY(LIBRT rt)

SET(SERVER_MODULES	src/network/broadcastagent.c
			src/network/connectionhandler.c
			src/network/network.c
			src/core/util.c
			src/core/logger.c
			src/main.c
			src/user.c
			src/clientthread.c
			src/application.c)

INCLUDE_DIRECTORIES(src)

ADD_EXECUTABLE(server ${SERVER_MODULES})
IF(LIBRT)
	TARGET_LINK_LIBRARIES(server Threads::Threads ${LIBRT})
ELSE()
	TARGET_LINK_LIBRARIES(server Threads::Threads)
ENDIF()
