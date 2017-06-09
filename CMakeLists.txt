cmake_minimum_required(VERSION 2.8.12)


cmake_policy(SET CMP0048 NEW)

project(AquetiTools VERSION "0.0.0.1" )
############################################################
# Configuration Step
############################################################
#set default build location if a project has not been defined. After the
#project below, the PROJECT_SOURCE_DIR should be defined
#set(CMAKE_INSTALL_PREFIX "$PROJECT_DEST" CACHE PATH "Install Directory" FORCE )

#if(NOT DEFINED PROJECT_SOURCE_DIR )
#endif()

#set( CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/INSTALL" CACHE PATH "Install Directory")
#set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/INSTALL" CACHE PATH "Install Directory" FORCE )

#General build options
option( MAKE_STATIC_LIB "Compile the library statically (off for dynamic)" ON )
option( MAKE_TESTS "Build test executables" ON )
option( USE_CLANG "Use clang for compiler instead of gcc" ON )
option( USE_SUPERBUILD "Build all dependencies in SUPERBUILD mode" ON)

#Determine if we have the cpack program for building packages
#Specify compiler options
if( USE_CLANG)
   if(NOT WIN32)
      #Make these cache variable so they persist if we are not using Superbuild
      set(CMAKE_C_COMPILER "/usr/bin/clang" CACHE STRING "Compiler to use" )
      set(CMAKE_CXX_COMPILER "/usr/bin/clang++" CACHE STRING "CXX Compiler to use")

      #See if the AddressSanitizer(ASAN) is installed. If so, make it an option
      find_program(ASAN asan_symbolize DOC "ASAN executable")
      string( COMPARE EQUAL ${ASAN} "ASAN-NOTFOUND" ASAN_NOTFOUND )
      if(NOT ASAN_NOTFOUND )
         option(USE_ASAN "Enable Address sanitizer" OFF)
         if(USE_ASAN)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-omit-frame-pointer -fsanitize=address -fno-optimize-sibling-calls -DVOGL_USE_STB_MALLOC=0")
            set(CMAKE_CXX_FLAGS_LIST "${CMAKE_CXX_FLAGS_LIST}  -fno-omit-frame-pointer -fsanitize=address -fno-optimize-sibling-calls -DVOGL_USE_STB_MALLOC=0")
            set(CMAKE_EXE_LINK_FLAGS_LIST "${CMAKE_EXE_LINK_FLAGS_LIST} -fsanitize=address -fno-omit-frame-pointer")
            set(CMAKE_SHARED_LINK_FLAGS_LIST "${CMAKE_SHARED_LINK_FLAGS_LIST} -fsanitize=address -fno-omit-frame-pointer")
         endif(USE_ASAN)
      endif(NOT ASAN_NOTFOUND)
   endif(NOT WIN32)
endif(USE_CLANG)

##############################################
## Build dependencies
##############################################
##Superbuild comes first. If USE_SUPERBUILD is set or not defined, only this following conditional wil
##be executed in this pass. The superbuild file will recursively call this CMakeLists.txt
##with USE_SUPERBUILD set to off to skip the SUPERBUILD part
if(USE_SUPERBUILD )
   set(GIT_HTTPS ON CACHE BOOL "Clone git repositories with HTTPS")
   include(ExternalProject)
   include(Superbuild.cmake)
   return()
endif(USE_SUPERBUILD)


#############################################
# Generate the revision.h file to integrate version
# information.
#############################################
configure_file( 
   "${CMAKE_SOURCE_DIR}/revision.h.in"
   "${CMAKE_INSTALL_PREFIX}/include/aquetitools/revision.h"
)

message("------------------------------INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}")
link_directories(${CMAKE_INSTALL_PREFIX}/lib)
include_directories(
   ${CMAKE_INSTALL_PREFIX}/include 
   ${CMAKE_CURRENT_SOURCE_DIR}/include
)


#include( ${CMAKE_INSTALL_PREFIX}/include)
#If Doxygen is installed, make it an option. Default to OFF so it's not built
#as a SubProject of a superbuild
find_package( Doxygen)
if( DOXYGEN_FOUND )
   option( USE_DOXYGEN "Build Documentation" OFF )
   if( USE_DOXYGEN)
       set( DOXYGEN_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/Documentation" CACHE PATH "Directory to install documentation")
   endif( USE_DOXYGEN)
endif( DOXYGEN_FOUND )

#############################################################
# From here on is the build component (SUPERBUILD is disabled)
############################################################
#############################################
# Determine build information. Version specs
# are specified at the top of the file
#############################################
# Get the current working branch
execute_process(
  COMMAND git rev-parse --abbrev-ref HEAD
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_BRANCH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Get the latest abbreviated commit hash of the working branch
execute_process(
  COMMAND git log -1 --format=%h
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_COMMIT_HASH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Get the date
execute_process(
   COMMAND date +%F
   WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
   OUTPUT_VARIABLE BUILD_DATE 
   OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Get the date
execute_process(
   COMMAND date +%T
   WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
   OUTPUT_VARIABLE BUILD_TIME
   OUTPUT_STRIP_TRAILING_WHITESPACE
)


#set the version tag
if( "${GIT_BRANCH}" MATCHES "master" )
   set(VERSION_TAG "dev")
else()
   set(VERSION_TAG "${GIT_BRANCH}")
endif()


include_directories( ${CMAKE_CURRENT_BINARY_DIR})

#If we use clang, set up parameters
#############################################
#Specify local compiler options
#############################################
#Determine Compiler options
if (NOT WIN32)
   set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra -Wno-unused-parameter -std=c++14 -fPIC -O0") #Compile faster on debug, with warnings
   set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -std=c++14 -fPIC") #Optimize compilation, no warnings
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++14 -fPIC -O2") #Else slightly optimize, with warnings

   add_definitions(-DUNIX)
endif(NOT WIN32)

list( APPEND ATOOL_HEADERS 
   Timer/Timer.h
)

include_directories( Timer )
set(Timer_SRC
   Timer/Timer.cpp
)

include_directories( Thread )
set(Thread_SRC
   Thread/Thread.cpp
   Thread/MultiThread.cpp
   Thread/ThreadPool.cpp
   Thread/Thread.cpp
   Thread/Thread.cpp
   Thread/Thread.cpp
)

list( APPEND ATOOL_HEADERS
   Thread/Thread.h
   Thread/MultiThread.h
   Thread/ThreadPool.h
   Thread/TaskManager.tcc
)

include_directories( DataTypes )
set( DataTypes_SRC
   DataTypes/LruCache.cpp
   DataTypes/TSMap.cpp
   DataTypes/TSQueue.cpp
)
list( APPEND ATOOL_HEADERS
   DataTypes/LruCache.tcc
   DataTypes/TSMap.tcc
   DataTypes/TSQueue.tcc
)


include_directories( Mutex )
set( Mutex_SRC
   Mutex/shared_mutex.cpp
)
list(APPEND ATOOL_HEADERS
   Mutex/shared_mutex.h
)

add_library( aquetitools STATIC  
   ${Timer_SRC}
   ${Thread_SRC}
   ${Mutex_SRC}
)

target_link_libraries( aquetitools
   libJsonBox.a
   pthread
)

LIST( APPEND LIB_LIST aquetitools )

if(MAKE_TESTS)
   add_executable( AquetiToolTest 
      test/AquetiToolTest.cpp
   )

   target_link_libraries( AquetiToolTest ${LIB_LIST})
   LIST( APPEND TARGET_LIST AquetiToolTest )
endif()

list(APPEND TARGET_LIST ${LIB_LIST})

#############################################
#install library files
# This sections initiates the build of the  components in th TARGET_LIST. 
# Each component
#############################################
message("installing: ${TARGET_LIST}")
install( TARGETS 
   ${TARGET_LIST}
#   ${LIB_LIST}
   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib${LIB_SUFFIX}
   ARCHIVE DESTINATION lib${LIB_SUFFIX}
   INCLUDES DESTINATION include/aqueti
   PUBLIC_HEADER DESTINATION include/aqueti
)

install( FILES ${ATOOL_HEADERS} DESTINATION include/aquetitools)

