package fuse.kinectforwindows2.streaming.examples;

import java.awt.Font;
import java.io.FileInputStream;
import java.util.ArrayList;
import java.util.List;

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

public class MultiKinectStreamingClient implements GLEventListener, KeyListener
{
	private static final int DEVICES = 3;
	
	public class KinectConfig
	{
		public String ip;
		public int port;
		
		public KinectConfig(String ip, int port)
		{
			this.ip = ip;
			this.port = port;
		}
	}
	
	public static void main(String[] args)
	{
		GLProfile profile = GLProfile.get(GLProfile.GL2);
		GLCapabilities caps = new GLCapabilities(profile);
		
		GLWindow glMainWindow = GLWindow.create(caps);
		final FPSAnimator animator = new FPSAnimator(glMainWindow, 60, true);
		
		MultiKinectStreamingClient test = new MultiKinectStreamingClient(animator);
		glMainWindow.addGLEventListener(test);
        glMainWindow.addKeyListener(test);
        glMainWindow.setSize(KinectClient.DEPTH_WIDTH * DEVICES, KinectClient.DEPTH_HEIGHT);
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
	private List<KinectClient> kinectClients;
	private TextRenderer textRenderer;
	
	private KinectConfig[] kinectConfigs = new KinectConfig[] {
			new KinectConfig("192.168.1.76", 4444),
			new KinectConfig("192.168.1.75", 4444),
			new KinectConfig("192.168.1.74", 4444),
	};
	
	public MultiKinectStreamingClient(FPSAnimator animator)
	{
		this.animator = animator;
	}
	
	@Override
	public void init(GLAutoDrawable drawable)
	{
		GL2 gl = drawable.getGL().getGL2();
		gl.glEnable(GL2.GL_BLEND);
		gl.glBlendFunc(GL2.GL_SRC_ALPHA, GL2.GL_ONE_MINUS_SRC_ALPHA);
		
		kinectClients = new ArrayList<>();
		for (int i = 0; i < DEVICES; i++)
		{
			KinectConfig config = kinectConfigs[i];
			KinectClient kinectClient = new KinectClient(config.ip, config.port);
			kinectClient.init(gl);
			kinectClient.start();
			kinectClients.add(kinectClient);
		}
        
        try
		{
        	Font font = Font.createFont(Font.TRUETYPE_FONT, new FileInputStream("kinectstreaming/data/Gotham-Black.otf")).deriveFont(Font.PLAIN, 16);
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
		
		for (int i = 0; i < kinectClients.size(); i++)
		{
			KinectClient kinectClient = kinectClients.get(i);
			kinectClient.update(gl);
			
			gl.glColor3f(1, 1, 1);
			gl.glEnable(GL2.GL_TEXTURE_2D);
			gl.glBindTexture(GL2.GL_TEXTURE_2D, kinectClient.textureID());
			gl.glBegin(GL2.GL_QUADS);
			gl.glTexCoord2f(0, 1);
			gl.glVertex2f(KinectClient.DEPTH_WIDTH * i, 0);
			gl.glTexCoord2f(1, 1);
			gl.glVertex2f(KinectClient.DEPTH_WIDTH * (i + 1), 0);
			gl.glTexCoord2f(1, 0);
			gl.glVertex2f(KinectClient.DEPTH_WIDTH * (i + 1), KinectClient.DEPTH_HEIGHT);
			gl.glTexCoord2f(0, 0);
			gl.glVertex2f(KinectClient.DEPTH_WIDTH * i, KinectClient.DEPTH_HEIGHT);
			gl.glEnd();
			gl.glBindTexture(GL2.GL_TEXTURE_2D, 0);
			gl.glDisable(GL2.GL_TEXTURE_2D);
			
			if (!kinectClient.running())
			{
				gl.glColor4f(1, 0, 0, 0.5f);
				gl.glBegin(GL2.GL_QUADS);
				gl.glVertex2f(KinectClient.DEPTH_WIDTH * i, 0);
				gl.glVertex2f(KinectClient.DEPTH_WIDTH * (i + 1), 0);
				gl.glVertex2f(KinectClient.DEPTH_WIDTH * (i + 1), KinectClient.DEPTH_HEIGHT);
				gl.glVertex2f(KinectClient.DEPTH_WIDTH * i, KinectClient.DEPTH_HEIGHT);
				gl.glEnd();
			}
			
			textRenderer.begin3DRendering();
			textRenderer.setColor(0, 0, 1, 1);
			textRenderer.draw("REFRESH TIME: " + kinectClient.refreshTime() + " MILLIS", KinectClient.DEPTH_WIDTH * i + 10, 30);
			textRenderer.draw("MAX REFRESH TIME: " + kinectClient.maxFrameTime() + " MILLIS", KinectClient.DEPTH_WIDTH * i + 10, 10);
			textRenderer.end3DRendering();
			textRenderer.flush();
		}
	}

	@Override
	public void dispose(GLAutoDrawable drawable)
	{
		for (KinectClient kinectClient : kinectClients) kinectClient.stop();
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
		else if (key == KeyEvent.VK_SPACE)
		{
			for (KinectClient kinectClient : kinectClients) kinectClient.resetMaxFrameTime();
		}
		else if (key == KeyEvent.VK_1)
		{
			KinectClient kinectClient = kinectClients.get(0);
			if (kinectClient.running()) kinectClient.stop();
			else kinectClient.start();
		}
		else if (key == KeyEvent.VK_2)
		{
			KinectClient kinectClient = kinectClients.get(1);
			if (kinectClient.running()) kinectClient.stop();
			else kinectClient.start();
		}
		else if (key == KeyEvent.VK_3)
		{
			KinectClient kinectClient = kinectClients.get(2);
			if (kinectClient.running()) kinectClient.stop();
			else kinectClient.start();
		}
	}

	@Override
	public void keyReleased(KeyEvent keyEvent)
	{
	}
}
