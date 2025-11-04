#include "MainComponent.h"

MainComponent::MainComponent()
{
    // connect GUI and Audio
    addAndMakeVisible(gui);
    gui.setAudio(&audio);

    setSize(500, 250);
    setAudioChannels(0, 2);

    // The GUI already registered listeners for its controls.




    juce::PropertiesFile::Options options;
    options.applicationName = "Simple Audio Player";
    options.filenameSuffix = ".xml"; 
    options.folderName = "SimpleAudioPlayer";
    options.osxLibrarySubFolder = "Application Support";

    appProperties.setStorageParameters(options);

   
    loadState();
}

MainComponent::~MainComponent()
{
    saveState();

    shutdownAudio();
}

void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    audio.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    if (auto* source = audio.getAudioSource())
        source->getNextAudioBlock(bufferToFill);
    else
        bufferToFill.clearActiveBufferRegion();
}

void MainComponent::releaseResources()
{
    audio.releaseResources();
}

void MainComponent::paint(juce::Graphics& g)
{
    gui.paint(g); // delegate painting to gui (keeps same appearance)
}

void MainComponent::resized()
{
    gui.setBounds(getLocalBounds());
}

void MainComponent::buttonClicked(juce::Button* button)
{
    // forward to gui
    gui.buttonClicked(button);
}

void MainComponent::sliderValueChanged(juce::Slider* slider)
{
    gui.sliderValueChanged(slider);
}

void MainComponent::timerCallback()
{
    gui.timerCallback();
}

juce::String MainComponent::formatTime(double seconds)
{
    int mins = static_cast<int>(seconds / 60);
    int secs = static_cast<int>(seconds) % 60;
    return juce::String(mins) + ":" + (secs < 10 ? "0" : "") + juce::String(secs);
}

void MainComponent::saveState()
{
	// get user settings file
    juce::PropertiesFile* props = appProperties.getUserSettings();
    if (props == nullptr)
        return;

	// save playlist
    const auto& playlist = gui.getPlaylistFileObjects(); 
    juce::StringArray playlistPaths;
    for (const auto& file : playlist)
    {
        playlistPaths.add(file.getFullPathName());
    }
    
    props->setValue("playlist", playlistPaths.joinIntoString("\n"));

    const auto& durationsVector = gui.playlistModel->trackDurations;

    juce::StringArray durationsArray;
    for (const auto& dur : durationsVector)
    {
        durationsArray.add(dur);
    }

	// save durations
    props->setValue("playlistDurations", durationsArray.joinIntoString("\n"));

	// save last file and position
    juce::File currentFile = audio.getCurrentFile();
    if (currentFile.existsAsFile())
    {
        props->setValue("lastFile", currentFile.getFullPathName());
        props->setValue("lastPosition", audio.getCurrentPosition());
    }
    else
    {
        props->removeValue("lastFile");
        props->removeValue("lastPosition");
    }

	// save settings (volume, speed, repeat)
    props->setValue("volume", audio.getGain());
    props->setValue("speed", audio.getSpeed());
    props->setValue("repeat", audio.isLooping()); 

	// save to disk
    appProperties.saveIfNeeded();
}


void MainComponent::loadState()
{
    juce::PropertiesFile* props = appProperties.getUserSettings();
    if (props == nullptr)
        return;

	// get saved settings
	// use default values if not found
    float volume = (float)props->getDoubleValue("volume", 0.5);
    audio.setGain(volume);

    double speed = props->getDoubleValue("speed", 1.0);
    audio.setSpeed(speed);

    bool repeat = props->getBoolValue("repeat", false);
    audio.setLooping(repeat);

	// get GUI to reflect audio settings
    gui.updateControlsFromAudio();

	// get saved playlist
    juce::StringArray playlistPaths;
    playlistPaths.addLines(props->getValue("playlist", ""));

    juce::StringArray playlistDurations;
    playlistDurations.addLines(props->getValue("playlistDurations", ""));

    std::vector<juce::File> playlistFiles;
    for (const auto& path : playlistPaths)
    {
        if (path.isNotEmpty())
            playlistFiles.push_back(juce::File(path));
    }

	// set playlist in GUI
    gui.setPlaylist(playlistFiles, playlistDurations);

	// get last loaded file and position
    juce::String lastFilePath = props->getValue("lastFile", "");
    if (lastFilePath.isNotEmpty())
    {
        juce::File lastFile(lastFilePath);
        if (lastFile.existsAsFile())
        {
            audio.loadFileDirect(lastFile);

			// set position
            double lastPosition = props->getDoubleValue("lastPosition", 0.0);
            audio.setPosition(lastPosition);

			// call onFileLoaded if set
            if (audio.onFileLoaded)
                audio.onFileLoaded();

			// update metadata label in GUI
            double total = audio.getTotalLengthSeconds();
            if (total > 0.0)
            {
                gui.progressSlider.setValue(lastPosition / total, juce::dontSendNotification);
                gui.currentTimeLabel.setText(gui.formatTime(lastPosition), juce::dontSendNotification);
                gui.totalTimeLabel.setText(gui.formatTime(total), juce::dontSendNotification);
            }

			// set play/pause button to pause icon
            gui.ppButton.setImages(gui.playIcon.get());
        }
    }
}

