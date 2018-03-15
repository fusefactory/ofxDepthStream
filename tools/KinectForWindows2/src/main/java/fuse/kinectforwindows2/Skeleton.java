package fuse.kinectforwindows2;

import java.util.HashMap;
import java.util.Map;

public class Skeleton
{
	private int id;
	private Map<JointType, SkeletonJoint> joints;
	
	public Skeleton(int id)
	{
		this.id = id;
		
		joints = new HashMap<>();
		for (JointType type : JointType.values())
		{
			if (type.equals(JointType.HandLeft) || type.equals(JointType.HandRight)) joints.put(type, new HandJoint());
			else joints.put(type, new SkeletonJoint());
		}
	}
	
	public int id()
	{
		return id;
	}

	public SkeletonJoint getJoint(JointType type)
	{
		return joints.get(type);
	}
	
	@Override
	public boolean equals(Object obj)
	{
		if (obj instanceof Skeleton)
		{
			Skeleton skeleton = (Skeleton)obj;
			return skeleton.id() == id;
		}
		return false;
	}
}
