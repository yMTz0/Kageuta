#include "mainwindow.h"
#include "styles.h"
#include "clickableframe.h"

#include <QApplication>
#include <QDesktopServices>
#include <QUrl>
#include <QImage>
#include <QPixmap>
#include <QBuffer>
#include <QSplitter>
#include <QScrollBar>
#include <QGraphicsDropShadowEffect>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("Kageuta");
    setMinimumSize(1200, 750);
    resize(1400, 850);

    setWindowIcon(QIcon(":/icon.png"));

    m_scraper = new Scraper(this);
    m_player = new Player(this);
    m_networkManager = new QNetworkAccessManager(this);

    setupUi();
    loadHome();
}

MainWindow::~MainWindow() {}

void MainWindow::setupUi() {
    QWidget* central = new QWidget(this);
    central->setObjectName("centralWidget");
    setCentralWidget(central);

    QHBoxLayout* mainLayout = new QHBoxLayout(central);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    QSplitter* splitter = new QSplitter(Qt::Horizontal);
    splitter->setHandleWidth(1);

    splitter->addWidget(createSidebar());
    splitter->addWidget(createContent());
    splitter->setSizes({220, 1180});

    mainLayout->addWidget(splitter);
}

QWidget* MainWindow::createSidebar() {
    QWidget* sidebar = new QWidget();
    sidebar->setObjectName("sidebar");
    sidebar->setFixedWidth(220);

    QVBoxLayout* layout = new QVBoxLayout(sidebar);
    layout->setContentsMargins(12, 16, 12, 16);
    layout->setSpacing(4);

    QHBoxLayout* logoLayout = new QHBoxLayout();
    logoLayout->setSpacing(8);

    QLabel* logoIcon = new QLabel();
    logoIcon->setPixmap(QPixmap(":/icon.png").scaled(32, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    logoIcon->setFixedSize(32, 32);
    logoLayout->addWidget(logoIcon);

    QLabel* logoText = new QLabel("KAGEUTA");
    logoText->setStyleSheet(QString("color: %1; font-size: 16px; font-weight: bold; letter-spacing: 3px;").arg(Styles::ACCENT));
    logoLayout->addWidget(logoText);
    logoLayout->addStretch();
    layout->addLayout(logoLayout);
    layout->addSpacing(16);

    m_searchInput = new QLineEdit();
    m_searchInput->setObjectName("searchInput");
    m_searchInput->setPlaceholderText("Pesquisar animes...");
    connect(m_searchInput, &QLineEdit::returnPressed, this, &MainWindow::onSearch);
    layout->addWidget(m_searchInput);
    layout->addSpacing(12);

    QVector<QPair<QString, QString>> navItems = {
        {"\U0001f3e0  Inicio", "home"},
        {"\U0001f525  Populares", "popular"},
        {"\U0001f399\ufe0f  Dublados", "dubbed"},
        {"\U0001f4dd  Legendados", "subtitled"},
        {"\U0001f4cb  Todos", "all"}
    };

    for (const auto& [label, key] : navItems) {
        QPushButton* btn = new QPushButton(label);
        btn->setObjectName("navButton");
        btn->setCheckable(true);
        connect(btn, &QPushButton::clicked, this, [this, key]() { onNavClicked(key); });
        layout->addWidget(btn);
    }

    layout->addStretch();
    return sidebar;
}

QWidget* MainWindow::createContent() {
    QWidget* content = new QWidget();
    content->setObjectName("contentArea");

    QVBoxLayout* layout = new QVBoxLayout(content);
    layout->setContentsMargins(20, 16, 20, 16);
    layout->setSpacing(16);

    m_stacked = new QStackedWidget();
    m_stacked->addWidget(createHomePage());
    m_stacked->addWidget(createAnimePage());
    m_stacked->addWidget(createPlayerPage());
    m_stacked->addWidget(createSearchPage());

    layout->addWidget(m_stacked);
    return content;
}

QWidget* MainWindow::createHomePage() {
    QWidget* page = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(20);

    QLabel* header = new QLabel("KAGEUTA");
    header->setObjectName("headerTitle");
    layout->addWidget(header);

    QLabel* subtitle = new QLabel("Assista seus animes favoritos de forma gratuita");
    subtitle->setStyleSheet(QString("color: %1; font-size: 13px; margin-bottom: 8px;").arg(Styles::TEXT_SECONDARY));
    layout->addWidget(subtitle);

    QLabel* recentLabel = new QLabel("Ultimos Episodios");
    recentLabel->setObjectName("sectionTitle");
    layout->addWidget(recentLabel);

    m_recentScroll = new QScrollArea();
    m_recentScroll->setWidgetResizable(true);
    m_recentScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_recentScroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_recentScroll->setFixedHeight(200);
    m_recentContainer = new QWidget();
    m_recentScroll->setWidget(m_recentContainer);
    layout->addWidget(m_recentScroll);

    QLabel* popularLabel = new QLabel("Animes Populares");
    popularLabel->setObjectName("sectionTitle");
    layout->addWidget(popularLabel);

    m_popularScroll = new QScrollArea();
    m_popularScroll->setWidgetResizable(true);
    m_popularScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_popularScroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_popularContainer = new QWidget();
    m_popularScroll->setWidget(m_popularContainer);
    layout->addWidget(m_popularScroll);

    layout->addStretch();
    return page;
}

QWidget* MainWindow::createAnimePage() {
    QWidget* page = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(16);

    QHBoxLayout* headerLayout = new QHBoxLayout();
    QPushButton* backBtn = new QPushButton("\u2190 Voltar");
    backBtn->setObjectName("actionButton");
    connect(backBtn, &QPushButton::clicked, this, [this]() { m_stacked->setCurrentIndex(0); });
    headerLayout->addWidget(backBtn);
    headerLayout->addStretch();
    layout->addLayout(headerLayout);

    QHBoxLayout* infoLayout = new QHBoxLayout();
    infoLayout->setSpacing(20);

    m_animeCover = new QLabel();
    m_animeCover->setFixedSize(200, 290);
    m_animeCover->setAlignment(Qt::AlignCenter);
    m_animeCover->setStyleSheet(QString("border-radius: 10px; background-color: %1;").arg(Styles::BG_HOVER));
    infoLayout->addWidget(m_animeCover);

    QVBoxLayout* detailsLayout = new QVBoxLayout();
    detailsLayout->setSpacing(8);

    m_animeDetailTitle = new QLabel();
    m_animeDetailTitle->setObjectName("headerTitle");
    m_animeDetailTitle->setWordWrap(true);
    detailsLayout->addWidget(m_animeDetailTitle);

    m_animeDetailInfo = new QLabel();
    m_animeDetailInfo->setStyleSheet(QString("color: %1; font-size: 12px;").arg(Styles::TEXT_SECONDARY));
    m_animeDetailInfo->setWordWrap(true);
    detailsLayout->addWidget(m_animeDetailInfo);

    detailsLayout->addStretch();
    infoLayout->addLayout(detailsLayout, 1);
    layout->addLayout(infoLayout);

    QLabel* episodesLabel = new QLabel("Episodios");
    episodesLabel->setObjectName("sectionTitle");
    layout->addWidget(episodesLabel);

    m_episodesScroll = new QScrollArea();
    m_episodesScroll->setWidgetResizable(true);
    m_episodesContainer = new QWidget();
    m_episodesScroll->setWidget(m_episodesContainer);
    layout->addWidget(m_episodesScroll);

    return page;
}

QWidget* MainWindow::createPlayerPage() {
    QWidget* page = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    QHBoxLayout* headerLayout = new QHBoxLayout();
    headerLayout->setContentsMargins(0, 0, 0, 8);

    QPushButton* playerBackBtn = new QPushButton("\u2190 Voltar");
    playerBackBtn->setObjectName("actionButton");
    connect(playerBackBtn, &QPushButton::clicked, this, &MainWindow::onBackFromPlayer);
    headerLayout->addWidget(playerBackBtn);

    m_playerEpisodeTitle = new QLabel();
    m_playerEpisodeTitle->setStyleSheet(QString("color: %1; font-size: 14px; font-weight: bold;").arg(Styles::TEXT_PRIMARY));
    headerLayout->addWidget(m_playerEpisodeTitle);
    headerLayout->addStretch();

    QLabel* sourceLabel = new QLabel("Fonte:");
    sourceLabel->setStyleSheet(QString("color: %1; font-size: 12px;").arg(Styles::TEXT_SECONDARY));
    headerLayout->addWidget(sourceLabel);

    m_sourceCombo = new QComboBox();
    m_sourceCombo->setObjectName("sourceCombo");
    connect(m_sourceCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onSourceChanged);
    headerLayout->addWidget(m_sourceCombo);

    layout->addLayout(headerLayout);

    QWidget* playerArea = new QWidget();
    QVBoxLayout* playerLayout = new QVBoxLayout(playerArea);
    playerLayout->setContentsMargins(0, 0, 0, 0);

    m_player->videoWidget()->setStyleSheet("background-color: #000; border-radius: 8px;");
    m_player->videoWidget()->setMinimumHeight(400);
    playerLayout->addWidget(m_player->videoWidget(), 1);

    QWidget* controlsWidget = new QWidget();
    controlsWidget->setStyleSheet(QString("background-color: %1; border-radius: 0 0 8px 8px;").arg(Styles::BG_SECONDARY));
    QVBoxLayout* controlsLayout = new QVBoxLayout(controlsWidget);
    controlsLayout->setContentsMargins(12, 8, 12, 8);
    controlsLayout->setSpacing(6);

    QHBoxLayout* progressLayout = new QHBoxLayout();
    progressLayout->setSpacing(8);

    m_timeLabel = new QLabel("00:00");
    m_timeLabel->setStyleSheet(QString("color: %1; font-size: 11px;").arg(Styles::TEXT_SECONDARY));
    m_timeLabel->setFixedWidth(50);
    progressLayout->addWidget(m_timeLabel);

    m_progressSlider = new QSlider(Qt::Horizontal);
    m_progressSlider->setObjectName("progressSlider");
    m_progressSlider->setRange(0, 1000);
    connect(m_progressSlider, &QSlider::sliderMoved, this, &MainWindow::onSeek);
    progressLayout->addWidget(m_progressSlider, 1);

    m_durationLabel = new QLabel("00:00");
    m_durationLabel->setStyleSheet(QString("color: %1; font-size: 11px;").arg(Styles::TEXT_SECONDARY));
    m_durationLabel->setFixedWidth(50);
    m_durationLabel->setAlignment(Qt::AlignRight);
    progressLayout->addWidget(m_durationLabel);

    controlsLayout->addLayout(progressLayout);

    QHBoxLayout* buttonsLayout = new QHBoxLayout();
    buttonsLayout->setSpacing(4);

    m_btnPlay = new QPushButton("\u25b6");
    m_btnPlay->setObjectName("controlButton");
    m_btnPlay->setFixedSize(44, 44);
    m_btnPlay->setStyleSheet(QString("QPushButton { background-color: %1; border-radius: 22px; font-size: 18px; } QPushButton:hover { background-color: %2; }").arg(Styles::ACCENT, Styles::ACCENT_HOVER));
    connect(m_btnPlay, &QPushButton::clicked, this, &MainWindow::onPlayPause);
    buttonsLayout->addWidget(m_btnPlay);

    QPushButton* btnRewind = new QPushButton("\u23ea");
    btnRewind->setObjectName("controlButton");
    btnRewind->setFixedSize(36, 36);
    connect(btnRewind, &QPushButton::clicked, this, &MainWindow::onRewind);
    buttonsLayout->addWidget(btnRewind);

    QPushButton* btnForward = new QPushButton("\u23e9");
    btnForward->setObjectName("controlButton");
    btnForward->setFixedSize(36, 36);
    connect(btnForward, &QPushButton::clicked, this, &MainWindow::onForward);
    buttonsLayout->addWidget(btnForward);

    buttonsLayout->addStretch();

    m_btnMute = new QPushButton("\U0001f50a");
    m_btnMute->setObjectName("controlButton");
    m_btnMute->setFixedSize(36, 36);
    connect(m_btnMute, &QPushButton::clicked, this, &MainWindow::onMuteToggle);
    buttonsLayout->addWidget(m_btnMute);

    m_volumeSlider = new QSlider(Qt::Horizontal);
    m_volumeSlider->setObjectName("volumeSlider");
    m_volumeSlider->setRange(0, 100);
    m_volumeSlider->setValue(80);
    m_volumeSlider->setFixedWidth(100);
    connect(m_volumeSlider, &QSlider::sliderMoved, this, &MainWindow::onVolumeChanged);
    buttonsLayout->addWidget(m_volumeSlider);

    controlsLayout->addLayout(buttonsLayout);
    playerLayout->addWidget(controlsWidget);

    layout->addWidget(playerArea, 1);

    connect(m_player, &Player::positionChanged, this, [this](qint64 pos) {
        if (!m_progressSlider->isSliderDown() && m_player->duration() > 0) {
            m_progressSlider->blockSignals(true);
            m_progressSlider->setValue(static_cast<int>(pos * 1000 / m_player->duration()));
            m_progressSlider->blockSignals(false);

            int s = static_cast<int>(pos / 1000);
            int m = s / 60; s %= 60;
            int h = m / 60; m %= 60;
            m_timeLabel->setText(h > 0 ? QString("%1:%2:%3").arg(h).arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0'))
                                      : QString("%1:%2").arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0')));
        }
    });

    connect(m_player, &Player::durationChanged, this, [this](qint64 dur) {
        int s = static_cast<int>(dur / 1000);
        int m = s / 60; s %= 60;
        int h = m / 60; m %= 60;
        m_durationLabel->setText(h > 0 ? QString("%1:%2:%3").arg(h).arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0'))
                                       : QString("%1:%2").arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0')));
    });

    connect(m_player, &Player::playbackStateChanged, this, [this](QMediaPlayer::PlaybackState state) {
        m_btnPlay->setText(state == QMediaPlayer::PlayingState ? "\u23f8" : "\u25b6");
    });

    return page;
}

QWidget* MainWindow::createSearchPage() {
    QWidget* page = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(16);

    m_searchTitle = new QLabel("Resultados da Pesquisa");
    m_searchTitle->setObjectName("headerTitle");
    layout->addWidget(m_searchTitle);

    m_searchResultsScroll = new QScrollArea();
    m_searchResultsScroll->setWidgetResizable(true);
    m_searchResultsContainer = new QWidget();
    m_searchResultsScroll->setWidget(m_searchResultsContainer);
    layout->addWidget(m_searchResultsScroll);

    return page;
}

QWidget* MainWindow::createAnimeCard(const Anime& anime) {
    ClickableFrame* card = new ClickableFrame();
    card->setObjectName("animeCard");
    card->setFixedSize(160, 260);

    QVBoxLayout* layout = new QVBoxLayout(card);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(6);

    QLabel* imageLabel = new QLabel();
    imageLabel->setFixedSize(144, 200);
    imageLabel->setAlignment(Qt::AlignCenter);
    imageLabel->setStyleSheet(QString("border-radius: 6px; background-color: %1;").arg(Styles::BG_HOVER));
    imageLabel->setText("...");
    layout->addWidget(imageLabel, 0, Qt::AlignCenter);

    QLabel* titleLabel = new QLabel(anime.title);
    titleLabel->setObjectName("animeTitle");
    titleLabel->setWordWrap(true);
    titleLabel->setMaximumHeight(32);
    layout->addWidget(titleLabel);

    if (!anime.rating.isEmpty()) {
        QLabel* ratingLabel = new QLabel("\u2605 " + anime.rating);
        ratingLabel->setObjectName("ratingLabel");
        layout->addWidget(ratingLabel);
    }

    if (!anime.thumbnail.isEmpty()) {
        downloadImage(anime.thumbnail, imageLabel, QSize(144, 200));
    }

    connect(card, &ClickableFrame::clicked, this, [this, anime]() { onAnimeClicked(anime); });

    return card;
}

QWidget* MainWindow::createEpisodeCard(const Episode& episode) {
    ClickableFrame* card = new ClickableFrame();
    card->setObjectName("episodeCard");
    card->setFixedHeight(70);

    QHBoxLayout* layout = new QHBoxLayout(card);
    layout->setContentsMargins(10, 8, 10, 8);
    layout->setSpacing(12);

    QLabel* imageLabel = new QLabel();
    imageLabel->setFixedSize(100, 56);
    imageLabel->setAlignment(Qt::AlignCenter);
    imageLabel->setStyleSheet(QString("border-radius: 4px; background-color: %1;").arg(Styles::BG_HOVER));
    imageLabel->setText("\u25b6");
    layout->addWidget(imageLabel);

    QVBoxLayout* infoLayout = new QVBoxLayout();
    infoLayout->setSpacing(4);

    QLabel* titleLabel = new QLabel(episode.title);
    titleLabel->setObjectName("episodeTitle");
    titleLabel->setWordWrap(true);
    infoLayout->addWidget(titleLabel);

    if (!episode.quality.isEmpty()) {
        QLabel* qualityLabel = new QLabel(episode.quality);
        qualityLabel->setObjectName("qualityBadge");
        qualityLabel->setFixedWidth(80);
        qualityLabel->setAlignment(Qt::AlignCenter);
        infoLayout->addWidget(qualityLabel, 0, Qt::AlignLeft);
    }

    layout->addLayout(infoLayout, 1);

    if (!episode.thumbnail.isEmpty()) {
        downloadImage(episode.thumbnail, imageLabel, QSize(100, 56));
    }

    connect(card, &ClickableFrame::clicked, this, [this, episode]() { onEpisodeClicked(episode); });

    return card;
}

void MainWindow::downloadImage(const QString& url, QLabel* label, QSize size) {
    if (m_imageCache.contains(url)) {
        label->setPixmap(m_imageCache[url].scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        return;
    }

    QNetworkRequest request{QUrl(url)};
    QNetworkReply* reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply, label, size, url]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray data = reply->readAll();
            QImage img;
            img.loadFromData(data);
            if (!img.isNull()) {
                QPixmap pixmap = QPixmap::fromImage(img).scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                m_imageCache[url] = pixmap;
                label->setPixmap(pixmap);
            }
        }
        reply->deleteLater();
    });
}

void MainWindow::onSearch() {
    QString query = m_searchInput->text().trimmed();
    if (query.isEmpty()) return;

    m_stacked->setCurrentIndex(3);
    m_searchTitle->setText("Resultados para: " + query);

    QLayoutItem* item;
    while ((item = m_searchResultsContainer->layout()->takeAt(0)) != nullptr) {
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }

    m_scraper->searchAnime(query, [this](const QList<Anime>& results) {
        showSearchResults(results);
    });
}

void MainWindow::onNavClicked(const QString& key) {
    m_stacked->setCurrentIndex(0);
    loadNavData(key);
}

void MainWindow::loadNavData(const QString& key) {
    if (key == "home") {
        loadHome();
    } else if (key == "popular") {
        m_scraper->fetchAllAnimes([this](const QList<Anime>& animes) {
            QList<Episode> empty;
            updateHome(empty, animes);
        });
    } else if (key == "dubbed") {
        m_scraper->fetchDubbedAnimes([this](const QList<Anime>& animes) {
            QList<Episode> empty;
            updateHome(empty, animes);
        });
    } else if (key == "subtitled") {
        m_scraper->fetchSubtitledAnimes([this](const QList<Anime>& animes) {
            QList<Episode> empty;
            updateHome(empty, animes);
        });
    } else if (key == "all") {
        m_scraper->fetchAllAnimes([this](const QList<Anime>& animes) {
            QList<Episode> empty;
            updateHome(empty, animes);
        });
    }
}

void MainWindow::loadHome() {
    m_scraper->fetchRecentEpisodes([this](const QList<Episode>& episodes) {
        m_scraper->fetchHomeAnimes([this, episodes](const QList<Anime>& animes) {
            updateHome(episodes, animes);
        });
    });
}

void MainWindow::updateHome(const QList<Episode>& episodes, const QList<Anime>& animes) {
    m_stacked->setCurrentIndex(0);

    QLayoutItem* item;
    while ((item = m_recentContainer->layout()->takeAt(0)) != nullptr) {
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }
    if (!m_recentContainer->layout()) {
        m_recentContainer->setLayout(new QHBoxLayout());
        m_recentContainer->layout()->setSpacing(12);
        m_recentContainer->layout()->setAlignment(Qt::AlignLeft);
    }

    for (const auto& ep : episodes) {
        m_recentContainer->layout()->addWidget(createEpisodeCard(ep));
    }

    while ((item = m_popularContainer->layout()->takeAt(0)) != nullptr) {
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }
    if (!m_popularContainer->layout()) {
        m_popularContainer->setLayout(new QHBoxLayout());
        m_popularContainer->layout()->setSpacing(12);
        m_popularContainer->layout()->setAlignment(Qt::AlignLeft);
    }

    for (const auto& anime : animes) {
        m_popularContainer->layout()->addWidget(createAnimeCard(anime));
    }
}

void MainWindow::onAnimeClicked(const Anime& anime) {
    m_currentAnime = anime;
    m_stacked->setCurrentIndex(1);

    m_animeDetailTitle->setText(anime.title);
    m_animeDetailInfo->setText("Fonte: " + anime.url);

    if (!anime.thumbnail.isEmpty()) {
        downloadImage(anime.thumbnail, m_animeCover, QSize(200, 290));
    }

    m_scraper->fetchAnimeEpisodes(anime.url, [this](const QList<Episode>& episodes) {
        showAnimeEpisodes(episodes);
    });
}

void MainWindow::showAnimeEpisodes(const QList<Episode>& episodes) {
    QLayoutItem* item;
    while ((item = m_episodesContainer->layout()->takeAt(0)) != nullptr) {
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }
    if (!m_episodesContainer->layout()) {
        m_episodesContainer->setLayout(new QVBoxLayout());
        m_episodesContainer->layout()->setSpacing(6);
        m_episodesContainer->layout()->setAlignment(Qt::AlignTop);
    }

    for (const auto& ep : episodes) {
        m_episodesContainer->layout()->addWidget(createEpisodeCard(ep));
    }
}

void MainWindow::onEpisodeClicked(const Episode& episode) {
    m_currentEpisode = episode;
    m_stacked->setCurrentIndex(2);
    m_playerEpisodeTitle->setText(episode.title);

    m_scraper->fetchVideoSources(episode.url, [this](const QList<VideoSource>& sources) {
        showEpisodeSources(sources);
    });
}

void MainWindow::showEpisodeSources(const QList<VideoSource>& sources) {
    m_sourceCombo->blockSignals(true);
    m_sourceCombo->clear();
    m_currentSources = sources;

    for (const auto& source : sources) {
        m_sourceCombo->addItem(source.label);
    }

    m_sourceCombo->blockSignals(false);

    if (!sources.isEmpty()) {
        onSourceChanged(0);
    }
}

void MainWindow::onSourceChanged(int index) {
    if (index < 0 || index >= m_currentSources.size()) return;

    const VideoSource& source = m_currentSources[index];
    QString url = source.iframeUrl;

    if (url.contains("blogger.com") || url.contains(".googlevideo.com") == false) {
        QDesktopServices::openUrl(QUrl(url));
        return;
    }

    m_player->play(url);
}

void MainWindow::onPlayPause() {
    m_player->togglePause();
}

void MainWindow::onRewind() {
    m_player->setPosition(qMax(0LL, m_player->position() - 10000));
}

void MainWindow::onForward() {
    m_player->setPosition(m_player->position() + 10000);
}

void MainWindow::onVolumeChanged(int value) {
    m_player->setVolume(value);
    if (value == 0) m_btnMute->setText("\U0001f507");
    else if (value < 50) m_btnMute->setText("\U0001f509");
    else m_btnMute->setText("\U0001f50a");
}

void MainWindow::onMuteToggle() {
    int vol = m_volumeSlider->value();
    if (vol > 0) {
        m_volumeSlider->setValue(0);
        m_player->setVolume(0);
        m_btnMute->setText("\U0001f507");
    } else {
        m_volumeSlider->setValue(80);
        m_player->setVolume(80);
        m_btnMute->setText("\U0001f50a");
    }
}

void MainWindow::onSeek(int value) {
    if (m_player->duration() > 0) {
        m_player->setPosition(value * m_player->duration() / 1000);
    }
}

void MainWindow::onBackFromPlayer() {
    m_player->stop();
    m_stacked->setCurrentIndex(1);
}

void MainWindow::showSearchResults(const QList<Anime>& results) {
    QLayoutItem* item;
    while ((item = m_searchResultsContainer->layout()->takeAt(0)) != nullptr) {
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }
    if (!m_searchResultsContainer->layout()) {
        m_searchResultsContainer->setLayout(new QGridLayout());
        m_searchResultsContainer->layout()->setSpacing(12);
    }

    QGridLayout* grid = qobject_cast<QGridLayout*>(m_searchResultsContainer->layout());
    int idx = 0;
    for (const auto& anime : results) {
        grid->addWidget(createAnimeCard(anime), idx / 6, idx % 6);
        idx++;
    }
}
