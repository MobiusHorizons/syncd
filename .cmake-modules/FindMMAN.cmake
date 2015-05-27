# - Find libmman
# Find the native MMAN headers and libraries.
#
#  MMAN_INCLUDE_DIRS - where to find MMAN.h, etc.
#  MMAN_LIBRARIES    - List of libraries when using curl.
#  MMAN_FOUND        - True if curl found.

# Look for the header file.
FIND_PATH(MMAN_INCLUDE_DIR sys/mman.h
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

MARK_AS_ADVANCED(MMAN_INCLUDE_DIR)

# Look for the library.
FIND_LIBRARY(MMAN_LIBRARY NAMES mman libmman PATHS
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

MARK_AS_ADVANCED(MMAN_LIBRARY)

IF(MMAN_INCLUDE_DIR)
	MESSAGE(STATUS "libmman include was found")
ENDIF(MMAN_INCLUDE_DIR)
IF(MMAN_LIBRARY)
  MESSAGE(STATUS "libmman lib was found")
ENDIF(MMAN_LIBRARY)

# Copy the results to the output variables.
IF(MMAN_INCLUDE_DIR AND MMAN_LIBRARY)
	SET(MMAN_FOUND 1)
	SET(MMAN_LIBRARIES ${MMAN_LIBRARY})
  SET(MMAN_INCLUDE_DIRS ${MMAN_INCLUDE_DIR})
ELSE(MMAN_INCLUDE_DIR AND MMAN_LIBRARY)
  SET(MMAN_FOUND 0)
  SET(MMAN_LIBRARIES)
  SET(MMAN_INCLUDE_DIRS)
ENDIF(MMAN_INCLUDE_DIR AND MMAN_LIBRARY)

# Report the results.
IF(MMAN_FOUND)
   IF (NOT MMAN_FIND_QUIETLY)
      MESSAGE(STATUS "Found libmman: ${MMAN_LIBRARY}")
   ENDIF (NOT MMAN_FIND_QUIETLY)
ELSE(MMAN_FOUND)
  SET(MMAN_DIR_MESSAGE "libmman was not found.")

  IF(MMAN_FIND_REQUIRED)
    MESSAGE(FATAL_ERROR "${MMAN_DIR_MESSAGE}")
  ELSE(MMAN_FIND_REQUIRED)
    IF(NOT MMAN_FIND_QUIETLY)
      MESSAGE(STATUS "${MMAN_DIR_MESSAGE}")
    ENDIF(NOT MMAN_FIND_QUIETLY)
    # Avoid cmake complaints if UV is not found
    SET(MMAN_INCLUDE_DIR "")
    SET(MMAN_LIBRARY "")
  ENDIF(MMAN_FIND_REQUIRED)

ENDIF(MMAN_FOUND)
