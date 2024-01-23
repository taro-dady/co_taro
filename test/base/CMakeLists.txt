PROJECT (base_test)

file(GLOB_RECURSE SRC_CPP *.cc)

ADD_EXECUTABLE(base_test ${SRC_CPP})
TARGET_LINK_LIBRARIES( base_test rt pthread dl )
