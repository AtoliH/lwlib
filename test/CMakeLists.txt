include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/e2239ee6043f73722e7aa812a459f54a28552929.zip) # release-1.11.0
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(
    lw_opencl_test
    lw_opencl_test_types_compatibility.cpp)

target_link_libraries(
    lw_opencl_test
    lwlib
    gtest_main)

include(GoogleTest)
gtest_discover_tests(lw_opencl_test)
