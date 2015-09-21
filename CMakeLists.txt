cmake_minimum_required(VERSION 2.8)
project(cppstdx)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "../parrotscmake")

## Compiler settings

message(STATUS "CMAKE_SYSTEM_NAME: " ${CMAKE_SYSTEM_NAME})
message(STATUS "CXX_COMPILER_ID: " ${CMAKE_CXX_COMPILER_ID})

if (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
    set(NEWCXX_ENABLED_FLAG "-std=c++11 -stdlib=libc++ -Wall -O2")
elseif (${CMAKE_CXX_COMPILER_ID} MATCHES "GNU")
    set(NEWCXX_ENABLED_FLAG "-std=c++11 -Wall -O2")
else()
    message(FATAL_ERROR, "Only GCC and clang is supported for now.")
endif()

set(CMAKE_CXX_FLAGS ${NEWCXX_ENABLED_FLAG})

enable_testing()

###################################
#
#  Dependencies
#
###################################

## Google Test

find_package(GTest REQUIRED)
if (GTEST_FOUND)
    include_directories(SYSTEM ${GTEST_INCLUDE_DIRS})
    message(STATUS "GTest Libraries: " ${GTEST_LIBRARIES})
endif()

# pthread library is required for Google Test to work properly
find_package(Threads REQUIRED)

###################################
#
#  Directories
#
###################################

include_directories(include)

set(TESTS tests)
set(EXECUTABLE_OUTPUT_PATH bin)


###################################
#
#  Unit testing
#
###################################

set(UNIT_TESTS
    test_optional
    test_string_view
)

foreach (tname ${UNIT_TESTS})
    add_executable(${tname} ${TESTS}/${tname}.cpp)    
    target_link_libraries(${tname} ${GTEST_BOTH_LIBRARIES})
    target_link_libraries(${tname} ${CMAKE_THREAD_LIBS_INIT})
    add_test(NAME ${tname} COMMAND ${tname})
endforeach()

