cmake_minimum_required(VERSION 3.5)
project(gb-emu)

set(BIN_DIR ${gb-emu_SOURCE_DIR}/bin)

if (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU" OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -std=c++11")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG} -g")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE} -O2")
elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
	if (CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
		string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
	endif()
endif()

find_package(SDL2 REQUIRED)

include_directories(${SDL2_INCLUDE_DIRS} ${gb-emu_SOURCE_DIR}/src/include)
add_executable(gb-emu src/main.cpp src/cpu.cpp src/bootloader.cpp src/ppu.cpp src/gui.cpp)
TARGET_LINK_LIBRARIES(gb-emu ${SDL2_LIBRARIES} ${SDL2IMAGE_LIBRARIES})
target_compile_features(gb-emu PRIVATE cxx_std_17)
install(TARGETS gb-emu RUNTIME DESTINATION ${BIN_DIR})