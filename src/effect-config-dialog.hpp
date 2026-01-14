#pragma once

#include "effect-config.hpp"
#include <QDialog>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QLabel>
#include <QStackedWidget>

class EffectConfigDialog : public QDialog {
    Q_OBJECT

public:
    explicit EffectConfigDialog(QWidget* parent = nullptr);
    ~EffectConfigDialog();

    void SetEffectSettings(const EffectSettings& settings);
    EffectSettings GetEffectSettings() const;

private slots:
    void OnEffectTypeChanged(int index);
    void OnBrowseMediaFile();
    void OnBrowseMediaFolder();
    void OnBrowseVideoFile();
    void OnBrowseVideoFolder();

private:
    void SetupUI();
    void UpdateParametersVisibility();

    // 基本設定
    QDoubleSpinBox* m_amountSpin;
    QComboBox* m_effectTypeCombo;
    QDoubleSpinBox* m_durationSpin;

    // スタックウィジェット（エフェクトごとのパラメータ）
    QStackedWidget* m_parameterStack;

    // === 画像オーバーレイ ===
    QWidget* m_imageWidget;
    QLineEdit* m_mediaPathEdit;
    QPushButton* m_browseMediaButton;
    QLineEdit* m_mediaFolderEdit;
    QPushButton* m_browseFolderButton;
    QDoubleSpinBox* m_imageScaleSpin;

    // === 動画オーバーレイ ===
    QWidget* m_videoWidget;
    QLineEdit* m_videoPathEdit;
    QPushButton* m_browseVideoButton;
    QLineEdit* m_videoFolderEdit;
    QPushButton* m_browseVideoFolderButton;
    QDoubleSpinBox* m_videoScaleSpin;

    // === 色相変化 ===
    QWidget* m_hueWidget;
    QComboBox* m_hueTypeCombo;
    QDoubleSpinBox* m_hueSpeedSpin;

    // === 点滅 ===
    QWidget* m_blinkWidget;
    QDoubleSpinBox* m_blinkIntervalSpin;
    QComboBox* m_blinkTypeCombo;
    QDoubleSpinBox* m_blinkFrequencySpin;

    // === 画面揺れ ===
    QWidget* m_shakeWidget;
    QComboBox* m_shakeTypeCombo;
    QDoubleSpinBox* m_shakeIntensitySpin;

    // === プログレスバー ===
    QWidget* m_progressWidget;
    QComboBox* m_progressBarTypeCombo;
    QComboBox* m_progressBarPositionCombo;

    // === 回転 ===
    QWidget* m_rotationWidget;
    QComboBox* m_rotationTypeCombo;
    QDoubleSpinBox* m_rotationSpeedSpin;
    QCheckBox* m_rotationReverseCheck;

    // === 画面縮小 ===
    QWidget* m_shrinkWidget;
    QDoubleSpinBox* m_shrinkPercentageSpin;
    QCheckBox* m_shrinkSmoothCheck;

    // === パーティクル ===
    QWidget* m_particleWidget;
    QSpinBox* m_particleCountSpin;
    QComboBox* m_particleTypeCombo;

    // ボタン
    QPushButton* m_okButton;
    QPushButton* m_cancelButton;
};
