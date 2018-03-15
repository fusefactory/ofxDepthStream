package fuse.kinectstreaming;

public interface KinectDevice
{
	int depthWidth();
	int depthHeight();
	int[] depthImage();
	int[] depthMap();
	void restart();
}
