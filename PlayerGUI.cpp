#include "PlayerGUI.h"

PlayerGUI::PlayerGUI()
{
  for (auto* btn : { &loadButton , &restartButton , &stopButton , &muteButton })
  {
    btn->addListener(this);
    addAndMakeVisible(btn);
  }

  volumeSlider.setRange(0.0, 1.0, 0.01);
  volumeSlider.setValue(0.5);
  volumeSlider.addListener(this);
  addAndMakeVisible(volumeSlider);

  // Progress Slider 
  progressSlider.setRange(0.0, 1.0);
  progressSlider.setSliderStyle(juce::Slider::LinearHorizontal);
  progressSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
  progressSlider.addListener(this);
  addAndMakeVisible(progressSlider);

  addAndMakeVisible(currentTimeLabel);
  addAndMakeVisible(totalTimeLabel);
  currentTimeLabel.setText("0:00", juce::dontSendNotification);
  totalTimeLabel.setText("0:00", juce::dontSendNotification);
  startTimer(250);

  repeatButton.addListener(this);
  addAndMakeVisible(repeatButton);

  // pause icon
  juce::Path pausePath;
  pausePath.addRectangle(0.0f, 0.0f, 6.0f, 20.0f);
  pausePath.addRectangle(14.0f, 0.0f, 6.0f, 20.0f);
  pauseButtonIcon = makeIcon(pausePath, juce::Colours::white);

  // play icon
  juce::Path testPath;
  testPath.addTriangle(0.0f, 0.0f, 20.0f, 10.0f, 0.0f, 20.0f);
  playIcon = makeIcon(testPath, juce::Colours::white);

  // toEnd icon
  juce::Path toEndPath;
  toEndPath.addTriangle(0.0f, 0.0f, 10.0f, 10.0f, 0.0f, 20.0f);
  toEndPath.addRectangle(12.0f, 0.0f, 4.0f, 20.0f);
  toEndIcon = makeIcon(toEndPath, juce::Colours::white);

  // toStart icon
  juce::Path toStartPath;
  toStartPath.addRectangle(0.0f, 0.0f, 4.0f, 20.0f);
  toStartPath.addTriangle(16.0f, 0.0f, 6.0f, 10.0f, 16.0f, 20.0f);
  toStartIcon = makeIcon(toStartPath, juce::Colours::white);

  // fw10
  juce::Path fw10Path;
  fw10Path.addTriangle(0.0f, 0.0f, 10.0f, 10.0f, 0.0f, 20.0f);
  fw10Path.addTriangle(12.0f, 0.0f, 22.0f, 10.0f, 12.0f, 20.0f);
  fw10Icon = makeIcon(fw10Path, juce::Colours::white);

  // bw10
  juce::Path bw10Path;
  bw10Path.addTriangle(22.0f, 0.0f, 12.0f, 10.0f, 22.0f, 20.0f);
  bw10Path.addTriangle(10.0f, 0.0f, 0.0f, 10.0f, 10.0f, 20.0f);
  bw10Icon = makeIcon(bw10Path, juce::Colours::white);

  ppButton.setImages(pauseButtonIcon.get());
  toEndButton.setImages(toEndIcon.get());
  toStartButton.setImages(toStartIcon.get());
  fw10Button.setImages(fw10Icon.get());
  bw10Button.setImages(bw10Icon.get());

  for (auto* btn : { &ppButton , &toEndButton , &toStartButton , &fw10Button , &bw10Button })
  {
    btn->addListener(this);
    addAndMakeVisible(btn);
  }
}

PlayerGUI::~PlayerGUI()
{
  stopTimer();
  for (auto* btn : { &loadButton , &restartButton , &stopButton , &muteButton })
    btn->removeListener(this);

  for (auto* btn : { &ppButton , &toEndButton , &toStartButton , &fw10Button , &bw10Button })
    btn->removeListener(this);

  volumeSlider.removeListener(this);
  progressSlider.removeListener(this);
  repeatButton.removeListener(this);
}

void PlayerGUI::paint(juce::Graphics& g)
{
  g.fillAll(juce::Colour::fromRGB(30, 30, 30));

  juce::ColourGradient gradient(juce::Colours::darkgrey, 0, 0,
    juce::Colours::black, 0, (float)getHeight(), false);
  g.setGradientFill(gradient);
  g.fillAll();

  g.setColour(juce::Colours::white);
  g.setFont(24.0f);
  g.drawFittedText("Simple Audio Player", getLocalBounds().reduced(10), juce::Justification::centredTop, 1);
}

void PlayerGUI::resized()
{
  auto area = getLocalBounds().reduced(50);

  auto buttonArea = area.removeFromTop(50);
  int buttonWidth = 85;
  int spacing = 10;
  loadButton.setBounds(buttonArea.removeFromLeft(buttonWidth));
  buttonArea.removeFromLeft(spacing);
  restartButton.setBounds(buttonArea.removeFromLeft(buttonWidth));
  buttonArea.removeFromLeft(spacing);
  stopButton.setBounds(buttonArea.removeFromLeft(buttonWidth));
  buttonArea.removeFromLeft(spacing);
  muteButton.setBounds(buttonArea.removeFromLeft(buttonWidth));

  area.removeFromTop(20);
  volumeSlider.setBounds(area.removeFromTop(40));

  progressSlider.setBounds(20, 180, getWidth() - 40, 20);
  currentTimeLabel.setBounds(20, 210, 60, 20);
  totalTimeLabel.setBounds(getWidth() - 65, 210, 60, 20);
  repeatButton.setBounds(getWidth() - 160, 200, buttonWidth, 40);

  int smallButtonWidth = 70;
  int smallButtonHeight = 40;
  int spacingBetween = 20;

  int totalWidth = (smallButtonWidth * 3) + (spacingBetween * 2);
  int startX = (getWidth() - totalWidth) / 2;
  int yPos = area.getY() + 50;

  ppButton.setBounds(startX + smallButtonWidth + spacingBetween, yPos, smallButtonWidth, smallButtonHeight);
  toEndButton.setBounds(startX + (smallButtonWidth + spacingBetween) * 3 + 20, yPos, smallButtonWidth, smallButtonHeight);
  toStartButton.setBounds(startX - (smallButtonWidth + spacingBetween) - 20, yPos, smallButtonWidth, smallButtonHeight);
  bw10Button.setBounds(startX, yPos, smallButtonWidth, smallButtonHeight);
  fw10Button.setBounds(startX + (smallButtonWidth + spacingBetween) * 2, yPos, smallButtonWidth, smallButtonHeight);
}

juce::String PlayerGUI::formatTime(double seconds)
{
  int mins = static_cast<int>(seconds / 60);
  int secs = static_cast<int>(seconds) % 60;
  return juce::String(mins) + ":" + (secs < 10 ? "0" : "") + juce::String(secs);
}

void PlayerGUI::buttonClicked(juce::Button* button)
{
  if (!audio)
    return;

  if (button == &loadButton)
  {
    audio->loadFileAsync();
  }

  if (button == &restartButton)
  {
    audio->restart();
  }

  if (button == &stopButton)
  {
    audio->stop();
    ppButton.setImages(playIcon.get());
  }

  if (button == &muteButton)
  {
    bool isMuted = audio->getGain() == 0.0f;
    audio->setGain(isMuted ? (float)volumeSlider.getValue() : 0.0f);
    if (isMuted) {
      muteButton.setButtonText("Mute");
      muteButton.removeColour(juce::TextButton::buttonColourId);
    }
    else {
      muteButton.setColour(juce::TextButton::buttonColourId, juce::Colours::red);
      muteButton.setButtonText("Unmute");
    }
  }

  if (button == &repeatButton)
  {
    bool shouldLooping = repeatButton.getToggleState();
    audio->setLooping(shouldLooping);
  }

  if (button == &ppButton)
  {
    if (audio->getReaderSource() != nullptr) {
      if (audio->isPlaying())
      {
        audio->stop();
        ppButton.setImages(playIcon.get());
      }
      else
      {
        audio->start();
        ppButton.setImages(pauseButtonIcon.get());
      }
    }
  }

  if (button == &toStartButton)
  {
    if (audio->getReaderSource() != nullptr)
    {
      audio->setPosition(0.0);
      audio->start();
    }
  }

  if (button == &toEndButton)
  {
    if (audio->getReaderSource() != nullptr)
    {
      double total = audio->getTotalLengthSeconds();
      audio->setPosition(total);
      progressSlider.setValue(1.0);
      currentTimeLabel.setText(formatTime(total), juce::dontSendNotification);
    }
  }

  if (button == &fw10Button)
  {
    if (audio->getReaderSource() != nullptr)
    {
      double current = audio->getCurrentPosition();
      double newPos = current + 10.0;
      audio->setPosition(newPos);
    }
  }

  if (button == &bw10Button)
  {
    if (audio->getReaderSource() != nullptr)
    {
      double current = audio->getCurrentPosition();
      double newPos = current - 10.0;
      if (newPos < 0.0)
        newPos = 0.0;
      audio->setPosition(newPos);
    }
  }
}

void PlayerGUI::sliderValueChanged(juce::Slider* slider)
{
  if (!audio) return;

  if (slider == &volumeSlider) {
    audio->setGain((float)slider->getValue());
    muteButton.setButtonText("Mute");
    muteButton.removeColour(juce::TextButton::buttonColourId);
  }

  if (slider == &progressSlider && audio->getReaderSource() != nullptr)
  {
    double total = audio->getTotalLengthSeconds();
    double newPos = progressSlider.getValue() * total;
    audio->setPosition(newPos);
  }
}

void PlayerGUI::timerCallback()
{
  if (audio->isPlaying() && audio->getReaderSource() != nullptr)
  {
    double current = audio->getCurrentPosition();
    double total = audio->getTotalLengthSeconds();

    progressSlider.setValue(current / total);
    currentTimeLabel.setText(formatTime(current), juce::dontSendNotification);
    totalTimeLabel.setText(formatTime(total), juce::dontSendNotification);
  }
}
