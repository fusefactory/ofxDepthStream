package fuse.kinectforwindows2;

public class HandJoint extends SkeletonJoint
{
	private HandState handState;
	
	public HandJoint()
	{
		handState = HandState.Unknown;
	}
	
	public HandState handState()
	{
		return handState;
	}
	
	public void setHandState(HandState state)
	{
		handState = state;
	}
}
