#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Presets.h"

using APVTS = juce::AudioProcessorValueTreeState;

static const char* kPresetParamIDs[12] = {
    "distOn","distTube","distMech","filtOn","filtCut","filtRes",
    "compOn","compAmt","compMode","masterOn","masterMix","masterVol"
};

int  ElephantCrushProcessor::getNumPrograms()               { return kNumPresets; }
const juce::String ElephantCrushProcessor::getProgramName (int i)
{
    return (i >= 0 && i < kNumPresets) ? juce::String (kPresets[i].name) : juce::String();
}
void ElephantCrushProcessor::setCurrentProgram (int index)
{
    if (index < 0 || index >= kNumPresets) return;
    currentProgram = index;
    for (int p = 0; p < 12; ++p)
        if (auto* prm = apvts.getParameter (kPresetParamIDs[p]))
            prm->setValueNotifyingHost (kPresets[index].v[p]);   // values are 0..1 normalised
}

static juce::String pct (float v, int) { return juce::String (juce::roundToInt (v)) + " %"; }

APVTS::ParameterLayout ElephantCrushProcessor::createLayout()
{
    using P  = juce::AudioParameterFloat;
    using PB = juce::AudioParameterBool;
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;
    auto range = juce::NormalisableRange<float> (0.0f, 100.0f, 0.01f);

    p.push_back (std::make_unique<PB> (juce::ParameterID{"distOn",1},  "Dist On", true));
    p.push_back (std::make_unique<P>  (juce::ParameterID{"distTube",1},"Dist Tube", range, 40.0f, juce::String(), P::genericParameter, pct));
    p.push_back (std::make_unique<P>  (juce::ParameterID{"distMech",1},"Dist Mech", range, 0.0f,  juce::String(), P::genericParameter, pct));

    p.push_back (std::make_unique<PB> (juce::ParameterID{"filtOn",1},  "Filter On", true));
    p.push_back (std::make_unique<P>  (juce::ParameterID{"filtCut",1}, "Filter Cutoff", range, 100.0f, juce::String(), P::genericParameter, pct));
    p.push_back (std::make_unique<P>  (juce::ParameterID{"filtRes",1}, "Filter Res", range, 0.0f, juce::String(), P::genericParameter, pct));

    p.push_back (std::make_unique<PB> (juce::ParameterID{"compOn",1},  "Comp On", true));
    p.push_back (std::make_unique<P>  (juce::ParameterID{"compAmt",1}, "Comp Amount", range, 0.0f, juce::String(), P::genericParameter, pct));
    p.push_back (std::make_unique<PB> (juce::ParameterID{"compMode",1},"Comp Mode", false));

    p.push_back (std::make_unique<PB> (juce::ParameterID{"masterOn",1},"Master On", true));
    p.push_back (std::make_unique<P>  (juce::ParameterID{"masterMix",1},"Master Mix", range, 100.0f, juce::String(), P::genericParameter, pct));
    p.push_back (std::make_unique<P>  (juce::ParameterID{"masterVol",1},"Master Volume", range, 80.0f, juce::String(), P::genericParameter, pct));

    return { p.begin(), p.end() };
}

ElephantCrushProcessor::ElephantCrushProcessor()
    : AudioProcessor (BusesProperties()
        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMS", createLayout())
{
    pDistOn=apvts.getRawParameterValue("distOn"); pTube=apvts.getRawParameterValue("distTube"); pMech=apvts.getRawParameterValue("distMech");
    pFiltOn=apvts.getRawParameterValue("filtOn"); pCut=apvts.getRawParameterValue("filtCut");  pRes=apvts.getRawParameterValue("filtRes");
    pCompOn=apvts.getRawParameterValue("compOn"); pAmt=apvts.getRawParameterValue("compAmt");  pMode=apvts.getRawParameterValue("compMode");
    pMasterOn=apvts.getRawParameterValue("masterOn"); pMix=apvts.getRawParameterValue("masterMix"); pVol=apvts.getRawParameterValue("masterVol");
}

void ElephantCrushProcessor::prepareToPlay (double sampleRate, int)
{
    currentSR = sampleRate;
    for (int c=0;c<2;++c){ filter[c].prepare (sampleRate); comp[c].prepare (sampleRate); }
}

bool ElephantCrushProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto in  = layouts.getMainInputChannelSet();
    const auto out = layouts.getMainOutputChannelSet();
    if (in != out) return false;
    return in == juce::AudioChannelSet::mono() || in == juce::AudioChannelSet::stereo();
}

void ElephantCrushProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int numCh = buffer.getNumChannels();
    const int n = buffer.getNumSamples();

    const bool distOn = *pDistOn > 0.5f, filtOn = *pFiltOn > 0.5f, compOn = *pCompOn > 0.5f, masterOn = *pMasterOn > 0.5f;
    tube.setAmount (*pTube * 0.01f);
    mech.setAmount (*pMech * 0.01f);
    const float mix = juce::jlimit (0.0f, 1.0f, *pMix * 0.01f);
    const float vol = cc::masterVolGain (*pVol * 0.01f);

    filter[0].setCutoffRes (*pCut * 0.01f, *pRes * 0.01f);
    filter[1].setCutoffRes (*pCut * 0.01f, *pRes * 0.01f);
    comp[0].setAmount (*pAmt * 0.01f, *pMode > 0.5f);

    float* L = buffer.getWritePointer (0);
    float* R = numCh > 1 ? buffer.getWritePointer (1) : L;
    for (int i=0;i<n;++i)
    {
        const float dL = L[i], dR = R[i];
        float xL = dL, xR = dR;
        if (distOn) { xL = mech.processSample (tube.processSample (xL));
                      xR = mech.processSample (tube.processSample (xR)); }
        if (filtOn) { xL = filter[0].processSample (xL); xR = filter[1].processSample (xR); }
        if (compOn) comp[0].processStereo (xL, xR);
        const float wL = masterOn ? (dL * (1.0f - mix) + xL * mix) : xL;
        const float wR = masterOn ? (dR * (1.0f - mix) + xR * mix) : xR;
        L[i] = juce::jlimit (-1.0f, 1.0f, wL * vol);
        if (numCh > 1) R[i] = juce::jlimit (-1.0f, 1.0f, wR * vol);
    }
}

void ElephantCrushProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    if (auto xml = apvts.copyState().createXml()) copyXmlToBinary (*xml, dest);
}
void ElephantCrushProcessor::setStateInformation (const void* data, int size)
{
    if (auto xml = getXmlFromBinary (data, size))
        apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessorEditor* ElephantCrushProcessor::createEditor() { return new ElephantCrushEditor (*this); }

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new ElephantCrushProcessor(); }
