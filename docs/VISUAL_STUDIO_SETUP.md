# Visual Studio セットアップガイド

このガイドでは、Visual StudioでOBS YouTube SuperChatプラグインを開発・ビルドする方法を説明します。

## 必要なソフトウェア

### 1. Visual Studio 2022
- **エディション**: Community、Professional、またはEnterprise
- **ダウンロード**: https://visualstudio.microsoft.com/ja/downloads/

#### 必要なワークロード
Visual Studioインストール時に以下を選択：
- ✅ **C++によるデスクトップ開発**
- ✅ **CMake用のC++ツール**

#### 個別のコンポーネント
「個別のコンポーネント」タブで以下も確認：
- ✅ MSVC v143 - VS 2022 C++ x64/x86ビルドツール
- ✅ Windows 10 SDK (最新版)
- ✅ CMake
- ✅ Git for Windows

### 2. CMake (スタンドアロン版 - オプション)
- **ダウンロード**: https://cmake.org/download/
- Visual Studioに含まれるCMakeでも可

### 3. Git
- **ダウンロード**: https://git-scm.com/download/win
- またはVisual Studio内蔵のGitを使用

## 依存関係のインストール

### vcpkgを使用する方法（推奨）

vcpkgはWindows向けC++パッケージマネージャーです。

#### 1. vcpkgのインストール

```powershell
# PowerShellまたはコマンドプロンプトで実行
cd C:\
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg integrate install
```

#### 2. 必要なパッケージのインストール

```powershell
# Qt6のインストール（時間がかかります）
.\vcpkg install qt6-base:x64-windows
.\vcpkg install qt6-widgets:x64-windows

# curlのインストール
.\vcpkg install curl:x64-windows

# nlohmann-jsonのインストール
.\vcpkg install nlohmann-json:x64-windows
```

**注意**: Qt6のビルドには30分〜1時間程度かかる場合があります。

### 手動インストール方法

vcpkgを使わない場合は、各ライブラリを個別にダウンロード・ビルドする必要があります。

#### Qt6
1. https://www.qt.io/download-qt-installer からインストーラーをダウンロード
2. Qt 6.x for MSVC 2022をインストール
3. 環境変数に追加：
   ```
   Qt6_DIR=C:\Qt\6.x.x\msvc2022_64
   ```

#### libcurl
1. https://curl.se/windows/ からビルド済みバイナリをダウンロード
2. 適切な場所に展開
3. 環境変数に追加

#### nlohmann-json
ヘッダーオンリーなので、CMakeが自動ダウンロードします（設定済み）

## OBS Studioの準備

プラグインをビルドするには、OBS Studioのビルド済みファイルまたはソースが必要です。

### オプション1: OBS Studio公式ビルドを使用（簡単）

1. OBS Studioをインストール
   - https://obsproject.com/download

2. OBS開発用ファイルをダウンロード
   - https://github.com/obsproject/obs-studio/releases
   - `obs-studio-xx.x.x-windows-x64.zip` をダウンロード
   - 適切な場所（例: `C:\obs-studio`）に展開

### オプション2: OBS Studioをソースからビルド（上級者向け）

詳細は公式ドキュメント参照：
https://github.com/obsproject/obs-studio/wiki/Build-Instructions-For-Windows

```powershell
git clone --recursive https://github.com/obsproject/obs-studio.git
cd obs-studio
mkdir build
cd build
cmake -G "Visual Studio 17 2022" -A x64 ..
cmake --build . --config Release
```

## プロジェクトのセットアップ

### 1. リポジトリのクローン

```powershell
cd C:\Projects
git clone https://github.com/moyashianji/Pokonyan-OBSObstruction-Plugin.git
cd Pokonyan-OBSObstruction-Plugin
```

### 2. 環境変数の設定

環境変数を設定するか、CMakeコマンド実行時に指定します。

#### 方法A: 環境変数で設定（推奨）

「システム環境変数」で以下を設定：

```
OBS_BUILD_DIR=C:\obs-studio\build
CMAKE_PREFIX_PATH=C:\obs-studio\build;C:\vcpkg\installed\x64-windows
```

#### 方法B: CMakeコマンドで指定

後述のCMake設定手順で直接パスを指定します。

### 3. Visual Studioでプロジェクトを開く

#### 方法A: 「フォルダーを開く」を使用

1. Visual Studio 2022を起動
2. **ファイル** → **開く** → **フォルダー**
3. `Pokonyan-OBSObstruction-Plugin` フォルダーを選択
4. Visual StudioがCMakeを自動検出して設定開始

#### 方法B: CMakeでソリューションを生成

```powershell
cd Pokonyan-OBSObstruction-Plugin
mkdir build
cd build

# vcpkg使用の場合
cmake -G "Visual Studio 17 2022" -A x64 ^
      -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake ^
      -DCMAKE_PREFIX_PATH=C:\obs-studio\build ^
      ..

# vcpkg未使用の場合
cmake -G "Visual Studio 17 2022" -A x64 ^
      -DCMAKE_PREFIX_PATH="C:\obs-studio\build;C:\Qt\6.x.x\msvc2022_64" ^
      ..
```

生成された `obs-youtube-superchat-plugin.sln` をVisual Studioで開く

### 4. CMake設定のカスタマイズ（フォルダーを開く場合）

`CMakeSettings.json` を編集：

```json
{
  "configurations": [
    {
      "name": "x64-Release",
      "generator": "Ninja",
      "configurationType": "Release",
      "inheritEnvironments": [ "msvc_x64_x64" ],
      "buildRoot": "${projectDir}\\build\\${name}",
      "installRoot": "${projectDir}\\install\\${name}",
      "cmakeCommandArgs": "",
      "buildCommandArgs": "",
      "ctestCommandArgs": "",
      "cmakeToolchain": "C:/vcpkg/scripts/buildsystems/vcpkg.cmake",
      "variables": [
        {
          "name": "CMAKE_PREFIX_PATH",
          "value": "C:/obs-studio/build",
          "type": "PATH"
        }
      ]
    },
    {
      "name": "x64-Debug",
      "generator": "Ninja",
      "configurationType": "Debug",
      "inheritEnvironments": [ "msvc_x64_x64" ],
      "buildRoot": "${projectDir}\\build\\${name}",
      "installRoot": "${projectDir}\\install\\${name}",
      "cmakeCommandArgs": "",
      "buildCommandArgs": "",
      "ctestCommandArgs": "",
      "cmakeToolchain": "C:/vcpkg/scripts/buildsystems/vcpkg.cmake",
      "variables": [
        {
          "name": "CMAKE_PREFIX_PATH",
          "value": "C:/obs-studio/build",
          "type": "PATH"
        }
      ]
    }
  ]
}
```

## ビルド方法

### Visual Studio内でビルド

#### 「フォルダーを開く」の場合
1. **ビルド** → **すべてビルド**
2. または `Ctrl + Shift + B`

#### ソリューションを開いた場合
1. ソリューションエクスプローラーで `obs-youtube-superchat-plugin` プロジェクトを右クリック
2. **ビルド**を選択

### コマンドラインでビルド

```powershell
cd build
cmake --build . --config Release
```

## インストール方法

### 自動インストール

```powershell
cd build
cmake --install . --config Release
```

デフォルトのインストール先：
```
C:\Program Files\obs-studio\obs-plugins\64bit\
C:\Program Files\obs-studio\data\obs-plugins\obs-youtube-superchat-plugin\
```

**注意**: 管理者権限が必要です。

### 手動インストール

ビルド成果物を手動でコピー：

1. **プラグインDLL:**
   ```
   コピー元: build\Release\obs-youtube-superchat-plugin.dll
   コピー先: C:\Program Files\obs-studio\obs-plugins\64bit\
   ```

2. **データファイル:**
   ```
   コピー元: data\
   コピー先: C:\Program Files\obs-studio\data\obs-plugins\obs-youtube-superchat-plugin\
   ```

### ユーザーディレクトリにインストール（推奨）

管理者権限不要で、開発中に便利：

```
プラグインDLL:
%APPDATA%\obs-studio\obs-plugins\64bit\obs-youtube-superchat-plugin.dll

データ:
%APPDATA%\obs-studio\obs-plugins\obs-youtube-superchat-plugin\data\
```

実際のパス例：
```
C:\Users\YourName\AppData\Roaming\obs-studio\obs-plugins\64bit\
C:\Users\YourName\AppData\Roaming\obs-studio\obs-plugins\obs-youtube-superchat-plugin\data\
```

## デバッグ方法

### 1. デバッグ設定

#### 「フォルダーを開く」の場合

`launch.vs.json` を作成：

```json
{
  "version": "0.2.1",
  "defaults": {},
  "configurations": [
    {
      "type": "default",
      "project": "CMakeLists.txt",
      "projectTarget": "obs-youtube-superchat-plugin.dll",
      "name": "OBS Studio (Debug)",
      "program": "C:\\Program Files\\obs-studio\\bin\\64bit\\obs64.exe",
      "args": [],
      "cwd": "${workspaceRoot}",
      "env": {}
    }
  ]
}
```

#### ソリューションの場合

1. `obs-youtube-superchat-plugin` プロジェクトを右クリック
2. **プロパティ** → **デバッグ**
3. **コマンド**に設定：
   ```
   C:\Program Files\obs-studio\bin\64bit\obs64.exe
   ```

### 2. デバッグ実行

1. ブレークポイントを設定
2. **F5** でデバッグ開始
3. OBS Studioが起動し、プラグインがロード

### 3. ログ確認

OBSのログファイル：
```
%APPDATA%\obs-studio\logs\
```

プラグインのログは `obs_log()` で出力され、OBSのログファイルに記録されます。

## トラブルシューティング

### CMakeエラー: libobs not found

**原因**: OBS Studioのビルドディレクトリが見つからない

**解決方法**:
```powershell
# CMAKE_PREFIX_PATHを正しく設定
cmake -DCMAKE_PREFIX_PATH=C:\path\to\obs-studio\build ..
```

### Qt6が見つからない

**原因**: Qt6のパスが通っていない

**解決方法**:
```powershell
# vcpkg使用の場合
cmake -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake ..

# 手動インストールの場合
cmake -DCMAKE_PREFIX_PATH=C:\Qt\6.x.x\msvc2022_64 ..
```

### curlが見つからない

**解決方法**:
```powershell
# vcpkgでインストール
vcpkg install curl:x64-windows
```

### プラグインがOBSに読み込まれない

**確認事項**:
1. DLLが正しいディレクトリにあるか
2. 依存DLL（Qt6、curl）がPATHに含まれているか
3. OBSのログでエラーを確認

**依存DLLの確認**:
```powershell
# Dependency Walkerまたはdumpbinで確認
dumpbin /dependents obs-youtube-superchat-plugin.dll
```

### ビルドは成功するがOBSでクラッシュ

**原因**: OBSとプラグインのビルド構成が異なる可能性

**解決方法**:
- OBSがReleaseビルドの場合、プラグインもReleaseでビルド
- Visual Studioのバージョンを合わせる
- 同じランタイムライブラリを使用

## 開発のヒント

### 高速なイテレーション開発

1. **ポストビルドイベントを設定**

CMakeLists.txtに追加：
```cmake
if(WIN32)
    add_custom_command(TARGET obs-youtube-superchat-plugin POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            $<TARGET_FILE:obs-youtube-superchat-plugin>
            "$ENV{APPDATA}/obs-studio/obs-plugins/64bit/"
    )
endif()
```

2. **OBSの自動再起動スクリプト**

`reload_obs.bat`:
```batch
@echo off
taskkill /F /IM obs64.exe
timeout /t 2
start "" "C:\Program Files\obs-studio\bin\64bit\obs64.exe"
```

### ホットリロード（上級）

OBSプラグインは通常ホットリロードできませんが、開発中は以下の方法で効率化：

1. OBSを閉じる
2. ビルド
3. OBSを起動

上記を自動化するスクリプトを作成すると便利です。

## おすすめの拡張機能

Visual Studio Marketplace から以下をインストール：

- **CMake Tools**: CMakeプロジェクトの管理
- **C++ Productivity Power Tools**: コーディング効率向上
- **Visual Studio Spell Checker**: スペルチェック

## さらなる情報

- OBS Studio Wiki: https://obsproject.com/wiki/
- OBS Plugin Development: https://obsproject.com/docs/plugins.html
- Qt Documentation: https://doc.qt.io/
