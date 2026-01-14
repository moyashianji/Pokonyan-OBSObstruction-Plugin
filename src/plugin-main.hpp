#pragma once

#include <obs-module.h>
#include <obs-frontend-api.h>
#include <memory>
#include <string>
#include <QVariantList>

class YouTubeChatClient;
class ObstructionManager;
class SettingsDialog;
struct DonationEvent;

// Plugin module info
#define PLUGIN_NAME "obs-youtube-superchat-plugin"
#define PLUGIN_VERSION "1.0.0"

// Global plugin instances
extern std::unique_ptr<YouTubeChatClient> g_chatClient;
extern std::unique_ptr<ObstructionManager> g_obstructionManager;
extern std::unique_ptr<SettingsDialog> g_settingsDialog;

// Plugin settings
struct PluginSettings {
    std::string youtubeApiKey;
    std::string videoId;
    bool enableObstructions;
    bool enableRecovery;
    double obstructionIntensity;
    double recoveryIntensity;
    QVariantList effectConfigurations;  // Serialized effect configurations

    // Test/Debug settings
    bool triggerOnRegularComment;       // 通常コメントでもエフェクト発動
    double regularCommentAmount;         // 通常コメントの扱い金額（デフォルト100円）
};

extern PluginSettings g_settings;

// Settings functions
void LoadSettings();
void SaveSettings();

// Donation handler (for testing)
void OnDonationReceived(const DonationEvent& event);
