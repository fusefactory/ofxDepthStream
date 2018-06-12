// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2017 Intel Corporation. All Rights Reserved.

#include <librealsense2/rs.hpp> // Include RealSense Cross Platform API
// #include "example.hpp"          // Include short list of convenience functions for rendering

#include "../../../libs/DepthStream/src/TransmitterAgent.h"
#include "key_handler.h"

// Capture Example demonstrates how to
// capture depth and color video streams and render them to the screen
int main(int argc, char * argv[]) try
{
  DepthStream::TransmitterAgent agent(argc, argv);
  bool verbose = agent.getVerbose();

  // Declare depth colorizer for pretty visualization of depth data
  // rs2::colorizer color_map;

  // Declare RealSense pipeline, encapsulating the actual device and sensors
  rs2::pipeline pipe;
  // Start streaming with default recommended configuration
  pipe.start();

  while(shouldContinue) { // Application still alive?
    rs2::frameset data = pipe.wait_for_frames(); // Wait for next set of frames from the camera
    auto frame = (rs2::video_frame)data.get_depth_frame(); // Find and colorize the depth data
    // rs2::frame colorizedDepth = color_map(depth);

    auto format = frame.get_profile().format();
    auto width = frame.get_width();
    auto height = frame.get_height();
    // auto stream = frame.get_profile().stream_type();


    if (format == RS2_FORMAT_Z16) {
      agent.submit(frame.get_data(), width*height*2);
      // if (verbose) std::cout << "Submitted " << width <<"x" << height << " RS2_FORMAT_Z16 frame to TransmitterAgent" << std::endl;
      continue;
    }

    std::cout << "Frame format not supported " << rs2_format_to_string(format) << std::endl;
  }

  return EXIT_SUCCESS;
}
catch (const rs2::error & e)
{
    std::cerr << "RealSense error calling " << e.get_failed_function() << "(" << e.get_failed_args() << "):\n    " << e.what() << std::endl;
    return EXIT_FAILURE;
}
catch (const std::exception& e)
{
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
}
