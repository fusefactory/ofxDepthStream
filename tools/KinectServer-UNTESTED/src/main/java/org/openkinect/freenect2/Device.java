//
//  This file is part of the ofxDepthStream [https://github.com/fusefactory/ofxDepthStream]
//  Copyright (C) 2018 Fuse srl
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
//

package org.openkinect.freenect2;

import java.io.File;

public class Device
{
	static
	{
		boolean found = false;
		String osName = System.getProperty("os.name").toLowerCase();
		String libraryPath = System.getProperty("freenect.libs.path");


		if (osName.indexOf("win") >= 0)
		{
		  String relativeName = "win\\libJNILibfreenect2.dll";
			for (String path : libraryPath.split(";"))
			{
			  File f = new File(path, relativeName);
				if (f.exists())
				{
				  System.load(new File(path, "win\\turbojpeg.dll").getAbsolutePath());
				  System.load(new File(path, "win\\libusb-1.0.dll").getAbsolutePath());
          System.load(f.getAbsolutePath());
//					System.loadLibrary("win/turbojpeg");
//					System.loadLibrary("win/libusb-1.0");
//					System.loadLibrary("win/libJNILibfreenect2");
					found = true;
					break;
				}
			}
			if (!found) System.err.println("KinectV2 library not found (java.library.path: "+libraryPath+")");
		}
		else if (osName.indexOf("mac") >= 0)
		{
		  String relativeName = "mac/libJNILibfreenect2.dylib";

			for (String path : libraryPath.split(";"))
			{
				File f = new File(path, relativeName);
				if (f.exists())
				{
					System.load(f.getAbsolutePath());
					found = true;
					break;
				}
			}
			if (!found) System.err.println("KinectV2 library not found (looking for: "+relativeName+" in freenect.libs.path: "+libraryPath+")");
		}
		else
		{
			System.err.println("KinectV2 not compatible with the current OS or is a 32 bit system");
		}
	}

	private long ptr;

	public Device()
	{
		ptr = jniInit();
	}

	// JNI functions
    public native long jniInit();
    public native void jniOpen();
    public native void jniOpenM(int indexKinect);
    public native void jniOpenMV(int indexKinect, int indexVideo); // 0 cpu 1 integrata 2 esterna
    public native void jniStop();
    public native void jniEumerateDevices();
    public native void jniUpdateDevice();
    public native boolean jniDeviceReady();

    // multiple kinect funtions
    public native void jniOpenSerial(String serialNumber);
    public native int jniGetNumDevices();
    public native String jniGetSerialDevice(int index);

    // get data functions
    public native int[] jniGetDepthData();
    public native int[] jniGetRawDepthData();

    public native int[] jniGetIrData();
    public native int[] jniGetColorData();
    public native int[] jniGetUndistorted();
    public native int[] jniGetRegistered();
    public native float[] jniGetDepthCameraPositions();

    // enables functions
    public native void jniEnableVideo(boolean enable);
    public native void jniEnableDepth(boolean enable);
    public native void jniEnableIR(boolean enable);
    public native void jniEnableRegistered(boolean enable);
}
