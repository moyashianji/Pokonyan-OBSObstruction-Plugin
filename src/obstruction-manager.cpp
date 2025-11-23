#include "obstruction-manager.hpp"
#include <obs-module.h>
#include <graphics/vec2.h>
#include <graphics/matrix4.h>
#include <util/platform.h>
#include <filesystem>
#include <algorithm>
#include <cmath>

namespace fs = std::filesystem;

ObstructionManager::ObstructionManager()
    : m_currentShrinkPercentage(0.0)
    , m_enabled(true)
    , m_randomEngine(std::random_device{}())
{
}

ObstructionManager::~ObstructionManager() {
    ClearAllObstructions();
}

void ObstructionManager::ApplyObstruction(double amount) {
    if (!m_enabled) return;

    obs_log(LOG_INFO, "[Obstruction] Applying obstruction for amount: %.2f JPY", amount);

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
}

void ObstructionManager::ShrinkMainSource(double percentage) {
    if (m_mainSourceName.empty()) {
        obs_log(LOG_WARNING, "[Obstruction] Main source name not set");
        return;
    }

    obs_source_t* source = FindSourceByName(m_mainSourceName);
    if (!source) {
        obs_log(LOG_WARNING, "[Obstruction] Main source not found: %s", m_mainSourceName.c_str());
        return;
    }

    // Accumulate shrink percentage (but cap at 80%)
    m_currentShrinkPercentage = std::min(m_currentShrinkPercentage + percentage, 80.0);

    // Calculate new scale
    double scale = 1.0 - (m_currentShrinkPercentage / 100.0);

    UpdateSourceTransform(source, scale);

    obs_log(LOG_INFO, "[Obstruction] Shrunk main source to %.1f%% (total shrink: %.1f%%)",
            scale * 100.0, m_currentShrinkPercentage);

    obs_source_release(source);
}

void ObstructionManager::AddRandomObstruction(double intensity) {
    std::string assetPath = SelectRandomObstructionAsset();
    if (assetPath.empty()) {
        obs_log(LOG_WARNING, "[Obstruction] No obstruction assets found");
        return;
    }

    CreateObstructionSource(assetPath, intensity);
}

void ObstructionManager::ApplyRecovery(double amount) {
    if (!m_enabled) return;

    obs_log(LOG_INFO, "[Recovery] Applying recovery for amount: %.2f JPY", amount);

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

    obs_log(LOG_INFO, "[Recovery] Expanded main source to %.1f%% (remaining shrink: %.1f%%)",
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
        obs_log(LOG_INFO, "[Recovery] No active obstructions to remove");
        return;
    }

    // Select random obstruction
    std::uniform_int_distribution<size_t> dist(0, activeIndices.size() - 1);
    size_t indexToRemove = activeIndices[dist(m_randomEngine)];

    RemoveObstructionSource(m_obstructions[indexToRemove]);
    m_obstructions.erase(m_obstructions.begin() + indexToRemove);

    obs_log(LOG_INFO, "[Recovery] Removed obstruction (%zu active remaining)",
            GetActiveObstructionCount());
}

void ObstructionManager::ClearAllObstructions() {
    obs_log(LOG_INFO, "[Recovery] Clearing all obstructions");

    for (auto& obstruction : m_obstructions) {
        RemoveObstructionSource(obstruction);
    }
    m_obstructions.clear();

    // Reset main source scale
    m_currentShrinkPercentage = 0.0;
    if (!m_mainSourceName.empty()) {
        obs_source_t* source = FindSourceByName(m_mainSourceName);
        if (source) {
            UpdateSourceTransform(source, 1.0);
            obs_source_release(source);
        }
    }
}

void ObstructionManager::SetMainSourceName(const std::string& name) {
    m_mainSourceName = name;
    obs_log(LOG_INFO, "[Obstruction] Main source set to: %s", name.c_str());
}

void ObstructionManager::SetObstructionAssetPath(const std::string& path) {
    m_assetPath = path;
    obs_log(LOG_INFO, "[Obstruction] Asset path set to: %s", path.c_str());
}

int ObstructionManager::GetActiveObstructionCount() const {
    return static_cast<int>(std::count_if(m_obstructions.begin(), m_obstructions.end(),
                                          [](const ObstructionSource& o) { return o.active; }));
}

obs_source_t* ObstructionManager::FindSourceByName(const std::string& name) {
    return obs_get_source_by_name(name.c_str());
}

obs_sceneitem_t* ObstructionManager::FindSceneItemForSource(obs_source_t* source) {
    obs_scene_t* currentScene = obs_frontend_get_current_scene();
    if (!currentScene) return nullptr;

    obs_source_t* sceneSource = obs_scene_get_source(currentScene);
    obs_sceneitem_t* item = obs_scene_find_source(currentScene, obs_source_get_name(source));

    obs_source_release(sceneSource);
    return item;
}

void ObstructionManager::UpdateSourceTransform(obs_source_t* source, double scale) {
    obs_sceneitem_t* sceneItem = FindSceneItemForSource(source);
    if (!sceneItem) {
        obs_log(LOG_WARNING, "[Obstruction] Scene item not found for source");
        return;
    }

    // Get current transform
    struct obs_transform_info info;
    obs_sceneitem_get_info(sceneItem, &info);

    // Update scale
    info.scale.x = static_cast<float>(scale);
    info.scale.y = static_cast<float>(scale);

    // Center the source
    obs_sceneitem_set_info(sceneItem, &info);
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
                if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" ||
                    ext == ".gif" || ext == ".mp4" || ext == ".webm") {
                    assets.push_back(entry.path().string());
                }
            }
        }
    } catch (const fs::filesystem_error& e) {
        obs_log(LOG_ERROR, "[Obstruction] Failed to read asset directory: %s", e.what());
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

        if (ext == ".mp4" || ext == ".webm") {
            obs_data_set_bool(settings, "looping", true);
            source = obs_source_create("ffmpeg_source", "Obstruction Video", settings, nullptr);
            type = "video";
        } else {
            source = obs_source_create("image_source", "Obstruction Image", settings, nullptr);
            type = "image";
        }

        obs_data_release(settings);
    }

    if (!source) {
        obs_log(LOG_ERROR, "[Obstruction] Failed to create obstruction source");
        return;
    }

    // Add to current scene
    obs_scene_t* currentScene = obs_frontend_get_current_scene();
    if (currentScene) {
        obs_sceneitem_t* sceneItem = obs_scene_add(currentScene, source);

        // Position randomly on screen
        std::uniform_int_distribution<int> xDist(0, 1920 - 200);
        std::uniform_int_distribution<int> yDist(0, 1080 - 200);

        struct vec2 pos;
        vec2_set(&pos, static_cast<float>(xDist(m_randomEngine)),
                       static_cast<float>(yDist(m_randomEngine)));

        obs_sceneitem_set_pos(sceneItem, &pos);

        // Set scale based on intensity
        struct vec2 scale;
        float scaleValue = 0.5f + static_cast<float>(intensity) * 0.5f;  // 0.5 to 1.0
        vec2_set(&scale, scaleValue, scaleValue);
        obs_sceneitem_set_scale(sceneItem, &scale);
    }

    // Store obstruction info
    ObstructionSource obstruction;
    obstruction.source = source;
    obstruction.type = type;
    obstruction.intensity = intensity;
    obstruction.active = true;

    m_obstructions.push_back(obstruction);

    obs_log(LOG_INFO, "[Obstruction] Created %s obstruction (total: %d)",
            type.c_str(), GetActiveObstructionCount());
}

void ObstructionManager::RemoveObstructionSource(ObstructionSource& obstruction) {
    if (!obstruction.active || !obstruction.source) return;

    // Remove from scene
    obs_scene_t* currentScene = obs_frontend_get_current_scene();
    if (currentScene) {
        obs_sceneitem_t* sceneItem = obs_scene_find_source(currentScene,
                                                           obs_source_get_name(obstruction.source));
        if (sceneItem) {
            obs_sceneitem_remove(sceneItem);
        }
    }

    // Release source
    obs_source_release(obstruction.source);
    obstruction.source = nullptr;
    obstruction.active = false;
}
