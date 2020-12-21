# - Config file for the k4LCIOReader package

include(CMakeFindDependencyMacro)
find_dependency(podio REQUIRED)
find_dependency(Gaudi REQUIRED)
find_dependency(k4FWCore REQUIRED)
find_dependency(EDM4HEP REQUIRED)

# - Include the targets file to create the imported targets that a client can
# link to (libraries) or execute (programs)
include("${CMAKE_CURRENT_LIST_DIR}/k4LCIOReaderTargets.cmake")

get_property(TEST_K4LCIOREADER_LIBRARY TARGET k4LCIOReader::k4LCIOReader PROPERTY LOCATION)
find_package_handle_standard_args(k4LCIOReader DEFAULT_MSG CMAKE_CURRENT_LIST_FILE TEST_K4LCIOREADER_LIBRARY)
