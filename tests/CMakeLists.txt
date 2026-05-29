cmake_minimum_required(VERSION 3.15)
project(CppUtilities-tests CXX)

include(FetchContent)
FetchContent_Declare(googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0
)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

set(LIB_INC ${CMAKE_CURRENT_SOURCE_DIR}/../include)

add_executable(tests
    source/main.cpp
    source/test_linear_fit.cpp
    ${LIB_INC}/Calculators/tests/source/TestCalculatorBase.cpp
    ${LIB_INC}/FiniteDifference/tests/source/test_finitedifference.cpp
    ${LIB_INC}/RingBuffer/tests/source/DataLoader.cpp
    ${LIB_INC}/RingBuffer/tests/source/test_buffer.cpp
    ${LIB_INC}/RingBuffer/tests/source/test_ringbuffer.cpp
    ${LIB_INC}/TemperatureMeasurement/tests/source/TestThermistorDivider.cpp
    ${LIB_INC}/Utilities/tests/source/test_Crc.cpp
)

set_property(TARGET tests PROPERTY CXX_STANDARD 20)
target_compile_definitions(tests PRIVATE TESTING UNITTESTING LINUX DEBUG)
target_compile_options(tests PRIVATE
    -O2
    -fsanitize=address
    -fsanitize=undefined
    -fno-omit-frame-pointer
    -Wall -Wextra -Wpedantic -Wconversion -Wshadow
    $<$<CXX_COMPILER_ID:GNU>:-Wsuggest-final-types>
    $<$<CXX_COMPILER_ID:GNU>:-Wmaybe-uninitialized>
    $<$<CXX_COMPILER_ID:GNU>:-Wdouble-promotion>
    $<$<CXX_COMPILER_ID:GNU>:-Wnull-dereference>
    $<$<COMPILE_LANGUAGE:CXX>:-Wold-style-cast>
)
target_link_options(tests PRIVATE -fsanitize=address -fsanitize=undefined)
target_link_libraries(tests PRIVATE GTest::gtest pthread)
target_include_directories(tests PRIVATE ${LIB_INC} source)
