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
