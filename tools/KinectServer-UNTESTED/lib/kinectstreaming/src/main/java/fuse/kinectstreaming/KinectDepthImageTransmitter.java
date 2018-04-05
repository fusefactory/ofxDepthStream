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

public class KinectDepthImageTransmitter extends KinectStreamTransmitter
{
	private ByteBuffer byteBuffer;

	public KinectDepthImageTransmitter(KinectDevice kinect, int port)
	{
		super(kinect, port);
		byteBuffer = ByteBuffer.allocate(kinect.depthWidth() * kinect.depthHeight() * 4);
	}

	@Override
	protected byte[] prepareData()
	{
		int[] depthData = kinect.depthImage();
		for (int i = 0; i < depthData.length; i++)
		{
			int argb = depthData[i];
			byte r = (byte)(argb >> 16 & 0xFF);
			byte g = (byte)(argb >> 8 & 0xFF);
			byte b = (byte)(argb & 0xFF);
			byte a = (byte)(argb >> 24 & 0xFF);
			byteBuffer.put(r);
			byteBuffer.put(g);
			byteBuffer.put(b);
			byteBuffer.put(a);
		}
		byteBuffer.rewind();
		return byteBuffer.array();
	}
}
