# PerseeServer
Transmits a depth-image stream over a TCP network from the Orbbec Persee.

##### note
_Note that the Makefiles are copied from one of the Sample applications from [orbbec's OpenNI SDK](https://github.com/orbbec/OpenNI2). The builddev.sh script was added for development purposes (to build the application on OSX, with mock-replacements for the OpenNI and OpenCV dependencies)._

## Build instructions

These instructions assume ubuntu (16.04) has been installed on the persee (see main README in this repo). The following instruction should be performed on the Persee command line. You can ssh into the persee with username: ubuntu and password: ' ' (a single space).

#### Install and build de OpenNI SDK
Download "OpenNI 2 for Linux" from http://orbbec3d.org/develop, extract the zip file and then extract the file *OpenNI-Linux-Arm-2.3-without_filter.zip*. cd into the extracted folder.

```bash
# install
chmod +x ./install.sh
sudo ./install.sh
# verify by trying to build a and a sample application
source OpenNIDevEnvironment
cd Samples/SimpleRead
make
cd Bin/Arm-Release/
./SimpleRead
```

#### Build PerseeServer app
Next copy the entire ofxDepthStream folder onto the persee (you can checkout the addon's git repo directly, or use scp: ```scp -r ../ofxDepthStream ubuntu@<persee's IP address>:~/```).

First, make sure you have _sourced_ the OpenNIDevEnvironment by running ```source OpenNIDevEnvironment``` from the OpenNI SDK folder (see instructions above) **do this in the same terminal session in which you run the build commands below**.

Next run the following from the ofxDepthStream folder:
```bash
cd tools/PerseeServer
make
```

#### Run PerseeServer app
```bash
cd Bin/Arm-Release
./PerseeServer
```

#### Configure PerseeServer
Currently de PerseeServer only streams the depth image ([color stream is a work in progress](https://3dclub.orbbec3d.com/t/color-stream-using-opencv-on-persee-with-ubuntu-16-04/1459)).

Command-line arguments:
```bash
  ./PerseeServer --depth-port 4445 # sets port from which depth image is streamed, 4445 by default
  ./PerseeServer --convert-32bit # enables 32-bit conversion, disabled by default (sends 16-bit)
  ./PerseeServer --verbose # enables verbose mode, disabled by default
  ./PerseeServer --fps 30 # configures the framerate (frames/second) to transmit (60 by default, though it's not guaranteed that this framerate is achieved)
```

# Color Stream through OpenCV; (Status: Work In Progress)

Follow these instructions to download, build and install the opencv library on the persee
https://docs.opencv.org/2.4/doc/tutorials/introduction/linux_install/linux_install.html

Make sure to check that the OpenNI2 libraries are found, by first running ```source OpenNIDevEnvironment``` from inside the
OpenNI folder (in the same terminal session in which you run the cmake commands), so in my case that looked like;
```bash
# make sure opencv's cmake can find our OpenNI2 library
cd ~/code/OpenNI-Linux-Arm-2.3
source ./OpenNIDevEnvironment
# create make build-files using cmake
cd ~/opencv
mkdir build
cd build
cmake -D CMAKE_BUILD_TYPE=RELEASE -D CMAKE_INSTALL_PREFIX=/usr/local -D WITH_OPENNI2=ON -D ..
# build opencv with OpenNI2 support
make
sudo make install
```
