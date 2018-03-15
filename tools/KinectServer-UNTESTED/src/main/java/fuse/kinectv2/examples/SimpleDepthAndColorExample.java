package fuse.kinectv2.examples;

import java.nio.FloatBuffer;
import java.nio.IntBuffer;

import com.jogamp.common.nio.Buffers;
import com.jogamp.nativewindow.ScalableSurface;
import com.jogamp.newt.Display;
import com.jogamp.newt.NewtFactory;
import com.jogamp.newt.Screen;
import com.jogamp.newt.event.KeyEvent;
import com.jogamp.newt.event.KeyListener;
import com.jogamp.newt.opengl.GLWindow;
import com.jogamp.opengl.GL2;
import com.jogamp.opengl.GLAutoDrawable;
import com.jogamp.opengl.GLCapabilities;
import com.jogamp.opengl.GLEventListener;
import com.jogamp.opengl.GLProfile;
import com.jogamp.opengl.util.FPSAnimator;

import fuse.kinectv2.KinectV2;

public class SimpleDepthAndColorExample implements GLEventListener, KeyListener
{
	public static void main(String[] args)
	{
		GLProfile profile = GLProfile.getDefault();
		GLCapabilities caps = new GLCapabilities(profile);
		
		Display display = NewtFactory.createDisplay(null);
		Screen screen = NewtFactory.createScreen(display, 0);
		screen.addReference();
		
		GLWindow glWindow = GLWindow.create(caps);
		FPSAnimator animator = new FPSAnimator(glWindow, 60, true);
		
		SimpleDepthAndColorExample renderer = new SimpleDepthAndColorExample(animator);
		glWindow.addGLEventListener(renderer);
        glWindow.addKeyListener(renderer);
        glWindow.setSize(512 + (int)(1920 / 1080f * 424) + 512, 424);
        glWindow.setVisible(true);
        glWindow.setSurfaceScale(new float [] { ScalableSurface.IDENTITY_PIXELSCALE, ScalableSurface.IDENTITY_PIXELSCALE });
        
        animator.setUpdateFPSFrames(10, null);
        animator.start();
	}
	
	private FPSAnimator animator;
	private int depthID, rgbID, registeredID;
	private KinectV2 kinect;
	private FloatBuffer depthBuffer;
	private FloatBuffer rgbBuffer;
	private FloatBuffer registeredBuffer;
	
	public SimpleDepthAndColorExample(FPSAnimator animator) 
	{
		this.animator = animator;
	}
	
	@Override
	public void init(GLAutoDrawable drawable)
	{
		GL2 gl = drawable.getGL().getGL2();
		gl.glEnable(GL2.GL_BLEND);
		gl.glBlendFunc(GL2.GL_SRC_ALPHA, GL2.GL_ONE_MINUS_SRC_ALPHA);
		gl.glEnable(GL2.GL_LINE_SMOOTH);
		gl.glHint(GL2.GL_LINE_SMOOTH_HINT, GL2.GL_DONT_CARE);
		gl.glHint(GL2.GL_PERSPECTIVE_CORRECTION_HINT, GL2.GL_NICEST);
		
		int[] buffer = new int[3];
		gl.glGenTextures(3, buffer, 0); 
		depthID = buffer[0];
		rgbID = buffer[1];
		registeredID = buffer[2];
		
		kinect = new KinectV2();
		kinect.enableDepth();
		kinect.enableRgb();
		kinect.enableRegistered();
		kinect.start();
		
		depthBuffer = Buffers.newDirectFloatBuffer(kinect.depthWidth * kinect.depthHeight * 4);
		rgbBuffer = Buffers.newDirectFloatBuffer(kinect.colorWidth * kinect.colorHeight * 4);
		registeredBuffer = Buffers.newDirectFloatBuffer(kinect.depthWidth * kinect.depthHeight * 4);
	}

	@Override
	public void display(GLAutoDrawable drawable)
	{
		IntBuffer depth = kinect.depthImage();
		for (int y = 0; y < kinect.depthHeight; y++)
		{
			for (int x = 0; x < kinect.depthWidth; x++)
			{
				int index = x + y * kinect.depthWidth;
				int rgba = depth.get(index);
				int r = rgba >> 16 & 0xFF;
		    	int g = rgba >> 8 & 0xFF;
		    	int b = rgba & 0xFF;
		    	int a = rgba >> 24 & 0xFF;
		    	depthBuffer.put(r / 255f);
		    	depthBuffer.put(g / 255f);
		    	depthBuffer.put(b / 255f);
		    	depthBuffer.put(a / 255f);
			}
		}
		depthBuffer.rewind();
		
		IntBuffer color = kinect.rgbImage();
		for (int y = 0; y < kinect.colorHeight; y++)
		{
			for (int x = 0; x < kinect.colorWidth; x++)
			{
				int index = x + y * kinect.colorWidth;
				int rgba = color.get(index);
				int r = rgba >> 16 & 0xFF;
		    	int g = rgba >> 8 & 0xFF;
		    	int b = rgba & 0xFF;
		    	int a = rgba >> 24 & 0xFF;
		    	rgbBuffer.put(r / 255f);
		    	rgbBuffer.put(g / 255f);
		    	rgbBuffer.put(b / 255f);
		    	rgbBuffer.put(a / 255f);
			}
		}
		rgbBuffer.rewind();
		
		IntBuffer registered = kinect.registeredImage();
		for (int y = 0; y < kinect.depthHeight; y++)
		{
			for (int x = 0; x < kinect.depthWidth; x++)
			{
				int index = x + y * kinect.depthWidth;
				int rgba = registered.get(index);
				int r = rgba >> 16 & 0xFF;
		    	int g = rgba >> 8 & 0xFF;
		    	int b = rgba & 0xFF;
		    	int a = rgba >> 24 & 0xFF;
		    	registeredBuffer.put(r / 255f);
		    	registeredBuffer.put(g / 255f);
		    	registeredBuffer.put(b / 255f);
		    	registeredBuffer.put(a / 255f);
			}
		}
		registeredBuffer.rewind();
		
		GL2 gl = drawable.getGL().getGL2();
		gl.glClearColor(0f, 0f, 0f, 0f);
		gl.glClear(GL2.GL_COLOR_BUFFER_BIT | GL2.GL_DEPTH_BUFFER_BIT);
		
		gl.glBindTexture(GL2.GL_TEXTURE_2D, depthID);
        gl.glTexParameteri(GL2.GL_TEXTURE_2D, GL2.GL_TEXTURE_MIN_FILTER, GL2.GL_LINEAR);
    	gl.glTexParameteri(GL2.GL_TEXTURE_2D, GL2.GL_TEXTURE_MAG_FILTER, GL2.GL_LINEAR);
        gl.glTexImage2D(GL2.GL_TEXTURE_2D, 0, GL2.GL_RGBA, kinect.depthWidth, kinect.depthHeight, 0, GL2.GL_RGBA, GL2.GL_FLOAT, depthBuffer);
        gl.glBindTexture(GL2.GL_TEXTURE_2D, rgbID);
        gl.glTexParameteri(GL2.GL_TEXTURE_2D, GL2.GL_TEXTURE_MIN_FILTER, GL2.GL_LINEAR);
    	gl.glTexParameteri(GL2.GL_TEXTURE_2D, GL2.GL_TEXTURE_MAG_FILTER, GL2.GL_LINEAR);
        gl.glTexImage2D(GL2.GL_TEXTURE_2D, 0, GL2.GL_RGBA, kinect.colorWidth, kinect.colorHeight, 0, GL2.GL_RGBA, GL2.GL_FLOAT, rgbBuffer);
        gl.glBindTexture(GL2.GL_TEXTURE_2D, registeredID);
        gl.glTexParameteri(GL2.GL_TEXTURE_2D, GL2.GL_TEXTURE_MIN_FILTER, GL2.GL_LINEAR);
    	gl.glTexParameteri(GL2.GL_TEXTURE_2D, GL2.GL_TEXTURE_MAG_FILTER, GL2.GL_LINEAR);
        gl.glTexImage2D(GL2.GL_TEXTURE_2D, 0, GL2.GL_RGBA, kinect.depthWidth, kinect.depthHeight, 0, GL2.GL_RGBA, GL2.GL_FLOAT, registeredBuffer);
        gl.glBindTexture(GL2.GL_TEXTURE_2D, 0);
        
 		gl.glColor3f(1, 1, 1);
 		gl.glEnable(GL2.GL_TEXTURE_2D);
 		gl.glBindTexture(GL2.GL_TEXTURE_2D, depthID);
 		gl.glBegin(GL2.GL_QUADS);
 		gl.glTexCoord2d(0, 0);
 		gl.glVertex2d(0.0, 0.0);
 		gl.glTexCoord2d(1, 0);
 		gl.glVertex2d(kinect.depthWidth, 0.0);
 		gl.glTexCoord2d(1, 1);
 		gl.glVertex2d(kinect.depthWidth, kinect.depthHeight);
 		gl.glTexCoord2d(0, 1);
 		gl.glVertex2d(0.0, kinect.depthHeight);
 		gl.glEnd();
 		
 		gl.glBindTexture(GL2.GL_TEXTURE_2D, rgbID);
 		gl.glBegin(GL2.GL_QUADS);
 		gl.glTexCoord2d(0, 0);
 		gl.glVertex2d(kinect.depthWidth, 0.0);
 		gl.glTexCoord2d(1, 0);
 		gl.glVertex2d(drawable.getSurfaceWidth() - kinect.depthWidth, 0.0);
 		gl.glTexCoord2d(1, 1);
 		gl.glVertex2d(drawable.getSurfaceWidth() - kinect.depthWidth, kinect.depthHeight);
 		gl.glTexCoord2d(0, 1);
 		gl.glVertex2d(kinect.depthWidth, kinect.depthHeight);
 		gl.glEnd();
 		gl.glBindTexture(GL2.GL_TEXTURE_2D, 0);
 		
 		gl.glDisable(GL2.GL_TEXTURE_2D);
 		gl.glColor3f(1, 1, 1);
 		gl.glEnable(GL2.GL_TEXTURE_2D);
 		gl.glBindTexture(GL2.GL_TEXTURE_2D, registeredID);
 		gl.glBegin(GL2.GL_QUADS);
 		gl.glTexCoord2d(0, 0);
 		gl.glVertex2d(drawable.getSurfaceWidth() - kinect.depthWidth, 0.0);
 		gl.glTexCoord2d(1, 0);
 		gl.glVertex2d(drawable.getSurfaceWidth(), 0.0);
 		gl.glTexCoord2d(1, 1);
 		gl.glVertex2d(drawable.getSurfaceWidth(), kinect.depthHeight);
 		gl.glTexCoord2d(0, 1);
 		gl.glVertex2d(drawable.getSurfaceWidth() - kinect.depthWidth, kinect.depthHeight);
 		gl.glEnd();
	}

	@Override
	public void reshape(GLAutoDrawable drawable, int x, int y, int width, int height)
	{
		GL2 gl = drawable.getGL().getGL2();
		gl.glMatrixMode(GL2.GL_PROJECTION);
        gl.glLoadIdentity();
        
        gl.glOrtho(0, width, height, 0, -1, 1);
        
        gl.glMatrixMode(GL2.GL_MODELVIEW);
        gl.glLoadIdentity();
	}
	
	@Override
	public void dispose(GLAutoDrawable drawable)
	{
	}

	@Override
	public void keyPressed(KeyEvent event)
	{
		short key = event.getKeyCode();
		
		if (key == KeyEvent.VK_ESCAPE)
		{
			if (animator.isStarted())
			{
				animator.stop();
				kinect.stop();
			}
            System.exit(0);
		}
		else if (key == KeyEvent.VK_SPACE) kinect.setMirror(!kinect.getMirror());
	}

	@Override
	public void keyReleased(KeyEvent event)
	{
	}
}
