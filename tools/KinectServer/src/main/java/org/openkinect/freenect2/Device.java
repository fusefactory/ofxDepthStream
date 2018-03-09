package org.openkinect.freenect2;

import java.io.File;

public class Device
{
	static
	{
		boolean found = false;
		String osName = System.getProperty("os.name").toLowerCase();
		if (osName.indexOf("win") >= 0)
		{
			String libraryPath = System.getProperty("java.library.path");
			for (String path : libraryPath.split(";"))
			{
				if (new File(path + "\\win\\libJNILibfreenect2.dll").exists())
				{
					System.loadLibrary("win/turbojpeg");
					System.loadLibrary("win/libusb-1.0");
					System.loadLibrary("win/libJNILibfreenect2");
					found = true;
					break;
				}
			}
			if (!found) System.err.println("KinectV2 library not found");
		}
		else if (osName.indexOf("mac") >= 0)
		{
			String libraryPath = System.getProperty("java.library.path");
			for (String path : libraryPath.split(":"))
			{
				if (new File(path + "/mac/libJNILibfreenect2.dylib").exists())
				{
					System.load(path + "/mac/libJNILibfreenect2.dylib");
					found = true;
					break;
				}
			}
			if (!found) System.err.println("KinectV2 library not found");
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
