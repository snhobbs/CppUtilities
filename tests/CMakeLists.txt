cmake_minimum_required(VERSION 3.10.0)

project(Testing)
set(dirs
 Calculators
 CooperativeScheduler
 FiniteDifference
 PIDFilters
 Utilities
 RingBuffer
 TemperatureMeasurement
)
foreach(dir ${dirs})
  set(path ${PROJECT_SOURCE_DIR}/include/${dir}/tests)
  if(EXISTS ${path})
    message(STATUS "${dir} has tests")
    list(APPEND source_directories ${path})
    list(APPEND source_directories ${path}/source)
  else()
    message(WARNING "${dir} has no tests")
  endif()
endforeach()

#-----------------------------------------------------------------------------

add_executable(tests)

option(Sanitizer "Run address sanitizer" OFF)

if(${Sanitizer})
target_compile_options(
tests
PRIVATE
$<$<CONFIG:DEBUG>:-fsanitize=address>
  -fno-omit-frame-pointer
  $<$<CONFIG:DEBUG>:-fsanitize=undefined>
  )
target_link_libraries( tests asan)
#target_link_libraries( tests tsan)
target_link_libraries( tests ubsan)
#target_link_libraries( tests msan)


endif()

target_compile_options(
  tests
  PRIVATE
    $<$<CONFIG:DEBUG>:-g3>
    -Wall
    -Wextra
    -pedantic
    -Wpedantic
    #-pedantic-errors
    #-Werror=pedantic
    -Wno-float-conversion

    -Wconversion
    $<$<COMPILE_LANGUAGE:C>:-Wimplicit>
    $<$<COMPILE_LANGUAGE:C>:-Wunused>

    #$<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>
    #$<$<COMPILE_LANGUAGE:CXX>:-Wabi>
    $<$<CXX_COMPILER_ID:GNU>:-Wsuggest-final-types>
    $<$<CXX_COMPILER_ID:GNU>:-Wmaybe-uninitialized>
    $<$<CXX_COMPILER_ID:GNU>:-Wdouble-promotion>
    $<$<CXX_COMPILER_ID:GNU>:-Wnull-dereference>
    -Winit-self

    $<$<CXX_COMPILER_ID:GNU>:-Wduplicated-cond>
    $<$<CXX_COMPILER_ID:GNU>:-Wduplicated-branches>
    $<$<CXX_COMPILER_ID:GNU>:-Wlogical-op>
    $<$<CXX_COMPILER_ID:GNU>:-Wrestrict>
    $<$<C_COMPILER_ID:GNU>:$<$<COMPILE_LANGUAGE:C>:-Wjump-misses-init>>
    $<$<COMPILE_LANGUAGE:CXX>:-Wold-style-cast>
    -Wshadow
)

target_compile_features(tests PUBLIC cxx_std_17)
target_compile_definitions(tests PRIVATE TESTING)
target_compile_definitions(tests PRIVATE UNITTESTING)
target_compile_definitions(tests PRIVATE LINUX)
set_property(TARGET tests PROPERTY CXX_STANDARD 17)
SET(CMAKE_VERBOSE_MAKEFILE ON)

#fixme this should have all the sources
foreach(dir ${source_directories})
  aux_source_directory(${dir} DIR_SRCS)
  message("${dir}")
endforeach()

foreach(src ${DIR_SRC})
  message("${src}")
endforeach()

target_sources(tests PUBLIC ${DIR_SRCS})
target_sources(tests PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)

get_target_property( srcs tests SOURCES)
foreach(src ${srcs})
  message("Source: ${src}")
endforeach()

find_package(gtest QUIET)
target_link_libraries(tests gtest)
target_link_libraries(tests pthread)

target_include_directories(tests PUBLIC ${TESTSRC})
target_include_directories(tests PUBLIC ${PROJECT_SOURCE_DIR}/include)

message(STATUS "Sources: ${DIR_SRCS}")
find_package(EmbeddedCMake)
include(${EmbeddedCMake_DIR}/source/Utilities.cmake)
GetStaticAnalysisFiles(tests "")
add_test(NAME ${CMAKE_SOURCE_DIR} COMMAND tests)
list(APPEND AnalyseFiles ${DIR_SRCS})

#------------------------------------------------------------
# Static Analysis
#------------------------------------------------------------
option(RUN_ANALYSIS "Run CppCheck, CppLint, and Clang-tidy" ON)
if(${RUN_ANALYSIS})
message("${AnalyseFiles}")
set(CMAKE_CXX_CPPLINT "cpplint;--verbose=5;--linelength=100")
set(CMAKE_CXX_CLANG_TIDY "clang-tidy;-checks=*;-format-style=google;-export-fixes=${CMAKE_CURRENT_BINARY_DIR}/clangtidy.results")

include(${EmbeddedCMake_DIR}/source/cpplint.cmake)
Cpplint("${CMAKE_CURRENT_SOURCE_DIR}" "${AnalyseFiles}" cpplint)

include(${EmbeddedCMake_DIR}/source/cppcheck.cmake)
Cppcheck("${CMAKE_CURRENT_SOURCE_DIR}" "${AnalyseFiles}" cppcheck)

#include(${EmbeddedCMake_DIR}/source/cccc.cmake)
#cccc("${CMAKE_CURRENT_SOURCE_DIR}" "${AnalyseFiles}" cccc)

include(${EmbeddedCMake_DIR}/source/flint++.cmake)
flint("${CMAKE_CURRENT_SOURCE_DIR}" "${AnalyseFiles}" flint++)

#include(${EmbeddedCMake_DIR}/source/flawfinder.cmake)
#flawfinder("${CMAKE_CURRENT_SOURCE_DIR}" "${AnalyseFiles}" flawfinder)

#include(${EmbeddedCMake_DIR}/source/pmd.cmake)
#set(pmd_command "$ENV{HOME}/pmd-bin-6.23.0/bin/run.sh pmd -l -d ${CMAKE_CURRENT_SOURCE_DIR} -l c++ -f text")
#pmd("${CMAKE_CURRENT_SOURCE_DIR}" "${AnalyseFiles}" ${pmd_command})
endif()

