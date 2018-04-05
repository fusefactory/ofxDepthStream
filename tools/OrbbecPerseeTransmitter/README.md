# OrbbecPerseeTransmitter
Transmits a depth-image stream over a TCP network from the Orbbec Persee.

##### note
_Note that the Makefiles are copied from one of the Sample applications from [orbbec's OpenNI SDK](https://github.com/orbbec/OpenNI2). The builddev.sh script was added for development purposes (to build the application on OSX, with mock-replacements for the OpenNI and OpenCV dependencies)._

## Instructions

### Install Ubuntu
This application needs to run on Ubuntu, not Android which the comes pre-installed on the Persee. These are the instruction to install Orbbec's Ubuntu 16.04 image on the Persee.

There is very little official documentation and your best source for information is [orbbec's forum](https://3dclub.orbbec3d.com): ["Universal download thread" where you can find the Ubuntu image (instructions included)](https://3dclub.orbbec3d.com/t/universal-download-thread-for-persee/694). _Spoiler: you'll need a windows machine to run the tools necessary to install the image (running window inside VirtualBox didn't work for me) \o/_

Once you've installed Ubuntu you can SSH into the device (default user: ubuntu, default password: ' ' [a single space]), and continue with a common linux command-line workflow (when using OpenFrameworks you'll need to grab the "linux armv7" version of OpenFrameworks);

### Install and build de OpenNI SDK
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

### Build OrbbecPerseeTransmitter app
Next copy the entire ofxDepthStream folder onto the persee (you can checkout the addon's git repo directly, or use scp: ```scp -r ../ofxDepthStream ubuntu@<persee's IP address>:~/```).

First, make sure you have _sourced_ the OpenNIDevEnvironment by running ```source OpenNIDevEnvironment``` from the OpenNI SDK folder (see instructions above) **do this in the same terminal session in which you run the build commands below**.

Next run the following from the ofxDepthStream folder:
```bash
cd tools/OrbbecPerseeTransmitter
make
```

### Run OrbbecPerseeTransmitter app
```bash
cd Bin/Arm-Release
./OrbbecPerseeTransmitter
```

### OrbbecPerseeTransmitter args
Currently de OrbbecPerseeTransmitter only streams the depth image ([color stream is a work in progress](https://3dclub.orbbec3d.com/t/color-stream-using-opencv-on-persee-with-ubuntu-16-04/1459)).

Command-line arguments:
```bash
./OrbbecPerseeTransmitter --convert-32bit # enables 32-bit conversion, disabled by default (sends 16-bit)
./OrbbecPerseeTransmitter -c
./OrbbecPerseeTransmitter --depth-port 4445 # sets port from which depth image is streamed, 4445 by default
./OrbbecPerseeTransmitter -d 4445
./OrbbecPerseeTransmitter --fps 30 # configures the framerate (frames/second) to transmit (60 by default, though it's not guaranteed that this framerate is achieved)
./OrbbecPerseeTransmitter -f 30
./OrbbecPerseeTransmitter --verbose # enables verbose mode, disabled by default
./OrbbecPerseeTransmitter -v
```

### Run OrbbecPerseeTransmitter at startup
Our prefered method is to create a startup.sh script in our user's home folder, and call that from /etc/rc.local,
so it's easy to change the startup script (without needing to use SUDO after the initial setup). The instructions below assume you are using the default ```ubuntu``` user account.

##### create a startup.sh script in your user's home folder
Create a file at ```/home/ubuntu/startup.sh``` with the below content. This is the script we'll configure to be executed at startup. It will run in the backgorund and log the application's output to a log file in the ```/var/log/``` system folder.
```bash
cd /home/ubuntu/ofxDepthStream/tools/OrbbecPerseeTransmitter/Bin/Arm-Release && ./OrbbecPerseeTransmitter >> /var/log/OrbbecPerseeTransmitter
```

##### register startup.sh to run at startup
Add the content below to the file ```/etc/rc.local``` on the persee. You'll have to use ```sudo``` to edit the file. Make sure to add these lines **BEFORE** the ```exit 0``` line at then end of the file. Note that this script will first check if the startup.sh script exists, so if it (accidentally?) gets (re-)moved, this will not cause any startup issues.
```bash
if [ -f /home/ubuntu/startup.sh ]; then
  /home/ubuntu/startup.sh &
fi
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
