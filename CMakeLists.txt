cmake_minimum_required(VERSION 3.8)

project(K4LCIOReader VERSION 0.0.1)

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
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/K4LCIOReaderVersion.h
    ${CMAKE_CURRENT_BINARY_DIR}/K4LCIOReaderVersion.h )
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/K4LCIOReaderVersion.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/K4LCIOReader )

#--- add CMake infrastructure --------------------------------------------------
include(cmake/K4LCIOReaderCreateConfig.cmake)

# - library K4LCIOReader
file(GLOB sources src/*.cc)
add_library(K4LCIOReader SHARED ${sources})
target_include_directories(K4LCIOReader PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    ${podio_INCLUDE_DIR}
    ${EDM4HEP_INCLUDE_DIR}
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_link_libraries(K4LCIOReader ${LCIO_LIBRARIES} podio::podio ${EDM4HEP_DIR}/../../libedm4hep.so)
target_compile_options(K4LCIOReader PRIVATE -pthread)

# - test
add_executable(read_lcio test/read_lcio.cc)
target_include_directories(read_lcio PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    ${podio_INCLUDE_DIR})
target_link_libraries(read_lcio K4LCIOReader)
#add_test(NAME read_lcio COMMAND read_lcio)

# - install
install(TARGETS K4LCIOReader
    EXPORT K4LCIOReaderTargets
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT shlib)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/K4LCIOReader DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
