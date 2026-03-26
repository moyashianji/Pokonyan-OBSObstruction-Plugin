<script lang="ts">
	interface Props {
		progress: number;
		status: 'idle' | 'loading' | 'converting' | 'complete' | 'error';
		message?: string;
	}

	let { progress = 0, status = 'idle', message = '' }: Props = $props();
	let displayProgress = $derived(Math.min(Math.max(Math.round(progress), 0), 100));
</script>

<div class="progress">
	<div class="info">
		<span class="message">{message || 'Processing...'}</span>
		<span class="percent">{displayProgress}%</span>
	</div>
	<div class="bar">
		<div
			class="fill"
			class:complete={status === 'complete'}
			class:error={status === 'error'}
			style="width: {displayProgress}%"
		></div>
	</div>
</div>

<style>
	.progress {
		display: flex;
		flex-direction: column;
		gap: 8px;
	}

	.info {
		display: flex;
		justify-content: space-between;
		align-items: baseline;
	}

	.message {
		font-size: 13px;
		color: var(--c-text-2);
	}

	.percent {
		font-size: 12px;
		font-weight: 500;
		color: var(--c-accent);
	}

	.bar {
		height: 4px;
		background: var(--c-border);
		border-radius: 2px;
		overflow: hidden;
	}

	.fill {
		height: 100%;
		background: var(--c-accent);
		border-radius: 2px;
		transition: width 0.2s ease;
	}

	.fill.complete {
		background: var(--c-success);
	}

	.fill.error {
		background: var(--c-error);
	}
</style>
