#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"

// ===== Fully procedural dark+gold look — NO image assets of any kind ==========
struct EC
{
    static juce::Colour gold()   { return juce::Colour (0xffE7B84B); }
    static juce::Colour goldDim() { return juce::Colour (0xff8a6f2e); }
    static juce::Colour panel()  { return juce::Colour (0xff26252b); }
    static juce::Colour panel2() { return juce::Colour (0xff1c1b21); }
    static juce::Colour deep()   { return juce::Colour (0xff111015); }
    static juce::Colour text()   { return juce::Colour (0xffCbb98a); }
};

class ECLookAndFeel : public juce::LookAndFeel_V4
{
public:
    void drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h,
                           float pos, float a0, float a1, juce::Slider&) override
    {
        auto b  = juce::Rectangle<float> ((float) x, (float) y, (float) w, (float) h).reduced (7.0f);
        auto cx = b.getCentreX(), cy = b.getCentreY();
        auto r  = juce::jmin (b.getWidth(), b.getHeight()) * 0.5f;
        // body with subtle vertical shade
        juce::ColourGradient grad (juce::Colour (0xff34333a), cx, cy - r, juce::Colour (0xff1e1d23), cx, cy + r, false);
        g.setGradientFill (grad); g.fillEllipse (cx - r, cy - r, 2 * r, 2 * r);
        g.setColour (juce::Colours::black.withAlpha (0.45f)); g.drawEllipse (cx - r, cy - r, 2 * r, 2 * r, 1.4f);
        // outer track
        const float rr = r + 3.5f;
        juce::Path track; track.addCentredArc (cx, cy, rr, rr, 0.0f, a0, a1, true);
        g.setColour (juce::Colour (0xff3c3b45)); g.strokePath (track, juce::PathStrokeType (2.6f));
        // value arc
        const float ang = a0 + pos * (a1 - a0);
        juce::Path val; val.addCentredArc (cx, cy, rr, rr, 0.0f, a0, ang, true);
        g.setColour (EC::gold()); g.strokePath (val, juce::PathStrokeType (2.6f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        // pointer
        juce::Path p; p.addRoundedRectangle (-1.3f, -r + 3.0f, 2.6f, r * 0.5f, 1.3f);
        g.setColour (EC::gold()); g.fillPath (p, juce::AffineTransform::rotation (ang).translated (cx, cy));
        // hub
        g.setColour (juce::Colour (0xff141319)); g.fillEllipse (cx - 3.0f, cy - 3.0f, 6.0f, 6.0f);
    }
};

// 2-state pill toggle that shows a caption ("ON" / "PHAT") — gold when active.
class PillToggle : public juce::Button
{
public:
    explicit PillToggle (juce::String caption) : juce::Button ({}), text (std::move (caption))
    { setClickingTogglesState (true); }
    void paintButton (juce::Graphics& g, bool, bool) override
    {
        auto b = getLocalBounds().toFloat().reduced (1.0f);
        const bool on = getToggleState();
        g.setColour (on ? EC::gold() : juce::Colour (0xff2a2930));
        g.fillRoundedRectangle (b, 4.0f);
        g.setColour (juce::Colours::black.withAlpha (0.5f)); g.drawRoundedRectangle (b, 4.0f, 1.0f);
        g.setColour (on ? juce::Colour (0xff201700) : EC::gold().withAlpha (0.5f));
        g.setFont (juce::Font (juce::jmin (11.0f, b.getHeight() * 0.62f), juce::Font::bold));
        g.drawText (text, getLocalBounds(), juce::Justification::centred);
    }
private:
    juce::String text;
};

// Momentary "RND" button.
class KickButton : public juce::Button
{
public:
    KickButton() : juce::Button ({}) {}
    void paintButton (juce::Graphics& g, bool, bool down) override
    {
        auto b = getLocalBounds().toFloat().reduced (1.0f);
        g.setColour (down ? EC::goldDim() : juce::Colour (0xff2a2930));
        g.fillRoundedRectangle (b, 4.0f);
        g.setColour (juce::Colours::black.withAlpha (0.5f)); g.drawRoundedRectangle (b, 4.0f, 1.0f);
        g.setColour (EC::gold()); g.setFont (juce::Font (10.0f, juce::Font::bold));
        g.drawText ("RND", getLocalBounds(), juce::Justification::centred);
    }
};

// Preset bar: dark field + gold arrow zones + name; click middle for dropdown.
class PresetBar : public juce::Component
{
public:
    explicit PresetBar (ElephantCrushProcessor& p) : proc (p) {}
    std::function<void()> onChange;
    void paint (juce::Graphics& g) override
    {
        auto b = getLocalBounds().toFloat().reduced (0.5f);
        g.setColour (EC::deep()); g.fillRoundedRectangle (b, 4.0f);
        g.setColour (EC::goldDim().withAlpha (0.7f)); g.drawRoundedRectangle (b, 4.0f, 1.0f);
        const float cy = getHeight() * 0.5f;
        drawArrow (g, 11.0f, cy, true,  pressed == 2);
        drawArrow (g, getWidth() - 11.0f, cy, false, pressed == 1);
        g.setColour (EC::gold());
        g.setFont (juce::Font (12.0f, juce::Font::bold));
        g.drawText (proc.getProgramName (proc.getCurrentProgram()),
                    getLocalBounds().reduced (24, 0), juce::Justification::centred, true);
    }
    void drawArrow (juce::Graphics& g, float cx, float cy, bool left, bool press)
    {
        const float w = 3.4f, h = 5.0f; juce::Path p;
        if (left) { p.startNewSubPath (cx - w, cy); p.lineTo (cx + w, cy - h); p.lineTo (cx + w, cy + h); }
        else      { p.startNewSubPath (cx + w, cy); p.lineTo (cx - w, cy - h); p.lineTo (cx - w, cy + h); }
        p.closeSubPath();
        g.setColour (press ? juce::Colours::white : EC::gold());
        g.fillPath (p);
    }
    void step (int d)
    {
        int n = proc.getNumPrograms();
        proc.setCurrentProgram ((proc.getCurrentProgram() + d + n) % n);
        repaint(); if (onChange) onChange();
    }
    void mouseDown (const juce::MouseEvent& e) override
    {
        const int w = getWidth();
        if (e.x < 22)          { pressed = 2; step (-1); }
        else if (e.x > w - 22) { pressed = 1; step (+1); }
        else
        {
            juce::PopupMenu m;
            for (int i = 0; i < proc.getNumPrograms(); ++i)
                m.addItem (i + 1, proc.getProgramName (i), true, i == proc.getCurrentProgram());
            m.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (this),
                [this](int r){ if (r > 0) { proc.setCurrentProgram (r - 1); repaint(); if (onChange) onChange(); } });
        }
        repaint();
    }
    void mouseUp (const juce::MouseEvent&) override { pressed = 0; repaint(); }
private:
    ElephantCrushProcessor& proc; int pressed = 0;
};

class ElephantCrushEditor : public juce::AudioProcessorEditor
{
public:
    explicit ElephantCrushEditor (ElephantCrushProcessor&);
    ~ElephantCrushEditor() override;
    void paint (juce::Graphics&) override;

private:
    using SA = juce::AudioProcessorValueTreeState::SliderAttachment;
    using BA = juce::AudioProcessorValueTreeState::ButtonAttachment;

    ElephantCrushProcessor& proc;
    ECLookAndFeel lnf;

    std::unique_ptr<juce::Slider> kTube,kMech,kCut,kRes,kAmt,kVol,kMix;
    std::unique_ptr<PillToggle>   bDist,bFilt,bComp,bMaster,bMode;
    std::unique_ptr<KickButton>   bRand;
    std::unique_ptr<PresetBar>    presetBar;
    std::vector<std::unique_ptr<SA>> sAtt;
    std::vector<std::unique_ptr<BA>> bAtt;

    juce::Slider* addKnob (std::unique_ptr<juce::Slider>&, const juce::String& id, int cx, int cy);
    PillToggle*   addToggle (std::unique_ptr<PillToggle>&, const juce::String& id, juce::String cap, int x,int y,int w,int h);
    void drawPanel (juce::Graphics&, juce::Rectangle<int>, const juce::String& title);
    void knobLabel (juce::Graphics&, int cx, int cyBottom, const juce::String&);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ElephantCrushEditor)
};
