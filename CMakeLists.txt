cmake_minimum_required(VERSION 3.8)

project(k4LCIOReader VERSION 0.0.1)

find_package(LCIO REQUIRED)
find_package(podio REQUIRED)
find_package(EDM4HEP REQUIRED)

include(GNUInstallDirs)

# - Define the C++ Standard to use (Simplest Possible)
set(CMAKE_CXX_STANDARD 17 CACHE STRING "")

if(NOT CMAKE_CXX_STANDARD MATCHES "14|17")
    message(FATAL_ERROR "Unsupported C++ standard: ${CMAKE_CXX_STANDARD}")
endif()

message (STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -DDROP_CGAL ${CPP_STANDARD_FLAGS} -Wall -Wextra -Wpedantic -Wno-unused-variable -Wno-unused-parameter")

#--- add version files ---------------------------------------------------------
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/k4LCIOReaderVersion.h
    ${CMAKE_CURRENT_BINARY_DIR}/k4LCIOReaderVersion.h )
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/k4LCIOReaderVersion.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/k4LCIOReader )

#--- add CMake infrastructure --------------------------------------------------
include(cmake/k4LCIOReaderCreateConfig.cmake)

# - library k4LCIOReader
file(GLOB sources src/*.cc)
add_library(k4LCIOReader SHARED ${sources})
target_include_directories(k4LCIOReader PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    ${podio_INCLUDE_DIR}
    ${EDM4HEP_INCLUDE_DIR}
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_link_libraries(k4LCIOReader ${LCIO_LIBRARIES} podio::podio ${EDM4HEP_DIR}/../../libedm4hep.so)
target_compile_options(k4LCIOReader PRIVATE -pthread)

# - test
add_executable(read_lcio test/read_lcio.cc)
target_include_directories(read_lcio PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    ${podio_INCLUDE_DIR})
target_link_libraries(read_lcio k4LCIOReader)
#add_test(NAME read_lcio COMMAND read_lcio)

# - install
install(TARGETS k4LCIOReader
    EXPORT k4LCIOReaderTargets
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT shlib)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/k4LCIOReader DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
