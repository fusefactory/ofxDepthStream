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

package fuse.kinectstreaming;


import java.nio.ByteBuffer;

public class KinectDepthMapTransmitter extends KinectStreamTransmitter
{
	private ByteBuffer byteBuffer;

	public KinectDepthMapTransmitter(KinectDevice kinect, int port)
	{
		super(kinect, port);
		byteBuffer =  ByteBuffer.allocate(kinect.depthWidth() * kinect.depthHeight() * 4);
	}

	@Override
	protected byte[] prepareData()
	{
		int[] depthMap = kinect.depthMap();
		for (int i = 0; i < depthMap.length; i++)
		{
			byteBuffer.putInt(depthMap[i]);
		}
		byteBuffer.rewind();
		return byteBuffer.array();
	}
}
