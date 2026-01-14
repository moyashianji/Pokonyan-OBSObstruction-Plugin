#include "youtube-chat-client.hpp"
#include <obs-module.h>
#include <util/base.h>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrlQuery>

YouTubeChatClient::YouTubeChatClient(QObject* parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_pollTimer(new QTimer(this))
    , m_isRunning(false)
    , m_pollIntervalMs(5000)  // Poll every 5 seconds
{
    connect(m_pollTimer, &QTimer::timeout, this, &YouTubeChatClient::PollChat);
}

YouTubeChatClient::~YouTubeChatClient() {
    Stop();
}

void YouTubeChatClient::SetApiKey(const std::string& apiKey) {
    m_apiKey = apiKey;
}

void YouTubeChatClient::SetVideoId(const std::string& videoId) {
    m_videoId = videoId;
    m_liveChatId.clear();
    m_nextPageToken.clear();
}

void YouTubeChatClient::SetDonationCallback(DonationCallback callback) {
    m_donationCallback = callback;
}

void YouTubeChatClient::Start() {
    if (m_isRunning) return;

    if (m_apiKey.empty() || m_videoId.empty()) {
        blog(LOG_WARNING, "[YouTube Chat] Cannot start: API key or Video ID not set");
        return;
    }

    blog(LOG_INFO, "[YouTube Chat] Starting chat monitoring for video: %s", m_videoId.c_str());

    // First, get the live chat ID
    FetchLiveChatId();

    m_isRunning = true;
    m_pollTimer->start(m_pollIntervalMs);
}

void YouTubeChatClient::Stop() {
    if (!m_isRunning) return;

    blog(LOG_INFO, "[YouTube Chat] Stopping chat monitoring");
    m_pollTimer->stop();
    m_isRunning = false;
}

void YouTubeChatClient::FetchLiveChatId() {
    if (m_apiKey.empty() || m_videoId.empty()) return;

    // Get video details to extract liveChatId
    QString url = QString("https://www.googleapis.com/youtube/v3/videos?part=liveStreamingDetails&id=%1&key=%2")
                      .arg(QString::fromStdString(m_videoId))
                      .arg(QString::fromStdString(m_apiKey));

    QNetworkRequest request(url);
    QNetworkReply* reply = m_networkManager->get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray data = reply->readAll();
            QJsonDocument doc = QJsonDocument::fromJson(data);
            QJsonObject obj = doc.object();

            if (obj.contains("items") && obj["items"].toArray().size() > 0) {
                QJsonObject item = obj["items"].toArray()[0].toObject();
                if (item.contains("liveStreamingDetails")) {
                    QJsonObject liveDetails = item["liveStreamingDetails"].toObject();
                    if (liveDetails.contains("activeLiveChatId")) {
                        m_liveChatId = liveDetails["activeLiveChatId"].toString().toStdString();
                        blog(LOG_INFO, "[YouTube Chat] Live chat ID: %s", m_liveChatId.c_str());
                    } else {
                        blog(LOG_WARNING, "[YouTube Chat] No active live chat found");
                    }
                }
            }
        } else {
            blog(LOG_ERROR, "[YouTube Chat] Failed to fetch live chat ID: %s",
                    reply->errorString().toStdString().c_str());
        }
        reply->deleteLater();
    });
}

void YouTubeChatClient::PollChat() {
    if (m_liveChatId.empty()) {
        // Try to fetch live chat ID again
        FetchLiveChatId();
        return;
    }

    // Build URL with pagination token
    QString url = QString("https://www.googleapis.com/youtube/v3/liveChat/messages?liveChatId=%1&part=snippet,authorDetails&key=%2")
                      .arg(QString::fromStdString(m_liveChatId))
                      .arg(QString::fromStdString(m_apiKey));

    if (!m_nextPageToken.empty()) {
        url += QString("&pageToken=%1").arg(QString::fromStdString(m_nextPageToken));
    }

    QNetworkRequest request(url);
    QNetworkReply* reply = m_networkManager->get(request);

    connect(reply, &QNetworkReply::finished, this, &YouTubeChatClient::OnChatDataReceived);
    connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::errorOccurred),
            this, &YouTubeChatClient::OnNetworkError);
}

void YouTubeChatClient::OnChatDataReceived() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;

    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        QJsonObject obj = doc.object();

        // Update next page token
        if (obj.contains("nextPageToken")) {
            m_nextPageToken = obj["nextPageToken"].toString().toStdString();
        }

        // Update polling interval based on API response
        if (obj.contains("pollingIntervalMillis")) {
            m_pollIntervalMs = obj["pollingIntervalMillis"].toInt();
            m_pollTimer->setInterval(m_pollIntervalMs);
        }

        // Process messages
        if (obj.contains("items")) {
            ProcessChatMessages(obj["items"].toArray());
        }
    }

    reply->deleteLater();
}

void YouTubeChatClient::OnNetworkError(QNetworkReply::NetworkError error) {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (reply) {
        blog(LOG_ERROR, "[YouTube Chat] Network error: %s", reply->errorString().toStdString().c_str());
    }
}

void YouTubeChatClient::ProcessChatMessages(const QJsonArray& items) {
    blog(LOG_INFO, "[YouTube Chat] Processing %d messages", items.size());

    for (const QJsonValue& value : items) {
        QJsonObject message = value.toObject();
        QJsonObject snippet = message["snippet"].toObject();
        QJsonObject authorDetails = message["authorDetails"].toObject();

        // Log message type for debugging
        QString messageType = message["snippet"].toObject()["type"].toString();
        blog(LOG_DEBUG, "[YouTube Chat] Message type: %s", messageType.toStdString().c_str());

        // Check if it's a super chat or super sticker
        if (snippet.contains("superChatDetails")) {
            QJsonObject superChat = snippet["superChatDetails"].toObject();

            DonationEvent event;
            event.type = DonationType::SuperChat;
            event.displayName = authorDetails["displayName"].toString().toStdString();
            // SuperChat message can be in displayMessage or superChatDetails.userComment
            event.message = snippet.contains("displayMessage")
                ? snippet["displayMessage"].toString().toStdString()
                : superChat["userComment"].toString().toStdString();

            // Parse amount
            if (superChat.contains("amountMicros")) {
                double micros = superChat["amountMicros"].toString().toDouble();
                event.amount = micros / 1000000.0;
            }

            event.currency = superChat.value("currency").toString().toStdString();

            // Convert to JPY for consistent processing
            event.amount = ConvertCurrency(event.amount, event.currency);

            blog(LOG_INFO, "[YouTube Chat] SuperChat from %s: ¥%.0f - %s",
                event.displayName.c_str(), event.amount, event.message.c_str());

            if (m_donationCallback) {
                m_donationCallback(event);
            }
        }
        else if (snippet.contains("superStickerDetails")) {
            QJsonObject superSticker = snippet["superStickerDetails"].toObject();

            DonationEvent event;
            event.type = DonationType::SuperSticker;
            event.displayName = authorDetails["displayName"].toString().toStdString();
            event.message = "";

            // Parse amount
            if (superSticker.contains("amountMicros")) {
                double micros = superSticker["amountMicros"].toString().toDouble();
                event.amount = micros / 1000000.0;
            }

            event.currency = superSticker.value("currency").toString().toStdString();

            // Convert to JPY
            event.amount = ConvertCurrency(event.amount, event.currency);

            blog(LOG_INFO, "[YouTube Chat] SuperSticker from %s: ¥%.0f",
                event.displayName.c_str(), event.amount);

            if (m_donationCallback) {
                m_donationCallback(event);
            }
        }
        else if (snippet.contains("textMessageDetails")) {
            // Regular chat message - treat as low value super chat for obstruction effects
            QJsonObject textMessageDetails = snippet["textMessageDetails"].toObject();

            DonationEvent event;
            event.type = DonationType::SuperChat;
            event.displayName = authorDetails["displayName"].toString().toStdString();
            event.message = textMessageDetails["messageText"].toString().toStdString();
            event.amount = 100.0;  // Treat as 100 JPY for obstruction effect
            event.currency = "JPY";

            blog(LOG_INFO, "[YouTube Chat] Regular chat from %s: %s",
                event.displayName.c_str(), event.message.c_str());

            if (m_donationCallback) {
                m_donationCallback(event);
            }
        }
    }
}

double YouTubeChatClient::ConvertCurrency(double amount, const std::string& currency) {
    // Simple currency conversion (in production, use real exchange rates)
    // These are approximate rates for demonstration
    if (currency == "JPY") {
        return amount;
    } else if (currency == "USD") {
        return amount * 150.0;  // 1 USD = 150 JPY
    } else if (currency == "EUR") {
        return amount * 160.0;  // 1 EUR = 160 JPY
    } else if (currency == "GBP") {
        return amount * 190.0;  // 1 GBP = 190 JPY
    }

    // Default: assume USD
    return amount * 150.0;
}
