#pragma once

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>

class TCPReceiver : public QObject {
    Q_OBJECT

public:
    explicit TCPReceiver(quint16 port = 45678, QObject* parent = nullptr);
    ~TCPReceiver();

    bool Start();
    void Stop();
    bool IsRunning() const { return m_server->isListening(); }

signals:
    void CommentReceived(const QString& text, double amount);

private slots:
    void OnNewConnection();
    void OnReadyRead();
    void OnDisconnected();

private:
    void ParseMessage(const QByteArray& data);

    QTcpServer* m_server;
    QList<QTcpSocket*> m_clients;
    quint16 m_port;
};
