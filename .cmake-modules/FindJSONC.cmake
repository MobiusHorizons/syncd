# - Find json-c
# Find the native JSONC headers and libraries.
#
#  JSONC_INCLUDE_DIRS - where to find jsonc/jsonc.h, etc.
#  JSONC_LIBRARIES    - List of libraries when using jsonc.
#  JSONC_FOUND        - True if jsonc found.


# Look for the header file.
FIND_PATH(JSONC_INCLUDE_DIR json-c/json.h
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

MARK_AS_ADVANCED(JSONC_INCLUDE_DIR)

# Look for the library.
FIND_LIBRARY(JSONC_LIBRARY NAMES json-c libjson-c PATHS
  $ENV{LIB}
  $ENV{PATH}
  "$ENV{LIB_DIR}/lib"
  /usr/local/lib
  /usr/lib
  /usr/lib/x86_64-linux-gnu
  /usr/lib/i386-linux-gnu
  c:/msys/lib
  /usr/x86_64-w64-mingw32/lib
  /usr/i686-w64-mingw32/lib
  NO_DEFAULT_PATH
  )

MARK_AS_ADVANCED(JSONC_LIBRARY)

IF(JSONC_INCLUDE_DIR)
	MESSAGE(STATUS "json-c include was found")
ENDIF(JSONC_INCLUDE_DIR)
IF(JSONC_LIBRARY)
  MESSAGE(STATUS "json-c lib was found")
ENDIF(JSONC_LIBRARY)

# Copy the results to the output variables.
IF(JSONC_INCLUDE_DIR AND JSONC_LIBRARY)
	SET(JSONC_FOUND 1)
	SET(JSONC_LIBRARIES ${JSONC_LIBRARY})
  SET(JSONC_INCLUDE_DIRS ${JSONC_INCLUDE_DIR})
ELSE(JSONC_INCLUDE_DIR AND JSONC_LIBRARY)
  SET(JSONC_FOUND 0)
  SET(JSONC_LIBRARIES)
  SET(JSONC_INCLUDE_DIRS)
ENDIF(JSONC_INCLUDE_DIR AND JSONC_LIBRARY)

# Report the results.
IF(JSONC_FOUND)
   IF (NOT JSONC_FIND_QUIETLY)
      MESSAGE(STATUS "Found json-c: ${JSONC_LIBRARY}")
   ENDIF (NOT JSONC_FIND_QUIETLY)
ELSE(JSONC_FOUND)
  SET(JSONC_DIR_MESSAGE "json-c was not found.")

  IF(JSONC_FIND_REQUIRED)
    MESSAGE(FATAL_ERROR "${JSONC_DIR_MESSAGE}")
  ELSE(JSONC_FIND_REQUIRED)
    IF(NOT JSONC_FIND_QUIETLY)
      MESSAGE(STATUS "${JSONC_DIR_MESSAGE}")
    ENDIF(NOT JSONC_FIND_QUIETLY)
    # Avoid cmake complaints if JSONC is not found
    SET(JSONC_INCLUDE_DIR "")
    SET(JSONC_LIBRARY "")
  ENDIF(JSONC_FIND_REQUIRED)

ENDIF(JSONC_FOUND)
