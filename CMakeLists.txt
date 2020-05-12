cmake_minimum_required(VERSION 3.15)
set(ENV{LSAN_OPTIONS} "verbosity=1:log_threads=1")
set(CPPUTILITIES_MAJOR_VERSION 1)
set(CPPUTILITIES_MINOR_VERSION 1)
set(CPPUTILITIES_PATCH_VERSION 1)
set(CPPUTILITIES_VERSION
  ${CPPUTILITIES_MAJOR_VERSION}.${CPPUTILITIES_MINOR_VERSION}.${CPPUTILITIES_PATCH_VERSION})

project(CppUtilities VERSION ${CPPUTILITIES_VERSION})
set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_COMPILER g++)
# Offer the user the choice of overriding the installation directories
set(INSTALL_LIB_DIR lib CACHE PATH "Installation directory for libraries")
set(INSTALL_BIN_DIR bin CACHE PATH "Installation directory for executables")
set(INSTALL_INCLUDE_DIR include CACHE PATH
  "Installation directory for header files")
if(WIN32 AND NOT CYGWIN)
  set(DEF_INSTALL_CMAKE_DIR CMake)
else()
  set(DEF_INSTALL_CMAKE_DIR lib/CMake/CppUtilities)
endif()
set(INSTALL_CMAKE_DIR ${DEF_INSTALL_CMAKE_DIR} CACHE PATH
  "Installation directory for CMake files")

# Make relative paths absolute (needed later on)
foreach(p LIB BIN INCLUDE CMAKE)
  set(var INSTALL_${p}_DIR)
  if(NOT IS_ABSOLUTE "${${var}}")
    set(${var} "${CMAKE_INSTALL_PREFIX}/${${var}}")
  endif()
endforeach()

# set up include-directorie
message(
  "${CMAKE_SOURCE_DIR}"
  " "
  "${PROJECT_SOURCE_DIR}"
  " "
  "${PROJECT_BINARY_DIR}")
include_directories(
  "${CMAKE_SOURCE_DIR}"
  "${PROJECT_SOURCE_DIR}"
  "${PROJECT_BINARY_DIR}")

# The interesting stuff goes here
# ===============================

# Export the package for use from the build-tree
# (this registers the build-tree with a global CMake-registry)
export(PACKAGE CppUtilities)

# Create the CppUtilitiesConfig.cmake and CppUtilitiesConfigVersion files
file(RELATIVE_PATH REL_INCLUDE_DIR "${INSTALL_CMAKE_DIR}"
   "${INSTALL_INCLUDE_DIR}")
# ... for the build tree
set(CONF_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}" "${PROJECT_BINARY_DIR}")
configure_file(CppUtilitiesConfig.cmake.in
  "${PROJECT_BINARY_DIR}/CppUtilitiesConfig.cmake" @ONLY)
# ... for the install tree
set(CONF_INCLUDE_DIRS "\${CPPUTILITIES_CMAKE_DIR}/${REL_INCLUDE_DIR}")
configure_file(CppUtilitiesConfig.cmake.in
  "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CppUtilitiesConfig.cmake" @ONLY)
# ... for both
configure_file(CppUtilitiesConfigVersion.cmake.in
  "${PROJECT_BINARY_DIR}/CppUtilitiesConfigVersion.cmake" @ONLY)

# Install the CppUtilitiesConfig.cmake and CppUtilitiesConfigVersion.cmake
install(FILES
  "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CppUtilitiesConfig.cmake"
  "${PROJECT_BINARY_DIR}/CppUtilitiesConfigVersion.cmake"
  DESTINATION "${INSTALL_CMAKE_DIR}" COMPONENT dev)

#-----------------------------------------------------------------------------
add_subdirectory(tests)

