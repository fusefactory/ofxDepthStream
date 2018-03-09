package fuse.kinectforwindows2;

import java.util.ArrayList;
import java.util.List;

import KinectPV2.Device;

public class KinectV2
{
	public final int depthWidth = 512;
    public final int depthHeight = 424;
    public final int colorWidth = 1920;
    public final int colorHeight = 1080;
    public final int bodyCount = 6;
    
    private Device device;
    
    private List<Skeleton> skeletons;
    
	public KinectV2()
	{
		device = new Device();
	}

	/**
	 * Avvia la periferica. Deve essere chiamato come ultimo metodo durante l'inizializzazione.
	 */
	public void start()
	{
		boolean initialized = device.jniInit();
		if (!initialized) System.err.println("Kinect device not initialized!");
	}
	
	/**
	 * Interrompe la periferica.
	 */
	public void stop()
	{
		device.jniStopDevice();
	}
	
	public void enableDepth()
	{
		device.jniEnableDepthFrame(true);
	}
	
	public int[] depthData()
	{
		return device.jniGetDepth256Data();
	}
	
	public int[] depthMap()
	{
		return device.jniGetRawDepth16Data();
	}
	
	public void enableSkeleton3D()
	{
		device.jniEnableSkeleton3D(true);
		skeletons = new ArrayList<>();
		for (int i = 0; i < bodyCount; i++) skeletons.add(new Skeleton(i + 1));
	}

	public List<Skeleton> getSkeleton3D()
	{
		List<Skeleton> trackedSkeletons = new ArrayList<>();
		float[] rawData = device.jniGetSkeleton3D();
		for (int i = 0; i < bodyCount; i++)
		{
			Skeleton skeleton = skeletons.get(i);
			int indexJoint = (i + 1) * (JointType.values().length + 1) * 9 - 1;
			boolean tracked = (rawData[indexJoint] == 1.0);
			if (tracked)
			{
				updateSkeleton(skeleton, rawData, i);
				trackedSkeletons.add(skeleton);
			}
		}
		return trackedSkeletons;
	}
	
	private void updateSkeleton(Skeleton skeleton, float[] rawData, int index)
	{
		int jointOffset = index * (JointType.values().length + 1) * 9;
		for(int i = 0; i < JointType.values().length; i++)
		{
			int jointIndex = i * 9;
			int state = (int)rawData[jointOffset + jointIndex + 7];
			int type = (int)rawData[jointOffset + jointIndex + 8];
			
			JointType jointType = JointType.values()[type];
			SkeletonJoint joint = skeleton.getJoint(jointType);

			joint.position().x = rawData[jointOffset + jointIndex + 0];
			joint.position().y = rawData[jointOffset + jointIndex + 1];
			joint.position().z = rawData[jointOffset + jointIndex + 2];

			joint.orientation().x = rawData[jointOffset + jointIndex + 3];
			joint.orientation().y = rawData[jointOffset + jointIndex + 4];
			joint.orientation().z = rawData[jointOffset + jointIndex + 5];
			joint.orientation().w = rawData[jointOffset + jointIndex + 6];

			TrackingState trackingState = TrackingState.values()[Math.min(state, TrackingState.Tracked.ordinal())];
			joint.setTrackingState(trackingState);
			if (joint instanceof HandJoint)
			{
				HandJoint handJoint = (HandJoint)joint;
				handJoint.setHandState(HandState.values()[state]);
			}
		}
	}
}
