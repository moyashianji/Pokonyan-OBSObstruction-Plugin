#include "comment-viewer-window.hpp"
#include "gl-widget.hpp"
#include "physics-engine.hpp"
#include "tcp-receiver.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QWidget>
#include <QMessageBox>

CommentViewerWindow::CommentViewerWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_glWidget(nullptr)
    , m_physicsEngine(nullptr)
    , m_tcpReceiver(nullptr)
    , m_updateTimer(nullptr)
    , m_testButton(nullptr)
    , m_testCommentCounter(0)
{
    SetupPhysics();
    SetupUI();
    SetupTCP();

    // Setup update timer for physics (60 FPS)
    m_updateTimer = new QTimer(this);
    connect(m_updateTimer, &QTimer::timeout, this, &CommentViewerWindow::OnUpdatePhysics);
    m_updateTimer->start(16); // ~60 FPS
}

CommentViewerWindow::~CommentViewerWindow() {
    if (m_updateTimer) {
        m_updateTimer->stop();
    }
    if (m_tcpReceiver) {
        m_tcpReceiver->Stop();
    }
    delete m_physicsEngine;
}

void CommentViewerWindow::SetupUI() {
    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // OpenGL widget for 3D rendering
    m_glWidget = new GLWidget(m_physicsEngine, this);
    mainLayout->addWidget(m_glWidget);

    // Control panel at bottom
    QWidget* controlPanel = new QWidget();
    controlPanel->setMaximumHeight(60);
    controlPanel->setStyleSheet("background-color: #1a1a1a; border-top: 1px solid #333;");

    QHBoxLayout* controlLayout = new QHBoxLayout(controlPanel);
    controlLayout->setContentsMargins(10, 10, 10, 10);

    QLabel* statusLabel = new QLabel("3D Super Chat Viewer - TCP Port: 45678");
    statusLabel->setStyleSheet("color: #00ff00; font-size: 10pt;");
    controlLayout->addWidget(statusLabel);

    controlLayout->addStretch();

    m_testButton = new QPushButton("テストコメント追加");
    m_testButton->setStyleSheet(R"(
        QPushButton {
            background-color: #2196F3;
            color: white;
            border: none;
            padding: 8px 16px;
            border-radius: 4px;
            font-size: 9pt;
            min-width: 120px;
        }
        QPushButton:hover {
            background-color: #1976D2;
        }
        QPushButton:pressed {
            background-color: #0D47A1;
        }
    )");
    connect(m_testButton, &QPushButton::clicked, this, &CommentViewerWindow::OnTestButtonClicked);
    controlLayout->addWidget(m_testButton);

    QPushButton* clearButton = new QPushButton("クリア");
    clearButton->setStyleSheet(R"(
        QPushButton {
            background-color: #f44336;
            color: white;
            border: none;
            padding: 8px 16px;
            border-radius: 4px;
            font-size: 9pt;
            min-width: 80px;
        }
        QPushButton:hover {
            background-color: #d32f2f;
        }
        QPushButton:pressed {
            background-color: #b71c1c;
        }
    )");
    connect(clearButton, &QPushButton::clicked, [this]() {
        if (m_physicsEngine) {
            m_physicsEngine->Clear();
            m_testCommentCounter = 0;
        }
    });
    controlLayout->addWidget(clearButton);

    mainLayout->addWidget(controlPanel);

    setCentralWidget(centralWidget);
}

void CommentViewerWindow::SetupPhysics() {
    m_physicsEngine = new PhysicsEngine();
    m_physicsEngine->Initialize();
}

void CommentViewerWindow::SetupTCP() {
    m_tcpReceiver = new TCPReceiver(45678, this);
    connect(m_tcpReceiver, &TCPReceiver::CommentReceived,
            this, &CommentViewerWindow::OnCommentReceived);

    if (!m_tcpReceiver->Start()) {
        QMessageBox::warning(this, "TCP Server Error",
                           "Failed to start TCP server on port 45678");
    }
}

void CommentViewerWindow::OnUpdatePhysics() {
    if (m_physicsEngine) {
        m_physicsEngine->Update(1.0f / 60.0f);
    }
    if (m_glWidget) {
        m_glWidget->update();
    }
}

void CommentViewerWindow::OnCommentReceived(const QString& text, double amount) {
    AddComment(text, amount);
}

void CommentViewerWindow::AddComment(const QString& text, double amount) {
    if (m_physicsEngine) {
        m_physicsEngine->AddComment(text, amount);
    }
}

void CommentViewerWindow::OnTestButtonClicked() {
    m_testCommentCounter++;
    QString testText = QString("テストコメント #%1").arg(m_testCommentCounter);
    double testAmount = 500.0 * m_testCommentCounter;
    AddComment(testText, testAmount);
}

void CommentViewerWindow::SpawnTestComment() {
    OnTestButtonClicked();
}
