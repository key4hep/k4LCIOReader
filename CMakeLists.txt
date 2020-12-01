cmake_minimum_required(VERSION 3.8)


option(BUILD_LCIOINPUT_IN_GAUDI "whether build the LCIOInput algorithm wrapper in Gaudi" ON)
option(BUILD_TESTING "whether build the test(s)" OFF)

find_package(ROOT COMPONENTS RIO Tree)

#---------------------------------------------------------------
# Load macros and functions for Gaudi-based projects

if( NOT DEFINED ENV{LCG_VERSION})
  set(HOST_BINARY_TAG x86_64-linux-gcc9-opt)
endif()
find_package(GaudiProject)
#---------------------------------------------------------------

include(GNUInstallDirs)

# Set up C++ Standard
# ``-DCMAKE_CXX_STANDARD=<standard>`` when invoking CMake
set(CMAKE_CXX_STANDARD 17 CACHE STRING "")

if(NOT CMAKE_CXX_STANDARD MATCHES "14|17")
  message(FATAL_ERROR "Unsupported C++ standard: ${CMAKE_CXX_STANDARD}")
endif()

gaudi_project(k4LCIOReader v0r2
              USE k4FWCore v0r1
              USE Gaudi v34r0)
