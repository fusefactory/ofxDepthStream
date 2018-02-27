# PerseeServer
Note that the Makefiles are copied from one of the Sample applications from [orbbec's OpenNI SDK](https://github.com/orbbec/OpenNI2). The builddev.sh script was added for development purposes (to build the application on OSX).

# Build instructions

These instructions assume ubuntu (16.04) has been installed on the persee (see main README). The following instruction should be performed on the Persee command line. You can ssh into the persee with username: ubuntu and password: ' ' (a single space).

#### _First install and build de OpenNI SDK_
Download "OpenNI 2 for Linux" from http://orbbec3d.org/develop, extract the zip file and then extract the file *OpenNI-Linux-Arm-2.3-without_filter.zip*. cd into the extracted folder.

```bash
# install
chmod +x ./install.sh
sudo ./install.sh
source OpenNIDevEnvironment
# verify by trying to build a and a sample application
cd Samples/SimpleRead
make
cd Bin/Arm-Release/
./SimpleRead
```

#### _Build PerseeServer app_
Next copy the entire ofxOrbbecPersee folder onto the persee (you can checkout the addon's git repo direcetly, or use for example scp: ```scp -r ofxOrbbecPersee ubuntu@<persee's IP address>:~/```).

First, make sure you have _sourced_ the OpenNIDevEnviroenment by running ```source OpenNIDevEnvironment``` from the OpenNI SDK folder (see instructions above) **from your current terminal session**.

Next run the following from the ofxOrbbecPersee folder:
```bash
# build
cd tools/PerseeServer
make
# run
cd Bin/Arm-Release
./PerseeServer
```


# Color Stream through OpenCV; (Status: Work In Progress)

Follow these instructions to download, build and install the opencv library on the persee
https://docs.opencv.org/2.4/doc/tutorials/introduction/linux_install/linux_install.html
