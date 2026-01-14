#include "plugin-main.hpp"
#include "youtube-chat-client.hpp"
#include "obstruction-manager.hpp"
#include "settings-dialog.hpp"
#include "effect-config.hpp"
#include "room-3d-source.hpp"

#include <obs-module.h>
#include <obs-frontend-api.h>
#include <util/config-file.h>
#include <util/base.h>
#include <QAction>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QJsonDocument>
#include <QJsonArray>
#include <QVariant>
#include <QProcess>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>

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

    // Safely load strings (config_get_string can return nullptr)
    const char* apiKey = config_get_string(config, CONFIG_SECTION, "ApiKey");
    const char* videoId = config_get_string(config, CONFIG_SECTION, "VideoId");

    g_settings.youtubeApiKey = apiKey ? apiKey : "";
    g_settings.videoId = videoId ? videoId : "";

    g_settings.enableObstructions = config_get_bool(config, CONFIG_SECTION, "EnableObstructions");
    g_settings.enableRecovery = config_get_bool(config, CONFIG_SECTION, "EnableRecovery");
    g_settings.obstructionIntensity = config_get_double(config, CONFIG_SECTION, "ObstructionIntensity");
    g_settings.recoveryIntensity = config_get_double(config, CONFIG_SECTION, "RecoveryIntensity");

    // Load effect configurations from JSON
    const char* effectConfigsJson = config_get_string(config, CONFIG_SECTION, "EffectConfigurations");
    if (effectConfigsJson && effectConfigsJson[0] != '\0') {
        QJsonDocument doc = QJsonDocument::fromJson(QByteArray(effectConfigsJson));
        if (doc.isArray()) {
            g_settings.effectConfigurations = doc.array().toVariantList();
            blog(LOG_INFO, "[Settings] Loaded %d effect configurations", g_settings.effectConfigurations.size());
        }
    } else {
        g_settings.effectConfigurations.clear();
    }

    // Load test/debug settings
    g_settings.triggerOnRegularComment = config_get_bool(config, CONFIG_SECTION, "TriggerOnRegularComment");
    g_settings.regularCommentAmount = config_get_double(config, CONFIG_SECTION, "RegularCommentAmount");

    // Set defaults if not configured
    if (g_settings.obstructionIntensity == 0.0)
        g_settings.obstructionIntensity = 1.0;
    if (g_settings.recoveryIntensity == 0.0)
        g_settings.recoveryIntensity = 1.0;
    if (g_settings.regularCommentAmount == 0.0)
        g_settings.regularCommentAmount = 100.0;  // デフォルト100円
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

    // Save effect configurations as JSON
    QJsonArray jsonArray = QJsonArray::fromVariantList(g_settings.effectConfigurations);
    QJsonDocument doc(jsonArray);
    QByteArray jsonData = doc.toJson(QJsonDocument::Compact);
    config_set_string(config, CONFIG_SECTION, "EffectConfigurations", jsonData.constData());
    blog(LOG_INFO, "[Settings] Saved %d effect configurations", g_settings.effectConfigurations.size());

    // Save test/debug settings
    config_set_bool(config, CONFIG_SECTION, "TriggerOnRegularComment", g_settings.triggerOnRegularComment);
    config_set_double(config, CONFIG_SECTION, "RegularCommentAmount", g_settings.regularCommentAmount);

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
            // Load effect configurations and find appropriate config for this amount
            EffectConfigList configs;
            configs.FromVariantList(g_settings.effectConfigurations);

            EffectSettings config = configs.FindConfigForAmount(event.amount);

            if (config.amount > 0.0) {
                // Found a configured effect for this amount
                blog(LOG_INFO, "[YouTube SuperChat] Using configured effect: Action=%d, Amount=%.2f, Duration=%.1f",
                     static_cast<int>(config.action), config.amount, config.duration);
                g_obstructionManager->ApplyConfiguredEffect(config);
            } else {
                // No configuration found, use default behavior
                blog(LOG_INFO, "[YouTube SuperChat] No configured effect found, using default");
                g_obstructionManager->ApplyObstruction(event.amount * g_settings.obstructionIntensity);
            }
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
    try {
        if (!g_settingsDialog) {
            QMainWindow* mainWindow = static_cast<QMainWindow*>(obs_frontend_get_main_window());
            if (!mainWindow) {
                blog(LOG_ERROR, "[YouTube SuperChat] Cannot create settings dialog: main window is null");
                return;
            }
            g_settingsDialog = std::make_unique<SettingsDialog>(mainWindow);
        }

        g_settingsDialog->LoadSettings();
        g_settingsDialog->exec();
    } catch (const std::exception& e) {
        blog(LOG_ERROR, "[YouTube SuperChat] Error showing settings dialog: %s", e.what());
    }
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

    // Register 3D Room Source
    register_room_3d_source();

    // Add Qt plugin paths for TLS backend
    // OBS uses Qt from .deps directory, we need to ensure TLS plugins are found
    QCoreApplication::addLibraryPath("C:/obs-studio/.deps/obs-deps-qt6-2025-08-23-x64/plugins");

    // Also try to add from OBS bin directory (in case plugins are copied there)
    QString obsPath = QCoreApplication::applicationDirPath();
    QCoreApplication::addLibraryPath(obsPath + "/../plugins");
    QCoreApplication::addLibraryPath(obsPath + "/plugins");

    blog(LOG_INFO, "[YouTube SuperChat] Qt plugin paths configured for TLS support");

    // Initialize managers
    try {
        g_obstructionManager = std::make_unique<ObstructionManager>();
        blog(LOG_INFO, "[YouTube SuperChat] ObstructionManager initialized");

        g_chatClient = std::make_unique<YouTubeChatClient>();
        blog(LOG_INFO, "[YouTube SuperChat] YouTubeChatClient initialized");

        // Set donation callback
        g_chatClient->SetDonationCallback(OnDonationReceived);
    } catch (const std::exception& e) {
        blog(LOG_ERROR, "[YouTube SuperChat] Failed to initialize: %s", e.what());
        return false;
    }

    // Add menu item
    QMainWindow* mainWindow = static_cast<QMainWindow*>(obs_frontend_get_main_window());
    if (mainWindow) {
        QAction* action = new QAction("YouTube SuperChat Plugin Settings", mainWindow);
        QObject::connect(action, &QAction::triggered, []() {
            ShowSettingsDialog();
        });

        // Get menu bar and find Tools menu
        QMenuBar* menuBar = mainWindow->menuBar();
        if (menuBar) {
            QMenu* toolsMenu = nullptr;
            QList<QAction*> menuActions = menuBar->actions();

            for (QAction* menuAction : menuActions) {
                QMenu* menu = menuAction->menu();
                if (menu) {
                    QString title = menu->title();
                    // Try different variations of the Tools menu title
                    if (title.contains("Tools", Qt::CaseInsensitive) ||
                        title.contains("ツール", Qt::CaseInsensitive)) {
                        toolsMenu = menu;
                        break;
                    }
                }
            }

            if (toolsMenu) {
                toolsMenu->addAction(action);
                blog(LOG_INFO, "[YouTube SuperChat] Menu item added to Tools menu");
            } else {
                blog(LOG_WARNING, "[YouTube SuperChat] Could not find Tools menu in menu bar");
            }
        } else {
            blog(LOG_WARNING, "[YouTube SuperChat] Could not get menu bar");
        }
    } else {
        blog(LOG_WARNING, "[YouTube SuperChat] Could not get main window");
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
