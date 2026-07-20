#ifndef HARDWARE_OUTPUT_DEVICE_H
#define HARDWARE_OUTPUT_DEVICE_H

#include <unordered_map>
#include "stringImproved.h"

class HardwareOutputDevice
{
public:
    HardwareOutputDevice() = default;
    virtual ~HardwareOutputDevice() = default;

    virtual bool configure(std::unordered_map<string, string> settings) = 0;

    //Set a hardware channel output. Value is 0.0 to 1.0 for no to max output.
    virtual void setChannelData(int channel, float value) = 0;

    //Flush the current channel data to the output device immediately, when supported.
    virtual void flush() {}

    //Return the number of output channels supported by this device.
    virtual int getChannelCount() = 0;
};

#endif//HARDWARE_OUTPUT_DEVICE_H
