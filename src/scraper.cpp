#include "scraper.h"
#include <curl/curl.h>
#include <QThread>
#include <QRegularExpression>
#include <QCoreApplication>
#include <QFile>
#include <QTextStream>

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

static QString fixUrl(const QString& url, const char* base) {
    if (url.isEmpty()) return url;
    if (url.startsWith("http")) return url;
    return QString(base) + "/" + url;
}

static QString fullResUrl(const QString& url) {
    QString u = url;
    u.remove(QRegularExpression(QStringLiteral("-\\d+x\\d+(?=\\.\\w{3,4}$)")));
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

QString Scraper::resolveBloggerUrl(const QString& bloggerUrl) {
    QString html = fetchUrl(bloggerUrl);

    QRegularExpression googlevideoRe(QStringLiteral("\"(https?://[^\"]*\\.googlevideo\\.com/[^\"]*)\""));
    QRegularExpressionMatch match = googlevideoRe.match(html);
    if (match.hasMatch()) {
        return match.captured(1);
    }

    QRegularExpression urlRe(QStringLiteral("\"url\"\\s*:\\s*\"(https?://[^\"]*)\""));
    match = urlRe.match(html);
    if (match.hasMatch()) {
        QString url = match.captured(1);
        url.replace(QStringLiteral("\\u003d"), QStringLiteral("="));
        url.replace(QStringLiteral("\\u0026"), QStringLiteral("&"));
        return url;
    }

    QRegularExpression sourceRe(QStringLiteral("<source[^>]*src=\"([^\"]*)\""));
    match = sourceRe.match(html);
    if (match.hasMatch()) {
        return match.captured(1);
    }

    return bloggerUrl;
}

QList<Anime> Scraper::parseAnimeList(const QString& html) {
    QList<Anime> animes;

    QRegularExpression articleRe(
        QStringLiteral("<article[^>]*class=\"[^\"]*item[^\"]*tvshows[^\"]*\"[^>]*>(.*?)</article>"),
        QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatchIterator it = articleRe.globalMatch(html);

    QRegularExpression hrefRe(QStringLiteral("<a\\s+href=\"([^\"]*)\"[^>]*>"));
    QRegularExpression titleRe(QStringLiteral("<h3[^>]*>\\s*<a[^>]*>(.*?)</a>"), QRegularExpression::DotMatchesEverythingOption);
    QRegularExpression imgRe(QStringLiteral("<img\\s+src=\"([^\"]*)\""));
    QRegularExpression ratingRe(QStringLiteral("<span[^>]*class=\"icon-star2\"[^>]*></span>\\s*([\\d.]+)"));

    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString block = match.captured(1);

        QRegularExpressionMatch hrefMatch = hrefRe.match(block);
        QRegularExpressionMatch titleMatch = titleRe.match(block);
        QRegularExpressionMatch imgMatch = imgRe.match(block);
        QRegularExpressionMatch ratingMatch = ratingRe.match(block);

        if (hrefMatch.hasMatch()) {
            Anime anime;
            anime.url = fixUrl(hrefMatch.captured(1), BASE_URL);
            anime.title = titleMatch.hasMatch() ? titleMatch.captured(1).trimmed() : "";
            anime.thumbnail = fullResUrl(fixUrl(imgMatch.captured(1), BASE_URL));
            anime.rating = ratingMatch.hasMatch() ? ratingMatch.captured(1) : "";

            if (!anime.title.isEmpty()) {
                bool dup = false;
                for (const auto& a : animes) {
                    if (a.url == anime.url) { dup = true; break; }
                }
                if (!dup) animes.append(anime);
            }
        }
    }
    return animes;
}

QList<Episode> Scraper::parseEpisodeList(const QString& html) {
    QList<Episode> episodes;

    QRegularExpression articleRe(
        QStringLiteral("<article[^>]*class=\"[^\"]*item[^\"]*episodes[^\"]*\"[^>]*>(.*?)</article>"),
        QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatchIterator it = articleRe.globalMatch(html);

    QRegularExpression hrefRe(QStringLiteral("<a\\s+href=\"([^\"]*)\"[^>]*>"));
    QRegularExpression titleRe(QStringLiteral("<h3[^>]*>\\s*<a[^>]*>(.*?)</a>"), QRegularExpression::DotMatchesEverythingOption);
    QRegularExpression imgRe(QStringLiteral("<img\\s+src=\"([^\"]*)\""));
    QRegularExpression qualityRe(QStringLiteral("<span[^>]*class=\"quality\"[^>]*>(.*?)</span>"));

    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString block = match.captured(1);

        QRegularExpressionMatch hrefMatch = hrefRe.match(block);
        QRegularExpressionMatch titleMatch = titleRe.match(block);
        QRegularExpressionMatch imgMatch = imgRe.match(block);
        QRegularExpressionMatch qualityMatch = qualityRe.match(block);

        if (hrefMatch.hasMatch()) {
            Episode ep;
            ep.url = fixUrl(hrefMatch.captured(1), BASE_URL);
            ep.title = titleMatch.hasMatch() ? titleMatch.captured(1).trimmed() : "";
            ep.thumbnail = fullResUrl(fixUrl(imgMatch.captured(1), BASE_URL));
            ep.quality = qualityMatch.hasMatch() ? qualityMatch.captured(1).trimmed() : "";
            if (!ep.title.isEmpty()) episodes.append(ep);
        }
    }
    return episodes;
}

QList<VideoSource> Scraper::parseVideoSources(const QString& html) {
    QList<VideoSource> sources;

    QRegularExpression tabRe(
        QStringLiteral("<a[^>]*class=\"options\"[^>]*href=\"#(option-\\d+)\"[^>]*>(.*?)</a>"),
        QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatchIterator tabIt = tabRe.globalMatch(html);

    while (tabIt.hasNext()) {
        QRegularExpressionMatch tabMatch = tabIt.next();
        QString optionId = tabMatch.captured(1);
        QString label = tabMatch.captured(2).trimmed();
        label.remove(QRegularExpression(QStringLiteral("<[^>]*>")));

        QString pattern = optionId + QStringLiteral("[^>]*>.*?<iframe[^>]*src=\"([^\"]*)\"");
        QRegularExpression iframeRe(pattern, QRegularExpression::DotMatchesEverythingOption);
        QRegularExpressionMatch iframeMatch = iframeRe.match(html);

        if (iframeMatch.hasMatch()) {
            VideoSource source;
            source.label = label;
            source.iframeUrl = iframeMatch.captured(1);
            sources.append(source);
        }
    }

    if (sources.isEmpty()) {
        QRegularExpression iframeRe(
            QStringLiteral("<iframe[^>]*class=\"[^\"]*metaframe[^\"]*\"[^>]*src=\"([^\"]*)\""));
        QRegularExpressionMatchIterator iframeIt = iframeRe.globalMatch(html);
        int idx = 1;
        while (iframeIt.hasNext()) {
            QRegularExpressionMatch m = iframeIt.next();
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
        QString html = fetchUrl(url);
        QList<Episode> episodes = parseEpisodeList(html);
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
        QString html = fetchUrl(url);
        QList<VideoSource> sources = parseVideoSources(html);
        QMetaObject::invokeMethod(this, [callback, sources]() { callback(sources); });
    });
    thread->setParent(this);
    thread->start();
}

void Scraper::fetchAllAnimes(std::function<void(QList<Anime>)> callback) {
    QThread* thread = QThread::create([this, callback]() {
        scraperLog("fetchAllAnimes: fetching...");
        QString html = fetchUrl(QString(BASE_URL) + "/anime/");
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
        QString html = fetchUrl(QString(BASE_URL) + "/genero/dublado/");
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
        QString html = fetchUrl(QString(BASE_URL) + "/genero/legendado/");
        scraperLog("fetchSubtitledAnimes: html length = " + QString::number(html.length()));
        QList<Anime> animes = parseAnimeList(html);
        scraperLog("fetchSubtitledAnimes: found " + QString::number(animes.size()) + " animes");
        QMetaObject::invokeMethod(this, [callback, animes]() { callback(animes); });
    });
    thread->setParent(this);
    thread->start();
}
