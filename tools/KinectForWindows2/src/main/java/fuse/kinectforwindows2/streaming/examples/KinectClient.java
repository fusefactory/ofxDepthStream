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

import java.nio.ByteBuffer;
import java.nio.FloatBuffer;


import com.jogamp.common.nio.Buffers;
import com.jogamp.opengl.GL2;

import fuse.kinectstreaming.KinectStreamListener;
import fuse.kinectstreaming.KinectStreamReceiver;

public class KinectClient implements KinectStreamListener
{
	public static final int DEPTH_WIDTH = 512;
	public static final int DEPTH_HEIGHT = 424;

	private KinectStreamReceiver kinectReceiver;
	private int textureID;
	private ByteBuffer kinectBuffer;
	private FloatBuffer kinectImage;
	private long frameTime, refreshTime, maxFrameTime;

	public KinectClient(String ip, int port)
	{
		kinectBuffer = Buffers.newDirectByteBuffer(DEPTH_WIDTH * DEPTH_HEIGHT * 4);
	     kinectImage = Buffers.newDirectFloatBuffer(DEPTH_WIDTH * DEPTH_HEIGHT * 4);

	     kinectReceiver = new KinectStreamReceiver(ip, port);
	     kinectReceiver.addListener(this);

	     frameTime = System.currentTimeMillis();
	     refreshTime = 0;
	     maxFrameTime = 0;
	}

	public void start()
	{
		kinectReceiver.start();
	}

	public void stop()
	{
		kinectReceiver.stop();
	}

	public boolean running()
	{
		return kinectReceiver.running();
	}

	public int textureID()
	{
		return textureID;
	}

	public long refreshTime()
	{
		return refreshTime;
	}

	public long maxFrameTime()
	{
		return maxFrameTime;
	}

	public void resetMaxFrameTime()
	{
		maxFrameTime = 0;
	}

	public void init(GL2 gl)
	{
		int[] buffer = new int[1];
		gl.glGenTextures(buffer.length, buffer, 0);
		textureID = buffer[0];
	}

	public void update(GL2 gl)
	{
		kinectBuffer.rewind();

		for (int i = 0; i < kinectBuffer.capacity(); i += 4)
		{
			byte byte0 = kinectBuffer.get(i);
			byte byte1 = kinectBuffer.get(i + 1);
			byte byte2 = kinectBuffer.get(i + 2);
			byte byte3 = kinectBuffer.get(i + 3);
			int depth = byte0 << 24 | (byte1 & 0xFF) << 16 | (byte2 & 0xFF) << 8 | (byte3 & 0xFF);
			if (depth >= 1000 && depth <= 2500)
			{
				kinectImage.put(1);
				kinectImage.put(1);
				kinectImage.put(1);
				kinectImage.put(1);
			}
			else
			{
				kinectImage.put(0);
				kinectImage.put(0);
				kinectImage.put(0);
				kinectImage.put(0);
			}
		}
		kinectImage.rewind();

		gl.glBindTexture(GL2.GL_TEXTURE_2D, textureID);
		gl.glTexParameteri(GL2.GL_TEXTURE_2D, GL2.GL_TEXTURE_MIN_FILTER, GL2.GL_LINEAR);
		gl.glTexParameteri(GL2.GL_TEXTURE_2D, GL2.GL_TEXTURE_MAG_FILTER, GL2.GL_LINEAR);
		gl.glTexImage2D(GL2.GL_TEXTURE_2D, 0, GL2.GL_RGBA, DEPTH_WIDTH, DEPTH_HEIGHT, 0, GL2.GL_RGBA, GL2.GL_FLOAT, kinectImage);

		refreshTime = System.currentTimeMillis() - frameTime;
		if (refreshTime > maxFrameTime) maxFrameTime = refreshTime;
	}

	@Override
	public void newData(byte[] data)
	{
		frameTime = System.currentTimeMillis();
		kinectBuffer.put(data);
		kinectBuffer.rewind();
	}
}
