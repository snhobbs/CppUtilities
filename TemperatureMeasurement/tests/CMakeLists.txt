CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

project(Tests)

#set (CMAKE_CXX_STANDARD 17)
add_definitions(
    -DLINUX
    -DTESTING
    -DUNITTESTING
    -DPGA_PRECISION_BITS=13
)

#add_definitions(-D__CORE_CMINSTR_H)
#add_definitions(-D__CORE_CMINSTR_H)
#add_definitions(-D__GNUC__)

add_compile_options(-Wall -Wextra -Wpedantic -Wfatal-errors)
add_compile_options(-std=c++17)
add_compile_options(-fpermissive)
#add_compile_options(-fprofile-arcs -ftest-coverage)

#SET(GCC_COVERAGE_LINK_FLAGS    "-coverage -lgcov")
#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}")

set(FSL_FEATURE_ADC_HAS_CTRL_LPWRMODE true)
SET(CMAKE_VERBOSE_MAKEFILE ON)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}")
SET(CMAKE_C_FLAGS_DEBUG "-g -O0")
SET(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
SET(CMAKE_C_FLAGS_RELEASE "-g -O3")
SET(CMAKE_CXX_FLAGS_RELEASE "-g -O3")

set(PROJ_SRC ..)
set(TESTSRC source/)
set(CORTEXLIBS /home/simon/CortexLibs)
set(DEVICELIBS ${CORTEXLIBS}/Devices/TestDevice)

message(STATUS "Driver Source: ${TESTSRC}/drivers")

#AUX_SOURCE_DIRECTORY(${PROJ_SRC} DIR_SRCS)
#LIST(REMOVE_ITEM ${DIR_SRCS} "main.cpp")
AUX_SOURCE_DIRECTORY(${TESTSRC} DIR_SRCS)
#LIST(APPEND DIR_SRCS "${PROJ_SRC}/Tasks.cpp")
#LIST(APPEND DIR_SRCS "${PROJ_SRC}/ScriptControl.cpp")
#LIST(APPEND DIR_SRCS "${PROJ_SRC}/SharedObjects.cpp")

message(STATUS "Sources: ${DIR_SRCS}")

ADD_EXECUTABLE(${PROJECT_NAME} ${DIR_SRCS})

#target_include_directories(${PROJECT_NAME} PRIVATE ../../UnitTest++/include/UnitTest++)
#add_library(./lib/libUnitTest++.a STATIC IMPORTED )# STATIC /usr/local/include/UnitTest++)

#get_filename_component(UNITTEST++_PATH ../../../UnitTest++/lib/libUnitTest++.a ABSOLUTE ./)
target_link_libraries(${PROJECT_NAME} ${UNITTEST++_PATH})
target_link_libraries( ${PROJECT_NAME} UnitTest++)

target_include_directories(${PROJECT_NAME} PRIVATE ${PROJ_SRC} )
target_include_directories(${PROJECT_NAME} PRIVATE ${TESTSRC} )
target_include_directories(${PROJECT_NAME} PRIVATE ${CORTEXLIBS} )
target_include_directories(${PROJECT_NAME} PRIVATE ${DEVICELIBS} )

