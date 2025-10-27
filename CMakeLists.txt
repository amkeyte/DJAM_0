cmake_minimum_required(VERSION 3.22)
project(DJAM_0 VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Paths (adjust if needed)
set(JUCE_DIR "C:/_LocalFiles/source/repos/juce-8.0.10-windows/JUCE" CACHE PATH "Path to JUCE")
set(VST3_SDK_DIR "C:/_LocalFiles/source/repos/vst3sdk" CACHE PATH "VST3 SDK Path")
set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/source")

# Stop JUCE from scanning/building a ton of extras
set(JUCE_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(JUCE_BUILD_EXTRAS   OFF CACHE BOOL "" FORCE)
set(JUCE_ENABLE_MODULE_SOURCE_GROUPS OFF CACHE BOOL "" FORCE)
#juce_set_vst3_sdk_path("${VST3_SDK_DIR}")


add_subdirectory("${JUCE_DIR}" "${CMAKE_BINARY_DIR}/JUCE")



# --- Plugin target
juce_add_plugin(DJAM_0
    # Target details
    COMPANY_NAME              "ArrynCo"
    PRODUCT_NAME              "D-Jam"
    BUNDLE_ID                 com.arrynco.djam
    PLUGIN_MANUFACTURER_CODE  ArCo
    PLUGIN_CODE               DJam

    # Formats
    FORMATS                   VST3 Standalone
    VST3_CATEGORIES           "Instrument|Sampler"
    COPY_PLUGIN_AFTER_BUILD   TRUE

    # Behaviour
    IS_SYNTH                  TRUE
    IS_MIDI_EFFECT            FALSE
    NEEDS_MIDI_INPUT          TRUE
    NEEDS_MIDI_OUTPUT         FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
)

# New project: don't try to replace VST2 (avoids param-ID warning)
target_compile_definitions(DJAM_0 PRIVATE
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
    JUCE_REPORT_APP_USAGE=0
    JUCE_STRICT_REFCOUNTEDPOINTER=1
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
)

juce_generate_juce_header(DJAM_0)

# --- Sources (use ${SRC_DIR})
target_sources(DJAM_0 PRIVATE
    ${SRC_DIR}/PluginProcessor.h
    ${SRC_DIR}/PluginProcessor.cpp
    ${SRC_DIR}/PluginEditor.h
    ${SRC_DIR}/PluginEditor.cpp
    ${SRC_DIR}/DJamHostSync.h
    ${SRC_DIR}/DJamHostSync.cpp
    ${SRC_DIR}/QuantizedScheduler.h
    ${SRC_DIR}/QuantizedScheduler.cpp
    ${SRC_DIR}/DJamClip.h
    ${SRC_DIR}/DJamClip.cpp
    ${SRC_DIR}/Slot.h
    ${SRC_DIR}/Slot.cpp
    ${SRC_DIR}/SceneBank.h
    ${SRC_DIR}/SceneBank.cpp
    ${SRC_DIR}/SlotRow.h
    ${SRC_DIR}/SlotRow.cpp
)

# --- Optional: embed small JSON/UI assets. Put files in Source/Assets/
# juce_add_binary_data(DJAM_Assets SOURCES
#     Source/Assets/SamplePack.json
#     Source/Assets/ui_icons.png
# )
# target_link_libraries(DJAM_0 PRIVATE DJAM_Assets)

# --- JUCE modules
target_link_libraries(DJAM_0 PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    juce::juce_gui_extra
)

# Help IntelliSense find the generated header
target_include_directories(DJAM_0 PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

# --- Warnings & build flags (MSVC-friendly)
if (MSVC)
    target_compile_options(DJAM_0 PRIVATE /W4 /permissive- /Zc:__cplusplus /MP)
else()
    target_compile_options(DJAM_0 PRIVATE -Wall -Wextra -Wpedantic)
endif()

# --- (Optional) Unity/Jumbo build for faster iteration
# set_property(TARGET DJAM_0 PROPERTY UNITY_BUILD ON)

# --- (Optional) Post-build: copy a Samples/ folder next to the .vst3 for easy loading
#   Adjust path as desired. On Windows, VST3 ends up in:
#   <build>\DJAM_0_artefacts\Debug\VST3\D-Jam.vst3\
set(DJAM_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/DJAM_0_artefacts/$<CONFIG>/VST3/D-Jam.vst3/Contents/Resources")
add_custom_command(TARGET DJAM_0 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${DJAM_OUTPUT_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/Samples"
            "${DJAM_OUTPUT_DIR}/Samples"
    COMMENT "Copying sample pack to plugin Resources/Samples"
    VERBATIM
)

# --- (Nice to have) Version info into the binary
target_compile_definitions(DJAM_0 PRIVATE
    DJAM_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
    DJAM_VERSION_MINOR=${PROJECT_VERSION_MINOR}
    DJAM_VERSION_PATCH=${PROJECT_VERSION_PATCH}
)
