#pragma once
#include <JuceHeader.h>

class PlayerAudio
{
public:
  PlayerAudio();
  ~PlayerAudio();

  void prepareToPlay(int samplesPerBlockExpected, double sampleRate);
  void releaseResources();

  // Loading
  void loadFileAsync(); // launches chooser (requires being called from GUI thread)
  void loadFile(const juce::File& file);

  // Controls
  void start();
  void stop();
  void restart();
  void setPosition(double seconds);
  double getCurrentPosition() const;
  double getTotalLengthSeconds() const;
  void setGain(float g);
  float getGain() const;
  bool isPlaying() const;
  void setLooping(bool shouldLoop);

  juce::AudioTransportSource* getTransportSource() noexcept { return &transportSource; }
  juce::AudioFormatManager* getFormatManager() noexcept { return &formatManager; }
  juce::AudioFormatReaderSource* getReaderSource() const noexcept { return readerSource.get(); }

private:
  juce::AudioFormatManager formatManager;
  std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
  juce::AudioTransportSource transportSource;

  std::unique_ptr<juce::FileChooser> fileChooser;
};
