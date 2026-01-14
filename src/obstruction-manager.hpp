#pragma once

#include <obs.h>
#include <vector>
#include <string>
#include <random>
#include <memory>

// Forward declarations
class EffectManager;
struct EffectSettings;

struct ObstructionSource {
    obs_source_t* source;
    std::string type;      // "image" or "video"
    double intensity;      // 0.0 to 1.0
    bool active;
};

class ObstructionManager {
public:
    ObstructionManager();
    ~ObstructionManager();

    // Obstruction effects (from Super Chat)
    void ApplyObstruction(double amount);
    void ShrinkMainSource(double percentage);
    void AddRandomObstruction(double intensity);

    // NEW: Apply specific effect based on configuration
    void ApplyConfiguredEffect(const EffectSettings& config);

    // Recovery effects (from Super Sticker)
    void ApplyRecovery(double amount);
    void ExpandMainSource(double percentage);
    void RemoveRandomObstruction();
    void ClearAllObstructions();

    // Configuration
    void SetMainSourceName(const std::string& name);
    void SetObstructionAssetPath(const std::string& path);
    void SetEnabled(bool enabled) { m_enabled = enabled; }

    // State
    double GetCurrentShrinkPercentage() const { return m_currentShrinkPercentage; }
    int GetActiveObstructionCount() const;

private:
    obs_source_t* FindSourceByName(const std::string& name);
    obs_sceneitem_t* FindSceneItemForSource(obs_source_t* source);
    void UpdateSourceTransform(obs_source_t* source, double scale);

    std::string SelectRandomObstructionAsset();
    void CreateObstructionSource(const std::string& assetPath, double intensity);
    void RemoveObstructionSource(ObstructionSource& obstruction);

    std::string m_mainSourceName;
    std::string m_assetPath;
    std::vector<ObstructionSource> m_obstructions;
    double m_currentShrinkPercentage;
    bool m_enabled;

    // Store original transform for reset
    bool m_originalTransformSaved;
    struct vec2 m_originalScale;
    struct vec2 m_originalPos;
    float m_originalRotation;

    std::mt19937 m_randomEngine;
    std::unique_ptr<EffectManager> m_effectManager;

    // Helper functions
    void SaveOriginalTransform(obs_sceneitem_t* sceneItem);
};
