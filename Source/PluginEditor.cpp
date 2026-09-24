#include "PluginEditor.h"

//==============================================================================
KnobControl::KnobControl (const juce::String& labelText, juce::Colour accent)
    : name (labelText)
{
    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    slider.setRotaryParameters (juce::MathConstants<float>::pi * 1.25f,
                                juce::MathConstants<float>::pi * 2.75f, true);
    slider.setColour (juce::Slider::rotarySliderFillColourId, accent);
    addAndMakeVisible (slider);

    nameLabel.setJustificationType (juce::Justification::centred);
    nameLabel.setFont (juce::Font (juce::FontOptions (10.5f)).withExtraKerningFactor (0.06f));
    nameLabel.setColour (juce::Label::textColourId, Palette::textDim);
    nameLabel.setInterceptsMouseClicks (false, false);
    addAndMakeVisible (nameLabel);

    refreshText (false);

    slider.addMouseListener (this, false);
    slider.onValueChange = [this] { if (showingValue) refreshText (true); };
    slider.onDragStart   = [this] { showingValue = true;  refreshText (true); };
    slider.onDragEnd     = [this] { showingValue = isMouseOver (true); refreshText (showingValue); };
}

void KnobControl::refreshText (bool showValue)
{
    if (showValue)
    {
        nameLabel.setText (slider.getTextFromValue (slider.getValue()), juce::dontSendNotification);
        nameLabel.setColour (juce::Label::textColourId,
                             slider.findColour (juce::Slider::rotarySliderFillColourId));
    }
    else
    {
        nameLabel.setText (name, juce::dontSendNotification);
        nameLabel.setColour (juce::Label::textColourId, Palette::textDim);
    }
}

void KnobControl::mouseEnter (const juce::MouseEvent&) { showingValue = true;  refreshText (true); }
void KnobControl::mouseExit  (const juce::MouseEvent&)
{
    if (! slider.isMouseButtonDown())
    {
        showingValue = false;
        refreshText (false);
    }
}

void KnobControl::resized()
{
    auto b = getLocalBounds();
    nameLabel.setBounds (b.removeFromBottom (14));

    const int d = juce::jmax (12, juce::jmin (b.getWidth() - 6, b.getHeight() - 2));
    slider.setBounds (juce::Rectangle<int> (d, d).withCentre (b.getCentre()));
}

//==============================================================================
ChoiceControl::ChoiceControl (const juce::String& labelText)
{
    addAndMakeVisible (box);

    nameLabel.setText (labelText, juce::dontSendNotification);
    nameLabel.setJustificationType (juce::Justification::centred);
    nameLabel.setFont (juce::Font (juce::FontOptions (10.5f)).withExtraKerningFactor (0.06f));
    nameLabel.setColour (juce::Label::textColourId, Palette::textDim);
    nameLabel.setInterceptsMouseClicks (false, false);
    addAndMakeVisible (nameLabel);
}

void ChoiceControl::resized()
{
    auto b = getLocalBounds();
    nameLabel.setBounds (b.removeFromBottom (14));
    box.setBounds (b.withSizeKeepingCentre (juce::jmin (b.getWidth() - 8, 128), 24));
}

//==============================================================================
ModSlotControl::ModSlotControl (int slotNumber) : number (slotNumber)
{
    for (auto* b : { &srcBox, &dstBox })
    {
        b->setColour (juce::ComboBox::backgroundColourId, juce::Colour (0xff10141f));
        addAndMakeVisible (b);
    }

    amountSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    amountSlider.setTextBoxStyle (juce::Slider::TextBoxRight, false, 40, 18);
    amountSlider.setColour (juce::Slider::trackColourId, Palette::accent3);
    amountSlider.setColour (juce::Slider::textBoxTextColourId, Palette::textDim);
    amountSlider.setNumDecimalPlacesToDisplay (2);
    amountSlider.setDoubleClickReturnValue (true, 0.0);
    addAndMakeVisible (amountSlider);
}

bool ModSlotControl::isActive() const
{
    return srcBox.getSelectedId() > 1 && dstBox.getSelectedId() > 1
           && std::abs (amountSlider.getValue()) > 0.001;
}

void ModSlotControl::refreshActive()
{
    const bool on = isActive();
    amountSlider.setColour (juce::Slider::trackColourId,
                            on ? Palette::accent3 : Palette::panelEdge);
    amountSlider.setColour (juce::Slider::textBoxTextColourId,
                            on ? Palette::text : Palette::textDim.withAlpha (0.55f));
    repaint();
}

void ModSlotControl::paint (juce::Graphics& g)
{
    const bool active = isActive();

    g.setColour (active ? Palette::accent3 : Palette::textDim.withAlpha (0.5f));
    g.setFont (juce::Font (juce::FontOptions (10.0f, juce::Font::bold)));
    g.drawText (juce::String (number), 2, 2, 13, 20, juce::Justification::centred, false);

    // 元 → 先 をつなぐ矢印
    const float y = 12.0f;
    const float x = (float) srcBox.getRight() + 2.0f;
    juce::Path arrow;
    arrow.startNewSubPath (x, y);
    arrow.lineTo (x + 6.0f, y);
    arrow.startNewSubPath (x + 3.0f, y - 3.0f);
    arrow.lineTo (x + 6.0f, y);
    arrow.lineTo (x + 3.0f, y + 3.0f);

    g.setColour (active ? Palette::accent3.withAlpha (0.85f) : Palette::panelEdge);
    g.strokePath (arrow, juce::PathStrokeType (1.2f));
}

void ModSlotControl::resized()
{
    auto b = getLocalBounds().reduced (5, 1);

    auto top = b.removeFromTop (22);
    top.removeFromLeft (14);
    const int srcW = juce::roundToInt ((float) (top.getWidth() - 11) * 0.44f);
    srcBox.setBounds (top.removeFromLeft (srcW));
    top.removeFromLeft (11);
    dstBox.setBounds (top);

    b.removeFromTop (3);
    amountSlider.setBounds (b.removeFromTop (juce::jmin (20, b.getHeight())).withTrimmedLeft (14));
}

//==============================================================================
Panel::Panel (const juce::String& titleText, juce::Colour accent, int numColumns)
    : title (titleText), accentColour (accent), columns (juce::jmax (1, numColumns))
{
}

void Panel::paint (juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat().reduced (0.5f);

    juce::ColourGradient grad (Palette::panel.brighter (0.05f), b.getX(), b.getY(),
                               Palette::panel.darker (0.25f), b.getX(), b.getBottom(), false);
    g.setGradientFill (grad);
    g.fillRoundedRectangle (b, 7.0f);

    g.setColour (Palette::panelEdge);
    g.drawRoundedRectangle (b, 7.0f, 1.0f);

    // タイトル
    g.setColour (accentColour);
    g.setFont (juce::Font (juce::FontOptions (10.5f, juce::Font::bold)).withExtraKerningFactor (0.16f));
    g.drawText (title, (int) b.getX() + 11, (int) b.getY() + 6, getWidth() - 20, 14,
                juce::Justification::centredLeft, false);

    g.setColour (accentColour.withAlpha (0.30f));
    g.fillRect (juce::Rectangle<float> (b.getX() + 11.0f, b.getY() + 21.0f, b.getWidth() - 22.0f, 1.0f));
}

void Panel::resized()
{
    auto r = getLocalBounds().reduced (8);
    r.removeFromTop (20);

    const int n = items.size();

    if (n == 0)
        return;

    const int rows = (n + columns - 1) / columns;
    const float cw = (float) r.getWidth()  / (float) columns;
    const float ch = (float) r.getHeight() / (float) rows;

    for (int i = 0; i < n; ++i)
    {
        const int col = i % columns;
        const int row = i / columns;

        items.getUnchecked (i)->setBounds (
            juce::Rectangle<float> (r.getX() + (float) col * cw,
                                    r.getY() + (float) row * ch, cw, ch).toNearestInt());
    }
}

//==============================================================================
WavetableDisplay::WavetableDisplay (AuroraWTAudioProcessor& p) : proc (p)
{
    scratch.resize (256);

    lastPosA   = proc.apvts.getRawParameterValue (ParamID::oscAPos)->load();
    lastPosB   = proc.apvts.getRawParameterValue (ParamID::oscBPos)->load();
    lastLevelB = proc.apvts.getRawParameterValue (ParamID::oscBLevel)->load();

    startTimerHz (30);
}

void WavetableDisplay::timerCallback()
{
    const float a  = proc.apvts.getRawParameterValue (ParamID::oscAPos)->load();
    const float b  = proc.apvts.getRawParameterValue (ParamID::oscBPos)->load();
    const float lb = proc.apvts.getRawParameterValue (ParamID::oscBLevel)->load();

    if (! juce::approximatelyEqual (a, lastPosA)
        || ! juce::approximatelyEqual (b, lastPosB)
        || ! juce::approximatelyEqual (lb, lastLevelB))
    {
        lastPosA = a;
        lastPosB = b;
        lastLevelB = lb;
        repaint();
    }
}

void WavetableDisplay::buildPath (juce::Path& path, float framePos,
                                  juce::Rectangle<float> area, float ampScale) const
{
    const int n = (int) scratch.size();
    auto* data = const_cast<std::vector<float>*> (&scratch)->data();
    proc.wavetableBank.fillDisplayWaveform (framePos, data, n);

    for (int i = 0; i < n; ++i)
    {
        const float x = area.getX() + area.getWidth() * ((float) i / (float) (n - 1));
        const float y = area.getCentreY() - data[i] * area.getHeight() * ampScale;

        if (i == 0) path.startNewSubPath (x, y);
        else        path.lineTo (x, y);
    }
}

void WavetableDisplay::paint (juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat().reduced (0.5f);

    juce::ColourGradient grad (juce::Colour (0xff141a28), b.getCentreX(), b.getY(),
                               juce::Colour (0xff0a0d15), b.getCentreX(), b.getBottom(), false);
    g.setGradientFill (grad);
    g.fillRoundedRectangle (b, 7.0f);
    g.setColour (Palette::panelEdge);
    g.drawRoundedRectangle (b, 7.0f, 1.0f);

    g.setColour (Palette::accent);
    g.setFont (juce::Font (juce::FontOptions (10.5f, juce::Font::bold)).withExtraKerningFactor (0.16f));
    g.drawText ("WAVETABLE", (int) b.getX() + 11, (int) b.getY() + 6, getWidth() - 20, 14,
                juce::Justification::centredLeft, false);

    if (! proc.wavetableBank.isReady())
        return;

    auto area = b.reduced (16.0f, 12.0f).withTrimmedTop (20.0f).withTrimmedBottom (10.0f);
    auto stackArea = area.removeFromTop (area.getHeight() * 0.50f);
    auto mainArea  = area;

    // 奥行きのある「フレームの積み重ね」を薄く描く
    const int layers = WavetableBank::numFrames;
    const float depthX = stackArea.getWidth()  * 0.11f;
    const float depthY = stackArea.getHeight() * 0.55f;

    auto layerBase = stackArea.withTrimmedRight (depthX)
                              .withTrimmedTop (depthY)
                              .withHeight (stackArea.getHeight() * 0.45f);

    for (int i = layers - 1; i >= 0; --i)
    {
        const float t = (float) i / (float) (layers - 1);
        juce::Path p;
        buildPath (p, t, layerBase.translated (depthX * t, -depthY * t));

        const float closeness = 1.0f - juce::jlimit (0.0f, 1.0f, std::abs (t - lastPosA) * 3.0f);
        g.setColour (Palette::accent.interpolatedWith (Palette::accent2, t)
                         .withAlpha (0.10f + 0.22f * closeness));
        g.strokePath (p, juce::PathStrokeType (1.0f));
    }

    g.setColour (Palette::panelEdge.withAlpha (0.55f));
    g.fillRect (mainArea.getX(), mainArea.getCentreY(), mainArea.getWidth(), 1.0f);

    if (lastLevelB > 0.001f)
    {
        juce::Path pb;
        buildPath (pb, lastPosB, mainArea, 0.48f);
        g.setColour (Palette::accent2.withAlpha (0.85f));
        g.strokePath (pb, juce::PathStrokeType (1.8f, juce::PathStrokeType::curved));
    }

    juce::Path pa;
    buildPath (pa, lastPosA, mainArea, 0.48f);

    g.setColour (Palette::accent.withAlpha (0.25f));
    g.strokePath (pa, juce::PathStrokeType (5.0f, juce::PathStrokeType::curved));
    g.setColour (Palette::accent);
    g.strokePath (pa, juce::PathStrokeType (2.0f, juce::PathStrokeType::curved));

    g.setColour (Palette::textDim);
    g.setFont (juce::Font (juce::FontOptions (10.0f)));
    g.drawText (juce::String ("A ") + juce::String (lastPosA, 2)
                + (lastLevelB > 0.001f ? juce::String ("   B ") + juce::String (lastPosB, 2)
                                       : juce::String()),
                (int) b.getX(), (int) b.getBottom() - 18, getWidth() - 12, 14,
                juce::Justification::centredRight, false);
}

//==============================================================================
PresetBar::PresetBar (PresetManager& pm) : presets (pm)
{
    prevButton.setButtonText (juce::String::charToString ((juce_wchar) 0x25c0));   // ◀
    nextButton.setButtonText (juce::String::charToString ((juce_wchar) 0x25b6));   // ▶

    for (auto* b : { &prevButton, &nextButton, &nameButton })
    {
        b->setColour (juce::TextButton::textColourOffId, Palette::text);
        b->setColour (juce::TextButton::textColourOnId, Palette::text);
        addAndMakeVisible (b);
    }

    prevButton.onClick = [this] { presets.loadPrevious(); };
    nextButton.onClick = [this] { presets.loadNext(); };
    nameButton.onClick = [this] { showMenu(); };

    nameButton.setTooltip ("クリックでプリセット一覧・保存");

    presets.onPresetChanged = [this] { refresh(); };
    refresh();
}

void PresetBar::refresh()
{
    auto name = presets.getCurrentPresetName();
    nameButton.setButtonText (name.isEmpty() ? "Init" : name);
}

void PresetBar::resized()
{
    auto b = getLocalBounds();
    prevButton.setBounds (b.removeFromLeft (28).reduced (1));
    nextButton.setBounds (b.removeFromRight (28).reduced (1));
    nameButton.setBounds (b.reduced (3, 1));
}

void PresetBar::showMenu()
{
    juce::PopupMenu menu;
    menu.setLookAndFeel (&getLookAndFeel());

    // --- 工場出荷プリセットをカテゴリ別にまとめる ---
    const auto& factory = PresetManager::getFactoryPresets();
    const int currentFactory = presets.getCurrentFactoryIndex();

    juce::StringArray categories;

    for (const auto& p : factory)
        categories.addIfNotAlreadyThere (p.category);

    for (const auto& cat : categories)
    {
        juce::PopupMenu sub;
        bool any = false;

        for (int i = 0; i < (int) factory.size(); ++i)
        {
            if (juce::String (factory[(size_t) i].category) != cat)
                continue;

            sub.addItem (i + 1, factory[(size_t) i].name, true, i == currentFactory);
            any = true;
        }

        if (any)
            menu.addSubMenu (cat, sub);
    }

    // --- ユーザープリセット ---
    const auto users = presets.getUserPresetNames();
    const auto currentName = presets.getCurrentPresetName();
    const bool currentIsUser = currentFactory < 0 && users.contains (currentName);

    if (! users.isEmpty())
    {
        juce::PopupMenu sub;

        for (int i = 0; i < users.size(); ++i)
            sub.addItem (1000 + i, users[i], true, users[i] == currentName);

        menu.addSeparator();
        menu.addSubMenu ("User", sub);
    }

    menu.addSeparator();
    menu.addItem (9001, "現在の音を保存...");
    menu.addItem (9002, "このユーザープリセットを削除", currentIsUser);
    menu.addItem (9003, "プリセットフォルダを開く");

    menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (&nameButton)
                                                  .withMinimumWidth (nameButton.getWidth()),
        [this, users, currentName] (int result)
        {
            if (result == 0)
                return;

            if (result == 9001)          showSaveDialog();
            else if (result == 9002)   { presets.deleteUserPreset (currentName); presets.loadFactoryPreset (0); }
            else if (result == 9003)     presets.getUserPresetDirectory().revealToUser();
            else if (result >= 1000)     presets.loadUserPreset (users[result - 1000]);
            else                         presets.loadFactoryPreset (result - 1);
        });
}

void PresetBar::showSaveDialog()
{
    saveWindow = std::make_unique<juce::AlertWindow> ("プリセットを保存",
                                                      "名前を入力してください",
                                                      juce::MessageBoxIconType::NoIcon);
    saveWindow->setLookAndFeel (&getLookAndFeel());
    saveWindow->addTextEditor ("name", presets.getCurrentPresetName(), {});
    saveWindow->addButton ("保存",       1, juce::KeyPress (juce::KeyPress::returnKey));
    saveWindow->addButton ("キャンセル", 0, juce::KeyPress (juce::KeyPress::escapeKey));

    saveWindow->enterModalState (true, juce::ModalCallbackFunction::create (
        [this] (int result)
        {
            juce::String name;

            if (saveWindow != nullptr)
                name = saveWindow->getTextEditorContents ("name");

            if (result == 1 && name.isNotEmpty())
                presets.saveUserPreset (name);

            juce::MessageManager::callAsync ([this]
            {
                if (saveWindow != nullptr)
                    saveWindow->setLookAndFeel (nullptr);

                saveWindow.reset();
            });
        }), false);
}

//==============================================================================
LevelMeter::LevelMeter (AuroraWTAudioProcessor& p) : proc (p)
{
    startTimerHz (30);
}

void LevelMeter::timerCallback()
{
    const float v = juce::jlimit (0.0f, 1.0f, proc.outputLevel.load() * 2.5f);
    smoothed = juce::jmax (v, smoothed * 0.82f);
    proc.outputLevel.store (proc.outputLevel.load() * 0.6f);
    repaint();
}

void LevelMeter::paint (juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat();

    g.setColour (Palette::track);
    g.fillRoundedRectangle (b, 3.0f);

    if (smoothed > 0.001f)
    {
        auto filled = b.withWidth (b.getWidth() * smoothed);
        juce::ColourGradient grad (Palette::accent, b.getX(), 0.0f,
                                   Palette::accent3, b.getRight(), 0.0f, false);
        grad.addColour (0.65, Palette::accent2);
        g.setGradientFill (grad);
        g.fillRoundedRectangle (filled, 3.0f);
    }
}

//==============================================================================
void AuroraWTAudioProcessorEditor::Content::paint (juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat();

    juce::ColourGradient bg (Palette::bgTop, b.getCentreX(), 0.0f,
                             Palette::bgBottom, b.getCentreX(), b.getBottom(), false);
    g.setGradientFill (bg);
    g.fillAll();

    // ヘッダーのオーロラ風グロー
    juce::ColourGradient glow (Palette::accent.withAlpha (0.18f), 90.0f, 10.0f,
                               juce::Colours::transparentBlack, 520.0f, 70.0f, true);
    g.setGradientFill (glow);
    g.fillRect (0.0f, 0.0f, b.getWidth(), 66.0f);

    g.setColour (Palette::text);
    g.setFont (juce::Font (juce::FontOptions (23.0f, juce::Font::bold)).withExtraKerningFactor (0.22f));
    g.drawText ("AURORA", 18, 12, 200, 28, juce::Justification::centredLeft, false);

    const int auroraWidth = juce::GlyphArrangement::getStringWidthInt (
        juce::Font (juce::FontOptions (23.0f, juce::Font::bold)).withExtraKerningFactor (0.22f), "AURORA ");

    g.setColour (Palette::accent);
    g.drawText ("WT", 18 + auroraWidth, 12, 120, 28, juce::Justification::centredLeft, false);

    g.setColour (Palette::textDim);
    g.setFont (juce::Font (juce::FontOptions (10.5f)).withExtraKerningFactor (0.18f));
    g.drawText ("WAVETABLE SYNTHESIZER", 20, 38, 320, 14, juce::Justification::centredLeft, false);

    g.setColour (Palette::panelEdge.withAlpha (0.8f));
    g.fillRect (12, 62, getWidth() - 24, 1);
}

//==============================================================================
AuroraWTAudioProcessorEditor::AuroraWTAudioProcessorEditor (AuroraWTAudioProcessor& p)
    : AudioProcessorEditor (&p), proc (p), display (p), meter (p),
      presetBar (p.presetManager),
      keyboard (p.keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    setLookAndFeel (&lnf);

    addAndMakeVisible (content);

    for (auto* panel : { &oscAPanel, &oscBPanel, &filtPanel, &ampPanel, &fegPanel, &megPanel,
                         &mixPanel, &lfo1Panel, &lfo2Panel, &chorPanel, &dlyPanel, &revPanel,
                         &matrixPanel })
        content.addAndMakeVisible (panel);

    content.addAndMakeVisible (display);
    content.addAndMakeVisible (meter);
    content.addAndMakeVisible (presetBar);
    content.addAndMakeVisible (masterKnob);
    content.addAndMakeVisible (keyboard);

    // ---- Oscillator A / B ----
    addKnob (oscAPanel, ParamID::oscAPos,    "POS",    Palette::accent);
    addKnob (oscAPanel, ParamID::oscALevel,  "LEVEL",  Palette::accent);
    addKnob (oscAPanel, ParamID::oscAOct,    "OCT",    Palette::accent);
    addKnob (oscAPanel, ParamID::oscASemi,   "SEMI",   Palette::accent);
    addKnob (oscAPanel, ParamID::oscAFine,   "FINE",   Palette::accent);
    addKnob (oscAPanel, ParamID::oscAUni,    "UNISON", Palette::accent);
    addKnob (oscAPanel, ParamID::oscADetune, "DETUNE", Palette::accent);
    addKnob (oscAPanel, ParamID::oscASpread, "SPREAD", Palette::accent);

    addKnob (oscBPanel, ParamID::oscBPos,    "POS",    Palette::accent2);
    addKnob (oscBPanel, ParamID::oscBLevel,  "LEVEL",  Palette::accent2);
    addKnob (oscBPanel, ParamID::oscBOct,    "OCT",    Palette::accent2);
    addKnob (oscBPanel, ParamID::oscBSemi,   "SEMI",   Palette::accent2);
    addKnob (oscBPanel, ParamID::oscBFine,   "FINE",   Palette::accent2);
    addKnob (oscBPanel, ParamID::oscBUni,    "UNISON", Palette::accent2);
    addKnob (oscBPanel, ParamID::oscBDetune, "DETUNE", Palette::accent2);
    addKnob (oscBPanel, ParamID::oscBSpread, "SPREAD", Palette::accent2);

    // ---- Filter ----
    addChoice (filtPanel, ParamID::filtType, "TYPE");
    addKnob (filtPanel, ParamID::filtCutoff, "CUTOFF",  Palette::accent3);
    addKnob (filtPanel, ParamID::filtRes,    "RESO",    Palette::accent3);
    addKnob (filtPanel, ParamID::filtDrive,  "DRIVE",   Palette::accent3);
    addKnob (filtPanel, ParamID::filtEnvAmt, "ENV AMT", Palette::accent3);
    addKnob (filtPanel, ParamID::filtKeyTrk, "KEY TRK", Palette::accent3);

    // ---- Envelopes ----
    addKnob (ampPanel, ParamID::ampAttack,  "ATTACK",  Palette::accent);
    addKnob (ampPanel, ParamID::ampDecay,   "DECAY",   Palette::accent);
    addKnob (ampPanel, ParamID::ampSustain, "SUSTAIN", Palette::accent);
    addKnob (ampPanel, ParamID::ampRelease, "RELEASE", Palette::accent);

    addKnob (fegPanel, ParamID::fegAttack,  "ATTACK",  Palette::accent3);
    addKnob (fegPanel, ParamID::fegDecay,   "DECAY",   Palette::accent3);
    addKnob (fegPanel, ParamID::fegSustain, "SUSTAIN", Palette::accent3);
    addKnob (fegPanel, ParamID::fegRelease, "RELEASE", Palette::accent3);

    addKnob (megPanel, ParamID::megAttack,  "ATTACK",  Palette::accent2);
    addKnob (megPanel, ParamID::megDecay,   "DECAY",   Palette::accent2);
    addKnob (megPanel, ParamID::megSustain, "SUSTAIN", Palette::accent2);
    addKnob (megPanel, ParamID::megRelease, "RELEASE", Palette::accent2);

    // ---- Mix / Voice ----
    addKnob (mixPanel, ParamID::subLevel,   "SUB",   Palette::accent2);
    addKnob (mixPanel, ParamID::noiseLevel, "NOISE", Palette::accent2);
    addKnob (mixPanel, ParamID::glide,      "GLIDE", Palette::accent2);

    // ---- LFO 1 / 2 ----
    addKnob   (lfo1Panel, ParamID::lfo1Rate,  "RATE",  Palette::accent2);
    addChoice (lfo1Panel, ParamID::lfo1Shape, "SHAPE");
    addChoice (lfo1Panel, ParamID::lfo1Trig,  "TRIG");

    addKnob   (lfo2Panel, ParamID::lfo2Rate,  "RATE",  Palette::accent2);
    addChoice (lfo2Panel, ParamID::lfo2Shape, "SHAPE");
    addChoice (lfo2Panel, ParamID::lfo2Trig,  "TRIG");

    // ---- モジュレーションマトリクス ----
    for (int i = 0; i < numModSlots; ++i)
        addModSlot (matrixPanel, i);

    // ---- FX ----
    addKnob (chorPanel, ParamID::chorusRate,  "RATE",  Palette::accent);
    addKnob (chorPanel, ParamID::chorusDepth, "DEPTH", Palette::accent);
    addKnob (chorPanel, ParamID::chorusMix,   "MIX",   Palette::accent);

    addKnob (dlyPanel, ParamID::delayTime, "TIME",     Palette::accent);
    addKnob (dlyPanel, ParamID::delayFb,   "FEEDBACK", Palette::accent);
    addKnob (dlyPanel, ParamID::delayMix,  "MIX",      Palette::accent);

    addKnob (revPanel, ParamID::reverbSize, "SIZE",  Palette::accent);
    addKnob (revPanel, ParamID::reverbDamp, "DAMP",  Palette::accent);
    addKnob (revPanel, ParamID::reverbMix,  "MIX",   Palette::accent);

    // ---- Master ----
    sliderAttachments.add (new juce::AudioProcessorValueTreeState::SliderAttachment (
        proc.apvts, ParamID::masterGain, masterKnob.slider));

    keyboard.setAvailableRange (36, 96);
    keyboard.setOctaveForMiddleC (4);
    keyboard.setKeyWidth (28.0f);
    keyboard.setScrollButtonsVisible (false);

    setResizable (true, true);
    setResizeLimits (820, 563, 1700, 1167);

    if (auto* c = getConstrainer())
        c->setFixedAspectRatio ((double) designWidth / (double) designHeight);

    setSize (designWidth, designHeight);
}

AuroraWTAudioProcessorEditor::~AuroraWTAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

void AuroraWTAudioProcessorEditor::addKnob (Panel& panel, const juce::String& paramID,
                                            const juce::String& label, juce::Colour accent)
{
    auto* k = panel.add (new KnobControl (label, accent));
    sliderAttachments.add (new juce::AudioProcessorValueTreeState::SliderAttachment (
        proc.apvts, paramID, k->slider));
}

void AuroraWTAudioProcessorEditor::addChoice (Panel& panel, const juce::String& paramID,
                                              const juce::String& label)
{
    auto* c = panel.add (new ChoiceControl (label));

    // アタッチメントは項目を自動で埋めてくれないので、先にパラメータから流し込む
    if (auto* param = proc.apvts.getParameter (paramID))
        c->box.addItemList (param->getAllValueStrings(), 1);

    comboAttachments.add (new juce::AudioProcessorValueTreeState::ComboBoxAttachment (
        proc.apvts, paramID, c->box));
}

void AuroraWTAudioProcessorEditor::addModSlot (Panel& panel, int slot)
{
    auto* m = panel.add (new ModSlotControl (slot + 1));

    if (auto* sp = proc.apvts.getParameter (ParamID::modSrc (slot)))
        m->srcBox.addItemList (sp->getAllValueStrings(), 1);

    if (auto* dp = proc.apvts.getParameter (ParamID::modDst (slot)))
        m->dstBox.addItemList (dp->getAllValueStrings(), 1);

    comboAttachments.add (new juce::AudioProcessorValueTreeState::ComboBoxAttachment (
        proc.apvts, ParamID::modSrc (slot), m->srcBox));
    comboAttachments.add (new juce::AudioProcessorValueTreeState::ComboBoxAttachment (
        proc.apvts, ParamID::modDst (slot), m->dstBox));
    sliderAttachments.add (new juce::AudioProcessorValueTreeState::SliderAttachment (
        proc.apvts, ParamID::modAmt (slot), m->amountSlider));

    // 有効／無効の見た目を更新する
    m->srcBox.onChange            = [m] { m->refreshActive(); };
    m->dstBox.onChange            = [m] { m->refreshActive(); };
    m->amountSlider.onValueChange = [m] { m->refreshActive(); };
    m->refreshActive();
}

void AuroraWTAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (Palette::bgBottom);
}

void AuroraWTAudioProcessorEditor::resized()
{
    const float scale = juce::jmin ((float) getWidth()  / (float) designWidth,
                                    (float) getHeight() / (float) designHeight);

    content.setTransform (juce::AffineTransform::scale (scale));
    content.setBounds (0, 0, designWidth, designHeight);

    // ---- ヘッダー ----
    presetBar .setBounds (360, 16, 360, 30);
    meter     .setBounds (760, 27, 220, 9);
    masterKnob.setBounds (1080, 4, 74, 56);

    // ---- 1 段目 ----
    oscAPanel.setBounds (12,  70, 360, 180);
    oscBPanel.setBounds (384, 70, 360, 180);
    display  .setBounds (756, 70, 412, 180);

    // ---- 2 段目 ----
    filtPanel.setBounds (12,  262, 360, 180);
    ampPanel .setBounds (384, 262, 180, 180);
    fegPanel .setBounds (576, 262, 180, 180);
    megPanel .setBounds (768, 262, 180, 180);
    mixPanel .setBounds (960, 262, 208, 180);

    // ---- 3 段目 ----
    lfo1Panel.setBounds (12,  454, 290, 140);
    lfo2Panel.setBounds (314, 454, 290, 140);
    chorPanel.setBounds (616, 454, 176, 140);
    dlyPanel .setBounds (804, 454, 176, 140);
    revPanel .setBounds (992, 454, 176, 140);

    // ---- 4 段目：モジュレーションマトリクス ----
    matrixPanel.setBounds (12, 606, 1156, 136);

    // ---- 鍵盤 ----
    keyboard.setBounds (12, 754, 1156, 44);
}
