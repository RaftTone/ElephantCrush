cmake_minimum_required(VERSION 3.22)
project(ElephantCrush VERSION 1.0.0)

# --- Native Apple Silicon build (also builds x86_64 -> universal) -------------
set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "" FORCE)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "" FORCE)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
    juce
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.4
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(juce)

juce_add_plugin(ElephantCrush
    PLUGIN_MANUFACTURER_CODE  Rfdt
    PLUGIN_CODE               ElCr
    COMPANY_NAME              "rafttone"
    PRODUCT_NAME              "ElephantCrush"
    FORMATS                   AU VST3
    IS_SYNTH                  FALSE
    NEEDS_MIDI_INPUT          FALSE
    COPY_PLUGIN_AFTER_BUILD   TRUE
    AU_MAIN_TYPE              kAudioUnitType_Effect
    VST3_CATEGORIES           Fx Distortion
    BUNDLE_ID                 com.rafttone.ElephantCrush
)

# No image/binary assets — the whole UI is drawn procedurally in code.

target_sources(ElephantCrush PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
)

target_compile_definitions(ElephantCrush PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)

target_link_libraries(ElephantCrush PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)
