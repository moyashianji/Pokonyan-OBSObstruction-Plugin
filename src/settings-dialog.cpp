#include "settings-dialog.hpp"
#include "plugin-main.hpp"
#include "youtube-chat-client.hpp"
#include "obstruction-manager.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QMessageBox>

extern std::unique_ptr<YouTubeChatClient> g_chatClient;
extern std::unique_ptr<ObstructionManager> g_obstructionManager;
extern PluginSettings g_settings;

SettingsDialog::SettingsDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("YouTube SuperChat Plugin Settings");
    setMinimumWidth(600);
    SetupUI();
}

SettingsDialog::~SettingsDialog() {
}

void SettingsDialog::SetupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // YouTube API Settings
    QGroupBox* apiGroup = new QGroupBox("YouTube API Settings");
    QFormLayout* apiLayout = new QFormLayout();

    m_apiKeyEdit = new QLineEdit();
    m_apiKeyEdit->setEchoMode(QLineEdit::Password);
    m_apiKeyEdit->setPlaceholderText("Enter your YouTube Data API v3 key");
    apiLayout->addRow("API Key:", m_apiKeyEdit);

    m_videoIdEdit = new QLineEdit();
    m_videoIdEdit->setPlaceholderText("YouTube video ID (e.g., dQw4w9WgXcQ)");
    apiLayout->addRow("Video ID:", m_videoIdEdit);

    apiGroup->setLayout(apiLayout);
    mainLayout->addWidget(apiGroup);

    // OBS Settings
    QGroupBox* obsGroup = new QGroupBox("OBS Settings");
    QFormLayout* obsLayout = new QFormLayout();

    m_mainSourceEdit = new QLineEdit();
    m_mainSourceEdit->setPlaceholderText("Name of the main source to shrink/expand");
    obsLayout->addRow("Main Source:", m_mainSourceEdit);

    obsGroup->setLayout(obsLayout);
    mainLayout->addWidget(obsGroup);

    // Effect Settings
    QGroupBox* effectGroup = new QGroupBox("Effect Settings");
    QFormLayout* effectLayout = new QFormLayout();

    m_enableObstructionsCheck = new QCheckBox("Enable Obstructions (Super Chat)");
    m_enableObstructionsCheck->setChecked(true);
    effectLayout->addRow(m_enableObstructionsCheck);

    m_obstructionIntensitySpin = new QDoubleSpinBox();
    m_obstructionIntensitySpin->setRange(0.1, 3.0);
    m_obstructionIntensitySpin->setSingleStep(0.1);
    m_obstructionIntensitySpin->setValue(1.0);
    m_obstructionIntensitySpin->setToolTip("Multiplier for obstruction effects (higher = stronger effects)");
    effectLayout->addRow("Obstruction Intensity:", m_obstructionIntensitySpin);

    m_enableRecoveryCheck = new QCheckBox("Enable Recovery (Super Sticker)");
    m_enableRecoveryCheck->setChecked(true);
    effectLayout->addRow(m_enableRecoveryCheck);

    m_recoveryIntensitySpin = new QDoubleSpinBox();
    m_recoveryIntensitySpin->setRange(0.1, 3.0);
    m_recoveryIntensitySpin->setSingleStep(0.1);
    m_recoveryIntensitySpin->setValue(1.0);
    m_recoveryIntensitySpin->setToolTip("Multiplier for recovery effects (higher = stronger recovery)");
    effectLayout->addRow("Recovery Intensity:", m_recoveryIntensitySpin);

    effectGroup->setLayout(effectLayout);
    mainLayout->addWidget(effectGroup);

    // Status
    QGroupBox* statusGroup = new QGroupBox("Status");
    QVBoxLayout* statusLayout = new QVBoxLayout();

    m_statusLabel = new QLabel("Not monitoring");
    m_statusLabel->setStyleSheet("QLabel { padding: 5px; background-color: #f0f0f0; }");
    statusLayout->addWidget(m_statusLabel);

    statusGroup->setLayout(statusLayout);
    mainLayout->addWidget(statusGroup);

    // Control Buttons
    QHBoxLayout* controlLayout = new QHBoxLayout();

    m_testButton = new QPushButton("Test Connection");
    m_startButton = new QPushButton("Start Monitoring");
    m_stopButton = new QPushButton("Stop Monitoring");
    m_stopButton->setEnabled(false);

    controlLayout->addWidget(m_testButton);
    controlLayout->addWidget(m_startButton);
    controlLayout->addWidget(m_stopButton);

    mainLayout->addLayout(controlLayout);

    // Dialog Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    m_saveButton = new QPushButton("Save");
    m_cancelButton = new QPushButton("Cancel");

    buttonLayout->addWidget(m_saveButton);
    buttonLayout->addWidget(m_cancelButton);

    mainLayout->addLayout(buttonLayout);

    // Connect signals
    connect(m_testButton, &QPushButton::clicked, this, &SettingsDialog::OnTestConnectionClicked);
    connect(m_startButton, &QPushButton::clicked, this, &SettingsDialog::OnStartMonitoringClicked);
    connect(m_stopButton, &QPushButton::clicked, this, &SettingsDialog::OnStopMonitoringClicked);
    connect(m_saveButton, &QPushButton::clicked, this, &SettingsDialog::OnSaveClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &SettingsDialog::OnCancelClicked);
}

void SettingsDialog::LoadSettings() {
    m_apiKeyEdit->setText(QString::fromStdString(g_settings.youtubeApiKey));
    m_videoIdEdit->setText(QString::fromStdString(g_settings.videoId));
    m_enableObstructionsCheck->setChecked(g_settings.enableObstructions);
    m_enableRecoveryCheck->setChecked(g_settings.enableRecovery);
    m_obstructionIntensitySpin->setValue(g_settings.obstructionIntensity);
    m_recoveryIntensitySpin->setValue(g_settings.recoveryIntensity);

    UpdateMonitoringState();
}

void SettingsDialog::SaveSettings() {
    g_settings.youtubeApiKey = m_apiKeyEdit->text().toStdString();
    g_settings.videoId = m_videoIdEdit->text().toStdString();
    g_settings.enableObstructions = m_enableObstructionsCheck->isChecked();
    g_settings.enableRecovery = m_enableRecoveryCheck->isChecked();
    g_settings.obstructionIntensity = m_obstructionIntensitySpin->value();
    g_settings.recoveryIntensity = m_recoveryIntensitySpin->value();

    ::SaveSettings();

    // Update managers
    if (g_chatClient) {
        g_chatClient->SetApiKey(g_settings.youtubeApiKey);
        g_chatClient->SetVideoId(g_settings.videoId);
    }

    if (g_obstructionManager) {
        g_obstructionManager->SetEnabled(g_settings.enableObstructions || g_settings.enableRecovery);

        std::string mainSource = m_mainSourceEdit->text().toStdString();
        if (!mainSource.empty()) {
            g_obstructionManager->SetMainSourceName(mainSource);
        }
    }
}

void SettingsDialog::OnSaveClicked() {
    SaveSettings();
    QMessageBox::information(this, "Settings Saved", "Settings have been saved successfully.");
    accept();
}

void SettingsDialog::OnCancelClicked() {
    reject();
}

void SettingsDialog::OnTestConnectionClicked() {
    if (m_apiKeyEdit->text().isEmpty() || m_videoIdEdit->text().isEmpty()) {
        QMessageBox::warning(this, "Missing Information",
                           "Please enter both API Key and Video ID.");
        return;
    }

    QMessageBox::information(this, "Test Connection",
                           "Connection test is not yet implemented.\n\n"
                           "To test, click 'Start Monitoring' and check OBS logs.");
}

void SettingsDialog::OnStartMonitoringClicked() {
    if (m_apiKeyEdit->text().isEmpty() || m_videoIdEdit->text().isEmpty()) {
        QMessageBox::warning(this, "Missing Information",
                           "Please enter both API Key and Video ID before starting.");
        return;
    }

    // Save current settings first
    SaveSettings();

    // Start monitoring
    if (g_chatClient) {
        g_chatClient->Start();
        UpdateMonitoringState();

        m_statusLabel->setText("Monitoring active - Waiting for donations...");
        m_statusLabel->setStyleSheet("QLabel { padding: 5px; background-color: #90EE90; }");
        m_startButton->setEnabled(false);
        m_stopButton->setEnabled(true);
    }
}

void SettingsDialog::OnStopMonitoringClicked() {
    if (g_chatClient) {
        g_chatClient->Stop();
        UpdateMonitoringState();

        m_statusLabel->setText("Not monitoring");
        m_statusLabel->setStyleSheet("QLabel { padding: 5px; background-color: #f0f0f0; }");
        m_startButton->setEnabled(true);
        m_stopButton->setEnabled(false);
    }
}

void SettingsDialog::UpdateMonitoringState() {
    if (g_chatClient && g_chatClient->IsRunning()) {
        m_startButton->setEnabled(false);
        m_stopButton->setEnabled(true);
        m_statusLabel->setText("Monitoring active");
        m_statusLabel->setStyleSheet("QLabel { padding: 5px; background-color: #90EE90; }");
    } else {
        m_startButton->setEnabled(true);
        m_stopButton->setEnabled(false);
        m_statusLabel->setText("Not monitoring");
        m_statusLabel->setStyleSheet("QLabel { padding: 5px; background-color: #f0f0f0; }");
    }
}
