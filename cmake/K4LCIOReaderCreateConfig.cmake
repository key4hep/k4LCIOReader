# - Use CMake's module to help generating relocatable config files
include(CMakePackageConfigHelpers)

# - Versioning
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/K4LCIOReaderConfigVersion.cmake
    VERSION ${K4LCIOReader_VERSION}
    COMPATIBILITY SameMajorVersion)

# - Install time config and target files
configure_package_config_file(${CMAKE_CURRENT_LIST_DIR}/K4LCIOReaderConfig.cmake.in
    "${PROJECT_BINARY_DIR}/K4LCIOReaderConfig.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/K4LCIOReader"
    PATH_VARS
    CMAKE_INSTALL_BINDIR
    CMAKE_INSTALL_INCLUDEDIR
    CMAKE_INSTALL_LIBDIR
    )

# - install and export
install(FILES
    "${PROJECT_BINARY_DIR}/K4LCIOReaderConfigVersion.cmake"
    "${PROJECT_BINARY_DIR}/K4LCIOReaderConfig.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/K4LCIOReader"
    )
install(EXPORT K4LCIOReaderTargets
    NAMESPACE K4LCIOReader::
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/K4LCIOReader"
    )

