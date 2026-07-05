#pragma once

#include <QMainWindow>
#include <QStackedWidget>
#include <QScrollArea>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QSlider>
#include <QFrame>
#include <QVector>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>

#include "scraper.h"
#include "player.h"

class WebView2Widget;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void onSearch();
    void onNavClicked(const QString& key);
    void onAnimeClicked(const Anime& anime);
    void onEpisodeClicked(const Episode& episode);
    void onSourceChanged(int index);
    void onPlayPause();
    void onRewind();
    void onForward();
    void onVolumeChanged(int value);
    void onMuteToggle();
    void onSeek(int value);
    void onBackFromPlayer();
    void onOpenInBrowser();
    void onToggleFullscreen();

protected:
    void keyPressEvent(QKeyEvent* event) override;

private:
    void setupUi();
    QWidget* createSidebar();
    QWidget* createContent();
    QWidget* createHomePage();
    QWidget* createAnimePage();
    QWidget* createPlayerPage();
    QWidget* createSearchPage();

    void loadHome();
    void loadNavData(const QString& key);
    void loadAnimeEpisodes(const QString& url);
    void loadEpisodeSources(const QString& url);
    void updateHome(const QList<Episode>& episodes, const QList<Anime>& animes);
    void showAnimeEpisodes(const QList<Episode>& episodes);
    void showEpisodeSources(const QList<VideoSource>& sources);
    void showSearchResults(const QList<Anime>& results);

    QWidget* createAnimeCard(const Anime& anime);
    QWidget* createEpisodeCard(const Episode& episode);

    void downloadImage(const QString& url, QLabel* label, QSize size);

    QStackedWidget* m_stacked;
    QLineEdit* m_searchInput;
    QScrollArea* m_recentScroll;
    QScrollArea* m_popularScroll;
    QScrollArea* m_episodesScroll;
    QScrollArea* m_searchResultsScroll;
    QWidget* m_recentContainer;
    QWidget* m_popularContainer;
    QWidget* m_episodesContainer;
    QWidget* m_searchResultsContainer;
    QLabel* m_animeCover;
    QLabel* m_animeDetailTitle;
    QLabel* m_animeDetailInfo;
    QLabel* m_playerEpisodeTitle;
    QComboBox* m_sourceCombo;
    QSlider* m_progressSlider;
    QSlider* m_volumeSlider;
    QLabel* m_timeLabel;
    QLabel* m_durationLabel;
    QPushButton* m_btnPlay;
    QPushButton* m_btnMute;
    QPushButton* m_btnFullscreen;
    QLabel* m_searchTitle;

    Scraper* m_scraper;
    Player* m_player;

    Anime m_currentAnime;
    Episode m_currentEpisode;
    QList<VideoSource> m_currentSources;

    QNetworkAccessManager* m_networkManager;
    QMap<QString, QPixmap> m_imageCache;
    WebView2Widget* m_webview = nullptr;
};
