#pragma once

#include <obs-module.h>
#include <obs-frontend-api.h>
#include <memory>
#include <string>

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
};

extern PluginSettings g_settings;

// Settings functions
void LoadSettings();
void SaveSettings();

// Donation handler (for testing)
void OnDonationReceived(const DonationEvent& event);
