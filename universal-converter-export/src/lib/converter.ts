import { FFmpeg } from '@ffmpeg/ffmpeg';
import { fetchFile, toBlobURL } from '@ffmpeg/util';

let ffmpeg: FFmpeg | null = null;
let loaded = false;

export type ConversionStatus = 'idle' | 'loading' | 'converting' | 'complete' | 'error';

export interface ConversionState {
	status: ConversionStatus;
	progress: number;
	message: string;
	outputUrl: string | null;
	outputFileName: string | null;
}

export type ProgressCallback = (state: Partial<ConversionState>) => void;

export function detectFileType(file: File): 'video' | 'audio' | 'image' | 'document' | null {
	const mimeType = file.type.toLowerCase();
	const extension = file.name.split('.').pop()?.toLowerCase() || '';

	// Video
	if (mimeType.startsWith('video/') || ['mp4', 'webm', 'avi', 'mov', 'mkv', 'flv', 'wmv'].includes(extension)) {
		return 'video';
	}

	// Audio
	if (mimeType.startsWith('audio/') || ['mp3', 'wav', 'ogg', 'aac', 'flac', 'm4a', 'wma'].includes(extension)) {
		return 'audio';
	}

	// Image
	if (mimeType.startsWith('image/') || ['png', 'jpg', 'jpeg', 'webp', 'gif', 'bmp', 'tiff', 'svg'].includes(extension)) {
		return 'image';
	}

	// Document
	if (['pdf', 'doc', 'docx', 'txt', 'rtf', 'odt'].includes(extension)) {
		return 'document';
	}

	return null;
}

export async function loadFFmpeg(onProgress?: ProgressCallback): Promise<void> {
	if (loaded && ffmpeg) return;

	onProgress?.({ status: 'loading', message: 'FFmpegをダウンロード中...', progress: 0 });

	ffmpeg = new FFmpeg();

	ffmpeg.on('log', ({ message }) => {
		console.log('[FFmpeg]', message);
	});

	ffmpeg.on('progress', ({ progress, time }) => {
		const percent = Math.min(Math.round(progress * 100), 100);
		onProgress?.({ progress: percent, message: `変換中... ${percent}%` });
	});

	const baseURL = 'https://unpkg.com/@ffmpeg/core@0.12.6/dist/esm';

	try {
		await ffmpeg.load({
			coreURL: await toBlobURL(`${baseURL}/ffmpeg-core.js`, 'text/javascript'),
			wasmURL: await toBlobURL(`${baseURL}/ffmpeg-core.wasm`, 'application/wasm'),
		});
		loaded = true;
		onProgress?.({ status: 'idle', message: 'FFmpeg準備完了', progress: 100 });
	} catch (error) {
		console.error('FFmpeg load error:', error);
		onProgress?.({ status: 'error', message: 'FFmpegの読み込みに失敗しました' });
		throw error;
	}
}

export async function convertFile(
	file: File,
	outputFormat: string,
	onProgress?: ProgressCallback
): Promise<{ url: string; fileName: string }> {
	if (!ffmpeg || !loaded) {
		await loadFFmpeg(onProgress);
	}

	onProgress?.({ status: 'converting', progress: 0, message: 'ファイルを読み込み中...' });

	const inputFileName = `input_${Date.now()}.${file.name.split('.').pop()}`;
	const outputFileName = `output_${Date.now()}.${outputFormat}`;
	const baseName = file.name.replace(/\.[^/.]+$/, '');
	const downloadFileName = `${baseName}.${outputFormat}`;

	try {
		// Write input file to FFmpeg virtual filesystem
		await ffmpeg!.writeFile(inputFileName, await fetchFile(file));

		onProgress?.({ progress: 10, message: '変換を開始...' });

		// Build FFmpeg command based on output format
		const args = buildFFmpegArgs(inputFileName, outputFileName, outputFormat, file);

		// Execute conversion
		await ffmpeg!.exec(args);

		onProgress?.({ progress: 90, message: '出力ファイルを準備中...' });

		// Read output file
		const data = await ffmpeg!.readFile(outputFileName);

		// Create blob URL
		const mimeType = getMimeType(outputFormat);
		const blob = new Blob([data], { type: mimeType });
		const url = URL.createObjectURL(blob);

		// Cleanup
		await ffmpeg!.deleteFile(inputFileName);
		await ffmpeg!.deleteFile(outputFileName);

		onProgress?.({
			status: 'complete',
			progress: 100,
			message: '変換完了!',
			outputUrl: url,
			outputFileName: downloadFileName
		});

		return { url, fileName: downloadFileName };
	} catch (error) {
		console.error('Conversion error:', error);
		onProgress?.({
			status: 'error',
			message: `変換エラー: ${error instanceof Error ? error.message : '不明なエラー'}`
		});
		throw error;
	}
}

function buildFFmpegArgs(
	input: string,
	output: string,
	format: string,
	file: File
): string[] {
	const fileType = detectFileType(file);
	const baseArgs = ['-i', input];

	switch (format) {
		// Video formats
		case 'mp4':
			return [...baseArgs, '-c:v', 'libx264', '-preset', 'fast', '-crf', '23', '-c:a', 'aac', '-b:a', '128k', output];
		case 'webm':
			return [...baseArgs, '-c:v', 'libvpx-vp9', '-crf', '30', '-b:v', '0', '-c:a', 'libopus', output];
		case 'avi':
			return [...baseArgs, '-c:v', 'mpeg4', '-c:a', 'mp3', output];
		case 'mov':
			return [...baseArgs, '-c:v', 'libx264', '-c:a', 'aac', output];
		case 'mkv':
			return [...baseArgs, '-c:v', 'libx264', '-c:a', 'aac', output];
		case 'gif':
			return [...baseArgs, '-vf', 'fps=15,scale=480:-1:flags=lanczos', '-loop', '0', output];

		// Audio formats
		case 'mp3':
			return [...baseArgs, '-vn', '-c:a', 'libmp3lame', '-q:a', '2', output];
		case 'wav':
			return [...baseArgs, '-vn', '-c:a', 'pcm_s16le', output];
		case 'ogg':
			return [...baseArgs, '-vn', '-c:a', 'libvorbis', '-q:a', '4', output];
		case 'aac':
			return [...baseArgs, '-vn', '-c:a', 'aac', '-b:a', '192k', output];
		case 'flac':
			return [...baseArgs, '-vn', '-c:a', 'flac', output];
		case 'm4a':
			return [...baseArgs, '-vn', '-c:a', 'aac', '-b:a', '256k', output];

		// Image formats
		case 'png':
			return fileType === 'video'
				? [...baseArgs, '-ss', '00:00:01', '-vframes', '1', output]
				: [...baseArgs, output];
		case 'jpg':
		case 'jpeg':
			return fileType === 'video'
				? [...baseArgs, '-ss', '00:00:01', '-vframes', '1', '-q:v', '2', output]
				: [...baseArgs, '-q:v', '2', output];
		case 'webp':
			return fileType === 'video'
				? [...baseArgs, '-ss', '00:00:01', '-vframes', '1', '-quality', '80', output]
				: [...baseArgs, '-quality', '80', output];
		case 'bmp':
			return fileType === 'video'
				? [...baseArgs, '-ss', '00:00:01', '-vframes', '1', output]
				: [...baseArgs, output];

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
		// Audio
		mp3: 'audio/mpeg',
		wav: 'audio/wav',
		ogg: 'audio/ogg',
		aac: 'audio/aac',
		flac: 'audio/flac',
		m4a: 'audio/mp4',
		// Image
		png: 'image/png',
		jpg: 'image/jpeg',
		jpeg: 'image/jpeg',
		webp: 'image/webp',
		bmp: 'image/bmp',
	};
	return mimeTypes[format] || 'application/octet-stream';
}

export function formatFileSize(bytes: number): string {
	if (bytes === 0) return '0 B';
	const k = 1024;
	const sizes = ['B', 'KB', 'MB', 'GB'];
	const i = Math.floor(Math.log(bytes) / Math.log(k));
	return parseFloat((bytes / Math.pow(k, i)).toFixed(2)) + ' ' + sizes[i];
}
