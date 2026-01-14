#include "tcp-receiver.hpp"
#include <QJsonDocument>
#include <QJsonObject>

TCPReceiver::TCPReceiver(quint16 port, QObject* parent)
    : QObject(parent)
    , m_server(nullptr)
    , m_port(port)
{
    m_server = new QTcpServer(this);
    connect(m_server, &QTcpServer::newConnection, this, &TCPReceiver::OnNewConnection);
}

TCPReceiver::~TCPReceiver() {
    Stop();
}

bool TCPReceiver::Start() {
    if (m_server->isListening()) {
        return true;
    }

    if (m_server->listen(QHostAddress::Any, m_port)) {
        qDebug() << "[TCPReceiver] Server started on port" << m_port;
        return true;
    } else {
        qDebug() << "[TCPReceiver] Failed to start server:" << m_server->errorString();
        return false;
    }
}

void TCPReceiver::Stop() {
    if (m_server->isListening()) {
        m_server->close();
    }

    for (QTcpSocket* client : m_clients) {
        client->disconnectFromHost();
        client->deleteLater();
    }
    m_clients.clear();
}

void TCPReceiver::OnNewConnection() {
    QTcpSocket* client = m_server->nextPendingConnection();
    if (client) {
        qDebug() << "[TCPReceiver] New connection from" << client->peerAddress().toString();

        connect(client, &QTcpSocket::readyRead, this, &TCPReceiver::OnReadyRead);
        connect(client, &QTcpSocket::disconnected, this, &TCPReceiver::OnDisconnected);

        m_clients.append(client);
    }
}

void TCPReceiver::OnReadyRead() {
    QTcpSocket* client = qobject_cast<QTcpSocket*>(sender());
    if (!client) {
        return;
    }

    while (client->canReadLine()) {
        QByteArray data = client->readLine().trimmed();
        if (!data.isEmpty()) {
            ParseMessage(data);
        }
    }
}

void TCPReceiver::OnDisconnected() {
    QTcpSocket* client = qobject_cast<QTcpSocket*>(sender());
    if (client) {
        qDebug() << "[TCPReceiver] Client disconnected:" << client->peerAddress().toString();
        m_clients.removeOne(client);
        client->deleteLater();
    }
}

void TCPReceiver::ParseMessage(const QByteArray& data) {
    // Expected JSON format: {"text": "comment text", "amount": 1000.0}
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        qDebug() << "[TCPReceiver] Invalid JSON:" << data;
        return;
    }

    QJsonObject obj = doc.object();
    QString text = obj["text"].toString();
    double amount = obj["amount"].toDouble();

    if (!text.isEmpty()) {
        qDebug() << "[TCPReceiver] Received comment:" << text << "Amount:" << amount;
        emit CommentReceived(text, amount);
    }
}
