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

RotationEffect::RotationEffect(obs_source_t* source, double duration, double rotationsPerSecond)
    : EffectBase(source, duration)
    , m_rotationsPerSecond(rotationsPerSecond)
    , m_currentRotation(0.0)
{
}

void RotationEffect::Start() {
    m_isActive = true;
    m_elapsedTime = 0.0;
    m_currentRotation = 0.0;
    m_timer->start();

    blog(LOG_INFO, "[Effect] Rotation effect started (%.1fs, %.1f rot/s)", m_duration, m_rotationsPerSecond);
}

void RotationEffect::Stop() {
    m_isActive = false;
    m_timer->stop();

    // Reset rotation
    obs_sceneitem_t* sceneItem = obs_scene_find_source(
        obs_scene_from_source(obs_frontend_get_current_scene()),
        obs_source_get_name(m_source)
    );

    if (sceneItem) {
        obs_sceneitem_set_rot(sceneItem, 0.0f);
    }

    blog(LOG_INFO, "[Effect] Rotation effect stopped");
}

void RotationEffect::Update(double elapsed) {
    // Calculate rotation angle (in degrees)
    m_currentRotation = fmod(elapsed * m_rotationsPerSecond * 360.0, 360.0);

    // Apply rotation
    obs_source_t* sceneSource = obs_frontend_get_current_scene();
    if (!sceneSource) return;

    obs_scene_t* scene = obs_scene_from_source(sceneSource);
    if (!scene) {
        obs_source_release(sceneSource);
        return;
    }

    obs_sceneitem_t* sceneItem = obs_scene_find_source(scene, obs_source_get_name(m_source));
    if (sceneItem) {
        obs_sceneitem_set_rot(sceneItem, static_cast<float>(m_currentRotation));
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
}

void KaleidoscopeEffect::Start() {
    m_isActive = true;
    m_elapsedTime = 0.0;
    m_timer->start();

    blog(LOG_INFO, "[Effect] Kaleidoscope effect started (%d segments)", m_segments);
}

void KaleidoscopeEffect::Stop() {
    m_isActive = false;
    m_timer->stop();

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
    // Simplified implementation - full kaleidoscope would require
    // creating multiple mirrored copies of the source and arranging them
    // in a radial pattern
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
}

void Rotation3DEffect::Start() {
    m_isActive = true;
    m_elapsedTime = 0.0;
    m_angleX = 0.0;
    m_angleY = 0.0;
    m_timer->start();

    blog(LOG_INFO, "[Effect] 3D Rotation effect started (X: %d, Y: %d)", m_rotateX, m_rotateY);
}

void Rotation3DEffect::Stop() {
    m_isActive = false;
    m_timer->stop();

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

    // Note: Full 3D rotation would require matrix transforms
    // This is a simplified version - OBS doesn't natively support 3D transforms
    // Would need custom shader implementation
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

    // Get source dimensions
    uint32_t sourceWidth = obs_source_get_width(m_source);
    uint32_t sourceHeight = obs_source_get_height(m_source);

    if (sourceWidth == 0 || sourceHeight == 0) {
        sourceWidth = 1920;
        sourceHeight = 1080;
    }

    // Initialize random shapes
    std::uniform_real_distribution<float> posXDist(0.0f, static_cast<float>(sourceWidth));
    std::uniform_real_distribution<float> posYDist(0.0f, static_cast<float>(sourceHeight));
    std::uniform_real_distribution<float> velDist(-100.0f, 100.0f);
    std::uniform_real_distribution<float> sizeDist(20.0f, 100.0f);
    std::uniform_int_distribution<int> typeDist(0, 2);
    std::uniform_int_distribution<uint32_t> colorDist(0x40000000, 0xFFFFFFFF);

    for (int i = 0; i < m_shapeCount; ++i) {
        Shape shape;
        shape.position.x = posXDist(m_randomEngine);
        shape.position.y = posYDist(m_randomEngine);
        shape.velocity.x = velDist(m_randomEngine);
        shape.velocity.y = velDist(m_randomEngine);
        shape.size = sizeDist(m_randomEngine);
        shape.color = colorDist(m_randomEngine) | 0xFF000000; // Ensure alpha
        shape.type = typeDist(m_randomEngine);
        m_shapes.push_back(shape);
    }

    m_timer->start();
    blog(LOG_INFO, "[Effect] Random shapes effect started (%d shapes)", m_shapeCount);
}

void RandomShapesEffect::Stop() {
    m_isActive = false;
    m_timer->stop();
    m_shapes.clear();

    blog(LOG_INFO, "[Effect] Random shapes effect stopped");
}

void RandomShapesEffect::Update(double elapsed) {
    // Get source dimensions for boundary checking
    uint32_t sourceWidth = obs_source_get_width(m_source);
    uint32_t sourceHeight = obs_source_get_height(m_source);

    if (sourceWidth == 0 || sourceHeight == 0) {
        sourceWidth = 1920;
        sourceHeight = 1080;
    }

    // Update shape positions
    for (auto& shape : m_shapes) {
        shape.position.x += shape.velocity.x * 0.016f;
        shape.position.y += shape.velocity.y * 0.016f;

        // Bounce off edges
        if (shape.position.x < 0 || shape.position.x > sourceWidth) {
            shape.velocity.x *= -1.0f;
            shape.position.x = std::clamp(shape.position.x, 0.0f, static_cast<float>(sourceWidth));
        }
        if (shape.position.y < 0 || shape.position.y > sourceHeight) {
            shape.velocity.y *= -1.0f;
            shape.position.y = std::clamp(shape.position.y, 0.0f, static_cast<float>(sourceHeight));
        }
    }

    // Note: Full implementation would render shapes to overlay
    // This requires custom rendering or using color sources as shape proxies
}

void RandomShapesEffect::CreateShapeSource() {
    // Placeholder for creating a source to render shapes
}

void RandomShapesEffect::UpdateShapeSource() {
    // Placeholder for updating shape rendering
}

// =============================================================================
// ParticleSystemEffect Implementation
// =============================================================================

ParticleSystemEffect::ParticleSystemEffect(obs_source_t* source, double duration, int particleCount)
    : EffectBase(source, duration)
    , m_particleCount(particleCount)
    , m_randomEngine(std::random_device{}())
    , m_particleSource(nullptr)
{
}

void ParticleSystemEffect::Start() {
    m_isActive = true;
    m_elapsedTime = 0.0;
    m_particles.clear();

    // Initialize particles
    for (int i = 0; i < m_particleCount; ++i) {
        Particle particle;
        RespawnParticle(particle);
        // Randomize initial life for staggered spawning
        std::uniform_real_distribution<float> lifeDist(0.0f, 1.0f);
        particle.life = lifeDist(m_randomEngine);
        m_particles.push_back(particle);
    }

    m_timer->start();
    blog(LOG_INFO, "[Effect] Particle system effect started (%d particles)", m_particleCount);
}

void ParticleSystemEffect::Stop() {
    m_isActive = false;
    m_timer->stop();
    m_particles.clear();

    blog(LOG_INFO, "[Effect] Particle system effect stopped");
}

void ParticleSystemEffect::Update(double elapsed) {
    float deltaTime = 0.016f;

    // Update all particles
    for (auto& particle : m_particles) {
        // Update life
        particle.life -= deltaTime;

        if (particle.life <= 0.0f) {
            RespawnParticle(particle);
        }

        // Update position
        particle.position.x += particle.velocity.x * deltaTime;
        particle.position.y += particle.velocity.y * deltaTime;

        // Apply gravity
        particle.velocity.y += 200.0f * deltaTime;

        // Update size based on life (fade out)
        particle.size = 5.0f * particle.life;
    }

    // Note: Full implementation would render particles to overlay
}

void ParticleSystemEffect::RespawnParticle(Particle& particle) {
    uint32_t sourceWidth = obs_source_get_width(m_source);
    uint32_t sourceHeight = obs_source_get_height(m_source);

    if (sourceWidth == 0 || sourceHeight == 0) {
        sourceWidth = 1920;
        sourceHeight = 1080;
    }

    std::uniform_real_distribution<float> posXDist(0.0f, static_cast<float>(sourceWidth));
    std::uniform_real_distribution<float> velXDist(-100.0f, 100.0f);
    std::uniform_real_distribution<float> velYDist(-300.0f, -100.0f);
    std::uniform_int_distribution<uint32_t> colorDist(0x80000000, 0xFFFFFFFF);

    particle.position.x = posXDist(m_randomEngine);
    particle.position.y = static_cast<float>(sourceHeight);
    particle.velocity.x = velXDist(m_randomEngine);
    particle.velocity.y = velYDist(m_randomEngine);
    particle.life = 3.0f;
    particle.size = 5.0f;
    particle.color = colorDist(m_randomEngine);
}

void ParticleSystemEffect::CreateParticleSource() {
    // Placeholder for creating particle rendering source
}

void ParticleSystemEffect::UpdateParticleSource() {
    // Placeholder for updating particle rendering
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
            effect = std::make_unique<RotationEffect>(source, duration, 0.5 * intensity);
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
