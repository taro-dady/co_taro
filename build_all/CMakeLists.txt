
PROJECT (co_taro)
PROJECT(assembler C ASM)

if(CMAKE_CL_64)
	INCLUDE_DIRECTORIES( ${OPENSSL}/Win64/include )
else(CMAKE_CL_64)
	INCLUDE_DIRECTORIES( ${OPENSSL}/Win32/include )
endif(CMAKE_CL_64)

IF (CMAKE_SYSTEM_NAME MATCHES "Windows")
	file(GLOB SRC_H ../include/base/*.h ../include/base/system/*.h ../include/base/memory/*.h ../include/base/function/*.h ../include/base/utils/*.h)
	file(GLOB SRC_CPP 
	    ../src/base/*.cc ../src/base/windows/*.cc 
		../src/log/*.cc 
		../src/co_routine/*.cc ../src/co_routine/windows/* 
	    ../src/net/*cc ../src/net/open_ssl/*cc ../src/net/windows/*.cc)
	ADD_DEFINITIONS(-D_WINSOCK_DEPRECATED_NO_WARNINGS -DWIN32_LEAN_AND_MEAN)
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "Linux")
	file(GLOB SRC_CPP ../src/base/*.cc ../src/base/linux/* 
	    ../src/co_routine/*.cc ../src/co_routine/linux/* 
		../src/log/*cc ../src/net/*cc ../src/net/open_ssl/*.cc ../src/net/linux/*.cc ../src/log/*.cc ../src/co_routine/*.cc ../src/co_routine/linux/*)
ENDIF (CMAKE_SYSTEM_NAME MATCHES "Windows")

IF (MSVC)
	IF (CMAKE_CL_64)
		list (APPEND SRC_ASM
			"../src/co_routine/windows/asm/jump_x86_64_ms_pe_masm.asm"
			"../src/co_routine/windows/asm/make_x86_64_ms_pe_masm.asm"
		)
	ELSE (CMAKE_CL_64)
		list (APPEND SRC_ASM
			"../src/co_routine/windows/asm/jump_i386_ms_pe_masm.asm"
			"../src/co_routine/windows/asm/make_i386_ms_pe_masm.asm"
		)
	ENDIF(CMAKE_CL_64)

	SET(MASMFound FALSE)
	ENABLE_LANGUAGE(ASM_MASM)
	IF(CMAKE_ASM_MASM_COMPILER_WORKS)
		SET(MASMFound TRUE)
	ELSE()
		# This could happen if the CMakeCache.txt was deleted; 
		#try finding the ASM_MASM compiler again
		INCLUDE(CMakeDetermineASM_MASMCompiler)
    	INCLUDE(CMakeTestASM_MASMCompiler)
    	IF(NOT CMAKE_ASM_MASM_COMPILER_WORKS)
			MESSAGE(FATAL_ERROR "enable  ASM_MASM failed")
		ENDIF()
	ENDIF(CMAKE_ASM_MASM_COMPILER_WORKS)
ENDIF (MSVC)

IF (CMAKE_SYSTEM_NAME MATCHES "Linux")
	ADD_LIBRARY(co_taro SHARED ${SRC_CPP} ${SRC_H} ${SRC_ASM})
	TARGET_LINK_LIBRARIES( co_taro yaml-cpp ssl )
ELSE (CMAKE_SYSTEM_NAME MATCHES "Linux")
	ADD_DEFINITIONS(-DTARO_USE_DLL)
	ADD_LIBRARY(co_taro SHARED ${SRC_CPP} ${SRC_H} ${SRC_ASM})
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SAFESEH:NO")
	if(CMAKE_CL_64)
		TARGET_LINK_LIBRARIES( co_taro yaml-cpp ${OPENSSL}/Win64/lib/libssl.lib ${OPENSSL}/Win64/lib/libcrypto.lib )
	else(CMAKE_CL_64)
		TARGET_LINK_LIBRARIES( co_taro yaml-cpp ${OPENSSL}/Win32/lib/libssl.lib ${OPENSSL}/Win32/lib/libcrypto.lib )
	endif(CMAKE_CL_64)
ENDIF (CMAKE_SYSTEM_NAME MATCHES "Linux")
