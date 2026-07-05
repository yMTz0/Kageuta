#include "scraper.h"
#include <curl/curl.h>
#include <QThread>
#include <QRegularExpression>
#include <QCoreApplication>
#include <QFile>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

static void scraperLog(const QString& msg) {
    QFile f(QCoreApplication::applicationDirPath() + "/debug.log");
    if (f.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream ts(&f);
        ts << "[Scraper] " << msg << "\n";
    }
}

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

static QString fullResUrl(const QString& url) {
    QString u = url;
    u.remove(QRegularExpression(QStringLiteral("\\?resize=\\d+,\\d+")));
    u.remove(QRegularExpression(QStringLiteral("\\?w=\\d+&h=\\d+")));
    return u;
}

Scraper::Scraper(QObject* parent) : QObject(parent) {
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

QString Scraper::fetchUrl(const QString& url) {
    CURL* curl = curl_easy_init();
    std::string response;
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.toStdString().c_str());
        curl_easy_setopt(curl, CURLOPT_USERAGENT, USER_AGENT);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) response = "";
        curl_easy_cleanup(curl);
    }
    return QString::fromStdString(response);
}

QList<Anime> Scraper::parseAnimeList(const QString& html) {
    QList<Anime> animes;

    QRegularExpression articleRe(
        QStringLiteral("<article\\s+class=\"bs\"[^>]*>.*?<a\\s+href=\"([^\"]*?)\"[^>]*>.*?<img\\s+src=\"([^\"]*?)\"[^>]*>.*?<div\\s+class=\"tt\">\\s*(.*?)\\s*<h2"),
        QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatchIterator it = articleRe.globalMatch(html);

    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        Anime anime;
        anime.url = match.captured(1);
        anime.thumbnail = fullResUrl(match.captured(2));
        QString titleBlock = match.captured(3).trimmed();
        titleBlock.remove(QRegularExpression(QStringLiteral("<[^>]*>")));
        titleBlock = titleBlock.trimmed();
        anime.title = titleBlock;

        if (!anime.title.isEmpty() && !anime.url.isEmpty()) {
            bool dup = false;
            for (const auto& a : animes) {
                if (a.url == anime.url) { dup = true; break; }
            }
            if (!dup) animes.append(anime);
        }
    }
    return animes;
}

QList<Episode> Scraper::parseEpisodeList(const QString& html) {
    QList<Episode> episodes;

    QRegularExpression articleRe(
        QStringLiteral("<article\\s+class=\"bs\"[^>]*>.*?<a\\s+href=\"([^\"]*?)\"[^>]*title=\"([^\"]*?)\"[^>]*>.*?<span\\s+class=\"epx\">(.*?)</span>.*?<span\\s+class=\"sb\\s+(Sub|Dub)\"[^>]*>(.*?)</span>.*?<img\\s+src=\"([^\"]*?)\""),
        QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatchIterator it = articleRe.globalMatch(html);

    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        Episode ep;
        ep.url = match.captured(1);
        ep.title = match.captured(2);
        QString quality = match.captured(4).trimmed();
        ep.quality = quality.isEmpty() ? match.captured(5).trimmed() : quality;
        ep.thumbnail = fullResUrl(match.captured(6));
        if (!ep.title.isEmpty()) episodes.append(ep);
    }

    if (episodes.isEmpty()) {
        QRegularExpression simpleRe(
            QStringLiteral("<article\\s+class=\"bs\"[^>]*>.*?<a\\s+href=\"([^\"]*?)\"[^>]*>.*?<div\\s+class=\"tt\">\\s*(.*?)\\s*<h2\\s+itemprop=\"headline\">(.*?)</h2>.*?<img\\s+src=\"([^\"]*?)\""),
            QRegularExpression::DotMatchesEverythingOption);
        QRegularExpressionMatchIterator sit = simpleRe.globalMatch(html);
        while (sit.hasNext()) {
            QRegularExpressionMatch m = sit.next();
            Episode ep;
            ep.url = m.captured(1);
            QString plainTitle = m.captured(2).trimmed();
            plainTitle.remove(QRegularExpression(QStringLiteral("<[^>]*>")));
            ep.title = plainTitle.isEmpty() ? m.captured(3).trimmed() : plainTitle;
            ep.thumbnail = fullResUrl(m.captured(4));
            if (!ep.title.isEmpty()) episodes.append(ep);
        }
    }
    return episodes;
}

QList<VideoSource> Scraper::parseVideoSources(const QString& html) {
    QList<VideoSource> sources;

    QRegularExpression iframeRe(
        QStringLiteral("<iframe[^>]*src=\"([^\"]*?)\"[^>]*>"));
    QRegularExpressionMatchIterator it = iframeRe.globalMatch(html);
    int idx = 1;
    while (it.hasNext()) {
        QRegularExpressionMatch m = it.next();
        QString src = m.captured(1);
        if (src.contains("blogger.com") || src.contains("video.g") ||
            src.contains("streamtape") || src.contains("sbplay") ||
            src.contains("doodstream") || src.contains("mp4upload") ||
            src.contains("embed") || src.contains("filemoon") ||
            src.contains("vidhide") || src.contains("streamvid")) {
            VideoSource source;
            source.label = QString("Fonte %1").arg(idx++);
            source.iframeUrl = src;
            sources.append(source);
        }
    }

    if (sources.isEmpty()) {
        QRegularExpression allIframeRe(QStringLiteral("<iframe[^>]*src=\"([^\"]*?)\""));
        QRegularExpressionMatchIterator ait = allIframeRe.globalMatch(html);
        while (ait.hasNext()) {
            QRegularExpressionMatch m = ait.next();
            VideoSource source;
            source.label = QString("Fonte %1").arg(idx++);
            source.iframeUrl = m.captured(1);
            sources.append(source);
        }
    }
    return sources;
}

void Scraper::fetchHomeAnimes(std::function<void(QList<Anime>)> callback) {
    QThread* thread = QThread::create([this, callback]() {
        scraperLog("fetchHomeAnimes: fetching...");
        QString html = fetchUrl(QString(BASE_URL));
        scraperLog("fetchHomeAnimes: html length = " + QString::number(html.length()));
        QList<Anime> animes = parseAnimeList(html);
        scraperLog("fetchHomeAnimes: found " + QString::number(animes.size()) + " animes");
        QMetaObject::invokeMethod(this, [callback, animes]() { callback(animes); });
    });
    thread->setParent(this);
    thread->start();
}

void Scraper::fetchRecentEpisodes(std::function<void(QList<Episode>)> callback) {
    QThread* thread = QThread::create([this, callback]() {
        scraperLog("fetchRecentEpisodes: fetching...");
        QString html = fetchUrl(QString(BASE_URL));
        scraperLog("fetchRecentEpisodes: html length = " + QString::number(html.length()));
        QList<Episode> episodes = parseEpisodeList(html);
        scraperLog("fetchRecentEpisodes: found " + QString::number(episodes.size()) + " episodes");
        QMetaObject::invokeMethod(this, [callback, episodes]() { callback(episodes); });
    });
    thread->setParent(this);
    thread->start();
}

void Scraper::fetchAnimeEpisodes(const QString& url, std::function<void(QList<Episode>)> callback) {
    QThread* thread = QThread::create([this, url, callback]() {
        scraperLog("fetchAnimeEpisodes: " + url);
        QString html = fetchUrl(url);
        scraperLog("fetchAnimeEpisodes: html length = " + QString::number(html.length()));
        QList<Episode> episodes = parseEpisodeList(html);
        scraperLog("fetchAnimeEpisodes: found " + QString::number(episodes.size()) + " episodes");
        QMetaObject::invokeMethod(this, [callback, episodes]() { callback(episodes); });
    });
    thread->setParent(this);
    thread->start();
}

void Scraper::searchAnime(const QString& query, std::function<void(QList<Anime>)> callback) {
    QThread* thread = QThread::create([this, query, callback]() {
        QString html = fetchUrl(QString(BASE_URL) + "/?s=" + query);
        QList<Anime> animes = parseAnimeList(html);
        QMetaObject::invokeMethod(this, [callback, animes]() { callback(animes); });
    });
    thread->setParent(this);
    thread->start();
}

void Scraper::fetchVideoSources(const QString& url, std::function<void(QList<VideoSource>)> callback) {
    QThread* thread = QThread::create([this, url, callback]() {
        scraperLog("fetchVideoSources: " + url);
        QString html = fetchUrl(url);
        scraperLog("fetchVideoSources: html length = " + QString::number(html.length()));
        QList<VideoSource> sources = parseVideoSources(html);
        scraperLog("fetchVideoSources: found " + QString::number(sources.size()) + " sources");
        QMetaObject::invokeMethod(this, [callback, sources]() { callback(sources); });
    });
    thread->setParent(this);
    thread->start();
}

void Scraper::fetchAllAnimes(std::function<void(QList<Anime>)> callback) {
    QThread* thread = QThread::create([this, callback]() {
        scraperLog("fetchAllAnimes: fetching...");
        QString html = fetchUrl(QString(BASE_URL) + "/anime");
        scraperLog("fetchAllAnimes: html length = " + QString::number(html.length()));
        QList<Anime> animes = parseAnimeList(html);
        scraperLog("fetchAllAnimes: found " + QString::number(animes.size()) + " animes");
        QMetaObject::invokeMethod(this, [callback, animes]() { callback(animes); });
    });
    thread->setParent(this);
    thread->start();
}

void Scraper::fetchDubbedAnimes(std::function<void(QList<Anime>)> callback) {
    QThread* thread = QThread::create([this, callback]() {
        scraperLog("fetchDubbedAnimes: fetching...");
        QString html = fetchUrl(QString(BASE_URL) + "/anime/?sub=dub");
        scraperLog("fetchDubbedAnimes: html length = " + QString::number(html.length()));
        QList<Anime> animes = parseAnimeList(html);
        scraperLog("fetchDubbedAnimes: found " + QString::number(animes.size()) + " animes");
        QMetaObject::invokeMethod(this, [callback, animes]() { callback(animes); });
    });
    thread->setParent(this);
    thread->start();
}

void Scraper::fetchSubtitledAnimes(std::function<void(QList<Anime>)> callback) {
    QThread* thread = QThread::create([this, callback]() {
        scraperLog("fetchSubtitledAnimes: fetching...");
        QString html = fetchUrl(QString(BASE_URL) + "/anime/?sub=sub");
        scraperLog("fetchSubtitledAnimes: html length = " + QString::number(html.length()));
        QList<Anime> animes = parseAnimeList(html);
        scraperLog("fetchSubtitledAnimes: found " + QString::number(animes.size()) + " animes");
        QMetaObject::invokeMethod(this, [callback, animes]() { callback(animes); });
    });
    thread->setParent(this);
    thread->start();
}

void Scraper::fetchJikanImage(const QString& title, std::function<void(const QString&)> callback) {
    QThread* thread = QThread::create([this, title, callback]() {
        QString cleanTitle = title;
        cleanTitle.remove(QRegularExpression(QStringLiteral("\\s*(Dublado|Legendado|\\d+p|720p|1080p)\\s*")));
        QString searchUrl = "https://api.jikan.moe/v4/anime?q=" + QUrl::toPercentEncoding(cleanTitle) + "&limit=1";
        scraperLog("fetchJikanImage: " + searchUrl);
        QString json = fetchUrl(searchUrl);
        QString imageUrl;
        QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8());
        if (doc.isObject()) {
            QJsonObject root = doc.object();
            QJsonArray data = root["data"].toArray();
            if (!data.isEmpty()) {
                QJsonObject anime = data[0].toObject();
                QJsonObject images = anime["images"].toObject();
                QJsonObject jpg = images["jpg"].toObject();
                imageUrl = jpg["large_image_url"].toString();
                if (imageUrl.isEmpty()) imageUrl = jpg["image_url"].toString();
            }
        }
        scraperLog("fetchJikanImage: result = " + imageUrl.left(100));
        QMetaObject::invokeMethod(this, [callback, imageUrl]() { callback(imageUrl); });
    });
    thread->setParent(this);
    thread->start();
}
