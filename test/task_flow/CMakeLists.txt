PROJECT (task_flow_test)

IF (CMAKE_SYSTEM_NAME MATCHES "Windows")
	ADD_DEFINITIONS(-D_WINSOCK_DEPRECATED_NO_WARNINGS -DWIN32_LEAN_AND_MEAN)
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "Linux")
ENDIF (CMAKE_SYSTEM_NAME MATCHES "Windows")

file(GLOB_RECURSE SRC_CPP *.cc)

ADD_EXECUTABLE(task_flow_test ${SRC_CPP})

IF (CMAKE_SYSTEM_NAME MATCHES "Windows")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SAFESEH:NO")
	TARGET_LINK_LIBRARIES( task_flow_test co_taro )
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "Linux")
	if(LINK_THREAD MATCHES "FALSE")
		message("not link thread")
		TARGET_LINK_LIBRARIES( task_flow_test co_taro rt dl )
	else()
		message("link thread")
		TARGET_LINK_LIBRARIES( task_flow_test co_taro rt pthread dl )
	endif()
ENDIF (CMAKE_SYSTEM_NAME MATCHES "Windows")