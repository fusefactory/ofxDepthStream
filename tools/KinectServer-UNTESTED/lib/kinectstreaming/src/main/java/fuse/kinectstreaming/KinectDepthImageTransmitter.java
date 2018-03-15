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
