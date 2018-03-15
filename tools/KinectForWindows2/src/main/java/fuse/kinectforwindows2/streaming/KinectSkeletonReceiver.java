package fuse.kinectforwindows2.streaming;

import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.net.Socket;
import java.util.Calendar;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

import fuse.kinectforwindows2.HandJoint;
import fuse.kinectforwindows2.HandState;
import fuse.kinectforwindows2.JointType;
import fuse.kinectforwindows2.Skeleton;
import fuse.kinectforwindows2.SkeletonJoint;
import fuse.kinectforwindows2.TrackingState;

public class KinectSkeletonReceiver
{
	private final int attemptTime = 5; // in seconds
	
	private boolean running;
	private String address;
	private int port;
	private Socket socket;
	private boolean connected;
	private Map<Skeleton, Long> skeletons;
	
	public KinectSkeletonReceiver(String address, int port)
	{
		this.address = address;
		this.port = port;
		
		skeletons = new ConcurrentHashMap<>();
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
				while (running)
				{
					while (!connected)
					{
						try
						{
							socket = new Socket(address, port);
							connected = true;
							System.out.println(Calendar.getInstance().getTime().toString() + " - KinectSkeletonReceiver: connected");
						}
						catch (Exception e)
						{
							connected = false;
							System.out.println(Calendar.getInstance().getTime().toString() + " - KinectSkeletonReceiver: cannot connect to " + address + ":" + port);
							
							long wait = System.currentTimeMillis();
							while (System.currentTimeMillis() - wait < attemptTime * 1000) { };
						}
					}
					
					try
					{
						while (true)
						{
							BufferedReader reader = new BufferedReader(new InputStreamReader(socket.getInputStream()));
							String line = reader.readLine();
							if (line != null) receiveMessage(line);
						}
					}
					catch (Exception e)
					{
						System.out.println(Calendar.getInstance().getTime().toString() + " - KinectSkeletonReceiver: connection lost! - " + e.getMessage());
						close();
						connected = false;
					}
				}
			}
		}).start();
	}
	
	public Map<Skeleton, Long> skeletons()
	{
		return skeletons;
	}
	
	private Skeleton getSkeletonById(int id)
	{
		for (Skeleton skeleton : skeletons.keySet())
		{
			if (skeleton.id() == id) return skeleton;
		}
		return null;
	}
	
	private void receiveMessage(String message)
	{
		String[] args = message.split(",");
		if (args.length == 10)
		{
			try
			{
				// id
				int id = Integer.valueOf(args[0]);
				Skeleton skeleton = getSkeletonById(id);
				if (skeleton == null) skeleton = new Skeleton(id);
					
				// joint type
				JointType jointType = JointType.values()[Integer.valueOf(args[1])];
				SkeletonJoint joint = skeleton.getJoint(jointType);
				
				// state
				int state = Integer.valueOf(args[2]);
				TrackingState trackingState = TrackingState.values()[Math.min(state, TrackingState.Tracked.ordinal())];
				joint.setTrackingState(trackingState);
				if (joint instanceof HandJoint)
				{
					HandJoint handJoint = (HandJoint)joint;
					handJoint.setHandState(HandState.values()[state]);
				}
				
				// position
				joint.position().x = Float.valueOf(args[3]);
				joint.position().y = Float.valueOf(args[4]);
				joint.position().z = Float.valueOf(args[5]);
				
				// orientation
				joint.orientation().x = Float.valueOf(args[6]);
				joint.orientation().y = Float.valueOf(args[7]);
				joint.orientation().z = Float.valueOf(args[8]);
				joint.orientation().w = Float.valueOf(args[9]);
				
				// time
				skeletons.put(skeleton, System.currentTimeMillis());
			}
			catch (Exception e)
			{
				System.out.println(Calendar.getInstance().getTime().toString() + " - KinectSkeletonReceiver: bad skeleton data - " + e.getMessage());
			}
			
		}
	}
	
	public void stop()
	{
		running = false;
		close();
		connected = false;
	}
	
	private void close()
	{
		try
		{
			if (socket != null)
			{
				socket.close();
				socket = null;
			}
		}
		catch (Exception e)
		{
			e.printStackTrace();
		}
	}
}
