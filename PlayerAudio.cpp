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
  juce::FileChooser chooser("Select an audio file...", juce::File{}, "*.mp3;*.wav");

  chooser.launchAsync(juce::FileBrowserComponent::openMode,
    [this](const juce::FileChooser& fc)
    {
      auto file = fc.getResult();
      if (file.existsAsFile())
        loadFile(file);
    });
}


void PlayerAudio::loadFileDirect(const juce::File& file)
{
  if (!file.existsAsFile())
    return;

  trackTitle.clear();
  trackArtist.clear();
  trackAlbum.clear();

  TagLib::FileRef f(file.getFullPathName().toRawUTF8());
  if (!f.isNull() && f.tag())
  {
    auto tag = f.tag();
    juce::String title = juce::String::fromUTF8(tag->title().toCString(true));
    juce::String artist = juce::String::fromUTF8(tag->artist().toCString(true));
    juce::String album = juce::String::fromUTF8(tag->album().toCString(true));
    trackTitle = title;
    trackArtist = artist;
    trackAlbum = album;
  }

  if (auto* reader = formatManager.createReaderFor(file))
  {
    transportSource.stop();
    transportSource.setSource(nullptr);
    readerSource.reset();

    readerSource = std::make_unique<juce::AudioFormatReaderSource>(reader, true);
    transportSource.setSource(readerSource.get(), 0, nullptr, reader->sampleRate);

    currentFile = file;

    if (resamplingSource)
      resamplingSource->setResamplingRatio(speedRatio);
  }
}


void PlayerAudio::loadFile(const juce::File& file)
{
  if (!file.existsAsFile())
    return;
  // Reset metadata
  trackTitle.clear();
  trackArtist.clear();
  trackAlbum.clear();

  TagLib::FileRef f(file.getFullPathName().toRawUTF8());
  if (!f.isNull() && f.tag())
  {
    auto tag = f.tag();

    juce::String title = juce::String::fromUTF8(tag->title().toCString(true));
    juce::String artist = juce::String::fromUTF8(tag->artist().toCString(true));
    juce::String album = juce::String::fromUTF8(tag->album().toCString(true));

    trackTitle = title;
    trackArtist = artist;
    trackAlbum = album;
  }




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

  if (onFileLoaded)
    onFileLoaded();

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

bool PlayerAudio::isLooping() const
{
  if (readerSource)
    return readerSource->isLooping();
  return false;
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

// setRegionLooping function
void PlayerAudio::setRegionLooping(bool shouldLoop, double start, double end)
{
  if (readerSource)
    readerSource->setLooping(false);

  // store state
  regionLoopingActive = shouldLoop;

  if (shouldLoop)
  {
    loopStart = juce::jmin(start, end);
    loopEnd = juce::jmax(start, end);

    // make sure current position is within loop
    if (transportSource.getCurrentPosition() < loopStart || transportSource.getCurrentPosition() >= loopEnd)
      transportSource.setPosition(loopStart);
  }
  else
  {
    // reset loop points
    loopStart = 0.0;
    loopEnd = 0.0;
  }
}

void PlayerAudio::updateMetadata(const juce::File& file)
{
  if (!file.existsAsFile())
    return;

  TagLib::FileRef ref(file.getFullPathName().toRawUTF8());
  if (!ref.isNull() && ref.tag())
  {
    trackTitle = juce::String::fromUTF8(ref.tag()->title().toCString(true));
    trackArtist = juce::String::fromUTF8(ref.tag()->artist().toCString(true));
    trackAlbum = juce::String::fromUTF8(ref.tag()->album().toCString(true));

    if (ref.audioProperties())
    {
      int totalSeconds = ref.audioProperties()->length();
      trackDuration = juce::String(totalSeconds / 60) + ":" +
        juce::String(totalSeconds % 60).paddedLeft('0', 2);
    }
  }
}



// New function to unload audio and clear metadata
void PlayerAudio::unloadFile()
{
  transportSource.stop();
  transportSource.setSource(nullptr);
  readerSource.reset();

  // Clear metadata and current file
  currentFile = juce::File{};
  trackTitle.clear();
  trackArtist.clear();
  trackAlbum.clear();
  trackDuration.clear();
}
