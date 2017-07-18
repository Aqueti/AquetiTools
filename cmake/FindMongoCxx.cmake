# - Find MongoDB
# Find the MongoDB includes and client library
# This module defines where to find bson.h
#
include(FindPackageHandleStandardArgs)

# if MONGOCXX_ROOT has defined in system environment, use this as a hint
set(MONGOCXX_ROOT "$ENV{MONGOCXX_ROOT}")
set(INSTALL_PATH "${CMAKE_BINARY_DIR}/INSTALL")

#find the header files
find_path(MONGOCXX_INCLUDE_DIRS "mongocxx/collection.hpp" HINTS 
   "${MONGOCXX_ROOT}/include"
   "${INSTALL_PATH}/include/mongocxx"
   "${INSTALL_PATH}/include/mongocxx/v_noabi/"
   "${CMAKE_INSTALL_PREFIX}/include/mongocxx/v_noabi/"
)


find_library(MONGOCXX_LIBRARY NAMES "mongocxx" HINTS 
   "${MONGOCXX_ROOT}/lib"
   "${INSTALL_PATH}/lib"
   "${CMAKE_INSTALL_PREFIX}/lib"
)

find_package_handle_standard_args(MONGOCXX DEFAULT_MSG MONGOCXX_INCLUDE_DIRS MONGOCXX_LIBRARY)
