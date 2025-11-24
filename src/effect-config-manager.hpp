#pragma once

#include "effect-config.hpp"
#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QLabel>

class EffectConfigManager : public QWidget {
    Q_OBJECT

public:
    explicit EffectConfigManager(QWidget* parent = nullptr);
    ~EffectConfigManager();

    // Configuration management
    void LoadConfigurations(const EffectConfigList& configs);
    EffectConfigList GetConfigurations() const;
    void ClearConfigurations();

signals:
    void ConfigurationsChanged();

private slots:
    void OnAddConfig();
    void OnEditConfig();
    void OnDeleteConfig();
    void OnTestConfig();
    void OnResetAll();
    void OnSelectionChanged();

private:
    void SetupUI();
    void RefreshTable();
    void UpdateButtonStates();
    QString EffectActionToString(EffectAction action) const;

    // UI elements
    QTableWidget* m_configTable;
    QPushButton* m_addButton;
    QPushButton* m_editButton;
    QPushButton* m_deleteButton;
    QPushButton* m_testButton;
    QPushButton* m_resetButton;
    QLabel* m_infoLabel;

    // Data
    EffectConfigList m_configs;
};
