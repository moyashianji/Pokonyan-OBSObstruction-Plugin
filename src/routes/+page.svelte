<script lang="ts">
	import { onMount } from 'svelte';
	import Icon from '$lib/components/Icon.svelte';
	import FileDropzone from '$lib/components/FileDropzone.svelte';
	import FormatSelector from '$lib/components/FormatSelector.svelte';
	import ConversionProgress from '$lib/components/ConversionProgress.svelte';
	import LogPanel, { type LogEntry } from '$lib/components/LogPanel.svelte';
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
		results: { format: string; url: string; fileName: string; outputSize?: number }[];
	}

	let fileItems = $state<FileItem[]>([]);
	let capabilities = $state({ webcodecs: false, ffmpeg: false, sharedArrayBuffer: false });
	let mounted = $state(false);
	let isConverting = $state(false);

	// Logging
	let logs = $state<LogEntry[]>([]);
	let logExpanded = $state(false);
	let logIdCounter = $state(0);
	let conversionStartTime = $state<number>(0);

	function addLog(level: LogEntry['level'], message: string, details?: Record<string, string | number>) {
		logs = [...logs, { id: logIdCounter++, timestamp: new Date(), level, message, details }];
	}

	function clearLogs() {
		logs = [];
	}

	onMount(() => {
		mounted = true;
		capabilities = getSystemCapabilities();
		addLog('info', 'System ready', {
			WebCodecs: capabilities.webcodecs ? 'available' : 'unavailable',
			FFmpeg: capabilities.sharedArrayBuffer ? 'available' : 'unavailable'
		});

		if (capabilities.sharedArrayBuffer) {
			preloadFFmpeg()
				.then(() => addLog('success', 'FFmpeg initialized'))
				.catch(() => addLog('warning', 'FFmpeg init failed'));
		}
	});

	function handleFiles(event: CustomEvent<File[]>) {
		const newFiles = event.detail.map(file => {
			const type = detectFileType(file);
			addLog('info', `File added: ${file.name}`, { size: file.size, type: type || 'unknown' });
			return {
				file,
				type,
				selectedFormats: [] as string[],
				state: { status: 'idle' as const, progress: 0, message: '', outputUrl: null, outputFileName: null },
				results: []
			};
		});
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
		fileItems[index].results.forEach(r => URL.revokeObjectURL(r.url));
		if (fileItems[index].state.outputUrl) URL.revokeObjectURL(fileItems[index].state.outputUrl!);
		fileItems = fileItems.filter((_, i) => i !== index);
	}

	async function startConversion() {
		isConverting = true;
		conversionStartTime = performance.now();
		logExpanded = true;

		const totalFiles = fileItems.filter(item => item.selectedFormats.length > 0).length;
		const totalFormats = fileItems.reduce((sum, item) => sum + item.selectedFormats.length, 0);
		addLog('info', `Starting conversion: ${totalFiles} files → ${totalFormats} outputs`);

		const conversionPromises = fileItems.map(async (item) => {
			if (item.selectedFormats.length === 0) return;

			const fileStartTime = performance.now();
			item.state = { status: 'converting', progress: 0, message: 'Initializing...', outputUrl: null, outputFileName: null };

			const formatPromises = item.selectedFormats.map(async (format) => {
				const formatStartTime = performance.now();
				addLog('debug', `Converting: ${item.file.name} → .${format}`);

				try {
					const result = await convertFile(item.file, format, (state) => {
						item.state = {
							...item.state,
							progress: Math.max(item.state.progress, state.progress || 0),
							message: state.message || item.state.message
						};
					});

					const response = await fetch(result.url);
					const blob = await response.blob();
					const outputSize = blob.size;
					const duration = performance.now() - formatStartTime;
					const ratio = ((outputSize / item.file.size) * 100).toFixed(0);

					addLog('success', `Done: ${item.file.name} → .${format}`, {
						inputSize: item.file.size,
						outputSize,
						ratio: `${ratio}%`,
						time: duration
					});

					return { format, url: result.url, fileName: result.fileName, outputSize };
				} catch (error) {
					addLog('error', `Failed: ${item.file.name} → .${format}`, {
						error: error instanceof Error ? error.message : 'Unknown error'
					});
					return null;
				}
			});

			const results = await Promise.all(formatPromises);
			item.results = results.filter((r): r is { format: string; url: string; fileName: string; outputSize?: number } => r !== null);

			const totalTime = performance.now() - fileStartTime;
			addLog('success', `Completed: ${item.file.name}`, {
				outputs: item.results.length,
				totalTime
			});

			item.state = { status: 'complete', progress: 100, message: 'Complete', outputUrl: null, outputFileName: null };
		});

		await Promise.all(conversionPromises);

		const totalTime = performance.now() - conversionStartTime;
		const successCount = fileItems.reduce((sum, item) => sum + item.results.length, 0);
		addLog('success', `All conversions complete`, {
			files: totalFiles,
			outputs: successCount,
			totalTime
		});

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
		item.results.forEach(downloadResult);
	}

	function reset() {
		fileItems.forEach(item => {
			item.results.forEach(r => URL.revokeObjectURL(r.url));
			if (item.state.outputUrl) URL.revokeObjectURL(item.state.outputUrl!);
		});
		fileItems = [];
		clearLogs();
		addLog('info', 'Ready for new conversion');
	}

	let totalSelectedFormats = $derived(fileItems.reduce((sum, item) => sum + item.selectedFormats.length, 0));
	let allComplete = $derived(fileItems.length > 0 && fileItems.every(item => item.state.status === 'complete'));
</script>

<svelte:head>
	<title>File Converter</title>
</svelte:head>

<main class="app" class:mounted>
	<header class="header">
		<h1>File Converter</h1>
		<p>Convert files locally in your browser. Nothing is uploaded.</p>
	</header>

	<div class="main-card">
		{#if fileItems.length === 0}
			<FileDropzone on:files={handleFiles} disabled={isConverting} />
		{:else}
			<div class="file-list">
				{#each fileItems as item, index (item.file.name + index)}
					<article class="file-card" class:complete={item.state.status === 'complete'}>
						<header class="file-header">
							<div class="file-icon">
								<Icon name={item.type === 'video' ? 'video' : item.type === 'audio' ? 'audio' : 'image'} size={18} />
							</div>
							<div class="file-details">
								<h3 class="file-name">{item.file.name}</h3>
								<p class="file-meta">
									{formatFileSize(item.file.size)}
									{#if item.selectedFormats.length > 0}
										<span class="meta-sep">·</span>
										<span class="format-badge">{item.selectedFormats.length} format{item.selectedFormats.length > 1 ? 's' : ''} selected</span>
									{/if}
								</p>
							</div>
							<button class="btn-close" onclick={() => removeFile(index)} disabled={isConverting} title="Remove">
								<Icon name="x" size={16} />
							</button>
						</header>

						{#if item.state.status === 'complete' && item.results.length > 0}
							<div class="results-section">
								<div class="results-list">
									{#each item.results as result}
										<button class="download-btn" onclick={() => downloadResult(result)}>
											<Icon name="download" size={14} />
											<span>.{result.fileName.split('.').pop()}</span>
											{#if result.outputSize}
												<span class="size">{formatFileSize(result.outputSize)}</span>
											{/if}
										</button>
									{/each}
								</div>
								{#if item.results.length > 1}
									<button class="download-all" onclick={() => downloadAllResults(item)}>
										Download all ({item.results.length})
									</button>
								{/if}
							</div>
						{:else if item.state.status === 'converting'}
							<div class="progress-section">
								<ConversionProgress progress={item.state.progress} status={item.state.status} message={item.state.message} />
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
					</article>
				{/each}
			</div>

			{#if !isConverting && !allComplete}
				<div class="add-more">
					<FileDropzone on:files={handleFiles} compact={true} />
				</div>
			{/if}

			<div class="actions">
				{#if allComplete}
					<button class="btn btn-default" onclick={reset}>
						Start new conversion
					</button>
				{:else}
					<button class="btn btn-ghost" onclick={reset} disabled={isConverting}>
						Clear all
					</button>
					<button
						class="btn btn-primary"
						disabled={totalSelectedFormats === 0 || isConverting}
						onclick={startConversion}
					>
						{#if isConverting}
							<span class="btn-spinner"></span>
							Converting...
						{:else}
							Convert{#if totalSelectedFormats > 0}<span class="btn-badge">{totalSelectedFormats}</span>{/if}
						{/if}
					</button>
				{/if}
			</div>

			<LogPanel {logs} bind:expanded={logExpanded} maxHeight="220px" />
		{/if}
	</div>

	<footer class="footer">
		<div class="status">
			<span class="status-item">
				<span class="dot" class:active={capabilities.webcodecs}></span>
				WebCodecs
			</span>
			<span class="status-item">
				<span class="dot" class:active={capabilities.sharedArrayBuffer}></span>
				FFmpeg
			</span>
		</div>
	</footer>
</main>

<style>
	.app {
		max-width: 680px;
		margin: 0 auto;
		padding: 48px 24px 32px;
		opacity: 0;
		transition: opacity 0.25s ease;
	}

	.app.mounted {
		opacity: 1;
	}

	/* Header */
	.header {
		margin-bottom: 28px;
	}

	.header h1 {
		font-size: 26px;
		font-weight: 650;
		letter-spacing: -0.025em;
		margin-bottom: 6px;
	}

	.header p {
		color: var(--c-text-2);
		font-size: 15px;
	}

	/* Main Card */
	.main-card {
		background: var(--c-surface);
		border: 1px solid var(--c-border);
		border-radius: var(--radius-lg);
		padding: 24px;
		box-shadow: var(--shadow);
	}

	/* File List */
	.file-list {
		display: flex;
		flex-direction: column;
		gap: 16px;
	}

	/* File Card */
	.file-card {
		background: var(--c-surface-raised);
		border: 1px solid var(--c-border-subtle);
		border-radius: var(--radius);
		padding: 18px;
		animation: fadeIn 0.25s ease;
	}

	.file-card.complete {
		border-color: color-mix(in srgb, var(--c-success) 30%, var(--c-border-subtle));
	}

	.file-header {
		display: flex;
		align-items: flex-start;
		gap: 14px;
	}

	.file-icon {
		width: 40px;
		height: 40px;
		display: flex;
		align-items: center;
		justify-content: center;
		background: var(--c-accent-subtle);
		color: var(--c-accent);
		border-radius: var(--radius-sm);
		flex-shrink: 0;
	}

	.file-details {
		flex: 1;
		min-width: 0;
	}

	.file-name {
		font-size: 14px;
		font-weight: 550;
		white-space: nowrap;
		overflow: hidden;
		text-overflow: ellipsis;
		margin-bottom: 2px;
	}

	.file-meta {
		font-size: 12px;
		color: var(--c-text-3);
	}

	.meta-sep {
		margin: 0 6px;
	}

	.format-badge {
		color: var(--c-accent);
	}

	.btn-close {
		width: 28px;
		height: 28px;
		display: flex;
		align-items: center;
		justify-content: center;
		background: none;
		border: none;
		color: var(--c-text-3);
		border-radius: var(--radius-sm);
		cursor: pointer;
		transition: all 0.15s;
	}

	.btn-close:hover:not(:disabled) {
		color: var(--c-error);
		background: var(--c-error-subtle);
	}

	.btn-close:disabled {
		opacity: 0.4;
		cursor: not-allowed;
	}

	/* Sections */
	.format-section,
	.progress-section,
	.results-section {
		margin-top: 18px;
		padding-top: 18px;
		border-top: 1px solid var(--c-border-subtle);
	}

	/* Results */
	.results-list {
		display: flex;
		flex-wrap: wrap;
		gap: 8px;
		margin-bottom: 12px;
	}

	.download-btn {
		display: inline-flex;
		align-items: center;
		gap: 6px;
		padding: 8px 14px;
		font-size: 13px;
		font-weight: 500;
		background: var(--c-success-subtle);
		color: var(--c-success);
		border: none;
		border-radius: var(--radius-sm);
		cursor: pointer;
		transition: all 0.15s;
	}

	.download-btn:hover {
		background: color-mix(in srgb, var(--c-success) 18%, transparent);
	}

	.download-btn .size {
		font-size: 11px;
		opacity: 0.7;
		margin-left: 2px;
	}

	.download-all {
		font-size: 12px;
		font-weight: 500;
		color: var(--c-text-2);
		background: none;
		border: none;
		cursor: pointer;
		padding: 4px 0;
	}

	.download-all:hover {
		color: var(--c-text);
	}

	/* Add More */
	.add-more {
		margin-top: 18px;
		padding-top: 18px;
		border-top: 1px dashed var(--c-border);
	}

	/* Actions */
	.actions {
		display: flex;
		gap: 10px;
		margin-top: 22px;
		padding-top: 18px;
		border-top: 1px solid var(--c-border);
	}

	/* Buttons */
	.btn {
		display: inline-flex;
		align-items: center;
		justify-content: center;
		gap: 8px;
		padding: 11px 20px;
		font-size: 14px;
		font-weight: 550;
		border: none;
		border-radius: var(--radius);
		cursor: pointer;
		transition: all 0.15s;
	}

	.btn:disabled {
		opacity: 0.5;
		cursor: not-allowed;
	}

	.btn-primary {
		flex: 1;
		background: var(--c-accent);
		color: white;
	}

	.btn-primary:hover:not(:disabled) {
		background: var(--c-accent-hover);
	}

	.btn-default {
		flex: 1;
		background: var(--c-surface-raised);
		color: var(--c-text);
		border: 1px solid var(--c-border);
	}

	.btn-default:hover:not(:disabled) {
		background: var(--c-surface-hover);
	}

	.btn-ghost {
		background: none;
		color: var(--c-text-2);
	}

	.btn-ghost:hover:not(:disabled) {
		color: var(--c-text);
		background: var(--c-surface-hover);
	}

	.btn-badge {
		display: inline-flex;
		align-items: center;
		justify-content: center;
		min-width: 20px;
		height: 20px;
		padding: 0 6px;
		font-size: 11px;
		font-weight: 600;
		background: rgba(255,255,255,0.2);
		border-radius: 10px;
	}

	.btn-spinner {
		width: 16px;
		height: 16px;
		border: 2px solid rgba(255,255,255,0.25);
		border-top-color: white;
		border-radius: 50%;
		animation: spin 0.6s linear infinite;
	}

	@keyframes spin {
		to { transform: rotate(360deg); }
	}

	/* Footer */
	.footer {
		margin-top: 28px;
		display: flex;
		justify-content: center;
	}

	.status {
		display: flex;
		gap: 16px;
	}

	.status-item {
		display: flex;
		align-items: center;
		gap: 6px;
		font-size: 12px;
		color: var(--c-text-3);
	}

	.dot {
		width: 6px;
		height: 6px;
		border-radius: 50%;
		background: var(--c-text-3);
		opacity: 0.4;
	}

	.dot.active {
		background: var(--c-success);
		opacity: 1;
	}

	/* Responsive */
	@media (max-width: 520px) {
		.app {
			padding: 24px 16px;
		}

		.main-card {
			padding: 18px;
		}

		.file-card {
			padding: 14px;
		}

		.actions {
			flex-direction: column;
		}

		.btn {
			width: 100%;
		}

		.btn-ghost {
			order: 1;
		}
	}
</style>
