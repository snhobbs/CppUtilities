cmake_minimum_required(VERSION 3.10.0)

find_package(UnitTest++ QUIET)
add_executable(tests)
#SET(CMAKE_CXX_COMPILER clang++-8)
SET(CMAKE_CXX_COMPILER /usr/bin/g++-8)
get_filename_component(TESTSRC source ABSOLUTE)

#fixme this should have all the sources
aux_source_directory(${TESTSRC} DIR_SRCS)

target_sources(tests PUBLIC ${DIR_SRCS})
#target_link_directories(tests PUBLIC /usr/lib/clang/8/lib/linux/)
#target_link_libraries( tests clang_rt.ubsan_standalone-x86_64.a)
#target_link_libraries(tests ${UNITTEST++_PATH})
target_link_libraries( tests asan)
#target_link_libraries( tests tsan)
target_link_libraries( tests ubsan)
#target_link_libraries( tests msan)

target_link_libraries( tests UnitTest++)

target_include_directories(tests PUBLIC ${TESTSRC} )
target_include_directories(tests PUBLIC ${CMAKE_SOURCE_DIR} )

message(STATUS "Sources: ${DIR_SRCS}")

add_test(NAME ${CMAKE_SOURCE_DIR} COMMAND tests)
