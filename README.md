# OBS YouTube SuperChat Plugin / Pokonyan OBS Obstruction Plugin

YouTubeのスーパーチャットとスーパーステッカーに反応して、OBSの画面に面白い演出を加えるプラグインです。

## 機能

### スーパーチャット（妨害効果）
視聴者がスーパーチャットを送ると、金額に応じて以下の効果が発生します：
- **画面縮小**: メインソースが徐々に小さくなります（最大80%まで）
- **障害物表示**: ランダムな画像や動画が画面上に表示されます
- **効果の強度**: 金額が大きいほど効果が強くなります
  - 100円: 最小効果
  - 10,000円以上: 最大効果

### スーパーステッカー（回復効果）
視聴者がスーパーステッカーを送ると、金額に応じて以下の効果が発生します：
- **画面拡大**: 縮小されたメインソースが元のサイズに戻ります
- **障害物除去**: 表示されている障害物がランダムに消えます
- **効果の強度**: 金額が大きいほど回復効果が強くなります

## 必要要件

- OBS Studio 28.0以上
- Qt 6.x
- CMake 3.16以上
- C++17対応コンパイラ
- libcurl
- nlohmann_json
- YouTube Data API v3キー

## ビルド方法

### Windows（Visual Studio 2022）

**初めての方へ**: [Windows向けクイックスタートガイド](QUICKSTART_WINDOWS.md)を参照してください。

**詳細な手順**: [Visual Studioセットアップガイド](docs/VISUAL_STUDIO_SETUP.md)

簡易版：
```bash
mkdir build
cd build
cmake -G "Visual Studio 17 2022" -DCMAKE_PREFIX_PATH=C:\path\to\obs-studio\build ..
cmake --build . --config Release
cmake --install . --config Release
```

### Linux / macOS

```bash
mkdir build
cd build
cmake -DCMAKE_PREFIX_PATH=/path/to/obs-studio/build ..
make
sudo make install
```

## セットアップ

### 1. YouTube Data API v3キーの取得

1. [Google Cloud Console](https://console.cloud.google.com/)にアクセス
2. 新しいプロジェクトを作成
3. YouTube Data API v3を有効化
4. APIキーを作成
5. APIキーに適切な制限を設定（YouTube Data API v3のみ）

### 2. プラグインの設定

1. OBS Studioを起動
2. メニューから **Tools > YouTube SuperChat Plugin Settings** を開く
3. 以下の情報を入力：
   - **API Key**: 取得したYouTube Data API v3キー
   - **Video ID**: 配信中のYouTube動画ID（URLの`v=`パラメータの値）
   - **Main Source**: 縮小/拡大対象のソース名（例: "Game Capture"）
4. **Enable Obstructions**: スーパーチャットの妨害効果を有効化
5. **Enable Recovery**: スーパーステッカーの回復効果を有効化
6. **Obstruction Intensity**: 妨害効果の強さを調整（0.1〜3.0、デフォルト: 1.0）
7. **Recovery Intensity**: 回復効果の強さを調整（0.1〜3.0、デフォルト: 1.0）
8. **Save**をクリックして設定を保存
9. **Start Monitoring**をクリックして監視を開始

### 3. 障害物アセットの追加（オプション）

デフォルトでは赤い四角が表示されますが、カスタム画像や動画を使用できます：

1. `data/obstruction_assets/`ディレクトリに画像や動画を配置
2. 対応フォーマット：
   - 画像: PNG, JPG, JPEG, GIF
   - 動画: MP4, WebM

プラグインは自動的にランダムなアセットを選んで表示します。

## 使用方法

1. YouTube Liveで配信を開始
2. OBSでプラグイン設定を開き、Video IDに配信のIDを入力
3. **Start Monitoring**をクリック
4. 視聴者からスーパーチャットやスーパーステッカーを受け取ると自動的に演出が発生

### 効果の例

- **500円のスーパーチャット**: 画面が約7%縮小、障害物1-2個追加
- **5,000円のスーパーチャット**: 画面が約17%縮小、障害物2-3個追加
- **10,000円のスーパーチャット**: 画面が最大30%縮小、障害物4個追加
- **500円のスーパーステッカー**: 画面が約15%拡大、障害物1-2個除去
- **5,000円のスーパーステッカー**: 画面が約30%拡大、障害物2-3個除去

## トラブルシューティング

### プラグインが読み込まれない

- OBS Studioのバージョンを確認（28.0以上が必要）
- プラグインが正しいディレクトリにインストールされているか確認
- OBSのログを確認（Help > Log Files > Current Log）

### YouTube Chat APIが動作しない

- APIキーが正しいか確認
- Video IDが正しいか確認（配信中の動画のみ対応）
- Google Cloud ConsoleでYouTube Data API v3が有効か確認
- APIの使用量制限に達していないか確認

### 効果が発生しない

- **Start Monitoring**がクリックされているか確認
- Main Sourceの名前が正しいか確認
- Enable ObstructionsまたはEnable Recoveryがチェックされているか確認
- OBSログでエラーメッセージを確認

## 設定ファイル

設定はOBSのグローバル設定ファイルに保存されます：
- Linux: `~/.config/obs-studio/global.ini`
- macOS: `~/Library/Application Support/obs-studio/global.ini`
- Windows: `%APPDATA%\obs-studio\global.ini`

セクション名: `[YouTubeSuperChatPlugin]`

## ライセンス

MIT License

## 開発者向け情報

### プロジェクト構造

```
Pokonyan-OBSObstruction-Plugin/
├── CMakeLists.txt              # ビルド設定
├── README.md                   # このファイル
├── data/
│   └── obstruction_assets/     # 障害物用アセット
└── src/
    ├── plugin-main.cpp         # プラグインエントリーポイント
    ├── plugin-main.hpp
    ├── youtube-chat-client.cpp # YouTube Chat API統合
    ├── youtube-chat-client.hpp
    ├── obstruction-manager.cpp # 演出効果マネージャー
    ├── obstruction-manager.hpp
    ├── settings-dialog.cpp     # 設定UI
    └── settings-dialog.hpp
```

### APIリファレンス

#### ObstructionManager

主な機能：
- `ApplyObstruction(double amount)`: 金額に応じた妨害効果を適用
- `ApplyRecovery(double amount)`: 金額に応じた回復効果を適用
- `ShrinkMainSource(double percentage)`: メインソースを縮小
- `ExpandMainSource(double percentage)`: メインソースを拡大
- `ClearAllObstructions()`: すべての障害物を削除

#### YouTubeChatClient

主な機能：
- `SetApiKey(const std::string& apiKey)`: APIキーを設定
- `SetVideoId(const std::string& videoId)`: 動画IDを設定
- `Start()`: チャット監視を開始
- `Stop()`: チャット監視を停止

## 貢献

プルリクエストを歓迎します！大きな変更の場合は、まずIssueを開いて変更内容を議論してください。

## サポート

問題が発生した場合は、GitHubのIssueセクションで報告してください。
