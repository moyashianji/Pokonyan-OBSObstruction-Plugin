/**
 * Universal File Converter - Hybrid System
 *
 * Best-in-class conversion using:
 * - Canvas API for images (fastest, native)
 * - FFmpeg.wasm for audio/video (full format support)
 * - Service Worker for SharedArrayBuffer headers
 */

import { convertImage, isImageFile, getSupportedImageFormats } from './image-converter';
import {
	initFFmpeg,
	convertWithFFmpeg,
	isFFmpegAvailable,
	isFFmpegLoaded,
	FFMPEG_VIDEO_FORMATS,
	FFMPEG_AUDIO_FORMATS
} from './ffmpeg-converter';
import { convertAudio, isAudioFile, getSupportedAudioFormats as getNativeAudioFormats } from './audio-converter';
import { convertVideo, isVideoFile, getSupportedVideoFormats as getNativeVideoFormats } from './video-converter';

export type FileType = 'image' | 'audio' | 'video' | 'document' | null;
export type ConversionStatus = 'idle' | 'loading' | 'converting' | 'complete' | 'error';

export interface ConversionState {
	status: ConversionStatus;
	progress: number;
	message: string;
	outputUrl: string | null;
	outputFileName: string | null;
}

export type ProgressCallback = (state: Partial<ConversionState>) => void;

export function detectFileType(file: File): FileType {
	if (isImageFile(file)) return 'image';
	if (isAudioFile(file)) return 'audio';
	if (isVideoFile(file)) return 'video';

	const ext = file.name.split('.').pop()?.toLowerCase() || '';
	if (['pdf', 'doc', 'docx', 'txt', 'rtf', 'odt'].includes(ext)) {
		return 'document';
	}

	return null;
}

export function getOutputFormats(fileType: FileType): string[] {
	const ffmpegEnabled = isFFmpegAvailable();

	switch (fileType) {
		case 'image':
			return getSupportedImageFormats();
		case 'audio':
			return ffmpegEnabled ? FFMPEG_AUDIO_FORMATS : getNativeAudioFormats();
		case 'video':
			if (ffmpegEnabled) {
				// Video can convert to video formats, audio formats, or image formats
				return [...FFMPEG_VIDEO_FORMATS, ...FFMPEG_AUDIO_FORMATS.slice(0, 3), 'png', 'jpg'];
			}
			return [...getNativeVideoFormats(), 'wav', 'png', 'jpg'];
		case 'document':
			return ['txt'];
		default:
			return [];
	}
}

export async function convertFile(
	file: File,
	outputFormat: string,
	onProgress?: ProgressCallback
): Promise<{ url: string; fileName: string }> {
	const fileType = detectFileType(file);

	onProgress?.({
		status: 'converting',
		progress: 0,
		message: '変換を開始...'
	});

	try {
		let result: { blob: Blob; fileName: string };

		switch (fileType) {
			case 'image':
				// Always use Canvas API for images (fastest)
				onProgress?.({ progress: 10, message: '画像を処理中...' });
				result = await convertImage(file, outputFormat as any);
				break;

			case 'audio':
				// Use FFmpeg if available, otherwise native
				if (isFFmpegAvailable()) {
					result = await convertWithFFmpeg(
						file,
						outputFormat,
						(progress, message) => {
							onProgress?.({ progress, message });
						}
					);
				} else {
					result = await convertAudio(
						file,
						outputFormat as any,
						{},
						(progress) => {
							onProgress?.({ progress, message: `音声を変換中... ${progress}%` });
						}
					);
				}
				break;

			case 'video':
				// Use FFmpeg if available for full format support
				if (isFFmpegAvailable()) {
					result = await convertWithFFmpeg(
						file,
						outputFormat,
						(progress, message) => {
							onProgress?.({ progress, message });
						}
					);
				} else {
					// Fallback to native APIs
					const outputType = ['mp3', 'wav', 'ogg'].includes(outputFormat) ? 'audio' :
									   ['png', 'jpg', 'jpeg'].includes(outputFormat) ? 'image' : 'video';
					result = await convertVideo(
						file,
						{ format: outputFormat as any, outputType },
						(progress) => {
							onProgress?.({ progress, message: `動画を変換中... ${progress}%` });
						}
					);
				}
				break;

			case 'document':
				result = await convertDocument(file, outputFormat);
				break;

			default:
				throw new Error('サポートされていないファイル形式です');
		}

		const url = URL.createObjectURL(result.blob);

		onProgress?.({
			status: 'complete',
			progress: 100,
			message: '変換完了!',
			outputUrl: url,
			outputFileName: result.fileName
		});

		return { url, fileName: result.fileName };
	} catch (error) {
		const message = error instanceof Error ? error.message : '変換中にエラーが発生しました';
		onProgress?.({
			status: 'error',
			progress: 0,
			message
		});
		throw error;
	}
}

async function convertDocument(
	file: File,
	outputFormat: string
): Promise<{ blob: Blob; fileName: string }> {
	const baseName = file.name.replace(/\.[^/.]+$/, '');

	if (outputFormat === 'txt') {
		const text = await file.text();
		const blob = new Blob([text], { type: 'text/plain' });
		return { blob, fileName: `${baseName}.txt` };
	}

	throw new Error('このドキュメント形式の変換はサポートされていません');
}

export function formatFileSize(bytes: number): string {
	if (bytes === 0) return '0 B';
	const k = 1024;
	const sizes = ['B', 'KB', 'MB', 'GB'];
	const i = Math.floor(Math.log(bytes) / Math.log(k));
	return parseFloat((bytes / Math.pow(k, i)).toFixed(2)) + ' ' + sizes[i];
}

// Pre-initialize FFmpeg in the background
export async function preloadFFmpeg(): Promise<void> {
	if (isFFmpegAvailable() && !isFFmpegLoaded()) {
		try {
			await initFFmpeg();
		} catch (error) {
			console.warn('FFmpeg preload failed:', error);
		}
	}
}
