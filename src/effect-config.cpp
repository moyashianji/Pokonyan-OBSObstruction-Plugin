#include "effect-config.hpp"
#include <algorithm>
#include <QVariantMap>

void EffectConfigList::AddConfig(const EffectSettings& settings) {
    m_configs.append(settings);
    SortByAmount();
}

void EffectConfigList::RemoveConfig(int index) {
    if (index >= 0 && index < m_configs.size()) {
        m_configs.removeAt(index);
    }
}

void EffectConfigList::UpdateConfig(int index, const EffectSettings& settings) {
    if (index >= 0 && index < m_configs.size()) {
        m_configs[index] = settings;
        SortByAmount();
    }
}

EffectSettings EffectConfigList::GetConfig(int index) const {
    if (index >= 0 && index < m_configs.size()) {
        return m_configs[index];
    }
    return EffectSettings();
}

EffectSettings EffectConfigList::FindConfigForAmount(double amount) const {
    // 金額以下の最大設定を探す
    EffectSettings result;
    for (const auto& config : m_configs) {
        if (amount >= config.amount) {
            result = config;
        } else {
            break;  // ソート済みなので、これ以上探す必要なし
        }
    }
    return result;
}

void EffectConfigList::SortByAmount() {
    std::sort(m_configs.begin(), m_configs.end(),
              [](const EffectSettings& a, const EffectSettings& b) {
                  return a.amount < b.amount;
              });
}

QVariantList EffectConfigList::ToVariantList() const {
    QVariantList list;
    for (const auto& config : m_configs) {
        QVariantMap map;
        map["amount"] = config.amount;
        map["action"] = static_cast<int>(config.action);
        map["duration"] = config.duration;
        map["mediaPath"] = config.mediaPath;
        map["mediaFolder"] = config.mediaFolder;
        map["imageScale"] = config.imageScale;
        map["hueType"] = config.hueType;
        map["hueSpeed"] = config.hueSpeed;
        map["blinkInterval"] = config.blinkInterval;
        map["blinkType"] = config.blinkType;
        map["blinkFrequency"] = config.blinkFrequency;
        map["shakeType"] = config.shakeType;
        map["shakeIntensity"] = config.shakeIntensity;
        map["progressBarType"] = config.progressBarType;
        map["progressBarPosition"] = config.progressBarPosition;
        map["rotationType"] = config.rotationType;
        map["rotationSpeed"] = config.rotationSpeed;
        map["rotationReverse"] = config.rotationReverse;
        map["shrinkPercentage"] = config.shrinkPercentage;
        map["shrinkSmooth"] = config.shrinkSmooth;
        map["particleCount"] = config.particleCount;
        map["particleType"] = config.particleType;
        list.append(map);
    }
    return list;
}

void EffectConfigList::FromVariantList(const QVariantList& list) {
    m_configs.clear();
    for (const auto& item : list) {
        QVariantMap map = item.toMap();
        EffectSettings config;
        config.amount = map["amount"].toDouble();
        config.action = static_cast<EffectAction>(map["action"].toInt());
        config.duration = map["duration"].toDouble();
        config.mediaPath = map["mediaPath"].toString();
        config.mediaFolder = map["mediaFolder"].toString();
        config.imageScale = map["imageScale"].toDouble();
        config.hueType = map["hueType"].toInt();
        config.hueSpeed = map["hueSpeed"].toDouble();
        config.blinkInterval = map["blinkInterval"].toDouble();
        config.blinkType = map["blinkType"].toInt();
        config.blinkFrequency = map["blinkFrequency"].toDouble();
        config.shakeType = map["shakeType"].toInt();
        config.shakeIntensity = map["shakeIntensity"].toDouble();
        config.progressBarType = map["progressBarType"].toInt();
        config.progressBarPosition = map["progressBarPosition"].toInt();
        config.rotationType = map["rotationType"].toInt();
        config.rotationSpeed = map["rotationSpeed"].toDouble();
        config.rotationReverse = map["rotationReverse"].toBool();
        config.shrinkPercentage = map["shrinkPercentage"].toDouble();
        config.shrinkSmooth = map["shrinkSmooth"].toBool();
        config.particleCount = map["particleCount"].toInt();
        config.particleType = map["particleType"].toInt();
        m_configs.append(config);
    }
    SortByAmount();
}

QString EffectActionToString(EffectAction action) {
    switch (action) {
        case EffectAction::Random: return "ランダム";
        case EffectAction::Rotation: return "回転";
        case EffectAction::Blink: return "点滅";
        case EffectAction::HueShift: return "色相変化";
        case EffectAction::Shake: return "画面揺れ";
        case EffectAction::ProgressBar: return "プログレスバー";
        case EffectAction::Rotation3D: return "3D回転";
        case EffectAction::Kaleidoscope: return "万華鏡";
        case EffectAction::ImageOverlay: return "画像表示";
        case EffectAction::VideoOverlay: return "動画/GIF表示";
        case EffectAction::ShrinkScreen: return "画面縮小";
        case EffectAction::Particle: return "パーティクル";
        case EffectAction::RandomShapes: return "ランダム図形";
        default: return "不明";
    }
}

EffectAction StringToEffectAction(const QString& str) {
    if (str == "ランダム") return EffectAction::Random;
    if (str == "回転") return EffectAction::Rotation;
    if (str == "点滅") return EffectAction::Blink;
    if (str == "色相変化") return EffectAction::HueShift;
    if (str == "画面揺れ") return EffectAction::Shake;
    if (str == "プログレスバー") return EffectAction::ProgressBar;
    if (str == "3D回転") return EffectAction::Rotation3D;
    if (str == "万華鏡") return EffectAction::Kaleidoscope;
    if (str == "画像表示") return EffectAction::ImageOverlay;
    if (str == "動画/GIF表示") return EffectAction::VideoOverlay;
    if (str == "画面縮小") return EffectAction::ShrinkScreen;
    if (str == "パーティクル") return EffectAction::Particle;
    if (str == "ランダム図形") return EffectAction::RandomShapes;
    return EffectAction::Random;
}
