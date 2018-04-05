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
