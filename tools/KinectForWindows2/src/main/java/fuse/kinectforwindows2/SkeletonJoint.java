package fuse.kinectforwindows2;

import javax.vecmath.Vector3f;
import javax.vecmath.Vector4f;

public class SkeletonJoint
{
	private Vector3f position;
	private Vector4f orientation;
	private TrackingState trackingState;
	
	public SkeletonJoint()
	{
		position = new Vector3f();
		orientation = new Vector4f();
		trackingState = TrackingState.NotTracked;
	}
	
	public Vector3f position()
	{
		return position;
	}
	
	public Vector4f orientation()
	{
		return orientation;
	}
	
	public TrackingState trackingState()
	{
		return trackingState;
	}
	
	public void setTrackingState(TrackingState state)
	{
		trackingState = state;
	}
}
