// This file is part of the Orbbec Astra SDK [https://orbbec3d.com]
// Copyright (c) 2015 Orbbec 3D
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Be excellent to each other.
#include <astra/astra.hpp>
#include <cstdio>
#include <chrono>
#include <iostream>
#include <iomanip>
#include "key_handler.h"

class SampleFrameListener : public astra::FrameListener
{
private:
    using buffer_ptr = std::unique_ptr<int16_t []>;
    buffer_ptr buffer_;
    unsigned int lastWidth_;
    unsigned int lastHeight_;

public:
    virtual void on_frame_ready(astra::StreamReader& reader,
                                astra::Frame& frame) override
    {
        const astra::DepthFrame depthFrame = frame.get<astra::DepthFrame>();

        if (depthFrame.is_valid())
        {
            print_depth(depthFrame,
            reader.stream<astra::DepthStream>().coordinateMapper());
            check_fps();
        }
    }

    void print_depth(const astra::DepthFrame& depthFrame,
                     const astra::CoordinateMapper& mapper)
    {
        if (depthFrame.is_valid())
        {
            int width = depthFrame.width();
            int height = depthFrame.height();
            int frameIndex = depthFrame.frame_index();

            //determine if buffer needs to be reallocated
            if (width != lastWidth_ || height != lastHeight_)
            {
                buffer_ = buffer_ptr(new int16_t[depthFrame.length()]);
                lastWidth_ = width;
                lastHeight_ = height;
            }
            depthFrame.copy_to(buffer_.get());

            size_t index = ((width * (height / 2.0f)) + (width / 2.0f));
            short middle = buffer_[index];

            float worldX, worldY, worldZ;
            float depthX, depthY, depthZ;
            mapper.convert_depth_to_world(width / 2.0f, height / 2.0f, middle, &worldX, &worldY, &worldZ);
            mapper.convert_world_to_depth(worldX, worldY, worldZ, &depthX, &depthY, &depthZ);

            std::cout << "depth frameIndex: " << frameIndex
                      << " value: " << middle
                      << " wX: " << worldX
                      << " wY: " << worldY
                      << " wZ: " << worldZ
                      << " dX: " << depthX
                      << " dY: " << depthY
                      << " dZ: " << depthZ
                      << std::endl;
        }
    }

    void check_fps()
    {
        const double frameWeight = 0.2;

        auto newTimepoint = clock_type::now();
        auto frameDuration = std::chrono::duration_cast<duration_type>(newTimepoint - lastTimepoint_);

        frameDuration_ = frameDuration * frameWeight + frameDuration_ * (1 - frameWeight);
        lastTimepoint_ = newTimepoint;

        double fps = 1.0 / frameDuration_.count();

        auto precision = std::cout.precision();
        std::cout << std::fixed
                  << std::setprecision(1)
                  << fps << " fps ("
                  << std::setprecision(2)
                  << frameDuration.count() * 1000 << " ms)"
                  << std::setprecision(precision)
                  << std::endl;
    }

private:
    using duration_type = std::chrono::duration < double > ;
    duration_type frameDuration_{ 0.0 };

    using clock_type = std::chrono::system_clock;
    std::chrono::time_point<clock_type> lastTimepoint_;
};

int main(int argc, char** argv)
{
    astra::initialize();

    set_key_handler();

    astra::StreamSet streamSet;
    astra::StreamReader reader = streamSet.create_reader();

    SampleFrameListener listener;

    reader.stream<astra::DepthStream>().start();

    std::cout << "depthStream -- hFov: "
              << reader.stream<astra::DepthStream>().hFov()
              << " vFov: "
              << reader.stream<astra::DepthStream>().vFov()
              << std::endl;

    reader.add_listener(listener);

    do
    {
        astra_temp_update();
    } while (shouldContinue);

    reader.remove_listener(listener);

    astra::terminate();
}
