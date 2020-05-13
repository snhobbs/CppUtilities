CMAKE_MINIMUM_REQUIRED(VERSION 3.15)

project(CalculatorTests)
add_executable(${PROJECT_NAME})
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)
target_compile_definitions(${PROJECT_NAME} PRIVATE TESTING)
target_compile_definitions(${PROJECT_NAME} PRIVATE UNITTESTING)
target_compile_definitions(${PROJECT_NAME} PRIVATE LINUX)

#add_definitions(-D__CORE_CMINSTR_H)
#add_definitions(-D__CORE_CMINSTR_H)
#add_definitions(-D__GNUC__)

target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Wfatal-errors)
target_compile_options(${PROJECT_NAME} PRIVATE -fpermissive)
#target_compile_options(-fprofile-arcs -ftest-coverage)

#SET(GCC_COVERAGE_LINK_FLAGS    "-coverage -lgcov")
#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}")

#set(FSL_FEATURE_ADC_HAS_CTRL_LPWRMODE true)
SET(CMAKE_VERBOSE_MAKEFILE ON)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}")
SET(CMAKE_C_FLAGS_DEBUG "-g -O0")
SET(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
SET(CMAKE_C_FLAGS_RELEASE "-g -O3")
SET(CMAKE_CXX_FLAGS_RELEASE "-g -O3")

set(PROJ_SRC ../)
set(TESTSRC ./source)
set(CORTEXLIBS /home/simon/CortexLibs)
set(CHIPLIB ${CORTEXLIBS}/Devices/TestDevice)

#fixme this should have all the sources
aux_source_directory(${TESTSRC} DIR_SRCS)

target_sources(${PROJECT_NAME} PUBLIC ${DIR_SRCS})

target_link_libraries( ${PROJECT_NAME} UnitTest++)

target_include_directories(${PROJECT_NAME} PUBLIC ${PROJ_SRC} )
target_include_directories(${PROJECT_NAME} PUBLIC ${TESTSRC} )
target_include_directories(${PROJECT_NAME} PUBLIC ${CORTEXLIBS} )
target_include_directories(${PROJECT_NAME} PUBLIC ${CHIPLIB} )

message(STATUS "Sources: ${DIR_SRCS}")
