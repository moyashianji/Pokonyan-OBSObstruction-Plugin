#pragma once

#include <QString>
#include <QList>
#include <QVariant>

// エフェクトの種類
enum class EffectAction {
    Random,              // ランダム
    Rotation,           // 回転
    Blink,              // 点滅
    HueShift,           // 色相変化
    Shake,              // 画面揺れ
    ProgressBar,        // プログレスバー
    Rotation3D,         // 3D回転
    Kaleidoscope,       // 万華鏡
    ImageOverlay,       // 画像表示
    VideoOverlay,       // 動画/GIF表示
    ShrinkScreen,       // 画面縮小
    Particle,           // パーティクル
    RandomShapes        // ランダム図形
};

// 各エフェクトの詳細設定
struct EffectSettings {
    double amount;              // 金額（JPY）
    EffectAction action;        // エフェクトの種類
    double duration;            // 持続時間（秒）

    // === エフェクト固有のパラメータ ===

    // 画像/動画オーバーレイ
    QString mediaPath;          // 画像/動画のパス
    QString mediaFolder;        // メディアフォルダ（ランダム選択用）
    double imageScale;          // 画像の拡大率（%、100=等倍）

    // 色相変化
    int hueType;                // 色相の種類（0=虹色循環, 1=赤→青, 2=青→緑, etc）
    double hueSpeed;            // 色相変化速度

    // 点滅
    double blinkInterval;       // 点滅間隔（秒）
    int blinkType;              // 点滅の種類（0=通常, 1=フェード, 2=ランダム）
    double blinkFrequency;      // 点滅周波数（Hz）

    // 画面揺れ
    int shakeType;              // 揺れの種類（0=ランダム, 1=左右, 2=上下, 3=円運動）
    double shakeIntensity;      // 揺れの強さ

    // プログレスバー
    int progressBarType;        // 種類（0=テキスト, 1=バー, 2=円形）
    int progressBarPosition;    // 位置（0=上, 1=中央, 2=下）

    // 回転
    int rotationType;           // 回転の種類（0=Z軸, 1=X軸, 2=Y軸, 3=全軸）
    double rotationSpeed;       // 回転速度（回転/秒）
    bool rotationReverse;       // 逆回転

    // 画面縮小
    double shrinkPercentage;    // 縮小率（%、0=消える, 100=変化なし）
    bool shrinkSmooth;          // スムーズ縮小

    // パーティクル
    int particleCount;          // パーティクル数
    int particleType;           // 種類（0=爆発, 1=雨, 2=雪, 3=星）

    // コンストラクタ（デフォルト値）
    EffectSettings()
        : amount(1000.0)
        , action(EffectAction::Random)
        , duration(5.0)
        , mediaPath("")
        , mediaFolder("")
        , imageScale(100.0)
        , hueType(0)
        , hueSpeed(180.0)
        , blinkInterval(0.5)
        , blinkType(0)
        , blinkFrequency(3.0)
        , shakeType(0)
        , shakeIntensity(10.0)
        , progressBarType(0)
        , progressBarPosition(0)
        , rotationType(0)
        , rotationSpeed(0.5)
        , rotationReverse(false)
        , shrinkPercentage(20.0)
        , shrinkSmooth(true)
        , particleCount(50)
        , particleType(0)
    {}
};

// エフェクト設定のリスト（金額順にソート）
class EffectConfigList {
public:
    void AddConfig(const EffectSettings& settings);
    void RemoveConfig(int index);
    void UpdateConfig(int index, const EffectSettings& settings);
    EffectSettings GetConfig(int index) const;
    int GetCount() const { return m_configs.size(); }
    void Clear() { m_configs.clear(); }

    // 金額に基づいて適切な設定を取得
    EffectSettings FindConfigForAmount(double amount) const;

    // すべての設定を取得
    const QList<EffectSettings>& GetAllConfigs() const { return m_configs; }

    // 設定の保存/読み込み
    QVariantList ToVariantList() const;
    void FromVariantList(const QVariantList& list);

private:
    QList<EffectSettings> m_configs;
    void SortByAmount();
};

// エフェクトアクションの文字列変換
QString EffectActionToString(EffectAction action);
EffectAction StringToEffectAction(const QString& str);
