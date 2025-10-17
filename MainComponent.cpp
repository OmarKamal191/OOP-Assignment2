#include "MainComponent.h"

MainComponent::MainComponent()
{
    formatManager.registerBasicFormats();

    // Add buttons
    for (auto* btn : { &loadButton, &restartButton , &stopButton , &muteButton})
    {
        btn->addListener(this);
        addAndMakeVisible(btn);
    }

    // Volume slider
    volumeSlider.setRange(0.0, 1.0, 0.01);
    volumeSlider.setValue(0.5);
    volumeSlider.addListener(this);
    addAndMakeVisible(volumeSlider);

    setSize(500, 250);
    setAudioChannels(0, 2);

    // Progress Slider 
    progressSlider.setRange(0.0, 1.0);
    progressSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    progressSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    progressSlider.addListener(this);
    addAndMakeVisible(progressSlider);

    // Labels
    addAndMakeVisible(currentTimeLabel);
    addAndMakeVisible(totalTimeLabel);
    currentTimeLabel.setText("0:00", juce::dontSendNotification);
    totalTimeLabel.setText("0:00", juce::dontSendNotification);
    startTimer(250);

}

MainComponent::~MainComponent()
{
    shutdownAudio();
}

void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    transportSource.getNextAudioBlock(bufferToFill);
}

void MainComponent::releaseResources()
{
    transportSource.releaseResources();
}

void MainComponent::paint(juce::Graphics& g)
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


void MainComponent::resized()
{
    auto area = getLocalBounds().reduced(50);

    auto buttonArea = area.removeFromTop(50);
    int buttonWidth = 100;
    int spacing = 10;
    loadButton.setBounds(buttonArea.removeFromLeft(buttonWidth));
    buttonArea.removeFromLeft(spacing);
    restartButton.setBounds(buttonArea.removeFromLeft(buttonWidth));
    buttonArea.removeFromLeft(spacing);
    stopButton.setBounds(buttonArea.removeFromLeft(buttonWidth));
    buttonArea.removeFromLeft(spacing);
    muteButton.setBounds(buttonArea.removeFromLeft(buttonWidth));

    /*prevButton.setBounds(340, 20, 80, 40);
    nextButton.setBounds(440, 20, 80, 40);*/

    area.removeFromTop(20);
    volumeSlider.setBounds(area.removeFromTop(40));

    progressSlider.setBounds(20, 180, getWidth() - 40, 20);
    currentTimeLabel.setBounds(20, 210, 60, 20);
    totalTimeLabel.setBounds(getWidth() - 65, 210, 60, 20);
}

void MainComponent::buttonClicked(juce::Button* button)
{
    if (button == &loadButton)
    {
        juce::FileChooser chooser("Select audio files...",
            juce::File{},
            "*.wav;*.mp3");

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
                {
                    if (auto* reader = formatManager.createReaderFor(file))
                    {
                        // 🔑 Disconnect old source first
                        transportSource.stop();
                        transportSource.setSource(nullptr);
                        readerSource.reset();

                        // Create new reader source
                        readerSource = std::make_unique<juce::AudioFormatReaderSource>(reader, true);

                        // Attach safely
                        transportSource.setSource(readerSource.get(),
                            0,
                            nullptr,
                            reader->sampleRate);
                        transportSource.start();
                    }
                }
            });
    }

    if (button == &restartButton)
    {
        transportSource.start();
    }

    if (button == &stopButton)
    {
        transportSource.stop();
        transportSource.setPosition(0.0);
    }

    if (button == &muteButton)
    {
        bool isMuted = transportSource.getGain() == 0.0f;
        transportSource.setGain(isMuted ? (float)volumeSlider.getValue() : 0.0f);
		}
}

void MainComponent::sliderValueChanged(juce::Slider* slider)
{
  if (slider == &volumeSlider)
    transportSource.setGain((float)slider->getValue());

  if (slider == &progressSlider && readerSource.get() != nullptr)
  {
    double total = readerSource->getTotalLength() / readerSource->getAudioFormatReader()->sampleRate;
    double newPos = progressSlider.getValue() * total;
    transportSource.setPosition(newPos);
  }
}


void MainComponent::timerCallback()
{
  if (transportSource.isPlaying() && readerSource.get() != nullptr)
  {
    double current = transportSource.getCurrentPosition();
    double total = readerSource->getTotalLength() / readerSource->getAudioFormatReader()->sampleRate;

    progressSlider.setValue(current / total);
    currentTimeLabel.setText(formatTime(current), juce::dontSendNotification);
    totalTimeLabel.setText(formatTime(total), juce::dontSendNotification);
  }
}

juce::String MainComponent::formatTime(double seconds)
{
  int mins = static_cast<int>(seconds / 60);
  int secs = static_cast<int>(seconds) % 60;
  return juce::String(mins) + ":" + (secs < 10 ? "0" : "") + juce::String(secs);
}


