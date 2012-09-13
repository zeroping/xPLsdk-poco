#Specify the version being used aswell as the language
cmake_minimum_required(VERSION 2.6)
#Name your project here
#project(xplsdk)
#set (xplsdk_VERSION_MAJOR 0)
#set (xplsdk_VERSION_MAJOR 1)


# configure a header file to pass some of the CMake settings
# to the source code
#configure_file (
#  "${PROJECT_SOURCE_DIR}/projconfig.h.in"
#  "${PROJECT_BINARY_DIR}/projconfig.h"
#  )

# add the binary tree to the search path for include files
# so that we will find TutorialConfig.h
#include_directories("${PROJECT_BINARY_DIR}")
#include_directories("${PROJECT_SOURCE_DIR}")


## Target
#set(TEST_SRCS test/ConsoleApp.cpp )
#add_executable(xplsdk xplComms.cpp xplDevice.cpp xplMsg.cpp xplStringUtils.cpp  xplConfigItem.cpp xplFilter.cpp xplMsgItem.cpp xplUDP.cpp test/ConsoleApp.cpp)
## disabled - RegUtils.cpp XMLUtils.cpp

add_library(xplsdk  xplComms.cpp xplDevice.cpp xplMsg.cpp xplStringUtils.cpp  xplConfigItem.cpp xplFilter.cpp xplMsgItem.cpp xplUDP.cpp)

## Link libraries
find_library(POCO_FOUNDATION PocoFoundation)
find_library(POCO_NET PocoNet)
target_link_libraries(xplsdk ${POCO_FOUNDATION} ${POCO_NET})

find_package(Threads REQUIRED)
target_link_libraries(xplsdk ${CMAKE_THREAD_LIBS_INIT})







#Sends the -std=c99 flag to the gcc compiler
#add_definitions(-std=c99)

