# Qt Creator インストール後のビルド手順

Qt Creatorを公式インストーラーでインストールした後の手順です。

## ✅ 現在の状況

- ✅ Qt Creator インストール済み
- ⏳ 残りの依存関係をインストール
- ⏳ OBS Studioを準備
- ⏳ プラグインをビルド

## 📋 次のステップ

### ステップ1: Qtのインストール場所を確認

PowerShellで以下を実行：

```powershell
# Qtの場所を確認
dir C:\Qt

# または
dir C:\Qt6
```

通常、以下のような場所にあります：
- `C:\Qt\6.6.0\msvc2022_64`
- `C:\Qt\6.7.0\msvc2022_64`
- `C:\Qt6\6.6.0\msvc2022_64`

**見つけたパスをメモしてください**（例: `C:\Qt\6.6.0\msvc2022_64`）

### ステップ2: vcpkgで残りの依存関係をインストール

```powershell
# vcpkgのディレクトリに移動
cd C:\

# vcpkgがまだない場合はインストール
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg integrate install

# すでにvcpkgがある場合
cd C:\vcpkg

# curl と nlohmann-json をインストール（数分で完了）
.\vcpkg install curl:x64-windows nlohmann-json:x64-windows
```

### ステップ3: OBS Studioの準備

#### 3-1. OBS Studioをインストール

https://obsproject.com/download からダウンロードしてインストール

#### 3-2. OBS開発ファイルをダウンロード

1. https://github.com/obsproject/obs-studio/releases/latest にアクセス
2. **Assets** から `obs-studio-XX.X.X-windows-x64.zip` をダウンロード
3. `C:\obs-studio` に展開

PowerShellで確認：
```powershell
dir C:\obs-studio
```

以下のようなフォルダが見えればOK：
- bin
- data
- obs-plugins
- include (ある場合)

### ステップ4: プロジェクトをビルド

#### 方法A: 改善されたbuild.batを使用（最も簡単！）

```powershell
# プロジェクトのディレクトリに移動
cd C:\Pokonyan-OBSObstruction-Plugin

# ビルドスクリプトを実行
.\build.bat
```

スクリプトが自動的に：
- ✅ Qtを検出（一般的なパスから自動検出）
- ✅ vcpkgを検出
- ✅ OBSディレクトリを入力（聞かれます）
- ✅ ビルド実行
- ✅ インストール先を選択

**Qtが自動検出されない場合**：
- スクリプトがQtのパスを聞いてきます
- ステップ1で確認したパスを入力してください

#### 方法B: PowerShellで直接ビルド

```powershell
cd C:\Pokonyan-OBSObstruction-Plugin
mkdir build
cd build

# CMakeで設定（パスを実際のものに変更してください）
cmake -G "Visual Studio 17 2022" -A x64 `
      -DCMAKE_PREFIX_PATH="C:\obs-studio;C:\Qt\6.6.0\msvc2022_64" `
      ..

# ビルド
cmake --build . --config Release
```

**重要**: `C:\Qt\6.6.0\msvc2022_64` の部分をステップ1で確認したパスに変更してください。

### ステップ5: プラグインをインストール

#### オプション1: ユーザーディレクトリにインストール（推奨）

管理者権限不要、開発中も便利：

```powershell
# buildディレクトリから実行
cd C:\Pokonyan-OBSObstruction-Plugin\build

# DLLをコピー
copy Release\obs-youtube-superchat-plugin.dll %APPDATA%\obs-studio\obs-plugins\64bit\

# データファイルをコピー
xcopy /E /I ..\data %APPDATA%\obs-studio\obs-plugins\obs-youtube-superchat-plugin\data\
```

#### オプション2: build.batのインストールメニューを使用

`build.bat`を実行した場合、最後にインストールオプションが表示されます：
1. ユーザーディレクトリにインストール（推奨）
2. システムディレクトリにインストール（管理者権限必要）
3. スキップ

**「1」を選択**すると自動的にインストールされます。

### ステップ6: OBSで確認

1. OBS Studioを起動（すでに起動中の場合は再起動）
2. メニューから **ツール** を開く
3. **YouTube SuperChat Plugin Settings** があれば成功！🎉

## ❓ トラブルシューティング

### Q1: CMakeがQtを見つけられない

**エラー**:
```
Could not find a package configuration file provided by "Qt6"
```

**解決方法**:
1. Qtのパスが正しいか確認
   ```powershell
   dir C:\Qt\6.6.0\msvc2022_64
   ```
2. CMakeコマンドのパスを修正
3. または環境変数を設定：
   ```powershell
   $env:Qt6_DIR = "C:\Qt\6.6.0\msvc2022_64"
   [Environment]::SetEnvironmentVariable("Qt6_DIR", "C:\Qt\6.6.0\msvc2022_64", "User")
   ```

### Q2: curlが見つからない

**エラー**:
```
Could not find CURL
```

**解決方法**:
```powershell
cd C:\vcpkg
.\vcpkg install curl:x64-windows
```

### Q3: ビルドは成功したがOBSでプラグインが表示されない

**確認事項**:
1. DLLが正しい場所にあるか
   ```powershell
   dir %APPDATA%\obs-studio\obs-plugins\64bit\obs-youtube-superchat-plugin.dll
   ```
2. OBSを再起動したか
3. OBSのログを確認（ヘルプ → ログファイル → 現在のログを表示）

### Q4: Visual Studioのバージョンエラー

**エラー**:
```
Visual Studio 17 2022 could not be found
```

**解決方法**:
- Visual Studio 2022がインストールされているか確認
- または古いバージョンを使用：
  ```powershell
  cmake -G "Visual Studio 16 2019" -A x64 ...
  ```

## 🎯 まとめ

1. ✅ Qt Creator インストール済み
2. ⏳ `vcpkg install curl:x64-windows nlohmann-json:x64-windows`
3. ⏳ OBS Studio + 開発ファイルを準備
4. ⏳ `.\build.bat` を実行
5. ⏳ OBSで確認

**最も簡単な方法**: `build.bat`を実行して、画面の指示に従ってください！

## 📚 詳細情報

- [完全なビルドガイド](VISUAL_STUDIO_SETUP.md)
- [vcpkgトラブルシューティング](VCPKG_TROUBLESHOOTING.md)
- [README](../README.md)
