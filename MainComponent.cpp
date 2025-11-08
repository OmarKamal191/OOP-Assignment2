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

	// Add audio1 sources to mixer
    mixerSource.addInputSource(audio1.getAudioSource(), false);

	// Add audio2 sources to mixer
    mixerSource.addInputSource(audio2.getAudioSource(), false);

    // Mixer Button
    addAndMakeVisible(MixerButton);
    MixerButton.setButtonText("Mixer");
    MixerButton.addListener(this);

    gui1.volumeSlider.addListener(this);
    gui2.volumeSlider.addListener(this);

    
    juce::PropertiesFile::Options options;
    options.applicationName = "Simple Audio Player";
    options.filenameSuffix = ".xml"; 
    options.folderName = "SimpleAudioPlayer";
    options.osxLibrarySubFolder = "Application Support";

    appProperties.setStorageParameters(options);

   
    loadState();

    updateMix();

}

MainComponent::~MainComponent()
{
    saveState();

    gui1.volumeSlider.removeListener(this);
    gui2.volumeSlider.removeListener(this);

    mixerSource.removeAllInputs();

    shutdownAudio();

    MixerButton.removeListener(this);
}

void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
	// Prepare individual audio sources
    audio1.prepareToPlay(samplesPerBlockExpected, sampleRate);
    audio2.prepareToPlay(samplesPerBlockExpected, sampleRate);

	// Prepare mixer source
    mixerSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
	// Get mixed audio from mixer source
    mixerSource.getNextAudioBlock(bufferToFill);
}

void MainComponent::releaseResources()
{
    mixerSource.releaseResources();
    audio1.releaseResources();
    audio2.releaseResources();
}

void MainComponent::paint(juce::Graphics& g)
{

    g.fillAll(juce::Colour::fromRGB(30, 30, 30));
}

void MainComponent::resized()
{
	// set fixed widths and heights
    const int fixedPlayerWidth = 700; 
    const int buttonWidth = 100;      
    const int buttonHeight = 40;      
    const int topBottomPadding = 10;  

	// Take the bounds and reduce for padding
    auto bounds = getLocalBounds().reduced(0, topBottomPadding);

	// calculate total content width
    int totalContentWidth = (fixedPlayerWidth * 2) + buttonWidth;

	// calculate starting x position to center content
    int contentX = (bounds.getWidth() - totalContentWidth) / 2;

	// Make a rectangle for the content area
    juce::Rectangle<int> contentArea(contentX, bounds.getY(), totalContentWidth, bounds.getHeight());

	// Place Player 1 (lift)
    gui1.setBounds(contentArea.removeFromLeft(fixedPlayerWidth));

	// Place Mixer Button (middle)
    MixerButton.setBounds(contentArea.removeFromLeft(buttonWidth).removeFromTop(buttonHeight));

    // Place Player 2 (right)
    gui2.setBounds(contentArea);
}

void MainComponent::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &gui1.volumeSlider || slider == &gui2.volumeSlider)
    {
        updateMix();
    }
}


void MainComponent::buttonClicked(juce::Button* button)
{
    if (button == &MixerButton)
    {
        audio1.restart();
        audio2.restart();

        if (gui1.pauseButtonIcon.get()) 
        {
            gui1.ppButton.setImages(gui1.pauseButtonIcon.get());
        }

        if (gui2.pauseButtonIcon.get()) 
        {
            gui2.ppButton.setImages(gui2.pauseButtonIcon.get());
        }
       
    }
}

void MainComponent::updateMix()
{
    float vol1 = (float)gui1.volumeSlider.getValue();
    float vol2 = (float)gui2.volumeSlider.getValue();

	// set gains
    audio1.setGain(vol1);
    audio2.setGain(vol2);
}



void MainComponent::saveState()
{
	// Player 1
    juce::PropertiesFile* props = appProperties.getUserSettings();
    if (props == nullptr)
        return;

	// get playlist files for Player 1
    const auto& playlist1 = gui1.getPlaylistFileObjects();
    juce::StringArray playlistPaths1;

	// store file paths in string array
    for (const auto& file : playlist1)
        playlistPaths1.add(file.getFullPathName());

	// save playlist paths as a single string with newlines
    props->setValue("playlist1", playlistPaths1.joinIntoString("\n")); 

	// get track durations for Player 1
    const auto& durationsVector1 = gui1.playlistModel->trackDurations;
    juce::StringArray durationsArray1;
    for (const auto& dur : durationsVector1)
        durationsArray1.add(dur);
    props->setValue("playlistDurations1", durationsArray1.joinIntoString("\n"));

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

    // Player 2
    const auto& playlist2 = gui2.getPlaylistFileObjects();
    juce::StringArray playlistPaths2;
    for (const auto& file : playlist2)
        playlistPaths2.add(file.getFullPathName());
    props->setValue("playlist2", playlistPaths2.joinIntoString("\n")); 

    const auto& durationsVector2 = gui2.playlistModel->trackDurations;
    juce::StringArray durationsArray2;
    for (const auto& dur : durationsVector2)
        durationsArray2.add(dur);
    props->setValue("playlistDurations2", durationsArray2.joinIntoString("\n")); 

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

   

    appProperties.saveIfNeeded();
}


void MainComponent::loadState()
{
    juce::PropertiesFile* props = appProperties.getUserSettings();
    if (props == nullptr)
        return;

	// Load Player 1 state
    float volume1 = (float)props->getDoubleValue("volume1", 0.5);
    gui1.volumeSlider.setValue(volume1, juce::dontSendNotification);

	// load speed and repeat
    double speed1 = props->getDoubleValue("speed1", 1.0);
    audio1.setSpeed(speed1);
    bool repeat1 = props->getBoolValue("repeat1", false);
    audio1.setLooping(repeat1);

	// load playlist
    juce::StringArray playlistPaths1;
    playlistPaths1.addLines(props->getValue("playlist1", ""));

	// load durations
    juce::StringArray playlistDurations1;
    playlistDurations1.addLines(props->getValue("playlistDurations1", ""));

	// convert paths to File objects
    std::vector<juce::File> playlistFiles1;
    for (const auto& path : playlistPaths1)
        if (path.isNotEmpty()) playlistFiles1.push_back(juce::File(path));

	// set playlist in GUI
    gui1.setPlaylist(playlistFiles1, playlistDurations1); 

	// load last file and position if available
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
    gui2.setPlaylist(playlistFiles2, playlistDurations2); 

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

    
    updateMix();
}


