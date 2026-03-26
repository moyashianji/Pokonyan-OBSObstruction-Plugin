<script lang="ts" module>
	export interface LogEntry {
		id: number;
		timestamp: Date;
		level: 'info' | 'success' | 'warning' | 'error' | 'debug';
		message: string;
		details?: Record<string, string | number>;
	}
</script>

<script lang="ts">
	import Icon from './Icon.svelte';

	interface Props {
		logs: LogEntry[];
		expanded?: boolean;
		maxHeight?: string;
	}

	let { logs = [], expanded = $bindable(false), maxHeight = '300px' }: Props = $props();

	let logContainer: HTMLDivElement | null = $state(null);

	// Auto-scroll to bottom when new logs arrive
	$effect(() => {
		if (logs.length && logContainer) {
			logContainer.scrollTop = logContainer.scrollHeight;
		}
	});

	const levelConfig = {
		info: { icon: 'clock', color: 'var(--color-text-secondary)', bg: 'transparent' },
		success: { icon: 'check', color: 'var(--color-success)', bg: 'rgba(16, 185, 129, 0.1)' },
		warning: { icon: 'zap', color: '#f59e0b', bg: 'rgba(245, 158, 11, 0.1)' },
		error: { icon: 'x', color: 'var(--color-error)', bg: 'rgba(239, 68, 68, 0.1)' },
		debug: { icon: 'settings', color: 'var(--color-text-muted)', bg: 'transparent' }
	};

	function formatTime(date: Date): string {
		return date.toLocaleTimeString('ja-JP', {
			hour: '2-digit',
			minute: '2-digit',
			second: '2-digit',
			fractionalSecondDigits: 3
		});
	}

	function formatValue(key: string, value: string | number): string {
		if (key.includes('size') || key.includes('Size')) {
			const bytes = typeof value === 'number' ? value : parseInt(value as string);
			if (bytes < 1024) return `${bytes} B`;
			if (bytes < 1024 * 1024) return `${(bytes / 1024).toFixed(1)} KB`;
			return `${(bytes / (1024 * 1024)).toFixed(2)} MB`;
		}
		if (key.includes('time') || key.includes('Time') || key.includes('duration')) {
			const ms = typeof value === 'number' ? value : parseFloat(value as string);
			if (ms < 1000) return `${ms.toFixed(0)}ms`;
			return `${(ms / 1000).toFixed(2)}s`;
		}
		if (key.includes('ratio') || key.includes('Ratio')) {
			return `${((value as number) * 100).toFixed(1)}%`;
		}
		return String(value);
	}
</script>

<div class="log-panel" class:expanded>
	<button class="log-header" onclick={() => expanded = !expanded}>
		<div class="header-left">
			<Icon name="layers" size={14} />
			<span class="header-title">詳細ログ</span>
			<span class="log-count">{logs.length}</span>
		</div>
		<div class="header-right">
			<span class="expand-hint">{expanded ? '折りたたむ' : '展開'}</span>
			<span class="expand-icon" class:rotated={expanded}>
				<Icon name="arrow-down" size={14} />
			</span>
		</div>
	</button>

	{#if expanded}
		<div class="log-container" bind:this={logContainer} style="max-height: {maxHeight}">
			{#if logs.length === 0}
				<div class="log-empty">
					<Icon name="clock" size={20} />
					<span>ログはまだありません</span>
				</div>
			{:else}
				{#each logs as log (log.id)}
					{@const config = levelConfig[log.level]}
					<div class="log-entry" style="background: {config.bg}">
						<span class="log-time">{formatTime(log.timestamp)}</span>
						<span class="log-icon" style="color: {config.color}">
							<Icon name={config.icon} size={12} />
						</span>
						<span class="log-message" style="color: {config.color}">{log.message}</span>
						{#if log.details}
							<div class="log-details">
								{#each Object.entries(log.details) as [key, value]}
									<span class="detail-item">
										<span class="detail-key">{key}:</span>
										<span class="detail-value">{formatValue(key, value)}</span>
									</span>
								{/each}
							</div>
						{/if}
					</div>
				{/each}
			{/if}
		</div>
	{/if}
</div>

<style>
	.log-panel {
		border: 1px solid var(--color-border);
		border-radius: var(--radius-md);
		background: var(--color-bg-tertiary);
		overflow: hidden;
		transition: all 0.2s ease;
	}

	.log-panel.expanded {
		border-color: var(--color-primary);
	}

	.log-header {
		display: flex;
		align-items: center;
		justify-content: space-between;
		width: 100%;
		padding: 0.6rem 0.75rem;
		background: transparent;
		border: none;
		cursor: pointer;
		color: var(--color-text);
		transition: background 0.15s ease;
	}

	.log-header:hover {
		background: rgba(99, 102, 241, 0.05);
	}

	.header-left {
		display: flex;
		align-items: center;
		gap: 0.5rem;
	}

	.header-title {
		font-size: 0.8rem;
		font-weight: 600;
	}

	.log-count {
		font-size: 0.65rem;
		font-weight: 500;
		color: var(--color-text-muted);
		background: var(--color-bg-secondary);
		padding: 0.1rem 0.4rem;
		border-radius: 1rem;
	}

	.header-right {
		display: flex;
		align-items: center;
		gap: 0.35rem;
	}

	.expand-hint {
		font-size: 0.7rem;
		color: var(--color-text-muted);
	}

	.expand-icon {
		transition: transform 0.2s ease;
	}

	.expand-icon.rotated {
		transform: rotate(180deg);
	}

	.log-container {
		overflow-y: auto;
		border-top: 1px solid var(--color-border);
		font-family: 'SF Mono', 'Consolas', 'Monaco', monospace;
		font-size: 0.7rem;
	}

	.log-empty {
		display: flex;
		align-items: center;
		justify-content: center;
		gap: 0.5rem;
		padding: 1.5rem;
		color: var(--color-text-muted);
		font-size: 0.75rem;
	}

	.log-entry {
		display: flex;
		flex-wrap: wrap;
		align-items: flex-start;
		gap: 0.4rem;
		padding: 0.35rem 0.6rem;
		border-bottom: 1px solid var(--color-border);
		animation: fadeIn 0.15s ease;
	}

	.log-entry:last-child {
		border-bottom: none;
	}

	@keyframes fadeIn {
		from { opacity: 0; transform: translateY(-4px); }
		to { opacity: 1; transform: translateY(0); }
	}

	.log-time {
		color: var(--color-text-muted);
		font-size: 0.65rem;
		flex-shrink: 0;
	}

	.log-icon {
		flex-shrink: 0;
		display: flex;
		align-items: center;
	}

	.log-message {
		flex: 1;
		min-width: 0;
		word-break: break-word;
	}

	.log-details {
		width: 100%;
		display: flex;
		flex-wrap: wrap;
		gap: 0.5rem;
		margin-top: 0.25rem;
		padding-left: 4.5rem;
	}

	.detail-item {
		display: inline-flex;
		gap: 0.25rem;
		background: var(--color-bg-secondary);
		padding: 0.15rem 0.4rem;
		border-radius: var(--radius-sm);
		font-size: 0.6rem;
	}

	.detail-key {
		color: var(--color-text-muted);
	}

	.detail-value {
		color: var(--color-primary-light);
		font-weight: 500;
	}

	/* Scrollbar styling */
	.log-container::-webkit-scrollbar {
		width: 6px;
	}

	.log-container::-webkit-scrollbar-track {
		background: transparent;
	}

	.log-container::-webkit-scrollbar-thumb {
		background: var(--color-border);
		border-radius: 3px;
	}

	.log-container::-webkit-scrollbar-thumb:hover {
		background: var(--color-text-muted);
	}

	/* Mobile */
	@media (max-width: 480px) {
		.log-details {
			padding-left: 0;
		}

		.log-time {
			font-size: 0.6rem;
		}
	}
</style>
