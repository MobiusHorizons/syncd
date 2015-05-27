# - Find libuv
# Find the native UV headers and libraries.
#
#  UV_INCLUDE_DIRS - where to find curl/curl.h, etc.
#  UV_LIBRARIES    - List of libraries when using curl.
#  UV_FOUND        - True if curl found.

# Look for the header file.
FIND_PATH(UV_INCLUDE_DIR uv.h
  $ENV{INCLUDE}
  "$ENV{LIB_DIR}/include"
  /usr/local/include
  /usr/include
  #mingw
  c:/msys/include
  /usr/x86_64-w64-mingw32/include
  /usr/i686-w64-mingw32/include
  NO_DEFAULT_PATH
  )

MARK_AS_ADVANCED(UV_INCLUDE_DIR)

# Look for the library.
FIND_LIBRARY(UV_LIBRARY NAMES uv libuv PATHS
  $ENV{LIB}
  "$ENV{LIB_DIR}/lib"
  /usr/local/lib
  /usr/lib
  c:/msys/lib
  c:/msys/bin
  /usr/x86_64-w64-mingw32/lib
  /usr/i686-w64-mingw32/lib
  NO_DEFAULT_PATH
  )

MARK_AS_ADVANCED(UV_LIBRARY)

IF(UV_INCLUDE_DIR)
	MESSAGE(STATUS "libuv include was found")
ENDIF(UV_INCLUDE_DIR)
IF(UV_LIBRARY)
  MESSAGE(STATUS "libuv lib was found")
ENDIF(UV_LIBRARY)

# Copy the results to the output variables.
IF(UV_INCLUDE_DIR AND UV_LIBRARY)
	SET(UV_FOUND 1)
	SET(UV_LIBRARIES ${UV_LIBRARY})
  SET(UV_INCLUDE_DIRS ${UV_INCLUDE_DIR})
ELSE(UV_INCLUDE_DIR AND UV_LIBRARY)
  SET(UV_FOUND 0)
  SET(UV_LIBRARIES)
  SET(UV_INCLUDE_DIRS)
ENDIF(UV_INCLUDE_DIR AND UV_LIBRARY)

# Report the results.
IF(UV_FOUND)
   IF (NOT UV_FIND_QUIETLY)
      MESSAGE(STATUS "Found libuv: ${UV_LIBRARY}")
   ENDIF (NOT UV_FIND_QUIETLY)
ELSE(UV_FOUND)
  SET(UV_DIR_MESSAGE "libuv was not found.")

  IF(UV_FIND_REQUIRED)
    MESSAGE(FATAL_ERROR "${UV_DIR_MESSAGE}")
  ELSE(UV_FIND_REQUIRED)
    IF(NOT UV_FIND_QUIETLY)
      MESSAGE(STATUS "${UV_DIR_MESSAGE}")
    ENDIF(NOT UV_FIND_QUIETLY)
    # Avoid cmake complaints if UV is not found
    SET(UV_INCLUDE_DIR "")
    SET(UV_LIBRARY "")
  ENDIF(UV_FIND_REQUIRED)

ENDIF(UV_FOUND)
