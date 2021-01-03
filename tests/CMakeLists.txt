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
set(PROJECT_SOURCE_DIR ..)
foreach(dir ${dirs})
  message("Directory: ${PROJECT_SOURCE_DIR}/include/${dir}/tests")
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
add_test(NAME ${CMAKE_SOURCE_DIR} COMMAND tests)

#------------------------------------------------------------
# Static Analysis
#------------------------------------------------------------
function(Analyze source_directories)
  message(Running Analysis)
list(APPEND AnalyseFiles ${source_directories})
include(CMakeStaticAnalysis/Utilities.cmake)
include(CMakeStaticAnalysis/StaticAnalysis.cmake)
GetStaticAnalysisFiles("tests" "")
message("${AnalyseFiles}")
set(CMAKE_CXX_CPPLINT "cpplint;--verbose=5;--linelength=100")
set(CMAKE_CXX_CLANG_TIDY "clang-tidy;-checks=*;-format-style=google;-export-fixes=${CMAKE_CURRENT_BINARY_DIR}/clangtidy.results")

Cpplint("${CMAKE_CURRENT_SOURCE_DIR}" "${AnalyseFiles}" cpplint)
Cppcheck("${CMAKE_CURRENT_SOURCE_DIR}" "${AnalyseFiles}" cppcheck)
flint("${CMAKE_CURRENT_SOURCE_DIR}" "${AnalyseFiles}" flint++)
endfunction()

option(RUN_ANALYSIS "Run CppCheck, CppLint, and Clang-tidy" ON)
if(RUN_ANALYSIS)
  Analyze("${DIR_SRCS}")
endif()
