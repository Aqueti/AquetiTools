# - Find MongoDB
# Find the MongoDB includes and client library
# This module defines where to find bson.h
#
include(FindPackageHandleStandardArgs)

# if MONGOC_ROOT has defined in system environment, use this as a hint
set(MONGOC_ROOT "$ENV{MONGOC_ROOT}")
set(INSTALL_PATH "${CMAKE_BINARY_DIR}/INSTALL")

#find the header files
find_path(MONGOC_INCLUDE_DIRS "mongoc.h" HINTS 
   "${MONGOC_ROOT}/include"
   "${INSTALL_PATH}/include/libmongoc-1.0"
   "${CMAKE_INSTALL_PREFIX}/include/libmongoc-1.0"
)


find_library(MONGOC_LIBRARY NAMES "mongoc-1.0" HINTS 
   "${MONGOC_ROOT}/lib"
   "${CMAKE_INSTALL_PREFIX}/lib"
)
#
find_package_handle_standard_args(MONGOC DEFAULT_MSG MONGOC_INCLUDE_DIRS MONGOC_LIBRARY)
