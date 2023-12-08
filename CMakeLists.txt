cmake_minimum_required(VERSION 3.16)
project(SFMLproject LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(COPY_ASSETS "false" CACHE STRING "Copy the assets folder to the build/bin directory")
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

include(FetchContent)
FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 2.6.x)
FetchContent_MakeAvailable(SFML)

FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui
  GIT_TAG 35b1148efb839381b84de9290d9caf0b66ad7d03
)
FetchContent_MakeAvailable(imgui)

FetchContent_Declare(
  imgui-sfml
  GIT_REPOSITORY https://github.com/SFML/imgui-sfml
  GIT_TAG 82dc2033e51b8323857c3ae1cf1f458b3a933c35
)
add_subdirectory(imgui-sfml)

add_executable(SFMLproject src/main.cpp)
target_link_libraries(SFMLproject PRIVATE sfml-graphics ImGui-SFML::ImGui-SFML)
target_compile_features(SFMLproject PRIVATE cxx_std_17)

if(WIN32)
    add_custom_command(
        TARGET SFMLproject
        COMMENT "Copy OpenAL DLL"
        PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${SFML_SOURCE_DIR}/extlibs/bin/$<IF:$<EQUAL:${CMAKE_SIZEOF_VOID_P},8>,x64,x86>/openal32.dll $<TARGET_FILE_DIR:SFMLproject>
        VERBATIM)
    # If Release build, turn on compiler optimizations and turn off debug info + console
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
            target_compile_options(SFMLproject PRIVATE /O2 /Zi)
            target_link_options(SFMLproject PRIVATE /OPT:REF /OPT:ICF /DEBUG:NONE /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup)
        elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
            target_compile_options(SFMLproject PRIVATE -O3 -s)
            target_link_options(SFMLproject PRIVATE -s)
        else ()
            message(FATAL_ERROR "Unknown compiler ${CMAKE_CXX_COMPILER_ID} for Release build")
        endif()
        # No need to copy DLLs in Release mode, it's statically linked
    endif ()

endif()

if (COPY_ASSETS STREQUAL "true")

    # Copy the assets folder to the build/bin directory
    add_custom_command(
        TARGET SFMLproject
        COMMENT "Copy assets folder"
        POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:SFMLproject>/assets
        VERBATIM
    )

endif()

install(TARGETS SFMLproject)
