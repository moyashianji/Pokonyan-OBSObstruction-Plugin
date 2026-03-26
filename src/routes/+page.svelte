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
		addLog('info', 'Ready', {
			WebCodecs: capabilities.webcodecs ? 'OK' : 'N/A',
			SharedArrayBuffer: capabilities.sharedArrayBuffer ? 'OK' : 'N/A'
		});

		if (capabilities.sharedArrayBuffer) {
			preloadFFmpeg()
				.then(() => addLog('success', 'FFmpeg loaded'))
				.catch(() => addLog('warning', 'FFmpeg load failed'));
		}
	});

	function handleFiles(event: CustomEvent<File[]>) {
		const newFiles = event.detail.map(file => {
			const type = detectFileType(file);
			addLog('info', `Added: ${file.name}`, { size: file.size });
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
		addLog('info', `Converting ${totalFiles} file(s) to ${totalFormats} format(s)`);

		const conversionPromises = fileItems.map(async (item) => {
			if (item.selectedFormats.length === 0) return;

			const fileStartTime = performance.now();
			item.state = { status: 'converting', progress: 0, message: 'Starting...', outputUrl: null, outputFileName: null };

			const formatPromises = item.selectedFormats.map(async (format) => {
				const formatStartTime = performance.now();
				addLog('debug', `${item.file.name} → ${format.toUpperCase()}`);

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

					addLog('success', `${item.file.name} → ${format.toUpperCase()}`, {
						in: item.file.size,
						out: outputSize,
						time: duration
					});

					return { format, url: result.url, fileName: result.fileName, outputSize };
				} catch (error) {
					addLog('error', `Failed: ${item.file.name} → ${format}`, {
						error: error instanceof Error ? error.message : 'Unknown'
					});
					return null;
				}
			});

			const results = await Promise.all(formatPromises);
			item.results = results.filter((r): r is { format: string; url: string; fileName: string; outputSize?: number } => r !== null);
			item.state = { status: 'complete', progress: 100, message: 'Done', outputUrl: null, outputFileName: null };

			addLog('success', `${item.file.name} complete`, { formats: item.results.length, time: performance.now() - fileStartTime });
		});

		await Promise.all(conversionPromises);
		addLog('success', 'All done', { total: performance.now() - conversionStartTime });
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
		addLog('info', 'Reset');
	}

	let totalSelectedFormats = $derived(fileItems.reduce((sum, item) => sum + item.selectedFormats.length, 0));
	let allComplete = $derived(fileItems.length > 0 && fileItems.every(item => item.state.status === 'complete'));
</script>

<svelte:head>
	<title>Converter</title>
</svelte:head>

<main class="app" class:mounted>
	<header class="header">
		<h1>File Converter</h1>
		<p class="desc">Browser-based conversion. No upload, no server.</p>
	</header>

	<div class="card">
		{#if fileItems.length === 0}
			<FileDropzone on:files={handleFiles} disabled={isConverting} />
		{:else}
			<div class="file-list">
				{#each fileItems as item, index}
					<div class="file-item" class:done={item.state.status === 'complete'}>
						<div class="file-header">
							<div class="file-info">
								<span class="file-name">{item.file.name}</span>
								<span class="file-meta">
									{formatFileSize(item.file.size)}
									{#if item.selectedFormats.length > 0}
										<span class="arrow">→</span> {item.selectedFormats.length} format{item.selectedFormats.length > 1 ? 's' : ''}
									{/if}
								</span>
							</div>
							<button class="btn-remove" onclick={() => removeFile(index)} disabled={isConverting}>
								<Icon name="x" size={16} />
							</button>
						</div>

						{#if item.state.status === 'complete' && item.results.length > 0}
							<div class="results">
								{#each item.results as result}
									<button class="result-item" onclick={() => downloadResult(result)}>
										<Icon name="download" size={14} />
										<span>.{result.fileName.split('.').pop()}</span>
									</button>
								{/each}
								{#if item.results.length > 1}
									<button class="result-item all" onclick={() => downloadAllResults(item)}>
										Download all
									</button>
								{/if}
							</div>
						{:else if item.state.status === 'converting'}
							<div class="progress-wrap">
								<ConversionProgress progress={item.state.progress} status={item.state.status} message={item.state.message} />
							</div>
						{:else}
							<div class="format-wrap">
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

			{#if !isConverting && !allComplete}
				<div class="add-section">
					<FileDropzone on:files={handleFiles} compact={true} />
				</div>
			{/if}

			<div class="actions">
				{#if allComplete}
					<button class="btn" onclick={reset}>New conversion</button>
				{:else}
					<button class="btn secondary" onclick={reset} disabled={isConverting}>Clear</button>
					<button class="btn primary" disabled={totalSelectedFormats === 0 || isConverting} onclick={startConversion}>
						{#if isConverting}
							<span class="spinner"></span>
							Converting...
						{:else}
							Convert {totalSelectedFormats > 0 ? `(${totalSelectedFormats})` : ''}
						{/if}
					</button>
				{/if}
			</div>

			<LogPanel {logs} bind:expanded={logExpanded} maxHeight="200px" />
		{/if}
	</div>

	<footer class="footer">
		<span>Local processing only</span>
		<span class="sep">·</span>
		<span>WebCodecs {capabilities.webcodecs ? '✓' : '✗'}</span>
		<span class="sep">·</span>
		<span>FFmpeg {capabilities.sharedArrayBuffer ? '✓' : '✗'}</span>
	</footer>
</main>

<style>
	.app {
		max-width: 640px;
		margin: 0 auto;
		padding: 48px 20px;
		opacity: 0;
		transition: opacity 0.2s;
	}

	.app.mounted {
		opacity: 1;
	}

	.header {
		margin-bottom: 32px;
	}

	.header h1 {
		font-size: 24px;
		font-weight: 600;
		letter-spacing: -0.02em;
		margin-bottom: 4px;
	}

	.desc {
		color: var(--c-text-2);
		font-size: 14px;
	}

	.card {
		background: var(--c-surface);
		border: 1px solid var(--c-border);
		border-radius: var(--radius-lg);
		padding: 20px;
	}

	.file-list {
		display: flex;
		flex-direction: column;
		gap: 16px;
	}

	.file-item {
		background: var(--c-surface-raised);
		border: 1px solid var(--c-border-subtle);
		border-radius: var(--radius);
		padding: 16px;
	}

	.file-item.done {
		border-color: var(--c-success);
		border-color: color-mix(in srgb, var(--c-success) 40%, var(--c-border-subtle));
	}

	.file-header {
		display: flex;
		justify-content: space-between;
		align-items: flex-start;
		gap: 12px;
	}

	.file-info {
		flex: 1;
		min-width: 0;
	}

	.file-name {
		display: block;
		font-weight: 500;
		font-size: 14px;
		white-space: nowrap;
		overflow: hidden;
		text-overflow: ellipsis;
	}

	.file-meta {
		display: block;
		font-size: 12px;
		color: var(--c-text-3);
		margin-top: 2px;
	}

	.arrow {
		color: var(--c-accent);
	}

	.btn-remove {
		background: none;
		border: none;
		color: var(--c-text-3);
		cursor: pointer;
		padding: 4px;
		border-radius: var(--radius-sm);
		transition: color 0.15s, background 0.15s;
	}

	.btn-remove:hover:not(:disabled) {
		color: var(--c-error);
		background: var(--c-error-subtle);
	}

	.btn-remove:disabled {
		opacity: 0.4;
		cursor: not-allowed;
	}

	.format-wrap, .progress-wrap {
		margin-top: 16px;
		padding-top: 16px;
		border-top: 1px solid var(--c-border-subtle);
	}

	.results {
		display: flex;
		flex-wrap: wrap;
		gap: 8px;
		margin-top: 12px;
	}

	.result-item {
		display: inline-flex;
		align-items: center;
		gap: 6px;
		padding: 6px 12px;
		font-size: 13px;
		font-weight: 500;
		background: var(--c-success-subtle);
		color: var(--c-success);
		border: none;
		border-radius: var(--radius-sm);
		cursor: pointer;
		transition: background 0.15s;
	}

	.result-item:hover {
		background: color-mix(in srgb, var(--c-success) 20%, transparent);
	}

	.result-item.all {
		background: var(--c-accent-subtle);
		color: var(--c-accent);
	}

	.result-item.all:hover {
		background: color-mix(in srgb, var(--c-accent) 20%, transparent);
	}

	.add-section {
		margin-top: 16px;
		padding-top: 16px;
		border-top: 1px dashed var(--c-border);
	}

	.actions {
		display: flex;
		gap: 8px;
		margin-top: 20px;
		padding-top: 16px;
		border-top: 1px solid var(--c-border);
	}

	.btn {
		display: inline-flex;
		align-items: center;
		justify-content: center;
		gap: 8px;
		padding: 10px 16px;
		font-size: 14px;
		font-weight: 500;
		border: none;
		border-radius: var(--radius);
		cursor: pointer;
		transition: background 0.15s, opacity 0.15s;
	}

	.btn:disabled {
		opacity: 0.5;
		cursor: not-allowed;
	}

	.btn.primary {
		flex: 1;
		background: var(--c-accent);
		color: white;
	}

	.btn.primary:hover:not(:disabled) {
		background: var(--c-accent-dim);
	}

	.btn.secondary {
		background: var(--c-surface-raised);
		color: var(--c-text);
		border: 1px solid var(--c-border);
	}

	.btn.secondary:hover:not(:disabled) {
		background: var(--c-border-subtle);
	}

	.spinner {
		width: 14px;
		height: 14px;
		border: 2px solid rgba(255,255,255,0.3);
		border-top-color: white;
		border-radius: 50%;
		animation: spin 0.6s linear infinite;
	}

	.footer {
		margin-top: 24px;
		text-align: center;
		font-size: 12px;
		color: var(--c-text-3);
	}

	.sep {
		margin: 0 6px;
		opacity: 0.5;
	}

	@media (max-width: 480px) {
		.app {
			padding: 24px 16px;
		}

		.actions {
			flex-direction: column;
		}

		.btn {
			width: 100%;
		}
	}
</style>
