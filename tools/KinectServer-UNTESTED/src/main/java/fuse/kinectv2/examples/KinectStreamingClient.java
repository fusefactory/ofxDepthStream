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

package fuse.kinectv2.examples;

import java.awt.Font;
import java.io.FileInputStream;
import java.nio.ByteBuffer;
import java.nio.FloatBuffer;

import com.jogamp.common.nio.Buffers;
import com.jogamp.nativewindow.ScalableSurface;
import com.jogamp.newt.event.KeyEvent;
import com.jogamp.newt.event.KeyListener;
import com.jogamp.newt.event.WindowAdapter;
import com.jogamp.newt.event.WindowEvent;
import com.jogamp.newt.opengl.GLWindow;
import com.jogamp.opengl.GL2;
import com.jogamp.opengl.GLAutoDrawable;
import com.jogamp.opengl.GLCapabilities;
import com.jogamp.opengl.GLEventListener;
import com.jogamp.opengl.GLProfile;
import com.jogamp.opengl.util.FPSAnimator;
import com.jogamp.opengl.util.awt.TextRenderer;

import fuse.kinectstreaming.KinectStreamListener;
import fuse.kinectstreaming.KinectStreamReceiver;

public class KinectStreamingClient implements GLEventListener, KeyListener, KinectStreamListener
{
	public static void main(String[] args)
	{
		GLProfile profile = GLProfile.get(GLProfile.GL2);
		GLCapabilities caps = new GLCapabilities(profile);

		GLWindow glMainWindow = GLWindow.create(caps);
		final FPSAnimator animator = new FPSAnimator(glMainWindow, 60, true);

		KinectStreamingClient test = new KinectStreamingClient(animator);
		glMainWindow.addGLEventListener(test);
        glMainWindow.addKeyListener(test);
        glMainWindow.setSize(512, 424);
        glMainWindow.setTitle("KINECT STREAMING");
        glMainWindow.setVisible(true);
        glMainWindow.setSurfaceScale(new float [] { ScalableSurface.IDENTITY_PIXELSCALE, ScalableSurface.IDENTITY_PIXELSCALE });

		glMainWindow.addWindowListener(new WindowAdapter()
		{
        	@Override
            public void windowDestroyNotify(WindowEvent event)
        	{
        		if (animator.isStarted()) animator.stop();
                System.exit(0);
            }
		});

		animator.setUpdateFPSFrames(5, null);
		animator.start();
	}

	private FPSAnimator animator;
	private KinectStreamReceiver kinectReicever;
	private long frameTime, maxFrameTime;
	private FloatBuffer kinectImage;
	private ByteBuffer kinectBuffer;
	private int textureID;
	private TextRenderer textRenderer;

	public KinectStreamingClient(FPSAnimator animator)
	{
		this.animator = animator;
		frameTime = System.currentTimeMillis();
		maxFrameTime = 0;
	}

	@Override
	public void init(GLAutoDrawable drawable)
	{
		GL2 gl = drawable.getGL().getGL2();

		int[] buffer = new int[1];
		gl.glGenTextures(buffer.length, buffer, 0);
		textureID = buffer[0];

        kinectBuffer = Buffers.newDirectByteBuffer(512 * 424 * 4);
        kinectImage = Buffers.newDirectFloatBuffer(512 * 424 * 4);
        kinectReicever = new KinectStreamReceiver("127.0.0.1", 4444);
        kinectReicever.addListener(this);
        kinectReicever.start();

        try
		{
        	Font font = Font.createFont(Font.TRUETYPE_FONT, new FileInputStream("data/Gotham-Black.otf")).deriveFont(Font.PLAIN, 16);
        	textRenderer = new TextRenderer(font, true, true);
        	textRenderer.setSmoothing(true);
			textRenderer.setUseVertexArrays(false);
		}
		catch (Exception e)
		{
			e.printStackTrace();
		}
	}

	@Override
	public void reshape(GLAutoDrawable drawable, int x, int y, int width, int height)
	{
		GL2 gl = drawable.getGL().getGL2();
		gl.glMatrixMode(GL2.GL_PROJECTION );
        gl.glLoadIdentity();
        gl.glOrthof(0, width, 0, height, -1, 1);

        gl.glMatrixMode(GL2.GL_MODELVIEW );
        gl.glLoadIdentity();
	}

	@Override
	public void display(GLAutoDrawable drawable)
	{
		GL2 gl = drawable.getGL().getGL2();
		gl.glClearColor(0f, 0f, 0f, 1f);
		gl.glClear(GL2.GL_COLOR_BUFFER_BIT | GL2.GL_DEPTH_BUFFER_BIT);

		kinectBuffer.rewind();

		// depth Image
//		gl.glBindTexture(GL2.GL_TEXTURE_2D, textureID);
//		gl.glTexParameteri(GL2.GL_TEXTURE_2D, GL2.GL_TEXTURE_MIN_FILTER, GL2.GL_LINEAR);
//    	gl.glTexParameteri(GL2.GL_TEXTURE_2D, GL2.GL_TEXTURE_MAG_FILTER, GL2.GL_LINEAR);
//    	gl.glTexImage2D(GL2.GL_TEXTURE_2D, 0, GL2.GL_RGBA, 512, 424, 0, GL2.GL_RGBA, GL2.GL_UNSIGNED_BYTE, kinectBuffer);

		// edge image
		for (int i = 0; i < kinectBuffer.capacity(); i += 4)
		{
			byte byte0 = kinectBuffer.get(i);
			byte byte1 = kinectBuffer.get(i + 1);
			byte byte2 = kinectBuffer.get(i + 2);
			byte byte3 = kinectBuffer.get(i + 3);
			int depth = byte0 << 24 | (byte1 & 0xFF) << 16 | (byte2 & 0xFF) << 8 | (byte3 & 0xFF);
			if (depth >= 2000 && depth <= 4000)
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
		gl.glTexImage2D(GL2.GL_TEXTURE_2D, 0, GL2.GL_RGBA, 512, 424, 0, GL2.GL_RGBA, GL2.GL_FLOAT, kinectImage);

        gl.glColor3f(1, 1, 1);
		gl.glEnable(GL2.GL_TEXTURE_2D);
		gl.glBindTexture(GL2.GL_TEXTURE_2D, textureID);
		gl.glBegin(GL2.GL_QUADS);
		gl.glTexCoord2f(0, 1);
		gl.glVertex2f(0, 0);
		gl.glTexCoord2f(1, 1);
		gl.glVertex2f(drawable.getSurfaceWidth(), 0);
		gl.glTexCoord2f(1, 0);
		gl.glVertex2f(drawable.getSurfaceWidth(), drawable.getSurfaceHeight());
		gl.glTexCoord2f(0, 0);
		gl.glVertex2f(0, drawable.getSurfaceHeight());
		gl.glEnd();
		gl.glBindTexture(GL2.GL_TEXTURE_2D, 0);
		gl.glDisable(GL2.GL_TEXTURE_2D);

		long refreshTime = System.currentTimeMillis() - frameTime;
		if (refreshTime > maxFrameTime) maxFrameTime = refreshTime;

		textRenderer.setColor(1, 0, 0, 1);
		textRenderer.begin3DRendering();
		textRenderer.setColor(0, 0, 1, 1);
		textRenderer.draw("REFRESH TIME: " + refreshTime + " MILLIS", 10, drawable.getSurfaceHeight() - 20);
		textRenderer.draw("MAX REFRESH TIME: " + maxFrameTime + " MILLIS", 10, drawable.getSurfaceHeight() - 50);
		textRenderer.end3DRendering();
		textRenderer.flush();
	}

	@Override
	public void dispose(GLAutoDrawable drawable)
	{
		kinectReicever.stop();
	}

	@Override
	public void keyPressed(KeyEvent event)
	{
		short key = event.getKeyCode();

		if (key == KeyEvent.VK_ESCAPE)
		{
			if (animator.isStarted()) animator.stop();
			System.exit(0);
		}
		else if (key == KeyEvent.VK_SPACE) maxFrameTime = 0;
		else if (key == KeyEvent.VK_ENTER) kinectReicever.restartKinect();
	}

	@Override
	public void keyReleased(KeyEvent keyEvent)
	{
	}

	@Override
	public void newData(byte[] data)
	{
		frameTime = System.currentTimeMillis();
		kinectBuffer.put(data);
		kinectBuffer.rewind();
	}
}
