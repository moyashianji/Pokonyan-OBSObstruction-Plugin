<script lang="ts">
	import { onMount } from 'svelte';
	import Icon from '$lib/components/Icon.svelte';
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

	interface FileItem {
		file: File;
		type: FileType;
		selectedFormats: string[];
		state: ConversionState;
		results: { format: string; url: string; fileName: string }[];
	}

	let fileItems = $state<FileItem[]>([]);
	let capabilities = $state({ webcodecs: false, ffmpeg: false, sharedArrayBuffer: false });
	let mounted = $state(false);
	let isConverting = $state(false);

	onMount(() => {
		mounted = true;
		capabilities = getSystemCapabilities();

		if (capabilities.sharedArrayBuffer) {
			preloadFFmpeg().catch(console.warn);
		}
	});

	function handleFiles(event: CustomEvent<File[]>) {
		const newFiles = event.detail.map(file => ({
			file,
			type: detectFileType(file),
			selectedFormats: [] as string[],
			state: {
				status: 'idle' as const,
				progress: 0,
				message: '',
				outputUrl: null,
				outputFileName: null
			},
			results: []
		}));
		fileItems = [...fileItems, ...newFiles];
	}

	function toggleFormat(fileIndex: number, format: string) {
		const item = fileItems[fileIndex];
		if (item.selectedFormats.includes(format)) {
			item.selectedFormats = item.selectedFormats.filter(f => f !== format);
		} else {
			item.selectedFormats = [...item.selectedFormats, format];
		}
	}

	function removeFile(index: number) {
		// Cleanup URLs
		fileItems[index].results.forEach(r => URL.revokeObjectURL(r.url));
		if (fileItems[index].state.outputUrl) {
			URL.revokeObjectURL(fileItems[index].state.outputUrl!);
		}
		fileItems = fileItems.filter((_, i) => i !== index);
	}

	async function convertSingleFileToFormat(fileItem: FileItem, format: string): Promise<{ format: string; url: string; fileName: string }> {
		const result = await convertFile(fileItem.file, format, (state) => {
			fileItem.state = { ...fileItem.state, ...state };
		});
		return { format, url: result.url, fileName: result.fileName };
	}

	async function startConversion() {
		isConverting = true;

		// Process all files in parallel
		const conversionPromises = fileItems.map(async (item, index) => {
			if (item.selectedFormats.length === 0) return;

			item.state = {
				status: 'converting',
				progress: 0,
				message: '変換開始...',
				outputUrl: null,
				outputFileName: null
			};

			// Convert to all selected formats in parallel
			const formatPromises = item.selectedFormats.map(async (format) => {
				try {
					const result = await convertFile(item.file, format, (state) => {
						// Update progress (average across formats)
						const currentProgress = item.state.progress;
						item.state = {
							...item.state,
							progress: Math.max(currentProgress, state.progress || 0),
							message: state.message || item.state.message
						};
					});
					return { format, url: result.url, fileName: result.fileName };
				} catch (error) {
					console.error(`Failed to convert to ${format}:`, error);
					return null;
				}
			});

			const results = await Promise.all(formatPromises);
			item.results = results.filter((r): r is { format: string; url: string; fileName: string } => r !== null);

			item.state = {
				status: 'complete',
				progress: 100,
				message: `${item.results.length}形式の変換完了`,
				outputUrl: null,
				outputFileName: null
			};
		});

		await Promise.all(conversionPromises);
		isConverting = false;
	}

	function downloadResult(result: { url: string; fileName: string }) {
		const a = document.createElement('a');
		a.href = result.url;
		a.download = result.fileName;
		document.body.appendChild(a);
		a.click();
		document.body.removeChild(a);
	}

	function downloadAllResults(item: FileItem) {
		item.results.forEach(result => {
			downloadResult(result);
		});
	}

	function reset() {
		fileItems.forEach(item => {
			item.results.forEach(r => URL.revokeObjectURL(r.url));
			if (item.state.outputUrl) {
				URL.revokeObjectURL(item.state.outputUrl!);
			}
		});
		fileItems = [];
	}

	let totalSelectedFormats = $derived(fileItems.reduce((sum, item) => sum + item.selectedFormats.length, 0));
	let allComplete = $derived(fileItems.length > 0 && fileItems.every(item => item.state.status === 'complete'));
	let hasResults = $derived(fileItems.some(item => item.results.length > 0));
</script>

<svelte:head>
	<title>Universal Converter - 究極のファイル変換</title>
	<meta name="description" content="ブラウザで完結する究極のファイル変換ツール。GPU高速変換、50+形式対応、完全プライベート。" />
	<link rel="preconnect" href="https://fonts.googleapis.com">
	<link rel="preconnect" href="https://fonts.gstatic.com" crossorigin="anonymous">
	<link href="https://fonts.googleapis.com/css2?family=Inter:wght@400;500;600;700&display=swap" rel="stylesheet">
</svelte:head>

<main class="container" class:mounted>
	<!-- Header -->
	<header class="header">
		<div class="logo">
			<Icon name="zap" size={32} />
		</div>
		<h1 class="title">
			<span class="gradient-text">Universal</span> Converter
		</h1>
		<p class="subtitle">
			GPU高速変換 · 50+形式対応 · 複数ファイル並列処理
		</p>

		<!-- Capability Badges -->
		<div class="badges">
			<span class="badge" class:active={capabilities.webcodecs}>
				<Icon name="gpu" size={14} />
				GPU高速
			</span>
			<span class="badge active">
				<Icon name="lock" size={14} />
				プライベート
			</span>
			<span class="badge" class:active={capabilities.ffmpeg}>
				<Icon name="layers" size={14} />
				50+形式
			</span>
		</div>
	</header>

	<!-- Main Card -->
	<div class="main-card glass">
		{#if fileItems.length === 0}
			<FileDropzone on:files={handleFiles} disabled={isConverting} />
		{:else}
			<!-- File List -->
			<div class="file-list">
				{#each fileItems as item, index}
					<div class="file-item" class:complete={item.state.status === 'complete'}>
						<!-- File Header -->
						<div class="file-header">
							<div class="file-icon-wrapper">
								<Icon name={item.type === 'video' ? 'video' : item.type === 'audio' ? 'audio' : item.type === 'image' ? 'image' : 'document'} size={20} />
							</div>
							<div class="file-info">
								<p class="file-name">{item.file.name}</p>
								<p class="file-meta">
									{formatFileSize(item.file.size)} · {item.type?.toUpperCase()}
									{#if item.selectedFormats.length > 0}
										<span class="format-count">→ {item.selectedFormats.length}形式</span>
									{/if}
								</p>
							</div>
							<button class="btn-icon" onclick={() => removeFile(index)} disabled={isConverting}>
								<Icon name="x" size={18} />
							</button>
						</div>

						<!-- Format Selection or Results -->
						{#if item.state.status === 'complete' && item.results.length > 0}
							<div class="results-section">
								<div class="results-header">
									<span class="results-label">変換完了</span>
									<button class="btn btn-sm btn-secondary" onclick={() => downloadAllResults(item)}>
										<Icon name="download" size={14} />
										全てダウンロード
									</button>
								</div>
								<div class="results-grid">
									{#each item.results as result}
										<button class="result-btn" onclick={() => downloadResult(result)}>
											<Icon name="download" size={16} />
											<span>{result.fileName.split('.').pop()?.toUpperCase()}</span>
										</button>
									{/each}
								</div>
							</div>
						{:else if item.state.status === 'converting'}
							<div class="progress-section">
								<ConversionProgress
									progress={item.state.progress}
									status={item.state.status}
									message={item.state.message}
								/>
							</div>
						{:else}
							<div class="format-section">
								<FormatSelector
									fileType={item.type}
									selectedFormats={item.selectedFormats}
									multiSelect={true}
									on:toggle={(e) => toggleFormat(index, e.detail)}
								/>
							</div>
						{/if}
					</div>
				{/each}
			</div>

			<!-- Add More Files -->
			{#if !isConverting && !allComplete}
				<div class="add-more">
					<FileDropzone on:files={handleFiles} compact={true} />
				</div>
			{/if}

			<!-- Action Buttons -->
			<div class="action-bar">
				{#if allComplete}
					<button class="btn btn-secondary" onclick={reset}>
						<Icon name="refresh" size={18} />
						新しい変換
					</button>
				{:else}
					<button class="btn btn-secondary" onclick={reset} disabled={isConverting}>
						<Icon name="x" size={18} />
						クリア
					</button>
					<button
						class="btn btn-primary btn-lg"
						disabled={totalSelectedFormats === 0 || isConverting}
						onclick={startConversion}
					>
						{#if isConverting}
							<span class="spinner"></span>
							変換中...
						{:else}
							<Icon name="play" size={18} />
							{totalSelectedFormats > 0 ? `${fileItems.length}ファイル × ${totalSelectedFormats}形式 変換開始` : '形式を選択'}
						{/if}
					</button>
				{/if}
			</div>
		{/if}
	</div>

	<!-- Features -->
	<section class="features">
		<div class="feature glass">
			<div class="feature-icon">
				<Icon name="zap" size={28} />
			</div>
			<h3>GPU高速変換</h3>
			<p>WebCodecs APIでハードウェア支援。従来の10-100倍高速。</p>
		</div>
		<div class="feature glass">
			<div class="feature-icon">
				<Icon name="layers" size={28} />
			</div>
			<h3>並列処理</h3>
			<p>複数ファイル・複数形式を同時に変換。待ち時間を大幅短縮。</p>
		</div>
		<div class="feature glass">
			<div class="feature-icon">
				<Icon name="lock" size={28} />
			</div>
			<h3>完全プライベート</h3>
			<p>全てブラウザ内で処理。サーバーに一切送信されません。</p>
		</div>
	</section>

	<footer class="footer">
		<p>Powered by WebCodecs · Canvas API · FFmpeg.wasm</p>
	</footer>
</main>

<style>
	.container {
		max-width: 900px;
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
		margin-bottom: 2rem;
	}

	.logo {
		margin-bottom: 0.75rem;
		color: var(--color-primary);
	}

	.title {
		font-size: 2.25rem;
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
		font-size: 0.95rem;
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
		gap: 0.35rem;
		padding: 0.35rem 0.75rem;
		font-size: 0.75rem;
		font-weight: 500;
		border-radius: 2rem;
		background: var(--color-bg-tertiary);
		color: var(--color-text-muted);
		border: 1px solid var(--color-border);
	}

	.badge.active {
		background: rgba(16, 185, 129, 0.15);
		color: var(--color-success);
		border-color: rgba(16, 185, 129, 0.3);
	}

	/* Main Card */
	.main-card {
		border-radius: var(--radius-xl);
		padding: 1.5rem;
		margin-bottom: 2rem;
		box-shadow: var(--shadow-lg);
	}

	/* File List */
	.file-list {
		display: flex;
		flex-direction: column;
		gap: 1rem;
	}

	.file-item {
		background: var(--color-bg-tertiary);
		border-radius: var(--radius-lg);
		padding: 1rem;
		border: 1px solid var(--color-border);
		transition: all 0.2s ease;
	}

	.file-item.complete {
		border-color: rgba(16, 185, 129, 0.3);
		background: rgba(16, 185, 129, 0.05);
	}

	.file-header {
		display: flex;
		align-items: center;
		gap: 0.75rem;
	}

	.file-icon-wrapper {
		width: 2.5rem;
		height: 2.5rem;
		display: flex;
		align-items: center;
		justify-content: center;
		background: var(--gradient-primary);
		border-radius: var(--radius-md);
		color: white;
	}

	.file-info {
		flex: 1;
		min-width: 0;
	}

	.file-name {
		font-weight: 600;
		font-size: 0.9rem;
		white-space: nowrap;
		overflow: hidden;
		text-overflow: ellipsis;
	}

	.file-meta {
		font-size: 0.75rem;
		color: var(--color-text-secondary);
	}

	.format-count {
		color: var(--color-primary-light);
		font-weight: 500;
	}

	.btn-icon {
		width: 2rem;
		height: 2rem;
		display: flex;
		align-items: center;
		justify-content: center;
		background: transparent;
		border: none;
		border-radius: var(--radius-sm);
		color: var(--color-text-secondary);
		cursor: pointer;
		transition: all 0.15s ease;
	}

	.btn-icon:hover:not(:disabled) {
		color: var(--color-error);
		background: rgba(239, 68, 68, 0.1);
	}

	.btn-icon:disabled {
		opacity: 0.5;
		cursor: not-allowed;
	}

	/* Format Section */
	.format-section {
		margin-top: 1rem;
		padding-top: 1rem;
		border-top: 1px solid var(--color-border);
	}

	/* Progress Section */
	.progress-section {
		margin-top: 1rem;
		padding-top: 1rem;
		border-top: 1px solid var(--color-border);
	}

	/* Results Section */
	.results-section {
		margin-top: 1rem;
		padding-top: 1rem;
		border-top: 1px solid rgba(16, 185, 129, 0.2);
	}

	.results-header {
		display: flex;
		justify-content: space-between;
		align-items: center;
		margin-bottom: 0.75rem;
	}

	.results-label {
		font-size: 0.8rem;
		font-weight: 600;
		color: var(--color-success);
	}

	.results-grid {
		display: flex;
		flex-wrap: wrap;
		gap: 0.5rem;
	}

	.result-btn {
		display: inline-flex;
		align-items: center;
		gap: 0.35rem;
		padding: 0.5rem 0.75rem;
		font-size: 0.8rem;
		font-weight: 600;
		background: var(--color-bg-glass);
		border: 1px solid var(--color-success);
		border-radius: var(--radius-sm);
		color: var(--color-success);
		cursor: pointer;
		transition: all 0.15s ease;
	}

	.result-btn:hover {
		background: rgba(16, 185, 129, 0.15);
	}

	/* Add More */
	.add-more {
		margin-top: 1rem;
		padding-top: 1rem;
		border-top: 1px dashed var(--color-border);
	}

	/* Action Bar */
	.action-bar {
		display: flex;
		gap: 0.75rem;
		margin-top: 1.5rem;
		padding-top: 1rem;
		border-top: 1px solid var(--color-border);
	}

	/* Buttons */
	.btn {
		display: inline-flex;
		align-items: center;
		justify-content: center;
		gap: 0.5rem;
		padding: 0.65rem 1.25rem;
		font-size: 0.9rem;
		font-weight: 500;
		border: none;
		border-radius: var(--radius-md);
		cursor: pointer;
		transition: all 0.15s ease;
	}

	.btn:disabled {
		opacity: 0.5;
		cursor: not-allowed;
	}

	.btn-sm {
		padding: 0.4rem 0.75rem;
		font-size: 0.75rem;
	}

	.btn-lg {
		flex: 1;
		padding: 0.85rem 1.5rem;
		font-size: 0.95rem;
	}

	.btn-primary {
		background: var(--gradient-primary);
		color: white;
	}

	.btn-primary:hover:not(:disabled) {
		box-shadow: var(--shadow-glow);
		transform: translateY(-1px);
	}

	.btn-secondary {
		background: var(--color-bg-tertiary);
		color: var(--color-text);
		border: 1px solid var(--color-border);
	}

	.btn-secondary:hover:not(:disabled) {
		border-color: var(--color-primary);
		background: rgba(99, 102, 241, 0.1);
	}

	.spinner {
		width: 1rem;
		height: 1rem;
		border: 2px solid rgba(255, 255, 255, 0.3);
		border-top-color: white;
		border-radius: 50%;
		animation: spin 0.8s linear infinite;
	}

	@keyframes spin {
		to { transform: rotate(360deg); }
	}

	/* Features */
	.features {
		display: grid;
		grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
		gap: 1rem;
		margin-bottom: 1.5rem;
	}

	.feature {
		padding: 1.25rem;
		border-radius: var(--radius-lg);
		text-align: center;
		transition: transform 0.15s ease;
	}

	.feature:hover {
		transform: translateY(-2px);
	}

	.feature-icon {
		color: var(--color-primary);
		margin-bottom: 0.75rem;
	}

	.feature h3 {
		font-size: 0.95rem;
		font-weight: 600;
		margin-bottom: 0.35rem;
	}

	.feature p {
		font-size: 0.75rem;
		color: var(--color-text-secondary);
		line-height: 1.4;
	}

	/* Footer */
	.footer {
		text-align: center;
		padding: 1rem;
		color: var(--color-text-muted);
		font-size: 0.7rem;
	}

	/* Mobile */
	@media (max-width: 640px) {
		.title {
			font-size: 1.75rem;
		}

		.main-card {
			padding: 1rem;
		}

		.action-bar {
			flex-direction: column;
		}

		.btn-lg {
			width: 100%;
		}
	}
</style>
