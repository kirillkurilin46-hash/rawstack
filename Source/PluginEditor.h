#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>
#include "PluginProcessor.h"

namespace ui
{
    const juce::Colour bg      { 0xff0f131d };
    const juce::Colour panel   { 0xff171d2c };
    const juce::Colour edge    { 0xff2a3248 };
    const juce::Colour text    { 0xffdce2f1 };
    const juce::Colour muted   { 0xff8992ad };
    const juce::Colour amber   { 0xffffb547 };
    const juce::Colour cyan    { 0xff7fd8ff };
    const juce::Colour rose    { 0xffff7a9c };
    const juce::Colour screen  { 0xff0b1020 };
    const juce::Colour knob    { 0xff252d42 };
    const juce::Colour track   { 0xff2f3850 };
    inline juce::Colour noteColour (int i)
    {
        static const juce::uint32 c[7] = { 0xffffb547, 0xffff7a9c, 0xff7fd8ff, 0xffa8f0a0, 0xffc9a6ff, 0xffffd27a, 0xff6fe3d2 };
        return juce::Colour (c[((i % 7) + 7) % 7]);
    }
}

class RawLookAndFeel : public juce::LookAndFeel_V4
{
public:
    RawLookAndFeel();
    void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h, float pos, float start, float end, juce::Slider&) override;
};

class Visualizer : public juce::Component, private juce::Timer
{
public:
    explicit Visualizer (RawstackProcessor&);
    ~Visualizer() override { stopTimer(); }
    void paint (juce::Graphics&) override;

private:
    void timerCallback() override;
    void drawLanes (juce::Graphics&, juce::Rectangle<float>, float windowSec);
    void drawOutput (juce::Graphics&, juce::Rectangle<float>, float windowSec);
    void drawSpectrum (juce::Graphics&, juce::Rectangle<float>);
    void drawReadout (juce::Graphics&, juce::Rectangle<float>);

    RawstackProcessor& proc;
    rs::VizState viz;
    std::vector<float> phases;
    double lastTime = 0;
    static constexpr int fftOrder = 12, fftSize = 1 << fftOrder;
    juce::dsp::FFT fft { fftOrder };
    std::vector<float> fftData, spectrumDb, scopeCopy;
};

class Cell : public juce::Component
{
public:
    Cell (RawstackProcessor&, const ParamDef&);
    void resized() override;
    int units() const { return isChoice ? 2 : 1; }

private:
    bool isChoice;
    juce::Label label;
    juce::Slider slider;
    juce::ComboBox box;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> cAtt;
};

class Group : public juce::Component
{
public:
    Group (juce::String title, juce::String subtitle, int widthUnits);
    void addCell (std::unique_ptr<Cell>);
    int preferredWidth() const;
    int preferredHeight() const;
    void paint (juce::Graphics&) override;
    void resized() override;

    static constexpr int unit = 74, cellH = 92, pad = 10, titleH = 28;

private:
    juce::String title, subtitle;
    int widthUnits;
    std::vector<std::unique_ptr<Cell>> cells;
    int rowsFor (int wu) const;
};

class RawstackEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit RawstackEditor (RawstackProcessor&);
    ~RawstackEditor() override;
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void layoutContent (int width);

    RawstackProcessor& proc;
    RawLookAndFeel lnf;
    Visualizer visualizer;
    juce::ComboBox presetBox;
    juce::TextButton prevBtn { "<" }, nextBtn { ">" };
    juce::Viewport viewport;
    juce::Component content;
    std::vector<std::unique_ptr<Group>> groups;
    int shownProgram = -1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RawstackEditor)
};
