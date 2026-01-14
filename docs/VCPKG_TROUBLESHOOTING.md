# vcpkg トラブルシューティングガイド

vcpkgでQt6のインストールに問題が発生した場合の対処方法をまとめました。

## 問題1: qt6-base が見つからない

### エラーメッセージ
```
error: while looking for qt6-base:x64-windows:
C:\vcpkg\ports\qt6-base: error: qt6-base does not exist
```

### 原因
vcpkgのポートリストが古いか、パッケージ名が異なる可能性があります。

### 解決方法

#### ステップ1: vcpkgを最新版に更新

```powershell
cd C:\vcpkg
git pull
.\bootstrap-vcpkg.bat
```

#### ステップ2: 利用可能なQt関連パッケージを検索

```powershell
# Qt6パッケージを検索
.\vcpkg search qt6

# すべてのQtパッケージを検索
.\vcpkg search qt

# インストール済みパッケージを確認
.\vcpkg list
```

#### ステップ3: 正しいパッケージ名でインストール

vcpkgのバージョンによって、パッケージ名が異なります。以下のいずれかを試してください：

##### オプションA: 最新のvcpkg（2024年以降）

```powershell
# qtがQt6を指す場合
.\vcpkg install qt:x64-windows

# または個別モジュール
.\vcpkg install qtbase:x64-windows
```

##### オプションB: 従来のvcpkg

```powershell
# Qt6の個別パッケージ
.\vcpkg install qt6-base:x64-windows
.\vcpkg install qt6-widgets:x64-windows
```

##### オプションC: Qt5を使用（フォールバック）

このプロジェクトはQt5/Qt6両対応になっています：

```powershell
.\vcpkg install qt5-base:x64-windows
.\vcpkg install qt5-widgets:x64-windows
```

#### ステップ4: その他の依存関係

```powershell
.\vcpkg install curl:x64-windows
.\vcpkg install nlohmann-json:x64-windows
```

## 問題2: vcpkgのインストールが非常に遅い

### 原因
Qtは大規模なライブラリで、ソースからビルドするため時間がかかります。

### 対処方法

#### オプション1: バイナリキャッシュを有効化

```powershell
# バイナリキャッシュを有効化（インストールを高速化）
.\vcpkg install qt --binarysource=clear;nuget,https://nuget.pkg.github.com/microsoft/index.json,readwrite
```

#### オプション2: Qt公式インストーラーを使用

vcpkgの代わりにQt公式インストーラーを使うと高速です：

1. https://www.qt.io/download-qt-installer からダウンロード
2. Qt 6.x → MSVC 2022 64-bitを選択してインストール
3. 環境変数を設定：
   ```powershell
   [Environment]::SetEnvironmentVariable("Qt6_DIR", "C:\Qt\6.6.0\msvc2022_64", "User")
   ```

4. ビルド時にパスを指定：
   ```powershell
   cmake -DCMAKE_PREFIX_PATH="C:\obs-studio;C:\Qt\6.6.0\msvc2022_64" ..
   ```

## 問題3: vcpkg integrate install が失敗

### エラーメッセージ
```
error: could not write to C:\Program Files\...
```

### 解決方法

PowerShellを**管理者権限**で実行：

```powershell
# PowerShellを右クリック → 管理者として実行
cd C:\vcpkg
.\vcpkg integrate install
```

## 問題4: ビルド時にQt6が見つからない

### エラーメッセージ
```
CMake Error at CMakeLists.txt:12 (find_package):
  Could not find a package configuration file provided by "Qt6"
```

### 解決方法

#### 方法1: vcpkg toolchainファイルを指定

```powershell
cmake -G "Visual Studio 17 2022" -A x64 ^
      -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake ^
      -DCMAKE_PREFIX_PATH=C:\obs-studio ^
      ..
```

#### 方法2: build.batスクリプトを使用

修正済みの`build.bat`を使えば自動的にvcpkgを検出します：

```batch
# プロジェクトのルートディレクトリで実行
.\build.bat
```

## 問題5: Qt5とQt6が混在してエラー

### 解決方法

#### オプション1: 不要なバージョンをアンインストール

```powershell
# Qt5をアンインストール
.\vcpkg remove qt5-base:x64-windows qt5-widgets:x64-windows

# または Qt6をアンインストール
.\vcpkg remove qt6-base:x64-windows qt6-widgets:x64-windows
```

#### オプション2: 明示的にバージョンを指定

CMakeコマンドでQt6を優先：

```powershell
cmake -DQT_VERSION_MAJOR=6 ..
```

## 推奨されるインストール手順（まとめ）

### 方法A: vcpkg使用（推奨・簡単）

```powershell
# 1. vcpkgをセットアップ
cd C:\
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg integrate install

# 2. vcpkgを最新版に更新
git pull
.\bootstrap-vcpkg.bat

# 3. 利用可能なQtパッケージを確認
.\vcpkg search qt

# 4. Qt5をインストール（Qt6が利用できない場合）
.\vcpkg install qt5-base:x64-windows
.\vcpkg install qt5-widgets:x64-windows
.\vcpkg install qt5-network:x64-windows

# 5. その他の依存関係
.\vcpkg install curl:x64-windows
.\vcpkg install nlohmann-json:x64-windows

# 6. プロジェクトをビルド
cd C:\Pokonyan-OBSObstruction-Plugin
.\build.bat
```

### 方法B: Qt公式インストーラー使用（高速）

```powershell
# 1. Qt公式インストーラーでQt6をインストール
# https://www.qt.io/download-qt-installer

# 2. vcpkgで残りの依存関係をインストール
cd C:\vcpkg
.\vcpkg install curl:x64-windows
.\vcpkg install nlohmann-json:x64-windows

# 3. 環境変数を設定
$env:Qt6_DIR = "C:\Qt\6.6.0\msvc2022_64"

# 4. ビルド
cd C:\Pokonyan-OBSObstruction-Plugin
mkdir build
cd build
cmake -G "Visual Studio 17 2022" -A x64 ^
      -DCMAKE_PREFIX_PATH="C:\obs-studio;C:\Qt\6.6.0\msvc2022_64" ^
      ..
cmake --build . --config Release
```

## よくある質問

### Q1: Qt6とQt5、どちらを使うべき？

**A**: 基本的にはQt6を推奨しますが、以下の場合はQt5でも問題ありません：
- vcpkgでQt6が利用できない
- インストール時間を短縮したい
- OBSがQt5でビルドされている場合

このプロジェクトはQt5/Qt6両対応です。

### Q2: vcpkgのインストールにどれくらい時間がかかる？

**A**:
- Qt6: 30分〜2時間（初回ビルド）
- Qt5: 20分〜1時間
- curl, nlohmann-json: 数分

バイナリキャッシュを有効化すると大幅に短縮できます。

### Q3: vcpkgとQt公式、どちらが良い？

**A**:
- **vcpkg**: 統一的な管理、自動リンク、複数プロジェクトで共有
- **Qt公式**: インストールが速い、最新版が利用可能、IDE統合

初心者には**Qt公式インストーラー**、複数プロジェクトがある場合は**vcpkg**を推奨。

### Q4: エラーが解決しない場合は？

**A**: 以下を試してください：
1. buildディレクトリを削除して最初から
   ```powershell
   rmdir /s /q build
   ```
2. vcpkgを再インストール
   ```powershell
   cd C:\
   rmdir /s /q vcpkg
   # 最初からやり直す
   ```
3. Visual Studioを最新版に更新

### Q5: OBSのQt version mismatchエラー

**エラー**:
```
Qt version mismatch: OBS uses Qt5 but plugin uses Qt6
```

**解決方法**:
OBSがQt5でビルドされている場合、プラグインもQt5でビルド：
```powershell
.\vcpkg install qt5-base:x64-windows qt5-widgets:x64-windows
```

CMakeが自動的にQt5を検出します。

## 参考リンク

- vcpkg公式: https://vcpkg.io/
- Qt公式: https://www.qt.io/
- OBS Studio: https://obsproject.com/
- このプロジェクトのissue: https://github.com/moyashianji/Pokonyan-OBSObstruction-Plugin/issues

## トラブルが解決しない場合

GitHubのIssueで質問してください。以下の情報を含めると回答が早くなります：

1. Windowsのバージョン
2. Visual Studioのバージョン
3. vcpkgのバージョン（`git log -1 --oneline`の結果）
4. 実行したコマンド
5. エラーメッセージの全文
6. CMakeのログ
