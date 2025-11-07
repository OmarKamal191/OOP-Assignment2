#include "MainComponent.h"

MainComponent::MainComponent()
{
	// Player 1
	addAndMakeVisible(gui1);
	gui1.setAudio(&audio1);

	// Player 2
	addAndMakeVisible(gui2);
	gui2.setAudio(&audio2);

	setSize(500, 250);
	setAudioChannels(0, 2);

	// Add audio sources to the Mixer
	mixerSource.addInputSource(audio1.getAudioSource(), false);
	mixerSource.addInputSource(audio2.getAudioSource(), false);

	// --- Configure the Crossfader slider ---
	addAndMakeVisible(crossfader);
	crossfader.setSliderStyle(juce::Slider::LinearHorizontal);
	crossfader.setRange(0.0, 1.0, 0.01); // 0.0 = Player 1, 1.0 = Player 2
	crossfader.setValue(0.5); // Default to center
	crossfader.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
	crossfader.addListener(this);

	// Make MainComponent listen to volume changes
	gui1.volumeSlider.addListener(this);
	gui2.volumeSlider.addListener(this);


	juce::PropertiesFile::Options options;
	options.applicationName = "Simple Audio Player";
	options.filenameSuffix = ".xml";
	options.folderName = "SimpleAudioPlayer";
	options.osxLibrarySubFolder = "Application Support";

	appProperties.setStorageParameters(options);


	loadState();

	// --- Initialize the mixer volumes correctly ---
	updateMix();

}

MainComponent::~MainComponent()
{
	saveState();

	// --- Remove volume listeners ---
	gui1.volumeSlider.removeListener(this);
	gui2.volumeSlider.removeListener(this);

	// (!!!) IMPORTANT: Remove all inputs from the mixer
	mixerSource.removeAllInputs();

	shutdownAudio();
}

void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
	// Prepare all sources in the Mixer chain
	audio1.prepareToPlay(samplesPerBlockExpected, sampleRate);
	audio2.prepareToPlay(samplesPerBlockExpected, sampleRate);
	mixerSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
	// Pass the buffer to the Mixer
	mixerSource.getNextAudioBlock(bufferToFill);
}

void MainComponent::releaseResources()
{
	// Release resources for all sources in the Mixer chain
	mixerSource.releaseResources();
	audio1.releaseResources();
	audio2.releaseResources();
}

void MainComponent::paint(juce::Graphics& g)
{
	// (gui.paint(g);) // Not needed here
	// The components handle their own painting. Draw the background here.
	g.fillAll(juce::Colour::fromRGB(30, 30, 30));
}

void MainComponent::resized()
{
	// (gui.setBounds(getLocalBounds());) // Not needed here

	// Divide the space (Player 1 left, Player 2 right)
	auto bounds = getLocalBounds();
	auto padding = 10;

	bounds.removeFromBottom(10);

	// Allocate space for the Crossfader at the bottom
	auto crossfaderArea = bounds.removeFromBottom(40).reduced(padding);

	// Split the remaining bounds area into two equal halves
	auto leftHalf = bounds.removeFromLeft(bounds.getWidth() / 2);
	gui1.setBounds(leftHalf.reduced(padding));
	gui2.setBounds(bounds.reduced(padding)); // bounds now holds the right half

	// Position the Crossfader component
	crossfader.setBounds(crossfaderArea);

}

void MainComponent::sliderValueChanged(juce::Slider* slider)
{
	if (slider == &crossfader || slider == &gui1.volumeSlider || slider == &gui2.volumeSlider)
	{
		updateMix();
	}
}

// Applies volume and crossfade changes
void MainComponent::updateMix()
{
	// Get current slider values
	float vol1 = (float)gui1.volumeSlider.getValue();
	float vol2 = (float)gui2.volumeSlider.getValue();
	float crossfadePos = (float)crossfader.getValue(); // (0.0 = Left, 1.0 = Right)

	// Calculate "Constant Power" crossfade (smooth volume transition)
	// This ensures the total perceived loudness remains consistent
	auto angle = crossfadePos * juce::MathConstants<float>::halfPi; // (0.0 to pi/2)
	float gain1 = vol1 * std::cos(angle);
	float gain2 = vol2 * std::sin(angle);

	// Apply the calculated gains to the audio sources
	audio1.setGain(gain1);
	audio2.setGain(gain2);
}




void MainComponent::saveState()
{
	juce::PropertiesFile* props = appProperties.getUserSettings();
	if (props == nullptr)
		return;

	// --- Save Player 1 state ---
	const auto& playlist1 = gui1.getPlaylistFileObjects();
	juce::StringArray playlistPaths1;
	for (const auto& file : playlist1)
		playlistPaths1.add(file.getFullPathName());
	props->setValue("playlist1", playlistPaths1.joinIntoString("\n")); // <-- Player 1

	const auto& durationsVector1 = gui1.playlistModel->trackDurations;
	juce::StringArray durationsArray1;
	for (const auto& dur : durationsVector1)
		durationsArray1.add(dur);
	props->setValue("playlistDurations1", durationsArray1.joinIntoString("\n")); // <-- Player 1

	juce::File currentFile1 = audio1.getCurrentFile();
	if (currentFile1.existsAsFile())
	{
		props->setValue("lastFile1", currentFile1.getFullPathName());
		props->setValue("lastPosition1", audio1.getCurrentPosition());
	}
	else
	{
		props->removeValue("lastFile1");
		props->removeValue("lastPosition1");
	}

	props->setValue("volume1", gui1.volumeSlider.getValue());
	props->setValue("speed1", audio1.getSpeed());
	props->setValue("repeat1", audio1.isLooping());

	// --- Save Player 2 state ---
	const auto& playlist2 = gui2.getPlaylistFileObjects();
	juce::StringArray playlistPaths2;
	for (const auto& file : playlist2)
		playlistPaths2.add(file.getFullPathName());
	props->setValue("playlist2", playlistPaths2.joinIntoString("\n")); // <-- Player 2

	const auto& durationsVector2 = gui2.playlistModel->trackDurations;
	juce::StringArray durationsArray2;
	for (const auto& dur : durationsVector2)
		durationsArray2.add(dur);
	props->setValue("playlistDurations2", durationsArray2.joinIntoString("\n")); // <-- Player 2

	juce::File currentFile2 = audio2.getCurrentFile();
	if (currentFile2.existsAsFile())
	{
		props->setValue("lastFile2", currentFile2.getFullPathName());
		props->setValue("lastPosition2", audio2.getCurrentPosition());
	}
	else
	{
		props->removeValue("lastFile2");
		props->removeValue("lastPosition2");
	}

	props->setValue("volume2", gui2.volumeSlider.getValue());
	props->setValue("speed2", audio2.getSpeed());
	props->setValue("repeat2", audio2.isLooping());

	// --- Save the Crossfader position ---
	props->setValue("crossfader", crossfader.getValue());

	appProperties.saveIfNeeded();
}


void MainComponent::loadState()
{
	juce::PropertiesFile* props = appProperties.getUserSettings();
	if (props == nullptr)
		return;

	// --- Load Player 1 state ---
	float volume1 = (float)props->getDoubleValue("volume1", 0.5);
	gui1.volumeSlider.setValue(volume1, juce::dontSendNotification);
	double speed1 = props->getDoubleValue("speed1", 1.0);
	audio1.setSpeed(speed1);
	bool repeat1 = props->getBoolValue("repeat1", false);
	audio1.setLooping(repeat1);


	juce::StringArray playlistPaths1;
	playlistPaths1.addLines(props->getValue("playlist1", ""));
	juce::StringArray playlistDurations1;
	playlistDurations1.addLines(props->getValue("playlistDurations1", ""));
	std::vector<juce::File> playlistFiles1;
	for (const auto& path : playlistPaths1)
		if (path.isNotEmpty()) playlistFiles1.push_back(juce::File(path));
	gui1.setPlaylist(playlistFiles1, playlistDurations1); // Load playlist 1

	juce::String lastFilePath1 = props->getValue("lastFile1", "");
	if (lastFilePath1.isNotEmpty())
	{
		juce::File lastFile1(lastFilePath1);
		if (lastFile1.existsAsFile())
		{
			audio1.loadFileDirect(lastFile1);
			double lastPosition1 = props->getDoubleValue("lastPosition1", 0.0);
			audio1.setPosition(lastPosition1);
			if (audio1.onFileLoaded) audio1.onFileLoaded();
			// ... (Update Player 1 GUI) ...
			double total1 = audio1.getTotalLengthSeconds();
			if (total1 > 0.0)
			{
				gui1.progressSlider.setValue(lastPosition1 / total1, juce::dontSendNotification);
				gui1.currentTimeLabel.setText(gui1.formatTime(lastPosition1), juce::dontSendNotification);
				gui1.totalTimeLabel.setText(gui1.formatTime(total1), juce::dontSendNotification);
			}
			gui1.ppButton.setImages(gui1.playIcon.get());
		}
	}


	// --- Load Player 2 state ---
	float volume2 = (float)props->getDoubleValue("volume2", 0.5);
	gui2.volumeSlider.setValue(volume2, juce::dontSendNotification);
	double speed2 = props->getDoubleValue("speed2", 1.0);
	audio2.setSpeed(speed2);
	bool repeat2 = props->getBoolValue("repeat2", false);
	audio2.setLooping(repeat2);

	juce::StringArray playlistPaths2;
	playlistPaths2.addLines(props->getValue("playlist2", ""));
	juce::StringArray playlistDurations2;
	playlistDurations2.addLines(props->getValue("playlistDurations2", ""));
	std::vector<juce::File> playlistFiles2;
	for (const auto& path : playlistPaths2)
		if (path.isNotEmpty()) playlistFiles2.push_back(juce::File(path));
	gui2.setPlaylist(playlistFiles2, playlistDurations2); // Load playlist 2

	juce::String lastFilePath2 = props->getValue("lastFile2", "");
	if (lastFilePath2.isNotEmpty())
	{
		juce::File lastFile2(lastFilePath2);
		if (lastFile2.existsAsFile())
		{
			audio2.loadFileDirect(lastFile2);
			double lastPosition2 = props->getDoubleValue("lastPosition2", 0.0);
			audio2.setPosition(lastPosition2);
			if (audio2.onFileLoaded) audio2.onFileLoaded();
			// ... (Update Player 2 GUI) ...
			double total2 = audio2.getTotalLengthSeconds();
			if (total2 > 0.0)
			{
				gui2.progressSlider.setValue(lastPosition2 / total2, juce::dontSendNotification);
				gui2.currentTimeLabel.setText(gui2.formatTime(lastPosition2), juce::dontSendNotification);
				gui2.totalTimeLabel.setText(gui2.formatTime(total2), juce::dontSendNotification);
			}
			gui2.ppButton.setImages(gui2.playIcon.get());
		}
	}

	crossfader.setValue(props->getDoubleValue("crossfader", 0.5));
	updateMix();
}
