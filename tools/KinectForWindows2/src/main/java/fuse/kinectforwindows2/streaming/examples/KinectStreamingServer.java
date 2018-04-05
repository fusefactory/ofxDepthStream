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

package fuse.kinectforwindows2.streaming.examples;

import fuse.kinectforwindows2.KinectV2;
//import fuse.kinectforwindows2.streaming.KinectSkeletonTransmitter;
import fuse.kinectstreaming.KinectDepthMapTransmitter;
import fuse.kinectstreaming.KinectDevice;
import fuse.kinectstreaming.KinectStreamTransmitter;

public class KinectStreamingServer
{
	public static void main(String[] args)
	{
		final KinectV2[] kinect = new KinectV2[1];

		final KinectDevice kinectDevice = new KinectDevice()
		{
			@Override
			public int depthWidth()
			{
				return kinect[0].depthWidth;
			}

			@Override
			public int depthHeight()
			{
				return kinect[0].depthHeight;
			}

			@Override
			public int[] depthMap()
			{
				return kinect[0].depthMap();
			}

			@Override
			public int[] depthImage()
			{
				return kinect[0].depthData();
			}

			@Override
			public void restart()
			{
				if (kinect[0] != null) kinect[0].stop();
				kinect[0] = new KinectV2();
				kinect[0].enableDepth();
//				kinect[0].enableSkeleton3D();
				kinect[0].start();
			}
		};

		new Thread(new Runnable()
		{
			@Override
			public void run()
			{
				kinectDevice.restart();
				KinectStreamTransmitter depthTransmitter = new KinectDepthMapTransmitter(kinectDevice, 4444);
				depthTransmitter.start();
			}
		}).start();

//		new Thread(new Runnable()
//		{
//
//			@Override
//			public void run()
//			{
//				KinectSkeletonTransmitter skeletonTransmitter = new KinectSkeletonTransmitter(kinect[0], 4445);
//				skeletonTransmitter.start();
//			}
//		}).start();
	}
}
