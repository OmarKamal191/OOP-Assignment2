#pragma once

#include <JuceHeader.h>
#include "PlayerGUI.h"
#include "PlayerAudio.h"

class MainComponent : public juce::AudioAppComponent,
	public juce::Slider::Listener,
	public juce::Button::Listener
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

	void sliderValueChanged(juce::Slider* slider) override;
	void buttonClicked(juce::Button* button) override;
	
private:
	// Player 1
	PlayerGUI gui1;
	PlayerAudio audio1;

	// Player 2
	PlayerGUI gui2;
	PlayerAudio audio2;

	// Mixer 
	juce::MixerAudioSource mixerSource;

	// Mixer Button
	juce::TextButton MixerButton;

	void saveState();
	void loadState();

	void updateMix();

	juce::ApplicationProperties appProperties;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};

