<script lang="ts">
	interface Props {
		progress: number;
		status: 'idle' | 'loading' | 'converting' | 'complete' | 'error';
		message?: string;
	}

	let { progress = 0, status = 'idle', message = '' }: Props = $props();

	let displayProgress = $derived(Math.min(Math.max(Math.round(progress), 0), 100));
</script>

<div class="progress-container">
	<!-- Status Icon -->
	<div class="status-icon" class:active={status === 'converting' || status === 'loading'}>
		{#if status === 'converting' || status === 'loading'}
			<svg class="icon-spin" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
				<path d="M12 2v4m0 12v4M4.93 4.93l2.83 2.83m8.48 8.48l2.83 2.83M2 12h4m12 0h4M4.93 19.07l2.83-2.83m8.48-8.48l2.83-2.83"/>
			</svg>
		{:else if status === 'complete'}
			<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2.5">
				<polyline points="20 6 9 17 4 12"/>
			</svg>
		{:else}
			<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
				<circle cx="12" cy="12" r="10"/>
			</svg>
		{/if}
	</div>

	<!-- Progress Info -->
	<div class="progress-info">
		<div class="progress-header">
			<span class="progress-message">{message || '処理中...'}</span>
			<span class="progress-percent">{displayProgress}%</span>
		</div>

		<!-- Progress Bar -->
		<div class="progress-bar-container">
			<div class="progress-bar-bg">
				<div
					class="progress-bar-fill"
					class:complete={status === 'complete'}
					class:error={status === 'error'}
					style="width: {displayProgress}%"
				>
					{#if status === 'converting' || status === 'loading'}
						<div class="progress-bar-shimmer"></div>
					{/if}
				</div>
			</div>
		</div>
	</div>
</div>

<style>
	.progress-container {
		display: flex;
		align-items: center;
		gap: 1rem;
	}

	/* Status Icon */
	.status-icon {
		width: 2.5rem;
		height: 2.5rem;
		flex-shrink: 0;
		display: flex;
		align-items: center;
		justify-content: center;
		background: var(--color-bg-tertiary);
		border-radius: 50%;
		color: var(--color-text-secondary);
		transition: all 0.3s ease;
	}

	.status-icon.active {
		background: var(--gradient-primary);
		color: white;
	}

	.status-icon svg {
		width: 1.25rem;
		height: 1.25rem;
	}

	.icon-spin {
		animation: spin 1.5s linear infinite;
	}

	@keyframes spin {
		from { transform: rotate(0deg); }
		to { transform: rotate(360deg); }
	}

	/* Progress Info */
	.progress-info {
		flex: 1;
		min-width: 0;
	}

	.progress-header {
		display: flex;
		justify-content: space-between;
		align-items: baseline;
		margin-bottom: 0.5rem;
	}

	.progress-message {
		font-size: 0.875rem;
		font-weight: 500;
		color: var(--color-text);
		white-space: nowrap;
		overflow: hidden;
		text-overflow: ellipsis;
	}

	.progress-percent {
		font-size: 0.875rem;
		font-weight: 600;
		color: var(--color-primary-light);
		margin-left: 0.5rem;
	}

	/* Progress Bar */
	.progress-bar-container {
		position: relative;
	}

	.progress-bar-bg {
		height: 8px;
		background: var(--color-bg-tertiary);
		border-radius: 4px;
		overflow: hidden;
	}

	.progress-bar-fill {
		height: 100%;
		background: var(--gradient-primary);
		border-radius: 4px;
		transition: width 0.3s ease;
		position: relative;
		overflow: hidden;
	}

	.progress-bar-fill.complete {
		background: linear-gradient(90deg, var(--color-success), #059669);
	}

	.progress-bar-fill.error {
		background: var(--color-error);
	}

	/* Shimmer Effect */
	.progress-bar-shimmer {
		position: absolute;
		top: 0;
		left: 0;
		right: 0;
		bottom: 0;
		background: linear-gradient(
			90deg,
			transparent 0%,
			rgba(255, 255, 255, 0.3) 50%,
			transparent 100%
		);
		animation: shimmer 1.5s ease-in-out infinite;
	}

	@keyframes shimmer {
		0% { transform: translateX(-100%); }
		100% { transform: translateX(100%); }
	}

	/* Mobile */
	@media (max-width: 480px) {
		.status-icon {
			width: 2rem;
			height: 2rem;
		}

		.status-icon svg {
			width: 1rem;
			height: 1rem;
		}

		.progress-message {
			font-size: 0.8rem;
		}
	}
</style>
