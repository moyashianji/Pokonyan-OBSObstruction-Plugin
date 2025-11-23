#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QLabel>

class SettingsDialog : public QDialog {
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget* parent = nullptr);
    ~SettingsDialog();

    void LoadSettings();
    void SaveSettings();

private slots:
    void OnSaveClicked();
    void OnCancelClicked();
    void OnTestConnectionClicked();
    void OnStartMonitoringClicked();
    void OnStopMonitoringClicked();

private:
    void SetupUI();
    void UpdateMonitoringState();

    // UI elements
    QLineEdit* m_apiKeyEdit;
    QLineEdit* m_videoIdEdit;
    QLineEdit* m_mainSourceEdit;

    QCheckBox* m_enableObstructionsCheck;
    QCheckBox* m_enableRecoveryCheck;

    QDoubleSpinBox* m_obstructionIntensitySpin;
    QDoubleSpinBox* m_recoveryIntensitySpin;

    QPushButton* m_testButton;
    QPushButton* m_startButton;
    QPushButton* m_stopButton;
    QPushButton* m_saveButton;
    QPushButton* m_cancelButton;

    QLabel* m_statusLabel;
};
