# Compile
```shell
mkdir build & cd build
```
Requirments:
+ `ROOT`:
  + manual install: if the ROOT cannot be found, you should use the file `cmake/FindROOT.cmake` (`include(/usr/local/sklib_gcc8/root_v5.34.38/build_cmake/ROOTConfig.cmake)`) with the path subsitute with the ROOTconfig path in your OS system
  + The ROOT can be found or package install, just remove the `cmake/FindROOT.cmake`
+ `HDF5`: manual install (download from [here](https://www.hdfgroup.org/downloads/hdf5/) according to your OS otherwise you may envounter `glibc version error`) or from package manager
  + manual install hdf5 lib: `HDF5_ROOT=/home/aiqiang/playground/hdf/HDF_Group/HDF5/1.14.1 cmake ..`
  + package install: `cmake ..`
+ compile: `make`
