/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "videoplayer.h"

#include <QtWidgets>
#include <qvideowidget.h>
#include <qvideosurfaceformat.h>
#include <pocketsphinx.h>
#include <stdio.h>
#include <iostream>
#include <string>
#define MODELDIR "/usr/local/share/pocketsphinx/model"

using namespace std;

string v_url; // video URL for our use
uint64_t v_dur; // video duration

VideoPlayer::VideoPlayer(QWidget *parent)
    : QWidget(parent)
    , mediaPlayer(0, QMediaPlayer::VideoSurface)
    , playButton(0)
    , searchButton(0)
    , positionSlider(0)
    , errorLabel(0)
    , l1(0)
    , userInput(0)
{
    QVideoWidget *videoWidget = new QVideoWidget;

    userInput = new QPlainTextEdit();
    userInput->setFixedHeight(30);

    QAbstractButton *openButton = new QPushButton(tr("Open..."));
    connect(openButton, &QAbstractButton::clicked, this, &VideoPlayer::openFile);

    playButton = new QPushButton;
    playButton->setEnabled(false);
    playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));

    searchButton = new QPushButton(tr("Search"));
    connect(searchButton, &QAbstractButton::clicked,
            this, &VideoPlayer::on_pushButton_clicked);

    connect(playButton, &QAbstractButton::clicked,
            this, &VideoPlayer::play);

    positionSlider = new QSlider(Qt::Horizontal);
    positionSlider->setRange(0, 0);

    connect(positionSlider, &QAbstractSlider::sliderMoved,
            this, &VideoPlayer::setPosition);

    errorLabel = new QLabel;
    errorLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
    l1 = new QLabel;
    l1->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);

    QBoxLayout *controlLayout = new QHBoxLayout;
    controlLayout->setMargin(0);
    controlLayout->addWidget(openButton);
    controlLayout->addWidget(playButton);
    controlLayout->addWidget(positionSlider);
    /*controlLayout->addWidget(userInput);
    controlLayout->addWidget(searchButton);*/

    QBoxLayout *controlLayout1 = new QHBoxLayout;
    controlLayout1->setMargin(0);
    controlLayout1->addWidget(userInput);
    controlLayout1->addWidget(searchButton);

    QBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(videoWidget);
    layout->addLayout(controlLayout);
    layout->addLayout(controlLayout1);
    layout->addWidget(errorLabel);
    layout->addWidget(l1);

    setLayout(layout);

    mediaPlayer.setVideoOutput(videoWidget);
    connect(&mediaPlayer, &QMediaPlayer::stateChanged,
            this, &VideoPlayer::mediaStateChanged);
    connect(&mediaPlayer, &QMediaPlayer::positionChanged, this, &VideoPlayer::positionChanged);
    connect(&mediaPlayer, &QMediaPlayer::durationChanged, this, &VideoPlayer::durationChanged);
    typedef void (QMediaPlayer::*ErrorSignal)(QMediaPlayer::Error);
    connect(&mediaPlayer, static_cast<ErrorSignal>(&QMediaPlayer::error),
            this, &VideoPlayer::handleError);
}

VideoPlayer::~VideoPlayer()
{
}

void VideoPlayer::openFile()
{
    QFileDialog fileDialog(this);
    fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
    fileDialog.setWindowTitle(tr("Open Movie"));
    QStringList supportedMimeTypes = mediaPlayer.supportedMimeTypes();
    if (!supportedMimeTypes.isEmpty())
        fileDialog.setMimeTypeFilters(supportedMimeTypes);
    fileDialog.setDirectory(QStandardPaths::standardLocations(QStandardPaths::MoviesLocation).value(0, QDir::homePath()));
    if (fileDialog.exec() == QDialog::Accepted)
        setUrl(fileDialog.selectedUrls().constFirst());
}

void VideoPlayer::setUrl(const QUrl &url)
{
    errorLabel->setText(QString());
    setWindowFilePath(url.isLocalFile() ? url.toLocalFile() : QString());
    mediaPlayer.setMedia(url);
    playButton->setEnabled(true);

    v_url = url.toString().toStdString();
}

void VideoPlayer::play()
{
    switch(mediaPlayer.state()) {
    case QMediaPlayer::PlayingState:
        mediaPlayer.pause();
        break;
    default:
        mediaPlayer.play();
        break;
    }
}

void VideoPlayer::mediaStateChanged(QMediaPlayer::State state)
{
    switch(state) {
    case QMediaPlayer::PlayingState:
        playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
        break;
    default:
        playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
        break;
    }
}

void VideoPlayer::positionChanged(qint64 position)
{
    positionSlider->setValue(position);
}

void VideoPlayer::durationChanged(qint64 duration)
{
    positionSlider->setRange(0, duration);

    // Set the video duration
    v_dur = positionSlider->maximum() / 1000;
}

void VideoPlayer::setPosition(int position)
{
    printf("\nPosition in setPosition is: %d", position);
    mediaPlayer.setPosition(position);
}

void VideoPlayer::handleError()
{
    playButton->setEnabled(false);
    const QString errorString = mediaPlayer.errorString();
    QString message = "Error: ";
    if (errorString.isEmpty())
        message += " #" + QString::number(int(mediaPlayer.error()));
    else
        message += errorString;
    errorLabel->setText(message);
}

void VideoPlayer::on_pushButton_clicked()
{
    ps_decoder_t *ps = nullptr;
        cmd_ln_t *config = nullptr;
        FILE *fh;
        char const *hyp;
        int16 buf[512];

        int32 score;
        ps_seg_t *seg;
        const char *segWord;
        int frameRate, startFrame, endFrame;

        config = cmd_ln_init(NULL, ps_args(), TRUE,
                             "-hmm", MODELDIR "/en-us/en-us",
                             "-lm", MODELDIR "/en-us/en-us.lm.bin",
                             "-dict", MODELDIR "/en-us/cmudict-en-us.dict",
                             NULL);
        if (!config) {
            fprintf(stderr, "Failed to create config object, see log for details\n");
            return;
        }

        ps = ps_init(config);
        if (!ps) {
            fprintf(stderr, "Failed to create recognizer, see log for details\n");
            return;
        }

        string avconvCommand = "./avconv -i " + v_url + " -ar 16000 -ac 1 tmp.wav";
        system(avconvCommand.c_str());
        fh = fopen("tmp.wav", "rb");
        if (!fh) {
            fprintf(stderr, "Unable to open file\n");
            return;
        }

        // Start decoding of the speech
        // Set to hot word listening mode
        ps_set_keyphrase(ps, "keyphrase_search", "box");
        ps_set_search(ps, "keyphrase_search");
        ps_start_utt(ps);


        // Read 512 samples at a time and feed them to the decoder
        while (!feof(fh)) {
            size_t nsamp = fread(buf, 2, 512, fh);
            ps_process_raw(ps, buf, nsamp, FALSE, FALSE);
        }

        // Mark end of utterance
        ps_end_utt(ps);

        seg = ps_seg_iter(ps);
        frameRate = cmd_ln_int32_r(config, "-frate");
        printf("Frame rate is %d\n", frameRate);

        while (seg) {
            segWord = ps_seg_word(seg);
            ps_seg_frames(seg, &startFrame, &endFrame);
            // The time computation is not exactly accurate, it's off by a few seconds
            cout << "Duration is " << v_dur << endl;
            int startTime = (startFrame / frameRate) - v_dur;
            int endTime = (endFrame / frameRate) - v_dur;
            printf("Found %s at time: %d-%d seconds\n\n\n", segWord, startTime, endTime);

            // Set the textbox
            string op = "Time 1 = " + std::to_string(startTime) + "\nTime 2 = " + std::to_string(endTime);
            QString qs = op.c_str();
            l1->setText(qs);

            seg = ps_seg_next(seg);
        }

        // Get recognition result
        hyp = ps_get_hyp(ps, &score);
        printf("Recognized: %s\n", hyp);

        // Release resources
        fclose(fh);
        cmd_ln_free_r(config);
        ps_free(ps);
}
