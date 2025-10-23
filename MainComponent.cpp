#include "MainComponent.h"

MainComponent::MainComponent()
{
  // connect GUI and Audio
  addAndMakeVisible(gui);
  gui.setAudio(&audio);

  setSize(500, 250);
  setAudioChannels(0, 2);

  // The GUI already registered listeners for its controls.
}

MainComponent::~MainComponent()
{
  shutdownAudio();
}

void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
  audio.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
  if (auto* transport = audio.getTransportSource())
    transport->getNextAudioBlock(bufferToFill);
  else
    bufferToFill.clearActiveBufferRegion();
}

void MainComponent::releaseResources()
{
  audio.releaseResources();
}

void MainComponent::paint(juce::Graphics& g)
{
  gui.paint(g); // delegate painting to gui (keeps same appearance)
}

void MainComponent::resized()
{
  gui.setBounds(getLocalBounds());
}

void MainComponent::buttonClicked(juce::Button* button)
{
  // forward to gui
  gui.buttonClicked(button);
}

void MainComponent::sliderValueChanged(juce::Slider* slider)
{
  gui.sliderValueChanged(slider);
}

void MainComponent::timerCallback()
{
  gui.timerCallback();
}

juce::String MainComponent::formatTime(double seconds)
{
  // keep same implementation as before (if any code in gui used its own formatTime,
  // this duplicate keeps names stable)
  int mins = static_cast<int>(seconds / 60);
  int secs = static_cast<int>(seconds) % 60;
  return juce::String(mins) + ":" + (secs < 10 ? "0" : "") + juce::String(secs);
}
