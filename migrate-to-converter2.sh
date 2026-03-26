#!/bin/bash
# Universal Converter → universal-converter2 移行スクリプト
# 使い方: bash migrate-to-converter2.sh

set -e

echo "=== Universal Converter 移行開始 ==="

# 一時ディレクトリ
WORK_DIR=$(mktemp -d)
echo "作業ディレクトリ: $WORK_DIR"

# 1. universal-converter2 をクローン
echo "[1/5] universal-converter2 をクローン中..."
git clone https://github.com/moyashianji/universal-converter2.git "$WORK_DIR/converter2"

# 2. ソースリポジトリをクローン（converterブランチ）
echo "[2/5] ソースコードを取得中..."
git clone --branch claude/universal-converter-site-ACFEe --single-branch \
  https://github.com/moyashianji/Pokonyan-OBSObstruction-Plugin.git "$WORK_DIR/source"

# 3. ファイルをコピー
echo "[3/5] ファイルをコピー中..."
cd "$WORK_DIR/converter2"

# 既存ファイルを一旦クリーン（READMEは残す）
rm -f package.json tsconfig.json vite.config.ts svelte.config.js

# コピー対象
cp -r "$WORK_DIR/source/src" .
cp -r "$WORK_DIR/source/static" .
cp -r "$WORK_DIR/source/.github" .
cp "$WORK_DIR/source/.gitignore" .
cp "$WORK_DIR/source/package.json" .
cp "$WORK_DIR/source/package-lock.json" .
cp "$WORK_DIR/source/tsconfig.json" .
cp "$WORK_DIR/source/vite.config.ts" .
cp "$WORK_DIR/source/svelte.config.js" .

# deploy.yml のブランチをmainのみに修正
sed -i 's/branches: \[main, claude\/universal-converter-site-ACFEe\]/branches: [main]/' .github/workflows/deploy.yml

# 4. コミット
echo "[4/5] コミット中..."
git add -A
git commit -m "Initial release: Universal File Converter

Full client-side file converter with zero server load.
- 50+ format support (image, video, audio, document)
- WebCodecs GPU-accelerated video processing
- Worker Pool parallel processing (all CPU cores)
- ImageDecoder API for 1.5x faster image decoding
- Compression Streams for 20x faster ZIP export
- Streaming/chunking for GB+ files
- 9 language i18n with auto-detection
- SEO optimized (JSON-LD, Open Graph, sitemap)
- GitHub Pages deployment ready"

# 5. プッシュ
echo "[5/5] プッシュ中..."
git push origin main

echo ""
echo "=== 移行完了! ==="
echo "https://github.com/moyashianji/universal-converter2"
echo ""

# クリーンアップ
rm -rf "$WORK_DIR"
echo "一時ファイル削除済み"
