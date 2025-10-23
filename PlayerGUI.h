#pragma once
#include <JuceHeader.h>
#include "PlayerAudio.h"

class PlayerGUI : public juce::Component,
  public juce::Button::Listener,
  public juce::Slider::Listener,
  private juce::Timer
{
public:
  PlayerGUI();
  ~PlayerGUI() override;

  void setAudio(PlayerAudio* audioPtr) noexcept { audio = audioPtr; }

  void paint(juce::Graphics& g) override;
  void resized() override;

  // keep same public handler names so MainComponent can call them (we'll forward)
  void buttonClicked(juce::Button* button) override;
  void sliderValueChanged(juce::Slider* slider) override;
  void timerCallback() override;

  // copy of makeIcon - same signature as original
  std::unique_ptr<juce::DrawablePath> makeIcon(juce::Path& path, juce::Colour colour)
  {
    auto drawablepath = std::make_unique<juce::DrawablePath>();
    drawablepath->setPath(path);
    drawablepath->setFill(colour);
    return drawablepath;
  }

  juce::String formatTime(double seconds);

  // expose some internals if needed (same names as original)
  juce::TextButton loadButton{ "Load Files" };
  juce::TextButton restartButton{ "Restart" };
  juce::TextButton stopButton{ "Stop" };
  juce::TextButton muteButton{ "Mute" };
  juce::Slider volumeSlider;
  juce::ToggleButton repeatButton{ "Repeat" };
  juce::DrawableButton ppButton{ "Play&Pause", juce::DrawableButton::ImageFitted };
  juce::DrawableButton toEndButton{ "toEnd", juce::DrawableButton::ImageFitted };
  juce::DrawableButton toStartButton{ "toStart", juce::DrawableButton::ImageFitted };
  juce::DrawableButton fw10Button{ "Add 10s", juce::DrawableButton::ImageFitted };
  juce::DrawableButton bw10Button{ "Sub 10s", juce::DrawableButton::ImageFitted };

  std::unique_ptr<juce::DrawablePath> playIcon; // Icon for play
  std::unique_ptr<juce::DrawablePath> pauseButtonIcon; // Icon for pause
  std::unique_ptr<juce::DrawablePath> toEndIcon; // Icon for toEnd
  std::unique_ptr<juce::DrawablePath> toStartIcon; // Icon for toStart
  std::unique_ptr<juce::DrawablePath> fw10Icon; // Icon for fw10
  std::unique_ptr<juce::DrawablePath> bw10Icon; // Icon for bw10

  juce::Slider progressSlider;
  juce::Label currentTimeLabel;
  juce::Label totalTimeLabel;
  juce::ToggleButton repeatToggle{ "Repeat" }; // local duplicate name avoided; use repeatButton in original code was ToggleButton named repeatButton
  juce::ToggleButton repeatButton2{ "Repeat" }; // (not used) kept to avoid renaming confusion

private:
  PlayerAudio* audio = nullptr;
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PlayerGUI)
};
