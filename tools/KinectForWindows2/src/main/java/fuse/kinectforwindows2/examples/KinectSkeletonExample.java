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

package fuse.kinectforwindows2.examples;

import java.util.List;

import fuse.kinectforwindows2.HandJoint;
import fuse.kinectforwindows2.JointType;
import fuse.kinectforwindows2.KinectV2;
import fuse.kinectforwindows2.Skeleton;
import fuse.kinectforwindows2.SkeletonJoint;

public class KinectSkeletonExample
{
	public static void main(String[] args)
	{
		KinectV2 kinectV2 = new KinectV2();
		kinectV2.enableSkeleton3D();
		kinectV2.start();

		while (true)
		{
			List<Skeleton> skeletons = kinectV2.getSkeleton3D();
			for (Skeleton skeleton : skeletons)
			{
				System.out.println("ID " + skeleton.id());
				for (JointType type : JointType.values())
				{
					SkeletonJoint joint = skeleton.getJoint(type);
//					System.out.println(type + " " + joint.position() + " " + joint.orientation() + " " + joint.trackingState());
					if (joint instanceof HandJoint)
					{
						HandJoint handJoint = (HandJoint)joint;
						System.out.println("Hand " + handJoint.handState());
					}
				}
				System.out.println();
			}
		}
	}
}
