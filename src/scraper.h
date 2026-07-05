#pragma once

#include <QObject>
#include <QString>
#include <QList>
#include <functional>

struct Anime {
    QString title;
    QString url;
    QString thumbnail;
    QString rating;
    QString year;
};

struct Episode {
    QString title;
    QString url;
    QString thumbnail;
    QString quality;
};

struct VideoSource {
    QString label;
    QString iframeUrl;
};

class Scraper : public QObject {
    Q_OBJECT

public:
    explicit Scraper(QObject* parent = nullptr);

    void fetchHomeAnimes(std::function<void(QList<Anime>)> callback);
    void fetchRecentEpisodes(std::function<void(QList<Episode>)> callback);
    void fetchAnimeEpisodes(const QString& url, std::function<void(QList<Episode>)> callback);
    void searchAnime(const QString& query, std::function<void(QList<Anime>)> callback);
    void fetchVideoSources(const QString& url, std::function<void(QList<VideoSource>)> callback);
    void fetchAllAnimes(std::function<void(QList<Anime>)> callback);
    void fetchDubbedAnimes(std::function<void(QList<Anime>)> callback);
    void fetchSubtitledAnimes(std::function<void(QList<Anime>)> callback);

private:
    static constexpr const char* BASE_URL = "https://animesonlinecc.to";
    static constexpr const char* USER_AGENT = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36";

    QString fetchUrl(const QString& url);
    QList<Anime> parseAnimeList(const QString& html);
    QList<Episode> parseEpisodeList(const QString& html);
    QList<VideoSource> parseVideoSources(const QString& html);
};
