#pragma once

#include <QMainWindow>
#include <QTimer>
#include <memory>

class GLWidget;
class PhysicsEngine;
class TCPReceiver;
class QPushButton;

class CommentViewerWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit CommentViewerWindow(QWidget* parent = nullptr);
    ~CommentViewerWindow();

    void AddComment(const QString& text, double amount);
    void SpawnTestComment();

private slots:
    void OnUpdatePhysics();
    void OnCommentReceived(const QString& text, double amount);
    void OnTestButtonClicked();

private:
    void SetupUI();
    void SetupPhysics();
    void SetupTCP();

    GLWidget* m_glWidget;
    PhysicsEngine* m_physicsEngine;
    TCPReceiver* m_tcpReceiver;
    QTimer* m_updateTimer;
    QPushButton* m_testButton;

    int m_testCommentCounter;
};
