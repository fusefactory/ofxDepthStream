# OrbbecAstraTransmitter

## Build Instructions



### macOS / cmake
_only tested on macOS_

_This app requires Orbbec's Astra SDK, which is NOT included with this repo (TODO: ad as submodule?)_

[Download Orbbec's Astra SDK](https://orbbec3d.com/develop/) (v0.5.0 at time of writing, so far only tested on OS X, High Sierra 10.13.3).

Extract the Astra SDK and make a note of the path of the folder (the folder containing the lib and include folders), for the rest of this README we'll assume this path is ```/Users/mark/AstraSDK```

From the OrbbecAstraTransmitter folder run the following commands in a terminal:
```bash
mkdir build
cd build
export ASTRA_ROOT=/Users/mark/AstraSDK && cmake ..
make
```

When successful this should create the ```OrbbecAstraTransmitter``` executable inside the build folder.




### macOS / Xcode
_This app requires Orbbec's Astra SDK, which is NOT included with this repo (TODO: ad as submodule?)_

[Download Orbbec's Astra SDK](https://orbbec3d.com/develop/) (v0.5.0 at time of writing, so far only tested on OS X, High Sierra 10.13.3).

Extract the Astra SDK and make a note of the path of the folder (the folder containing the lib and include folders), for the rest of this README we'll assume this path is ```/Users/mark/AstraSDK```

You'll need cmake to generate the xcode project files;
You can either download it from https://cmake.org/download/ or use [homebrew](https://brew.sh);
```bash
brew install cmake
```

Then run the following commands from the OrbbecAstraTransmitter folder (replace the ASTRA_ROOT value for);
```bash
mkdir xcode
cd xcode
export ASTRA_ROOT=/Users/mark/AstraSDK && cmake -G Xcode ..
```

This should create an ```./xcode``` folder with XCode project files.


### Windows / Visual Studio 2015
See project files in the ```vs2015``` folder. All required dependencies (zlib and astra SDK libs+headers) are included. Binaries are generated in the ```vs2015/bin/``` folder.



## Options

```bash
# -v, --verbose; enable verbose mode
OrbbecAstraTransmitter -v

# -c --convert-32bit; transmit 32-bit instead of 16-bit depth-frames (each pixel 4 bytes instead of 2 bytes)
OrbbecAstraTransmitter -c

# -d --depth-port; set port from which to stream
OrbbecAstraTransmitter -d 8080

# -f --fps; set framerate (not guaranteed this rate is achieved)
OrbbecAstraTransmitter -f 30
```
