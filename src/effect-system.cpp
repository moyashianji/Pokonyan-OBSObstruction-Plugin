#include "effect-system.hpp"
#include <obs-module.h>
#include <obs-frontend-api.h>
#include <obs-source.h>
#include <graphics/vec2.h>
#include <graphics/vec3.h>
#include <graphics/matrix4.h>
#include <cmath>
#include <random>
#include <algorithm>

// =============================================================================
// EffectBase Implementation
// =============================================================================

EffectBase::EffectBase(obs_source_t* source, double duration, QObject* parent)
    : QObject(parent)
    , m_source(source)
    , m_duration(duration)
    , m_elapsedTime(0.0)
    , m_isActive(false)
    , m_timer(new QTimer(this))
{
    // Note: Source reference is managed by the caller, no need to addref

    // 60 FPS update rate
    m_timer->setInterval(16); // ~60 FPS
    connect(m_timer, &QTimer::timeout, this, &EffectBase::OnTimerTick);
}

EffectBase::~EffectBase() {
    // Note: Source reference is managed by the caller, no need to release
}

void EffectBase::OnTimerTick() {
    m_elapsedTime += 0.016; // 16ms in seconds

    Update(m_elapsedTime);

    if (m_elapsedTime >= m_duration) {
        Stop();
    }
}

// =============================================================================
// RotationEffect Implementation
// =============================================================================

RotationEffect::RotationEffect(obs_source_t* source, double duration, double rotationsPerSecond,
                               int rotationType, bool reverse)
    : EffectBase(source, duration)
    , m_rotationsPerSecond(rotationsPerSecond)
    , m_currentRotation(0.0)
    , m_rotationType(rotationType)
    , m_reverse(reverse)
{
    vec2_zero(&m_originalPos);
    vec2_zero(&m_originalScale);
}

void RotationEffect::Start() {
    m_isActive = true;
    m_elapsedTime = 0.0;
    m_currentRotation = 0.0;

    // Save original transform
    obs_source_t* sceneSource = obs_frontend_get_current_scene();
    if (sceneSource) {
        obs_scene_t* scene = obs_scene_from_source(sceneSource);
        if (scene) {
            obs_sceneitem_t* sceneItem = obs_scene_find_source(scene, obs_source_get_name(m_source));
            if (sceneItem) {
                obs_sceneitem_get_pos(sceneItem, &m_originalPos);
                obs_sceneitem_get_scale(sceneItem, &m_originalScale);
            }
        }
        obs_source_release(sceneSource);
    }

    m_timer->start();

    const char* rotationTypeStr = (m_rotationType == 0) ? "Z軸" :
                                  (m_rotationType == 1) ? "X軸" :
                                  (m_rotationType == 2) ? "Y軸" : "全軸";
    blog(LOG_INFO, "[Effect] Rotation effect started (%.1fs, %.1f rot/s, %s%s)",
         m_duration, m_rotationsPerSecond, rotationTypeStr, m_reverse ? ", 逆回転" : "");
}

void RotationEffect::Stop() {
    m_isActive = false;
    m_timer->stop();

    // Restore original transform
    obs_source_t* sceneSource = obs_frontend_get_current_scene();
    if (sceneSource) {
        obs_scene_t* scene = obs_scene_from_source(sceneSource);
        if (scene) {
            obs_sceneitem_t* sceneItem = obs_scene_find_source(scene, obs_source_get_name(m_source));
            if (sceneItem) {
                obs_sceneitem_set_rot(sceneItem, 0.0f);
                obs_sceneitem_set_pos(sceneItem, &m_originalPos);
                obs_sceneitem_set_scale(sceneItem, &m_originalScale);
            }
        }
        obs_source_release(sceneSource);
    }

    blog(LOG_INFO, "[Effect] Rotation effect stopped");
}

void RotationEffect::Update(double elapsed) {
    // Calculate rotation angle (in degrees)
    double rotation = elapsed * m_rotationsPerSecond * 360.0;
    if (m_reverse) {
        rotation = -rotation;
    }

    obs_source_t* sceneSource = obs_frontend_get_current_scene();
    if (!sceneSource) return;

    obs_scene_t* scene = obs_scene_from_source(sceneSource);
    if (!scene) {
        obs_source_release(sceneSource);
        return;
    }

    obs_sceneitem_t* sceneItem = obs_scene_find_source(scene, obs_source_get_name(m_source));
    if (sceneItem) {
        // Apply rotation based on type
        switch (m_rotationType) {
            case 0: {  // Z軸回転（通常の2D回転）
                m_currentRotation = fmod(rotation, 360.0);
                obs_sceneitem_set_rot(sceneItem, static_cast<float>(m_currentRotation));
                break;
            }
            case 1: {  // X軸回転（上下の3D回転風）
                float angle = fmod(rotation, 360.0);
                float scaleY = 0.2f + 0.8f * fabs(cos(angle * 3.14159 / 180.0));
                struct vec2 scaleVec;
                scaleVec.x = m_originalScale.x;
                scaleVec.y = m_originalScale.y * scaleY;
                obs_sceneitem_set_scale(sceneItem, &scaleVec);
                break;
            }
            case 2: {  // Y軸回転（左右の3D回転風）
                float angle = fmod(rotation, 360.0);
                float scaleX = 0.2f + 0.8f * fabs(cos(angle * 3.14159 / 180.0));
                struct vec2 scaleVec;
                scaleVec.x = m_originalScale.x * scaleX;
                scaleVec.y = m_originalScale.y;
                obs_sceneitem_set_scale(sceneItem, &scaleVec);
                break;
            }
            case 3: {  // 全軸回転
                float angleZ = fmod(rotation, 360.0);
                float angleX = fmod(rotation * 0.7, 360.0);
                float angleY = fmod(rotation * 0.5, 360.0);

                float scaleX = 0.3f + 0.7f * fabs(cos(angleY * 3.14159 / 180.0));
                float scaleY = 0.3f + 0.7f * fabs(cos(angleX * 3.14159 / 180.0));

                struct vec2 scaleVec;
                scaleVec.x = m_originalScale.x * scaleX;
                scaleVec.y = m_originalScale.y * scaleY;
                obs_sceneitem_set_scale(sceneItem, &scaleVec);
                obs_sceneitem_set_rot(sceneItem, angleZ);
                break;
            }
        }
    }

    obs_source_release(sceneSource);
}

// =============================================================================
// BlinkEffect Implementation
// =============================================================================

BlinkEffect::BlinkEffect(obs_source_t* source, double duration, double blinkFrequency)
    : EffectBase(source, duration)
    , m_blinkFrequency(blinkFrequency)
    , m_isVisible(true)
{
}

void BlinkEffect::Start() {
    m_isActive = true;
    m_elapsedTime = 0.0;
    m_isVisible = true;
    m_timer->start();

    blog(LOG_INFO, "[Effect] Blink effect started (%.1fs, %.1f Hz)", m_duration, m_blinkFrequency);
}

void BlinkEffect::Stop() {
    m_isActive = false;
    m_timer->stop();

    // Reset visibility
    obs_source_t* sceneSource = obs_frontend_get_current_scene();
    if (!sceneSource) return;

    obs_scene_t* scene = obs_scene_from_source(sceneSource);
    if (!scene) {
        obs_source_release(sceneSource);
        return;
    }

    obs_sceneitem_t* sceneItem = obs_scene_find_source(scene, obs_source_get_name(m_source));
    if (sceneItem) {
        obs_sceneitem_set_visible(sceneItem, true);
    }

    obs_source_release(sceneSource);
    blog(LOG_INFO, "[Effect] Blink effect stopped");
}

void BlinkEffect::Update(double elapsed) {
    // Toggle visibility based on frequency
    double cycleTime = 1.0 / m_blinkFrequency;
    double phase = fmod(elapsed, cycleTime);
    bool shouldBeVisible = (phase < cycleTime / 2.0);

    if (shouldBeVisible != m_isVisible) {
        m_isVisible = shouldBeVisible;

        obs_source_t* sceneSource = obs_frontend_get_current_scene();
        if (!sceneSource) return;

        obs_scene_t* scene = obs_scene_from_source(sceneSource);
        if (!scene) {
            obs_source_release(sceneSource);
            return;
        }

        obs_sceneitem_t* sceneItem = obs_scene_find_source(scene, obs_source_get_name(m_source));
        if (sceneItem) {
            obs_sceneitem_set_visible(sceneItem, m_isVisible);
        }

        obs_source_release(sceneSource);
    }
}

// =============================================================================
// HueShiftEffect Implementation
// =============================================================================

HueShiftEffect::HueShiftEffect(obs_source_t* source, double duration, double shiftSpeed)
    : EffectBase(source, duration)
    , m_shiftSpeed(shiftSpeed)
    , m_currentHue(0.0)
    , m_colorFilter(nullptr)
{
}

void HueShiftEffect::Start() {
    m_isActive = true;
    m_elapsedTime = 0.0;
    m_currentHue = 0.0;

    // Create color correction filter
    obs_data_t* settings = obs_data_create();
    obs_data_set_double(settings, "hue_shift", 0.0);

    m_colorFilter = obs_source_create("color_filter", "temp_hue_shift_filter", settings, nullptr);
    if (m_colorFilter) {
        obs_source_filter_add(m_source, m_colorFilter);
        blog(LOG_INFO, "[Effect] Hue shift effect started (%.1fs, %.1f deg/s)", m_duration, m_shiftSpeed);
    } else {
        blog(LOG_WARNING, "[Effect] Failed to create color filter for hue shift effect");
        m_isActive = false;
    }

    obs_data_release(settings);

    if (m_isActive) {
        m_timer->start();
    }
}

void HueShiftEffect::Stop() {
    m_isActive = false;
    m_timer->stop();

    // Remove color correction filter if added
    if (m_colorFilter) {
        obs_source_filter_remove(m_source, m_colorFilter);
        obs_source_release(m_colorFilter);
        m_colorFilter = nullptr;
    }

    blog(LOG_INFO, "[Effect] Hue shift effect stopped");
}

void HueShiftEffect::Update(double elapsed) {
    if (!m_colorFilter) return;

    // Calculate hue shift (0-360 degrees, map to -180 to 180 for filter)
    m_currentHue = fmod(elapsed * m_shiftSpeed, 360.0);
    double hueShiftValue = m_currentHue - 180.0;  // Center around 0

    // Update the color filter settings
    obs_data_t* settings = obs_source_get_settings(m_colorFilter);
    obs_data_set_double(settings, "hue_shift", hueShiftValue);
    obs_source_update(m_colorFilter, settings);
    obs_data_release(settings);
}

// =============================================================================
// ShakeEffect Implementation
// =============================================================================

ShakeEffect::ShakeEffect(obs_source_t* source, double duration, double intensity)
    : EffectBase(source, duration)
    , m_intensity(intensity)
    , m_randomEngine(std::random_device{}())
{
    vec2_zero(&m_originalPos);
}

void ShakeEffect::Start() {
    m_isActive = true;
    m_elapsedTime = 0.0;
    m_timer->start();

    // Store original position
    obs_source_t* sceneSource = obs_frontend_get_current_scene();
    if (!sceneSource) return;

    obs_scene_t* scene = obs_scene_from_source(sceneSource);
    if (!scene) {
        obs_source_release(sceneSource);
        return;
    }

    obs_sceneitem_t* sceneItem = obs_scene_find_source(scene, obs_source_get_name(m_source));
    if (sceneItem) {
        obs_sceneitem_get_pos(sceneItem, &m_originalPos);
    }

    obs_source_release(sceneSource);

    blog(LOG_INFO, "[Effect] Shake effect started (%.1fs, intensity: %.1f)", m_duration, m_intensity);
}

void ShakeEffect::Stop() {
    m_isActive = false;
    m_timer->stop();

    // Restore original position
    obs_source_t* sceneSource = obs_frontend_get_current_scene();
    if (!sceneSource) return;

    obs_scene_t* scene = obs_scene_from_source(sceneSource);
    if (!scene) {
        obs_source_release(sceneSource);
        return;
    }

    obs_sceneitem_t* sceneItem = obs_scene_find_source(scene, obs_source_get_name(m_source));
    if (sceneItem) {
        obs_sceneitem_set_pos(sceneItem, &m_originalPos);
    }

    obs_source_release(sceneSource);

    blog(LOG_INFO, "[Effect] Shake effect stopped");
}

void ShakeEffect::Update(double elapsed) {
    std::uniform_real_distribution<float> dist(-static_cast<float>(m_intensity),
                                               static_cast<float>(m_intensity));

    vec2 shakeyPos;
    shakeyPos.x = m_originalPos.x + dist(m_randomEngine);
    shakeyPos.y = m_originalPos.y + dist(m_randomEngine);

    obs_source_t* sceneSource = obs_frontend_get_current_scene();
    if (!sceneSource) return;

    obs_scene_t* scene = obs_scene_from_source(sceneSource);
    if (!scene) {
        obs_source_release(sceneSource);
        return;
    }

    obs_sceneitem_t* sceneItem = obs_scene_find_source(scene, obs_source_get_name(m_source));
    if (sceneItem) {
        obs_sceneitem_set_pos(sceneItem, &shakeyPos);
    }

    obs_source_release(sceneSource);
}

// =============================================================================
// KaleidoscopeEffect Implementation (Simplified)
// =============================================================================

KaleidoscopeEffect::KaleidoscopeEffect(obs_source_t* source, double duration, int segments)
    : EffectBase(source, duration)
    , m_segments(segments)
{
    vec2_zero(&m_originalPos);
    vec2_zero(&m_originalScale);
    m_originalRot = 0.0f;
}

void KaleidoscopeEffect::Start() {
    m_isActive = true;
    m_elapsedTime = 0.0;

    // Save original transform
    obs_source_t* sceneSource = obs_frontend_get_current_scene();
    if (sceneSource) {
        obs_scene_t* scene = obs_scene_from_source(sceneSource);
        if (scene) {
            obs_sceneitem_t* sceneItem = obs_scene_find_source(scene, obs_source_get_name(m_source));
            if (sceneItem) {
                obs_sceneitem_get_pos(sceneItem, &m_originalPos);
                obs_sceneitem_get_scale(sceneItem, &m_originalScale);
                m_originalRot = obs_sceneitem_get_rot(sceneItem);
                blog(LOG_INFO, "[Effect] Saved kaleidoscope original transform: pos=(%.2f,%.2f), scale=(%.2f,%.2f), rot=%.2f",
                     m_originalPos.x, m_originalPos.y, m_originalScale.x, m_originalScale.y, m_originalRot);
            }
        }
        obs_source_release(sceneSource);
    }

    m_timer->start();
    blog(LOG_INFO, "[Effect] Kaleidoscope effect started (%d segments)", m_segments);
}

void KaleidoscopeEffect::Stop() {
    m_isActive = false;
    m_timer->stop();

    // Restore original transform
    obs_source_t* sceneSource = obs_frontend_get_current_scene();
    if (sceneSource) {
        obs_scene_t* scene = obs_scene_from_source(sceneSource);
        if (scene) {
            obs_sceneitem_t* sceneItem = obs_scene_find_source(scene, obs_source_get_name(m_source));
            if (sceneItem) {
                obs_sceneitem_set_pos(sceneItem, &m_originalPos);
                obs_sceneitem_set_scale(sceneItem, &m_originalScale);
                obs_sceneitem_set_rot(sceneItem, m_originalRot);
                blog(LOG_INFO, "[Effect] Restored kaleidoscope original transform");
            }
        }
        obs_source_release(sceneSource);
    }

    // Clean up mirror sources
    for (auto* mirrorSource : m_mirrorSources) {
        if (mirrorSource) {
            obs_source_release(mirrorSource);
        }
    }
    m_mirrorSources.clear();

    blog(LOG_INFO, "[Effect] Kaleidoscope effect stopped");
}

void KaleidoscopeEffect::Update(double elapsed) {
    // Kaleidoscope effect: Rapid rotation combined with scaling
    // This creates a visual effect similar to a kaleidoscope

    obs_source_t* sceneSource = obs_frontend_get_current_scene();
    if (!sceneSource) return;

    obs_scene_t* scene = obs_scene_from_source(sceneSource);
    if (!scene) {
        obs_source_release(sceneSource);
        return;
    }

    obs_sceneitem_t* sceneItem = obs_scene_find_source(scene, obs_source_get_name(m_source));
    if (sceneItem) {
        // Rapid rotation for kaleidoscope effect
        float rotation = fmod(elapsed * 360.0, 360.0); // Full rotation per second
        obs_sceneitem_set_rot(sceneItem, rotation);

        // Pulsating scale for extra visual effect
        float scale = 0.9f + 0.2f * sin(elapsed * 3.14159 * 2.0); // Pulse between 0.9 and 1.1
        struct vec2 scaleVec;
        scaleVec.x = scale;
        scaleVec.y = scale;
        obs_sceneitem_set_scale(sceneItem, &scaleVec);
    }

    obs_source_release(sceneSource);
}

// =============================================================================
// Rotation3DEffect Implementation
// =============================================================================

Rotation3DEffect::Rotation3DEffect(obs_source_t* source, double duration, bool rotateX, bool rotateY)
    : EffectBase(source, duration)
    , m_rotateX(rotateX)
    , m_rotateY(rotateY)
    , m_angleX(0.0)
    , m_angleY(0.0)
{
    vec2_zero(&m_originalPos);
    vec2_zero(&m_originalScale);
    m_originalRot = 0.0f;
}

void Rotation3DEffect::Start() {
    m_isActive = true;
    m_elapsedTime = 0.0;
    m_angleX = 0.0;
    m_angleY = 0.0;

    // Save original transform
    obs_source_t* sceneSource = obs_frontend_get_current_scene();
    if (sceneSource) {
        obs_scene_t* scene = obs_scene_from_source(sceneSource);
        if (scene) {
            obs_sceneitem_t* sceneItem = obs_scene_find_source(scene, obs_source_get_name(m_source));
            if (sceneItem) {
                obs_sceneitem_get_pos(sceneItem, &m_originalPos);
                obs_sceneitem_get_scale(sceneItem, &m_originalScale);
                m_originalRot = obs_sceneitem_get_rot(sceneItem);
                blog(LOG_INFO, "[Effect] Saved 3D rotation original transform: pos=(%.2f,%.2f), scale=(%.2f,%.2f), rot=%.2f",
                     m_originalPos.x, m_originalPos.y, m_originalScale.x, m_originalScale.y, m_originalRot);
            }
        }
        obs_source_release(sceneSource);
    }

    m_timer->start();
    blog(LOG_INFO, "[Effect] 3D Rotation effect started (X: %d, Y: %d)", m_rotateX, m_rotateY);
}

void Rotation3DEffect::Stop() {
    m_isActive = false;
    m_timer->stop();

    // Restore original transform completely
    obs_source_t* sceneSource = obs_frontend_get_current_scene();
    if (sceneSource) {
        obs_scene_t* scene = obs_scene_from_source(sceneSource);
        if (scene) {
            obs_sceneitem_t* sceneItem = obs_scene_find_source(scene, obs_source_get_name(m_source));
            if (sceneItem) {
                obs_sceneitem_set_pos(sceneItem, &m_originalPos);
                obs_sceneitem_set_scale(sceneItem, &m_originalScale);
                obs_sceneitem_set_rot(sceneItem, m_originalRot);
                blog(LOG_INFO, "[Effect] Restored 3D rotation original transform");
            }
        }
        obs_source_release(sceneSource);
    }

    blog(LOG_INFO, "[Effect] 3D Rotation effect stopped");
}

void Rotation3DEffect::Update(double elapsed) {
    // Update rotation angles
    if (m_rotateX) {
        m_angleX = fmod(elapsed * 90.0, 360.0); // 90 degrees per second
    }
    if (m_rotateY) {
        m_angleY = fmod(elapsed * 120.0, 360.0); // 120 degrees per second
    }

    // Simulate 3D rotation using scale transformations
    // This creates a perspective effect similar to 3D rotation
    obs_source_t* sceneSource = obs_frontend_get_current_scene();
    if (!sceneSource) return;

    obs_scene_t* scene = obs_scene_from_source(sceneSource);
    if (!scene) {
        obs_source_release(sceneSource);
        return;
    }

    obs_sceneitem_t* sceneItem = obs_scene_find_source(scene, obs_source_get_name(m_source));
    if (sceneItem) {
        // Simulate 3D rotation with scale changes
        float scaleX = 1.0f;
        float scaleY = 1.0f;

        if (m_rotateY) {
            // Y-axis rotation: compress horizontally when facing sideways
            scaleX = 0.2f + 0.8f * fabs(cos(m_angleY * 3.14159 / 180.0));
        }

        if (m_rotateX) {
            // X-axis rotation: compress vertically when facing up/down
            scaleY = 0.2f + 0.8f * fabs(cos(m_angleX * 3.14159 / 180.0));
        }

        struct vec2 scaleVec;
        scaleVec.x = scaleX;
        scaleVec.y = scaleY;
        obs_sceneitem_set_scale(sceneItem, &scaleVec);

        // Also add Z-axis rotation for more 3D feel
        float rotation = fmod(elapsed * 45.0, 360.0);
        obs_sceneitem_set_rot(sceneItem, rotation);
    }

    obs_source_release(sceneSource);
}

// =============================================================================
// RandomShapesEffect Implementation
// =============================================================================

RandomShapesEffect::RandomShapesEffect(obs_source_t* source, double duration, int shapeCount)
    : EffectBase(source, duration)
    , m_shapeCount(shapeCount)
    , m_randomEngine(std::random_device{}())
    , m_shapeSource(nullptr)
{
}

void RandomShapesEffect::Start() {
    m_isActive = true;
    m_elapsedTime = 0.0;
    m_shapes.clear();

    // Use full screen dimensions for shapes (not source dimensions)
    uint32_t screenWidth = 1920;
    uint32_t screenHeight = 1080;

    // Initialize random shapes with actual color sources across entire screen
    std::uniform_real_distribution<float> posXDist(0.0f, static_cast<float>(screenWidth));
    std::uniform_real_distribution<float> posYDist(0.0f, static_cast<float>(screenHeight));
    std::uniform_real_distribution<float> velDist(-100.0f, 100.0f);
    std::uniform_real_distribution<float> sizeDist(30.0f, 80.0f);
    std::uniform_int_distribution<int> typeDist(0, 2);
    std::uniform_int_distribution<uint32_t> colorDist(0x80000000, 0xFFFFFFFF);

    obs_source_t* sceneSource = obs_frontend_get_current_scene();
    if (!sceneSource) {
        blog(LOG_WARNING, "[Effect] Cannot create random shapes - no current scene");
        return;
    }

    obs_scene_t* scene = obs_scene_from_source(sceneSource);
    if (!scene) {
        obs_source_release(sceneSource);
        return;
    }

    for (int i = 0; i < m_shapeCount; ++i) {
        Shape shape;
        shape.position.x = posXDist(m_randomEngine);
        shape.position.y = posYDist(m_randomEngine);
        shape.velocity.x = velDist(m_randomEngine);
        shape.velocity.y = velDist(m_randomEngine);
        shape.size = sizeDist(m_randomEngine);
        shape.color = colorDist(m_randomEngine) | 0xFF000000; // Ensure alpha
        shape.type = typeDist(m_randomEngine);

        // Create a color source for this shape
        obs_data_t* settings = obs_data_create();
        obs_data_set_int(settings, "color", shape.color);
        obs_data_set_int(settings, "width", static_cast<int>(shape.size));
        obs_data_set_int(settings, "height", static_cast<int>(shape.size));

        char shapeName[64];
        snprintf(shapeName, sizeof(shapeName), "random_shape_%d", i);

        // Try v3 first, then fallback to regular color_source
        obs_source_t* colorSource = obs_source_create("color_source_v3", shapeName, settings, nullptr);
        if (!colorSource) {
            colorSource = obs_source_create("color_source", shapeName, settings, nullptr);
        }
        obs_data_release(settings);

        if (colorSource) {
            obs_sceneitem_t* item = obs_scene_add(scene, colorSource);
            if (item) {
                // Set initial position
                vec2 pos;
                pos.x = shape.position.x;
                pos.y = shape.position.y;
                obs_sceneitem_set_pos(item, &pos);

                // Set blend mode for nice visual effect
                obs_sceneitem_set_blending_mode(item, OBS_BLEND_ADDITIVE);

                blog(LOG_INFO, "[Effect] Created random shape %d at (%.1f, %.1f) size=%.1f color=0x%08X",
                     i, shape.position.x, shape.position.y, shape.size, shape.color);
            }
            // Release the creation reference (scene now owns it)
            obs_source_release(colorSource);
        } else {
            blog(LOG_WARNING, "[Effect] Failed to create color source for random shape %d", i);
        }

        m_shapes.push_back(shape);
    }

    obs_source_release(sceneSource);

    m_timer->start();
    blog(LOG_INFO, "[Effect] Random shapes effect started (%d shapes)", m_shapeCount);
}

void RandomShapesEffect::Stop() {
    m_isActive = false;
    m_timer->stop();

    // Remove all shape sources from scene
    obs_source_t* sceneSource = obs_frontend_get_current_scene();
    if (sceneSource) {
        obs_scene_t* scene = obs_scene_from_source(sceneSource);
        if (scene) {
            for (int i = 0; i < static_cast<int>(m_shapes.size()); ++i) {
                char shapeName[64];
                snprintf(shapeName, sizeof(shapeName), "random_shape_%d", i);

                obs_sceneitem_t* item = obs_scene_find_source(scene, shapeName);
                if (item) {
                    obs_sceneitem_remove(item);
                }
            }
        }
        obs_source_release(sceneSource);
    }

    m_shapes.clear();
    blog(LOG_INFO, "[Effect] Random shapes effect stopped");
}

void RandomShapesEffect::Update(double elapsed) {
    // Use full screen dimensions for boundary checking
    uint32_t screenWidth = 1920;
    uint32_t screenHeight = 1080;

    obs_source_t* sceneSource = obs_frontend_get_current_scene();
    if (!sceneSource) return;

    obs_scene_t* scene = obs_scene_from_source(sceneSource);
    if (!scene) {
        obs_source_release(sceneSource);
        return;
    }

    // Update shape positions
    for (size_t i = 0; i < m_shapes.size(); ++i) {
        auto& shape = m_shapes[i];

        shape.position.x += shape.velocity.x * 0.016f;
        shape.position.y += shape.velocity.y * 0.016f;

        // Bounce off edges of entire screen
        if (shape.position.x < 0 || shape.position.x > screenWidth) {
            shape.velocity.x *= -1.0f;
            shape.position.x = std::clamp(shape.position.x, 0.0f, static_cast<float>(screenWidth));
        }
        if (shape.position.y < 0 || shape.position.y > screenHeight) {
            shape.velocity.y *= -1.0f;
            shape.position.y = std::clamp(shape.position.y, 0.0f, static_cast<float>(screenHeight));
        }

        // Update the color source position in scene
        char shapeName[64];
        snprintf(shapeName, sizeof(shapeName), "random_shape_%zu", i);
        obs_sceneitem_t* item = obs_scene_find_source(scene, shapeName);
        if (item) {
            vec2 pos;
            pos.x = shape.position.x;
            pos.y = shape.position.y;
            obs_sceneitem_set_pos(item, &pos);
        }
    }

    obs_source_release(sceneSource);
}

void RandomShapesEffect::CreateShapeSource() {
    // Implementation now in Start()
}

void RandomShapesEffect::UpdateShapeSource() {
    // Implementation now in Update()
}

// =============================================================================
// ParticleSystemEffect Implementation - Cinema-Quality Particle System
// =============================================================================

ParticleSystemEffect::ParticleSystemEffect(obs_source_t* source, double duration, int particleCount, int particleType)
    : EffectBase(source, duration)
    , m_particleCount(particleCount)
    , m_particleType(particleType)
    , m_randomEngine(std::random_device{}())
    , m_spawnTimer(0.0)
    , m_nextParticleIndex(0)
{
}

void ParticleSystemEffect::Start() {
    m_isActive = true;
    m_elapsedTime = 0.0;
    m_spawnTimer = 0.0;
    m_nextParticleIndex = 0;
    m_particles.clear();
    m_particleSources.clear();

    // Pre-allocate particles
    m_particles.resize(m_particleCount);
    m_particleSources.resize(m_particleCount, nullptr);

    // Get screen center for explosion effects
    vec2 screenCenter;
    screenCenter.x = 1920.0f / 2.0f;
    screenCenter.y = 1080.0f / 2.0f;

    // Initialize all particles based on type
    for (int i = 0; i < m_particleCount; ++i) {
        auto& particle = m_particles[i];

        switch (m_particleType) {
            case 0: { // Explosion
                InitExplosionParticle(particle, screenCenter);
                // Stagger spawning for cascading explosion effect
                particle.life = -static_cast<float>(i) * 0.01f;
                break;
            }
            case 1: { // Rain
                InitRainParticle(particle);
                // Randomize initial spawn time
                std::uniform_real_distribution<float> spawnDist(0.0f, 2.0f);
                particle.life = particle.maxLife * spawnDist(m_randomEngine);
                break;
            }
            case 2: { // Snow
                InitSnowParticle(particle);
                // Randomize initial spawn time
                std::uniform_real_distribution<float> snowSpawnDist(0.0f, 3.0f);
                particle.life = particle.maxLife * snowSpawnDist(m_randomEngine);
                break;
            }
            case 3: { // Stars
                InitStarParticle(particle);
                // Stagger twinkling
                std::uniform_real_distribution<float> starSpawnDist(0.0f, 2.0f);
                particle.life = particle.maxLife * starSpawnDist(m_randomEngine);
                break;
            }
        }

        particle.isActive = (particle.life >= 0.0f);
    }

    // Create visual sources for all particles
    obs_source_t* sceneSource = obs_frontend_get_current_scene();
    if (sceneSource) {
        obs_scene_t* scene = obs_scene_from_source(sceneSource);
        if (scene) {
            for (int i = 0; i < m_particleCount; ++i) {
                CreateParticleSource(i, m_particles[i]);
            }
        }
        obs_source_release(sceneSource);
    }

    m_timer->start();
    const char* typeStr = (m_particleType == 0) ? "爆発" :
                         (m_particleType == 1) ? "雨" :
                         (m_particleType == 2) ? "雪" : "星";
    blog(LOG_INFO, "[Effect] Cinema-quality particle system started: %s (%d particles)",
         typeStr, m_particleCount);
}

void ParticleSystemEffect::Stop() {
    m_isActive = false;
    m_timer->stop();

    RemoveAllParticleSources();
    m_particles.clear();
    m_particleSources.clear();

    blog(LOG_INFO, "[Effect] Particle system effect stopped");
}

void ParticleSystemEffect::Update(double elapsed) {
    const float deltaTime = 0.016f; // 60 FPS

    // Update all particles based on type
    for (size_t i = 0; i < m_particles.size(); ++i) {
        auto& particle = m_particles[i];

        // Decrease life
        particle.life -= deltaTime;

        // Respawn particle if dead
        if (particle.life <= 0.0f) {
            vec2 screenCenter;
            screenCenter.x = 1920.0f / 2.0f;
            screenCenter.y = 1080.0f / 2.0f;

            switch (m_particleType) {
                case 0: InitExplosionParticle(particle, screenCenter); break;
                case 1: InitRainParticle(particle); break;
                case 2: InitSnowParticle(particle); break;
                case 3: InitStarParticle(particle); break;
            }
            particle.isActive = true;
        }

        // Update particle based on type
        if (particle.isActive && particle.life > 0.0f) {
            switch (m_particleType) {
                case 0: UpdateExplosionParticle(particle, deltaTime); break;
                case 1: UpdateRainParticle(particle, deltaTime); break;
                case 2: UpdateSnowParticle(particle, deltaTime); break;
                case 3: UpdateStarParticle(particle, deltaTime); break;
            }

            // Update visual representation
            UpdateParticleSource(static_cast<int>(i), particle);
        }
    }
}

// =============================================================================
// Explosion Particle System
// =============================================================================

void ParticleSystemEffect::InitExplosionParticle(Particle& particle, const vec2& center) {
    std::uniform_real_distribution<float> angleDist(0.0f, 2.0f * 3.14159f);
    std::uniform_real_distribution<float> speedDist(200.0f, 800.0f);
    std::uniform_real_distribution<float> sizeDist(15.0f, 50.0f);
    std::uniform_real_distribution<float> lifeDist(1.0f, 3.0f);
    std::uniform_int_distribution<int> phaseDist(0, 2);

    float angle = angleDist(m_randomEngine);
    float speed = speedDist(m_randomEngine);

    particle.position = center;
    particle.velocity.x = cos(angle) * speed;
    particle.velocity.y = sin(angle) * speed;
    particle.acceleration.x = 0.0f;
    particle.acceleration.y = 300.0f; // Gravity
    particle.maxLife = lifeDist(m_randomEngine);
    particle.life = particle.maxLife;
    particle.size = sizeDist(m_randomEngine);
    particle.rotation = 0.0f;
    particle.rotationSpeed = angleDist(m_randomEngine) * 180.0f;
    particle.phase = phaseDist(m_randomEngine); // Different explosion colors
    particle.alpha = 1.0f;
    particle.isActive = true;

    // Initial color: bright red/orange/yellow
    uint32_t explosionColors[] = {
        0xFFFF3030, // Bright red
        0xFFFF6030, // Orange-red
        0xFFFF9030, // Orange
        0xFFFFC030, // Yellow-orange
        0xFFFFFFFF  // White hot
    };
    std::uniform_int_distribution<int> colorDist(0, 4);
    particle.color = explosionColors[colorDist(m_randomEngine)];
}

void ParticleSystemEffect::UpdateExplosionParticle(Particle& particle, float deltaTime) {
    // Apply physics
    particle.velocity.x += particle.acceleration.x * deltaTime;
    particle.velocity.y += particle.acceleration.y * deltaTime;
    particle.position.x += particle.velocity.x * deltaTime;
    particle.position.y += particle.velocity.y * deltaTime;

    // Apply air resistance
    particle.velocity.x *= 0.98f;
    particle.velocity.y *= 0.99f;

    // Update rotation
    particle.rotation += particle.rotationSpeed * deltaTime;

    // Life progress (0.0 = just born, 1.0 = dying)
    float lifeProgress = 1.0f - (particle.life / particle.maxLife);

    // Color transition: Red -> Orange -> Yellow -> White -> Gray -> Black
    uint32_t startColor = particle.color;
    uint32_t midColor = 0xFFFFFF60;   // Yellow
    uint32_t endColor = 0xFF808080;   // Gray

    if (lifeProgress < 0.3f) {
        // Red/Orange to Yellow
        particle.color = InterpolateColor(startColor, midColor, lifeProgress / 0.3f);
    } else if (lifeProgress < 0.6f) {
        // Yellow to White
        particle.color = InterpolateColor(midColor, 0xFFFFFFFF, (lifeProgress - 0.3f) / 0.3f);
    } else {
        // White to Gray (smoke)
        particle.color = InterpolateColor(0xFFFFFFFF, endColor, (lifeProgress - 0.6f) / 0.4f);
    }

    // Fade out
    particle.alpha = particle.life / particle.maxLife;
    if (particle.alpha > 1.0f) particle.alpha = 1.0f;

    // Size increases as particle disperses (smoke effect)
    particle.size = particle.size * (1.0f + lifeProgress * 0.5f);
}

// =============================================================================
// Rain Particle System
// =============================================================================

void ParticleSystemEffect::InitRainParticle(Particle& particle) {
    std::uniform_real_distribution<float> posXDist(0.0f, 1920.0f);
    std::uniform_real_distribution<float> speedDist(400.0f, 800.0f);
    std::uniform_real_distribution<float> windDist(-30.0f, 30.0f);

    particle.position.x = posXDist(m_randomEngine);
    particle.position.y = -50.0f; // Start above screen
    particle.velocity.x = windDist(m_randomEngine);
    particle.velocity.y = speedDist(m_randomEngine);
    particle.acceleration.x = 0.0f;
    particle.acceleration.y = 100.0f; // Slight acceleration
    particle.maxLife = 5.0f;
    particle.life = particle.maxLife;
    particle.size = 3.0f; // Thin rain drops
    particle.rotation = 90.0f; // Vertical
    particle.rotationSpeed = 0.0f;
    particle.color = 0xFFB0C4DE; // Light steel blue
    particle.alpha = 0.7f;
    particle.phase = 0;
    particle.isActive = true;
}

void ParticleSystemEffect::UpdateRainParticle(Particle& particle, float deltaTime) {
    // Update velocity with acceleration
    particle.velocity.y += particle.acceleration.y * deltaTime;

    // Update position
    particle.position.x += particle.velocity.x * deltaTime;
    particle.position.y += particle.velocity.y * deltaTime;

    // Check if hit ground
    if (particle.position.y > 1080.0f) {
        // Create splash effect by changing phase
        if (particle.phase == 0) {
            particle.phase = 1; // Splash
            particle.velocity.y = 0.0f;
            particle.life = 0.3f; // Short splash life
            particle.size = 10.0f;
        } else {
            particle.life = 0.0f; // Respawn
        }
    }

    // Splash effect
    if (particle.phase == 1) {
        particle.alpha = particle.life / 0.3f;
        particle.size += 20.0f * deltaTime; // Expand splash
    }
}

// =============================================================================
// Snow Particle System
// =============================================================================

void ParticleSystemEffect::InitSnowParticle(Particle& particle) {
    std::uniform_real_distribution<float> posXDist(0.0f, 1920.0f);
    std::uniform_real_distribution<float> posYDist(-100.0f, 0.0f);
    std::uniform_real_distribution<float> speedDist(20.0f, 60.0f);
    std::uniform_real_distribution<float> sizeDist(8.0f, 25.0f);
    std::uniform_real_distribution<float> swayDist(0.0f, 2.0f * 3.14159f);
    std::uniform_real_distribution<float> rotSpeedDist(-45.0f, 45.0f);

    particle.position.x = posXDist(m_randomEngine);
    particle.position.y = posYDist(m_randomEngine);
    particle.velocity.x = 0.0f;
    particle.velocity.y = speedDist(m_randomEngine);
    particle.acceleration.x = 0.0f;
    particle.acceleration.y = 5.0f; // Very light gravity
    particle.maxLife = 15.0f; // Long life for slow fall
    particle.life = particle.maxLife;
    particle.size = sizeDist(m_randomEngine);
    particle.rotation = swayDist(m_randomEngine);
    particle.rotationSpeed = rotSpeedDist(m_randomEngine);
    particle.color = 0xFFFFFFFF; // Pure white
    particle.alpha = 0.9f;
    particle.phase = 0; // Used for sway animation
    particle.isActive = true;
}

void ParticleSystemEffect::UpdateSnowParticle(Particle& particle, float deltaTime) {
    // Update falling
    particle.velocity.y += particle.acceleration.y * deltaTime;
    particle.position.y += particle.velocity.y * deltaTime;

    // Swaying motion (sine wave)
    particle.phase += 1 * deltaTime; // Sway frequency
    float swayAmount = 30.0f;
    particle.position.x += sin(particle.phase) * swayAmount * deltaTime;

    // Rotation
    particle.rotation += particle.rotationSpeed * deltaTime;

    // Check if off screen
    if (particle.position.y > 1150.0f || particle.position.x < -50.0f || particle.position.x > 1970.0f) {
        particle.life = 0.0f; // Respawn
    }

    // Gentle fade based on life
    float lifeProgress = 1.0f - (particle.life / particle.maxLife);
    particle.alpha = 0.9f * (1.0f - lifeProgress * 0.3f);
}

// =============================================================================
// Star/Sparkle Particle System
// =============================================================================

void ParticleSystemEffect::InitStarParticle(Particle& particle) {
    std::uniform_real_distribution<float> posXDist(0.0f, 1920.0f);
    std::uniform_real_distribution<float> posYDist(0.0f, 1080.0f);
    std::uniform_real_distribution<float> sizeDist(10.0f, 30.0f);
    std::uniform_real_distribution<float> lifeDist(2.0f, 5.0f);

    particle.position.x = posXDist(m_randomEngine);
    particle.position.y = posYDist(m_randomEngine);
    particle.velocity.x = 0.0f;
    particle.velocity.y = 0.0f;
    particle.acceleration.x = 0.0f;
    particle.acceleration.y = 0.0f;
    particle.maxLife = lifeDist(m_randomEngine);
    particle.life = particle.maxLife;
    particle.size = sizeDist(m_randomEngine);
    particle.rotation = 0.0f;
    particle.rotationSpeed = 180.0f; // Rotate for sparkle effect
    particle.phase = 0;
    particle.alpha = 0.0f; // Start invisible for fade-in
    particle.isActive = true;

    // Random star colors: white, yellow, light blue, pink
    uint32_t starColors[] = {
        0xFFFFFFFF, // White
        0xFFFFFF66, // Yellow
        0xFF66CCFF, // Light blue
        0xFFFFB6C1, // Light pink
        0xFFFFD700  // Gold
    };
    std::uniform_int_distribution<int> colorDist(0, 4);
    particle.color = starColors[colorDist(m_randomEngine)];
}

void ParticleSystemEffect::UpdateStarParticle(Particle& particle, float deltaTime) {
    // Rotation for twinkling effect
    particle.rotation += particle.rotationSpeed * deltaTime;

    // Life progress
    float lifeProgress = 1.0f - (particle.life / particle.maxLife);

    // Twinkle animation: fade in, stay bright, fade out
    if (lifeProgress < 0.2f) {
        // Fade in
        particle.alpha = lifeProgress / 0.2f;
    } else if (lifeProgress < 0.8f) {
        // Stay bright with pulsing
        float pulse = sin(particle.life * 10.0f) * 0.2f + 0.8f;
        particle.alpha = pulse;
    } else {
        // Fade out
        particle.alpha = (1.0f - lifeProgress) / 0.2f;
    }

    // Size pulsing for sparkle effect
    float sizePulse = sin(particle.life * 15.0f) * 0.3f + 1.0f;
    float currentSize = particle.size * sizePulse;

    // Update the actual rendered size
    particle.size = currentSize;
}

// =============================================================================
// Utility Functions
// =============================================================================

uint32_t ParticleSystemEffect::InterpolateColor(uint32_t color1, uint32_t color2, float t) {
    if (t <= 0.0f) return color1;
    if (t >= 1.0f) return color2;

    uint8_t a1 = (color1 >> 24) & 0xFF;
    uint8_t r1 = (color1 >> 16) & 0xFF;
    uint8_t g1 = (color1 >> 8) & 0xFF;
    uint8_t b1 = color1 & 0xFF;

    uint8_t a2 = (color2 >> 24) & 0xFF;
    uint8_t r2 = (color2 >> 16) & 0xFF;
    uint8_t g2 = (color2 >> 8) & 0xFF;
    uint8_t b2 = color2 & 0xFF;

    uint8_t a = static_cast<uint8_t>(a1 + (a2 - a1) * t);
    uint8_t r = static_cast<uint8_t>(r1 + (r2 - r1) * t);
    uint8_t g = static_cast<uint8_t>(g1 + (g2 - g1) * t);
    uint8_t b = static_cast<uint8_t>(b1 + (b2 - b1) * t);

    return (a << 24) | (r << 16) | (g << 8) | b;
}

void ParticleSystemEffect::CreateParticleSource(int index, const Particle& particle) {
    if (index < 0 || index >= static_cast<int>(m_particleSources.size())) return;

    obs_source_t* sceneSource = obs_frontend_get_current_scene();
    if (!sceneSource) return;

    obs_scene_t* scene = obs_scene_from_source(sceneSource);
    if (!scene) {
        obs_source_release(sceneSource);
        return;
    }

    // Create color source for particle
    obs_data_t* settings = obs_data_create();
    obs_data_set_int(settings, "color", particle.color);
    obs_data_set_int(settings, "width", static_cast<int>(particle.size));
    obs_data_set_int(settings, "height", static_cast<int>(particle.size));

    char particleName[64];
    snprintf(particleName, sizeof(particleName), "particle_%d", index);

    obs_source_t* colorSource = obs_source_create("color_source_v3", particleName, settings, nullptr);
    if (!colorSource) {
        colorSource = obs_source_create("color_source", particleName, settings, nullptr);
    }
    obs_data_release(settings);

    if (colorSource) {
        obs_sceneitem_t* item = obs_scene_add(scene, colorSource);
        if (item) {
            // Set initial position
            vec2 pos;
            pos.x = particle.position.x;
            pos.y = particle.position.y;
            obs_sceneitem_set_pos(item, &pos);

            // Set rotation
            obs_sceneitem_set_rot(item, particle.rotation);

            // Set blend mode for nice effects
            obs_sceneitem_set_blending_mode(item, OBS_BLEND_ADDITIVE);
        }
        m_particleSources[index] = colorSource;
    }

    obs_source_release(sceneSource);
}

void ParticleSystemEffect::UpdateParticleSource(int index, const Particle& particle) {
    if (index < 0 || index >= static_cast<int>(m_particleSources.size())) return;
    if (!m_particleSources[index]) return;

    obs_source_t* sceneSource = obs_frontend_get_current_scene();
    if (!sceneSource) return;

    obs_scene_t* scene = obs_scene_from_source(sceneSource);
    if (!scene) {
        obs_source_release(sceneSource);
        return;
    }

    char particleName[64];
    snprintf(particleName, sizeof(particleName), "particle_%d", index);
    obs_sceneitem_t* item = obs_scene_find_source(scene, particleName);

    if (item) {
        // Update position
        vec2 pos;
        pos.x = particle.position.x;
        pos.y = particle.position.y;
        obs_sceneitem_set_pos(item, &pos);

        // Update rotation
        obs_sceneitem_set_rot(item, particle.rotation);

        // Update visibility based on alpha and active state
        obs_sceneitem_set_visible(item, particle.isActive && particle.alpha > 0.01f);

        // Update color and size through source settings
        obs_data_t* settings = obs_source_get_settings(m_particleSources[index]);
        if (settings) {
            // Apply alpha to color
            uint32_t colorWithAlpha = (particle.color & 0x00FFFFFF) |
                                     (static_cast<uint32_t>(particle.alpha * 255.0f) << 24);
            obs_data_set_int(settings, "color", colorWithAlpha);
            obs_data_set_int(settings, "width", static_cast<int>(particle.size));
            obs_data_set_int(settings, "height", static_cast<int>(particle.size));
            obs_source_update(m_particleSources[index], settings);
            obs_data_release(settings);
        }
    }

    obs_source_release(sceneSource);
}

void ParticleSystemEffect::RemoveAllParticleSources() {
    obs_source_t* sceneSource = obs_frontend_get_current_scene();
    if (!sceneSource) return;

    obs_scene_t* scene = obs_scene_from_source(sceneSource);
    if (scene) {
        for (size_t i = 0; i < m_particleSources.size(); ++i) {
            if (m_particleSources[i]) {
                char particleName[64];
                snprintf(particleName, sizeof(particleName), "particle_%d", static_cast<int>(i));
                obs_sceneitem_t* item = obs_scene_find_source(scene, particleName);
                if (item) {
                    obs_sceneitem_remove(item);
                }
                obs_source_release(m_particleSources[i]);
                m_particleSources[i] = nullptr;
            }
        }
    }

    obs_source_release(sceneSource);
}

// =============================================================================
// ProgressBarEffect Implementation
// =============================================================================

ProgressBarEffect::ProgressBarEffect(obs_source_t* source, double duration, double maxValue)
    : EffectBase(source, duration)
    , m_maxValue(maxValue)
    , m_currentProgress(0.0)
    , m_progressBarSource(nullptr)
{
}

void ProgressBarEffect::Start() {
    m_isActive = true;
    m_elapsedTime = 0.0;
    m_currentProgress = 0.0;

    // Create a text source for the progress bar
    obs_data_t* settings = obs_data_create();
    obs_data_set_string(settings, "text", "Progress: 0%");
    obs_data_set_int(settings, "color", 0xFFFFFFFF);
    obs_data_set_bool(settings, "outline", true);
    obs_data_set_int(settings, "outline_color", 0xFF000000);

    m_progressBarSource = obs_source_create("text_gdiplus", "temp_progress_bar", settings, nullptr);
    if (!m_progressBarSource) {
        // Try alternative text source
        m_progressBarSource = obs_source_create("text_ft2_source", "temp_progress_bar", settings, nullptr);
    }

    if (m_progressBarSource) {
        // Add to current scene
        obs_source_t* sceneSource = obs_frontend_get_current_scene();
        if (sceneSource) {
            obs_scene_t* scene = obs_scene_from_source(sceneSource);
            if (scene) {
                obs_sceneitem_t* item = obs_scene_add(scene, m_progressBarSource);
                if (item) {
                    // Position at top center
                    vec2 pos;
                    pos.x = 1920.0f / 2.0f - 100.0f;
                    pos.y = 50.0f;
                    obs_sceneitem_set_pos(item, &pos);
                }
            }
            obs_source_release(sceneSource);
        }
        blog(LOG_INFO, "[Effect] Progress bar effect started");
    } else {
        blog(LOG_WARNING, "[Effect] Failed to create progress bar source");
        m_isActive = false;
    }

    obs_data_release(settings);

    if (m_isActive) {
        m_timer->start();
    }
}

void ProgressBarEffect::Stop() {
    m_isActive = false;
    m_timer->stop();

    // Remove progress bar from scene
    if (m_progressBarSource) {
        obs_source_t* sceneSource = obs_frontend_get_current_scene();
        if (sceneSource) {
            obs_scene_t* scene = obs_scene_from_source(sceneSource);
            if (scene) {
                obs_sceneitem_t* item = obs_scene_find_source(scene, obs_source_get_name(m_progressBarSource));
                if (item) {
                    obs_sceneitem_remove(item);
                }
            }
            obs_source_release(sceneSource);
        }
        obs_source_release(m_progressBarSource);
        m_progressBarSource = nullptr;
    }

    blog(LOG_INFO, "[Effect] Progress bar effect stopped");
}

void ProgressBarEffect::Update(double elapsed) {
    if (!m_progressBarSource) return;

    // Update progress based on elapsed time
    m_currentProgress = (elapsed / m_duration) * 100.0;

    // Update text
    char progressText[256];
    snprintf(progressText, sizeof(progressText), "Progress: %.0f%%", m_currentProgress);

    obs_data_t* settings = obs_data_create();
    obs_data_set_string(settings, "text", progressText);
    obs_source_update(m_progressBarSource, settings);
    obs_data_release(settings);
}

void ProgressBarEffect::SetProgress(double progress) {
    m_currentProgress = progress;
}

void ProgressBarEffect::CreateProgressBarSource() {
    // Implemented in Start()
}

void ProgressBarEffect::UpdateProgressBarSource() {
    // Implemented in Update()
}

// =============================================================================
// EffectManager Implementation
// =============================================================================

EffectManager::EffectManager(QObject* parent)
    : QObject(parent)
    , m_randomEngine(std::random_device{}())
{
    // Check for completed effects every 100ms
    QTimer* cleanupTimer = new QTimer(this);
    connect(cleanupTimer, &QTimer::timeout, this, &EffectManager::CheckEffectCompletion);
    cleanupTimer->start(100);
}

EffectManager::~EffectManager() {
    ClearAllEffects();
}

void EffectManager::ApplyRandomEffect(obs_source_t* source, double intensity, double duration) {
    // Select random effect type (0-8 for 9 effects)
    std::uniform_int_distribution<int> dist(0, 8);
    int effectIndex = dist(m_randomEngine);

    EffectType type;
    switch (effectIndex) {
        case 0: type = EffectType::Rotation; break;
        case 1: type = EffectType::Blink; break;
        case 2: type = EffectType::Shake; break;
        case 3: type = EffectType::HueShift; break;
        case 4: type = EffectType::Rotation3D; break;
        case 5: type = EffectType::RandomShapes; break;
        case 6: type = EffectType::ParticleSystem; break;
        case 7: type = EffectType::ProgressBar; break;
        case 8: type = EffectType::Kaleidoscope; break;
        default: type = EffectType::Rotation; break;
    }

    ApplyEffect(source, type, intensity, duration);
}

void EffectManager::ApplyEffect(obs_source_t* source, EffectType type, double intensity, double duration) {
    std::unique_ptr<EffectBase> effect;

    switch (type) {
        case EffectType::Rotation:
            effect = std::make_unique<RotationEffect>(source, duration, 0.5 * intensity, 0, false);
            break;

        case EffectType::Blink:
            effect = std::make_unique<BlinkEffect>(source, duration, 3.0 + intensity * 2.0);
            break;

        case EffectType::Shake:
            effect = std::make_unique<ShakeEffect>(source, duration, 5.0 + intensity * 15.0);
            break;

        case EffectType::HueShift:
            effect = std::make_unique<HueShiftEffect>(source, duration, 90.0 + intensity * 90.0);
            break;

        case EffectType::Rotation3D:
            effect = std::make_unique<Rotation3DEffect>(source, duration, true, true);
            break;

        case EffectType::RandomShapes:
            effect = std::make_unique<RandomShapesEffect>(source, duration, static_cast<int>(5 + intensity * 10));
            break;

        case EffectType::ParticleSystem:
            effect = std::make_unique<ParticleSystemEffect>(source, duration, static_cast<int>(20 + intensity * 50));
            break;

        case EffectType::ProgressBar:
            effect = std::make_unique<ProgressBarEffect>(source, duration, 100.0);
            break;

        case EffectType::Kaleidoscope:
            effect = std::make_unique<KaleidoscopeEffect>(source, duration, static_cast<int>(4 + intensity * 4));
            break;

        default:
            blog(LOG_WARNING, "[EffectManager] Unknown effect type");
            return;
    }

    if (effect) {
        effect->Start();
        m_activeEffects.push_back(std::move(effect));

        blog(LOG_INFO, "[EffectManager] Applied effect, total active: %d", GetActiveEffectCount());
    }
}

void EffectManager::ApplyRotationEffect(obs_source_t* source, double duration, double speed,
                                        int rotationType, bool reverse) {
    auto effect = std::make_unique<RotationEffect>(source, duration, speed, rotationType, reverse);

    if (effect) {
        effect->Start();
        m_activeEffects.push_back(std::move(effect));

        blog(LOG_INFO, "[EffectManager] Applied rotation effect with custom params, total active: %d",
             GetActiveEffectCount());
    }
}

void EffectManager::ApplyParticleEffect(obs_source_t* source, double duration, int particleCount, int particleType) {
    auto effect = std::make_unique<ParticleSystemEffect>(source, duration, particleCount, particleType);

    if (effect) {
        effect->Start();
        m_activeEffects.push_back(std::move(effect));

        const char* typeStr = (particleType == 0) ? "爆発" :
                             (particleType == 1) ? "雨" :
                             (particleType == 2) ? "雪" : "星";
        blog(LOG_INFO, "[EffectManager] Applied particle effect: type=%s, count=%d, total active: %d",
             typeStr, particleCount, GetActiveEffectCount());
    }
}

void EffectManager::ClearAllEffects() {
    for (auto& effect : m_activeEffects) {
        if (effect && effect->IsActive()) {
            effect->Stop();
        }
    }
    m_activeEffects.clear();

    blog(LOG_INFO, "[EffectManager] All effects cleared");
}

void EffectManager::CheckEffectCompletion() {
    // Remove completed effects
    m_activeEffects.erase(
        std::remove_if(m_activeEffects.begin(), m_activeEffects.end(),
            [](const std::unique_ptr<EffectBase>& effect) {
                return !effect || !effect->IsActive();
            }),
        m_activeEffects.end()
    );
}
