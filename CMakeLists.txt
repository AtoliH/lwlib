cmake_minimum_required(VERSION 3.23)
project(lwlib VERSION 0.0.1 DESCRIPTION "Tools for Touhou LostWord advanced strategy")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_OBJECT_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/obj)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/obj)

set(CMAKE_CXX_STANDARD 17)

set(SOURCE_FILES
    src/TurnAction.cpp
    src/Attack.cpp
    src/Character.cpp
    src/Analyzer.cpp
    src/Modifier.cpp

    src/FastDevice.cpp

    src/incbeta.c
)

set(HEADER_FILES 
    include/lwlib/TurnAction.hpp
    include/lwlib/Attack.hpp
    include/lwlib/Character.hpp
    include/lwlib/Analyzer.hpp
    include/lwlib/Modifier.hpp

    include/lwlib/FastDevice.hpp

    # TODO: Public for now but will maybe have to move to private
    include/lwlib/internal/types.h
    include/lwlib/internal/macro.h
)

set(PRIVATE_HEADER_FILES
   include/lwlib/internal/incbeta.h
)

add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES} ${HEADER_FILES} ${PRIVATE_HEADER_FILES})

target_sources(${PROJECT_NAME}
        PRIVATE ${SOURCE_FILES} ${PRIVATE_HEADER_FILES}
        PUBLIC FILE_SET HEADERS
        BASE_DIRS ${PROJECT_SOURCE_DIR}/include
        FILES ${HEADER_FILES}
)

set_target_properties(lwlib PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 0
    PUBLIC_HEADER "${HEADER_FILES}")

if(MSVC)
  target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
else()
  target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()

set(lwlib_build_include_dirs
    "${lwlib_SOURCE_DIR}/include"
    "${CMAKE_SOURCE_DIR}/thirdparty/OpenCL/include")

target_include_directories(lwlib SYSTEM PUBLIC
    "$<BUILD_INTERFACE:${lwlib_build_include_dirs}>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")


include(FindOpenCL)
target_link_libraries(lwlib PUBLIC ${OpenCL_LIBRARIES})

# Embed file in binary: https://stackoverflow.com/a/56006001
function( add_resource input directory )
    file(RELATIVE_PATH input_rel ${directory} ${input})
    string( MAKE_C_IDENTIFIER ${input_rel} input_identifier )
    set( output "${CMAKE_OBJECT_OUTPUT_DIRECTORY}/${input_identifier}.o" )

    if (APPLE)
        add_custom_command(
            OUTPUT ${output}
            COMMAND touch ${CMAKE_OBJECT_OUTPUT_DIRECTORY}/stub.c
            COMMAND gcc -o ${CMAKE_OBJECT_OUTPUT_DIRECTORY}/stub.o -c ${CMAKE_OBJECT_OUTPUT_DIRECTORY}/stub.c
            COMMAND ld -r -o ${output} -sectcreate binary ${input_identifier}_bin ${input} ${CMAKE_OBJECT_OUTPUT_DIRECTORY}/stub.o
            DEPENDS ${input}
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
    else ()
        add_custom_command(
            OUTPUT ${output}
            COMMAND ${CMAKE_LINKER} --relocatable --format binary --output ${output} ${input}
            DEPENDS ${input}
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
    endif ()

    # Scuffed but it works, that way the dependencies are good
    add_custom_target("binary_${input_identifier}" DEPENDS ${output})
    add_library("binary_${input_identifier}lib" INTERFACE)
    add_dependencies("binary_${input_identifier}lib" "binary_${input_identifier}")

    target_link_libraries(${PROJECT_NAME} PRIVATE "binary_${input_identifier}lib")
    target_link_libraries(${PROJECT_NAME} PRIVATE ${output})
endfunction()

add_resource("${PROJECT_SOURCE_DIR}/src/opencl/dmgcalc.cl" "${PROJECT_SOURCE_DIR}/src/opencl")
add_resource("${PROJECT_SOURCE_DIR}/src/opencl/types.cl" "${PROJECT_SOURCE_DIR}/src/opencl")
add_resource("${PROJECT_SOURCE_DIR}/include/lwlib/internal/macro.h" "${PROJECT_SOURCE_DIR}/include/lwlib/internal")

include(GNUInstallDirs)

install(TARGETS lwlib
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
    
include(cmake/clang-cxx-dev-tools.cmake)

enable_testing()
add_subdirectory(test)

