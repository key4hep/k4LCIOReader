find_package(LCIO REQUIRED)
find_package(podio REQUIRED)
find_package(EDM4HEP REQUIRED)

gaudi_add_library(k4LCIOReader
                 SOURCES src/k4LCIOReader.cc
                         src/k4LCIOConverter.cc
                 LINK ${LCIO_LIBRARIES} 
                      podio::podio
                      EDM4HEP::edm4hep)

target_include_directories(k4LCIOReader PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

# - test
if(BUILD_TESTING)
    add_executable(read_lcio test/read_lcio.cc)
    target_include_directories(read_lcio PUBLIC
        ${podio_INCLUDE_DIR})
    target_link_libraries(read_lcio k4LCIOReader)
    add_test(NAME read_lcio COMMAND read_lcio)
endif()
