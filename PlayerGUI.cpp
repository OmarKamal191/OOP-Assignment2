#include "PlayerGUI.h"

PlayerGUI::PlayerGUI()
{
    for (auto* btn : { &loadButton , &restartButton , &stopButton , &muteButton , &loopRegionButton })
    {
        btn->addListener(this);
        addAndMakeVisible(btn);
    }

    // Sleep Timer button (next to Mute)
    sleepTimerButton.addListener(this);
    addAndMakeVisible(sleepTimerButton);

    volumeSlider.setRange(0.0, 1.0, 0.01);
    volumeSlider.setValue(0.5);
    volumeSlider.addListener(this);
    addAndMakeVisible(volumeSlider);

    // Speed slider (new)
    speedSlider.setRange(0.25, 2.0, 0.05);
    speedSlider.setValue(1.0);
    speedSlider.setTextValueSuffix("x");
    speedSlider.setNumDecimalPlacesToDisplay(2);
    speedSlider.setSkewFactorFromMidPoint(1.0); // finer control near 1.0
    speedSlider.addListener(this);
    addAndMakeVisible(speedSlider);

    // Progress Slider 
    progressSlider.setRange(0.0, 1.0);
    progressSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    progressSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    progressSlider.addListener(this);
    addAndMakeVisible(progressSlider);

    addAndMakeVisible(currentTimeLabel);
    addAndMakeVisible(totalTimeLabel);
    currentTimeLabel.setText("0:00", juce::dontSendNotification);
    totalTimeLabel.setText("0:00", juce::dontSendNotification);
    startTimer(250);

    repeatButton.addListener(this);
    addAndMakeVisible(repeatButton);

    // pause icon
    juce::Path pausePath;
    pausePath.addRectangle(0.0f, 0.0f, 6.0f, 20.0f);
    pausePath.addRectangle(14.0f, 0.0f, 6.0f, 20.0f);
    pauseButtonIcon = makeIcon(pausePath, juce::Colours::white);

    // play icon
    juce::Path testPath;
    testPath.addTriangle(0.0f, 0.0f, 20.0f, 10.0f, 0.0f, 20.0f);
    playIcon = makeIcon(testPath, juce::Colours::white);

    // toEnd icon
    juce::Path toEndPath;
    toEndPath.addTriangle(0.0f, 0.0f, 10.0f, 10.0f, 0.0f, 20.0f);
    toEndPath.addRectangle(12.0f, 0.0f, 4.0f, 20.0f);
    toEndIcon = makeIcon(toEndPath, juce::Colours::white);

    // toStart icon
    juce::Path toStartPath;
    toStartPath.addRectangle(0.0f, 0.0f, 4.0f, 20.0f);
    toStartPath.addTriangle(16.0f, 0.0f, 6.0f, 10.0f, 16.0f, 20.0f);
    toStartIcon = makeIcon(toStartPath, juce::Colours::white);

    // fw10
    juce::Path fw10Path;
    fw10Path.addTriangle(0.0f, 0.0f, 10.0f, 10.0f, 0.0f, 20.0f);
    fw10Path.addTriangle(12.0f, 0.0f, 22.0f, 10.0f, 12.0f, 20.0f);
    fw10Icon = makeIcon(fw10Path, juce::Colours::white);

    // bw10
    juce::Path bw10Path;
    bw10Path.addTriangle(22.0f, 0.0f, 12.0f, 10.0f, 22.0f, 20.0f);
    bw10Path.addTriangle(10.0f, 0.0f, 0.0f, 10.0f, 10.0f, 20.0f);
    bw10Icon = makeIcon(bw10Path, juce::Colours::white);

    ppButton.setImages(pauseButtonIcon.get());
    toEndButton.setImages(toEndIcon.get());
    toStartButton.setImages(toStartIcon.get());
    fw10Button.setImages(fw10Icon.get());
    bw10Button.setImages(bw10Icon.get());

    for (auto* btn : { &ppButton , &toEndButton , &toStartButton , &fw10Button , &bw10Button })
    {
        btn->addListener(this);
        addAndMakeVisible(btn);
    }
}

PlayerGUI::~PlayerGUI()
{
    stopTimer();
    for (auto* btn : { &loadButton , &restartButton , &stopButton , &muteButton ,&loopRegionButton })
        btn->removeListener(this);

    // remove Sleep Timer listener
    sleepTimerButton.removeListener(this);
    removeChildComponent(&sleepTimerButton);

    for (auto* btn : { &ppButton , &toEndButton , &toStartButton , &fw10Button , &bw10Button })
        btn->removeListener(this);

    volumeSlider.removeListener(this);
    speedSlider.removeListener(this);
    progressSlider.removeListener(this);
    repeatButton.removeListener(this);

    // reset thumbnail
    thumbnail.reset();
    thumbnailCache.reset();
}

void PlayerGUI::setAudio(PlayerAudio* audioPtr) noexcept
{
    audio = audioPtr;

    // create thumbnail+cache now that we can access a format manager
    if (audio != nullptr && audio->getFormatManager() != nullptr)
    {
        thumbnailCache = std::make_unique<juce::AudioThumbnailCache>(5);
        thumbnail = std::make_unique<juce::AudioThumbnail>(512, *audio->getFormatManager(), *thumbnailCache);

        // if audio already has a file, set it now
        juce::File f = audio->getCurrentFile();
        if (f.existsAsFile())
        {
            thumbnail->setSource(new juce::FileInputSource(f));
            lastLoadedFile = f;
        }
    }
}

void PlayerGUI::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour::fromRGB(30, 30, 30));

    juce::ColourGradient gradient(juce::Colours::darkgrey, 0, 0,
        juce::Colours::black, 0, (float)getHeight(), false);
    g.setGradientFill(gradient);
    g.fillAll();

    g.setColour(juce::Colours::white);
    g.setFont(24.0f);
    g.drawFittedText("Simple Audio Player", getLocalBounds().reduced(10), juce::Justification::centredTop, 1);

    // Draw waveform if available
    if (thumbnail)
    {
        // background for waveform
        g.setColour(juce::Colours::black.withAlpha(0.6f));
        g.fillRect(waveformBounds);

        // outline
        g.setColour(juce::Colours::grey);
        g.drawRect(waveformBounds);

        double total = audio ? audio->getTotalLengthSeconds() : 0.0;
        if (total > 0.0)
        {
            thumbnail->drawChannels(g, waveformBounds.reduced(4), 0.0, total, 1.0f);

            // New: Draw Loop Region Markers (رسم مؤشرات منطقة التكرار)
            if (loopRegionActive)
            {
                // حساب موضع البداية والنهاية على الشاشة (بكسل)
                auto getX = [&](double seconds)
                    {
                        double pos = juce::jlimit(0.0, 1.0, seconds / total);
                        return waveformBounds.getX() + static_cast<int>(pos * (double)waveformBounds.getWidth());
                    };

                int startX = getX(loopStartSeconds);
                int endX = getX(loopEndSeconds);

                // 1. تظليل المنطقة بين start و end
                juce::Rectangle<int> loopArea(startX, waveformBounds.getY(), endX - startX, waveformBounds.getHeight());
                g.setColour(juce::Colours::blue.withAlpha(0.2f));
                g.fillRect(loopArea);

                // 2. رسم خطوط البداية والنهاية
                g.setColour(juce::Colours::yellow);
                g.drawLine((float)startX, (float)waveformBounds.getY(), (float)startX, (float)waveformBounds.getBottom(), 2.0f); // خط البداية
                g.drawLine((float)endX, (float)waveformBounds.getY(), (float)endX, (float)waveformBounds.getBottom(), 2.0f);   // خط النهاية
            }

            // draw current position pointer
            double current = audio ? audio->getCurrentPosition() : 0.0;
            double pos = juce::jlimit(0.0, 1.0, (total > 0.0) ? (current / total) : 0.0);
            int x = waveformBounds.getX() + static_cast<int>(pos * (double)waveformBounds.getWidth());
            g.setColour(juce::Colours::deepskyblue);
            g.drawLine((float)x, (float)waveformBounds.getY(), (float)x, (float)waveformBounds.getBottom(), 2.0f);
        }
        else
        {
            g.setColour(juce::Colours::darkgrey);
            g.drawText("Waveform", waveformBounds, juce::Justification::centred);
        }
    }

}

void PlayerGUI::resized()
{
    auto area = getLocalBounds().reduced(50);

    auto buttonArea = area.removeFromTop(50);
    int buttonWidth = 85;
    int spacing = 10;
    loadButton.setBounds(buttonArea.removeFromLeft(buttonWidth));
    buttonArea.removeFromLeft(spacing);
    restartButton.setBounds(buttonArea.removeFromLeft(buttonWidth));
    buttonArea.removeFromLeft(spacing);
    stopButton.setBounds(buttonArea.removeFromLeft(buttonWidth));
    buttonArea.removeFromLeft(spacing);
    muteButton.setBounds(buttonArea.removeFromLeft(buttonWidth));

    // Place the Sleep Timer button directly to the right of the Mute button
    buttonArea.removeFromLeft(spacing);
    sleepTimerButton.setBounds(buttonArea.removeFromLeft(buttonWidth));
    area.removeFromTop(20);
    volumeSlider.setBounds(area.removeFromTop(40));

    // Place the new Loop Region button 
    buttonArea.removeFromLeft(spacing);
    loopRegionButton.setBounds(buttonArea.removeFromLeft(buttonWidth + 20)); // <--- إضافة
    area.removeFromTop(10);

    // Place speed slider under volume slider
    speedSlider.setBounds(area.removeFromTop(30));


    // Place progress slider and lables and repeat button
    progressSlider.setBounds(20, 220, getWidth() - 40, 20);
    currentTimeLabel.setBounds(20, 240, 60, 20);
    totalTimeLabel.setBounds(getWidth() - 65, 240, 60, 20);
    repeatButton.setBounds(getWidth() - 160, 230, buttonWidth, 40);


    // Place control button
    int smallButtonWidth = 70;
    int smallButtonHeight = 40;
    int spacingBetween = 20;

    int totalWidth = (smallButtonWidth * 3) + (spacingBetween * 2);
    int startX = (getWidth() - totalWidth) / 2;
    int yPos = area.getY() + 50;

    ppButton.setBounds(startX + smallButtonWidth + spacingBetween, yPos, smallButtonWidth, smallButtonHeight);
    toEndButton.setBounds(startX + (smallButtonWidth + spacingBetween) * 3 + 20, yPos, smallButtonWidth, smallButtonHeight);
    toStartButton.setBounds(startX - (smallButtonWidth + spacingBetween) - 20, yPos, smallButtonWidth, smallButtonHeight);
    bw10Button.setBounds(startX, yPos, smallButtonWidth, smallButtonHeight);
    fw10Button.setBounds(startX + (smallButtonWidth + spacingBetween) * 2, yPos, smallButtonWidth, smallButtonHeight);

    // Place waveform under the small control buttons
    int wfHeight = 120;
    int wfY = yPos + smallButtonHeight + 20;
    waveformBounds = { 20, wfY, getWidth() - 40, wfHeight };
}

juce::String PlayerGUI::formatTime(double seconds)
{
    int mins = static_cast<int>(seconds / 60);
    int secs = static_cast<int>(seconds) % 60;
    return juce::String(mins) + ":" + (secs < 10 ? "0" : "") + juce::String(secs);
}

void PlayerGUI::buttonClicked(juce::Button* button)
{
    if (!audio)
        return;

    if (button == &loadButton)
    {
        audio->loadFileAsync();
    }

    if (button == &restartButton)
    {
        audio->restart();
    }

    if (button == &stopButton)
    {
        audio->stop();
        audio->setPosition(0.0);
        ppButton.setImages(playIcon.get());
    }

    if (button == &muteButton)
    {
        bool isMuted = audio->getGain() == 0.0f;
        audio->setGain(isMuted ? (float)volumeSlider.getValue() : 0.0f);
        if (isMuted) {
            muteButton.setButtonText("Mute");
            muteButton.removeColour(juce::TextButton::buttonColourId);
        }
        else {
            muteButton.setColour(juce::TextButton::buttonColourId, juce::Colours::green);
            muteButton.setButtonText("Unmute");
        }
    }

    if (button == &repeatButton)
    {
        bool shouldLooping = repeatButton.getToggleState();
        audio->setLooping(shouldLooping);
    }

    if (button == &ppButton)
    {
        if (audio->getReaderSource() != nullptr) {
            if (audio->isPlaying())
            {
                audio->stop();
                ppButton.setImages(playIcon.get());
            }
            else
            {
                audio->start();
                ppButton.setImages(pauseButtonIcon.get());
            }
        }
    }

    if (button == &toStartButton)
    {
        if (audio->getReaderSource() != nullptr)
        {
            audio->setPosition(0.0);
            audio->start();
        }
    }

    if (button == &toEndButton)
    {
        if (audio->getReaderSource() != nullptr)
        {
            double total = audio->getTotalLengthSeconds();
            audio->setPosition(total);
            progressSlider.setValue(1.0);
            currentTimeLabel.setText(formatTime(total), juce::dontSendNotification);
        }
    }

    if (button == &fw10Button)
    {
        if (audio->getReaderSource() != nullptr)
        {
            double current = audio->getCurrentPosition();
            double newPos = current + 10.0;
            audio->setPosition(newPos);
        }
    }

    if (button == &bw10Button)
    {
        if (audio->getReaderSource() != nullptr)
        {
            double current = audio->getCurrentPosition();
            double newPos = current - 10.0;
            if (newPos < 0.0)
                newPos = 0.0;
            audio->setPosition(newPos);
        }
    }

    // Sleep Timer button handling
    if (button == &sleepTimerButton)
    {
        juce::PopupMenu menu;
        menu.addItem(1, "1 minute");
        menu.addItem(2, "2 minutes");
        menu.addItem(5, "5 minutes");
        menu.addItem(10, "10 minutes");
        menu.addItem(15, "15 minutes");
        menu.addItem(20, "20 minutes");
        menu.addItem(30, "30 minutes");
        menu.addSeparator();
        menu.addItem(1000, "Cancel sleep timer");

        // showMenuAsync to avoid using the non-existent synchronous show() method
        juce::Component::SafePointer<PlayerGUI> safe(this);
        menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(this),
            [safe](int choice)
            {
                if (auto* p = safe.getComponent())
                {
                    if (choice > 0 && choice != 1000)
                    {
                        // start timer for choice minutes
                        p->sleepTimerEnd = juce::Time::getCurrentTime() + juce::RelativeTime::minutes(choice);
                        p->sleepTimerActive = true;

                        // update button text to show initial countdown
                        auto secs = choice * 60;
                        p->sleepTimerButton.setButtonText("Sleep: " + p->formatTime(secs));
                    }
                    else if (choice == 1000)
                    {
                        // cancel
                        p->sleepTimerActive = false;
                        p->sleepTimerButton.setButtonText("Sleep Timer");
                    }
                }
            });
    }
    // Loop Region button 
    if (button == &loopRegionButton)
    {
        if (!audio->getReaderSource())
            return;

        if (!loopRegionActive)
        {
            //تفعيل وضع التكرار
            loopRegionActive = true;
            loopStartSeconds = 0.0; // إعادة تعيين النقاط
            loopEndSeconds = audio->getTotalLengthSeconds();

            // طلب من المستخدم تعيين النقاط 
            loopRegionButton.setButtonText("Set Start (Click Waveform)");
            settingLoopPoint = LoopPointState::SettingStart;
            loopRegionButton.setColour(juce::TextButton::buttonColourId, juce::Colours::orange);

            // تحديث حالة الصوت
            audio->setRegionLooping(true, loopStartSeconds, loopEndSeconds);
        }
        else
        {
            // إلغاء وضع التكرار
            loopRegionActive = false;
            settingLoopPoint = LoopPointState::None;

            // إعادة الزر للحالة الافتراضية
            loopRegionButton.setButtonText("Repeat a Track");
            loopRegionButton.removeColour(juce::TextButton::buttonColourId);

            // تحديث حالة الصوت لإلغاء التكرار
            audio->setRegionLooping(false, 0.0, 0.0);
        }
    }
}

void PlayerGUI::sliderValueChanged(juce::Slider* slider)
{
    if (!audio) return;

    if (slider == &volumeSlider) {
        audio->setGain((float)slider->getValue());
        muteButton.setButtonText("Mute");
        muteButton.removeColour(juce::TextButton::buttonColourId);
    }

    // Speed slider handling (new)
    if (slider == &speedSlider)
    {
        double ratio = speedSlider.getValue();
        audio->setSpeed(ratio);
        return;
    }

    if (slider == &progressSlider && audio->getReaderSource() != nullptr)
    {
        double total = audio->getTotalLengthSeconds();
        double newPos = progressSlider.getValue() * total;

        // If region looping is active, clamp newPos to [loopStartSeconds, loopEndSeconds]
        if (loopRegionActive)
        {
            if (newPos < loopStartSeconds) newPos = loopStartSeconds;
            if (newPos > loopEndSeconds) newPos = loopEndSeconds;
        }

        audio->setPosition(newPos);

    }

}

void PlayerGUI::timerCallback()
{
    // If a new file was loaded, set the thumbnail source
    if (audio != nullptr)
    {
        juce::File f = audio->getCurrentFile();
        if (f != lastLoadedFile && f.existsAsFile() && thumbnail)
        {
            thumbnail->setSource(new juce::FileInputSource(f));
            lastLoadedFile = f;
        }
    }

    if (audio->isPlaying() && audio->getReaderSource() != nullptr)
    {
        double current = audio->getCurrentPosition();
        double total = audio->getTotalLengthSeconds();

        if (total > 0.0)
        {
            // Use dontSendNotification to avoid triggering sliderValueChanged, which would call setPosition()
            progressSlider.setValue(current / total, juce::dontSendNotification);
            currentTimeLabel.setText(formatTime(current), juce::dontSendNotification);
            totalTimeLabel.setText(formatTime(total), juce::dontSendNotification);
        }
        if (loopRegionActive && current >= loopEndSeconds)
        {
            // loop back to start of region
            audio->setPosition(loopStartSeconds);
            // make sure playback continues (in case it was paused/stopped)
            if (!audio->isPlaying())
                audio->start();
        }
        else if (loopRegionActive && current < loopStartSeconds)
        {
            // if playback is before loop start, jump to loop start
            audio->setPosition(loopStartSeconds);
            if (!audio->isPlaying())
                audio->start();
        }
    }

    // Sleep timer update and enforcement
    if (sleepTimerActive)
    {
        auto now = juce::Time::getCurrentTime();
        if (now >= sleepTimerEnd)
        {
            juce::JUCEApplication::getInstance()->systemRequestedQuit();
        }
        else
        {
            auto remainingSeconds = static_cast<int>((sleepTimerEnd.toMilliseconds() - now.toMilliseconds()) / 1000);
            if (remainingSeconds < 0) remainingSeconds = 0;
            sleepTimerButton.setButtonText("Sleep: " + formatTime(remainingSeconds));
        }
    }

    // request repaint so pointer moves smoothly
    repaint(waveformBounds);
}

void PlayerGUI::mouseDown(const juce::MouseEvent& event)
{
    if (!audio) return;

    if (waveformBounds.contains(event.getPosition()))
    {
        double total = audio->getTotalLengthSeconds();
        if (total <= 0.0) return;

        double localX = event.x - waveformBounds.getX();
        double proportion = juce::jlimit(0.0, 1.0, localX / (double)waveformBounds.getWidth());
        double clickedPos = proportion * total; // الموضع بالثواني

        // New: Logic to set loop points (منطق تعيين نقاط التكرار)
        if (settingLoopPoint != LoopPointState::None)
        {
            if (settingLoopPoint == LoopPointState::SettingStart)
            {
                loopStartSeconds = clickedPos;
                settingLoopPoint = LoopPointState::SettingEnd;
                loopRegionButton.setButtonText("Set End (Click Waveform)");

                // إذا كانت نقطة البداية الجديدة أكبر من النهاية الحالية، اضبط النهاية على طول الملف مؤقتاً
                if (loopStartSeconds > loopEndSeconds)
                    loopEndSeconds = total;
            }
            else if (settingLoopPoint == LoopPointState::SettingEnd)
            {
                loopEndSeconds = clickedPos;
                settingLoopPoint = LoopPointState::None; // الانتهاء من التعيين

                // تأكد من أن start <= end
                if (loopStartSeconds > loopEndSeconds)
                    std::swap(loopStartSeconds, loopEndSeconds);

                // إعادة الزر إلى حالة التكرار النشطة
                loopRegionButton.setButtonText("Looping (" + formatTime(loopStartSeconds) + " to " + formatTime(loopEndSeconds) + ")");
                loopRegionButton.setColour(juce::TextButton::buttonColourId, juce::Colours::green);
            }

            // تحديث منطق الصوت في PlayerAudio بالنقاط الجديدة
            audio->setRegionLooping(true, loopStartSeconds, loopEndSeconds);

            // قم بوضع مؤشر التشغيل على نقطة البداية وكرر
            audio->setPosition(loopStartSeconds);
        }
        else
        {
            // المنطق الأصلي (للتحريك العادي للمؤشر إذا لم يكن في وضع تعيين التكرار)
            audio->setPosition(clickedPos);
            progressSlider.setValue(proportion, juce::dontSendNotification);
        }

        repaint(waveformBounds);
    }
}