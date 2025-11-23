#include "plugin-main.hpp"
#include "youtube-chat-client.hpp"
#include "obstruction-manager.hpp"
#include "settings-dialog.hpp"

#include <obs-module.h>
#include <obs-frontend-api.h>
#include <util/config-file.h>
#include <util/base.h>
#include <QAction>
#include <QMainWindow>
#include <QMenu>

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE(PLUGIN_NAME, "en-US")

// Global instances
std::unique_ptr<YouTubeChatClient> g_chatClient;
std::unique_ptr<ObstructionManager> g_obstructionManager;
std::unique_ptr<SettingsDialog> g_settingsDialog;

PluginSettings g_settings;

// Settings file
static const char* CONFIG_SECTION = "YouTubeSuperChatPlugin";

void LoadSettings() {
    config_t* config = obs_frontend_get_global_config();
    if (!config) return;

    g_settings.youtubeApiKey = config_get_string(config, CONFIG_SECTION, "ApiKey");
    g_settings.videoId = config_get_string(config, CONFIG_SECTION, "VideoId");
    g_settings.enableObstructions = config_get_bool(config, CONFIG_SECTION, "EnableObstructions");
    g_settings.enableRecovery = config_get_bool(config, CONFIG_SECTION, "EnableRecovery");
    g_settings.obstructionIntensity = config_get_double(config, CONFIG_SECTION, "ObstructionIntensity");
    g_settings.recoveryIntensity = config_get_double(config, CONFIG_SECTION, "RecoveryIntensity");

    // Set defaults if not configured
    if (g_settings.obstructionIntensity == 0.0)
        g_settings.obstructionIntensity = 1.0;
    if (g_settings.recoveryIntensity == 0.0)
        g_settings.recoveryIntensity = 1.0;
}

void SaveSettings() {
    config_t* config = obs_frontend_get_global_config();
    if (!config) return;

    config_set_string(config, CONFIG_SECTION, "ApiKey", g_settings.youtubeApiKey.c_str());
    config_set_string(config, CONFIG_SECTION, "VideoId", g_settings.videoId.c_str());
    config_set_bool(config, CONFIG_SECTION, "EnableObstructions", g_settings.enableObstructions);
    config_set_bool(config, CONFIG_SECTION, "EnableRecovery", g_settings.enableRecovery);
    config_set_double(config, CONFIG_SECTION, "ObstructionIntensity", g_settings.obstructionIntensity);
    config_set_double(config, CONFIG_SECTION, "RecoveryIntensity", g_settings.recoveryIntensity);

    config_save(config);
}

// Donation callback handler
void OnDonationReceived(const DonationEvent& event) {
    if (!g_obstructionManager) return;

    blog(LOG_INFO, "[YouTube SuperChat] Donation received: %s - %.2f %s (Type: %s)",
            event.displayName.c_str(),
            event.amount,
            event.currency.c_str(),
            event.type == DonationType::SuperChat ? "SuperChat" : "SuperSticker");

    if (event.type == DonationType::SuperChat) {
        // Apply obstruction effects
        if (g_settings.enableObstructions) {
            g_obstructionManager->ApplyObstruction(event.amount * g_settings.obstructionIntensity);
        }
    } else if (event.type == DonationType::SuperSticker) {
        // Apply recovery effects
        if (g_settings.enableRecovery) {
            g_obstructionManager->ApplyRecovery(event.amount * g_settings.recoveryIntensity);
        }
    }
}

// Show settings dialog
void ShowSettingsDialog() {
    if (!g_settingsDialog) {
        QMainWindow* mainWindow = static_cast<QMainWindow*>(obs_frontend_get_main_window());
        g_settingsDialog = std::make_unique<SettingsDialog>(mainWindow);
    }

    g_settingsDialog->LoadSettings();
    g_settingsDialog->exec();
}

// Frontend event callback
void OnFrontendEvent(enum obs_frontend_event event, void* private_data) {
    switch (event) {
    case OBS_FRONTEND_EVENT_FINISHED_LOADING:
        LoadSettings();
        break;
    case OBS_FRONTEND_EVENT_EXIT:
        if (g_chatClient && g_chatClient->IsRunning()) {
            g_chatClient->Stop();
        }
        break;
    default:
        break;
    }
}

bool obs_module_load(void) {
    blog(LOG_INFO, "YouTube SuperChat Plugin v%s loaded", PLUGIN_VERSION);

    // Initialize managers
    g_obstructionManager = std::make_unique<ObstructionManager>();
    g_chatClient = std::make_unique<YouTubeChatClient>();

    // Set donation callback
    g_chatClient->SetDonationCallback(OnDonationReceived);

    // Add menu item
    QMainWindow* mainWindow = static_cast<QMainWindow*>(obs_frontend_get_main_window());
    if (mainWindow) {
        QAction* action = new QAction("YouTube SuperChat Plugin Settings", mainWindow);
        QObject::connect(action, &QAction::triggered, []() {
            ShowSettingsDialog();
        });

        // Add to Tools menu
        QMenu* toolsMenu = nullptr;
        QList<QMenu*> menus = mainWindow->findChildren<QMenu*>();
        for (QMenu* menu : menus) {
            if (menu->title() == "Tools" || menu->title() == "ツール") {
                toolsMenu = menu;
                break;
            }
        }

        if (toolsMenu) {
            toolsMenu->addAction(action);
        }
    }

    // Register frontend callbacks
    obs_frontend_add_event_callback(OnFrontendEvent, nullptr);

    return true;
}

void obs_module_unload(void) {
    blog(LOG_INFO, "YouTube SuperChat Plugin unloaded");

    // Stop chat monitoring
    if (g_chatClient && g_chatClient->IsRunning()) {
        g_chatClient->Stop();
    }

    // Clean up
    g_settingsDialog.reset();
    g_chatClient.reset();
    g_obstructionManager.reset();
}

const char* obs_module_name(void) {
    return "YouTube SuperChat Plugin";
}

const char* obs_module_description(void) {
    return "Adds interactive obstruction and recovery effects based on YouTube Super Chats and Super Stickers";
}
