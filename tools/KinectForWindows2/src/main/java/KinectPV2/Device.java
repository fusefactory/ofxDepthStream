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

package KinectPV2;

import java.io.File;

public class Device
{
	static
	{
		int arch = Integer.parseInt(System.getProperty("sun.arch.data.model"));
		String platformName = System.getProperty("os.name");
		platformName = platformName.toLowerCase();
		System.out.println(arch + " " + platformName);

		boolean loaded = false;

		if (arch == 64)
		{
			String libsPath = System.getProperty("kinect.libs.path");
			if(libsPath != null) {
				System.out.println("Found `kinect.libs.path` property: "+libsPath);

				String fname1 = "Kinect20.Face.dll";
				String fname2 = "KinectPV2.dll";

				for (String path : libsPath.split(";"))
				{
					File f1 = new File(path, fname1);
					File f2 = new File(path, fname2);
					if (f1.exists() && f2.exists())
					{
						System.load(f1.getAbsolutePath());
						System.load(f2.getAbsolutePath());
						System.out.println("Loaded KinectV2 libraries");
						loaded = true;
						break;
					}
				}

				if (!loaded) System.err.println("Kinect libraries not found (looking for: "+fname1+" and "+fname2+")");
			}

			if(!loaded) { // try default libs path
				System.loadLibrary("kinect/Kinect20.Face");
				System.loadLibrary("kinect/KinectPV2");
				System.out.println("Loading KinectV2");
			}
		}
		else
		{
			System.out.println("not compatible with 32bits");
		}
	}

	private long ptr;

	public Device()
	{
		jniDevice();
	}

	// ------JNI FUNCTIONS
	private native void jniDevice();
	public native boolean jniInit();
	public native String jniVersion();
	public native boolean jniUpdate();

	// STOP
	public native void jniStopDevice();
	public native boolean jniStopSignal();

	// ENABLE FRAMES
	public native void jniEnableColorFrame(boolean toggle);
	public native void jniEnableDepthFrame(boolean toggle);
	public native void jniEnableDepthMaskFrame(boolean toggle);
	public native void jniEnableInfraredFrame(boolean toggle);
	public native void jniEnableBodyTrackFrame(boolean toggle);
	public native void jniEnableInfraredLongExposure(boolean toggle);
	public native void jniEnableSkeletonDepth(boolean toggle);
	public native void jniEnableSkeletonColor(boolean toggle);
	public native void jniEnableSkeleton3D(boolean toggle);
	public native void jniEnableFaceDetection(boolean toggle);
	public native void jniEnableHDFaceDetection(boolean toggle);
	public native void jniEnablePointCloud(boolean toggle);

	// COLOR CHANNEL
	public native void jniEnableColorChannel(boolean toggle);
	public native float[] jniGetColorChannel();
	public native int[] jniGetColorData();

	// DEPTH
	public native int[] jniGetDepth16Data();
	public native int[] jniGetDepth256Data();

	// DEPTH RAW
	public native int[] jniGetRawDepth16Data();
	public native int[] jniGetRawDepth256Data();
	public native int[] jniGetInfraredData();
	public native int[] jniGetInfraredLongExposure();
	public native int[] jniGetBodyTrack();
	public native int[] jniGetDepthMask();
	public native float[] jniGetSkeleton3D();
	public native float[] jniGetSkeletonDepth();
	public native float[] jniGetSkeletonColor();
	public native float[] jniGetFaceColorData();
	public native float[] jniGetFaceInfraredData();
	public native float[] jniGetHDFaceDetection();

	// POINT CLOUD
	public native float[] jniGetPointCloudDeptMap();
	public native float[] jniGetPointCloudColorMap();
	public native int[] jniGetPointCloudDepthImage();

	// PC THRESHOLDS
	public native void jniSetLowThresholdDepthPC(int val);
	public native int jniGetLowThresholdDepthPC();
	public native void jniSetHighThresholdDepthPC(int val);
	public native int jniGetHighThresholdDepthPC();

	// BODY INDEX
	public native void jniSetNumberOfUsers(int index);
	public native int[] jniGetBodyIndexUser(int index);
	public native int[] jniGetBodyTrackIds();
	public native int[] jniGetRawBodyTrack();
	public native int jniGetNumberOfUsers();

	// MAPERS
	public native float[] jniMapCameraPointToDepthSpace(float camaraSpacePointX, float cameraSpacePointY, float cameraSpacePointZ);
	public native float[] jniMapCameraPointToColorSpace(float camaraSpacePointX, float cameraSpacePointY, float cameraSpacePointZ);
	public native float[] jniGetMapDethToColorSpace();
	public native void jniEnableCoordinateMapperRGBDepth();
}
