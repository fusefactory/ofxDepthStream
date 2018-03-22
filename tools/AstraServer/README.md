# AstraServer

## Build Instructions
_This app requires Orbbec's Astra SDK, which is NOT included with this repo (TODO: ad as submodule?)_

[Download Orbbec's Astra SDK](https://orbbec3d.com/develop/) (v0.5.0 at time of writing, so far only tested on OS X, High Sierra 10.13.3).

Extract the Astra SDK and make a note of the path of the folder (the folder containing the lib and include folders), for the rest of this README we'll assume this path is ```/Users/mark/AstraSDK```

From the AstraServer folder run the following commands in a terminal:
```bash
mkdir build
cd build
export ASTRA_ROOT=/Users/mark/AstraSDK && cmake ..
make
```

When successful this should create the ```AstraServer``` executable inside the build folder.
