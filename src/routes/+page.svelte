<script lang="ts">
	import { onMount } from 'svelte';
	import FileDropzone from '$lib/components/FileDropzone.svelte';
	import FormatSelector from '$lib/components/FormatSelector.svelte';
	import ConversionProgress from '$lib/components/ConversionProgress.svelte';
	import {
		convertFile,
		detectFileType,
		formatFileSize,
		preloadFFmpeg,
		type ConversionState
	} from '$lib/converter';

	let files = $state<File[]>([]);
	let selectedFormat = $state('');
	let fileType = $state<'video' | 'audio' | 'image' | 'document' | null>(null);
	let ffmpegStatus = $state<'loading' | 'ready' | 'unavailable' | 'idle'>('idle');

	let conversionState = $state<ConversionState>({
		status: 'idle',
		progress: 0,
		message: '',
		outputUrl: null,
		outputFileName: null
	});

	let isConverting = $derived(conversionState.status === 'converting' || conversionState.status === 'loading');

	onMount(() => {
		// Preload FFmpeg in the background after page loads
		if (typeof SharedArrayBuffer !== 'undefined') {
			ffmpegStatus = 'loading';
			preloadFFmpeg()
				.then(() => { ffmpegStatus = 'ready'; })
				.catch(() => { ffmpegStatus = 'unavailable'; });
		} else {
			ffmpegStatus = 'unavailable';
		}
	});

	function handleFiles(event: CustomEvent<File[]>) {
		files = event.detail;
		if (files.length > 0) {
			fileType = detectFileType(files[0]);
			selectedFormat = '';
			conversionState = {
				status: 'idle',
				progress: 0,
				message: '',
				outputUrl: null,
				outputFileName: null
			};
		}
	}

	function handleFormatSelect(event: CustomEvent<string>) {
		selectedFormat = event.detail;
	}

	async function startConversion() {
		if (files.length === 0 || !selectedFormat) return;

		try {
			await convertFile(files[0], selectedFormat, (state) => {
				conversionState = { ...conversionState, ...state };
			});
		} catch (error) {
			console.error('Conversion failed:', error);
		}
	}

	function downloadFile() {
		if (!conversionState.outputUrl || !conversionState.outputFileName) return;

		const a = document.createElement('a');
		a.href = conversionState.outputUrl;
		a.download = conversionState.outputFileName;
		a.click();
	}

	function reset() {
		files = [];
		selectedFormat = '';
		fileType = null;
		if (conversionState.outputUrl) {
			URL.revokeObjectURL(conversionState.outputUrl);
		}
		conversionState = {
			status: 'idle',
			progress: 0,
			message: '',
			outputUrl: null,
			outputFileName: null
		};
	}
</script>

<svelte:head>
	<title>Universal Converter - 高速ファイル変換</title>
	<meta name="description" content="ブラウザで完結する高速ファイル変換ツール。動画、音声、画像を瞬時に変換。" />
</svelte:head>

<main class="container">
	<header class="header">
		<h1 class="title">
			<span class="gradient-text">Universal</span> Converter
		</h1>
		<p class="subtitle">
			ブラウザで完結する超高速ファイル変換<br />
			<span class="highlight">サーバー不要・完全プライベート</span>
		</p>
	</header>

	<div class="converter-card">
		{#if files.length === 0}
			<FileDropzone on:files={handleFiles} disabled={isConverting} />
		{:else}
			<div class="file-info">
				<div class="file-preview">
					<svg class="file-icon" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
						<path d="M14 2H6a2 2 0 0 0-2 2v16a2 2 0 0 0 2 2h12a2 2 0 0 0 2-2V8z" />
						<polyline points="14 2 14 8 20 8" />
					</svg>
					<div class="file-details">
						<p class="file-name">{files[0].name}</p>
						<p class="file-size">{formatFileSize(files[0].size)}</p>
					</div>
					<button class="remove-btn" onclick={reset} aria-label="ファイルを削除">
						<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
							<line x1="18" y1="6" x2="6" y2="18" />
							<line x1="6" y1="6" x2="18" y2="18" />
						</svg>
					</button>
				</div>
			</div>

			{#if conversionState.status !== 'complete'}
				<div class="format-section">
					<FormatSelector
						{selectedFormat}
						{fileType}
						on:select={handleFormatSelect}
					/>
				</div>
			{/if}

			{#if conversionState.status !== 'idle' && conversionState.status !== 'complete'}
				<div class="progress-section">
					<ConversionProgress
						progress={conversionState.progress}
						status={conversionState.status}
						message={conversionState.message}
					/>
				</div>
			{/if}

			{#if conversionState.status === 'complete'}
				<div class="success-section">
					<div class="success-icon">
						<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
							<polyline points="20 6 9 17 4 12" />
						</svg>
					</div>
					<p class="success-text">変換が完了しました!</p>
					<div class="action-buttons">
						<button class="btn btn-primary" onclick={downloadFile}>
							<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
								<path d="M21 15v4a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2v-4" />
								<polyline points="7 10 12 15 17 10" />
								<line x1="12" y1="15" x2="12" y2="3" />
							</svg>
							ダウンロード
						</button>
						<button class="btn btn-secondary" onclick={reset}>
							別のファイルを変換
						</button>
					</div>
				</div>
			{:else}
				<div class="convert-section">
					<button
						class="btn btn-convert"
						disabled={!selectedFormat || isConverting}
						onclick={startConversion}
					>
						{#if isConverting}
							<span class="spinner"></span>
							変換中...
						{:else}
							変換を開始
						{/if}
					</button>
				</div>
			{/if}
		{/if}
	</div>

	<section class="features">
		<div class="feature">
			<div class="feature-icon">
				<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
					<circle cx="12" cy="12" r="10" />
					<polyline points="12 6 12 12 16 14" />
				</svg>
			</div>
			<h3>超高速変換</h3>
			<p>ブラウザのネイティブAPIで、サーバーを経由せず即座に変換</p>
		</div>
		<div class="feature">
			<div class="feature-icon">
				<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
					<rect x="3" y="11" width="18" height="11" rx="2" ry="2" />
					<path d="M7 11V7a5 5 0 0 1 10 0v4" />
				</svg>
			</div>
			<h3>完全プライベート</h3>
			<p>ファイルはブラウザ内で処理。サーバーにアップロードされません</p>
		</div>
		<div class="feature">
			<div class="feature-icon">
				<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
					<path d="M12 2L2 7l10 5 10-5-10-5z" />
					<path d="M2 17l10 5 10-5" />
					<path d="M2 12l10 5 10-5" />
				</svg>
			</div>
			<h3>多形式対応</h3>
			<p>動画・音声・画像など、幅広いフォーマットに対応</p>
		</div>
	</section>
</main>

<style>
	.container {
		max-width: 900px;
		margin: 0 auto;
		padding: 2rem 1rem;
	}

	.header {
		text-align: center;
		margin-bottom: 3rem;
	}

	.title {
		font-size: 2.5rem;
		font-weight: 700;
		margin-bottom: 1rem;
	}

	.gradient-text {
		background: linear-gradient(135deg, var(--color-primary), var(--color-secondary));
		-webkit-background-clip: text;
		-webkit-text-fill-color: transparent;
		background-clip: text;
	}

	.subtitle {
		font-size: 1.1rem;
		color: var(--color-text-secondary);
		line-height: 1.6;
	}

	.subtitle .highlight {
		color: var(--color-accent);
		font-weight: 500;
	}

	.converter-card {
		background: var(--color-bg-secondary);
		border-radius: 1rem;
		padding: 2rem;
		border: 1px solid var(--color-border);
		margin-bottom: 3rem;
	}

	.file-info {
		margin-bottom: 1.5rem;
	}

	.file-preview {
		display: flex;
		align-items: center;
		gap: 1rem;
		padding: 1rem;
		background: var(--color-bg-tertiary);
		border-radius: 0.5rem;
	}

	.file-icon {
		width: 2.5rem;
		height: 2.5rem;
		color: var(--color-primary);
		flex-shrink: 0;
	}

	.file-details {
		flex: 1;
		min-width: 0;
	}

	.file-name {
		font-weight: 500;
		color: var(--color-text);
		overflow: hidden;
		text-overflow: ellipsis;
		white-space: nowrap;
	}

	.file-size {
		font-size: 0.875rem;
		color: var(--color-text-secondary);
	}

	.remove-btn {
		width: 2rem;
		height: 2rem;
		padding: 0.25rem;
		border: none;
		background: transparent;
		color: var(--color-text-secondary);
		cursor: pointer;
		border-radius: 0.25rem;
		transition: all 0.2s ease;
	}

	.remove-btn:hover {
		color: var(--color-error);
		background: rgba(239, 68, 68, 0.1);
	}

	.remove-btn svg {
		width: 100%;
		height: 100%;
	}

	.format-section {
		margin-bottom: 1.5rem;
		padding-top: 1rem;
		border-top: 1px solid var(--color-border);
	}

	.progress-section {
		margin-bottom: 1.5rem;
	}

	.convert-section {
		display: flex;
		justify-content: center;
	}

	.btn {
		display: inline-flex;
		align-items: center;
		gap: 0.5rem;
		padding: 0.75rem 1.5rem;
		border-radius: 0.5rem;
		font-size: 1rem;
		font-weight: 500;
		cursor: pointer;
		transition: all 0.2s ease;
		border: none;
	}

	.btn svg {
		width: 1.25rem;
		height: 1.25rem;
	}

	.btn-convert {
		background: linear-gradient(135deg, var(--color-primary), var(--color-secondary));
		color: white;
		padding: 1rem 2rem;
		font-size: 1.1rem;
	}

	.btn-convert:hover:not(:disabled) {
		transform: translateY(-2px);
		box-shadow: 0 8px 20px rgba(99, 102, 241, 0.3);
	}

	.btn-convert:disabled {
		opacity: 0.5;
		cursor: not-allowed;
	}

	.btn-primary {
		background: var(--color-success);
		color: white;
	}

	.btn-primary:hover {
		background: #059669;
	}

	.btn-secondary {
		background: var(--color-bg-tertiary);
		color: var(--color-text);
		border: 1px solid var(--color-border);
	}

	.btn-secondary:hover {
		border-color: var(--color-primary);
	}

	.spinner {
		width: 1.25rem;
		height: 1.25rem;
		border: 2px solid rgba(255, 255, 255, 0.3);
		border-top-color: white;
		border-radius: 50%;
		animation: spin 0.8s linear infinite;
	}

	@keyframes spin {
		to { transform: rotate(360deg); }
	}

	.success-section {
		text-align: center;
		padding: 2rem 0;
	}

	.success-icon {
		width: 4rem;
		height: 4rem;
		margin: 0 auto 1rem;
		background: var(--color-success);
		border-radius: 50%;
		display: flex;
		align-items: center;
		justify-content: center;
	}

	.success-icon svg {
		width: 2rem;
		height: 2rem;
		color: white;
	}

	.success-text {
		font-size: 1.25rem;
		font-weight: 600;
		color: var(--color-text);
		margin-bottom: 1.5rem;
	}

	.action-buttons {
		display: flex;
		gap: 1rem;
		justify-content: center;
		flex-wrap: wrap;
	}

	.features {
		display: grid;
		grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
		gap: 2rem;
	}

	.feature {
		text-align: center;
		padding: 1.5rem;
	}

	.feature-icon {
		width: 3rem;
		height: 3rem;
		margin: 0 auto 1rem;
		color: var(--color-primary);
	}

	.feature-icon svg {
		width: 100%;
		height: 100%;
	}

	.feature h3 {
		font-size: 1.1rem;
		font-weight: 600;
		margin-bottom: 0.5rem;
		color: var(--color-text);
	}

	.feature p {
		font-size: 0.875rem;
		color: var(--color-text-secondary);
		line-height: 1.5;
	}

	@media (max-width: 640px) {
		.title {
			font-size: 2rem;
		}

		.converter-card {
			padding: 1.5rem;
		}

		.action-buttons {
			flex-direction: column;
		}

		.btn {
			width: 100%;
			justify-content: center;
		}
	}
</style>
