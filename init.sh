# get dependencies from cvmfs (available currently only for centos7)
source /cvmfs/sw-nightlies.hsf.org/key4hep/setup.sh

export CMAKE_PREFIX_PATH=/cvmfs/sw-nightlies.hsf.org/key4hep/packageviews/edm4hep/latest/x86_64-centos7-gcc8-opt:$CMAKE_PREFIX_PATH
