cmake_minimum_required(VERSION 2.8.12)

cmake_policy(SET CMP0048 NEW)

project(acl VERSION "0.0.0.1" )
############################################################
# Configuration Step
############################################################

#General build options
option( BUILD_STATIC_LIB "Compile the library statically (off for dynamic)" ON )
option( USE_SUPERBUILD "Build all dependencies in SUPERBUILD mode" ON)

# Doxygen support
# add a target to generate API documentation with Doxygen
find_package(Doxygen)
if(DOXYGEN_FOUND)
    set(DOXYGEN_DIR "${CMAKE_BINARY_DIR}/Doxygen" CACHE STRING "Where to build Doxygen")
    file(MAKE_DIRECTORY ${DOXYGEN_DIR})
    configure_file(${CMAKE_SOURCE_DIR}/Doxyfile.in ${DOXYGEN_DIR}/Doxyfile @ONLY)
    add_custom_target(doc
        ${DOXYGEN_EXECUTABLE} ${DOXYGEN_DIR}/Doxyfile
        WORKING_DIRECTORY ${DOXYGEN_DIR}
        COMMENT "Generating API documentation with Doxygen" VERBATIM
    )
endif(DOXYGEN_FOUND)

#We need a git executable so we can determine versions
find_package(Git REQUIRED)

##############################################
## Build dependencies
##############################################
##Superbuild comes first. If USE_SUPERBUILD is set or not defined, only this following conditional wil
##be executed in this pass. The superbuild file will recursively call this CMakeLists.txt
##with USE_SUPERBUILD set to off to skip the SUPERBUILD part
if(USE_SUPERBUILD )
   include(ExternalProject)
   include(Superbuild.cmake)
   return()
endif(USE_SUPERBUILD)

list(APPEND CMAKE_PREFIX_PATH
    ${CMAKE_INSTALL_PREFIX}/lib/cmake
    usr/lib/cmake
)

include_directories(
   ${CMAKE_INSTALL_PREFIX}/include 
   ${CMAKE_CURRENT_SOURCE_DIR}/include
   ${CMAKE_CURRENT_BINARY_DIR}
)

#############################################################
# From here on is the build component (SUPERBUILD is disabled)
############################################################
#############################################
# Determine build information. Version specs
# are specified at the top of the file
#############################################
# Get the current working branch
execute_process(
  COMMAND ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_BRANCH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Get the latest abbreviated commit hash of the working branch
execute_process(
  COMMAND ${GIT_EXECUTABLE} log -1 --format=%h
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_COMMIT_HASH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
if(NOT ${GIT_COMMIT_HASH} MATCHES "${GIT_COMMIT_HASH_PREV}")
    set(CONFIGURE ON)
endif()
set(GIT_COMMIT_HASH_PREV ${GIT_COMMIT_HASH} CACHE STRING "Git Commit Hash" FORCE)

# Get the date
execute_process(
   COMMAND date +%F
   WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
   OUTPUT_VARIABLE BUILD_DATE 
   OUTPUT_STRIP_TRAILING_WHITESPACE
)
if(NOT ${BUILD_DATE} MATCHES "${BUILD_DATE_PREV}")
    set(CONFIGURE ON)
endif()
set(BUILD_DATE_PREV ${BUILD_DATE} CACHE STRING "Build Date" FORCE)

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
if(NOT ${GIT_BRANCH} MATCHES "${GIT_BRANCH_PREV}")
    set(CONFIGURE ON)
endif()
set(GIT_BRANCH_PREV ${GIT_BRANCH} CACHE STRING "Git Branch" FORCE)

if(CONFIGURE OR NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/revision.h)
    configure_file( 
        "${CMAKE_SOURCE_DIR}/revision.h.in"
        "revision.h"
    )
endif()

#############################################
#Specify local compiler options
#############################################
#Determine Compiler options
if (NOT WIN32)
   set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra -Wno-unused-function -Wno-unused-parameter -std=c++11 -fPIC") #Compile faster on debug, with warnings
   set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -std=c++11 -fPIC") #Optimize compilation, no warnings
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-unused-function -std=c++11 -fPIC") #Else slightly optimize, with warnings

   add_definitions(-DUNIX)
endif(NOT WIN32)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

list(APPEND CMAKE_PREFIX_PATH
    ${CMAKE_INSTALL_PREFIX}/lib/cmake
    usr/lib/cmake
)

# If we're debugging, link in helgrind
string(TOLOWER "${CMAKE_BUILD_TYPE}" type_lower)
if(type_lower MATCHES "debug")
    find_package(Helgrind)
    if(Helgrind_FOUND)
        add_definitions(-DUSE_HELGRIND)
        include_directories(${Helgrind_INCLUDE_DIR})
    endif(Helgrind_FOUND)
endif(type_lower MATCHES "debug")

find_package(Threads REQUIRED)

list( APPEND ATOOL_HEADERS 
    Timer/Timer.h
)

include_directories( Timer )
set(Timer_SRC
   Timer/Timer.cpp
)

include_directories( Sockets )
set(Sockets_SRC
   Sockets/CoreSocket.cpp
)
list( APPEND ATOOL_HEADERS
   Sockets/CoreSocket.hpp
)

include_directories( Thread )
set(Thread_SRC
   Thread/Thread.cpp
   Thread/MultiThread.cpp
   Thread/ThreadPool.cpp
   Thread/Thread.cpp
   Thread/Thread.cpp
   Thread/Thread.cpp
   Thread/ThreadWorker.cpp
)

list( APPEND ATOOL_HEADERS
   Thread/Thread.h
   Thread/ThreadWorker.h
   Thread/MultiThread.h
   Thread/ThreadPool.h
   Thread/TaskManager.tcc
)

include_directories( DataStructures )
set( DataStructures_SRC
)
list( APPEND ATOOL_HEADERS
   DataStructures/LruCache.tcc
   DataStructures/TSMap.tcc
   DataStructures/TSQueue.tcc
)

include_directories( Math ) 
list(APPEND ATOOL_HEADERS
    Math/polyfit.tcc
    Math/givensQR.hpp
    Math/matrix.hpp
    )

include_directories( Tools ) 
set( Tools_SRC
   Tools/StringTools.cpp
)
list( APPEND ATOOL_HEADERS 
   Tools/StringTools.h
)

include_directories( Mutex )
set( Mutex_SRC
   Mutex/shared_mutex.cpp
)
list(APPEND ATOOL_HEADERS
   Mutex/shared_mutex.h
   Mutex/AclMutexWrap.h
)


include_directories( RandomIDGenerator )
set( RandomIDGenerator_SRC
    RandomIDGenerator/RandomIDGenerator.cpp
)
list( APPEND ATOOL_HEADERS
    RandomIDGenerator/RandomIDGenerator.hpp
)

include_directories( FileIO )
set( FileIO_SRC
   FileIO/FileIO.cpp
)
list( APPEND ATOOL_HEADERS
   FileIO/FileIO.h
)



add_library( acl STATIC  
   ${Timer_SRC}
   ${Thread_SRC}
   ${Mutex_SRC}
   ${DataStructures_SRC}
   ${Tools_SRC}
   ${RandomIDGenerator_SRC}
   ${FileIO_SRC}
   ${Sockets_SRC}
)

target_link_libraries( acl
    Threads::Threads
)

list(APPEND TARGET_LIST acl )


#############################################
#install library files
# This sections initiates the build of the  components in th TARGET_LIST. 
# Each component
#############################################
configure_file(${CMAKE_SOURCE_DIR}/aclConfig.cmake.in
               ${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
               @ONLY)

export(TARGETS ${TARGET_LIST} FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake)

install(TARGETS ${TARGET_LIST} EXPORT ${PROJECT_NAME}
   RUNTIME DESTINATION bin COMPONENT lib
   LIBRARY DESTINATION lib${LIB_SUFFIX} COMPONENT lib
   ARCHIVE DESTINATION lib${LIB_SUFFIX} COMPONENT lib
   INCLUDES DESTINATION include/acl
   PUBLIC_HEADER DESTINATION include/acl
)

install( FILES ${ATOOL_HEADERS} DESTINATION include/acl)

install(FILES ${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
        DESTINATION lib${LIB_SUFFIX}/cmake)
install(EXPORT ${PROJECT_NAME}
        DESTINATION lib${LIB_SUFFIX}/cmake
        FILE ${PROJECT_NAME}Targets.cmake)

