if(BUILD_LCIOINPUT_IN_GAUDI)
    find_package(EDM4HEP REQUIRED)
    find_package(k4FWCore REQUIRED)
    gaudi_add_module(LCIOInput
                     SOURCES src/components/LCIOInput.cc
                     LINK k4LCIOReader 
                          k4FWCore::k4FWCore 
                          Gaudi::GaudiAlgLib
                          Gaudi::GaudiKernel 
                          EDM4HEP::edm4hep)
endif()
