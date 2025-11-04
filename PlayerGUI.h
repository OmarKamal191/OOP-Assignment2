#pragma once
#include <JuceHeader.h>
#include "PlayerAudio.h"

class PlayerGUI : public juce::Component,
    public juce::Button::Listener,
    public juce::Slider::Listener,
    private juce::Timer
{
public:
    PlayerGUI();
    ~PlayerGUI() override;

    // setAudio now implemented in cpp so we can build the thumbnail when audio is provided
    void setAudio(PlayerAudio* audioPtr) noexcept;

    void paint(juce::Graphics& g) override;
    void resized() override;

    // keep same public handler names so MainComponent can call them (we'll forward)
    void buttonClicked(juce::Button* button) override;
    void sliderValueChanged(juce::Slider* slider) override;
    void timerCallback() override;

    // allow clicking on waveform to seek
    void mouseDown(const juce::MouseEvent& event) override;

	// set playlist files and durations
    void setPlaylist(const std::vector<juce::File>& files, const juce::StringArray& durations);
    void updateControlsFromAudio();
    const std::vector<juce::File>& getPlaylistFileObjects() const { return playlistFileObjects; }


    void refreshPlaylistDisplay();

    void clearMarkers();

    


    // copy of makeIcon - same signature as original
    std::unique_ptr<juce::DrawablePath> makeIcon(juce::Path& path, juce::Colour colour)
    {
        auto drawablepath = std::make_unique<juce::DrawablePath>();
        drawablepath->setPath(path);
        drawablepath->setFill(colour);
        return drawablepath;
    }

    juce::String formatTime(double seconds);

    // expose some internals if needed (same names as original)
    juce::TextButton loadButton{ "Load Files" };
    juce::TextButton restartButton{ "Restart" };
    juce::TextButton stopButton{ "Stop" };
    juce::TextButton muteButton{ "Mute" };
    juce::Slider volumeSlider;
    juce::ToggleButton repeatButton{ "Repeat" };
    juce::DrawableButton ppButton{ "Play&Pause", juce::DrawableButton::ImageFitted };
    juce::DrawableButton toEndButton{ "toEnd", juce::DrawableButton::ImageFitted };
    juce::DrawableButton toStartButton{ "toStart", juce::DrawableButton::ImageFitted };
    juce::DrawableButton fw10Button{ "Add 10s", juce::DrawableButton::ImageFitted };
    juce::DrawableButton bw10Button{ "Sub 10s", juce::DrawableButton::ImageFitted };
    juce::TextButton loopRegionButton{ "Repeat a Track" };
    juce::TextButton removeSelectedButton{ "Remove Selected" };
    juce::TextButton clearAllButton{ "Clear All" };
    juce::TextButton addMarkerButton{ "Add Marker" };
    juce::TextButton clearMarkersButton{ "clear Markers" };
	juce::Label MarkerBoxLabel;
    

    std::unique_ptr<juce::DrawablePath> playIcon; // Icon for play
    std::unique_ptr<juce::DrawablePath> pauseButtonIcon; // Icon for pause
    std::unique_ptr<juce::DrawablePath> toEndIcon; // Icon for toEnd
    std::unique_ptr<juce::DrawablePath> toStartIcon; // Icon for toStart
    std::unique_ptr<juce::DrawablePath> fw10Icon; // Icon for fw10
    std::unique_ptr<juce::DrawablePath> bw10Icon; // Icon for bw10

    juce::Slider progressSlider;
    juce::Label currentTimeLabel;
    juce::Label totalTimeLabel;
    juce::ToggleButton repeatToggle{ "Repeat" }; // local duplicate name avoided; use repeatButton in original code was ToggleButton named repeatButton
    juce::ToggleButton repeatButton2{ "Repeat" }; // (not used) kept to avoid renaming confusion

    // speed control
    juce::Slider speedSlider; 

    // Sleep Timer button 
    juce::TextButton sleepTimerButton{ "Sleep Timer" };

	// Playlist Model
    class PlaylistModel : public juce::ListBoxModel
    {
    public:

        std::vector<juce::String> trackTitles;
        std::vector<juce::String> trackDurations;

        PlaylistModel(PlayerGUI& owner) : gui(owner) {}

        int getNumRows() override { return gui.playlistFiles.size(); }

        void paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override;

        void listBoxItemClicked(int row, const juce::MouseEvent&) override
        {
            if (row >= 0 && row < gui.playlistFileObjects.size())
            {

                gui.markerTimes.clear();
                gui.markerBox.updateContent();
                gui.markerBox.repaint();
                gui.repaint(gui.waveformBounds);

                juce::File f = gui.playlistFileObjects[row];

				// load the selected file
                gui.audio->loadFileDirect(f);

				// update metadata display
                TagLib::FileRef ref(f.getFullPathName().toRawUTF8());
                juce::String displayTitle;

                if (!ref.isNull() && ref.tag())
                {
                    auto* tag = ref.tag();
                    juce::String title = juce::String::fromUTF8(tag->title().toCString(true));
                    juce::String artist = juce::String::fromUTF8(tag->artist().toCString(true));
                    juce::String album = juce::String::fromUTF8(tag->album().toCString(true));

					// merge metadata into display string
                    if (title.isNotEmpty()) displayTitle = title;
                    if (artist.isNotEmpty()) displayTitle += " - " + artist;
                    if (album.isNotEmpty()) displayTitle += " | " + album;
                }

				// if no metadata, use filename
                if (displayTitle.isEmpty())
                    displayTitle = f.getFileNameWithoutExtension();

				// display the title
                gui.metadataLabel.setText(displayTitle, juce::dontSendNotification);

				// start playback
                gui.audio->start();
                gui.ppButton.setImages(gui.pauseButtonIcon.get());
            }
        }
       

    



    private:
        PlayerGUI& gui;
    };
    std::unique_ptr<PlaylistModel> playlistModel;

    class MarkerModel : public juce::ListBoxModel
    {
    public:
        MarkerModel(PlayerGUI& owner) : gui(owner) {}

        int getNumRows() override;
        void paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override;
        void listBoxItemClicked(int row, const juce::MouseEvent&) override;

    private:
        PlayerGUI& gui;
    };
    std::unique_ptr<MarkerModel> markerModel;


private:
    PlayerAudio* audio = nullptr;

    // Waveform thumbnail + cache
    std::unique_ptr<juce::AudioThumbnailCache> thumbnailCache;
    std::unique_ptr<juce::AudioThumbnail> thumbnail;
    juce::File lastLoadedFile;
    juce::Rectangle<int> waveformBounds;

    juce::ListBox markerBox;
    std::vector<double> markerTimes;

    // Sleep timer state
    bool sleepTimerActive = false;
    juce::Time sleepTimerEnd;

    // Region Loop State
    bool loopRegionActive = false;
    double loopStartSeconds = 0.0;
    double loopEndSeconds = 0.0;

	// mode for setting loop points
    enum class LoopPointState { None, SettingStart, SettingEnd };
    LoopPointState settingLoopPoint = LoopPointState::None;
    
	// Metadata display
    juce::Label metadataLabel;

    // Playlist components
    juce::ListBox playlistBox;
    juce::StringArray playlistFiles;
    std::vector<juce::File> playlistFileObjects;

    
    std::unique_ptr<juce::FileChooser> fileChooser;


    


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PlayerGUI)
};
