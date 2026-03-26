/**
 * FFmpeg.wasm based converter for audio and video
 * Provides full format support with SharedArrayBuffer via Service Worker
 */

import { FFmpeg } from '@ffmpeg/ffmpeg';
import { fetchFile, toBlobURL } from '@ffmpeg/util';

let ffmpeg: FFmpeg | null = null;
let loaded = false;
let loadPromise: Promise<void> | null = null;

export type ProgressCallback = (progress: number, message: string) => void;

export async function initFFmpeg(onProgress?: ProgressCallback): Promise<boolean> {
	// Check if SharedArrayBuffer is available
	if (typeof SharedArrayBuffer === 'undefined') {
		console.warn('SharedArrayBuffer not available. FFmpeg features limited.');
		return false;
	}

	if (loaded && ffmpeg) return true;
	if (loadPromise) {
		await loadPromise;
		return loaded;
	}

	loadPromise = (async () => {
		onProgress?.(0, 'FFmpegを初期化中...');

		ffmpeg = new FFmpeg();

		ffmpeg.on('log', ({ message }) => {
			console.log('[FFmpeg]', message);
		});

		ffmpeg.on('progress', ({ progress }) => {
			const percent = Math.min(Math.round(progress * 100), 100);
			onProgress?.(percent, `処理中... ${percent}%`);
		});

		const baseURL = 'https://unpkg.com/@ffmpeg/core@0.12.6/dist/esm';

		try {
			onProgress?.(10, 'FFmpegコアをダウンロード中...');

			await ffmpeg.load({
				coreURL: await toBlobURL(`${baseURL}/ffmpeg-core.js`, 'text/javascript'),
				wasmURL: await toBlobURL(`${baseURL}/ffmpeg-core.wasm`, 'application/wasm'),
			});

			loaded = true;
			onProgress?.(100, 'FFmpeg準備完了');
		} catch (error) {
			console.error('FFmpeg load error:', error);
			loaded = false;
			throw error;
		}
	})();

	try {
		await loadPromise;
		return true;
	} catch {
		return false;
	}
}

export function isFFmpegAvailable(): boolean {
	return typeof SharedArrayBuffer !== 'undefined';
}

export function isFFmpegLoaded(): boolean {
	return loaded;
}

export async function convertWithFFmpeg(
	file: File,
	outputFormat: string,
	onProgress?: ProgressCallback
): Promise<{ blob: Blob; fileName: string }> {
	if (!loaded || !ffmpeg) {
		const success = await initFFmpeg(onProgress);
		if (!success) {
			throw new Error('FFmpegの初期化に失敗しました');
		}
	}

	const inputExt = file.name.split('.').pop() || 'dat';
	const inputFileName = `input_${Date.now()}.${inputExt}`;
	const outputFileName = `output_${Date.now()}.${outputFormat}`;
	const baseName = file.name.replace(/\.[^/.]+$/, '');

	onProgress?.(5, 'ファイルを読み込み中...');

	// Write input file
	await ffmpeg!.writeFile(inputFileName, await fetchFile(file));

	onProgress?.(15, '変換を開始...');

	// Build FFmpeg command
	const args = buildFFmpegArgs(inputFileName, outputFileName, outputFormat, file.type);

	// Execute
	await ffmpeg!.exec(args);

	onProgress?.(90, '出力ファイルを準備中...');

	// Read output
	const data = await ffmpeg!.readFile(outputFileName);

	// Create blob
	const mimeType = getMimeType(outputFormat);
	const blob = new Blob([data], { type: mimeType });

	// Cleanup
	try {
		await ffmpeg!.deleteFile(inputFileName);
		await ffmpeg!.deleteFile(outputFileName);
	} catch {
		// Ignore cleanup errors
	}

	onProgress?.(100, '変換完了!');

	return {
		blob,
		fileName: `${baseName}.${outputFormat}`
	};
}

function buildFFmpegArgs(input: string, output: string, format: string, mimeType: string): string[] {
	const baseArgs = ['-i', input];
	const isVideo = mimeType.startsWith('video/');
	const isAudio = mimeType.startsWith('audio/');

	switch (format) {
		// Video formats
		case 'mp4':
			return [...baseArgs, '-c:v', 'libx264', '-preset', 'fast', '-crf', '23', '-c:a', 'aac', '-b:a', '128k', '-movflags', '+faststart', output];
		case 'webm':
			return [...baseArgs, '-c:v', 'libvpx-vp9', '-crf', '30', '-b:v', '0', '-c:a', 'libopus', '-b:a', '128k', output];
		case 'avi':
			return [...baseArgs, '-c:v', 'mpeg4', '-q:v', '5', '-c:a', 'mp3', '-b:a', '192k', output];
		case 'mov':
			return [...baseArgs, '-c:v', 'libx264', '-preset', 'fast', '-c:a', 'aac', '-b:a', '192k', output];
		case 'mkv':
			return [...baseArgs, '-c:v', 'libx264', '-preset', 'fast', '-c:a', 'aac', '-b:a', '192k', output];
		case 'gif':
			return [...baseArgs, '-vf', 'fps=15,scale=480:-1:flags=lanczos,split[s0][s1];[s0]palettegen[p];[s1][p]paletteuse', '-loop', '0', output];
		case 'flv':
			return [...baseArgs, '-c:v', 'libx264', '-c:a', 'aac', output];

		// Audio formats
		case 'mp3':
			return [...baseArgs, '-vn', '-c:a', 'libmp3lame', '-q:a', '2', output];
		case 'wav':
			return [...baseArgs, '-vn', '-c:a', 'pcm_s16le', output];
		case 'ogg':
			return [...baseArgs, '-vn', '-c:a', 'libvorbis', '-q:a', '5', output];
		case 'aac':
			return [...baseArgs, '-vn', '-c:a', 'aac', '-b:a', '192k', output];
		case 'flac':
			return [...baseArgs, '-vn', '-c:a', 'flac', output];
		case 'm4a':
			return [...baseArgs, '-vn', '-c:a', 'aac', '-b:a', '256k', output];
		case 'opus':
			return [...baseArgs, '-vn', '-c:a', 'libopus', '-b:a', '128k', output];

		// Image formats (extract frame from video)
		case 'png':
			return isVideo
				? [...baseArgs, '-ss', '00:00:01', '-vframes', '1', '-f', 'image2', output]
				: [...baseArgs, output];
		case 'jpg':
		case 'jpeg':
			return isVideo
				? [...baseArgs, '-ss', '00:00:01', '-vframes', '1', '-q:v', '2', '-f', 'image2', output]
				: [...baseArgs, '-q:v', '2', output];
		case 'webp':
			return isVideo
				? [...baseArgs, '-ss', '00:00:01', '-vframes', '1', '-quality', '80', output]
				: [...baseArgs, '-quality', '80', output];

		default:
			return [...baseArgs, output];
	}
}

function getMimeType(format: string): string {
	const mimeTypes: Record<string, string> = {
		// Video
		mp4: 'video/mp4',
		webm: 'video/webm',
		avi: 'video/x-msvideo',
		mov: 'video/quicktime',
		mkv: 'video/x-matroska',
		gif: 'image/gif',
		flv: 'video/x-flv',
		// Audio
		mp3: 'audio/mpeg',
		wav: 'audio/wav',
		ogg: 'audio/ogg',
		aac: 'audio/aac',
		flac: 'audio/flac',
		m4a: 'audio/mp4',
		opus: 'audio/opus',
		// Image
		png: 'image/png',
		jpg: 'image/jpeg',
		jpeg: 'image/jpeg',
		webp: 'image/webp',
	};
	return mimeTypes[format] || 'application/octet-stream';
}

// Supported formats when FFmpeg is available
export const FFMPEG_VIDEO_FORMATS = ['mp4', 'webm', 'avi', 'mov', 'mkv', 'gif', 'flv'];
export const FFMPEG_AUDIO_FORMATS = ['mp3', 'wav', 'ogg', 'aac', 'flac', 'm4a', 'opus'];
export const FFMPEG_IMAGE_OUTPUTS = ['png', 'jpg', 'webp']; // For extracting frames from video
