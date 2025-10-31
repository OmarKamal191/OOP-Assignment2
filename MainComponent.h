#pragma once

#include <JuceHeader.h>
#include "PlayerGUI.h"
#include "PlayerAudio.h"

class MainComponent : public juce::AudioAppComponent
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

	// keep these function names (MainComponent will forward to gui)
	void buttonClicked(juce::Button* button);
	void sliderValueChanged(juce::Slider* slider);
	void timerCallback();

	juce::String formatTime(double seconds);

private:
	PlayerGUI gui;
	PlayerAudio audio;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
