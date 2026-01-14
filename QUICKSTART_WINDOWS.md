# Windows向けクイックスタートガイド

Visual Studioを使ってプラグインをビルドして使うまでの最短手順です。

## 📋 必要なもの

- Windows 10/11 (64bit)
- Visual Studio 2022 (Community版でOK)
- インターネット接続

## 🚀 5ステップで始める

### ステップ1: Visual Studio 2022のインストール

1. https://visualstudio.microsoft.com/ja/downloads/ からダウンロード
2. インストール時に **「C++によるデスクトップ開発」** を選択
3. インストール完了まで待つ（20-30分程度）

### ステップ2: vcpkg（パッケージマネージャー）のセットアップ

**PowerShell**を管理者権限で開いて実行：

```powershell
# vcpkgをC:\にインストール
cd C:\
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg integrate install
```

### ステップ3: 必要なライブラリをインストール

同じPowerShellで続けて実行。

#### まず、vcpkgを最新版に更新

```powershell
git pull
.\bootstrap-vcpkg.bat
```

#### 利用可能なQtパッケージを確認

```powershell
.\vcpkg search qt6
```

#### Qtをインストール（**20-60分かかります**、コーヒータイム☕）

以下のいずれかを実行：

**Qt6の場合（推奨）:**
```powershell
# 新しいvcpkgの場合
.\vcpkg install qt:x64-windows

# または従来の方法
.\vcpkg install qt6-base:x64-windows qt6-widgets:x64-windows qt6-network:x64-windows
```

**Qt6が見つからない場合はQt5を使用:**
```powershell
.\vcpkg install qt5-base:x64-windows qt5-widgets:x64-windows qt5-network:x64-windows
```

**注意**: `qt6-base does not exist`エラーが出る場合は、[vcpkgトラブルシューティング](docs/VCPKG_TROUBLESHOOTING.md)を参照してください。

#### その他のライブラリ

```powershell
.\vcpkg install curl:x64-windows nlohmann-json:x64-windows
```

### ステップ4: OBS Studioの準備

#### 簡単な方法（推奨）:

1. OBS Studioをインストール
   - https://obsproject.com/download からダウンロード＆インストール

2. OBS開発ファイルをダウンロード
   - https://github.com/obsproject/obs-studio/releases/latest
   - `obs-studio-XX.X.X-windows-x64.zip` をダウンロード
   - `C:\obs-studio` に展開

### ステップ5: プラグインのビルド

**PowerShell**で実行：

```powershell
# プロジェクトをクローン
cd C:\
git clone https://github.com/moyashianji/Pokonyan-OBSObstruction-Plugin.git
cd Pokonyan-OBSObstruction-Plugin

# ビルドディレクトリを作成
mkdir build
cd build

# CMakeでプロジェクトを設定
cmake -G "Visual Studio 17 2022" -A x64 ^
      -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake ^
      -DCMAKE_PREFIX_PATH=C:\obs-studio ^
      ..

# ビルド実行
cmake --build . --config Release
```

### ステップ6: プラグインをインストール

#### 方法A: 自動インストール（管理者権限必要）

```powershell
# 管理者権限のPowerShellで実行
cmake --install . --config Release
```

#### 方法B: 手動コピー（推奨）

エクスプローラーで以下をコピー：

1. **プラグインファイル:**
   ```
   コピー元:
   C:\Pokonyan-OBSObstruction-Plugin\build\Release\obs-youtube-superchat-plugin.dll

   コピー先:
   C:\Users\[ユーザー名]\AppData\Roaming\obs-studio\obs-plugins\64bit\
   ```
   ※ フォルダが存在しない場合は作成

2. **データファイル:**
   ```
   コピー元:
   C:\Pokonyan-OBSObstruction-Plugin\data\

   コピー先:
   C:\Users\[ユーザー名]\AppData\Roaming\obs-studio\obs-plugins\obs-youtube-superchat-plugin\
   ```

### ステップ7: OBSで確認

1. OBS Studioを起動
2. メニューから **ツール** を開く
3. **YouTube SuperChat Plugin Settings** があればインストール成功！🎉

## 🎮 使い方

### 1. YouTube API Keyを取得

1. https://console.cloud.google.com/ にアクセス
2. 新規プロジェクトを作成
3. **YouTube Data API v3** を有効化
4. **認証情報** → **APIキーを作成**
5. キーをコピー

### 2. プラグインを設定

1. OBSで **ツール** → **YouTube SuperChat Plugin Settings** を開く
2. 以下を入力：
   - **API Key**: 取得したAPIキー
   - **Video ID**: YouTube配信のID（URLの`v=`の後の部分）
   - **Main Source**: 縮小したいソース名（例: "ゲームキャプチャ"）
3. **Enable Obstructions** と **Enable Recovery** にチェック
4. **Save** をクリック
5. **Start Monitoring** をクリック

### 3. 配信開始

YouTube Liveで配信を開始すると、スーパーチャット/スーパーステッカーに自動的に反応します！

## ❓ よくあるトラブル

### Q: 「CMakeが見つかりません」エラー

**A**: Visual Studioの「CMake用のC++ツール」がインストールされているか確認
- Visual Studio Installer → 変更 → 個別のコンポーネント → CMake

### Q: 「qt6-base does not exist」エラー

**A**: vcpkgのパッケージ名が異なる可能性があります。以下を試してください：

```powershell
cd C:\vcpkg

# vcpkgを最新版に更新
git pull
.\bootstrap-vcpkg.bat

# 利用可能なパッケージを検索
.\vcpkg search qt6

# Qt6が見つからない場合はQt5を使用（このプロジェクトはQt5/Qt6両対応）
.\vcpkg install qt5-base:x64-windows qt5-widgets:x64-windows qt5-network:x64-windows
```

詳細は [vcpkgトラブルシューティング](docs/VCPKG_TROUBLESHOOTING.md) を参照してください。

### Q: 「libobs.libが見つかりません」エラー

**A**: CMAKE_PREFIX_PATHが正しいか確認
```powershell
# OBS Studioの展開先を確認
dir C:\obs-studio
```

### Q: プラグインがOBSに表示されない

**A**: 以下を確認：
1. DLLが正しい場所にあるか
   - `%APPDATA%\obs-studio\obs-plugins\64bit\obs-youtube-superchat-plugin.dll`
2. OBSを再起動したか
3. OBSのログを確認（ヘルプ → ログファイル → 現在のログを表示）

### Q: ビルドに失敗する

**A**: 以下を試す：
1. buildフォルダを削除して最初からやり直す
   ```powershell
   cd C:\Pokonyan-OBSObstruction-Plugin
   rmdir /s build
   mkdir build
   cd build
   # CMakeコマンドを再実行
   ```
2. vcpkgのパッケージを再インストール
3. Visual Studioを最新版に更新

## 📝 次のステップ

### カスタム障害物を追加

1. 好きな画像や動画を準備（PNG、JPG、MP4など）
2. 以下のフォルダに配置：
   ```
   %APPDATA%\obs-studio\obs-plugins\obs-youtube-superchat-plugin\obstruction_assets\
   ```
3. プラグインが自動的にランダム表示します

### 効果をカスタマイズ

設定画面で **Obstruction Intensity** と **Recovery Intensity** を調整して、効果の強さを変更できます。
- 0.5 = 弱め
- 1.0 = 標準（デフォルト）
- 2.0 = 強め

## 📚 詳細情報

- **完全なドキュメント**: `README.md`
- **Visual Studio詳細ガイド**: `docs/VISUAL_STUDIO_SETUP.md`
- **APIリファレンス**: `docs/API.md`
- **セットアップガイド**: `docs/SETUP_GUIDE.md`

## 💡 ヒント

- 初回ビルドは時間がかかりますが、2回目以降は数分で完了します
- 開発中は、OBSを閉じてからビルド→コピー→OBS起動のサイクルで進めます
- Visual Studioの「フォルダーを開く」機能でプロジェクトを開くと、CMakeが自動設定されます

---

楽しい配信ライフを！🎮✨
