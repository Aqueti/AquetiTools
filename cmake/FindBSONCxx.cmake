# - Find MongoDB
# Find the MongoDB includes and client library
# This module defines where to find bson.h
#
include(FindPackageHandleStandardArgs)

# if BSONCXX_ROOT has defined in system environment, use this as a hint
set(BSONCXX_ROOT "$ENV{BSONCXX_ROOT}")
set(INSTALL_PATH "${CMAKE_BINARY_DIR}/INSTALL")

#find the header files
find_path(BSONCXX_INCLUDE_DIRS "bsoncxx/oid.hpp" HINTS 
#   "${BSONCXX_ROOT}/include"
#   "${INSTALL_PATH}/include/bsoncxx"
   "${INSTALL_PATH}/include/bsoncxx/v_noabi/"
   "${CMAKE_INSTALL_PREFIX}/include/bsoncxx/v_noabi/"
)


find_library(BSONCXX_LIBRARY NAMES "bsoncxx" HINTS 
   "${BSONCXX_ROOT}/lib"
   "${INSTALL_PATH}/lib"
   "${CMAKE_INSTALL_PREFIX}/lib"
)

message("BSONCXX: ${BSONCXX_INCLUDE_DIRS}, ${BSONCXX_LIBRARY}")
#
find_package_handle_standard_args(BSONCXX DEFAULT_MSG BSONCXX_INCLUDE_DIRS BSONCXX_LIBRARY)
