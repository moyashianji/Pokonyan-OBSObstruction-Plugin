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
		getSystemCapabilities,
		type ConversionState,
		type FileType
	} from '$lib/converter';

	let files = $state<File[]>([]);
	let selectedFormat = $state('');
	let fileType = $state<FileType>(null);
	let capabilities = $state({ webcodecs: false, ffmpeg: false, sharedArrayBuffer: false });
	let mounted = $state(false);

	let conversionState = $state<ConversionState>({
		status: 'idle',
		progress: 0,
		message: '',
		outputUrl: null,
		outputFileName: null
	});

	let isConverting = $derived(conversionState.status === 'converting' || conversionState.status === 'loading');

	// File type icons
	const fileTypeIcons: Record<string, string> = {
		image: '🖼️',
		audio: '🎵',
		video: '🎬',
		document: '📄'
	};

	onMount(() => {
		mounted = true;
		capabilities = getSystemCapabilities();

		if (capabilities.sharedArrayBuffer) {
			preloadFFmpeg().catch(console.warn);
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
		document.body.appendChild(a);
		a.click();
		document.body.removeChild(a);
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
	<title>Universal Converter - 究極のファイル変換</title>
	<meta name="description" content="ブラウザで完結する究極のファイル変換ツール。GPU高速変換、50+形式対応、完全プライベート。" />
	<link rel="preconnect" href="https://fonts.googleapis.com">
	<link rel="preconnect" href="https://fonts.gstatic.com" crossorigin="anonymous">
	<link href="https://fonts.googleapis.com/css2?family=Inter:wght@400;500;600;700&display=swap" rel="stylesheet">
</svelte:head>

<main class="container" class:mounted>
	<!-- Hero Header -->
	<header class="header">
		<div class="logo">
			<span class="logo-icon">⚡</span>
		</div>
		<h1 class="title">
			<span class="gradient-text">Universal</span> Converter
		</h1>
		<p class="subtitle">
			GPU高速変換 · 50+形式対応 · 完全プライベート
		</p>

		<!-- Capability Badges -->
		<div class="badges">
			<span class="badge" class:active={capabilities.webcodecs}>
				<span class="badge-icon">⚡</span>
				GPU高速
			</span>
			<span class="badge active">
				<span class="badge-icon">🔒</span>
				プライベート
			</span>
			<span class="badge" class:active={capabilities.ffmpeg}>
				<span class="badge-icon">🎬</span>
				FFmpeg
			</span>
		</div>
	</header>

	<!-- Main Card -->
	<div class="main-card glass">
		{#if files.length === 0}
			<!-- File Upload State -->
			<FileDropzone on:files={handleFiles} disabled={isConverting} />
		{:else if conversionState.status === 'complete'}
			<!-- Success State -->
			<div class="success-state animate-fadeInScale">
				<div class="success-icon">
					<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2.5">
						<polyline points="20 6 9 17 4 12" />
					</svg>
				</div>
				<h2 class="success-title">変換完了!</h2>
				<p class="success-filename">{conversionState.outputFileName}</p>

				<div class="success-actions">
					<button class="btn btn-primary btn-lg" onclick={downloadFile}>
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
			<!-- Conversion Flow -->
			<div class="conversion-flow">
				<!-- File Info -->
				<div class="file-card animate-fadeInUp">
					<div class="file-icon-wrapper">
						<span class="file-type-icon">{fileTypeIcons[fileType || 'document']}</span>
					</div>
					<div class="file-info">
						<p class="file-name">{files[0].name}</p>
						<p class="file-meta">
							<span>{formatFileSize(files[0].size)}</span>
							<span class="separator">·</span>
							<span class="file-type-label">{fileType?.toUpperCase()}</span>
						</p>
					</div>
					<button class="btn-icon" onclick={reset} aria-label="削除" disabled={isConverting}>
						<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
							<line x1="18" y1="6" x2="6" y2="18" />
							<line x1="6" y1="6" x2="18" y2="18" />
						</svg>
					</button>
				</div>

				<!-- Arrow -->
				<div class="flow-arrow">
					<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
						<line x1="12" y1="5" x2="12" y2="19" />
						<polyline points="19 12 12 19 5 12" />
					</svg>
				</div>

				<!-- Format Selection or Progress -->
				{#if isConverting}
					<div class="progress-card animate-fadeInScale">
						<ConversionProgress
							progress={conversionState.progress}
							status={conversionState.status}
							message={conversionState.message}
						/>
					</div>
				{:else}
					<div class="format-card animate-fadeInUp" style="animation-delay: 100ms">
						<FormatSelector
							{selectedFormat}
							{fileType}
							on:select={handleFormatSelect}
						/>
					</div>

					<!-- Convert Button -->
					<button
						class="btn btn-convert"
						class:ready={selectedFormat}
						disabled={!selectedFormat}
						onclick={startConversion}
					>
						<span class="btn-content">
							<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
								<polygon points="5 3 19 12 5 21 5 3" />
							</svg>
							{selectedFormat ? `${selectedFormat.toUpperCase()} に変換` : '形式を選択してください'}
						</span>
					</button>
				{/if}
			</div>
		{/if}
	</div>

	<!-- Features -->
	<section class="features">
		<div class="feature glass">
			<div class="feature-icon gpu">⚡</div>
			<h3>GPU高速変換</h3>
			<p>WebCodecs APIでハードウェアアクセラレーション。従来の10-100倍高速。</p>
		</div>
		<div class="feature glass">
			<div class="feature-icon">🔒</div>
			<h3>完全プライベート</h3>
			<p>ファイルはブラウザ内で処理。サーバーに一切送信されません。</p>
		</div>
		<div class="feature glass">
			<div class="feature-icon">🎬</div>
			<h3>50+形式対応</h3>
			<p>動画、音声、画像あらゆる形式に対応。FFmpegの力をブラウザで。</p>
		</div>
	</section>

	<!-- Footer -->
	<footer class="footer">
		<p>Powered by WebCodecs · Canvas API · FFmpeg.wasm</p>
	</footer>
</main>

<style>
	/* Container */
	.container {
		max-width: 800px;
		margin: 0 auto;
		padding: 2rem 1rem;
		opacity: 0;
		transition: opacity 0.5s ease;
	}

	.container.mounted {
		opacity: 1;
	}

	/* Header */
	.header {
		text-align: center;
		margin-bottom: 2.5rem;
	}

	.logo {
		margin-bottom: 1rem;
	}

	.logo-icon {
		font-size: 3rem;
		display: inline-block;
		animation: float 3s ease-in-out infinite;
	}

	.title {
		font-size: 2.5rem;
		font-weight: 700;
		margin-bottom: 0.5rem;
		letter-spacing: -0.02em;
	}

	.gradient-text {
		background: var(--gradient-primary);
		-webkit-background-clip: text;
		-webkit-text-fill-color: transparent;
		background-clip: text;
	}

	.subtitle {
		font-size: 1rem;
		color: var(--color-text-secondary);
		margin-bottom: 1rem;
	}

	/* Badges */
	.badges {
		display: flex;
		justify-content: center;
		gap: 0.5rem;
		flex-wrap: wrap;
	}

	.badge {
		display: inline-flex;
		align-items: center;
		gap: 0.25rem;
		padding: 0.35rem 0.75rem;
		font-size: 0.75rem;
		font-weight: 500;
		border-radius: 2rem;
		background: var(--color-bg-tertiary);
		color: var(--color-text-muted);
		border: 1px solid var(--color-border);
		transition: all var(--transition-fast);
	}

	.badge.active {
		background: rgba(16, 185, 129, 0.15);
		color: var(--color-success);
		border-color: rgba(16, 185, 129, 0.3);
	}

	.badge-icon {
		font-size: 0.875rem;
	}

	/* Main Card */
	.main-card {
		border-radius: var(--radius-xl);
		padding: 2rem;
		margin-bottom: 2.5rem;
		box-shadow: var(--shadow-lg);
	}

	/* Conversion Flow */
	.conversion-flow {
		display: flex;
		flex-direction: column;
		gap: 1rem;
	}

	.file-card {
		display: flex;
		align-items: center;
		gap: 1rem;
		padding: 1rem;
		background: var(--color-bg-tertiary);
		border-radius: var(--radius-md);
		border: 1px solid var(--color-border);
	}

	.file-icon-wrapper {
		width: 3rem;
		height: 3rem;
		display: flex;
		align-items: center;
		justify-content: center;
		background: var(--gradient-primary);
		border-radius: var(--radius-md);
		font-size: 1.5rem;
	}

	.file-info {
		flex: 1;
		min-width: 0;
	}

	.file-name {
		font-weight: 600;
		font-size: 0.95rem;
		white-space: nowrap;
		overflow: hidden;
		text-overflow: ellipsis;
	}

	.file-meta {
		font-size: 0.8rem;
		color: var(--color-text-secondary);
		display: flex;
		align-items: center;
		gap: 0.5rem;
	}

	.separator {
		color: var(--color-text-muted);
	}

	.file-type-label {
		color: var(--color-primary-light);
		font-weight: 500;
	}

	.btn-icon {
		width: 2.25rem;
		height: 2.25rem;
		display: flex;
		align-items: center;
		justify-content: center;
		background: transparent;
		border: none;
		border-radius: var(--radius-sm);
		color: var(--color-text-secondary);
		cursor: pointer;
		transition: all var(--transition-fast);
	}

	.btn-icon:hover:not(:disabled) {
		color: var(--color-error);
		background: rgba(239, 68, 68, 0.1);
	}

	.btn-icon:disabled {
		opacity: 0.5;
		cursor: not-allowed;
	}

	.btn-icon svg {
		width: 1.25rem;
		height: 1.25rem;
	}

	/* Flow Arrow */
	.flow-arrow {
		display: flex;
		justify-content: center;
		color: var(--color-text-muted);
	}

	.flow-arrow svg {
		width: 1.5rem;
		height: 1.5rem;
	}

	/* Format Card */
	.format-card {
		padding: 0.5rem 0;
	}

	/* Progress Card */
	.progress-card {
		padding: 1.5rem;
		background: var(--color-bg-tertiary);
		border-radius: var(--radius-md);
		border: 1px solid var(--color-border);
	}

	/* Convert Button */
	.btn-convert {
		width: 100%;
		padding: 1rem 1.5rem;
		font-size: 1rem;
		font-weight: 600;
		border: none;
		border-radius: var(--radius-md);
		background: var(--color-bg-tertiary);
		color: var(--color-text-muted);
		cursor: not-allowed;
		transition: all var(--transition-normal);
	}

	.btn-convert.ready {
		background: var(--gradient-primary);
		color: white;
		cursor: pointer;
		box-shadow: var(--shadow-glow);
	}

	.btn-convert.ready:hover {
		transform: translateY(-2px);
		box-shadow: 0 8px 30px rgba(99, 102, 241, 0.4);
	}

	.btn-convert.ready:active {
		transform: translateY(0);
	}

	.btn-content {
		display: flex;
		align-items: center;
		justify-content: center;
		gap: 0.5rem;
	}

	.btn-content svg {
		width: 1.25rem;
		height: 1.25rem;
	}

	/* Success State */
	.success-state {
		text-align: center;
		padding: 2rem 0;
	}

	.success-icon {
		width: 5rem;
		height: 5rem;
		margin: 0 auto 1.5rem;
		display: flex;
		align-items: center;
		justify-content: center;
		background: linear-gradient(135deg, var(--color-success), #059669);
		border-radius: 50%;
		color: white;
		box-shadow: 0 0 40px rgba(16, 185, 129, 0.4);
	}

	.success-icon svg {
		width: 2.5rem;
		height: 2.5rem;
	}

	.success-title {
		font-size: 1.5rem;
		font-weight: 700;
		margin-bottom: 0.5rem;
	}

	.success-filename {
		color: var(--color-text-secondary);
		font-size: 0.9rem;
		margin-bottom: 1.5rem;
	}

	.success-actions {
		display: flex;
		flex-direction: column;
		gap: 0.75rem;
		max-width: 300px;
		margin: 0 auto;
	}

	/* Buttons */
	.btn {
		display: inline-flex;
		align-items: center;
		justify-content: center;
		gap: 0.5rem;
		padding: 0.75rem 1.5rem;
		font-size: 0.95rem;
		font-weight: 500;
		border: none;
		border-radius: var(--radius-md);
		cursor: pointer;
		transition: all var(--transition-fast);
	}

	.btn svg {
		width: 1.25rem;
		height: 1.25rem;
	}

	.btn-primary {
		background: var(--gradient-primary);
		color: white;
	}

	.btn-primary:hover {
		transform: translateY(-2px);
		box-shadow: var(--shadow-glow);
	}

	.btn-lg {
		padding: 1rem 2rem;
		font-size: 1rem;
	}

	.btn-secondary {
		background: var(--color-bg-tertiary);
		color: var(--color-text);
		border: 1px solid var(--color-border);
	}

	.btn-secondary:hover {
		border-color: var(--color-primary);
		background: rgba(99, 102, 241, 0.1);
	}

	/* Features */
	.features {
		display: grid;
		grid-template-columns: repeat(auto-fit, minmax(220px, 1fr));
		gap: 1rem;
		margin-bottom: 2rem;
	}

	.feature {
		padding: 1.5rem;
		border-radius: var(--radius-lg);
		text-align: center;
		transition: transform var(--transition-fast);
	}

	.feature:hover {
		transform: translateY(-4px);
	}

	.feature-icon {
		font-size: 2rem;
		margin-bottom: 0.75rem;
		display: block;
	}

	.feature-icon.gpu {
		animation: glow 2s ease-in-out infinite;
	}

	.feature h3 {
		font-size: 1rem;
		font-weight: 600;
		margin-bottom: 0.5rem;
	}

	.feature p {
		font-size: 0.8rem;
		color: var(--color-text-secondary);
		line-height: 1.5;
	}

	/* Footer */
	.footer {
		text-align: center;
		padding: 1rem;
		color: var(--color-text-muted);
		font-size: 0.75rem;
	}

	/* Mobile */
	@media (max-width: 640px) {
		.container {
			padding: 1.5rem 1rem;
		}

		.title {
			font-size: 2rem;
		}

		.main-card {
			padding: 1.5rem;
		}

		.badges {
			gap: 0.35rem;
		}

		.badge {
			padding: 0.25rem 0.5rem;
			font-size: 0.7rem;
		}

		.file-card {
			padding: 0.75rem;
		}

		.file-icon-wrapper {
			width: 2.5rem;
			height: 2.5rem;
			font-size: 1.25rem;
		}

		.file-name {
			font-size: 0.85rem;
		}
	}
</style>
