cmake_minimum_required(VERSION 3.22)

project(DJAM_0 VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -------------------------------------------------------------------
# 🧱 JUCE and VST3 paths
# -------------------------------------------------------------------

set(JUCE_DIR "C:/_LocalFiles/source/repos/juce-8.0.10-windows/JUCE" CACHE PATH "Path to JUCE")
set(VST3_SDK_DIR "C:/_LocalFiles/source/repos/vst3sdk" CACHE PATH "Path to VST3 SDK")

set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/source")

# -------------------------------------------------------------------
# 🔧 Optional JUCE options
# -------------------------------------------------------------------

set(JUCE_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(JUCE_BUILD_EXTRAS OFF CACHE BOOL "" FORCE)
set(JUCE_ENABLE_MODULE_SOURCE_GROUPS OFF CACHE BOOL "" FORCE)

# -------------------------------------------------------------------
# 📦 Add JUCE
# -------------------------------------------------------------------

add_subdirectory("${JUCE_DIR}" "${CMAKE_BINARY_DIR}/JUCE")
juce_set_vst3_sdk_path("${VST3_SDK_DIR}")

# -------------------------------------------------------------------
# 🎛️ Define the plugin
# -------------------------------------------------------------------

juce_add_plugin(DJAM_0
    COMPANY_NAME              "ArrynCo"
    PRODUCT_NAME              "D-Jam"
    BUNDLE_ID                 com.arrynco.djam
    PLUGIN_MANUFACTURER_CODE  ArCo
    PLUGIN_CODE               DJam
    FORMATS                   VST3 Standalone
    VST3_CATEGORIES           "Instrument|Sampler"
    COPY_PLUGIN_AFTER_BUILD   TRUE
    IS_SYNTH                  TRUE
    IS_MIDI_EFFECT            FALSE
    NEEDS_MIDI_INPUT          TRUE
    NEEDS_MIDI_OUTPUT         FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
)

juce_generate_juce_header(DJAM_0)

# -------------------------------------------------------------------
# 📂 Automatically add all .cpp/.h files in /source
# -------------------------------------------------------------------

file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS
    "${SRC_DIR}/*.cpp"
    "${SRC_DIR}/*.h"
)

list(FILTER PROJECT_SOURCES EXCLUDE REGEX "scrap\\.txt$")

target_sources(DJAM_0 PRIVATE ${PROJECT_SOURCES})

message(STATUS "SRC_DIR = ${SRC_DIR}")
foreach(f ${PROJECT_SOURCES})
    message(STATUS " - Found: ${f}")
endforeach()

# -------------------------------------------------------------------
# 📚 Link against required JUCE modules
# -------------------------------------------------------------------

target_link_libraries(DJAM_0 PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    juce::juce_gui_extra
)

# -------------------------------------------------------------------
# 🛠️ Compiler flags
# -------------------------------------------------------------------

if (MSVC)
    target_compile_options(DJAM_0 PRIVATE /W4 /permissive- /Zc:__cplusplus /MP)
else()
    target_compile_options(DJAM_0 PRIVATE -Wall -Wextra -Wpedantic)
endif()

target_compile_definitions(DJAM_0 PRIVATE
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
    JUCE_REPORT_APP_USAGE=0
    JUCE_STRICT_REFCOUNTEDPOINTER=1
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0

    DJAM_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
    DJAM_VERSION_MINOR=${PROJECT_VERSION_MINOR}
    DJAM_VERSION_PATCH=${PROJECT_VERSION_PATCH}
)


# -------------------------------------------------------------------
# 🔎 Optional: Help IntelliSense find generated headers
# -------------------------------------------------------------------

target_include_directories(DJAM_0 PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
