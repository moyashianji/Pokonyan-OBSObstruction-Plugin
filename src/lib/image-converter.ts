/**
 * Native Image Converter using Canvas API
 * Supports: PNG, JPEG, WebP, BMP, GIF
 * Zero dependencies, ultra-fast, works everywhere
 */

export type ImageFormat = 'png' | 'jpeg' | 'jpg' | 'webp' | 'bmp' | 'gif';

export interface ImageConversionOptions {
	quality?: number; // 0-1 for lossy formats
	width?: number;
	height?: number;
	maintainAspectRatio?: boolean;
}

const MIME_TYPES: Record<ImageFormat, string> = {
	png: 'image/png',
	jpeg: 'image/jpeg',
	jpg: 'image/jpeg',
	webp: 'image/webp',
	bmp: 'image/bmp',
	gif: 'image/gif'
};

export function getSupportedImageFormats(): ImageFormat[] {
	return ['png', 'jpeg', 'webp', 'gif', 'bmp'];
}

export async function convertImage(
	file: File,
	targetFormat: ImageFormat,
	options: ImageConversionOptions = {}
): Promise<{ blob: Blob; fileName: string }> {
	const { quality = 0.92, width, height, maintainAspectRatio = true } = options;

	// Load image
	const img = await loadImage(file);

	// Calculate dimensions
	let targetWidth = width || img.naturalWidth;
	let targetHeight = height || img.naturalHeight;

	if (width && !height && maintainAspectRatio) {
		targetHeight = Math.round((img.naturalHeight / img.naturalWidth) * width);
	} else if (height && !width && maintainAspectRatio) {
		targetWidth = Math.round((img.naturalWidth / img.naturalHeight) * height);
	}

	// Create canvas
	const canvas = document.createElement('canvas');
	canvas.width = targetWidth;
	canvas.height = targetHeight;

	const ctx = canvas.getContext('2d');
	if (!ctx) {
		throw new Error('Canvas context not available');
	}

	// Enable high-quality rendering
	ctx.imageSmoothingEnabled = true;
	ctx.imageSmoothingQuality = 'high';

	// Draw image
	ctx.drawImage(img, 0, 0, targetWidth, targetHeight);

	// Convert to target format
	const mimeType = MIME_TYPES[targetFormat];
	const blob = await canvasToBlob(canvas, mimeType, quality);

	// Generate filename
	const baseName = file.name.replace(/\.[^/.]+$/, '');
	const fileName = `${baseName}.${targetFormat === 'jpg' ? 'jpg' : targetFormat}`;

	return { blob, fileName };
}

function loadImage(file: File): Promise<HTMLImageElement> {
	return new Promise((resolve, reject) => {
		const img = new Image();
		img.onload = () => {
			URL.revokeObjectURL(img.src);
			resolve(img);
		};
		img.onerror = () => {
			URL.revokeObjectURL(img.src);
			reject(new Error('Failed to load image'));
		};
		img.src = URL.createObjectURL(file);
	});
}

function canvasToBlob(canvas: HTMLCanvasElement, mimeType: string, quality: number): Promise<Blob> {
	return new Promise((resolve, reject) => {
		canvas.toBlob(
			(blob) => {
				if (blob) {
					resolve(blob);
				} else {
					reject(new Error('Failed to convert canvas to blob'));
				}
			},
			mimeType,
			quality
		);
	});
}

export function isImageFile(file: File): boolean {
	return file.type.startsWith('image/') ||
		/\.(png|jpe?g|webp|gif|bmp|tiff?|svg)$/i.test(file.name);
}
