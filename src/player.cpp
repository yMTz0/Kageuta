#include "player.h"

Player::Player(QObject* parent) : QObject(parent) {
    m_player = new QMediaPlayer(this);
    m_audioOutput = new QAudioOutput(this);
    m_videoWidget = new QVideoWidget();

    m_player->setAudioOutput(m_audioOutput);
    m_player->setVideoOutput(m_videoWidget);

    m_audioOutput->setVolume(0.8f);

    connect(m_player, &QMediaPlayer::positionChanged, this, &Player::positionChanged);
    connect(m_player, &QMediaPlayer::durationChanged, this, &Player::durationChanged);
    connect(m_player, &QMediaPlayer::playbackStateChanged, this, &Player::playbackStateChanged);
    connect(m_player, &QMediaPlayer::mediaStatusChanged, this, &Player::mediaStatusChanged);
}

Player::~Player() {
    delete m_videoWidget;
}

void Player::play(const QString& url) {
    m_player->setSource(QUrl(url));
    m_player->play();
}

void Player::stop() {
    m_player->stop();
}

void Player::pause() {
    m_player->pause();
}

void Player::togglePause() {
    if (m_player->playbackState() == QMediaPlayer::PlayingState) {
        m_player->pause();
    } else {
        m_player->play();
    }
}

void Player::setVolume(int volume) {
    m_audioOutput->setVolume(volume / 100.0f);
}

int Player::volume() const {
    return static_cast<int>(m_audioOutput->volume() * 100);
}

void Player::setPosition(qint64 position) {
    m_player->setPosition(position);
}

qint64 Player::position() const {
    return m_player->position();
}

qint64 Player::duration() const {
    return m_player->duration();
}

bool Player::isPlaying() const {
    return m_player->playbackState() == QMediaPlayer::PlayingState;
}

QVideoWidget* Player::videoWidget() const {
    return m_videoWidget;
}
