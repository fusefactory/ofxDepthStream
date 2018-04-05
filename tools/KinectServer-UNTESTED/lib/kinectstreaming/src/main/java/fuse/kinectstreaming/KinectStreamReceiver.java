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

package fuse.kinectstreaming;


import java.io.BufferedWriter;
import java.io.ByteArrayOutputStream;
import java.io.DataInputStream;
import java.io.OutputStreamWriter;
import java.net.Socket;
import java.util.Calendar;
import java.util.List;
import java.util.concurrent.CopyOnWriteArrayList;
import java.util.zip.Inflater;

public class KinectStreamReceiver
{
	private final int attemptTime = 5; // in seconds

	private boolean running;
	private String address;
	private int port;
	private Socket socket;
	private boolean connected;
	private List<KinectStreamListener> listeners;
	private boolean restartKinect;

	public KinectStreamReceiver(String address, int port)
	{
		this.address = address;
		this.port = port;

		listeners = new CopyOnWriteArrayList<>();
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
							System.out.println(Calendar.getInstance().getTime().toString() + " - KinectStreamReceiver: connected");
						}
						catch (Exception e)
						{
							connected = false;
							System.out.println(Calendar.getInstance().getTime().toString() + " - KinectStreamReceiver: cannot connect to " + address + ":" + port);

							long wait = System.currentTimeMillis();
							while (System.currentTimeMillis() - wait < attemptTime * 1000) { };
						}
					}

					try
					{
						while (true)
						{
							DataInputStream inputStream = new DataInputStream(socket.getInputStream());
							int length = inputStream.readInt();
							byte[] data = new byte[length];
							int read = 0;
							while (read < length)
							{
								read += inputStream.read(data, read, length - read);
							}

							byte[] decompressed = decompress(data);
							for (KinectStreamListener listener : listeners) listener.newData(decompressed);

							if (restartKinect)
							{
								BufferedWriter bw = new BufferedWriter(new OutputStreamWriter(socket.getOutputStream()));
								bw.write(KinectStreamMessages.Restart);
								bw.newLine();
								bw.flush();
								restartKinect = false;
							}
						}
					}
					catch (Exception e)
					{
						System.out.println(Calendar.getInstance().getTime().toString() + " - KinectStreamReceiver: connection lost! - " + e.getMessage());
						close();
						connected = false;
					}
				}
			}
		}).start();
	}

	private byte[] decompress(final byte[] data)
	{
		if (data == null || data.length == 0) return new byte[0];

		final Inflater inflater = new Inflater();
		inflater.setInput(data);

		try
		{
			ByteArrayOutputStream out = new ByteArrayOutputStream(data.length);
			final byte[] buffer = new byte[1024];
			while (!inflater.finished())
			{
				out.write(buffer, 0, inflater.inflate(buffer));
			}
			out.close();
			return out.toByteArray();
		}
		catch (Exception e)
		{
			System.err.println(Calendar.getInstance().getTime().toString() + " - Decompression failed! Returning the compressed data...");
			e.printStackTrace();
			return data;
		}
	}

	public void stop()
	{
		running = false;
		close();
		connected = false;
	}

	public void addListener(KinectStreamListener listener)
	{
		listeners.add(listener);
	}

	/**
	 * Send restart command to KinectStreamTransmitter.
	 */
	public void restartKinect()
	{
		restartKinect = true;
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
