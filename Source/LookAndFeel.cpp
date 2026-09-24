#include "LookAndFeel.h"

//==============================================================================
AuroraLookAndFeel::AuroraLookAndFeel()
{
    setColour (juce::ResizableWindow::backgroundColourId, Palette::bgTop);
    setColour (juce::Slider::rotarySliderFillColourId,    Palette::accent);
    setColour (juce::Slider::textBoxTextColourId,         Palette::text);
    setColour (juce::Slider::textBoxOutlineColourId,      juce::Colours::transparentBlack);
    setColour (juce::Slider::textBoxBackgroundColourId,   juce::Colours::transparentBlack);
    setColour (juce::Label::textColourId,                 Palette::text);
    setColour (juce::ComboBox::backgroundColourId,        Palette::track);
    setColour (juce::ComboBox::textColourId,              Palette::text);
    setColour (juce::ComboBox::outlineColourId,           Palette::panelEdge);
    setColour (juce::ComboBox::arrowColourId,             Palette::accent);
    setColour (juce::PopupMenu::backgroundColourId,       Palette::panel);
    setColour (juce::PopupMenu::textColourId,             Palette::text);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, Palette::accent.withAlpha (0.25f));
    setColour (juce::PopupMenu::highlightedTextColourId,  juce::Colours::white);
    setColour (juce::TooltipWindow::backgroundColourId,   Palette::panel);
    setColour (juce::TooltipWindow::textColourId,         Palette::text);
    setColour (juce::MidiKeyboardComponent::whiteNoteColourId,      juce::Colour (0xffd7dde9));
    setColour (juce::MidiKeyboardComponent::blackNoteColourId,      juce::Colour (0xff171b26));
    setColour (juce::MidiKeyboardComponent::keySeparatorLineColourId, juce::Colour (0xff0b0d13));
    setColour (juce::MidiKeyboardComponent::keyDownOverlayColourId, Palette::accent.withAlpha (0.75f));
    setColour (juce::MidiKeyboardComponent::mouseOverKeyOverlayColourId, Palette::accent.withAlpha (0.30f));
    setColour (juce::MidiKeyboardComponent::shadowColourId,         juce::Colours::transparentBlack);
}

juce::Colour AuroraLookAndFeel::accentFor (const juce::Slider& s)
{
    return s.findColour (juce::Slider::rotarySliderFillColourId);
}

//==============================================================================
void AuroraLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                          float sliderPos, float rotaryStartAngle,
                                          float rotaryEndAngle, juce::Slider& slider)
{
    const auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat().reduced (2.0f);
    const float radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const auto centre  = bounds.getCentre();
    const float toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    const float lineW   = juce::jmax (2.5f, radius * 0.16f);
    const float arcR    = radius - lineW * 0.5f;

    const auto colour = accentFor (slider);

    // 背景の溝
    {
        juce::Path track;
        track.addCentredArc (centre.x, centre.y, arcR, arcR, 0.0f,
                             rotaryStartAngle, rotaryEndAngle, true);
        g.setColour (Palette::track);
        g.strokePath (track, juce::PathStrokeType (lineW, juce::PathStrokeType::curved,
                                                   juce::PathStrokeType::rounded));
    }

    // 値のアーク（バイポーラなら中央から伸ばす）
    const bool bipolar = slider.getMinimum() < -0.0001 && slider.getMaximum() > 0.0001
                         && std::abs (slider.getMinimum() + slider.getMaximum()) < 0.0001;
    const float startFrom = bipolar ? (rotaryStartAngle + rotaryEndAngle) * 0.5f : rotaryStartAngle;

    if (std::abs (toAngle - startFrom) > 0.001f)
    {
        juce::Path value;
        value.addCentredArc (centre.x, centre.y, arcR, arcR, 0.0f,
                             juce::jmin (startFrom, toAngle),
                             juce::jmax (startFrom, toAngle), true);

        g.setColour (colour.withAlpha (0.22f));
        g.strokePath (value, juce::PathStrokeType (lineW * 2.1f, juce::PathStrokeType::curved,
                                                   juce::PathStrokeType::rounded));
        g.setColour (colour);
        g.strokePath (value, juce::PathStrokeType (lineW, juce::PathStrokeType::curved,
                                                   juce::PathStrokeType::rounded));
    }

    // 中央のノブ
    const float knobR = arcR - lineW * 1.15f;

    if (knobR > 2.0f)
    {
        juce::ColourGradient grad (juce::Colour (0xff2b3348), centre.x, centre.y - knobR,
                                   juce::Colour (0xff161b28), centre.x, centre.y + knobR, false);
        g.setGradientFill (grad);
        g.fillEllipse (juce::Rectangle<float> (knobR * 2.0f, knobR * 2.0f).withCentre (centre));

        g.setColour (juce::Colour (0xff39435c));
        g.drawEllipse (juce::Rectangle<float> (knobR * 2.0f, knobR * 2.0f).withCentre (centre), 1.0f);

        // 指針
        juce::Path pointer;
        const float pw = juce::jmax (2.0f, knobR * 0.16f);
        pointer.addRoundedRectangle (-pw * 0.5f, -knobR * 0.92f, pw, knobR * 0.55f, pw * 0.5f);
        pointer.applyTransform (juce::AffineTransform::rotation (toAngle).translated (centre));
        g.setColour (colour.brighter (0.25f));
        g.fillPath (pointer);
    }
}

//==============================================================================
void AuroraLookAndFeel::drawComboBox (juce::Graphics& g, int width, int height, bool,
                                      int, int, int, int, juce::ComboBox& box)
{
    const auto bounds = juce::Rectangle<float> (0.0f, 0.0f, (float) width, (float) height).reduced (0.5f);
    const float corner = 4.0f;

    g.setColour (Palette::track);
    g.fillRoundedRectangle (bounds, corner);

    g.setColour (box.hasKeyboardFocus (true) ? Palette::accent.withAlpha (0.8f) : Palette::panelEdge);
    g.drawRoundedRectangle (bounds, corner, 1.0f);

    juce::Path arrow;
    const float cx = (float) width - 12.0f;
    const float cy = (float) height * 0.5f;
    arrow.startNewSubPath (cx - 4.0f, cy - 2.0f);
    arrow.lineTo (cx, cy + 3.0f);
    arrow.lineTo (cx + 4.0f, cy - 2.0f);

    g.setColour (Palette::accent);
    g.strokePath (arrow, juce::PathStrokeType (1.6f, juce::PathStrokeType::curved,
                                               juce::PathStrokeType::rounded));
}

void AuroraLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& button,
                                              const juce::Colour&, bool isHighlighted, bool isDown)
{
    const auto bounds = button.getLocalBounds().toFloat().reduced (0.5f);
    const float corner = 4.0f;

    auto fill = Palette::track;

    if (isDown)            fill = Palette::accent.withAlpha (0.30f);
    else if (isHighlighted) fill = Palette::track.brighter (0.22f);

    g.setColour (fill);
    g.fillRoundedRectangle (bounds, corner);

    g.setColour (isHighlighted ? Palette::accent.withAlpha (0.7f) : Palette::panelEdge);
    g.drawRoundedRectangle (bounds, corner, 1.0f);
}

juce::Font AuroraLookAndFeel::getTextButtonFont (juce::TextButton&, int buttonHeight)
{
    return juce::Font (juce::FontOptions (juce::jmin (14.0f, (float) buttonHeight * 0.5f)));
}

void AuroraLookAndFeel::positionComboBoxText (juce::ComboBox& box, juce::Label& label)
{
    label.setBounds (8, 0, box.getWidth() - 24, box.getHeight());
    label.setFont (getComboBoxFont (box));
    label.setJustificationType (juce::Justification::centredLeft);
}

juce::Font AuroraLookAndFeel::getComboBoxFont (juce::ComboBox&)
{
    return juce::Font (juce::FontOptions (12.5f));
}

juce::Font AuroraLookAndFeel::getPopupMenuFont()
{
    return juce::Font (juce::FontOptions (13.0f));
}

juce::Font AuroraLookAndFeel::getLabelFont (juce::Label& label)
{
    return label.getFont();
}
