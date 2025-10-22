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

	// make icon for testButton
	std::unique_ptr<juce::DrawablePath> makeIcon(juce::Path& path, juce::Colour colour)
	{
		auto drawablepath = std::make_unique<juce::DrawablePath>();
		drawablepath->setPath(path);
		drawablepath->setFill(colour);
		return drawablepath;
	}

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
	juce::TextButton rewindButton{ "|<" };
	juce::TextButton endButton{ ">|" };
	juce::DrawableButton ppButton{ "Play&Pause", juce::DrawableButton::ImageFitted };
	juce::DrawableButton toEndButton{ "toEnd", juce::DrawableButton::ImageFitted };
	juce::DrawableButton toStartButton{ "toStart", juce::DrawableButton::ImageFitted };

	// Icons for Buttons
	std::unique_ptr<juce::DrawablePath> playIcon; // Icon for play
	std::unique_ptr<juce::DrawablePath> pauseButtonIcon; // Icon for pause
	std::unique_ptr<juce::DrawablePath> toEndIcon; // Icon for toEnd
	std::unique_ptr<juce::DrawablePath> toStartIcon; // Icon for toStrat


	juce::Slider progressSlider;
	juce::Label currentTimeLabel;
	juce::Label totalTimeLabel;

	void timerCallback() override;

	juce::String formatTime(double seconds);

	std::unique_ptr<juce::FileChooser> fileChooser;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
