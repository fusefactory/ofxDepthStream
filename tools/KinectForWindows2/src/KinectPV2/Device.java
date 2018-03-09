package KinectPV2;

public class Device
{
	static
	{
		int arch = Integer.parseInt(System.getProperty("sun.arch.data.model"));
		String platformName = System.getProperty("os.name");
		platformName = platformName.toLowerCase();
		System.out.println(arch + " " + platformName);
		if (arch == 64)
		{
			System.loadLibrary("Kinect20.Face");
			System.loadLibrary("KinectPV2");
			System.out.println("Loading KinectV2");
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
