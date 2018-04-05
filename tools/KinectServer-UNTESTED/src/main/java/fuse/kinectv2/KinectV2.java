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

package fuse.kinectv2;

import java.nio.FloatBuffer;
import java.nio.IntBuffer;

import org.openkinect.freenect2.Device;

import com.jogamp.common.nio.Buffers;

public class KinectV2
{
	public final int depthWidth = 512;
    public final int depthHeight = 424;
    public final int colorWidth = 1920;
    public final int colorHeight = 1080;

    // device
    private Device device;
    private int deviceNumber;
    private boolean mirror;
    private boolean running;

    // depth
    private boolean depthEnabled;
    private FloatBuffer depthPositionsBuffer;
    private IntBuffer depthBuffer;
    private int[] depthMap;
    private FloatBuffer edgeBuffer;
    private boolean[] edgeMap;

    // rgb
    private boolean rgbEnabled;
    private IntBuffer rgbBuffer;

    // registered
    private boolean registeredEnabled;
    private IntBuffer registeredImage;
    private Platform platform;

    private enum Platform
    {
        OTHER, WINDOWS, MACOSX, LINUX
    }

    /**
     * Ritorna il sistema operativo corrente.
     * @return
     */

    private Platform getCurrentPlatform()
    {
    	String osname = System.getProperty("os.name");

    	if (osname.indexOf("Mac") != -1)
    	{
    		return Platform.MACOSX;
    	}

    	else if (osname.indexOf("Windows") != -1)
    	{
    		return Platform.WINDOWS;
    	}
    	else if (osname.equals("Linux"))
    	{
    		return Platform.LINUX;
    	}
    	else
    	{
    		return Platform.OTHER;
		}
    }

	public KinectV2()
	{
		this(0);
	}

	public KinectV2(int deviceNumber)
	{
		platform = getCurrentPlatform();
		System.out.println("Current platform: " + platform);

		this.deviceNumber = deviceNumber;
		device = new Device();
		mirror = true; // default
	}

	public boolean isReady()
	{
		return device.jniDeviceReady();
	}

	/**
	 * Avvia la periferica. Deve essere chiamato come ultimo metodo durante l'inizializzazione.
	 */
	public void start()
	{
		device.jniOpenM(deviceNumber);
		if (platform == Platform.MACOSX || platform == Platform.LINUX) startKinectThread();
	}

	/**
	 * Disponibile solo per MAC e LINUX.
	 *
	 * Avvia la periferica. Deve essere chiamato come ultimo metodo durante l'inizializzazione.
	 */
	public void start(int indexVideo)
	{
		if (platform == Platform.MACOSX || platform == Platform.LINUX)
		{
			device.jniOpenMV(deviceNumber, indexVideo);
			startKinectThread();
		}
		else
		{
			System.err.println("start(int indexVideo) not avaiable in WIN");
		}
	}

	/**
	 * Disponibile solo per MAC e LINUX.
	 *
	 * Avvia un thread per l'update di kinect.
	 */
	private void startKinectThread()
	{
		running = true;
		new Thread(new Runnable()
		{
			@Override
			public void run()
			{
				while (running) device.jniUpdateDevice();
			}
		}).start();
	}

	public void stop()
	{
		running = false;
		device.jniStop();
	}

	public void setMirror(boolean mirror)
	{
		this.mirror = mirror;
	}

	public boolean getMirror()
	{
		return mirror;
	}

	public void enableDepth()
	{
		if (!depthEnabled)
		{
			depthPositionsBuffer = Buffers.newDirectFloatBuffer(depthWidth * depthHeight * 3);
			depthBuffer = Buffers.newDirectIntBuffer(depthWidth * depthHeight);
			depthMap = new int[depthWidth * depthHeight];
			edgeBuffer = Buffers.newDirectFloatBuffer(depthWidth * depthHeight * 4);
			edgeMap = new boolean[depthWidth * depthHeight];

			device.jniEnableDepth(true);
			depthEnabled = true;
		}
	}

	public IntBuffer depthImage()
	{
		int[] depthRawData = device.jniGetDepthData();
		for (int y = 0; y < depthHeight; y++)
		{
			for (int x = 0; x < depthWidth; x++)
			{
				int index = x + y * depthWidth;
				if (mirror) index = (depthWidth - 1 - x) + y * depthWidth;
				depthBuffer.put(depthRawData[index]);
			}
		}
        depthBuffer.rewind();
        return depthBuffer;
	}

	/**
	 * Restituisce l'array di interi che costituiscono la depth image.
	 * @return
	 */
	public int[] depthData()
	{
		return device.jniGetDepthData();
	}

	/**
	 * Posizione nello spazio (x, y, z) di ogni singolo pixel.
	 * @return
	 */
	public FloatBuffer depthPositions()
	{
		float[] pcRawData = device.jniGetDepthCameraPositions();
		for (int y = 0; y < depthHeight; y++)
		{
			for (int x = 0; x < depthWidth; x++)
			{
				int index = x + y * depthWidth;
				if (mirror) index = (depthWidth - 1 - x) + y * depthWidth;
				depthPositionsBuffer.put(pcRawData[index * 3]);
				depthPositionsBuffer.put(pcRawData[index * 3 + 1]);
				depthPositionsBuffer.put(pcRawData[index * 3 + 2]);
			}
		}
        depthPositionsBuffer.rewind();
        return depthPositionsBuffer;
	}

	/**
	 * I valori, espressi in millimetri, vanno da 0 a 4500.
	 * @return
	 */
	public int[] depthMap()
	{
		int[] tmpMap = device.jniGetRawDepthData();
		for (int y = 0; y < depthHeight; y++)
		{
			for (int x = 0; x < depthWidth; x++)
			{
				int index = x + y * depthWidth;
				int depthIndex = index;
				if (mirror) depthIndex = (depthWidth - 1 - x) + y * depthWidth;
				depthMap[depthIndex] = tmpMap[index];
			}
		}
		return depthMap;
	}

	/**
	 * Restituisce l'immagine soglia in base alle distance specificate. Se il pixel è all'interno dell'intervallo risulta bianco, se è al di fuori risulta nero.
	 * @param minDistance La distanza minima dell'intervallo desiderato.
	 * @param maxDistance La distanza massima dell'intervallo.
	 */
	public FloatBuffer edgeImage(int minDistance, int maxDistance)
	{
		int[] depthMap = depthMap();
		for (int i = 0; i < depthMap.length; i++)
        {
			int z = depthMap[i];
			if (z > minDistance && z <= maxDistance)
			{
				edgeBuffer.put(1);
				edgeBuffer.put(1);
				edgeBuffer.put(1);
				edgeBuffer.put(1);
				edgeMap[i] = true;
			}
			else
			{
				edgeBuffer.put(0);
				edgeBuffer.put(0);
				edgeBuffer.put(0);
				edgeBuffer.put(0);
				edgeMap[i] = false;
			}
        }
        edgeBuffer.rewind();
        return edgeBuffer;
	}

	/**
	 * Restituisce l'immagine soglia in base alle distance specificate escludendo i bordi in base al margine. Se il pixel è all'interno dell'intervallo risulta bianco, se è al di fuori risulta nero.
	 * @param minDistance La distanza minima dell'intervallo desiderato.
	 * @param maxDistance La distanza massima dell'intervallo.
	 * @param margin Il margine dei bordi da escludere.
	 */
	public FloatBuffer edgeImage(int minDistance, int maxDistance, int margin)
	{
		return edgeImage(minDistance, maxDistance, margin, margin);
	}

	/**
	 * Restituisce l'immagine soglia in base alle distance specificate escludendo i bordi in base ai margini. Se il pixel è all'interno dell'intervallo risulta bianco, se è al di fuori risulta nero.
	 * @param minDistance La distanza minima dell'intervallo desiderato.
	 * @param maxDistance La distanza massima dell'intervallo.
	 * @param leftRightMargin Il margine dai bordi sinistro e destro.
	 * @param topBottomMargin Il margine dai bordi superiore e inferiore.
	 * @return
	 */
	public FloatBuffer edgeImage(int minDistance, int maxDistance, int leftRightMargin, int topBottomMargin)
	{
		int[] depthMap = depthMap();
        for (int y = 0; y < depthHeight; y++)
        {
        	for (int x = 0; x < depthWidth; x++)
        	{
        		int index = x + y * depthWidth;
        		int z = depthMap[index];
        		if (z > minDistance && z <= maxDistance && x >= leftRightMargin && x < depthWidth - leftRightMargin && y >= topBottomMargin && y < depthHeight - topBottomMargin)
        		{
        			edgeBuffer.put(1);
    				edgeBuffer.put(1);
    				edgeBuffer.put(1);
    				edgeBuffer.put(1);
    				edgeMap[index] = true;
        		}
    			else
    			{
    				edgeBuffer.put(0);
    				edgeBuffer.put(0);
    				edgeBuffer.put(0);
    				edgeBuffer.put(0);
    				edgeMap[index] = false;
    			}
        	}
        }
        edgeBuffer.rewind();
        return edgeBuffer;
	}

	public boolean[] edgeMap()
	{
		return edgeMap;
	}

	public void enableRgb()
	{
		if (!rgbEnabled)
		{
			rgbBuffer = Buffers.newDirectIntBuffer(colorWidth * colorHeight);
			device.jniEnableVideo(true);
			rgbEnabled = true;
		}
	}

	public IntBuffer rgbImage()
	{
		int[] colorRawData = device.jniGetColorData();
		for (int y = 0; y < colorHeight; y++)
		{
			for (int x = 0; x < colorWidth; x++)
			{
				int index = x + y * colorWidth;
				if (mirror) index = (colorWidth - 1 - x) + y * colorWidth;
				rgbBuffer.put(colorRawData[index]);
			}
		}
		rgbBuffer.rewind();
		return rgbBuffer;
	}

	public void enableRegistered()
	{
		if (!registeredEnabled)
		{
			registeredImage = Buffers.newDirectIntBuffer(depthWidth * depthHeight);
			device.jniEnableRegistered(true);
			registeredEnabled = true;
		}
	}

	public IntBuffer registeredImage()
	{
		int[] registeredData = device.jniGetRegistered();
		for (int y = 0; y < depthHeight; y++)
		{
			for (int x = 0; x < depthWidth; x++)
			{
				int index = x + y * depthWidth;
				if (mirror) index = (depthWidth - 1 - x) + y * depthWidth;
				registeredImage.put(registeredData[index]);
			}
		}
		registeredImage.rewind();
        return registeredImage;
	}

	public void close()
	{
		device.jniStop();
	}

	//TODO: da controllare se va su win
	public int numDevices()
	{
		if (platform == Platform.MACOSX || platform == Platform.LINUX) return device.jniGetNumDevices();
		else return -1;
	}
}
