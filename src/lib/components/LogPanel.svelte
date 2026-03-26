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
	interface Props {
		logs: LogEntry[];
		expanded?: boolean;
		maxHeight?: string;
	}

	let { logs = [], expanded = $bindable(false), maxHeight = '200px' }: Props = $props();
	let container: HTMLDivElement | null = $state(null);

	$effect(() => {
		if (logs.length && container) {
			container.scrollTop = container.scrollHeight;
		}
	});

	function formatTime(date: Date): string {
		return date.toLocaleTimeString('en-US', { hour12: false, hour: '2-digit', minute: '2-digit', second: '2-digit' });
	}

	function formatValue(key: string, value: string | number): string {
		if (typeof value === 'number') {
			if (key === 'in' || key === 'out' || key === 'size') {
				if (value < 1024) return `${value}B`;
				if (value < 1024 * 1024) return `${(value / 1024).toFixed(1)}KB`;
				return `${(value / (1024 * 1024)).toFixed(2)}MB`;
			}
			if (key === 'time' || key === 'total' || key === 'elapsed') {
				return value < 1000 ? `${Math.round(value)}ms` : `${(value / 1000).toFixed(2)}s`;
			}
		}
		return String(value);
	}

	const levelColors: Record<string, string> = {
		info: 'var(--c-text-3)',
		success: 'var(--c-success)',
		warning: 'var(--c-warning)',
		error: 'var(--c-error)',
		debug: 'var(--c-text-3)'
	};
</script>

<div class="panel">
	<button class="toggle" onclick={() => expanded = !expanded}>
		<span>Log ({logs.length})</span>
		<span class="arrow" class:open={expanded}>▼</span>
	</button>

	{#if expanded}
		<div class="content" bind:this={container} style="max-height: {maxHeight}">
			{#each logs as log (log.id)}
				<div class="entry">
					<span class="time">{formatTime(log.timestamp)}</span>
					<span class="msg" style="color: {levelColors[log.level]}">{log.message}</span>
					{#if log.details}
						<span class="details">
							{#each Object.entries(log.details) as [k, v]}
								<span class="detail">{k}={formatValue(k, v)}</span>
							{/each}
						</span>
					{/if}
				</div>
			{/each}
			{#if logs.length === 0}
				<div class="empty">No logs yet</div>
			{/if}
		</div>
	{/if}
</div>

<style>
	.panel {
		margin-top: 16px;
		border: 1px solid var(--c-border-subtle);
		border-radius: var(--radius-sm);
		overflow: hidden;
	}

	.toggle {
		display: flex;
		justify-content: space-between;
		align-items: center;
		width: 100%;
		padding: 8px 12px;
		font-size: 12px;
		font-weight: 500;
		color: var(--c-text-3);
		background: var(--c-surface-raised);
		border: none;
		cursor: pointer;
	}

	.toggle:hover {
		color: var(--c-text-2);
	}

	.arrow {
		font-size: 10px;
		transition: transform 0.15s;
	}

	.arrow.open {
		transform: rotate(180deg);
	}

	.content {
		overflow-y: auto;
		font-family: ui-monospace, 'SF Mono', Consolas, monospace;
		font-size: 11px;
		line-height: 1.6;
		background: var(--c-bg);
	}

	.entry {
		display: flex;
		flex-wrap: wrap;
		gap: 8px;
		padding: 4px 12px;
		border-bottom: 1px solid var(--c-border-subtle);
	}

	.entry:last-child {
		border-bottom: none;
	}

	.time {
		color: var(--c-text-3);
	}

	.msg {
		flex: 1;
	}

	.details {
		width: 100%;
		display: flex;
		gap: 12px;
		padding-left: 64px;
		color: var(--c-text-3);
	}

	.detail {
		font-size: 10px;
	}

	.empty {
		padding: 16px;
		text-align: center;
		color: var(--c-text-3);
	}
</style>
