# - Use CMake's module to help generating relocatable config files
include(CMakePackageConfigHelpers)

# - Versioning
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/k4LCIOReaderConfigVersion.cmake
    VERSION ${k4LCIOReader_VERSION}
    COMPATIBILITY SameMajorVersion)

# - Install time config and target files
configure_package_config_file(${CMAKE_CURRENT_LIST_DIR}/k4LCIOReaderConfig.cmake.in
    "${PROJECT_BINARY_DIR}/k4LCIOReaderConfig.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/k4LCIOReader"
    PATH_VARS
    CMAKE_INSTALL_BINDIR
    CMAKE_INSTALL_INCLUDEDIR
    CMAKE_INSTALL_LIBDIR
    )

# - install and export
install(FILES
    "${PROJECT_BINARY_DIR}/k4LCIOReaderConfigVersion.cmake"
    "${PROJECT_BINARY_DIR}/k4LCIOReaderConfig.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/k4LCIOReader"
    )
install(EXPORT k4LCIOReaderTargets
    NAMESPACE k4LCIOReader::
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/k4LCIOReader"
    )

