# - Find MongoDB
# Find the MongoDB includes and client library
# This module defines where to find bson.h
#
include(FindPackageHandleStandardArgs)

# if BSON_ROOT has defined in system environment, use this as a hint
set(BSON_ROOT "$ENV{BSON_ROOT}")
set(INSTALL_PATH "${CMAKE_BINARY_DIR}/INSTALL")

#find the header files
find_path(BSON_INCLUDE_DIRS "bson.h" HINTS 
   "${BSON_ROOT}/include"
   "${INSTALL_PATH}/include/libbson-1.0"
   "${CMAKE_INSTALL_PREFIX}/include/libbson-1.0/"
)


message( "BSON INSTALL_PATH: ${INSTALL_PATH}")
find_library(BSON_LIBRARIES NAMES "bson-1.0" HINTS 
   "${INSTALL_PATH}/lib"
   "${CMAKE_INSTALL_PREFIX}/lib"
)

message("LIBSSON: ${BSON_LIBRARIES}")



find_package_handle_standard_args(BSON DEFAULT_MSG BSON_INCLUDE_DIRS BSON_LIBRARIES)
