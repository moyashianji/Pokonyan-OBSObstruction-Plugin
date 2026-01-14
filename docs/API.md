# API リファレンス

## ObstructionManager クラス

画面の妨害効果と回復効果を管理するクラス。

### コンストラクタ

```cpp
ObstructionManager();
```

### パブリックメソッド

#### ApplyObstruction
```cpp
void ApplyObstruction(double amount);
```
スーパーチャットの金額に応じた妨害効果を適用します。

**パラメータ:**
- `amount`: 金額（JPY換算）

**効果:**
- 画面縮小（5-30%）
- 障害物の追加（1-4個）

---

#### ShrinkMainSource
```cpp
void ShrinkMainSource(double percentage);
```
メインソースを指定した割合だけ縮小します。

**パラメータ:**
- `percentage`: 縮小率（%）

**制限:**
- 最大80%まで縮小可能
- 縮小率は累積される

---

#### AddRandomObstruction
```cpp
void AddRandomObstruction(double intensity);
```
ランダムな障害物をシーンに追加します。

**パラメータ:**
- `intensity`: 効果の強さ（0.0-1.0）

**動作:**
- アセットディレクトリからランダムに選択
- ランダムな位置に配置
- 強度に応じてサイズを調整

---

#### ApplyRecovery
```cpp
void ApplyRecovery(double amount);
```
スーパーステッカーの金額に応じた回復効果を適用します。

**パラメータ:**
- `amount`: 金額（JPY換算）

**効果:**
- 画面拡大（10-40%）
- 障害物の除去（1-3個）

---

#### ExpandMainSource
```cpp
void ExpandMainSource(double percentage);
```
縮小されたメインソースを指定した割合だけ拡大（回復）します。

**パラメータ:**
- `percentage`: 拡大率（%）

**制限:**
- 元のサイズ（100%）まで回復可能

---

#### RemoveRandomObstruction
```cpp
void RemoveRandomObstruction();
```
アクティブな障害物からランダムに1つ除去します。

---

#### ClearAllObstructions
```cpp
void ClearAllObstructions();
```
すべての障害物を除去し、メインソースを元のサイズに戻します。

---

#### SetMainSourceName
```cpp
void SetMainSourceName(const std::string& name);
```
縮小/拡大対象のメインソース名を設定します。

**パラメータ:**
- `name`: OBSソースの名前

---

#### SetObstructionAssetPath
```cpp
void SetObstructionAssetPath(const std::string& path);
```
障害物アセットのディレクトリパスを設定します。

**パラメータ:**
- `path`: アセットディレクトリの絶対パス

---

#### SetEnabled
```cpp
void SetEnabled(bool enabled);
```
妨害効果の有効/無効を切り替えます。

**パラメータ:**
- `enabled`: true=有効, false=無効

---

#### GetCurrentShrinkPercentage
```cpp
double GetCurrentShrinkPercentage() const;
```
現在の縮小率を取得します。

**戻り値:**
- 縮小率（%）

---

#### GetActiveObstructionCount
```cpp
int GetActiveObstructionCount() const;
```
アクティブな障害物の数を取得します。

**戻り値:**
- 障害物の数

---

## YouTubeChatClient クラス

YouTube Chat APIと統合し、スーパーチャット/スーパーステッカーを監視するクラス。

### コンストラクタ

```cpp
YouTubeChatClient(QObject* parent = nullptr);
```

### パブリックメソッド

#### SetApiKey
```cpp
void SetApiKey(const std::string& apiKey);
```
YouTube Data API v3のAPIキーを設定します。

**パラメータ:**
- `apiKey`: APIキー文字列

---

#### SetVideoId
```cpp
void SetVideoId(const std::string& videoId);
```
監視するYouTube動画のIDを設定します。

**パラメータ:**
- `videoId`: 動画ID（例: "dQw4w9WgXcQ"）

---

#### SetDonationCallback
```cpp
void SetDonationCallback(DonationCallback callback);
```
スーパーチャット/スーパーステッカー受信時のコールバックを設定します。

**パラメータ:**
- `callback`: `void(const DonationEvent&)`型の関数

**例:**
```cpp
client->SetDonationCallback([](const DonationEvent& event) {
    if (event.type == DonationType::SuperChat) {
        // スーパーチャット処理
    } else {
        // スーパーステッカー処理
    }
});
```

---

#### Start
```cpp
void Start();
```
チャット監視を開始します。

**前提条件:**
- APIキーとVideo IDが設定されていること
- 対象動画が配信中であること

---

#### Stop
```cpp
void Stop();
```
チャット監視を停止します。

---

#### IsRunning
```cpp
bool IsRunning() const;
```
チャット監視が実行中かどうかを確認します。

**戻り値:**
- true: 実行中
- false: 停止中

---

## データ構造

### DonationEvent

スーパーチャット/スーパーステッカーのイベント情報。

```cpp
struct DonationEvent {
    DonationType type;          // SuperChat または SuperSticker
    double amount;              // 金額（JPY換算）
    std::string displayName;    // 送信者の表示名
    std::string message;        // メッセージ（SuperChatのみ）
    std::string currency;       // 元の通貨コード
};
```

### DonationType

```cpp
enum class DonationType {
    SuperChat,      // スーパーチャット
    SuperSticker    // スーパーステッカー
};
```

### PluginSettings

プラグインの設定情報。

```cpp
struct PluginSettings {
    std::string youtubeApiKey;      // YouTube API キー
    std::string videoId;            // 動画ID
    bool enableObstructions;        // 妨害効果の有効化
    bool enableRecovery;            // 回復効果の有効化
    double obstructionIntensity;    // 妨害効果の強度
    double recoveryIntensity;       // 回復効果の強度
};
```

---

## グローバル変数

```cpp
extern std::unique_ptr<YouTubeChatClient> g_chatClient;
extern std::unique_ptr<ObstructionManager> g_obstructionManager;
extern std::unique_ptr<SettingsDialog> g_settingsDialog;
extern PluginSettings g_settings;
```

---

## 設定の読み書き

### LoadSettings
```cpp
void LoadSettings();
```
OBSの設定ファイルから設定を読み込みます。

---

### SaveSettings
```cpp
void SaveSettings();
```
現在の設定をOBSの設定ファイルに保存します。

---

## 通貨換算

YouTube Chat APIは各国の通貨で金額を返します。プラグイン内部では統一処理のためJPYに換算しています。

### 換算レート（概算）

```cpp
double ConvertCurrency(double amount, const std::string& currency);
```

- **JPY**: そのまま
- **USD**: 1 USD = 150 JPY
- **EUR**: 1 EUR = 160 JPY
- **GBP**: 1 GBP = 190 JPY
- **その他**: USD換算

**注意**: 本番環境では、リアルタイムの為替レートAPIを使用することを推奨します。

---

## 使用例

### 基本的な使用例

```cpp
#include "plugin-main.hpp"
#include "youtube-chat-client.hpp"
#include "obstruction-manager.hpp"

// グローバルインスタンスの初期化
g_chatClient = std::make_unique<YouTubeChatClient>();
g_obstructionManager = std::make_unique<ObstructionManager>();

// 設定
g_chatClient->SetApiKey("YOUR_API_KEY");
g_chatClient->SetVideoId("VIDEO_ID");
g_obstructionManager->SetMainSourceName("Game Capture");

// コールバック設定
g_chatClient->SetDonationCallback([](const DonationEvent& event) {
    if (event.type == DonationType::SuperChat) {
        g_obstructionManager->ApplyObstruction(event.amount);
    } else {
        g_obstructionManager->ApplyRecovery(event.amount);
    }
});

// 監視開始
g_chatClient->Start();
```

### カスタム効果の追加

```cpp
// カスタム妨害効果
void CustomObstruction(double amount) {
    // 金額に応じた独自の処理
    if (amount >= 10000) {
        // 高額の場合は特別演出
        g_obstructionManager->ShrinkMainSource(50.0);
        for (int i = 0; i < 10; i++) {
            g_obstructionManager->AddRandomObstruction(1.0);
        }
    } else {
        // 通常の処理
        g_obstructionManager->ApplyObstruction(amount);
    }
}

// コールバックに設定
g_chatClient->SetDonationCallback([](const DonationEvent& event) {
    if (event.type == DonationType::SuperChat) {
        CustomObstruction(event.amount);
    }
});
```
