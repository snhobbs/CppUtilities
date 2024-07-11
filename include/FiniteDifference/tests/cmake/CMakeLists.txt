CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

set(CMAKE_CXX_COMPILER /usr/bin/g++-8)


project(Tests)
#set (CMAKE_CXX_STANDARD 17)
add_definitions(
    -DLINUX
    -DTESTING
    -DUNITTESTING
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

set(PROJ_SRC ../../)
set(TESTSRC ../source)
set(CORTEXLIBS /home/simon/CortexLibs)

message(STATUS "Driver Source: ${TESTSRC}/drivers")

AUX_SOURCE_DIRECTORY(${PROJ_SRC} DIR_SRCS)
AUX_SOURCE_DIRECTORY(${TESTSRC} DIR_SRCS)

message(STATUS "Sources: ${DIR_SRCS}")

ADD_EXECUTABLE(${PROJECT_NAME} ${DIR_SRCS})

target_link_libraries( ${PROJECT_NAME} UnitTest++)

target_include_directories(${PROJECT_NAME} PRIVATE ${PROJ_SRC} )
target_include_directories(${PROJECT_NAME} PRIVATE ${TESTSRC} )
target_include_directories(${PROJECT_NAME} PRIVATE ${CORTEXLIBS} )
