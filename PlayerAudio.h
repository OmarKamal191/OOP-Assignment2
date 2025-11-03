#pragma once
#include <JuceHeader.h>
#include <taglib/fileref.h>
#include <taglib/tag.h>


class PlayerAudio
{
public:
	PlayerAudio();
	~PlayerAudio();

	void prepareToPlay(int samplesPerBlockExpected, double sampleRate);
	void releaseResources();

	// Loading
	void loadFileAsync(); // launches chooser (requires being called from GUI thread)
	void loadFile(const juce::File& file);

	// Direct loading without file chooser (for internal use)
	void loadFileDirect(const juce::File& file);

	



	// Controls
	void start();
	void stop();
	void restart();
	void setPosition(double seconds);
	double getCurrentPosition() const;
	double getTotalLengthSeconds() const;
	void setGain(float g);
	float getGain() const;
	bool isPlaying() const;
	void setLooping(bool shouldLoop);

	// Speed control (1.0 = normal) - optional if you kept earlier changes
	void setSpeed(double ratio);
	double getSpeed() const noexcept { return speedRatio; }

	juce::AudioFormatManager* getFormatManager() noexcept { return &formatManager; }
	juce::AudioFormatReaderSource* getReaderSource() const noexcept { return readerSource.get(); }

	// Returns the top-most AudioSource that should be queried for audio blocks.
	juce::AudioSource* getAudioSource() noexcept;

	// Expose the last loaded file so GUI can build a thumbnail
	juce::File getCurrentFile() const noexcept { return currentFile; }

	// kept for compatibility (returns the transport directly)
	juce::AudioTransportSource* getTransportSource() noexcept { return &transportSource; }

	// New: Region Looping Control (التحكم في تكرار المنطقة)
	void setRegionLooping(bool shouldLoop, double start, double end);
	bool isRegionLooping() const noexcept { return regionLoopingActive; }

	
	void updateMetadata(const juce::File& file);


	// Metadata
	juce::String trackTitle;
	juce::String trackArtist;
	juce::String trackAlbum;


	juce::String trackDuration;



	std::function<void()> onFileLoaded;



private:
	juce::AudioFormatManager formatManager;
	std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
	juce::AudioTransportSource transportSource;

	// Resampler -- may be null if not prepared
	std::unique_ptr<juce::ResamplingAudioSource> resamplingSource;
	double speedRatio = 1.0;

	std::unique_ptr<juce::FileChooser> fileChooser;

	// store the file currently loaded (empty if none)
	juce::File currentFile;

	// Region Looping Data
	bool regionLoopingActive = false;
	double loopStart = 0.0;
	double loopEnd = 0.0;
};
