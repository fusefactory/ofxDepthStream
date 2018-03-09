package fuse.kinectforwindows2.streaming;

import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.net.InetAddress;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.Calendar;
import java.util.List;

import fuse.kinectforwindows2.HandJoint;
import fuse.kinectforwindows2.JointType;
import fuse.kinectforwindows2.KinectV2;
import fuse.kinectforwindows2.Skeleton;
import fuse.kinectforwindows2.SkeletonJoint;

public class KinectSkeletonTransmitter
{
	private int port;
	private boolean running;
	private KinectV2 kinect;
	private ServerSocket serverSocket;
	private Socket socket;
	
	public KinectSkeletonTransmitter(KinectV2 kinect, int port)
	{
		this.kinect = kinect;
		this.port = port;
	}
	
	public boolean running()
	{
		return running;
	}
	
	public void start()
	{
		running = true;
		
		new Thread(new Runnable()
		{
			
			@Override
			public void run()
			{
				try
				{
					serverSocket = new ServerSocket(port);
					System.out.println(Calendar.getInstance().getTime().toString() + " - KinectSkeletonTransmitter: starting " + InetAddress.getLocalHost() + ":" + serverSocket.getLocalPort());
					
					while (running)
					{
						socket = serverSocket.accept();
						System.out.println(Calendar.getInstance().getTime().toString() + " - KinectSkeletonTransmitter: transmitting to " + socket.getInetAddress().getHostAddress());
						
						boolean sent = true;
						while (sent)
						{
							long time = System.currentTimeMillis();
							while (System.currentTimeMillis() - time < 20) { };
							sent = send(socket.getOutputStream());
						}
						
						System.out.println(Calendar.getInstance().getTime().toString() + " - KinectSkeletonTransmitter: stop transmitting");
					}
				}
				catch (Exception e)
				{
					e.printStackTrace();
				}
				
			}
		}).start();
	}
	
	private boolean send(OutputStream out)
	{
		try
		{
			List<Skeleton> skeletons = kinect.getSkeleton3D();
			for (Skeleton skeleton : skeletons)
			{
				OutputStreamWriter outputStream = new OutputStreamWriter(out);
				
				for (JointType type : JointType.values())
				{
					// joint data
					SkeletonJoint joint = skeleton.getJoint(type);
					int state = joint.trackingState().ordinal();
					if (joint instanceof HandJoint)
					{
						HandJoint handJoint = (HandJoint)joint;
						state = handJoint.handState().ordinal();
					}
					
					StringBuilder builder = new StringBuilder();
					builder.append(skeleton.id() + ",");
					builder.append(type.ordinal() + ",");
					builder.append(state + ",");
					builder.append(joint.position().x + ",");
					builder.append(joint.position().y + ",");
					builder.append(joint.position().z + ",");
					builder.append(joint.orientation().x + ",");
					builder.append(joint.orientation().y + ",");
					builder.append(joint.orientation().z + ",");
					builder.append(joint.orientation().w);
					
					outputStream.write(builder.toString() + "\n");
					outputStream.flush();
				}
			}
			return true;
		}
		catch (Exception e)
		{
			System.out.println(Calendar.getInstance().getTime().toString() + " - KinectSkeletonTransmitter: error sending data: " + e.getMessage());
			return false;
		}
	}
	
	public void stop()
	{
		running = false;
	}
}
