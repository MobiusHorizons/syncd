# - Find libltdl
# Find the native UV headers and libraries.
#
#  LTDL_INCLUDE_DIRS - where to find ltdl.h, etc.
#  LTDL_LIBRARIES    - List of libraries when using curl.
#  LTDL_FOUND        - True if curl found.

# use pkg-config to get the directories and then use these values
# in the FIND_PATH() and FIND_LIBRARY() calls
if(NOT WIN32)
   find_package(PkgConfig)
   pkg_check_modules(PC_LTDL libcurl)
   set(LTDL_DEFINITIONS ${PC_LTDL_CFLAGS_OTHER})
endif(NOT WIN32)
s
# Look for the header file.
FIND_PATH(LTDL_INCLUDE_DIR ltdl.h
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

MARK_AS_ADVANCED(LTDL_INCLUDE_DIR)

# Look for the library.
FIND_LIBRARY(LTDL_LIBRARY NAMES ltdl libltdl PATHS
  $ENV{LIB}
  "$ENV{LIB_DIR}/lib"
  /usr/local/lib
  /usr/lib
  c:/msys/lib
  c:/msys/bin
  /usr/x86_64-w64-mingw32/lib
  /usr/i686-w64-mingw32/lib
  HINTS
  ${PC_LTDL_LIBDIR}
  ${PC_LTDL_LIBRARY_DIRS}
  NO_DEFAULT_PATH
  )

MARK_AS_ADVANCED(LTDL_LIBRARY)

IF(LTDL_INCLUDE_DIR)
	MESSAGE(STATUS "libltdl include was found")
ENDIF(LTDL_INCLUDE_DIR)
IF(LTDL_LIBRARY)
  MESSAGE(STATUS "libltdl lib was found")
ENDIF(LTDL_LIBRARY)

# Copy the results to the output variables.
IF(LTDL_INCLUDE_DIR AND LTDL_LIBRARY)
	SET(LTDL_FOUND 1)
	SET(LTDL_LIBRARIES ${LTDL_LIBRARY})
  SET(LTDL_INCLUDE_DIRS ${LTDL_INCLUDE_DIR})
ELSE(LTDL_INCLUDE_DIR AND LTDL_LIBRARY)
  SET(LTDL_FOUND 0)
  SET(LTDL_LIBRARIES)
  SET(LTDL_INCLUDE_DIRS)
ENDIF(LTDL_INCLUDE_DIR AND LTDL_LIBRARY)

# Report the results.
IF(LTDL_FOUND)
   IF (NOT LTDL_FIND_QUIETLY)
      MESSAGE(STATUS "Found libltdl: ${LTDL_LIBRARY}")
   ENDIF (NOT LTDL_FIND_QUIETLY)
ELSE(LTDL_FOUND)
  SET(LTDL_DIR_MESSAGE "libltdl was not found.")

  IF(LTDL_FIND_REQUIRED)
    MESSAGE(FATAL_ERROR "${LTDL_DIR_MESSAGE}")
  ELSE(LTDL_FIND_REQUIRED)
    IF(NOT LTDL_FIND_QUIETLY)
      MESSAGE(STATUS "${LTDL_DIR_MESSAGE}")
    ENDIF(NOT LTDL_FIND_QUIETLY)
    # Avoid cmake complaints if UV is not found
    SET(LTDL_INCLUDE_DIR "")
    SET(LTDL_LIBRARY "")
  ENDIF(LTDL_FIND_REQUIRED)

ENDIF(LTDL_FOUND)
