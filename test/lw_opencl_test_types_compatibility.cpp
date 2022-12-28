#include <gtest/gtest.h>
#include <vector>
#include <string>
#include <cstring>
#include <cstdlib>
#include <iostream>

#include <lwlib/internal/types.h>
#include <lwlib/internal/macro.h>
#include <lwlib/TurnAction.hpp>

#include <lwlib/FastDevice.hpp>

class TypesCompatibilityTest : public ::testing::Test
{
protected:
    template <unsigned int D>
    class NDRange : public cl::NDRange
    {
    private:
        template<int index, typename T0, typename... T1s>
        void setSizes(T0&& t0, T1s&&... t1s)
        {
            sizes_[index] = t0;
            setSizes<index + 1, T1s...>(std::forward<T1s>(t1s)...);
        }

        template<int index, typename T0>
        void setSizes(T0&& t0)
        {
            sizes_[index] = t0;
        }

        template<int index>
        void setSizes()
        {
        }

    public:
        template<typename... Ts, std::enable_if_t<D == sizeof...(Ts) && D <= NDRange_MAX_DIMENSIONS>* = nullptr>
        NDRange(Ts... ts)
        {
            dimensions_ = D;
            setSizes<0>(std::forward<Ts>(ts)...);
        }
    };
    
    const cl::Context context;
    
    TypesCompatibilityTest()
    {
        // get all platforms (drivers), e.g. NVIDIA
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

        context.setDefault({defaultDevice});
    }
};

TEST_F(TypesCompatibilityTest, TurnAction)
{
    cl::Program::Sources sources;

    std::string testCode(R"(
void kernel test_turn_action(global turn_action *test_data, global unsigned long* out)
{
        out[0] = sizeof(((turn_action*)0)->spell);
        out[1] = sizeof(((turn_action*)0)->boost);
        out[2] = sizeof(((turn_action*)0)->graze);
        out[3] = sizeof(((turn_action*)0)->skill);
        out[4] = sizeof(((turn_action*)0)->target);
        out[5] = sizeof(struct turn_action);

        out[6] = test_data->spell > 0;
        out[7] = test_data->boost > 0;
        out[8] = test_data->graze > 0;
        out[9] = test_data->skill > 0;
        out[10] = test_data->target > 0;
}
    )");

    auto define = [](std::string a, auto b) {
        return "-D " + a + "=" + std::to_string(b) + " ";
    };

    std::string preprocessorOptions =
        define("SKILL_MAX_MODIFIERS", SKILL_MAX_MODIFIERS)
        + define("SC_MAX_MODIFIERS", SC_MAX_MODIFIERS)
        + define("SPELL_MAX_MODIFIERS", SPELL_MAX_MODIFIERS)
        + define("BARRIER_MAX_AMOUNT", BARRIER_MAX_AMOUNT)
        + define("NUMBER_OF_UNIQUE_MODIFIERS", NUMBER_OF_UNIQUE_MODIFIERS);

    std::string buildOptions(preprocessorOptions);
    
    std::string totalCode(FastDevice::getTypes() + testCode);
    sources.push_back({totalCode.c_str(), totalCode.length()});

    cl::Program program(context, sources);
    if (program.build({cl::Device::getDefault()}, buildOptions.c_str()) != CL_SUCCESS) {
        std::cout << "Error building: " << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(cl::Device::getDefault()) << std::endl;
        return;
    }

    cl::Buffer bufferTestData(context, CL_MEM_READ_ONLY, sizeof(turn_action));
    cl::Buffer bufferOUT(context, CL_MEM_WRITE_ONLY, sizeof(cl_ulong) * 11);

    cl::CommandQueue queue(context, cl::Device::getDefault());

    turn_action testData;
    testData.spell = -1;
    testData.boost = -1;
    testData.graze = -1;
    testData.skill = -1;
    testData.target = -1;

    queue.enqueueWriteBuffer(bufferTestData, CL_TRUE, 0, sizeof(turn_action), &testData);

    cl::KernelFunctor<cl::Buffer, cl::Buffer> testTurnAction(program, "test_turn_action");
    testTurnAction(cl::EnqueueArgs(queue, NDRange<3>(1,1,1)), bufferTestData, bufferOUT);

    std::vector<cl_ulong> out(11);
    queue.enqueueReadBuffer(bufferOUT, CL_TRUE, 0, sizeof(cl_ulong) * 11, out.data());

    // Check if type size matches
    EXPECT_EQ(out[0], sizeof(turn_action::spell));
    EXPECT_EQ(out[1], sizeof(turn_action::boost));
    EXPECT_EQ(out[2], sizeof(turn_action::graze));
    EXPECT_EQ(out[3], sizeof(turn_action::skill));
    EXPECT_EQ(out[4], sizeof(turn_action::target));
    EXPECT_EQ(out[5], sizeof(turn_action));

    // Check if type sign matches
    EXPECT_TRUE((out[6] > 0 && testData.spell > 0) || (out[6] == 0 && testData.spell < 0));
    EXPECT_TRUE((out[7] > 0 && testData.boost > 0) || (out[6] == 0 && testData.boost < 0));
    EXPECT_TRUE((out[8] > 0 && testData.graze > 0) || (out[6] == 0 && testData.graze < 0));
    EXPECT_TRUE((out[9] > 0 && testData.skill > 0) || (out[6] == 0 && testData.skill < 0));
    EXPECT_TRUE((out[10] > 0 && testData.target > 0) || (out[6] == 0 && testData.target < 0));
}
