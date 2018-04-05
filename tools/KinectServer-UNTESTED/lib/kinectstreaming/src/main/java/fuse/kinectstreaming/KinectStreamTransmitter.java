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


import java.io.BufferedReader;
import java.io.ByteArrayOutputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.net.InetAddress;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.Calendar;
import java.util.zip.Deflater;

public abstract class KinectStreamTransmitter
{
	private int port;
	private boolean running;
	protected KinectDevice kinect;
	private ServerSocket serverSocket;
	private Socket socket;

	public KinectStreamTransmitter(KinectDevice kinect, int port)
	{
		this.kinect = kinect;
		this.port = port;
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
					System.out.println(Calendar.getInstance().getTime().toString() + " - KinectStreamTransmitter: starting " + InetAddress.getLocalHost() + ":" + serverSocket.getLocalPort());

					while (running)
					{
						socket = serverSocket.accept();
						System.out.println(Calendar.getInstance().getTime().toString() + " - KinectStreamTransmitter: transmitting to " + socket.getInetAddress().getHostAddress());

						InputStream input = socket.getInputStream();


						boolean sent = true;
						while (sent)
						{
							sent = send(socket.getOutputStream());
							if (input.available() > 0)
							{
								BufferedReader br = new BufferedReader(new InputStreamReader(input));
								String command = br.readLine();
								if (command.equals(KinectStreamMessages.Restart))
								{
									System.out.println("Restart command received!");
									kinect.restart();
								}
							}
						}

						System.out.println(Calendar.getInstance().getTime().toString() + " - KinectStreamTransmitter: stop transmitting");
					}
				}
				catch (Exception e)
				{
					e.printStackTrace();
				}
			}
		}).start();
	}

	protected abstract byte[] prepareData();

	private boolean send(OutputStream out)
	{
		try
		{
			byte[] original = prepareData();
			byte[] compressed = compress(original);

			DataOutputStream outputStream = new DataOutputStream(out);
			outputStream.writeInt(compressed.length);
			outputStream.write(compressed);
			outputStream.flush();
			return true;
		}
		catch (Exception e)
		{
			System.out.println(Calendar.getInstance().getTime().toString() + " - KinectStreamTransmitter: error sending data: " + e.getMessage());
			return false;
		}
	}

	private byte[] compress(byte[] data)
	{
		if (data == null || data.length == 0) return new byte[0];

		try (final ByteArrayOutputStream out = new ByteArrayOutputStream(data.length))
		{
			Deflater deflater = new Deflater();
			deflater.setLevel(Deflater.BEST_SPEED);
			deflater.setInput(data);
			deflater.finish();

			final byte[] buffer = new byte[1024];
			while (!deflater.finished())
			{
				out.write(buffer, 0, deflater.deflate(buffer));
			}
			deflater.end();
			return out.toByteArray();
		}
		catch (final IOException e)
		{
			System.err.println(Calendar.getInstance().getTime().toString() + " - Compression failed! Returning the original data...");
			e.printStackTrace();
			return data;
		}
	}
}
