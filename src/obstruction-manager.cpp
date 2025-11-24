#include "obstruction-manager.hpp"
#include "effect-system.hpp"
#include "effect-config.hpp"
#include <obs-module.h>
#include <obs-frontend-api.h>
#include <graphics/vec2.h>
#include <graphics/matrix4.h>
#include <util/platform.h>
#include <util/base.h>
#include <filesystem>
#include <algorithm>
#include <cmath>
#include <QDir>
#include <QFileInfo>
#include <QFileInfoList>

namespace fs = std::filesystem;

ObstructionManager::ObstructionManager()
    : m_currentShrinkPercentage(0.0)
    , m_enabled(true)
    , m_originalTransformSaved(false)
    , m_randomEngine(std::random_device{}())
    , m_effectManager(std::make_unique<EffectManager>())
{
    m_originalScale.x = 1.0f;
    m_originalScale.y = 1.0f;
    m_originalPos.x = 0.0f;
    m_originalPos.y = 0.0f;
    m_originalRotation = 0.0f;

    blog(LOG_INFO, "[Obstruction] EffectManager initialized");
}

ObstructionManager::~ObstructionManager() {
    ClearAllObstructions();
}

void ObstructionManager::ApplyObstruction(double amount) {
    if (!m_enabled) return;

    blog(LOG_INFO, "[Obstruction] Applying obstruction for amount: %.2f JPY", amount);

    // Scale effects based on donation amount
    // 100 JPY = minimal effect, 10000 JPY = maximum effect
    double intensity = std::min(amount / 10000.0, 1.0);

    // Shrink main source (5-30% based on amount)
    double shrinkPercent = 5.0 + (intensity * 25.0);
    ShrinkMainSource(shrinkPercent);

    // Add obstruction overlays
    int numObstructions = static_cast<int>(1 + intensity * 3);  // 1-4 obstructions
    for (int i = 0; i < numObstructions; ++i) {
        AddRandomObstruction(intensity);
    }

    // Apply visual effects to main source
    if (m_effectManager && !m_mainSourceName.empty()) {
        obs_source_t* mainSource = FindSourceByName(m_mainSourceName);
        if (mainSource) {
            // Calculate effect duration based on donation amount (3-15 seconds)
            double effectDuration = 3.0 + (intensity * 12.0);

            // Apply 1-3 random effects based on intensity
            int numEffects = static_cast<int>(1 + intensity * 2);
            for (int i = 0; i < numEffects; ++i) {
                m_effectManager->ApplyRandomEffect(mainSource, intensity, effectDuration);
            }

            obs_source_release(mainSource);
            blog(LOG_INFO, "[Obstruction] Applied %d visual effects (duration: %.1fs)", numEffects, effectDuration);
        }
    }
}

void ObstructionManager::ShrinkMainSource(double percentage) {
    if (m_mainSourceName.empty()) {
        blog(LOG_WARNING, "[Obstruction] Main source name not set");
        return;
    }

    obs_source_t* source = FindSourceByName(m_mainSourceName);
    if (!source) {
        blog(LOG_WARNING, "[Obstruction] Main source not found: %s", m_mainSourceName.c_str());
        return;
    }

    // Accumulate shrink percentage (but cap at 80%)
    m_currentShrinkPercentage = std::min(m_currentShrinkPercentage + percentage, 80.0);

    // Calculate new scale
    double scale = 1.0 - (m_currentShrinkPercentage / 100.0);

    UpdateSourceTransform(source, scale);

    blog(LOG_INFO, "[Obstruction] Shrunk main source to %.1f%% (total shrink: %.1f%%)",
            scale * 100.0, m_currentShrinkPercentage);

    obs_source_release(source);
}

void ObstructionManager::AddRandomObstruction(double intensity) {
    std::string assetPath = SelectRandomObstructionAsset();
    if (assetPath.empty()) {
        blog(LOG_WARNING, "[Obstruction] No obstruction assets found");
        return;
    }

    CreateObstructionSource(assetPath, intensity);
}

void ObstructionManager::ApplyConfiguredEffect(const EffectSettings& config) {
    if (!m_enabled) return;

    blog(LOG_INFO, "[Obstruction] Applying configured effect: Action=%d, Duration=%.1f, Amount=%.2f",
         static_cast<int>(config.action), config.duration, config.amount);

    obs_source_t* mainSource = nullptr;
    if (!m_mainSourceName.empty()) {
        mainSource = FindSourceByName(m_mainSourceName);
    }

    switch (config.action) {
        case EffectAction::Random: {
            // Apply random effect
            if (m_effectManager && mainSource) {
                m_effectManager->ApplyRandomEffect(mainSource, 0.5, config.duration);
            } else {
                ApplyObstruction(config.amount);
            }
            break;
        }

        case EffectAction::Rotation: {
            // Apply rotation effect with configured parameters
            if (m_effectManager && mainSource) {
                m_effectManager->ApplyRotationEffect(mainSource, config.duration, config.rotationSpeed,
                                                     config.rotationType, config.rotationReverse);
                const char* rotTypeStr = (config.rotationType == 0) ? "Z軸" :
                                        (config.rotationType == 1) ? "X軸" :
                                        (config.rotationType == 2) ? "Y軸" : "全軸";
                blog(LOG_INFO, "[Obstruction] Applied rotation: type=%s, speed=%.1f, duration=%.1f, reverse=%d",
                     rotTypeStr, config.rotationSpeed, config.duration, config.rotationReverse);
            }
            break;
        }

        case EffectAction::Blink: {
            // Apply blink effect
            if (m_effectManager && mainSource) {
                m_effectManager->ApplyEffect(mainSource, EffectType::Blink, config.blinkFrequency, config.duration);
                blog(LOG_INFO, "[Obstruction] Applied blink: frequency=%.1f Hz, duration=%.1f",
                     config.blinkFrequency, config.duration);
            }
            break;
        }

        case EffectAction::HueShift: {
            // Apply hue shift effect
            if (m_effectManager && mainSource) {
                m_effectManager->ApplyEffect(mainSource, EffectType::HueShift, config.hueSpeed / 180.0, config.duration);
                blog(LOG_INFO, "[Obstruction] Applied hue shift: speed=%.1f deg/s, duration=%.1f",
                     config.hueSpeed, config.duration);
            }
            break;
        }

        case EffectAction::Shake: {
            // Apply shake effect
            if (m_effectManager && mainSource) {
                m_effectManager->ApplyEffect(mainSource, EffectType::Shake, config.shakeIntensity / 10.0, config.duration);
                blog(LOG_INFO, "[Obstruction] Applied shake: intensity=%.1f, duration=%.1f",
                     config.shakeIntensity, config.duration);
            }
            break;
        }

        case EffectAction::ProgressBar: {
            // Apply progress bar effect
            if (m_effectManager && mainSource) {
                m_effectManager->ApplyEffect(mainSource, EffectType::ProgressBar, 1.0, config.duration);
                blog(LOG_INFO, "[Obstruction] Applied progress bar: duration=%.1f", config.duration);
            }
            break;
        }

        case EffectAction::Rotation3D: {
            // Apply 3D rotation effect
            if (m_effectManager && mainSource) {
                m_effectManager->ApplyEffect(mainSource, EffectType::Rotation3D, 0.5, config.duration);
                blog(LOG_INFO, "[Obstruction] Applied 3D rotation: duration=%.1f", config.duration);
            }
            break;
        }

        case EffectAction::Kaleidoscope: {
            // Apply kaleidoscope effect
            if (m_effectManager && mainSource) {
                m_effectManager->ApplyEffect(mainSource, EffectType::Kaleidoscope, 1.0, config.duration);
                blog(LOG_INFO, "[Obstruction] Applied kaleidoscope: duration=%.1f", config.duration);
            }
            break;
        }

        case EffectAction::ImageOverlay: {
            // Apply image overlay
            QString imagePath = config.mediaPath;

            // If folder is specified, select random image from folder
            if (!config.mediaFolder.isEmpty() && QDir(config.mediaFolder).exists()) {
                QDir dir(config.mediaFolder);
                QStringList filters;
                filters << "*.png" << "*.jpg" << "*.jpeg" << "*.gif" << "*.bmp";
                QFileInfoList files = dir.entryInfoList(filters, QDir::Files);

                if (!files.isEmpty()) {
                    int randomIndex = std::uniform_int_distribution<int>(0, files.size() - 1)(m_randomEngine);
                    imagePath = files[randomIndex].absoluteFilePath();
                }
            }

            if (!imagePath.isEmpty()) {
                CreateObstructionSource(imagePath.toStdString(), config.imageScale / 100.0);
                blog(LOG_INFO, "[Obstruction] Applied image overlay: %s, scale=%.0f%%",
                     imagePath.toStdString().c_str(), config.imageScale);
            }
            break;
        }

        case EffectAction::VideoOverlay: {
            // Apply video/GIF overlay
            QString videoPath = config.mediaPath;

            // If folder is specified, select random video from folder
            if (!config.mediaFolder.isEmpty() && QDir(config.mediaFolder).exists()) {
                QDir dir(config.mediaFolder);
                QStringList filters;
                filters << "*.mp4" << "*.webm" << "*.gif" << "*.mov";
                QFileInfoList files = dir.entryInfoList(filters, QDir::Files);

                if (!files.isEmpty()) {
                    int randomIndex = std::uniform_int_distribution<int>(0, files.size() - 1)(m_randomEngine);
                    videoPath = files[randomIndex].absoluteFilePath();
                }
            }

            if (!videoPath.isEmpty()) {
                CreateObstructionSource(videoPath.toStdString(), 1.0);
                blog(LOG_INFO, "[Obstruction] Applied video overlay: %s", videoPath.toStdString().c_str());
            }
            break;
        }

        case EffectAction::ShrinkScreen: {
            // Apply screen shrink
            ShrinkMainSource(config.shrinkPercentage);
            blog(LOG_INFO, "[Obstruction] Applied screen shrink: %.0f%%", config.shrinkPercentage);
            break;
        }

        case EffectAction::Particle: {
            // Apply particle effect with specific type
            if (m_effectManager && mainSource) {
                m_effectManager->ApplyParticleEffect(mainSource, config.duration, config.particleCount, config.particleType);
                const char* particleTypeStr = (config.particleType == 0) ? "爆発" :
                                             (config.particleType == 1) ? "雨" :
                                             (config.particleType == 2) ? "雪" : "星";
                blog(LOG_INFO, "[Obstruction] Applied particle effect: type=%s, count=%d, duration=%.1f",
                     particleTypeStr, config.particleCount, config.duration);
            }
            break;
        }

        case EffectAction::RandomShapes: {
            // Apply random shapes effect
            if (m_effectManager && mainSource) {
                m_effectManager->ApplyEffect(mainSource, EffectType::RandomShapes, 1.0, config.duration);
                blog(LOG_INFO, "[Obstruction] Applied random shapes: duration=%.1f", config.duration);
            }
            break;
        }

        default:
            blog(LOG_WARNING, "[Obstruction] Unknown effect action: %d", static_cast<int>(config.action));
            break;
    }

    if (mainSource) {
        obs_source_release(mainSource);
    }
}

void ObstructionManager::ApplyRecovery(double amount) {
    if (!m_enabled) return;

    blog(LOG_INFO, "[Recovery] Applying recovery for amount: %.2f JPY", amount);

    // Scale recovery based on donation amount
    double intensity = std::min(amount / 10000.0, 1.0);

    // Expand main source (10-40% based on amount)
    double expandPercent = 10.0 + (intensity * 30.0);
    ExpandMainSource(expandPercent);

    // Remove obstructions
    int numToRemove = static_cast<int>(1 + intensity * 2);  // 1-3 obstructions
    for (int i = 0; i < numToRemove; ++i) {
        RemoveRandomObstruction();
    }
}

void ObstructionManager::ExpandMainSource(double percentage) {
    if (m_mainSourceName.empty()) return;

    // Reduce shrink percentage (but don't go negative)
    m_currentShrinkPercentage = std::max(m_currentShrinkPercentage - percentage, 0.0);

    obs_source_t* source = FindSourceByName(m_mainSourceName);
    if (!source) return;

    // Calculate new scale
    double scale = 1.0 - (m_currentShrinkPercentage / 100.0);

    UpdateSourceTransform(source, scale);

    blog(LOG_INFO, "[Recovery] Expanded main source to %.1f%% (remaining shrink: %.1f%%)",
            scale * 100.0, m_currentShrinkPercentage);

    obs_source_release(source);
}

void ObstructionManager::RemoveRandomObstruction() {
    // Find active obstructions
    std::vector<size_t> activeIndices;
    for (size_t i = 0; i < m_obstructions.size(); ++i) {
        if (m_obstructions[i].active) {
            activeIndices.push_back(i);
        }
    }

    if (activeIndices.empty()) {
        blog(LOG_INFO, "[Recovery] No active obstructions to remove");
        return;
    }

    // Select random obstruction
    std::uniform_int_distribution<size_t> dist(0, activeIndices.size() - 1);
    size_t indexToRemove = activeIndices[dist(m_randomEngine)];

    RemoveObstructionSource(m_obstructions[indexToRemove]);
    m_obstructions.erase(m_obstructions.begin() + indexToRemove);

    blog(LOG_INFO, "[Recovery] Removed obstruction (%zu active remaining)",
            GetActiveObstructionCount());
}

void ObstructionManager::ClearAllObstructions() {
    blog(LOG_INFO, "[Recovery] Clearing all obstructions");

    // First, clear tracked obstructions
    for (auto& obstruction : m_obstructions) {
        RemoveObstructionSource(obstruction);
    }
    m_obstructions.clear();

    // Also search for and remove any orphaned obstruction sources from all scenes
    // This handles obstructions that may have been left over from previous OBS sessions
    auto removeOrphanedObstructions = [](void* param, obs_source_t* source) -> bool {
        const char* sourceName = obs_source_get_name(source);
        std::string name(sourceName ? sourceName : "");

        // Check if this is an obstruction source (starts with "Obstruction" or contains "obstruction")
        if (name.find("Obstruction") != std::string::npos ||
            name.find("obstruction") != std::string::npos) {

            // Get current scene
            obs_source_t* sceneSource = obs_frontend_get_current_scene();
            if (sceneSource) {
                obs_scene_t* scene = obs_scene_from_source(sceneSource);
                if (scene) {
                    // Find and remove the scene item
                    obs_sceneitem_t* item = obs_scene_find_source(scene, sourceName);
                    if (item) {
                        obs_sceneitem_remove(item);
                        blog(LOG_INFO, "[Recovery] Removed orphaned obstruction: %s", sourceName);
                    }
                }
                obs_source_release(sceneSource);
            }
        }
        return true;
    };

    obs_enum_sources(removeOrphanedObstructions, nullptr);

    // Clear all visual effects
    if (m_effectManager) {
        m_effectManager->ClearAllEffects();
        blog(LOG_INFO, "[Recovery] Cleared all visual effects");
    }

    // Reset main source completely to original transform
    m_currentShrinkPercentage = 0.0;
    if (!m_mainSourceName.empty()) {
        obs_source_t* source = FindSourceByName(m_mainSourceName);
        if (source) {
            obs_sceneitem_t* sceneItem = FindSceneItemForSource(source);
            if (sceneItem) {
                // Restore to saved original transform if available
                if (m_originalTransformSaved) {
                    obs_sceneitem_set_scale(sceneItem, &m_originalScale);
                    obs_sceneitem_set_pos(sceneItem, &m_originalPos);
                    obs_sceneitem_set_rot(sceneItem, m_originalRotation);

                    blog(LOG_INFO, "[Recovery] Restored to original transform: scale=(%.2f, %.2f), pos=(%.2f, %.2f), rot=%.2f",
                         m_originalScale.x, m_originalScale.y, m_originalPos.x, m_originalPos.y, m_originalRotation);
                } else {
                    // Fallback to default reset
                    struct vec2 scaleVec;
                    scaleVec.x = 1.0f;
                    scaleVec.y = 1.0f;
                    obs_sceneitem_set_scale(sceneItem, &scaleVec);
                    obs_sceneitem_set_rot(sceneItem, 0.0f);

                    blog(LOG_INFO, "[Recovery] Reset to default transform (scale=100%%, rotation=0°)");
                }

                // Make sure source is visible
                obs_sceneitem_set_visible(sceneItem, true);
            }

            // Remove all filters from the main source
            auto removeAllFilters = [](obs_source_t* parent, obs_source_t* filter, void* param) {
                const char* filterName = obs_source_get_name(filter);
                obs_source_filter_remove(parent, filter);
                blog(LOG_INFO, "[Recovery] Removed filter: %s", filterName);
            };

            obs_source_enum_filters(source, removeAllFilters, nullptr);

            obs_source_release(source);
            blog(LOG_INFO, "[Recovery] Main source '%s' fully reset", m_mainSourceName.c_str());
        }
    }

    blog(LOG_INFO, "[Recovery] All obstructions cleared and screen fully restored");
}

void ObstructionManager::SaveOriginalTransform(obs_sceneitem_t* sceneItem) {
    if (!sceneItem || m_originalTransformSaved) return;

    obs_sceneitem_get_scale(sceneItem, &m_originalScale);
    obs_sceneitem_get_pos(sceneItem, &m_originalPos);
    m_originalRotation = obs_sceneitem_get_rot(sceneItem);

    m_originalTransformSaved = true;

    blog(LOG_INFO, "[Obstruction] Saved original transform: scale=(%.2f, %.2f), pos=(%.2f, %.2f), rot=%.2f",
         m_originalScale.x, m_originalScale.y, m_originalPos.x, m_originalPos.y, m_originalRotation);
}

void ObstructionManager::SetMainSourceName(const std::string& name) {
    m_mainSourceName = name;
    m_originalTransformSaved = false;  // Reset when changing main source

    // Save the original transform immediately
    if (!name.empty()) {
        obs_source_t* source = FindSourceByName(name);
        if (source) {
            obs_sceneitem_t* sceneItem = FindSceneItemForSource(source);
            if (sceneItem) {
                SaveOriginalTransform(sceneItem);
            }
            obs_source_release(source);
        }
    }

    blog(LOG_INFO, "[Obstruction] Main source set to: %s", name.c_str());
}

void ObstructionManager::SetObstructionAssetPath(const std::string& path) {
    m_assetPath = path;
    blog(LOG_INFO, "[Obstruction] Asset path set to: %s", path.c_str());
}

int ObstructionManager::GetActiveObstructionCount() const {
    return static_cast<int>(std::count_if(m_obstructions.begin(), m_obstructions.end(),
                                          [](const ObstructionSource& o) { return o.active; }));
}

obs_source_t* ObstructionManager::FindSourceByName(const std::string& name) {
    return obs_get_source_by_name(name.c_str());
}

obs_sceneitem_t* ObstructionManager::FindSceneItemForSource(obs_source_t* source) {
    obs_source_t* sceneSource = obs_frontend_get_current_scene();
    if (!sceneSource) return nullptr;

    obs_scene_t* currentScene = obs_scene_from_source(sceneSource);
    if (!currentScene) {
        obs_source_release(sceneSource);
        return nullptr;
    }

    obs_sceneitem_t* item = obs_scene_find_source(currentScene, obs_source_get_name(source));

    obs_source_release(sceneSource);
    return item;
}

void ObstructionManager::UpdateSourceTransform(obs_source_t* source, double scale) {
    obs_sceneitem_t* sceneItem = FindSceneItemForSource(source);
    if (!sceneItem) {
        blog(LOG_WARNING, "[Obstruction] Scene item not found for source");
        return;
    }

    // Save original transform on first use
    if (!m_originalTransformSaved) {
        SaveOriginalTransform(sceneItem);
    }

    // Update scale using new API
    struct vec2 scaleVec;
    scaleVec.x = static_cast<float>(scale);
    scaleVec.y = static_cast<float>(scale);
    obs_sceneitem_set_scale(sceneItem, &scaleVec);
}

std::string ObstructionManager::SelectRandomObstructionAsset() {
    if (m_assetPath.empty()) {
        // Use default obstruction (solid color overlay)
        return "builtin:color";
    }

    // List files in asset directory
    std::vector<std::string> assets;
    try {
        for (const auto& entry : fs::directory_iterator(m_assetPath)) {
            if (entry.is_regular_file()) {
                std::string ext = entry.path().extension().string();
                std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

                // Check for image/video formats
                if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp" ||
                    ext == ".gif" || ext == ".mp4" || ext == ".webm" || ext == ".mov" || ext == ".avi") {
                    assets.push_back(entry.path().string());
                }
            }
        }
    } catch (const fs::filesystem_error& e) {
        blog(LOG_ERROR, "[Obstruction] Failed to read asset directory: %s", e.what());
        return "builtin:color";
    }

    if (assets.empty()) {
        return "builtin:color";
    }

    // Select random asset
    std::uniform_int_distribution<size_t> dist(0, assets.size() - 1);
    return assets[dist(m_randomEngine)];
}

void ObstructionManager::CreateObstructionSource(const std::string& assetPath, double intensity) {
    obs_source_t* source = nullptr;
    std::string type;

    if (assetPath == "builtin:color") {
        // Create color source
        obs_data_t* settings = obs_data_create();
        obs_data_set_int(settings, "color", 0xFF0000FF);  // Red color
        obs_data_set_int(settings, "width", 200);
        obs_data_set_int(settings, "height", 200);

        source = obs_source_create("color_source", "Obstruction", settings, nullptr);
        type = "color";
        obs_data_release(settings);
    } else {
        // Determine if it's image or video
        std::string ext = fs::path(assetPath).extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

        obs_data_t* settings = obs_data_create();
        obs_data_set_string(settings, "file", assetPath.c_str());

        if (ext == ".mp4" || ext == ".webm" || ext == ".gif" || ext == ".mov" || ext == ".avi") {
            // Use ffmpeg_source for videos and animated GIFs
            obs_data_set_bool(settings, "looping", true);
            obs_data_set_bool(settings, "is_local_file", true);
            source = obs_source_create("ffmpeg_source", "Obstruction Video", settings, nullptr);
            type = "video";
            blog(LOG_INFO, "[Obstruction] Creating video source: %s", assetPath.c_str());
        } else {
            // Use image_source for static images
            obs_data_set_string(settings, "unload", "false");
            source = obs_source_create("image_source", "Obstruction Image", settings, nullptr);
            type = "image";
            blog(LOG_INFO, "[Obstruction] Creating image source: %s", assetPath.c_str());
        }

        obs_data_release(settings);
    }

    if (!source) {
        blog(LOG_ERROR, "[Obstruction] Failed to create obstruction source");
        return;
    }

    // Add to current scene
    obs_source_t* sceneSource = obs_frontend_get_current_scene();
    if (sceneSource) {
        obs_scene_t* currentScene = obs_scene_from_source(sceneSource);
        if (!currentScene) {
            obs_source_release(sceneSource);
            obs_source_release(source);
            return;
        }

        obs_sceneitem_t* sceneItem = obs_scene_add(currentScene, source);

        // Position randomly on screen
        std::uniform_int_distribution<int> xDist(0, 1920 - 200);
        std::uniform_int_distribution<int> yDist(0, 1080 - 200);

        struct vec2 pos;
        vec2_set(&pos, static_cast<float>(xDist(m_randomEngine)),
                       static_cast<float>(yDist(m_randomEngine)));

        obs_sceneitem_set_pos(sceneItem, &pos);

        // Set scale based on intensity
        // For image/video overlay: intensity is imageScale / 100.0 (e.g., 1.0 = 100%, 1.5 = 150%)
        // For other effects: intensity is 0.0-1.0 range
        struct vec2 scale;
        float scaleValue = static_cast<float>(intensity);
        if (scaleValue < 0.1f) {
            // If intensity is very small, use it as 0-1 range and scale accordingly
            scaleValue = 0.5f + scaleValue * 0.5f;  // 0.5 to 1.0
        }
        vec2_set(&scale, scaleValue, scaleValue);
        obs_sceneitem_set_scale(sceneItem, &scale);

        blog(LOG_INFO, "[Obstruction] Set scale to %.2f for %s", scaleValue, type.c_str());

        obs_source_release(sceneSource);
    }

    // Store obstruction info
    ObstructionSource obstruction;
    obstruction.source = source;
    obstruction.type = type;
    obstruction.intensity = intensity;
    obstruction.active = true;

    m_obstructions.push_back(obstruction);

    blog(LOG_INFO, "[Obstruction] Created %s obstruction (total: %d)",
            type.c_str(), GetActiveObstructionCount());
}

void ObstructionManager::RemoveObstructionSource(ObstructionSource& obstruction) {
    if (!obstruction.active || !obstruction.source) return;

    // Remove from scene
    obs_source_t* sceneSource = obs_frontend_get_current_scene();
    if (sceneSource) {
        obs_scene_t* currentScene = obs_scene_from_source(sceneSource);
        if (currentScene) {
            obs_sceneitem_t* sceneItem = obs_scene_find_source(currentScene,
                                                               obs_source_get_name(obstruction.source));
            if (sceneItem) {
                obs_sceneitem_remove(sceneItem);
            }
        }
        obs_source_release(sceneSource);
    }

    // Release source
    obs_source_release(obstruction.source);
    obstruction.source = nullptr;
    obstruction.active = false;
}
