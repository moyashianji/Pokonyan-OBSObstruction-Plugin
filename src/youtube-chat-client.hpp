#pragma once

#include <QObject>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <string>
#include <functional>

enum class DonationType {
    SuperChat,
    SuperSticker
};

struct DonationEvent {
    DonationType type;
    double amount;          // Amount in JPY
    std::string displayName;
    std::string message;
    std::string currency;
};

using DonationCallback = std::function<void(const DonationEvent&)>;

class YouTubeChatClient : public QObject {
    Q_OBJECT

public:
    explicit YouTubeChatClient(QObject* parent = nullptr);
    ~YouTubeChatClient();

    void SetApiKey(const std::string& apiKey);
    void SetVideoId(const std::string& videoId);
    void SetDonationCallback(DonationCallback callback);

    void Start();
    void Stop();
    bool IsRunning() const { return m_isRunning; }

private slots:
    void PollChat();
    void OnChatDataReceived();
    void OnNetworkError(QNetworkReply::NetworkError error);

private:
    void FetchLiveChatId();
    void ProcessChatMessages(const QJsonArray& items);
    double ConvertCurrency(double amount, const std::string& currency);

    QNetworkAccessManager* m_networkManager;
    QTimer* m_pollTimer;

    std::string m_apiKey;
    std::string m_videoId;
    std::string m_liveChatId;
    std::string m_nextPageToken;

    DonationCallback m_donationCallback;
    bool m_isRunning;
    int m_pollIntervalMs;
};
