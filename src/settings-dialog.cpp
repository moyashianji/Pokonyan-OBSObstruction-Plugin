#include "settings-dialog.hpp"
#include "effect-config-manager.hpp"
#include "effect-config.hpp"
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
    setMinimumSize(900, 700);
    resize(1000, 800);
    SetupUI();
}

SettingsDialog::~SettingsDialog() {
}

void SettingsDialog::SetupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // Apply modern styling with dark theme
    QString styleSheet = R"(
        QDialog {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #2c3e50, stop:1 #34495e);
        }

        QWidget {
            color: #ecf0f1;
        }

        QTabWidget::pane {
            border: 2px solid #34495e;
            background-color: #2c3e50;
            border-radius: 8px;
        }

        QTabBar::tab {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #34495e, stop:1 #2c3e50);
            color: #bdc3c7;
            padding: 12px 24px;
            margin-right: 2px;
            border-top-left-radius: 8px;
            border-top-right-radius: 8px;
            font-weight: bold;
            font-size: 11pt;
            border: 1px solid #1a252f;
        }

        QTabBar::tab:selected {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #3498db, stop:1 #2980b9);
            color: white;
            border-bottom: 3px solid #1abc9c;
        }

        QTabBar::tab:hover:!selected {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #3d5a73, stop:1 #34495e);
            color: #ecf0f1;
        }

        QGroupBox {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #34495e, stop:1 #2c3e50);
            border: 2px solid #1abc9c;
            border-radius: 8px;
            margin-top: 12px;
            padding-top: 16px;
            font-weight: bold;
            font-size: 8pt;
            color: #1abc9c;
        }

        QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top left;
            left: 15px;
            top: 5px;
            padding: 0 8px;
            color: #1abc9c;
            background-color: #2c3e50;
        }

        QLabel {
            color: #ecf0f1;
            font-size: 8pt;
        }

        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #3498db, stop:1 #2980b9);
            color: white;
            border: none;
            padding: 8px 16px;
            border-radius: 6px;
            font-weight: bold;
            font-size: 8pt;
            min-width: 90px;
        }

        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #5dade2, stop:1 #3498db);
        }

        QPushButton:pressed {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #2980b9, stop:1 #21618c);
        }

        QPushButton:disabled {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #7f8c8d, stop:1 #5d6d7e);
            color: #95a5a6;
        }

        QPushButton#testButton,
        QPushButton#testObstructionButton {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #2ecc71, stop:1 #27ae60);
        }

        QPushButton#testButton:hover,
        QPushButton#testObstructionButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #58d68d, stop:1 #2ecc71);
        }

        QPushButton#stopButton {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #e74c3c, stop:1 #c0392b);
        }

        QPushButton#stopButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #ec7063, stop:1 #e74c3c);
        }

        QPushButton#saveButton {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #1abc9c, stop:1 #16a085);
            padding: 12px 36px;
            font-size: 10pt;
        }

        QPushButton#saveButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #48c9b0, stop:1 #1abc9c);
        }

        QPushButton#cancelButton {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #95a5a6, stop:1 #7f8c8d);
            padding: 12px 36px;
            font-size: 10pt;
        }

        QPushButton#cancelButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #bdc3c7, stop:1 #95a5a6);
        }

        QLineEdit, QSpinBox, QDoubleSpinBox {
            padding: 6px;
            border: 2px solid #34495e;
            border-radius: 4px;
            background-color: #1a252f;
            color: #ecf0f1;
            selection-background-color: #3498db;
            selection-color: white;
            font-size: 8pt;
        }

        QLineEdit:focus, QSpinBox:focus, QDoubleSpinBox:focus {
            border: 2px solid #1abc9c;
            background-color: #21313f;
        }

        QLineEdit::placeholder {
            color: #7f8c8d;
        }

        QSpinBox::up-button, QDoubleSpinBox::up-button {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #3498db, stop:1 #2980b9);
            border-top-right-radius: 4px;
        }

        QSpinBox::down-button, QDoubleSpinBox::down-button {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #3498db, stop:1 #2980b9);
            border-bottom-right-radius: 4px;
        }

        QCheckBox {
            spacing: 8px;
            color: #ecf0f1;
            font-size: 9pt;
        }

        QCheckBox::indicator {
            width: 20px;
            height: 20px;
            border-radius: 4px;
            border: 2px solid #34495e;
            background-color: #1a252f;
        }

        QCheckBox::indicator:hover {
            border: 2px solid #1abc9c;
        }

        QCheckBox::indicator:checked {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #1abc9c, stop:1 #16a085);
            border-color: #1abc9c;
        }

        QLabel#statusLabel {
            padding: 10px 16px;
            border-radius: 6px;
            font-weight: bold;
            font-size: 10pt;
        }

        QComboBox {
            padding: 6px;
            border: 2px solid #34495e;
            border-radius: 6px;
            background-color: #1a252f;
            color: #ecf0f1;
            font-size: 9pt;
        }

        QComboBox:focus {
            border: 2px solid #1abc9c;
        }

        QComboBox::drop-down {
            border: none;
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #3498db, stop:1 #2980b9);
        }

        QComboBox QAbstractItemView {
            background-color: #2c3e50;
            color: #ecf0f1;
            selection-background-color: #3498db;
            border: 2px solid #1abc9c;
        }
    )";

    setStyleSheet(styleSheet);

    // Create tab widget
    m_tabWidget = new QTabWidget();

    // ===== Tab 1: Basic Settings =====
    QWidget* basicTab = new QWidget();
    QVBoxLayout* basicLayout = new QVBoxLayout(basicTab);

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
    basicLayout->addWidget(apiGroup);

    // OBS Settings
    QGroupBox* obsGroup = new QGroupBox("OBS Settings");
    QFormLayout* obsLayout = new QFormLayout();

    m_mainSourceEdit = new QLineEdit();
    m_mainSourceEdit->setPlaceholderText("Name of the main source to shrink/expand");
    obsLayout->addRow("Main Source:", m_mainSourceEdit);

    obsGroup->setLayout(obsLayout);
    basicLayout->addWidget(obsGroup);

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
    basicLayout->addWidget(effectGroup);

    // Status
    QGroupBox* statusGroup = new QGroupBox("Status");
    QVBoxLayout* statusLayout = new QVBoxLayout();

    m_statusLabel = new QLabel("モニタリング停止中");
    m_statusLabel->setObjectName("statusLabel");
    m_statusLabel->setStyleSheet("background-color: #34495e; color: #95a5a6; border: 2px solid #7f8c8d;");
    statusLayout->addWidget(m_statusLabel);

    statusGroup->setLayout(statusLayout);
    basicLayout->addWidget(statusGroup);

    // Test Features
    QGroupBox* testGroup = new QGroupBox("Test Effects (No API Required)");
    QFormLayout* testLayout = new QFormLayout();

    m_testAmountSpin = new QDoubleSpinBox();
    m_testAmountSpin->setRange(100, 50000);
    m_testAmountSpin->setValue(5000);
    m_testAmountSpin->setSuffix(" JPY");
    m_testAmountSpin->setToolTip("Amount to test (in Japanese Yen)");
    testLayout->addRow("Test Amount:", m_testAmountSpin);

    QHBoxLayout* testButtonLayout = new QHBoxLayout();
    m_testObstructionButton = new QPushButton("テスト実行 (SuperChat)");
    m_testObstructionButton->setObjectName("testObstructionButton");
    m_testRecoveryButton = new QPushButton("テスト実行 (SuperSticker)");
    m_testRecoveryButton->setStyleSheet("background-color: #FF9800;");
    testButtonLayout->addWidget(m_testObstructionButton);
    testButtonLayout->addWidget(m_testRecoveryButton);
    testLayout->addRow(testButtonLayout);

    testGroup->setLayout(testLayout);
    basicLayout->addWidget(testGroup);

    // Regular Comment Test Mode
    QGroupBox* commentTestGroup = new QGroupBox("通常コメントでテスト (収益化不要)");
    QFormLayout* commentTestLayout = new QFormLayout();

    m_triggerOnCommentCheck = new QCheckBox("通常コメントでエフェクト発動");
    m_triggerOnCommentCheck->setToolTip("チェックするとスパチャ以外の通常コメントでもエフェクトが発動します（テスト用）");
    commentTestLayout->addRow(m_triggerOnCommentCheck);

    m_commentAmountSpin = new QDoubleSpinBox();
    m_commentAmountSpin->setRange(100, 50000);
    m_commentAmountSpin->setValue(100);
    m_commentAmountSpin->setSuffix(" JPY");
    m_commentAmountSpin->setToolTip("通常コメントを何円のスパチャとして扱うか");
    commentTestLayout->addRow("コメント扱い金額:", m_commentAmountSpin);

    QLabel* commentHelpLabel = new QLabel(
        "この機能を有効にすると、YouTubeライブの通常コメントが\n"
        "指定金額のスーパーチャットとして扱われ、エフェクトが発動します。\n"
        "収益化されていないチャンネルでもテストが可能です。"
    );
    commentHelpLabel->setStyleSheet("color: #7f8c8d; font-size: 9pt;");
    commentTestLayout->addRow(commentHelpLabel);

    commentTestGroup->setLayout(commentTestLayout);
    basicLayout->addWidget(commentTestGroup);

    // Control Buttons
    QHBoxLayout* controlLayout = new QHBoxLayout();

    m_testButton = new QPushButton("接続テスト");
    m_testButton->setObjectName("testButton");
    m_startButton = new QPushButton("モニタリング開始");
    m_stopButton = new QPushButton("モニタリング停止");
    m_stopButton->setObjectName("stopButton");
    m_stopButton->setEnabled(false);

    controlLayout->addWidget(m_testButton);
    controlLayout->addWidget(m_startButton);
    controlLayout->addWidget(m_stopButton);

    basicLayout->addLayout(controlLayout);
    basicLayout->addStretch();

    // ===== Tab 2: Effect Configurations =====
    QWidget* effectConfigTab = new QWidget();
    QVBoxLayout* effectConfigLayout = new QVBoxLayout(effectConfigTab);

    m_effectConfigManager = new EffectConfigManager();
    connect(m_effectConfigManager, &EffectConfigManager::ConfigurationsChanged,
            this, &SettingsDialog::OnEffectConfigsChanged);

    effectConfigLayout->addWidget(m_effectConfigManager);

    // Add tabs to tab widget
    m_tabWidget->addTab(basicTab, "基本設定");
    m_tabWidget->addTab(effectConfigTab, "エフェクト設定");

    mainLayout->addWidget(m_tabWidget);

    // Dialog Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    m_saveButton = new QPushButton("保存");
    m_saveButton->setObjectName("saveButton");
    m_cancelButton = new QPushButton("キャンセル");
    m_cancelButton->setObjectName("cancelButton");

    buttonLayout->addWidget(m_saveButton);
    buttonLayout->addWidget(m_cancelButton);

    mainLayout->addLayout(buttonLayout);

    // Connect signals
    connect(m_testButton, &QPushButton::clicked, this, &SettingsDialog::OnTestConnectionClicked);
    connect(m_startButton, &QPushButton::clicked, this, &SettingsDialog::OnStartMonitoringClicked);
    connect(m_stopButton, &QPushButton::clicked, this, &SettingsDialog::OnStopMonitoringClicked);
    connect(m_testObstructionButton, &QPushButton::clicked, this, &SettingsDialog::OnTestObstructionClicked);
    connect(m_testRecoveryButton, &QPushButton::clicked, this, &SettingsDialog::OnTestRecoveryClicked);
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

    // Load regular comment test settings
    m_triggerOnCommentCheck->setChecked(g_settings.triggerOnRegularComment);
    m_commentAmountSpin->setValue(g_settings.regularCommentAmount);

    // Load effect configurations
    if (m_effectConfigManager) {
        EffectConfigList configs;
        configs.FromVariantList(g_settings.effectConfigurations);
        m_effectConfigManager->LoadConfigurations(configs);
    }

    UpdateMonitoringState();
}

void SettingsDialog::SaveSettings() {
    g_settings.youtubeApiKey = m_apiKeyEdit->text().toStdString();
    g_settings.videoId = m_videoIdEdit->text().toStdString();
    g_settings.enableObstructions = m_enableObstructionsCheck->isChecked();
    g_settings.enableRecovery = m_enableRecoveryCheck->isChecked();
    g_settings.obstructionIntensity = m_obstructionIntensitySpin->value();
    g_settings.recoveryIntensity = m_recoveryIntensitySpin->value();

    // Save regular comment test settings
    g_settings.triggerOnRegularComment = m_triggerOnCommentCheck->isChecked();
    g_settings.regularCommentAmount = m_commentAmountSpin->value();

    // Save effect configurations
    if (m_effectConfigManager) {
        EffectConfigList configs = m_effectConfigManager->GetConfigurations();
        g_settings.effectConfigurations = configs.ToVariantList();
    }

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

        m_statusLabel->setText("🟢 モニタリング中 - 投げ銭を待機しています...");
        m_statusLabel->setStyleSheet("background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #27ae60, stop:1 #2ecc71); color: white; border: 2px solid #1abc9c;");
        m_startButton->setEnabled(false);
        m_stopButton->setEnabled(true);
    }
}

void SettingsDialog::OnStopMonitoringClicked() {
    if (g_chatClient) {
        g_chatClient->Stop();
        UpdateMonitoringState();

        m_statusLabel->setText("⭕ モニタリング停止中");
        m_statusLabel->setStyleSheet("background-color: #34495e; color: #95a5a6; border: 2px solid #7f8c8d;");
        m_startButton->setEnabled(true);
        m_stopButton->setEnabled(false);
    }
}

void SettingsDialog::UpdateMonitoringState() {
    if (g_chatClient && g_chatClient->IsRunning()) {
        m_startButton->setEnabled(false);
        m_stopButton->setEnabled(true);
        m_statusLabel->setText("🟢 モニタリング中");
        m_statusLabel->setStyleSheet("background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #27ae60, stop:1 #2ecc71); color: white; border: 2px solid #1abc9c;");
    } else {
        m_startButton->setEnabled(true);
        m_stopButton->setEnabled(false);
        m_statusLabel->setText("⭕ モニタリング停止中");
        m_statusLabel->setStyleSheet("background-color: #34495e; color: #95a5a6; border: 2px solid #7f8c8d;");
    }
}

void SettingsDialog::OnTestObstructionClicked() {
    // Save current settings first
    SaveSettings();

    if (!g_obstructionManager) {
        QMessageBox::warning(this, "エラー", "エフェクトマネージャーが初期化されていません。");
        return;
    }

    // Check if main source is set
    std::string mainSource = m_mainSourceEdit->text().toStdString();
    if (mainSource.empty()) {
        QMessageBox::warning(this, "メインソース未設定",
                           "テストする前にメインソース名を設定してください。\n\n"
                           "これは縮小したいソースの名前です（例: 'ゲームキャプチャ'）。");
        return;
    }

    // Get test amount
    double testAmount = m_testAmountSpin->value();

    // Load effect configurations and find matching config
    EffectConfigList configs;
    configs.FromVariantList(g_settings.effectConfigurations);
    EffectSettings config = configs.FindConfigForAmount(testAmount);

    QString resultMessage;

    if (config.amount > 0.0) {
        // Found a configured effect
        g_obstructionManager->ApplyConfiguredEffect(config);

        resultMessage = QString(
            "✓ 設定済みエフェクトを適用\n\n"
            "テスト金額: %1 JPY\n"
            "適用される設定: %2 JPY\n"
            "エフェクト: %3\n"
            "持続時間: %4秒\n\n"
            "OBSプレビューを確認してください。"
        ).arg(testAmount)
         .arg(config.amount)
         .arg(::EffectActionToString(config.action))
         .arg(config.duration);

        blog(LOG_INFO, "[Test] Using configured effect for %.2f JPY: Action=%d",
             testAmount, static_cast<int>(config.action));
    } else {
        // No configuration found - use fallback
        g_obstructionManager->ApplyObstruction(testAmount * g_settings.obstructionIntensity);

        resultMessage = QString(
            "⚠ デフォルト動作を適用\n\n"
            "テスト金額: %1 JPY\n"
            "この金額に対する設定が見つかりませんでした。\n\n"
            "ランダムエフェクトが適用されます。\n"
            "特定のエフェクトをテストするには、\n"
            "「エフェクト設定」タブで金額を設定してください。"
        ).arg(testAmount);

        blog(LOG_INFO, "[Test] No config found for %.2f JPY, using default random behavior", testAmount);
    }

    QMessageBox::information(this, "テスト実行", resultMessage);
}

void SettingsDialog::OnTestRecoveryClicked() {
    // Save current settings first
    SaveSettings();

    if (!g_obstructionManager) {
        QMessageBox::warning(this, "Error", "Obstruction manager not initialized.");
        return;
    }

    // Check if main source is set
    std::string mainSource = m_mainSourceEdit->text().toStdString();
    if (mainSource.empty()) {
        QMessageBox::warning(this, "Missing Main Source",
                           "Please set the Main Source name before testing.\n\n"
                           "This should be the name of the source you want to expand (e.g., 'Game Capture').");
        return;
    }

    // Get test amount
    double testAmount = m_testAmountSpin->value();

    // Create a simulated SuperSticker event (this simulates the entire API flow)
    DonationEvent simulatedEvent;
    simulatedEvent.type = DonationType::SuperSticker;
    simulatedEvent.amount = testAmount;
    simulatedEvent.displayName = "Test User";
    simulatedEvent.message = "";
    simulatedEvent.currency = "JPY";

    // Call the same handler that would be called when real API data arrives
    // This tests the complete flow: API -> Handler -> Effects
    OnDonationReceived(simulatedEvent);

    QMessageBox::information(this, "Test Applied",
                           QString("Simulated SuperSticker: %1 JPY from '%2'\n\n"
                                   "This tests the complete API flow!\n"
                                   "Check your OBS preview and logs.").arg(testAmount).arg("Test User"));
}

void SettingsDialog::OnEffectConfigsChanged() {
    // Mark that configurations have been modified
    // They will be saved when user clicks Save button
    blog(LOG_INFO, "[Settings] Effect configurations modified");
}
