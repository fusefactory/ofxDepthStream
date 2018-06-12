# RealSenseTransmitter

This tool has been tested with the Intel RealSense D435 on macOS High Sierra (10.13.14)

## Build Preparations

First you have to make sure you have installed the realsense2 SDK.

Follow installation instructions;
https://realsense.intel.com/sdk-2/#install

And make sure you don't only build the SDK, but also _install_ the it, so the headers and libraries are copied to your system-wide ```include``` and ```lib``` folders (because they are not included in this repository).

##### Mac
The [SDK installation instruction for macOS](https://github.com/IntelRealSense/librealsense/blob/master/doc/installation_osx.md) suggest to use XCode. We prefer to use make. To do that, replace that last cmake with the following (removing the "-G Xcode" part at the end);
```sh
cmake .. -DBUILD_EXAMPLES=true -DBUILD_WITH_OPENMP=false -DHWM_OVER_XU=false
```

And then run;
```sh
make
make install
```

## Build Instructions

Make sure you have read the Build Preparation part above and have cmake installed (```brew install cmake```).

Then, from the RealSenseTransmitter folder, run;
```sh
mkdir build
cd build
cmake ..
make
```

This should create and executable called RealSenseTransmitter in your current folder, which you can run like this;
```sh
./RealSenseTransmitter
```

This will start the transmitter app which listens for TCP clients on default port 4445.
