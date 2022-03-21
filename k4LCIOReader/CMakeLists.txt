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
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

# - test
if(BUILD_TESTING)
    add_executable(k4lcioreader_read-lcio test/read-lcio.cc)
    target_include_directories(k4lcioreader_read-lcio PUBLIC
        ${podio_INCLUDE_DIR})
    target_link_libraries(k4lcioreader_read-lcio k4LCIOReader)
    add_test(NAME prepare-input-files COMMAND wget -N https://key4hep.web.cern.ch/key4hep/testFiles/ddsimOutput/testSimulation.slcio)
    set_tests_properties(prepare-input-files PROPERTIES FIXTURES_SETUP external_test_files)
    add_test(NAME read-lcio COMMAND k4lcioreader_read-lcio testSimulation.slcio)
    set_tests_properties(read-lcio PROPERTIES FIXTURES_REQUIRED external_test_files)
endif()

install(TARGETS k4LCIOReader
  EXPORT k4LCIOReaderTargets
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT bin
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT shlib
  COMPONENT dev)
