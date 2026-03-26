<script lang="ts">
	interface Props {
		progress: number;
		status: 'idle' | 'loading' | 'converting' | 'complete' | 'error';
		message?: string;
	}

	let { progress = 0, status = 'idle', message = '' }: Props = $props();

	const statusMessages = {
		idle: '待機中',
		loading: 'FFmpegを読み込み中...',
		converting: '変換中...',
		complete: '完了!',
		error: 'エラーが発生しました'
	};

	const statusColors = {
		idle: 'var(--color-text-secondary)',
		loading: 'var(--color-accent)',
		converting: 'var(--color-primary)',
		complete: 'var(--color-success)',
		error: 'var(--color-error)'
	};
</script>

<div class="progress-container">
	<div class="progress-header">
		<span class="status-text" style="color: {statusColors[status]}">
			{message || statusMessages[status]}
		</span>
		<span class="progress-percent">{Math.round(progress)}%</span>
	</div>

	<div class="progress-bar">
		<div
			class="progress-fill"
			class:complete={status === 'complete'}
			class:error={status === 'error'}
			style="width: {progress}%"
		></div>
	</div>
</div>

<style>
	.progress-container {
		width: 100%;
	}

	.progress-header {
		display: flex;
		justify-content: space-between;
		align-items: center;
		margin-bottom: 0.5rem;
	}

	.status-text {
		font-size: 0.875rem;
		font-weight: 500;
		transition: color 0.3s ease;
	}

	.progress-percent {
		font-size: 0.875rem;
		font-weight: 600;
		color: var(--color-text);
	}

	.progress-bar {
		height: 8px;
		background: var(--color-bg-tertiary);
		border-radius: 4px;
		overflow: hidden;
	}

	.progress-fill {
		height: 100%;
		background: linear-gradient(90deg, var(--color-primary), var(--color-secondary));
		border-radius: 4px;
		transition: width 0.3s ease;
	}

	.progress-fill.complete {
		background: var(--color-success);
	}

	.progress-fill.error {
		background: var(--color-error);
	}
</style>
