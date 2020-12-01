find_package(EDM4HEP REQUIRED)

gaudi_subdir(LCIOInput v0r2)

gaudi_depends_on_subdirs(k4LCIOReader)

gaudi_add_module(LCIOInput  src/components/*.cc
    INCLUDE_DIRS k4FWCore GaudiKernel EDM4HEP::edm4hep EDM4HEP::edm4hepDict
    LINK_LIBRARIES k4LCIOReader k4FWCore GaudiAlgLib GaudiKernel EDM4HEP::edm4hep
    )
