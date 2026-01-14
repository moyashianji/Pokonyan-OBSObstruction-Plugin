# セットアップガイド

## YouTube Data API v3の設定

### 1. Google Cloud Projectの作成

1. [Google Cloud Console](https://console.cloud.google.com/)にアクセス
2. 「プロジェクトを選択」をクリック
3. 「新しいプロジェクト」をクリック
4. プロジェクト名を入力（例: "OBS SuperChat Plugin"）
5. 「作成」をクリック

### 2. YouTube Data API v3の有効化

1. 左側のメニューから「APIとサービス」→「ライブラリ」を選択
2. 検索ボックスに「YouTube Data API v3」と入力
3. 「YouTube Data API v3」を選択
4. 「有効にする」をクリック

### 3. APIキーの作成

1. 左側のメニューから「APIとサービス」→「認証情報」を選択
2. 「認証情報を作成」→「APIキー」を選択
3. APIキーが表示されるのでコピーして保存

### 4. APIキーの制限設定（推奨）

1. 作成したAPIキーの右側にある編集アイコンをクリック
2. 「アプリケーションの制限」で「なし」を選択（開発中）
   - 本番環境では「HTTPリファラー」や「IPアドレス」で制限することを推奨
3. 「APIの制限」で「キーを制限」を選択
4. 「YouTube Data API v3」のみを選択
5. 「保存」をクリック

## OBSでの設定

### 1. プラグインのインストール確認

1. OBS Studioを起動
2. メニューから「Tools」を開く
3. 「YouTube SuperChat Plugin Settings」が表示されていることを確認

### 2. 基本設定

1. 「YouTube SuperChat Plugin Settings」を開く
2. **API Key**に取得したAPIキーを入力
3. **Video ID**に配信のVideo IDを入力
   - Video IDの確認方法：
     - YouTube配信ページのURL: `https://www.youtube.com/watch?v=VIDEO_ID`
     - `v=`の後の文字列がVideo ID
4. **Main Source**に縮小/拡大したいソースの名前を入力
   - OBSのソース一覧で確認できます
   - 例: "ゲームキャプチャ", "ウィンドウキャプチャ", "映像キャプチャデバイス"

### 3. 効果の設定

#### 妨害効果（スーパーチャット）
- **Enable Obstructions**: チェックを入れると有効化
- **Obstruction Intensity**: 効果の強さを調整
  - 0.5 = 弱い効果
  - 1.0 = 標準（デフォルト）
  - 2.0 = 強い効果

#### 回復効果（スーパーステッカー）
- **Enable Recovery**: チェックを入れると有効化
- **Recovery Intensity**: 効果の強さを調整
  - 0.5 = 弱い回復
  - 1.0 = 標準（デフォルト）
  - 2.0 = 強い回復

### 4. 監視の開始

1. 設定を入力後、「Save」をクリック
2. 「Start Monitoring」をクリック
3. ステータスが「Monitoring active」になることを確認

## カスタム障害物の追加

### 1. アセットの準備

障害物として表示したい画像や動画を準備します：
- **推奨サイズ**: 200x200px 〜 500x500px
- **対応形式**: PNG, JPG, GIF, MP4, WebM
- **ファイル名**: 任意（日本語も可）

### 2. アセットの配置

OBSのプラグインデータディレクトリにアセットを配置：

**Linux:**
```bash
mkdir -p ~/.config/obs-studio/plugins/obs-youtube-superchat-plugin/obstruction_assets
cp your-image.png ~/.config/obs-studio/plugins/obs-youtube-superchat-plugin/obstruction_assets/
```

**macOS:**
```bash
mkdir -p ~/Library/Application\ Support/obs-studio/plugins/obs-youtube-superchat-plugin/obstruction_assets
cp your-image.png ~/Library/Application\ Support/obs-studio/plugins/obs-youtube-superchat-plugin/obstruction_assets/
```

**Windows:**
```
C:\Users\YourName\AppData\Roaming\obs-studio\plugins\obs-youtube-superchat-plugin\obstruction_assets\
```

### 3. アセットの確認

1. OBSを再起動
2. テスト用にスーパーチャットを送信（またはテストモード使用）
3. カスタム画像/動画が表示されることを確認

## テスト方法

### 本番環境でのテスト

1. YouTube Liveで実際に配信を開始
2. 別のアカウントから少額のスーパーチャット/スーパーステッカーを送信
3. OBSで効果が発生することを確認

### デバッグ方法

OBSのログを確認してエラーをチェック：

1. OBSのメニューから「Help」→「Log Files」→「View Current Log」
2. `[YouTube Chat]`または`[Obstruction]`でログを検索
3. エラーメッセージを確認

よくあるエラー：
- `Failed to fetch live chat ID`: Video IDが間違っている、または配信が開始されていない
- `Network error`: インターネット接続の問題、またはAPIキーが無効
- `Main source not found`: Main Sourceの名前が間違っている

## パフォーマンス最適化

### 1. ポーリング間隔の調整

YouTube Chat APIは自動的に最適なポーリング間隔を返しますが、手動で調整も可能です。
`youtube-chat-client.cpp`の`m_pollIntervalMs`を変更：

```cpp
m_pollIntervalMs(5000)  // 5秒（デフォルト）
m_pollIntervalMs(10000) // 10秒（低負荷）
m_pollIntervalMs(2000)  // 2秒（高応答性）
```

### 2. 障害物の数の制限

大量の障害物が表示されるとパフォーマンスが低下する可能性があります。
`obstruction-manager.cpp`で最大数を制限：

```cpp
// ApplyObstruction関数内
int numObstructions = std::min(static_cast<int>(1 + intensity * 3), 5);  // 最大5個
```

### 3. 動画アセットの使用を制限

動画ファイルは画像より負荷が高いため、必要に応じて画像のみ使用することを推奨します。

## セキュリティのベストプラクティス

### APIキーの保護

1. **APIキーを公開しない**
   - GitHubなどにアップロードしない
   - 配信中に設定画面を映さない

2. **APIキーに制限を設定**
   - YouTube Data API v3のみに制限
   - 可能であればIPアドレスで制限

3. **定期的なキーのローテーション**
   - 定期的に新しいAPIキーを生成
   - 古いキーは削除

### 使用量制限の監視

YouTube Data API v3には無料枠があります：
- **無料枠**: 1日10,000ユニット
- **Live Chat Messages取得**: 1リクエスト = 5ユニット
- **5秒ごとのポーリング**: 1日約86,000ユニット

長時間配信する場合は、ポーリング間隔を調整して使用量を抑えてください。

## トラブルシューティング

### プラグインが見つからない

**症状**: ToolsメニューにYouTube SuperChat Plugin Settingsが表示されない

**解決方法**:
1. プラグインが正しくインストールされているか確認
2. OBSを再起動
3. OBSのバージョンを確認（28.0以上が必要）

### 効果が発生しない

**症状**: スーパーチャットを送っても何も起こらない

**解決方法**:
1. Start Monitoringがクリックされているか確認
2. Main Sourceの名前が正しいか確認
3. OBSログでエラーを確認

### 画面が縮小したまま戻らない

**解決方法**:
1. 設定ダイアログを開く
2. Stop Monitoringをクリック
3. OBSのソーストランスフォームを手動でリセット

または、プラグインコード内で`ClearAllObstructions()`を呼び出すボタンを追加することも可能です。
