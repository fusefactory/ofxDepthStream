/******************************************************************************\
* Copyright (C) 2012-2014 Leap Motion, Inc. All rights reserved.               *
* Leap Motion proprietary and confidential. Not for distribution.              *
* Use subject to the terms of the Leap Motion SDK Agreement available at       *
* https://developer.leapmotion.com/sdk_agreement, or another agreement         *
* between Leap Motion and you, your company or other organization.             *
\******************************************************************************/

#include <iostream>
#include <cstring>
#include "Leap.h"

#include "../../../libs/DepthStream/src/TransmitterAgent.h"

using namespace Leap;
using namespace DepthStream;

class DepthStreamListener : public Listener {
  public:
    DepthStreamListener(TransmitterAgent& transmitterAgent) : transmitterAgent(&transmitterAgent) {
    }

    virtual void onInit(const Controller&);
    virtual void onConnect(const Controller&);
    virtual void onDisconnect(const Controller&);
    virtual void onExit(const Controller&);
    virtual void onFrame(const Controller&);
    virtual void onFocusGained(const Controller&);
    virtual void onFocusLost(const Controller&);
    virtual void onDeviceChange(const Controller&);
    virtual void onServiceConnect(const Controller&);
    virtual void onServiceDisconnect(const Controller&);

    void setVerbose(bool v=true) { bVerbose = v; }

  private:
    bool bVerbose=false;
    TransmitterAgent* transmitterAgent;
};

// const std::string fingerNames[] = {"Thumb", "Index", "Middle", "Ring", "Pinky"};
// const std::string boneNames[] = {"Metacarpal", "Proximal", "Middle", "Distal"};
// const std::string stateNames[] = {"STATE_INVALID", "STATE_START", "STATE_UPDATE", "STATE_END"};

void DepthStreamListener::onInit(const Controller& controller) {
  if (bVerbose) std::cout << "Initialized" << std::endl;
}

void DepthStreamListener::onConnect(const Controller& controller) {
  if (bVerbose) std::cout << "Connected" << std::endl;
  // controller.enableGesture(Gesture::TYPE_CIRCLE);
  // controller.enableGesture(Gesture::TYPE_KEY_TAP);
  // controller.enableGesture(Gesture::TYPE_SCREEN_TAP);
  // controller.enableGesture(Gesture::TYPE_SWIPE);
}

void DepthStreamListener::onDisconnect(const Controller& controller) {
  // Note: not dispatched when running in a debugger.
  if (bVerbose) std::cout << "Disconnected" << std::endl;
}

void DepthStreamListener::onExit(const Controller& controller) {
  if (bVerbose) std::cout << "Exited" << std::endl;
}

void DepthStreamListener::onFrame(const Controller& controller) {
  // Get the most recent frame and report some basic information
  const Frame frame = controller.frame();
  if (bVerbose) std::cout << "Frame id: " << frame.id()
            << ", timestamp: " << frame.timestamp()
            // << ", hands: " << frame.hands().count()
            // << ", extended fingers: " << frame.fingers().extended().count()
            // << ", tools: " << frame.tools().count()
            // << ", gestures: " << frame.gestures().count()
            << ", images: " << frame.images().count()
            << std::endl;

  if (this->transmitterAgent) {
    if (frame.images().count() > 0) {
      auto image = frame.images()[0];
      this->transmitterAgent->submit(image.data(), image.width() * image.height() * image.bytesPerPixel());
    }
  }

  // HandList hands = frame.hands();
  // for (HandList::const_iterator hl = hands.begin(); hl != hands.end(); ++hl) {
  //   // Get the first hand
  //   const Hand hand = *hl;
  //   std::string handType = hand.isLeft() ? "Left hand" : "Right hand";
  //   if (bVerbose) std::cout << std::string(2, ' ') << handType << ", id: " << hand.id()
  //             << ", palm position: " << hand.palmPosition() << std::endl;
  //   // Get the hand's normal vector and direction
  //   const Vector normal = hand.palmNormal();
  //   const Vector direction = hand.direction();

  //   // Calculate the hand's pitch, roll, and yaw angles
  //   if (bVerbose) std::cout << std::string(2, ' ') <<  "pitch: " << direction.pitch() * RAD_TO_DEG << " degrees, "
  //             << "roll: " << normal.roll() * RAD_TO_DEG << " degrees, "
  //             << "yaw: " << direction.yaw() * RAD_TO_DEG << " degrees" << std::endl;

  //   // Get the Arm bone
  //   Arm arm = hand.arm();
  //   if (bVerbose) std::cout << std::string(2, ' ') <<  "Arm direction: " << arm.direction()
  //             << " wrist position: " << arm.wristPosition()
  //             << " elbow position: " << arm.elbowPosition() << std::endl;

  //   // Get fingers
  //   const FingerList fingers = hand.fingers();
  //   for (FingerList::const_iterator fl = fingers.begin(); fl != fingers.end(); ++fl) {
  //     const Finger finger = *fl;
  //     if (bVerbose) std::cout << std::string(4, ' ') <<  fingerNames[finger.type()]
  //               << " finger, id: " << finger.id()
  //               << ", length: " << finger.length()
  //               << "mm, width: " << finger.width() << std::endl;

  //     // Get finger bones
  //     for (int b = 0; b < 4; ++b) {
  //       Bone::Type boneType = static_cast<Bone::Type>(b);
  //       Bone bone = finger.bone(boneType);
  //       if (bVerbose) std::cout << std::string(6, ' ') <<  boneNames[boneType]
  //                 << " bone, start: " << bone.prevJoint()
  //                 << ", end: " << bone.nextJoint()
  //                 << ", direction: " << bone.direction() << std::endl;
  //     }
  //   }
  // }

  // // Get tools
  // const ToolList tools = frame.tools();
  // for (ToolList::const_iterator tl = tools.begin(); tl != tools.end(); ++tl) {
  //   const Tool tool = *tl;
  //   if (bVerbose) std::cout << std::string(2, ' ') <<  "Tool, id: " << tool.id()
  //             << ", position: " << tool.tipPosition()
  //             << ", direction: " << tool.direction() << std::endl;
  // }

  // // Get gestures
  // const GestureList gestures = frame.gestures();
  // for (int g = 0; g < gestures.count(); ++g) {
  //   Gesture gesture = gestures[g];

  //   switch (gesture.type()) {
  //     case Gesture::TYPE_CIRCLE:
  //     {
  //       CircleGesture circle = gesture;
  //       std::string clockwiseness;

  //       if (circle.pointable().direction().angleTo(circle.normal()) <= PI/2) {
  //         clockwiseness = "clockwise";
  //       } else {
  //         clockwiseness = "counterclockwise";
  //       }

  //       // Calculate angle swept since last frame
  //       float sweptAngle = 0;
  //       if (circle.state() != Gesture::STATE_START) {
  //         CircleGesture previousUpdate = CircleGesture(controller.frame(1).gesture(circle.id()));
  //         sweptAngle = (circle.progress() - previousUpdate.progress()) * 2 * PI;
  //       }
  //       if (bVerbose) std::cout << std::string(2, ' ')
  //                 << "Circle id: " << gesture.id()
  //                 << ", state: " << stateNames[gesture.state()]
  //                 << ", progress: " << circle.progress()
  //                 << ", radius: " << circle.radius()
  //                 << ", angle " << sweptAngle * RAD_TO_DEG
  //                 <<  ", " << clockwiseness << std::endl;
  //       break;
  //     }
  //     case Gesture::TYPE_SWIPE:
  //     {
  //       SwipeGesture swipe = gesture;
  //       if (bVerbose) std::cout << std::string(2, ' ')
  //         << "Swipe id: " << gesture.id()
  //         << ", state: " << stateNames[gesture.state()]
  //         << ", direction: " << swipe.direction()
  //         << ", speed: " << swipe.speed() << std::endl;
  //       break;
  //     }
  //     case Gesture::TYPE_KEY_TAP:
  //     {
  //       KeyTapGesture tap = gesture;
  //       if (bVerbose) std::cout << std::string(2, ' ')
  //         << "Key Tap id: " << gesture.id()
  //         << ", state: " << stateNames[gesture.state()]
  //         << ", position: " << tap.position()
  //         << ", direction: " << tap.direction()<< std::endl;
  //       break;
  //     }
  //     case Gesture::TYPE_SCREEN_TAP:
  //     {
  //       ScreenTapGesture screentap = gesture;
  //       if (bVerbose) std::cout << std::string(2, ' ')
  //         << "Screen Tap id: " << gesture.id()
  //         << ", state: " << stateNames[gesture.state()]
  //         << ", position: " << screentap.position()
  //         << ", direction: " << screentap.direction()<< std::endl;
  //       break;
  //     }
  //     default:
  //       if (bVerbose) std::cout << std::string(2, ' ')  << "Unknown gesture type." << std::endl;
  //       break;
  //   }
  // }

  // if (!frame.hands().isEmpty() || !gestures.isEmpty()) {
  //   if (bVerbose) std::cout << std::endl;
  // }
}

void DepthStreamListener::onFocusGained(const Controller& controller) {
  if (bVerbose) std::cout << "Focus Gained" << std::endl;
}

void DepthStreamListener::onFocusLost(const Controller& controller) {
  if (bVerbose) std::cout << "Focus Lost" << std::endl;
}

void DepthStreamListener::onDeviceChange(const Controller& controller) {
  if (bVerbose) std::cout << "Device Changed" << std::endl;
  const DeviceList devices = controller.devices();

  for (int i = 0; i < devices.count(); ++i) {
    if (bVerbose) std::cout << "id: " << devices[i].toString() << std::endl;
    if (bVerbose) std::cout << "  isStreaming: " << (devices[i].isStreaming() ? "true" : "false") << std::endl;
  }
}

void DepthStreamListener::onServiceConnect(const Controller& controller) {
  if (bVerbose) std::cout << "Service Connected" << std::endl;
}

void DepthStreamListener::onServiceDisconnect(const Controller& controller) {
  if (bVerbose) std::cout << "Service Disconnected" << std::endl;
}

int main(int argc, char** argv) {
  Controller controller;
  controller.setPolicy(Leap::Controller::POLICY_IMAGES);

  TransmitterAgent agent(argc, argv);
  DepthStreamListener listener(agent);
  listener.setVerbose(agent.getVerbose());
  controller.addListener(listener);
 
  // Keep this process running until Enter is pressed
  std::cout << "Press Enter to quit..." << std::endl;
  std::cin.get();

  // Remove the sample listener when done
  controller.removeListener(listener);

  return 0;
}
