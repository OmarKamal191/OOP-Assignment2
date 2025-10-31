#include "PlayerAudio.h"

PlayerAudio::PlayerAudio()
{
  formatManager.registerBasicFormats();
}

PlayerAudio::~PlayerAudio()
{
  if (resamplingSource)
  {
    resamplingSource->releaseResources();
    resamplingSource.reset();
  }

  transportSource.stop();
  transportSource.setSource(nullptr);
  readerSource.reset();
}

void PlayerAudio::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
  transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);

  if (!resamplingSource)
    resamplingSource = std::make_unique<juce::ResamplingAudioSource>(&transportSource, false, 2);

  resamplingSource->prepareToPlay(samplesPerBlockExpected, sampleRate);
  resamplingSource->setResamplingRatio(speedRatio);
}

void PlayerAudio::releaseResources()
{
  if (resamplingSource)
    resamplingSource->releaseResources();

  transportSource.releaseResources();
}

void PlayerAudio::loadFileAsync()
{
  fileChooser = std::make_unique<juce::FileChooser>(
    "Select an audio file...",
    juce::File{},
    "*.wav;*.mp3");

  fileChooser->launchAsync(
    juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
    [this](const juce::FileChooser& fc)
    {
      auto file = fc.getResult();
      if (file.existsAsFile())
        loadFile(file);
    });
}

void PlayerAudio::loadFile(const juce::File& file)
{
  if (!file.existsAsFile())
    return;

  if (auto* reader = formatManager.createReaderFor(file))
  {
    transportSource.stop();
    transportSource.setSource(nullptr);
    readerSource.reset();

    readerSource = std::make_unique<juce::AudioFormatReaderSource>(reader, true);
    transportSource.setSource(readerSource.get(),
      0,
      nullptr,
      reader->sampleRate);

    // remember loaded file so GUI can build a thumbnail
    currentFile = file;

    transportSource.start();

    // ensure resampler uses stored ratio
    if (resamplingSource)
      resamplingSource->setResamplingRatio(speedRatio);
  }
}

void PlayerAudio::start()
{
  if (transportSource.getLengthInSeconds() > 0.0)
    transportSource.start();
}

void PlayerAudio::stop()
{
  transportSource.stop();
}

void PlayerAudio::restart()
{
  transportSource.stop();
  transportSource.setPosition(0.0);
  transportSource.start();
}

void PlayerAudio::setPosition(double seconds)
{
  transportSource.setPosition(seconds);
}

double PlayerAudio::getCurrentPosition() const
{
  return transportSource.getCurrentPosition();
}

double PlayerAudio::getTotalLengthSeconds() const
{
  if (readerSource)
  {
    if (auto* r = readerSource->getAudioFormatReader())
      return readerSource->getTotalLength() / r->sampleRate;
  }
  return 0.0;
}

void PlayerAudio::setGain(float g)
{
  transportSource.setGain(g);
}

float PlayerAudio::getGain() const
{
  return transportSource.getGain();
}

bool PlayerAudio::isPlaying() const
{
  return transportSource.isPlaying();
}

void PlayerAudio::setLooping(bool shouldLoop)
{
  if (readerSource)
    readerSource->setLooping(shouldLoop);
}

void PlayerAudio::setSpeed(double ratio)
{
  if (ratio < 0.01) ratio = 0.01;
  if (ratio > 8.0) ratio = 8.0;
  speedRatio = ratio;
  if (resamplingSource)
    resamplingSource->setResamplingRatio(speedRatio);
}

juce::AudioSource* PlayerAudio::getAudioSource() noexcept
{
  if (resamplingSource)
    return resamplingSource.get();

  return &transportSource;
}
  