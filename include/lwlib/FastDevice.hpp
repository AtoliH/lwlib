#ifndef FastDevice_hpp
#define FastDevice_hpp

#include <string>
#include <iostream>

#include "lwlib/internal/types.h"

class FastDevice
{
public:
    FastDevice()
    {
        std::vector<cl::Platform> allPlatforms;
        cl::Platform::get(&allPlatforms);

        if (allPlatforms.size()==0) {
            std::cout<<" No platforms found. Check OpenCL installation!\n";
            return;
        }
        cl::Platform defaultPlatform=allPlatforms[0];
        std::cout << "Using platform: "<<defaultPlatform.getInfo<CL_PLATFORM_NAME>()<<"\n";

        // get default device (CPUs, GPUs) of the default platform
        std::vector<cl::Device> allDevices;
        defaultPlatform.getDevices(CL_DEVICE_TYPE_ALL, &allDevices);
        if(allDevices.size()==0){
            std::cout<<" No devices found. Check OpenCL installation!\n";
            return;
        }

        // use device[1] because that's a GPU; device[0] is the CPU
        cl::Device defaultDevice=allDevices[1];
        std::cout<< "Using device: "<<defaultDevice.getInfo<CL_DEVICE_NAME>()<<"\n";

        cl::Context context({defaultDevice});
    }

    static std::string getTypes();

    static std::string getDmgcalc();

    static std::string getMacro();
};

#endif
