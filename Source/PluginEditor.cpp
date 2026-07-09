#include "PluginEditor.h"

static constexpr int WIN_W = 360, WIN_H = 352;

juce::Slider* ElephantCrushEditor::addKnob (std::unique_ptr<juce::Slider>& k, const juce::String& id, int cx, int cy)
{
    k = std::make_unique<juce::Slider> (juce::Slider::RotaryVerticalDrag, juce::Slider::NoTextBox);
    k->setRotaryParameters (juce::MathConstants<float>::pi * 1.2f, juce::MathConstants<float>::pi * 2.8f, true);
    k->setRange (0.0, 100.0, 0.01);
    addAndMakeVisible (*k);
    k->setBounds (cx - 24, cy - 24, 48, 48);
    sAtt.push_back (std::make_unique<SA> (proc.apvts, id, *k));
    return k.get();
}

PillToggle* ElephantCrushEditor::addToggle (std::unique_ptr<PillToggle>& b, const juce::String& id, juce::String cap, int x,int y,int w,int h)
{
    b = std::make_unique<PillToggle> (std::move (cap));
    addAndMakeVisible (*b);
    b->setBounds (x, y, w, h);
    bAtt.push_back (std::make_unique<BA> (proc.apvts, id, *b));
    return b.get();
}

ElephantCrushEditor::ElephantCrushEditor (ElephantCrushProcessor& p)
    : juce::AudioProcessorEditor (&p), proc (p)
{
    setLookAndFeel (&lnf);

    // knobs (centres)
    addKnob (kTube, "distTube", 56, 150);
    addKnob (kMech, "distMech", 120,150);
    addKnob (kCut,  "filtCut",  228,150);
    addKnob (kRes,  "filtRes",  292,150);
    addKnob (kAmt,  "compAmt",  56, 274);
    addKnob (kVol,  "masterVol",228,274);
    addKnob (kMix,  "masterMix",292,274);

    // section on/off pills
    addToggle (bDist,   "distOn",   "ON",   130, 90,  34, 15);
    addToggle (bFilt,   "filtOn",   "ON",   302, 90,  34, 15);
    addToggle (bComp,   "compOn",   "ON",   130, 214, 34, 15);
    addToggle (bMaster, "masterOn", "ON",   302, 214, 34, 15);
    addToggle (bMode,   "compMode", "PHAT", 104, 262, 48, 24);

    // randomize
    bRand = std::make_unique<KickButton>();
    addAndMakeVisible (*bRand);
    bRand->setBounds (292, 46, 54, 24);
    bRand->onClick = [this]
    {
        auto rnd = [this] (const char* id)
        {
            if (auto* pr = proc.apvts.getParameter (id))
                pr->setValueNotifyingHost (juce::Random::getSystemRandom().nextFloat());
        };
        for (auto* id : { "distTube","distMech","filtCut","filtRes","compAmt","masterMix" }) rnd (id);
    };

    // preset bar
    presetBar = std::make_unique<PresetBar> (proc);
    addAndMakeVisible (*presetBar);
    presetBar->setBounds (14, 46, 270, 24);

    setSize (WIN_W, WIN_H);
}

ElephantCrushEditor::~ElephantCrushEditor() { setLookAndFeel (nullptr); }

void ElephantCrushEditor::drawPanel (juce::Graphics& g, juce::Rectangle<int> r, const juce::String& title)
{
    auto b = r.toFloat();
    juce::ColourGradient grad (EC::panel(), b.getX(), b.getY(), EC::panel2(), b.getX(), b.getBottom(), false);
    g.setGradientFill (grad); g.fillRoundedRectangle (b, 7.0f);
    g.setColour (juce::Colours::black.withAlpha (0.5f)); g.drawRoundedRectangle (b.reduced (0.5f), 7.0f, 1.0f);
    g.setColour (EC::goldDim().withAlpha (0.25f)); g.drawRoundedRectangle (b.reduced (1.5f), 6.0f, 1.0f);
    g.setColour (EC::gold());
    g.setFont (juce::Font (11.5f, juce::Font::bold));
    g.drawText (title, r.withHeight (20).translated (12, 6), juce::Justification::centredLeft);
}

void ElephantCrushEditor::knobLabel (juce::Graphics& g, int cx, int cyBottom, const juce::String& t)
{
    g.setColour (EC::text());
    g.setFont (juce::Font (9.5f, juce::Font::bold));
    g.drawText (t, cx - 40, cyBottom, 80, 12, juce::Justification::centred);
}

void ElephantCrushEditor::paint (juce::Graphics& g)
{
    // backdrop
    juce::ColourGradient bg (juce::Colour (0xff1b1a20), 0, 0, EC::deep(), 0, (float) WIN_H, false);
    g.setGradientFill (bg); g.fillAll();
    g.setColour (EC::goldDim().withAlpha (0.55f));
    g.drawRoundedRectangle (juce::Rectangle<float> (1.5f, 1.5f, WIN_W - 3.0f, WIN_H - 3.0f), 9.0f, 1.2f);

    // title
    g.setColour (EC::gold());
    g.setFont (juce::Font (21.0f, juce::Font::bold));
    g.drawText ("Elephant", 14, 10, WIN_W - 28, 26, juce::Justification::centredLeft);
    auto tw = juce::Font (21.0f, juce::Font::bold).getStringWidth ("Elephant");
    g.setColour (EC::text());
    g.drawText ("Crush", 14 + tw + 2, 10, WIN_W - 28, 26, juce::Justification::centredLeft);
    g.setColour (EC::goldDim());
    g.setFont (juce::Font (8.5f, juce::Font::plain));
    g.drawText ("native arm64", 14, 30, WIN_W - 28, 12, juce::Justification::centredLeft);

    // panels
    drawPanel (g, { 14, 82, 160, 116 }, "DISTORTION");
    drawPanel (g, { 186, 82, 160, 116 }, "FILTER");
    drawPanel (g, { 14, 206, 160, 116 }, "COMPRESSOR");
    drawPanel (g, { 186, 206, 160, 116 }, "MASTER");

    // knob labels
    knobLabel (g, 56, 176, "TUBE");
    knobLabel (g, 120,176, "MECH");
    knobLabel (g, 228,176, "CUTOFF");
    knobLabel (g, 292,176, "RES");
    knobLabel (g, 56, 300, "AMOUNT");
    knobLabel (g, 228,300, "VOLUME");
    knobLabel (g, 292,300, "MIX");
}
