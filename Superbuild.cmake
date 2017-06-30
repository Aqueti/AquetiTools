set(cmake_common_args
    -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
    -DCMAKE_C_COMPILER:PATH=${CMAKE_C_COMPILER}
    -DCMAKE_C_FLAGS:STRING=${CMAKE_C_FLAGS}
    -DCMAKE_CXX_COMPILER:PATH=${CMAKE_CXX_COMPILER}
    -DCMAKE_CXX_FLAGS:STRING=${CMAKE_CXX_FLAGS}
    -DLIB_SUFFIX:STRING=${LIB_SUFFIX}
    -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_BINARY_DIR}/INSTALL
    -DCMAKE_INSTALL_RPATH:PATH=${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}
    -DCMAKE_INSTALL_LIBDIR:PATH=lib
    -DCMAKE_INCLUDE_PATH:PATH=${CMAKE_INSTALL_PREFIX}/include
    -DMAKE_TESTS:BOOL=${MAKE_TESTS}
    -DUSE_DOXYGEN:BOOL=${USE_DOXYGEN}
    -DMAKE_STATIC_LIB:BOOL=${MAKE_STATIC_LIB}
    -DMAKE_DEB_PACKAGE:BOOL=${MAKE_DEB_PACKAGE}
    -DUSE_SUPERBUILD:BOOL=OFF
)

# if MASTER is true, project will always be built. Else, only built if dependency
macro(add_external_project MYNAME LOCATION MASTER DEPENDS ARGS)
    if(NOT ${MASTER})
        set(EXCLUDE ON)
    else()
        set(EXCLUDE OFF)
    endif(NOT ${MASTER})
    ExternalProject_Add( ${MYNAME}
        SOURCE_DIR ${CMAKE_SOURCE_DIR}/${LOCATION}
        BUILD_ALWAYS 1
        EXCLUDE_FROM_ALL ${EXCLUDE}
        DOWNLOAD_COMMAND git submodule update --init --checkout ${CMAKE_SOURCE_DIR}/${LOCATION}
        DOWNLOAD_DIR ${CMAKE_SOURCE_DIR}
        CMAKE_ARGS ${cmake_common_args} ${ARGS}
        INSTALL_DIR ${CMAKE_INSTALL_PREFIX}
        DEPENDS ${DEPENDS}
    )
endmacro(add_external_project)

add_external_project(JsonBox dependencies/JsonBox OFF "" "")

#ATL
ExternalProject_Add(AquetiTools
  SOURCE_DIR ${CMAKE_SOURCE_DIR}
  BUILD_ALWAYS 1
  CMAKE_ARGS ${cmake_common_args} 
  INSTALL_DIR ${CMAKE_INSTALL_PREFIX}
  LOG_INSTALL 1
  DEPENDS JsonBox
)

