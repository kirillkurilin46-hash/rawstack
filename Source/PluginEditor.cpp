#include "PluginEditor.h"

// ===================== внешний вид =====================
RawLookAndFeel::RawLookAndFeel()
{
    setColour (juce::ResizableWindow::backgroundColourId, ui::bg);
    setColour (juce::ComboBox::backgroundColourId, ui::knob);
    setColour (juce::ComboBox::outlineColourId, ui::edge);
    setColour (juce::ComboBox::textColourId, ui::text);
    setColour (juce::ComboBox::arrowColourId, ui::amber);
    setColour (juce::PopupMenu::backgroundColourId, ui::panel);
    setColour (juce::PopupMenu::textColourId, ui::text);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, ui::amber.withAlpha (0.25f));
    setColour (juce::PopupMenu::highlightedTextColourId, ui::text);
    setColour (juce::PopupMenu::headerTextColourId, ui::amber);
    setColour (juce::Label::textColourId, ui::text);
    setColour (juce::Slider::textBoxTextColourId, ui::text);
    setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    setColour (juce::TextButton::buttonColourId, ui::knob);
    setColour (juce::TextButton::textColourOffId, ui::text);
    setColour (juce::ScrollBar::thumbColourId, ui::edge);
}

void RawLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos, float start, float end, juce::Slider&)
{
    auto b = juce::Rectangle<float> ((float) x, (float) y, (float) w, (float) h).reduced (4.0f);
    const float size = juce::jmin (b.getWidth(), b.getHeight());
    b = b.withSizeKeepingCentre (size, size);
    const auto c = b.getCentre();
    const float r = size * 0.5f - 2.0f;
    const float ang = start + pos * (end - start);

    juce::Path trackArc; trackArc.addCentredArc (c.x, c.y, r, r, 0.0f, start, end, true);
    g.setColour (ui::track); g.strokePath (trackArc, juce::PathStrokeType (3.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    if (pos > 0.002f)
    {
        juce::Path valArc; valArc.addCentredArc (c.x, c.y, r, r, 0.0f, start, ang, true);
        g.setColour (ui::amber); g.strokePath (valArc, juce::PathStrokeType (3.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }
    const float fr = r * 0.66f;
    g.setColour (ui::knob); g.fillEllipse (c.x - fr, c.y - fr, fr * 2, fr * 2);
    g.setColour (ui::edge); g.drawEllipse (c.x - fr, c.y - fr, fr * 2, fr * 2, 1.0f);
    const juce::Point<float> tip (c.x + fr * 0.85f * std::sin (ang), c.y - fr * 0.85f * std::cos (ang));
    g.setColour (ui::text); g.drawLine (c.x, c.y, tip.x, tip.y, 2.2f);
}

// ===================== формат значений =====================
static juce::String formatValue (const char* fmt, double v)
{
    const juce::String f (fmt);
    if (f == "pct")  return juce::String (juce::roundToInt (v * 100.0)) + "%";
    if (f == "int")  return juce::String (juce::roundToInt (v));
    if (f == "ct±")  return u8 ("±") + juce::String (juce::roundToInt (v)) + u8 (" ц");
    if (f == "ct")   return juce::String (juce::roundToInt (v)) + u8 (" ц");
    if (f == "rate") return juce::String (v, 2) + u8 (" Гц");
    if (f == "st")   return (v > 0 ? "+" : "") + juce::String (v, 1) + u8 (" пт");
    if (f == "sec")  return v < 1.0 ? juce::String (juce::roundToInt (v * 1000.0)) + u8 (" мс") : juce::String (v, 2) + u8 (" с");
    if (f == "hz")   return v >= 1000.0 ? juce::String (v / 1000.0, v >= 10000.0 ? 0 : 1) + u8 (" кГц") : juce::String (juce::roundToInt (v)) + u8 (" Гц");
    if (f == "x1s")  return juce::String (v, 1) + u8 (" с");
    if (f == "x1")   return juce::String (v, 1);
    return juce::String (v, 2);
}

// ===================== ячейка параметра =====================
Cell::Cell (RawstackProcessor& p, const ParamDef& d) : isChoice (d.type == 'c')
{
    label.setText (u8 (d.name), juce::dontSendNotification);
    label.setJustificationType (juce::Justification::centred);
    label.setColour (juce::Label::textColourId, ui::muted);
    label.setFont (juce::Font (juce::FontOptions (12.0f)));
    label.setMinimumHorizontalScale (0.7f);
    addAndMakeVisible (label);

    if (isChoice)
    {
        for (int i = 0; i < (int) d.choices.size(); ++i) box.addItem (u8 (d.choices[(size_t) i]), i + 1);
        addAndMakeVisible (box);
        cAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (p.apvts, d.id, box);
    }
    else
    {
        slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 72, 16);
        slider.setRotaryParameters (juce::MathConstants<float>::pi * 1.25f, juce::MathConstants<float>::pi * 2.75f, true);
        slider.setVelocityBasedMode (false);
        addAndMakeVisible (slider);
        sAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (p.apvts, d.id, slider);
        const char* fmt = d.fmt;
        slider.textFromValueFunction = [fmt] (double v) { return formatValue (fmt, v); };
        slider.updateText();
    }
}

void Cell::resized()
{
    auto b = getLocalBounds();
    if (isChoice)
    {
        label.setBounds (b.removeFromTop (22));
        box.setBounds (b.removeFromTop (28).reduced (4, 0));
    }
    else
    {
        label.setBounds (b.removeFromBottom (18));
        slider.setBounds (b);
    }
}

// ===================== группа =====================
Group::Group (juce::String t, juce::String s, int wu) : title (std::move (t)), subtitle (std::move (s)), widthUnits (wu) {}

void Group::addCell (std::unique_ptr<Cell> c) { addAndMakeVisible (*c); cells.push_back (std::move (c)); }

int Group::rowsFor (int wu) const
{
    int rows = 1, x = 0;
    for (auto& c : cells) { if (x + c->units() > wu) { ++rows; x = 0; } x += c->units(); }
    return rows;
}

int Group::preferredWidth() const  { return widthUnits * unit + pad * 2; }
int Group::preferredHeight() const { return titleH + rowsFor (widthUnits) * cellH + pad; }

void Group::paint (juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat().reduced (0.5f);
    g.setColour (ui::panel); g.fillRoundedRectangle (b, 10.0f);
    g.setColour (ui::edge); g.drawRoundedRectangle (b, 10.0f, 1.0f);
    g.setColour (ui::text); g.setFont (juce::FontOptions (15.0f, juce::Font::bold));
    g.drawText (title, pad, 6, getWidth() - pad * 2, 18, juce::Justification::left);
    g.setColour (ui::muted); g.setFont (juce::FontOptions (11.5f));
    g.drawText (subtitle, pad, 6, getWidth() - pad * 2, 18, juce::Justification::right);
}

void Group::resized()
{
    int x = 0, row = 0;
    for (auto& c : cells)
    {
        if (x + c->units() > widthUnits) { ++row; x = 0; }
        c->setBounds (pad + x * unit, titleH + row * cellH, c->units() * unit, cellH);
        x += c->units();
    }
}

// ===================== визуализация =====================
Visualizer::Visualizer (RawstackProcessor& p) : proc (p)
{
    phases.assign ((size_t) (rs::Engine::MAX_NOTES * 48), 0.0f);
    fftData.assign ((size_t) fftSize * 2, 0.0f);
    spectrumDb.assign ((size_t) fftSize / 2, -100.0f);
    lastTime = juce::Time::getMillisecondCounterHiRes() * 0.001;
    startTimerHz (30);
}

static float waveAt (const rs::VizState& v, int layer, float ph)
{
    const auto& wb = rs::WaveBank::get();
    ph -= std::floor (ph);
    if (layer == 2) return wb.read (rs::W_SINE, 0, ph);
    float a = wb.read (v.waveA, 0, ph);
    if (v.waveMix > 0.001f) a += (wb.read (v.waveB, 0, ph) - a) * v.waveMix;
    return a;
}

void Visualizer::timerCallback()
{
    const double now = juce::Time::getMillisecondCounterHiRes() * 0.001;
    const float dt = (float) juce::jlimit (0.0, 0.1, now - lastTime);
    lastTime = now;
    proc.engine.copyViz (viz);
    for (int i = 0; i < viz.numNotes; ++i)
    {
        const auto& n = viz.notes[i];
        for (int k = 0; k < n.numOsc; ++k)
        {
            auto& ph = phases[(size_t) (n.slot * 48 + k)];
            ph += rs::mtof (n.midi + n.osc[k].semis) * dt;
            ph -= std::floor (ph);
        }
    }
    // копия выхода и спектр
    const int S = rs::Engine::SCOPE;
    const int pos = proc.engine.scopePos.load();
    scopeCopy.resize ((size_t) S);
    for (int i = 0; i < S; ++i) scopeCopy[(size_t) i] = proc.engine.scope[(size_t) ((pos + i) & (S - 1))];
    std::fill (fftData.begin(), fftData.end(), 0.0f);
    for (int i = 0; i < fftSize; ++i)
    {
        const float w = 0.5f - 0.5f * std::cos (2.0f * juce::MathConstants<float>::pi * (float) i / (float) (fftSize - 1));
        fftData[(size_t) i] = scopeCopy[(size_t) (S - fftSize + i)] * w;
    }
    fft.performFrequencyOnlyForwardTransform (fftData.data());
    for (int i = 0; i < fftSize / 2; ++i)
    {
        const float db = juce::Decibels::gainToDecibels (fftData[(size_t) i] / (float) (fftSize / 4), -120.0f);
        auto& s = spectrumDb[(size_t) i];
        s = db > s ? db : s * 0.75f + db * 0.25f;
    }
    repaint();
}

void Visualizer::paint (juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat();
    auto readout = b.removeFromRight (250.0f);
    b.removeFromRight (10.0f);
    g.setColour (ui::screen); g.fillRoundedRectangle (b, 10.0f);
    g.fillRoundedRectangle (readout, 10.0f);

    auto inner = b.reduced (8.0f);
    auto spec = inner.removeFromBottom (inner.getHeight() * 0.3f);
    auto lanes = inner.removeFromTop (inner.getHeight() * 0.64f);
    auto out = inner;

    // сетка
    g.setColour (juce::Colour (0x17788cc8));
    for (int i = 1; i < 12; ++i) g.drawVerticalLine ((int) (b.getX() + b.getWidth() * i / 12.0f), b.getY(), spec.getY());

    float fLow = 110.0f;
    if (viz.numNotes > 0) { fLow = 1e9f; for (int i = 0; i < viz.numNotes; ++i) fLow = juce::jmin (fLow, rs::mtof (viz.notes[i].midi)); }
    const float T = juce::jlimit (0.003f, 0.05f, 2.4f / fLow);

    drawLanes (g, lanes, T);
    g.setColour (ui::cyan.withAlpha (0.25f)); g.drawHorizontalLine ((int) out.getY(), out.getX(), out.getRight());
    drawOutput (g, out, T);
    drawSpectrum (g, spec);
    drawReadout (g, readout.reduced (14.0f));
}

void Visualizer::drawLanes (juce::Graphics& g, juce::Rectangle<float> r, float T)
{
    std::vector<int> order;
    for (int i = 0; i < viz.numNotes; ++i) order.push_back (i);
    std::sort (order.begin(), order.end(), [this] (int a, int b) { return viz.notes[a].midi < viz.notes[b].midi; });
    g.setFont (juce::FontOptions (11.0f));
    if (order.empty())
    {
        g.setColour (ui::muted);
        g.drawText (u8 ("Сыграйте аккорд в пиано-ролле: здесь появится каждый осциллятор"), r.reduced (6.0f), juce::Justification::topLeft);
        return;
    }
    const float lh = r.getHeight() / (float) order.size();
    const float x0 = r.getX() + 36.0f, wPlot = r.getRight() - x0;
    for (size_t li = 0; li < order.size(); ++li)
    {
        const auto& n = viz.notes[order[li]];
        const float cy = r.getBottom() - lh * ((float) li + 0.5f);
        const auto col = ui::noteColour ((int) li);
        g.setColour (col); g.drawText (juce::String (rs::noteName ((int) std::lround (n.midi))), (int) r.getX() + 2, (int) (cy - lh * 0.45f), 34, 14, juce::Justification::left);
        const float amp = juce::jlimit (0.0f, 1.0f, n.env);
        for (int k = 0; k < n.numOsc; ++k)
        {
            const auto& o = n.osc[k];
            const float hz = rs::mtof (n.midi + o.semis);
            const float ph0 = phases[(size_t) (n.slot * 48 + k)];
            const float h = lh * 0.42f * amp * juce::jmin (1.0f, o.gain * 2.5f + 0.25f);
            juce::Path path;
            const int steps = 140;
            for (int s = 0; s <= steps; ++s)
            {
                const float t = (float) s / steps;
                const float y = cy - waveAt (viz, o.layer, ph0 + hz * t * T) * h;
                const float x = x0 + t * wPlot;
                if (s == 0) path.startNewSubPath (x, y); else path.lineTo (x, y);
            }
            g.setColour ((o.layer == 2 ? ui::cyan : col).withAlpha (o.layer == 0 ? 0.42f : 0.22f));
            g.strokePath (path, juce::PathStrokeType (1.0f));
        }
    }
}

void Visualizer::drawOutput (juce::Graphics& g, juce::Rectangle<float> r, float T)
{
    if (scopeCopy.empty()) return;
    const float sr = proc.engine.getSampleRate();
    const int S = (int) scopeCopy.size();
    const int len = juce::jlimit (256, 8000, (int) (T * sr));
    int start = S - len - 2048, trig = start;
    for (int i = start + 1; i < start + 2048; ++i)
        if (scopeCopy[(size_t) (i - 1)] < 0.0f && scopeCopy[(size_t) i] >= 0.0f) { trig = i; break; }
    const float cy = r.getCentreY(), hh = r.getHeight() * 0.45f, x0 = r.getX() + 36.0f, wPlot = r.getRight() - x0;
    juce::Path path;
    for (int i = 0; i < len; ++i)
    {
        const float v = juce::jlimit (-1.2f, 1.2f, scopeCopy[(size_t) (trig + i)]);
        const float x = x0 + (float) i / (float) (len - 1) * wPlot, y = cy - v * hh;
        if (i == 0) path.startNewSubPath (x, y); else path.lineTo (x, y);
    }
    g.setColour (ui::amber.withAlpha (0.25f)); g.strokePath (path, juce::PathStrokeType (5.0f));
    g.setColour (ui::amber); g.strokePath (path, juce::PathStrokeType (1.6f));
    g.setFont (juce::FontOptions (11.0f)); g.drawText (u8 ("выход"), (int) r.getX() + 2, (int) r.getY() + 4, 40, 14, juce::Justification::left);
}

void Visualizer::drawSpectrum (juce::Graphics& g, juce::Rectangle<float> r)
{
    const float sr = proc.engine.getSampleRate();
    auto xOf = [&] (float f) { return r.getX() + std::log (f / 20.0f) / std::log (1000.0f) * r.getWidth(); };
    g.setFont (juce::FontOptions (10.0f));
    const float marks[] = { 50, 100, 200, 500, 1000, 2000, 5000, 10000 };
    for (float f : marks)
    {
        const float x = xOf (f);
        g.setColour (juce::Colour (0x17788cc8)); g.drawVerticalLine ((int) x, r.getY(), r.getBottom());
        g.setColour (ui::muted.withAlpha (0.7f));
        g.drawText (f >= 1000 ? juce::String ((int) (f / 1000)) + "k" : juce::String ((int) f), (int) x + 2, (int) r.getBottom() - 13, 30, 12, juce::Justification::left);
    }
    // полосы голосов
    for (int i = 0; i < viz.numNotes; ++i)
    {
        const auto& n = viz.notes[i];
        const auto col = ui::noteColour (i);
        float lo = 1e9f, hi = 0;
        for (int k = 0; k < n.numOsc; ++k) if (n.osc[k].layer == 0) { const float hz = rs::mtof (n.midi + n.osc[k].semis); lo = juce::jmin (lo, hz); hi = juce::jmax (hi, hz); }
        if (hi <= 0) continue;
        g.setColour (col.withAlpha (0.16f * n.env)); g.fillRect (juce::Rectangle<float> (xOf (lo), r.getY(), juce::jmax (1.0f, xOf (hi) - xOf (lo)), r.getHeight()));
        g.setColour (col.withAlpha (juce::jlimit (0.0f, 1.0f, n.env)));
        g.drawText (juce::String (rs::noteName ((int) std::lround (n.midi))), (int) xOf (rs::mtof (n.midi)) + 2, (int) r.getY() + 1, 40, 12, juce::Justification::left);
    }
    // спектр
    juce::Path p; bool started = false;
    const float bw = sr / (float) fftSize;
    for (int i = 1; i < fftSize / 2; ++i)
    {
        const float f = (float) i * bw;
        if (f < 20.0f) continue;
        if (f > 20000.0f) break;
        const float y = r.getBottom() - juce::jlimit (0.0f, 1.0f, (spectrumDb[(size_t) i] + 100.0f) / 88.0f) * r.getHeight() * 0.9f;
        if (! started) { p.startNewSubPath (xOf (f), r.getBottom()); started = true; }
        p.lineTo (xOf (f), y);
    }
    if (started)
    {
        juce::Path fill (p); fill.lineTo (r.getRight(), r.getBottom()); fill.closeSubPath();
        g.setGradientFill (juce::ColourGradient (ui::cyan.withAlpha (0.5f), 0, r.getY(), ui::cyan.withAlpha (0.0f), 0, r.getBottom(), false));
        g.fillPath (fill);
        g.setColour (ui::cyan); g.strokePath (p, juce::PathStrokeType (1.2f));
    }
    // срез фильтра
    if (viz.numNotes > 0)
    {
        const float x = xOf (juce::jlimit (20.0f, 20000.0f, viz.notes[0].cutoff));
        g.setColour (ui::rose);
        const float dashes[] = { 4.0f, 4.0f };
        g.drawDashedLine (juce::Line<float> (x, r.getY(), x, r.getBottom()), dashes, 2, 1.0f);
        g.drawText (u8 ("срез"), (int) x + 3, (int) (r.getY() + r.getHeight() * 0.35f), 40, 12, juce::Justification::left);
    }
}

void Visualizer::drawReadout (juce::Graphics& g, juce::Rectangle<float> r)
{
    g.setColour (ui::amber); g.setFont (juce::FontOptions (40.0f, juce::Font::bold));
    g.drawFittedText (viz.chordName.empty() ? juce::String ("-") : juce::String (viz.chordName), r.removeFromTop (48).toNearestInt(), juce::Justification::left, 1);
    g.setColour (juce::Colour (0xff9fb0d6)); g.setFont (juce::FontOptions (13.0f));
    g.drawFittedText (viz.chordNotes.empty() ? u8 ("Ноты появятся здесь") : juce::String (viz.chordNotes), r.removeFromTop (36).toNearestInt(), juce::Justification::topLeft, 2);
    auto line = [&] (const juce::String& k, const juce::String& v)
    {
        auto row = r.removeFromTop (26);
        g.setColour (ui::edge); g.drawHorizontalLine ((int) row.getY(), row.getX(), row.getRight());
        g.setColour (ui::muted); g.drawText (k, row, juce::Justification::left);
        g.setColour (ui::cyan); g.drawText (v, row, juce::Justification::right);
    };
    auto& p = proc.engine.p;
    static const char* gm[] = { "Как в треке", "Голосоведение", "Параллельно", "Выключен" };
    line (u8 ("Осцилляторов"), juce::String (viz.oscCount));
    line (u8 ("Голосов на ноту"), juce::String (p.unison));
    line (u8 ("Детюн"), u8 ("±") + juce::String (juce::roundToInt (p.detune)) + u8 (" ц"));
    line (u8 ("Глайд"), u8 (gm[juce::jlimit (0, 3, p.glideMode)]) + "  " + juce::String (p.glide, 1) + u8 (" с"));
    g.setColour (ui::muted); g.setFont (juce::FontOptions (11.5f));
    g.drawFittedText (u8 ("Сверху: дорожка на каждую ноту и все её осцилляторы. Ниже: реальный выход и спектр."), r.removeFromBottom (44).toNearestInt(), juce::Justification::bottomLeft, 3);
}

// ===================== окно плагина =====================
RawstackEditor::RawstackEditor (RawstackProcessor& p)
    : AudioProcessorEditor (&p), proc (p), visualizer (p)
{
    setLookAndFeel (&lnf);
    addAndMakeVisible (visualizer);

    const auto& ps = factoryPresets();
    juce::String lastGroup;
    for (int i = 0; i < (int) ps.size(); ++i)
    {
        const juce::String name = u8 (ps[(size_t) i].name);
        const juce::String grp = name.startsWith ("just RAW") ? u8 ("Из трека just RAW") : u8 ("Другие");
        if (grp != lastGroup) { presetBox.addSectionHeading (grp); lastGroup = grp; }
        presetBox.addItem (name, i + 1);
    }
    presetBox.onChange = [this] { const int id = presetBox.getSelectedId(); if (id > 0 && id - 1 != proc.getCurrentProgram()) proc.setCurrentProgram (id - 1); };
    addAndMakeVisible (presetBox);
    prevBtn.onClick = [this] { const int n = proc.getNumPrograms(); proc.setCurrentProgram ((proc.getCurrentProgram() + n - 1) % n); };
    nextBtn.onClick = [this] { const int n = proc.getNumPrograms(); proc.setCurrentProgram ((proc.getCurrentProgram() + 1) % n); };
    addAndMakeVisible (prevBtn); addAndMakeVisible (nextBtn);

    struct G { const char* id; const char* title; const char* sub; int wu; };
    static const G gs[] = {
        { "osc", "Осцилляторы", "источник", 7 }, { "mod", "Движение", "дрейф, вибрато, нырок", 3 }, { "glide", "Глайд", "частоты съезжаются", 4 },
        { "chord", "Аккорды", "из одной ноты", 4 }, { "filt", "Фильтр", "тембр", 5 }, { "fx", "Драйв и эффекты", "грязь и воздух", 6 },
        { "env", "Огибающая", "громкость", 4 }, { "out", "Выход", "стерео и срезы", 4 } };
    for (auto& gdef : gs)
    {
        auto grp = std::make_unique<Group> (u8 (gdef.title), u8 (gdef.sub), gdef.wu);
        for (const auto& d : paramDefs())
            if (juce::String (d.group) == gdef.id)
                grp->addCell (std::make_unique<Cell> (proc, d));
        content.addAndMakeVisible (*grp);
        groups.push_back (std::move (grp));
    }
    viewport.setViewedComponent (&content, false);
    viewport.setScrollBarsShown (true, false);
    addAndMakeVisible (viewport);

    setResizable (true, true);
    setResizeLimits (900, 620, 1800, 1400);
    setSize (1190, 900);
    startTimerHz (5);
}

RawstackEditor::~RawstackEditor() { setLookAndFeel (nullptr); }

void RawstackEditor::timerCallback()
{
    const int cp = proc.getCurrentProgram();
    if (cp != shownProgram) { shownProgram = cp; presetBox.setSelectedId (cp + 1, juce::dontSendNotification); }
}

void RawstackEditor::paint (juce::Graphics& g)
{
    g.fillAll (ui::bg);
    g.setColour (ui::text); g.setFont (juce::FontOptions (26.0f, juce::Font::bold));
    g.drawText ("Rawstack", 14, 8, 160, 30, juce::Justification::left);
    g.setColour (ui::muted); g.setFont (juce::FontOptions (12.5f));
    g.drawText (u8 ("стек осцилляторов, аккорды и глайды"), 150, 14, 300, 20, juce::Justification::left);
}

void RawstackEditor::layoutContent (int width)
{
    int x = 0, y = 0, rowH = 0;
    const int gap = 8;
    for (auto& g : groups)
    {
        const int w = g->preferredWidth(), h = g->preferredHeight();
        if (x > 0 && x + w > width) { x = 0; y += rowH + gap; rowH = 0; }
        g->setBounds (x, y, w, h);
        x += w + gap; rowH = juce::jmax (rowH, h);
    }
    content.setSize (width, y + rowH + gap);
}

void RawstackEditor::resized()
{
    auto b = getLocalBounds().reduced (10);
    auto top = b.removeFromTop (34);
    nextBtn.setBounds (top.removeFromRight (34));
    top.removeFromRight (4);
    presetBox.setBounds (top.removeFromRight (300));
    top.removeFromRight (4);
    prevBtn.setBounds (top.removeFromRight (34));
    b.removeFromTop (8);
    visualizer.setBounds (b.removeFromTop (juce::jmax (240, (int) (b.getHeight() * 0.38f))));
    b.removeFromTop (10);
    viewport.setBounds (b);
    layoutContent (b.getWidth() - viewport.getScrollBarThickness() - 2);
}
