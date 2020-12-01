find_package(LCIO REQUIRED)
find_package(podio REQUIRED)
find_package(EDM4HEP REQUIRED)

gaudi_subdir(k4LCIOReader v0r2)

gaudi_add_library(k4LCIOReader
                 src/*.cc
                 INCLUDE_DIRS ${LCIO_INCLUDE_DIRS} podio::podio EDM4HEP::edm4hep
                 LINK_LIBRARIES ${LCIO_LIBRARIES} podio::podio EDM4HEP::edm4hep)

gaudi_install_headers(k4LCIOReader)

# - test
if(BUILD_TESTING)
    add_executable(read_lcio test/read_lcio.cc)
    target_include_directories(read_lcio PUBLIC
        ${podio_INCLUDE_DIR})
    target_link_libraries(read_lcio k4LCIOReader)
    add_test(NAME read_lcio COMMAND read_lcio)
endif()
