#include "effect-config-manager.hpp"
#include "effect-config-dialog.hpp"
#include "plugin-main.hpp"
#include "obstruction-manager.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <obs-module.h>

extern std::unique_ptr<ObstructionManager> g_obstructionManager;

EffectConfigManager::EffectConfigManager(QWidget* parent)
    : QWidget(parent)
{
    SetupUI();
}

EffectConfigManager::~EffectConfigManager() {
}

void EffectConfigManager::SetupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // Apply modern dark theme styling
    QString styleSheet = R"(
        QWidget {
            color: #ecf0f1;
        }

        QTableWidget {
            background-color: #1a252f;
            border: 2px solid #1abc9c;
            border-radius: 8px;
            gridline-color: #34495e;
            selection-background-color: #3498db;
            selection-color: white;
            font-size: 9pt;
        }

        QTableWidget::item {
            padding: 10px;
            color: #ecf0f1;
            border-bottom: 1px solid #34495e;
        }

        QTableWidget::item:selected {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #3498db, stop:1 #2980b9);
            color: white;
        }

        QTableWidget::item:hover {
            background-color: #2c3e50;
        }

        QHeaderView::section {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #1abc9c, stop:1 #16a085);
            color: white;
            padding: 12px;
            border: none;
            font-weight: bold;
            font-size: 10pt;
        }

        QHeaderView::section:first {
            border-top-left-radius: 6px;
        }

        QHeaderView::section:last {
            border-top-right-radius: 6px;
        }

        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #3498db, stop:1 #2980b9);
            color: white;
            border: none;
            padding: 10px 20px;
            border-radius: 6px;
            font-weight: bold;
            font-size: 9pt;
            min-width: 100px;
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

        QPushButton#addButton {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #2ecc71, stop:1 #27ae60);
        }

        QPushButton#addButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #58d68d, stop:1 #2ecc71);
        }

        QPushButton#editButton {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #f39c12, stop:1 #e67e22);
        }

        QPushButton#editButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #f5b041, stop:1 #f39c12);
        }

        QPushButton#deleteButton {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #e74c3c, stop:1 #c0392b);
        }

        QPushButton#deleteButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #ec7063, stop:1 #e74c3c);
        }

        QPushButton#testButton {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #9b59b6, stop:1 #8e44ad);
        }

        QPushButton#testButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #af7ac5, stop:1 #9b59b6);
        }

        QPushButton#resetButton {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #e74c3c, stop:1 #c0392b);
            color: white;
            font-weight: bold;
            padding: 12px 24px;
            font-size: 10pt;
        }

        QPushButton#resetButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #ec7063, stop:1 #e74c3c);
        }

        QLabel {
            color: #ecf0f1;
            font-size: 9pt;
        }
    )";

    setStyleSheet(styleSheet);

    // Info label with modern styling
    m_infoLabel = new QLabel(
        "💡 設定した金額に応じて異なるエフェクトが発動します。\n"
        "各金額設定をテストして、期待通りの動作を確認してください。"
    );
    m_infoLabel->setStyleSheet(
        "QLabel { "
        "padding: 14px; "
        "background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #2c3e50, stop:1 #34495e); "
        "border-left: 5px solid #1abc9c; "
        "border-radius: 6px; "
        "color: #1abc9c; "
        "font-size: 10pt; "
        "font-weight: bold; "
        "border: 2px solid #1abc9c; "
        "}"
    );
    m_infoLabel->setWordWrap(true);
    mainLayout->addWidget(m_infoLabel);

    // Table
    m_configTable = new QTableWidget();
    m_configTable->setColumnCount(4);
    m_configTable->setHorizontalHeaderLabels({"金額 (JPY)", "エフェクト", "持続時間 (秒)", "詳細"});
    m_configTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_configTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_configTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_configTable->horizontalHeader()->setStretchLastSection(true);
    m_configTable->setMinimumHeight(200);

    mainLayout->addWidget(m_configTable);

    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();

    m_addButton = new QPushButton("➕ 追加");
    m_addButton->setObjectName("addButton");
    m_addButton->setToolTip("新しい金額設定を追加");

    m_editButton = new QPushButton("✏️ 編集");
    m_editButton->setObjectName("editButton");
    m_editButton->setToolTip("選択した設定を編集");
    m_editButton->setEnabled(false);

    m_deleteButton = new QPushButton("🗑️ 削除");
    m_deleteButton->setObjectName("deleteButton");
    m_deleteButton->setToolTip("選択した設定を削除");
    m_deleteButton->setEnabled(false);

    m_testButton = new QPushButton("▶️ テスト");
    m_testButton->setObjectName("testButton");
    m_testButton->setToolTip("選択した設定をテスト実行");
    m_testButton->setEnabled(false);

    buttonLayout->addWidget(m_addButton);
    buttonLayout->addWidget(m_editButton);
    buttonLayout->addWidget(m_deleteButton);
    buttonLayout->addWidget(m_testButton);
    buttonLayout->addStretch();

    // Reset button (prominent)
    m_resetButton = new QPushButton("🔄 全エフェクトをリセット");
    m_resetButton->setObjectName("resetButton");
    m_resetButton->setToolTip("すべてのエフェクトを停止し、画面を元に戻す");
    buttonLayout->addWidget(m_resetButton);

    mainLayout->addLayout(buttonLayout);

    // Connect signals
    connect(m_addButton, &QPushButton::clicked, this, &EffectConfigManager::OnAddConfig);
    connect(m_editButton, &QPushButton::clicked, this, &EffectConfigManager::OnEditConfig);
    connect(m_deleteButton, &QPushButton::clicked, this, &EffectConfigManager::OnDeleteConfig);
    connect(m_testButton, &QPushButton::clicked, this, &EffectConfigManager::OnTestConfig);
    connect(m_resetButton, &QPushButton::clicked, this, &EffectConfigManager::OnResetAll);
    connect(m_configTable, &QTableWidget::itemSelectionChanged, this, &EffectConfigManager::OnSelectionChanged);
    connect(m_configTable, &QTableWidget::cellDoubleClicked, this, &EffectConfigManager::OnEditConfig);
}

void EffectConfigManager::LoadConfigurations(const EffectConfigList& configs) {
    m_configs = configs;
    RefreshTable();
}

EffectConfigList EffectConfigManager::GetConfigurations() const {
    return m_configs;
}

void EffectConfigManager::ClearConfigurations() {
    m_configs.Clear();
    RefreshTable();
    emit ConfigurationsChanged();
}

void EffectConfigManager::OnAddConfig() {
    EffectConfigDialog dialog(this);

    if (dialog.exec() == QDialog::Accepted) {
        EffectSettings settings = dialog.GetEffectSettings();
        m_configs.AddConfig(settings);
        RefreshTable();
        emit ConfigurationsChanged();

        blog(LOG_INFO, "[EffectConfig] Added new configuration for %.2f JPY", settings.amount);
    }
}

void EffectConfigManager::OnEditConfig() {
    int row = m_configTable->currentRow();
    if (row < 0) return;

    const auto& allConfigs = m_configs.GetAllConfigs();
    if (row >= static_cast<int>(allConfigs.size())) return;

    EffectSettings settings = allConfigs[row];

    EffectConfigDialog dialog(this);
    dialog.SetEffectSettings(settings);

    if (dialog.exec() == QDialog::Accepted) {
        EffectSettings newSettings = dialog.GetEffectSettings();
        m_configs.UpdateConfig(row, newSettings);
        RefreshTable();
        emit ConfigurationsChanged();

        blog(LOG_INFO, "[EffectConfig] Updated configuration for %.2f JPY", newSettings.amount);
    }
}

void EffectConfigManager::OnDeleteConfig() {
    int row = m_configTable->currentRow();
    if (row < 0) return;

    const auto& allConfigs = m_configs.GetAllConfigs();
    if (row >= static_cast<int>(allConfigs.size())) return;

    EffectSettings settings = allConfigs[row];

    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "設定の削除",
        QString("金額 %1 JPY の設定を削除しますか？").arg(settings.amount),
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        m_configs.RemoveConfig(row);
        RefreshTable();
        emit ConfigurationsChanged();

        blog(LOG_INFO, "[EffectConfig] Deleted configuration for %.2f JPY", settings.amount);
    }
}

void EffectConfigManager::OnTestConfig() {
    int row = m_configTable->currentRow();
    if (row < 0) return;

    const auto& allConfigs = m_configs.GetAllConfigs();
    if (row >= static_cast<int>(allConfigs.size())) return;

    EffectSettings settings = allConfigs[row];

    if (!g_obstructionManager) {
        QMessageBox::warning(this, "エラー", "エフェクトマネージャーが初期化されていません。");
        return;
    }

    blog(LOG_INFO, "[EffectConfig] Testing configuration: %.2f JPY, Action: %d",
         settings.amount, static_cast<int>(settings.action));

    // Apply the configured effect directly
    g_obstructionManager->ApplyConfiguredEffect(settings);

    QString effectDetails;
    switch (settings.action) {
        case EffectAction::ImageOverlay:
            effectDetails = QString("\n画像: %1\n拡大率: %2%")
                .arg(settings.mediaPath.isEmpty() ? "フォルダから選択" : settings.mediaPath)
                .arg(settings.imageScale);
            break;
        case EffectAction::VideoOverlay:
            effectDetails = QString("\n動画: %1\n拡大率: %2%")
                .arg(settings.mediaPath.isEmpty() ? "フォルダから選択" : settings.mediaPath)
                .arg(settings.imageScale);
            break;
        case EffectAction::ShrinkScreen:
            effectDetails = QString("\n縮小率: %1%").arg(settings.shrinkPercentage);
            break;
        case EffectAction::Rotation:
            effectDetails = QString("\n回転速度: %1").arg(settings.rotationSpeed);
            break;
        case EffectAction::Particle:
            effectDetails = QString("\nパーティクル数: %1").arg(settings.particleCount);
            break;
        default:
            effectDetails = "";
            break;
    }

    QMessageBox::information(
        this,
        "✓ テスト実行",
        QString("金額: %1 JPY\nエフェクト: %2\n持続時間: %3秒%4\n\n設定したエフェクトを適用しました。\nOBSプレビューを確認してください。")
            .arg(settings.amount)
            .arg(EffectActionToString(settings.action))
            .arg(settings.duration)
            .arg(effectDetails)
    );
}

void EffectConfigManager::OnResetAll() {
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "全エフェクトをリセット",
        "すべてのエフェクトを停止し、画面を元のサイズに戻しますか？",
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        if (g_obstructionManager) {
            g_obstructionManager->ClearAllObstructions();
            blog(LOG_INFO, "[EffectConfig] All effects cleared and screen restored");

            QMessageBox::information(this, "リセット完了", "すべてのエフェクトをクリアし、画面を元に戻しました。");
        }
    }
}

void EffectConfigManager::OnSelectionChanged() {
    UpdateButtonStates();
}

void EffectConfigManager::RefreshTable() {
    m_configTable->setRowCount(0);

    const auto& allConfigs = m_configs.GetAllConfigs();

    for (size_t i = 0; i < allConfigs.size(); ++i) {
        const auto& config = allConfigs[i];

        int row = m_configTable->rowCount();
        m_configTable->insertRow(row);

        // Amount
        m_configTable->setItem(row, 0, new QTableWidgetItem(QString::number(config.amount, 'f', 0)));

        // Effect type
        m_configTable->setItem(row, 1, new QTableWidgetItem(EffectActionToString(config.action)));

        // Duration
        m_configTable->setItem(row, 2, new QTableWidgetItem(QString::number(config.duration, 'f', 1)));

        // Details
        QString details;
        switch (config.action) {
            case EffectAction::ImageOverlay:
                details = QString("画像: %1, 拡大率: %2%")
                    .arg(config.mediaPath.isEmpty() ? "フォルダから選択" : "選択済み")
                    .arg(config.imageScale * 100, 0, 'f', 0);
                break;
            case EffectAction::VideoOverlay:
                details = QString("動画: %1").arg(config.mediaPath.isEmpty() ? "未選択" : "選択済み");
                break;
            case EffectAction::ShrinkScreen:
                details = QString("縮小率: %1%").arg(config.shrinkPercentage, 0, 'f', 0);
                break;
            case EffectAction::Rotation:
                details = QString("タイプ: %1, 速度: %2")
                    .arg(config.rotationType)
                    .arg(config.rotationSpeed, 0, 'f', 1);
                break;
            case EffectAction::Blink:
                details = QString("間隔: %1秒, タイプ: %2")
                    .arg(config.blinkInterval, 0, 'f', 1)
                    .arg(config.blinkType);
                break;
            case EffectAction::HueShift:
                details = QString("タイプ: %1, 速度: %2")
                    .arg(config.hueType)
                    .arg(config.hueSpeed, 0, 'f', 1);
                break;
            case EffectAction::Shake:
                details = QString("タイプ: %1, 強度: %2")
                    .arg(config.shakeType)
                    .arg(config.shakeIntensity, 0, 'f', 1);
                break;
            case EffectAction::ProgressBar:
                details = QString("タイプ: %1").arg(config.progressBarType);
                break;
            case EffectAction::Particle:
                details = QString("数: %1, タイプ: %2")
                    .arg(config.particleCount)
                    .arg(config.particleType);
                break;
            default:
                details = "-";
                break;
        }

        m_configTable->setItem(row, 3, new QTableWidgetItem(details));
    }

    // Adjust column widths
    m_configTable->resizeColumnsToContents();
    m_configTable->horizontalHeader()->setStretchLastSection(true);

    UpdateButtonStates();
}

void EffectConfigManager::UpdateButtonStates() {
    bool hasSelection = m_configTable->currentRow() >= 0;
    m_editButton->setEnabled(hasSelection);
    m_deleteButton->setEnabled(hasSelection);
    m_testButton->setEnabled(hasSelection);
}

QString EffectConfigManager::EffectActionToString(EffectAction action) const {
    switch (action) {
        case EffectAction::Random: return "ランダム";
        case EffectAction::Rotation: return "回転";
        case EffectAction::Blink: return "点滅";
        case EffectAction::HueShift: return "色相変化";
        case EffectAction::Shake: return "画面揺れ";
        case EffectAction::ProgressBar: return "プログレスバー";
        case EffectAction::Rotation3D: return "3D回転";
        case EffectAction::Kaleidoscope: return "万華鏡";
        case EffectAction::ImageOverlay: return "画像表示";
        case EffectAction::VideoOverlay: return "動画/GIF表示";
        case EffectAction::ShrinkScreen: return "画面縮小";
        case EffectAction::Particle: return "パーティクル";
        case EffectAction::RandomShapes: return "ランダム図形";
        default: return "不明";
    }
}
