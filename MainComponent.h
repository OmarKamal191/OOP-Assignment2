#pragma once

#include <JuceHeader.h>
#include <vector>

class MainComponent : public juce::AudioAppComponent,
	public juce::Button::Listener,
	public juce::Slider::Listener,
	private juce::Timer
{
public:
	MainComponent();
	~MainComponent() override;

	// Audio
	void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
	void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
	void releaseResources() override;

	// GUI
	void paint(juce::Graphics& g) override;
	void resized() override;


	// Event handlers
	void buttonClicked(juce::Button* button) override;
	void sliderValueChanged(juce::Slider* slider) override;

private:
	// Audio
	juce::AudioFormatManager formatManager;
	std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
	juce::AudioTransportSource transportSource;

	// GUI Controls
	juce::TextButton loadButton{ "Load Files" };
	juce::TextButton restartButton{ "Restart" };
	juce::TextButton stopButton{ "Stop" };
	juce::TextButton muteButton{ "Mute" };
	juce::Slider volumeSlider;
	juce::ToggleButton repeatButton{ "Repeat" };
	juce::TextButton pauseButton{ "||" };
	juce::TextButton rewindButton{ "|<" };
	juce::TextButton endButton{ ">|" };
	juce::TextButton add10Button{ ">>" };
	juce::TextButton sub10Button{ "<<" };

	juce::Slider progressSlider;
	juce::Label currentTimeLabel;
	juce::Label totalTimeLabel;

	void timerCallback() override;

	juce::String formatTime(double seconds);

	std::unique_ptr<juce::FileChooser> fileChooser;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
