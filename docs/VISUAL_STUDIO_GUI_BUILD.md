# Visual Studio GUI でビルドする完全ガイド

Visual StudioのGUI（IDE）を使ってプラグインをビルドする方法です。

## 📋 前提条件

- ✅ Visual Studio 2022 インストール済み
- ✅ Qt Creator インストール済み（または vcpkg でQt5/Qt6）
- ⏳ vcpkg で curl, nlohmann-json をインストール
- ⏳ OBS Studio の準備

## 🚀 ステップ1: リポジトリをクローン

### Visual Studio内でクローン（推奨）

1. **Visual Studio 2022を起動**

2. **スタート画面で「リポジトリをクローンする」をクリック**
   - または、**Git → リポジトリをクローンする**

3. **リポジトリの場所：**
   ```
   https://github.com/moyashianji/Pokonyan-OBSObstruction-Plugin.git
   ```

4. **パス：**
   ```
   C:\Projects\Pokonyan-OBSObstruction-Plugin
   ```

5. **「クローン」をクリック**

### コマンドラインでクローン

```powershell
cd C:\Projects
git clone https://github.com/moyashianji/Pokonyan-OBSObstruction-Plugin.git
```

その後、Visual Studioで開く：
- **ファイル → 開く → フォルダー**
- `C:\Projects\Pokonyan-OBSObstruction-Plugin` を選択

## 📝 ステップ2: CMake設定を編集

### 2-1. CMakeSettings.jsonを開く

プロジェクトを開くと、リポジトリに含まれる `CMakeSettings.json` が自動的に読み込まれます。

### 2-2. パスを自分の環境に合わせる

`CMakeSettings.json` を編集して、以下のパスを実際の環境に合わせてください：

```json
{
  "configurations": [
    {
      "name": "x64-Release",
      "variables": [
        {
          "name": "CMAKE_PREFIX_PATH",
          "value": "C:/obs-studio;C:/Qt/6.6.0/msvc2022_64",  // ← ここを変更
          "type": "STRING"
        },
        {
          "name": "CMAKE_TOOLCHAIN_FILE",
          "value": "C:/vcpkg/scripts/buildsystems/vcpkg.cmake",  // ← vcpkgのパス
          "type": "FILEPATH"
        }
      ]
    }
  ]
}
```

**変更が必要な箇所：**

1. **OBS Studioのパス:**
   - `C:/obs-studio` → あなたがOBSを展開した場所

2. **Qtのパス:**
   - `C:/Qt/6.6.0/msvc2022_64` → 実際のQtのインストール先
   - 確認方法: エクスプローラーで `C:\Qt` を開いて確認

3. **vcpkgのパス:**
   - `C:/vcpkg/scripts/buildsystems/vcpkg.cmake` → vcpkgの場所

**Qtのパスを確認：**
```powershell
# PowerShellで実行
dir C:\Qt
# または
dir C:\Qt6
```

一般的なパス：
- `C:/Qt/6.6.0/msvc2022_64`
- `C:/Qt/6.7.0/msvc2022_64`
- `C:/Qt6/6.6.0/msvc2022_64`

### 2-3. 設定を保存

`CMakeSettings.json` を保存すると、Visual StudioがCMakeキャッシュを再生成します。

## 🔧 ステップ3: 依存関係をインストール

ビルド前に、curlとnlohmann-jsonが必要です。

```powershell
# PowerShellで実行
cd C:\vcpkg

# vcpkgがまだない場合
git clone https://github.com/Microsoft/vcpkg.git .
.\bootstrap-vcpkg.bat
.\vcpkg integrate install

# 依存関係をインストール
.\vcpkg install curl:x64-windows nlohmann-json:x64-windows
```

## 🏗️ ステップ4: ビルド

### 4-1. ビルド構成を選択

Visual Studio上部のツールバーで：
- **x64-Release** または **x64-Debug** を選択

### 4-2. ビルド実行

以下のいずれかの方法で：

**方法1: メニューから**
- **ビルド → すべてビルド**

**方法2: キーボードショートカット**
- **Ctrl + Shift + B**

**方法3: ソリューションエクスプローラーから**
- `CMakeLists.txt` を右クリック
- **「ビルド」** をクリック

### 4-3. ビルド成功を確認

**出力ウィンドウ** に以下のように表示されれば成功：
```
ビルドが完了しました。
```

ビルド成果物は以下に生成されます：
```
C:\Projects\Pokonyan-OBSObstruction-Plugin\build\x64-Release\obs-youtube-superchat-plugin.dll
```

## 📦 ステップ5: プラグインをインストール

### 方法A: 手動でコピー（推奨）

1. **エクスプローラーでビルドフォルダを開く**
   ```
   C:\Projects\Pokonyan-OBSObstruction-Plugin\build\x64-Release
   ```

2. **DLLをコピー**
   ```
   コピー元:
   obs-youtube-superchat-plugin.dll

   コピー先:
   C:\Users\[ユーザー名]\AppData\Roaming\obs-studio\obs-plugins\64bit\
   ```

   フォルダが存在しない場合は作成してください。

3. **データファイルをコピー**
   ```
   コピー元:
   C:\Projects\Pokonyan-OBSObstruction-Plugin\data\

   コピー先:
   C:\Users\[ユーザー名]\AppData\Roaming\obs-studio\obs-plugins\obs-youtube-superchat-plugin\
   ```

### 方法B: PowerShellでコピー

```powershell
# プロジェクトのディレクトリで実行
cd C:\Projects\Pokonyan-OBSObstruction-Plugin

# DLLをコピー
copy build\x64-Release\obs-youtube-superchat-plugin.dll %APPDATA%\obs-studio\obs-plugins\64bit\

# データをコピー
xcopy /E /I data %APPDATA%\obs-studio\obs-plugins\obs-youtube-superchat-plugin\data\
```

### 方法C: CMakeでインストール

**開発者PowerShell for VS 2022** を管理者権限で開いて：

```powershell
cd C:\Projects\Pokonyan-OBSObstruction-Plugin\build\x64-Release
cmake --install . --config Release
```

## 🎯 ステップ6: OBSで確認

1. **OBS Studioを起動**（すでに起動中の場合は再起動）

2. **ツール** メニューを開く

3. **YouTube SuperChat Plugin Settings** があれば成功！🎉

## 🐛 デバッグ方法

### デバッグ設定

リポジトリに含まれる `launch.vs.json` により、Visual StudioからOBSを起動してデバッグできます。

### デバッグ実行

1. **デバッグ構成を選択**
   - ツールバーで **「OBS Studio (Debug)」** を選択

2. **ブレークポイントを設定**
   - ソースコードの行番号をクリック

3. **デバッグ開始**
   - **F5** を押す
   - または **デバッグ → デバッグの開始**

4. **OBS Studioが起動**してデバッグ可能になります

### デバッグのヒント

- **x64-Debug** 構成でビルドすること
- OBSのログは `%APPDATA%\obs-studio\logs\` にあります
- プラグインのログは `obs_log()` で出力されます

## 📂 プロジェクト構造（Visual Studio内）

```
Pokonyan-OBSObstruction-Plugin/
├── CMakeLists.txt              # CMake設定（メイン）
├── CMakeSettings.json          # Visual Studio用CMake設定
├── launch.vs.json              # デバッグ設定
├── src/
│   ├── plugin-main.cpp
│   ├── youtube-chat-client.cpp
│   ├── obstruction-manager.cpp
│   └── settings-dialog.cpp
├── data/                       # プラグインデータ
└── docs/                       # ドキュメント
```

## ⚙️ 詳細設定

### CMakeSettings.jsonの詳細

```json
{
  "configurations": [
    {
      "name": "x64-Release",              // 構成名
      "generator": "Ninja",                // ビルドシステム
      "configurationType": "Release",      // リリースビルド
      "buildRoot": "${projectDir}\\build\\${name}",  // ビルド出力先
      "variables": [
        {
          "name": "CMAKE_PREFIX_PATH",     // OBSとQtの場所
          "value": "C:/obs-studio;C:/Qt/6.6.0/msvc2022_64"
        },
        {
          "name": "CMAKE_TOOLCHAIN_FILE",  // vcpkg統合
          "value": "C:/vcpkg/scripts/buildsystems/vcpkg.cmake"
        }
      ]
    }
  ]
}
```

### vcpkgを使わない場合

`CMAKE_TOOLCHAIN_FILE` の行を削除：

```json
{
  "variables": [
    {
      "name": "CMAKE_PREFIX_PATH",
      "value": "C:/obs-studio;C:/Qt/6.6.0/msvc2022_64"
    }
    // CMAKE_TOOLCHAIN_FILE の行を削除
  ]
}
```

ただし、curlとnlohmann-jsonを手動でインストールする必要があります。

## 🔄 高速な開発サイクル

### 自動コピースクリプト

ビルド後に自動的にOBSディレクトリにコピーするスクリプト：

`copy_to_obs.bat` を作成：
```batch
@echo off
set BUILD_DIR=build\x64-Release
set OBS_PLUGINS=%APPDATA%\obs-studio\obs-plugins\64bit
set OBS_DATA=%APPDATA%\obs-studio\obs-plugins\obs-youtube-superchat-plugin

copy /Y %BUILD_DIR%\obs-youtube-superchat-plugin.dll %OBS_PLUGINS%\
xcopy /E /I /Y data %OBS_DATA%\data\

echo Plugin copied to OBS!
```

ビルド後に実行：
```powershell
.\copy_to_obs.bat
```

### ポストビルドイベント（上級）

CMakeLists.txtに追加すると、ビルド後に自動コピー：

```cmake
if(WIN32)
    add_custom_command(TARGET obs-youtube-superchat-plugin POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            $<TARGET_FILE:obs-youtube-superchat-plugin>
            "$ENV{APPDATA}/obs-studio/obs-plugins/64bit/"
    )
endif()
```

## ❓ トラブルシューティング

### Q1: CMakeキャッシュの生成に失敗

**症状**: 「CMakeキャッシュの生成に失敗しました」

**解決方法**:
1. `CMakeSettings.json` のパスを確認
2. **プロジェクト → CMakeキャッシュの削除**
3. **プロジェクト → CMakeの構成**

### Q2: Qt6が見つからない

**症状**: `Could not find a package configuration file provided by "Qt6"`

**解決方法**:
1. Qtのパスを確認
   ```powershell
   dir C:\Qt
   ```
2. `CMakeSettings.json` の `CMAKE_PREFIX_PATH` を更新
3. CMakeキャッシュを再生成

### Q3: curlが見つからない

**症状**: `Could not find CURL`

**解決方法**:
```powershell
cd C:\vcpkg
.\vcpkg install curl:x64-windows
```

その後、Visual Studioでキャッシュを再生成。

### Q4: ビルドは成功するがOBSで読み込まれない

**確認事項**:
1. DLLが正しい場所にあるか
   ```powershell
   dir %APPDATA%\obs-studio\obs-plugins\64bit\obs-youtube-superchat-plugin.dll
   ```
2. OBSを再起動したか
3. OBSのログを確認
   - **ヘルプ → ログファイル → 現在のログを表示**

### Q5: デバッグ時にOBSが起動しない

**解決方法**:
1. `launch.vs.json` のOBSパスを確認
2. OBSがインストールされているか確認
   ```powershell
   dir "C:\Program Files\obs-studio\bin\64bit\obs64.exe"
   ```

## 💡 ヒント

1. **インテリセンスが動作しない場合**
   - CMakeキャッシュを再生成
   - Visual Studioを再起動

2. **ビルドが遅い場合**
   - Ninjaジェネレーターを使用（デフォルト）
   - 並列ビルドを有効化（自動）

3. **Git統合**
   - Visual Studio内でコミット、プッシュ可能
   - **Git → コミット** または **Git → プッシュ**

4. **複数構成の切り替え**
   - Release: 本番用（最適化）
   - Debug: デバッグ用（シンボル付き）

## 📚 関連ドキュメント

- [Windows向けクイックスタート](../QUICKSTART_WINDOWS.md)
- [Visual Studioセットアップガイド](VISUAL_STUDIO_SETUP.md)
- [vcpkgトラブルシューティング](VCPKG_TROUBLESHOOTING.md)
- [Qt Creator後の手順](QT_CREATOR_INSTALL_NEXT_STEPS.md)

## ✅ チェックリスト

セットアップ完了の確認：

- [ ] Visual Studio 2022 インストール済み
- [ ] リポジトリをクローン済み
- [ ] CMakeSettings.json のパスを編集済み
- [ ] vcpkg で curl, nlohmann-json インストール済み
- [ ] Qt インストール済み（公式またはvcpkg）
- [ ] OBS Studio と開発ファイル準備済み
- [ ] Visual Studio でビルド成功
- [ ] プラグインをOBSディレクトリにコピー済み
- [ ] OBSでプラグインが表示される

すべてチェックが付けば完了です！🎉
