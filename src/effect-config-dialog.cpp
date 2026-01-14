#include "effect-config-dialog.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QFileDialog>
#include <QLabel>
#include <QScrollArea>

EffectConfigDialog::EffectConfigDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("エフェクト設定");
    setMinimumSize(500, 400);
    resize(550, 450);
    SetupUI();
}

EffectConfigDialog::~EffectConfigDialog() {
}

void EffectConfigDialog::SetupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // Apply modern dark theme styling
    QString styleSheet = R"(
        QDialog {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #2c3e50, stop:1 #34495e);
        }

        QWidget {
            color: #ecf0f1;
        }

        QGroupBox {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #34495e, stop:1 #2c3e50);
            border: 2px solid #1abc9c;
            border-radius: 8px;
            margin-top: 16px;
            padding-top: 20px;
            font-weight: bold;
            font-size: 10pt;
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
            font-size: 9pt;
        }

        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #3498db, stop:1 #2980b9);
            color: white;
            border: none;
            padding: 10px 24px;
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

        QLineEdit, QSpinBox, QDoubleSpinBox {
            padding: 8px;
            border: 2px solid #34495e;
            border-radius: 6px;
            background-color: #1a252f;
            color: #ecf0f1;
            selection-background-color: #3498db;
            selection-color: white;
            font-size: 9pt;
        }

        QLineEdit:focus, QSpinBox:focus, QDoubleSpinBox:focus {
            border: 2px solid #1abc9c;
            background-color: #21313f;
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

        QStackedWidget {
            background-color: transparent;
        }

        QScrollArea {
            border: none;
            background-color: transparent;
        }

        QScrollBar:vertical {
            background: #34495e;
            width: 12px;
            border-radius: 6px;
        }

        QScrollBar::handle:vertical {
            background: #1abc9c;
            border-radius: 6px;
            min-height: 20px;
        }

        QScrollBar::handle:vertical:hover {
            background: #16a085;
        }

        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            height: 0px;
        }
    )";

    setStyleSheet(styleSheet);

    // Create scroll area for content
    QScrollArea* scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    QWidget* scrollContent = new QWidget();
    QVBoxLayout* contentLayout = new QVBoxLayout(scrollContent);

    // === 基本設定 ===
    QGroupBox* basicGroup = new QGroupBox("基本設定");
    QFormLayout* basicLayout = new QFormLayout();

    m_amountSpin = new QDoubleSpinBox();
    m_amountSpin->setRange(100, 1000000);
    m_amountSpin->setValue(1000);
    m_amountSpin->setSuffix(" JPY");
    m_amountSpin->setToolTip("この金額以上のスパチャでこのエフェクトが発動");
    basicLayout->addRow("金額:", m_amountSpin);

    m_effectTypeCombo = new QComboBox();
    m_effectTypeCombo->addItem("ランダム");
    m_effectTypeCombo->addItem("回転");
    m_effectTypeCombo->addItem("点滅");
    m_effectTypeCombo->addItem("色相変化");
    m_effectTypeCombo->addItem("画面揺れ");
    m_effectTypeCombo->addItem("プログレスバー");
    m_effectTypeCombo->addItem("3D回転");
    m_effectTypeCombo->addItem("万華鏡");
    m_effectTypeCombo->addItem("画像表示");
    m_effectTypeCombo->addItem("動画/GIF表示");
    m_effectTypeCombo->addItem("画面縮小");
    m_effectTypeCombo->addItem("パーティクル");
    m_effectTypeCombo->addItem("ランダム図形");
    connect(m_effectTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &EffectConfigDialog::OnEffectTypeChanged);
    basicLayout->addRow("エフェクトの種類:", m_effectTypeCombo);

    m_durationSpin = new QDoubleSpinBox();
    m_durationSpin->setRange(1.0, 60.0);
    m_durationSpin->setValue(5.0);
    m_durationSpin->setSingleStep(0.5);
    m_durationSpin->setSuffix(" 秒");
    m_durationSpin->setToolTip("エフェクトの持続時間");
    basicLayout->addRow("持続時間:", m_durationSpin);

    basicGroup->setLayout(basicLayout);
    contentLayout->addWidget(basicGroup);

    // === エフェクト固有のパラメータ ===
    QGroupBox* paramGroup = new QGroupBox("詳細設定");
    QVBoxLayout* paramLayout = new QVBoxLayout();

    m_parameterStack = new QStackedWidget();

    // 0: ランダム（パラメータなし）
    QWidget* randomWidget = new QWidget();
    QVBoxLayout* randomLayout = new QVBoxLayout(randomWidget);
    randomLayout->addWidget(new QLabel("すべてのエフェクトからランダムに選択されます"));
    m_parameterStack->addWidget(randomWidget);

    // 1: 回転
    m_rotationWidget = new QWidget();
    QFormLayout* rotationLayout = new QFormLayout(m_rotationWidget);

    m_rotationTypeCombo = new QComboBox();
    m_rotationTypeCombo->addItem("Z軸回転（平面）");
    m_rotationTypeCombo->addItem("X軸回転（縦）");
    m_rotationTypeCombo->addItem("Y軸回転（横）");
    m_rotationTypeCombo->addItem("全軸回転");
    rotationLayout->addRow("回転の種類:", m_rotationTypeCombo);

    m_rotationSpeedSpin = new QDoubleSpinBox();
    m_rotationSpeedSpin->setRange(0.1, 10.0);
    m_rotationSpeedSpin->setValue(0.5);
    m_rotationSpeedSpin->setSingleStep(0.1);
    m_rotationSpeedSpin->setSuffix(" 回転/秒");
    rotationLayout->addRow("回転速度:", m_rotationSpeedSpin);

    m_rotationReverseCheck = new QCheckBox("逆回転");
    rotationLayout->addRow("", m_rotationReverseCheck);

    m_parameterStack->addWidget(m_rotationWidget);

    // 2: 点滅
    m_blinkWidget = new QWidget();
    QFormLayout* blinkLayout = new QFormLayout(m_blinkWidget);

    m_blinkTypeCombo = new QComboBox();
    m_blinkTypeCombo->addItem("通常点滅");
    m_blinkTypeCombo->addItem("フェード点滅");
    m_blinkTypeCombo->addItem("ランダム点滅");
    blinkLayout->addRow("点滅の種類:", m_blinkTypeCombo);

    m_blinkFrequencySpin = new QDoubleSpinBox();
    m_blinkFrequencySpin->setRange(0.5, 20.0);
    m_blinkFrequencySpin->setValue(3.0);
    m_blinkFrequencySpin->setSingleStep(0.5);
    m_blinkFrequencySpin->setSuffix(" Hz");
    blinkLayout->addRow("点滅周波数:", m_blinkFrequencySpin);

    m_blinkIntervalSpin = new QDoubleSpinBox();
    m_blinkIntervalSpin->setRange(0.1, 5.0);
    m_blinkIntervalSpin->setValue(0.5);
    m_blinkIntervalSpin->setSingleStep(0.1);
    m_blinkIntervalSpin->setSuffix(" 秒");
    blinkLayout->addRow("点滅間隔:", m_blinkIntervalSpin);

    m_parameterStack->addWidget(m_blinkWidget);

    // 3: 色相変化
    m_hueWidget = new QWidget();
    QFormLayout* hueLayout = new QFormLayout(m_hueWidget);

    m_hueTypeCombo = new QComboBox();
    m_hueTypeCombo->addItem("虹色循環");
    m_hueTypeCombo->addItem("赤→青");
    m_hueTypeCombo->addItem("青→緑");
    m_hueTypeCombo->addItem("緑→赤");
    m_hueTypeCombo->addItem("ランダム");
    hueLayout->addRow("色相の種類:", m_hueTypeCombo);

    m_hueSpeedSpin = new QDoubleSpinBox();
    m_hueSpeedSpin->setRange(10.0, 360.0);
    m_hueSpeedSpin->setValue(180.0);
    m_hueSpeedSpin->setSingleStep(10.0);
    m_hueSpeedSpin->setSuffix(" 度/秒");
    hueLayout->addRow("変化速度:", m_hueSpeedSpin);

    m_parameterStack->addWidget(m_hueWidget);

    // 4: 画面揺れ
    m_shakeWidget = new QWidget();
    QFormLayout* shakeLayout = new QFormLayout(m_shakeWidget);

    m_shakeTypeCombo = new QComboBox();
    m_shakeTypeCombo->addItem("ランダム");
    m_shakeTypeCombo->addItem("左右揺れ");
    m_shakeTypeCombo->addItem("上下揺れ");
    m_shakeTypeCombo->addItem("円運動");
    m_shakeTypeCombo->addItem("地震");
    shakeLayout->addRow("揺れの種類:", m_shakeTypeCombo);

    m_shakeIntensitySpin = new QDoubleSpinBox();
    m_shakeIntensitySpin->setRange(1.0, 100.0);
    m_shakeIntensitySpin->setValue(10.0);
    m_shakeIntensitySpin->setSingleStep(1.0);
    m_shakeIntensitySpin->setSuffix(" px");
    shakeLayout->addRow("揺れの強さ:", m_shakeIntensitySpin);

    m_parameterStack->addWidget(m_shakeWidget);

    // 5: プログレスバー
    m_progressWidget = new QWidget();
    QFormLayout* progressLayout = new QFormLayout(m_progressWidget);

    m_progressBarTypeCombo = new QComboBox();
    m_progressBarTypeCombo->addItem("テキスト表示");
    m_progressBarTypeCombo->addItem("バー表示");
    m_progressBarTypeCombo->addItem("円形表示");
    progressLayout->addRow("表示の種類:", m_progressBarTypeCombo);

    m_progressBarPositionCombo = new QComboBox();
    m_progressBarPositionCombo->addItem("画面上部");
    m_progressBarPositionCombo->addItem("画面中央");
    m_progressBarPositionCombo->addItem("画面下部");
    progressLayout->addRow("表示位置:", m_progressBarPositionCombo);

    m_parameterStack->addWidget(m_progressWidget);

    // 6-7: 3D回転, 万華鏡（パラメータなし - 回転と同じ設定を使用）
    for (int i = 0; i < 2; ++i) {
        QWidget* simpleWidget = new QWidget();
        QVBoxLayout* simpleLayout = new QVBoxLayout(simpleWidget);
        simpleLayout->addWidget(new QLabel("基本設定のみで動作します"));
        m_parameterStack->addWidget(simpleWidget);
    }

    // 8: 画像表示
    m_imageWidget = new QWidget();
    QFormLayout* imageLayout = new QFormLayout(m_imageWidget);

    m_mediaPathEdit = new QLineEdit();
    m_mediaPathEdit->setPlaceholderText("画像ファイルを選択...");
    m_browseMediaButton = new QPushButton("参照...");
    connect(m_browseMediaButton, &QPushButton::clicked, this, &EffectConfigDialog::OnBrowseMediaFile);
    QHBoxLayout* imagePathLayout = new QHBoxLayout();
    imagePathLayout->addWidget(m_mediaPathEdit);
    imagePathLayout->addWidget(m_browseMediaButton);
    imageLayout->addRow("画像ファイル:", imagePathLayout);

    m_mediaFolderEdit = new QLineEdit();
    m_mediaFolderEdit->setPlaceholderText("フォルダを選択するとランダムに選択されます");
    m_browseFolderButton = new QPushButton("フォルダ参照...");
    connect(m_browseFolderButton, &QPushButton::clicked, this, &EffectConfigDialog::OnBrowseMediaFolder);
    QHBoxLayout* imageFolderLayout = new QHBoxLayout();
    imageFolderLayout->addWidget(m_mediaFolderEdit);
    imageFolderLayout->addWidget(m_browseFolderButton);
    imageLayout->addRow("フォルダ:", imageFolderLayout);

    m_imageScaleSpin = new QDoubleSpinBox();
    m_imageScaleSpin->setRange(10.0, 500.0);
    m_imageScaleSpin->setValue(100.0);
    m_imageScaleSpin->setSingleStep(5.0);
    m_imageScaleSpin->setSuffix(" %");
    imageLayout->addRow("拡大率:", m_imageScaleSpin);

    m_parameterStack->addWidget(m_imageWidget);

    // 9: 動画/GIF表示
    m_videoWidget = new QWidget();
    QFormLayout* videoLayout = new QFormLayout(m_videoWidget);

    m_videoPathEdit = new QLineEdit();
    m_videoPathEdit->setPlaceholderText("動画/GIFファイルを選択...");
    m_browseVideoButton = new QPushButton("参照...");
    connect(m_browseVideoButton, &QPushButton::clicked, this, &EffectConfigDialog::OnBrowseVideoFile);
    QHBoxLayout* videoPathLayout = new QHBoxLayout();
    videoPathLayout->addWidget(m_videoPathEdit);
    videoPathLayout->addWidget(m_browseVideoButton);
    videoLayout->addRow("動画ファイル:", videoPathLayout);

    m_videoFolderEdit = new QLineEdit();
    m_videoFolderEdit->setPlaceholderText("フォルダを選択するとランダムに選択されます");
    m_browseVideoFolderButton = new QPushButton("フォルダ参照...");
    connect(m_browseVideoFolderButton, &QPushButton::clicked, this, &EffectConfigDialog::OnBrowseVideoFolder);
    QHBoxLayout* videoFolderLayout = new QHBoxLayout();
    videoFolderLayout->addWidget(m_videoFolderEdit);
    videoFolderLayout->addWidget(m_browseVideoFolderButton);
    videoLayout->addRow("フォルダ:", videoFolderLayout);

    m_videoScaleSpin = new QDoubleSpinBox();
    m_videoScaleSpin->setRange(10.0, 500.0);
    m_videoScaleSpin->setValue(100.0);
    m_videoScaleSpin->setSingleStep(5.0);
    m_videoScaleSpin->setSuffix(" %");
    videoLayout->addRow("拡大率:", m_videoScaleSpin);

    m_parameterStack->addWidget(m_videoWidget);

    // 10: 画面縮小
    m_shrinkWidget = new QWidget();
    QFormLayout* shrinkLayout = new QFormLayout(m_shrinkWidget);

    m_shrinkPercentageSpin = new QDoubleSpinBox();
    m_shrinkPercentageSpin->setRange(1.0, 90.0);
    m_shrinkPercentageSpin->setValue(20.0);
    m_shrinkPercentageSpin->setSingleStep(1.0);
    m_shrinkPercentageSpin->setSuffix(" %");
    m_shrinkPercentageSpin->setToolTip("画面を縮小する割合（20% = 80%サイズに縮小）");
    shrinkLayout->addRow("縮小率:", m_shrinkPercentageSpin);

    m_shrinkSmoothCheck = new QCheckBox("スムーズに縮小");
    m_shrinkSmoothCheck->setChecked(true);
    shrinkLayout->addRow("", m_shrinkSmoothCheck);

    m_parameterStack->addWidget(m_shrinkWidget);

    // 11: パーティクル
    m_particleWidget = new QWidget();
    QFormLayout* particleLayout = new QFormLayout(m_particleWidget);

    m_particleCountSpin = new QSpinBox();
    m_particleCountSpin->setRange(10, 500);
    m_particleCountSpin->setValue(50);
    m_particleCountSpin->setSingleStep(10);
    particleLayout->addRow("パーティクル数:", m_particleCountSpin);

    m_particleTypeCombo = new QComboBox();
    m_particleTypeCombo->addItem("爆発");
    m_particleTypeCombo->addItem("雨");
    m_particleTypeCombo->addItem("雪");
    m_particleTypeCombo->addItem("星");
    particleLayout->addRow("パーティクルの種類:", m_particleTypeCombo);

    m_parameterStack->addWidget(m_particleWidget);

    // 12: ランダム図形（パラメータなし）
    QWidget* shapesWidget = new QWidget();
    QVBoxLayout* shapesLayout = new QVBoxLayout(shapesWidget);
    shapesLayout->addWidget(new QLabel("ランダムな図形が画面に表示されます"));
    m_parameterStack->addWidget(shapesWidget);

    paramLayout->addWidget(m_parameterStack);
    paramGroup->setLayout(paramLayout);
    contentLayout->addWidget(paramGroup);

    // Set scroll content and add to scroll area
    scrollArea->setWidget(scrollContent);
    mainLayout->addWidget(scrollArea);

    // === ダイアログボタン ===
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    m_okButton = new QPushButton("OK");
    m_cancelButton = new QPushButton("キャンセル");

    connect(m_okButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);

    buttonLayout->addWidget(m_okButton);
    buttonLayout->addWidget(m_cancelButton);

    mainLayout->addLayout(buttonLayout);

    // 初期表示
    UpdateParametersVisibility();
}

void EffectConfigDialog::OnEffectTypeChanged(int index) {
    UpdateParametersVisibility();
}

void EffectConfigDialog::UpdateParametersVisibility() {
    int index = m_effectTypeCombo->currentIndex();
    m_parameterStack->setCurrentIndex(index);
}

void EffectConfigDialog::OnBrowseMediaFile() {
    QString fileName = QFileDialog::getOpenFileName(this,
        "画像ファイルを選択",
        "",
        "画像ファイル (*.png *.jpg *.jpeg *.gif *.bmp);;すべてのファイル (*.*)");

    if (!fileName.isEmpty()) {
        m_mediaPathEdit->setText(fileName);
    }
}

void EffectConfigDialog::OnBrowseMediaFolder() {
    QString folderName = QFileDialog::getExistingDirectory(this,
        "フォルダを選択",
        "",
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (!folderName.isEmpty()) {
        m_mediaFolderEdit->setText(folderName);
    }
}

void EffectConfigDialog::OnBrowseVideoFile() {
    QString fileName = QFileDialog::getOpenFileName(this,
        "動画/GIFファイルを選択",
        "",
        "動画ファイル (*.mp4 *.webm *.gif *.mov *.avi);;すべてのファイル (*.*)");

    if (!fileName.isEmpty()) {
        m_videoPathEdit->setText(fileName);
    }
}

void EffectConfigDialog::OnBrowseVideoFolder() {
    QString folderName = QFileDialog::getExistingDirectory(this,
        "フォルダを選択",
        "",
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (!folderName.isEmpty()) {
        m_videoFolderEdit->setText(folderName);
    }
}

void EffectConfigDialog::SetEffectSettings(const EffectSettings& settings) {
    m_amountSpin->setValue(settings.amount);
    m_effectTypeCombo->setCurrentIndex(static_cast<int>(settings.action));
    m_durationSpin->setValue(settings.duration);

    // 画像/動画パス設定
    m_mediaPathEdit->setText(settings.mediaPath);
    m_mediaFolderEdit->setText(settings.mediaFolder);
    m_imageScaleSpin->setValue(settings.imageScale);
    m_videoPathEdit->setText(settings.mediaPath);
    m_videoFolderEdit->setText(settings.mediaFolder);
    m_videoScaleSpin->setValue(settings.imageScale);

    m_hueTypeCombo->setCurrentIndex(settings.hueType);
    m_hueSpeedSpin->setValue(settings.hueSpeed);

    m_blinkIntervalSpin->setValue(settings.blinkInterval);
    m_blinkTypeCombo->setCurrentIndex(settings.blinkType);
    m_blinkFrequencySpin->setValue(settings.blinkFrequency);

    m_shakeTypeCombo->setCurrentIndex(settings.shakeType);
    m_shakeIntensitySpin->setValue(settings.shakeIntensity);

    m_progressBarTypeCombo->setCurrentIndex(settings.progressBarType);
    m_progressBarPositionCombo->setCurrentIndex(settings.progressBarPosition);

    m_rotationTypeCombo->setCurrentIndex(settings.rotationType);
    m_rotationSpeedSpin->setValue(settings.rotationSpeed);
    m_rotationReverseCheck->setChecked(settings.rotationReverse);

    m_shrinkPercentageSpin->setValue(settings.shrinkPercentage);
    m_shrinkSmoothCheck->setChecked(settings.shrinkSmooth);

    m_particleCountSpin->setValue(settings.particleCount);
    m_particleTypeCombo->setCurrentIndex(settings.particleType);

    UpdateParametersVisibility();
}

EffectSettings EffectConfigDialog::GetEffectSettings() const {
    EffectSettings settings;

    settings.amount = m_amountSpin->value();
    settings.action = static_cast<EffectAction>(m_effectTypeCombo->currentIndex());
    settings.duration = m_durationSpin->value();

    // 画像または動画パスを取得（選択されたエフェクトタイプに応じて）
    if (settings.action == EffectAction::ImageOverlay) {
        settings.mediaPath = m_mediaPathEdit->text();
        settings.mediaFolder = m_mediaFolderEdit->text();
        settings.imageScale = m_imageScaleSpin->value();
    } else if (settings.action == EffectAction::VideoOverlay) {
        settings.mediaPath = m_videoPathEdit->text();
        settings.mediaFolder = m_videoFolderEdit->text();
        settings.imageScale = m_videoScaleSpin->value();
    }

    settings.hueType = m_hueTypeCombo->currentIndex();
    settings.hueSpeed = m_hueSpeedSpin->value();

    settings.blinkInterval = m_blinkIntervalSpin->value();
    settings.blinkType = m_blinkTypeCombo->currentIndex();
    settings.blinkFrequency = m_blinkFrequencySpin->value();

    settings.shakeType = m_shakeTypeCombo->currentIndex();
    settings.shakeIntensity = m_shakeIntensitySpin->value();

    settings.progressBarType = m_progressBarTypeCombo->currentIndex();
    settings.progressBarPosition = m_progressBarPositionCombo->currentIndex();

    settings.rotationType = m_rotationTypeCombo->currentIndex();
    settings.rotationSpeed = m_rotationSpeedSpin->value();
    settings.rotationReverse = m_rotationReverseCheck->isChecked();

    settings.shrinkPercentage = m_shrinkPercentageSpin->value();
    settings.shrinkSmooth = m_shrinkSmoothCheck->isChecked();

    settings.particleCount = m_particleCountSpin->value();
    settings.particleType = m_particleTypeCombo->currentIndex();

    return settings;
}
