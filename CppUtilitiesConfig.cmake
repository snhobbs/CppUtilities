# - Config file for the CppUtilities package
# It defines the following variables
#  CPPUTILITIES_INCLUDE_DIRS - include directories for CppUtilities
#  CPPUTILITIES_LIBRARIES    - libraries to link against
#  CPPUTILITIES_EXECUTABLE   - the bar executable

# Compute paths
get_filename_component("CPPUTILITIES_CMAKE_DIR" "${CMAKE_CURRENT_LIST_FILE}" PATH)
set("CPPUTILITIES_INCLUDE_DIRS" "/home/simon/software/CppUtilities;/home/simon/software/CppUtilities")
