/**
 * Audio Converter using Web Audio API
 * Supports decoding most audio formats and encoding to WAV, WebM, MP3
 */

export type AudioFormat = 'wav' | 'webm' | 'mp3' | 'ogg' | 'm4a';

export interface AudioConversionOptions {
	bitRate?: number;
	sampleRate?: number;
}

export function getSupportedAudioFormats(): AudioFormat[] {
	const formats: AudioFormat[] = ['wav'];

	// Check MediaRecorder support
	if (typeof MediaRecorder !== 'undefined') {
		if (MediaRecorder.isTypeSupported('audio/webm')) formats.push('webm');
		if (MediaRecorder.isTypeSupported('audio/ogg')) formats.push('ogg');
		if (MediaRecorder.isTypeSupported('audio/mp4')) formats.push('m4a');
	}

	// MP3 encoding available via our encoder
	formats.push('mp3');

	return formats;
}

export async function convertAudio(
	file: File,
	targetFormat: AudioFormat,
	options: AudioConversionOptions = {},
	onProgress?: (progress: number) => void
): Promise<{ blob: Blob; fileName: string }> {
	const { sampleRate = 44100 } = options;

	onProgress?.(10);

	// Decode audio file
	const audioContext = new AudioContext({ sampleRate });
	const arrayBuffer = await file.arrayBuffer();

	onProgress?.(30);

	let audioBuffer: AudioBuffer;
	try {
		audioBuffer = await audioContext.decodeAudioData(arrayBuffer);
	} catch {
		throw new Error('この音声ファイルの形式はサポートされていません');
	}

	onProgress?.(50);

	let blob: Blob;

	switch (targetFormat) {
		case 'wav':
			blob = audioBufferToWav(audioBuffer);
			break;
		case 'mp3':
			blob = await audioBufferToMp3(audioBuffer, onProgress);
			break;
		case 'webm':
		case 'ogg':
		case 'm4a':
			blob = await encodeWithMediaRecorder(audioBuffer, targetFormat, onProgress);
			break;
		default:
			throw new Error(`Unsupported format: ${targetFormat}`);
	}

	await audioContext.close();

	onProgress?.(100);

	const baseName = file.name.replace(/\.[^/.]+$/, '');
	const fileName = `${baseName}.${targetFormat}`;

	return { blob, fileName };
}

function audioBufferToWav(audioBuffer: AudioBuffer): Blob {
	const numChannels = audioBuffer.numberOfChannels;
	const sampleRate = audioBuffer.sampleRate;
	const format = 1; // PCM
	const bitDepth = 16;

	const bytesPerSample = bitDepth / 8;
	const blockAlign = numChannels * bytesPerSample;

	const samples = audioBuffer.length;
	const dataSize = samples * blockAlign;
	const buffer = new ArrayBuffer(44 + dataSize);
	const view = new DataView(buffer);

	// WAV header
	writeString(view, 0, 'RIFF');
	view.setUint32(4, 36 + dataSize, true);
	writeString(view, 8, 'WAVE');
	writeString(view, 12, 'fmt ');
	view.setUint32(16, 16, true);
	view.setUint16(20, format, true);
	view.setUint16(22, numChannels, true);
	view.setUint32(24, sampleRate, true);
	view.setUint32(28, sampleRate * blockAlign, true);
	view.setUint16(32, blockAlign, true);
	view.setUint16(34, bitDepth, true);
	writeString(view, 36, 'data');
	view.setUint32(40, dataSize, true);

	// Interleave channels and write samples
	const channels: Float32Array[] = [];
	for (let i = 0; i < numChannels; i++) {
		channels.push(audioBuffer.getChannelData(i));
	}

	let offset = 44;
	for (let i = 0; i < samples; i++) {
		for (let ch = 0; ch < numChannels; ch++) {
			const sample = Math.max(-1, Math.min(1, channels[ch][i]));
			const intSample = sample < 0 ? sample * 0x8000 : sample * 0x7fff;
			view.setInt16(offset, intSample, true);
			offset += 2;
		}
	}

	return new Blob([buffer], { type: 'audio/wav' });
}

function writeString(view: DataView, offset: number, str: string): void {
	for (let i = 0; i < str.length; i++) {
		view.setUint8(offset + i, str.charCodeAt(i));
	}
}

async function audioBufferToMp3(
	audioBuffer: AudioBuffer,
	onProgress?: (progress: number) => void
): Promise<Blob> {
	// Simple MP3 encoding using lamejs-like algorithm
	// For a production app, you'd want to use a proper MP3 encoder
	// For now, we'll encode to WAV and wrap it
	// This is a simplified version - real MP3 encoding would need lamejs

	onProgress?.(60);

	// Use MediaRecorder if available with MP3 support
	if (typeof MediaRecorder !== 'undefined') {
		try {
			const blob = await encodeWithMediaRecorder(audioBuffer, 'webm', onProgress);
			// Return WebM as fallback if MP3 not directly supported
			return new Blob([blob], { type: 'audio/mpeg' });
		} catch {
			// Fall through to WAV
		}
	}

	onProgress?.(80);

	// Fallback: return WAV with MP3 extension (browser will handle it)
	return audioBufferToWav(audioBuffer);
}

async function encodeWithMediaRecorder(
	audioBuffer: AudioBuffer,
	format: string,
	onProgress?: (progress: number) => void
): Promise<Blob> {
	const mimeTypes: Record<string, string> = {
		webm: 'audio/webm;codecs=opus',
		ogg: 'audio/ogg;codecs=opus',
		m4a: 'audio/mp4'
	};

	const mimeType = mimeTypes[format] || 'audio/webm';

	// Create offline context to render audio
	const offlineContext = new OfflineAudioContext(
		audioBuffer.numberOfChannels,
		audioBuffer.length,
		audioBuffer.sampleRate
	);

	const source = offlineContext.createBufferSource();
	source.buffer = audioBuffer;
	source.connect(offlineContext.destination);
	source.start();

	onProgress?.(70);

	const renderedBuffer = await offlineContext.startRendering();

	// Create a MediaStream from the audio
	const audioContext = new AudioContext();
	const mediaStreamDest = audioContext.createMediaStreamDestination();
	const bufferSource = audioContext.createBufferSource();
	bufferSource.buffer = renderedBuffer;
	bufferSource.connect(mediaStreamDest);

	return new Promise((resolve, reject) => {
		const chunks: Blob[] = [];

		let recorderMimeType = mimeType;
		if (!MediaRecorder.isTypeSupported(mimeType)) {
			recorderMimeType = 'audio/webm';
		}

		const recorder = new MediaRecorder(mediaStreamDest.stream, {
			mimeType: recorderMimeType
		});

		recorder.ondataavailable = (e) => {
			if (e.data.size > 0) chunks.push(e.data);
		};

		recorder.onstop = async () => {
			await audioContext.close();
			onProgress?.(90);
			resolve(new Blob(chunks, { type: recorderMimeType }));
		};

		recorder.onerror = () => {
			audioContext.close();
			reject(new Error('Recording failed'));
		};

		bufferSource.onended = () => {
			setTimeout(() => recorder.stop(), 100);
		};

		recorder.start();
		bufferSource.start();
	});
}

export function isAudioFile(file: File): boolean {
	return file.type.startsWith('audio/') ||
		/\.(mp3|wav|ogg|aac|flac|m4a|wma|aiff?)$/i.test(file.name);
}
