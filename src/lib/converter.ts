/**
 * Universal File Converter
 * Uses native browser APIs for maximum compatibility and speed
 * No external dependencies that require special headers
 */

import { convertImage, isImageFile, getSupportedImageFormats, type ImageFormat } from './image-converter';
import { convertAudio, isAudioFile, getSupportedAudioFormats, type AudioFormat } from './audio-converter';
import { convertVideo, isVideoFile, getSupportedVideoFormats, type VideoFormat } from './video-converter';

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
	switch (fileType) {
		case 'image':
			return getSupportedImageFormats();
		case 'audio':
			return getSupportedAudioFormats();
		case 'video':
			return [...getSupportedVideoFormats(), 'mp3', 'wav', 'png', 'jpg'];
		case 'document':
			return ['txt', 'pdf'];
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
				onProgress?.({ progress: 10, message: '画像を処理中...' });
				result = await convertImage(file, outputFormat as ImageFormat);
				break;

			case 'audio':
				result = await convertAudio(
					file,
					outputFormat as AudioFormat,
					{},
					(progress) => {
						onProgress?.({ progress, message: `音声を変換中... ${progress}%` });
					}
				);
				break;

			case 'video':
				// Determine output type based on format
				let outputType: 'video' | 'audio' | 'image' = 'video';
				if (['mp3', 'wav', 'ogg', 'webm-audio'].includes(outputFormat)) {
					outputType = 'audio';
				} else if (['png', 'jpg', 'jpeg'].includes(outputFormat)) {
					outputType = 'image';
				}

				result = await convertVideo(
					file,
					{
						format: outputFormat as VideoFormat,
						outputType
					},
					(progress) => {
						onProgress?.({ progress, message: `動画を変換中... ${progress}%` });
					}
				);
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
		// Convert to plain text
		const text = await file.text();
		const blob = new Blob([text], { type: 'text/plain' });
		return { blob, fileName: `${baseName}.txt` };
	}

	// For other document formats, we can't convert without a server
	throw new Error('このドキュメント形式の変換はサポートされていません');
}

export function formatFileSize(bytes: number): string {
	if (bytes === 0) return '0 B';
	const k = 1024;
	const sizes = ['B', 'KB', 'MB', 'GB'];
	const i = Math.floor(Math.log(bytes) / Math.log(k));
	return parseFloat((bytes / Math.pow(k, i)).toFixed(2)) + ' ' + sizes[i];
}

// Export for backward compatibility
export async function loadFFmpeg(): Promise<void> {
	// No longer needed - using native APIs
	return Promise.resolve();
}
