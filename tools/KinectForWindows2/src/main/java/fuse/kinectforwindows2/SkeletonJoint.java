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

package fuse.kinectforwindows2;

import javax.vecmath.Vector3f;
import javax.vecmath.Vector4f;

public class SkeletonJoint
{
	private Vector3f position;
	private Vector4f orientation;
	private TrackingState trackingState;

	public SkeletonJoint()
	{
		position = new Vector3f();
		orientation = new Vector4f();
		trackingState = TrackingState.NotTracked;
	}

	public Vector3f position()
	{
		return position;
	}

	public Vector4f orientation()
	{
		return orientation;
	}

	public TrackingState trackingState()
	{
		return trackingState;
	}

	public void setTrackingState(TrackingState state)
	{
		trackingState = state;
	}
}
