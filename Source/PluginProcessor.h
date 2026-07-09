#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "CrusherDSP.h"

class ElephantCrushProcessor : public juce::AudioProcessor
{
public:
    ElephantCrushProcessor();
    ~ElephantCrushProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "ElephantCrush"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override;
    int getCurrentProgram() override { return currentProgram; }
    void setCurrentProgram (int) override;
    const juce::String getProgramName (int) override;
    void changeProgramName (int, const juce::String&) override {}
    int currentProgram = 0;

    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    juce::AudioProcessorValueTreeState apvts;
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();

private:
    cc::DistTube    tube;
    cc::DistMech    mech;
    cc::MMFilter    filter[2];
    cc::Compressor  comp[2];
    double currentSR = 44100.0;

    std::atomic<float>* pDistOn=nullptr;  std::atomic<float>* pTube=nullptr;  std::atomic<float>* pMech=nullptr;
    std::atomic<float>* pFiltOn=nullptr;  std::atomic<float>* pCut=nullptr;   std::atomic<float>* pRes=nullptr;
    std::atomic<float>* pCompOn=nullptr;  std::atomic<float>* pAmt=nullptr;   std::atomic<float>* pMode=nullptr;
    std::atomic<float>* pMasterOn=nullptr;std::atomic<float>* pMix=nullptr;   std::atomic<float>* pVol=nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ElephantCrushProcessor)
};
