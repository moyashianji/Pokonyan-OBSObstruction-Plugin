#pragma once

#include <obs.h>
#include <QTimer>
#include <QObject>
#include <memory>
#include <vector>
#include <random>
#include <string>

// Forward declarations
class EffectBase;

// Effect types enum
enum class EffectType {
    Rotation,           // 画面回転
    Blink,              // 点滅
    HueShift,           // 色相変化
    Shake,              // 画面揺れ
    Kaleidoscope,       // 万華鏡
    Rotation3D,         // 3D回転
    CustomShader,       // カスタムシェーダー
    RandomShapes,       // ランダム図形
    ParticleSystem,     // パーティクル
    ProgressBar         // プログレスバー
};

// Base class for all effects
class EffectBase : public QObject {
    Q_OBJECT

public:
    explicit EffectBase(obs_source_t* source, double duration, QObject* parent = nullptr);
    virtual ~EffectBase();

    virtual void Start() = 0;
    virtual void Stop() = 0;
    virtual void Update(double elapsed) = 0;

    bool IsActive() const { return m_isActive; }
    double GetDuration() const { return m_duration; }
    double GetElapsedTime() const { return m_elapsedTime; }

protected:
    obs_source_t* m_source;
    double m_duration;      // Effect duration in seconds
    double m_elapsedTime;   // Elapsed time in seconds
    bool m_isActive;
    QTimer* m_timer;

protected slots:
    virtual void OnTimerTick();
};

// Rotation Effect
class RotationEffect : public EffectBase {
    Q_OBJECT

public:
    RotationEffect(obs_source_t* source, double duration, double rotationsPerSecond = 0.5);

    void Start() override;
    void Stop() override;
    void Update(double elapsed) override;

private:
    double m_rotationsPerSecond;
    double m_currentRotation;
};

// Blink Effect
class BlinkEffect : public EffectBase {
    Q_OBJECT

public:
    BlinkEffect(obs_source_t* source, double duration, double blinkFrequency = 5.0);

    void Start() override;
    void Stop() override;
    void Update(double elapsed) override;

private:
    double m_blinkFrequency;
    bool m_isVisible;
};

// Hue Shift Effect
class HueShiftEffect : public EffectBase {
    Q_OBJECT

public:
    HueShiftEffect(obs_source_t* source, double duration, double shiftSpeed = 180.0);

    void Start() override;
    void Stop() override;
    void Update(double elapsed) override;

private:
    double m_shiftSpeed;      // Degrees per second
    double m_currentHue;
    obs_source_t* m_colorFilter;
};

// Shake Effect
class ShakeEffect : public EffectBase {
    Q_OBJECT

public:
    ShakeEffect(obs_source_t* source, double duration, double intensity = 10.0);

    void Start() override;
    void Stop() override;
    void Update(double elapsed) override;

private:
    double m_intensity;
    vec2 m_originalPos;
    std::mt19937 m_randomEngine;
};

// Kaleidoscope Effect
class KaleidoscopeEffect : public EffectBase {
    Q_OBJECT

public:
    KaleidoscopeEffect(obs_source_t* source, double duration, int segments = 6);

    void Start() override;
    void Stop() override;
    void Update(double elapsed) override;

private:
    int m_segments;
    std::vector<obs_source_t*> m_mirrorSources;
};

// 3D Rotation Effect
class Rotation3DEffect : public EffectBase {
    Q_OBJECT

public:
    Rotation3DEffect(obs_source_t* source, double duration, bool rotateX = true, bool rotateY = true);

    void Start() override;
    void Stop() override;
    void Update(double elapsed) override;

private:
    bool m_rotateX;
    bool m_rotateY;
    double m_angleX;
    double m_angleY;
};

// Random Shapes Effect
class RandomShapesEffect : public EffectBase {
    Q_OBJECT

public:
    RandomShapesEffect(obs_source_t* source, double duration, int shapeCount = 10);

    void Start() override;
    void Stop() override;
    void Update(double elapsed) override;

private:
    struct Shape {
        vec2 position;
        vec2 velocity;
        float size;
        uint32_t color;
        int type; // 0=circle, 1=square, 2=triangle
    };

    int m_shapeCount;
    std::vector<Shape> m_shapes;
    std::mt19937 m_randomEngine;
    obs_source_t* m_shapeSource;

    void CreateShapeSource();
    void UpdateShapeSource();
};

// Particle System Effect
class ParticleSystemEffect : public EffectBase {
    Q_OBJECT

public:
    ParticleSystemEffect(obs_source_t* source, double duration, int particleCount = 50);

    void Start() override;
    void Stop() override;
    void Update(double elapsed) override;

private:
    struct Particle {
        vec2 position;
        vec2 velocity;
        float life;
        float size;
        uint32_t color;
    };

    int m_particleCount;
    std::vector<Particle> m_particles;
    std::mt19937 m_randomEngine;
    obs_source_t* m_particleSource;

    void CreateParticleSource();
    void UpdateParticleSource();
    void RespawnParticle(Particle& particle);
};

// Progress Bar Effect
class ProgressBarEffect : public EffectBase {
    Q_OBJECT

public:
    ProgressBarEffect(obs_source_t* source, double duration, double maxValue = 100.0);

    void Start() override;
    void Stop() override;
    void Update(double elapsed) override;

    void SetProgress(double progress);

private:
    double m_maxValue;
    double m_currentProgress;
    obs_source_t* m_progressBarSource;

    void CreateProgressBarSource();
    void UpdateProgressBarSource();
};

// Effect Manager
class EffectManager : public QObject {
    Q_OBJECT

public:
    explicit EffectManager(QObject* parent = nullptr);
    ~EffectManager();

    // Apply random effect to source
    void ApplyRandomEffect(obs_source_t* source, double intensity, double duration);

    // Apply specific effect
    void ApplyEffect(obs_source_t* source, EffectType type, double intensity, double duration);

    // Clear all effects
    void ClearAllEffects();

    // Get active effect count
    int GetActiveEffectCount() const { return static_cast<int>(m_activeEffects.size()); }

private:
    std::vector<std::unique_ptr<EffectBase>> m_activeEffects;
    std::mt19937 m_randomEngine;

private slots:
    void CheckEffectCompletion();
};
