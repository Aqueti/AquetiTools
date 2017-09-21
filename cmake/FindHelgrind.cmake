# - Find Helgrind
# Find the Helgrind includes and client library
# This module defines
#  Helgrind_INCLUDE_DIR, where to find Helgrind/client/dbclient.h
#  Helgrind_FOUND, If false, do not try to use Helgrind.

if(Helgrind_INCLUDE_DIR)
    set(Helgrind_FOUND TRUE)
    return()
endif(Helgrind_INCLUDE_DIR)


find_path(Helgrind_INCLUDE_DIR helgrind.h
    include/
    include/valgrind/
    /usr/include/
    /usr/include/valgrind/
    /usr/local/include/
    /usr/local/include/valgrind/
    )

if(Helgrind_INCLUDE_DIR)
    set(Helgrind_FOUND TRUE)
    message(STATUS "Found Helgrind: ${Helgrind_INCLUDE_DIR}")
else(Helgrind_INCLUDE_DIR)
    set(Helgrind_FOUND FALSE)
    if (Helgrind_FIND_REQUIRED)
        message(FATAL_ERROR "Helgrind not found.")
    else (Helgrind_FIND_REQUIRED)
        message(STATUS "Helgrind not found.")
    endif (Helgrind_FIND_REQUIRED)
endif(Helgrind_INCLUDE_DIR)

mark_as_advanced(Helgrind_INCLUDE_DIR)

