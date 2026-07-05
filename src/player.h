#pragma once

#include <QObject>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QVideoWidget>

class Player : public QObject {
    Q_OBJECT

public:
    explicit Player(QObject* parent = nullptr);
    ~Player();

    void play(const QString& url);
    void stop();
    void pause();
    void togglePause();

    void setVolume(int volume);
    int volume() const;

    void setPosition(qint64 position);
    qint64 position() const;
    qint64 duration() const;

    bool isPlaying() const;
    QVideoWidget* videoWidget() const;

signals:
    void positionChanged(qint64 position);
    void durationChanged(qint64 duration);
    void playbackStateChanged(QMediaPlayer::PlaybackState state);
    void mediaStatusChanged(QMediaPlayer::MediaStatus status);

private:
    QMediaPlayer* m_player;
    QAudioOutput* m_audioOutput;
    QVideoWidget* m_videoWidget;
};
