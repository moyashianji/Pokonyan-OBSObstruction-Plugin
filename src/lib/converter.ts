/**
 * Universal File Converter - Hybrid System
 *
 * Best-in-class conversion using:
 * - Canvas API for images (fastest, native)
 * - FFmpeg.wasm for audio/video (comprehensive format support)
 * - Service Worker for SharedArrayBuffer headers
 */

import { convertImage, isImageFile, getSupportedImageFormats } from './image-converter';
import {
	initFFmpeg,
	convertWithFFmpeg,
	isFFmpegAvailable,
	isFFmpegLoaded,
	FFMPEG_VIDEO_FORMATS,
	FFMPEG_AUDIO_FORMATS,
	FFMPEG_IMAGE_OUTPUTS
} from './ffmpeg-converter';
import { convertAudio, isAudioFile, getSupportedAudioFormats as getNativeAudioFormats } from './audio-converter';
import { convertVideo, isVideoFile, getSupportedVideoFormats as getNativeVideoFormats } from './video-converter';
import { COMMON_VIDEO_OUTPUTS, COMMON_AUDIO_OUTPUTS, COMMON_IMAGE_OUTPUTS } from './formats';

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
			// Images use Canvas API for common formats, FFmpeg for others
			if (ffmpegEnabled) {
				return [...COMMON_IMAGE_OUTPUTS];
			}
			return getSupportedImageFormats();

		case 'audio':
			if (ffmpegEnabled) {
				return COMMON_AUDIO_OUTPUTS;
			}
			return getNativeAudioFormats();

		case 'video':
			if (ffmpegEnabled) {
				// Video can output to video, audio, or image formats
				return [
					...COMMON_VIDEO_OUTPUTS,
					'mp3', 'wav', 'aac', 'ogg', 'flac',  // Audio extraction
					'png', 'jpg', 'webp'  // Frame extraction
				];
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

		// Determine if we should use FFmpeg
		const useFFmpeg = isFFmpegAvailable() && (
			fileType === 'audio' ||
			fileType === 'video' ||
			(fileType === 'image' && !getSupportedImageFormats().includes(outputFormat as any))
		);

		if (fileType === 'image' && getSupportedImageFormats().includes(outputFormat as any)) {
			// Use fast Canvas API for common image formats
			onProgress?.({ progress: 10, message: '画像を処理中...' });
			result = await convertImage(file, outputFormat as any);
		} else if (useFFmpeg) {
			// Use FFmpeg for audio, video, and advanced image formats
			result = await convertWithFFmpeg(
				file,
				outputFormat,
				(progress, message) => {
					onProgress?.({ progress, message });
				}
			);
		} else if (fileType === 'audio') {
			// Fallback to native audio API
			result = await convertAudio(
				file,
				outputFormat as any,
				{},
				(progress) => {
					onProgress?.({ progress, message: `音声を変換中... ${progress}%` });
				}
			);
		} else if (fileType === 'video') {
			// Fallback to native video API
			const outputType = ['mp3', 'wav', 'ogg'].includes(outputFormat) ? 'audio' :
							   ['png', 'jpg', 'jpeg'].includes(outputFormat) ? 'image' : 'video';
			result = await convertVideo(
				file,
				{ format: outputFormat as any, outputType },
				(progress) => {
					onProgress?.({ progress, message: `動画を変換中... ${progress}%` });
				}
			);
		} else if (fileType === 'document') {
			result = await convertDocument(file, outputFormat);
		} else {
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

export async function preloadFFmpeg(): Promise<void> {
	if (isFFmpegAvailable() && !isFFmpegLoaded()) {
		try {
			await initFFmpeg();
		} catch (error) {
			console.warn('FFmpeg preload failed:', error);
		}
	}
}
