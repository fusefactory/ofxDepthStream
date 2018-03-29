// This file is part of the Orbbec Astra SDK [https://orbbec3d.com]
// Copyright (c) 2015-2017 Orbbec 3D
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
#include <key_handler.h>

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
        const float frameWeight = 0.5;

        auto newTimepoint = clock_type::now();
        float frameDuration = std::chrono::duration_cast<duration_type>(newTimepoint - lastTimepoint_).count();

        frameDuration_ = frameDuration * frameWeight + frameDuration_ * (1 - frameWeight);
        lastTimepoint_ = newTimepoint;

        double fps = 1.0 / frameDuration_;

        auto precision = std::cout.precision();
        std::cout << std::fixed
                  << std::setprecision(1)
                  << fps << " fps ("
                  << std::setprecision(2)
                  << frameDuration_ * 1000.0 << " ms)"
                  << std::setprecision(precision)
                  << std::endl;
    }

private:
    using duration_type = std::chrono::duration<float>;
    float frameDuration_{ 0.0 };

    using clock_type = std::chrono::system_clock;
    std::chrono::time_point<clock_type> lastTimepoint_{clock_type::now()};
};

int main(int argc, char** argv)
{
    astra::initialize();

    set_key_handler();

    astra::StreamSet streamSet;
    astra::StreamReader reader = streamSet.create_reader();

    SampleFrameListener listener;

    auto depthStream = reader.stream<astra::DepthStream>();
    depthStream.start();

    char serialnumber[128];
    depthStream.serial_number(serialnumber, 128);

    std::cout << "depthStream -- hFov: "
              << reader.stream<astra::DepthStream>().hFov()
              << " vFov: "
              << reader.stream<astra::DepthStream>().vFov()
              << " serial number: "
              << serialnumber
              << std::endl;

    const uint32_t chipId = depthStream.chip_id();

    switch (chipId)
    {
        case ASTRA_CHIP_ID_MX400:
            std::cout << "Chip ID: MX400" << std::endl;
            break;
        case ASTRA_CHIP_ID_MX6000:
            std::cout << "Chip ID: MX6000" << std::endl;
            break;
        case ASTRA_CHIP_ID_UNKNOWN:
        default:
            std::cout << "Chip ID: Unknown" << std::endl;
            break;
    }

	const astra_usb_info_t usbinfo = depthStream.usb_info();

	std::cout << "usbInfo ---pid:" << usbinfo.pid<<" vid: "<< usbinfo.vid<< std::endl;


    reader.add_listener(listener);

    do
    {
        astra_update();
    } while (shouldContinue);

    reader.remove_listener(listener);

    astra::terminate();
}
