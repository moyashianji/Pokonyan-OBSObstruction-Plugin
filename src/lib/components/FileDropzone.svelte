<script lang="ts">
	import { createEventDispatcher } from 'svelte';
	import Icon from './Icon.svelte';

	interface Props {
		accept?: string;
		multiple?: boolean;
		disabled?: boolean;
		compact?: boolean;
	}

	let { accept = '*', multiple = true, disabled = false, compact = false }: Props = $props();

	let isDragging = $state(false);
	let isHovering = $state(false);

	const dispatch = createEventDispatcher<{ files: File[] }>();

	function handleDragOver(e: DragEvent) {
		e.preventDefault();
		if (!disabled) isDragging = true;
	}

	function handleDragLeave(e: DragEvent) {
		e.preventDefault();
		isDragging = false;
	}

	function handleDrop(e: DragEvent) {
		e.preventDefault();
		isDragging = false;
		if (disabled || !e.dataTransfer) return;

		const files = Array.from(e.dataTransfer.files);
		if (files.length > 0) {
			dispatch('files', files);
		}
	}

	function handleChange(e: Event) {
		const input = e.target as HTMLInputElement;
		if (input.files && input.files.length > 0) {
			dispatch('files', Array.from(input.files));
			input.value = '';
		}
	}
</script>

<div
	class="dropzone-wrapper"
	class:dragging={isDragging}
	class:hovering={isHovering}
	class:disabled={disabled}
	class:compact
	role="button"
	tabindex="0"
	ondragover={handleDragOver}
	ondragleave={handleDragLeave}
	ondrop={handleDrop}
	onmouseenter={() => !disabled && (isHovering = true)}
	onmouseleave={() => isHovering = false}
>
	<input
		type="file"
		id="file-upload-input-{compact ? 'compact' : 'main'}"
		accept={accept}
		multiple={multiple}
		onchange={handleChange}
		disabled={disabled}
		class="file-input"
	/>

	<label for="file-upload-input-{compact ? 'compact' : 'main'}" class="dropzone-label">
		<div class="dropzone-content">
			{#if compact}
				<!-- Compact Mode -->
				<div class="compact-content">
					<Icon name="plus" size={20} class="add-icon" />
					<span class="compact-text">ファイルを追加</span>
				</div>
			{:else}
				<!-- Full Mode -->
				<!-- Animated Icon -->
				<div class="icon-wrapper">
					<div class="icon-bg"></div>
					<Icon name="upload" size={40} class="upload-icon" />
				</div>

				<!-- Text -->
				<div class="dropzone-text">
					<p class="primary-text">
						ファイルをドロップ
					</p>
					<p class="secondary-text">
						または<span class="highlight">クリックして選択</span>
					</p>
				</div>

				<!-- Supported formats -->
				<div class="format-chips">
					<span class="chip">
						<Icon name="video" size={12} />
						<span>動画</span>
					</span>
					<span class="chip">
						<Icon name="audio" size={12} />
						<span>音声</span>
					</span>
					<span class="chip">
						<Icon name="image" size={12} />
						<span>画像</span>
					</span>
				</div>
			{/if}
		</div>
	</label>
</div>

<style>
	.dropzone-wrapper {
		position: relative;
		width: 100%;
	}

	.file-input {
		position: absolute;
		width: 100%;
		height: 100%;
		top: 0;
		left: 0;
		opacity: 0;
		cursor: pointer;
		z-index: 10;
	}

	.dropzone-label {
		display: block;
		position: relative;
		padding: 3rem 2rem;
		border: 2px dashed var(--color-border);
		border-radius: var(--radius-lg);
		background: var(--color-bg-glass);
		cursor: pointer;
		transition: all 0.3s ease;
		overflow: hidden;
	}

	/* Compact mode */
	.compact .dropzone-label {
		padding: 1rem 1.5rem;
		border-style: dashed;
		border-width: 1px;
	}

	.compact-content {
		display: flex;
		align-items: center;
		justify-content: center;
		gap: 0.5rem;
		color: var(--color-text-secondary);
	}

	.compact-text {
		font-size: 0.875rem;
		font-weight: 500;
	}

	.compact .dropzone-label:hover .compact-content {
		color: var(--color-primary);
	}

	/* Glow effect on hover/drag */
	.dropzone-label::before {
		content: '';
		position: absolute;
		top: 50%;
		left: 50%;
		width: 100%;
		height: 100%;
		background: radial-gradient(circle, rgba(99, 102, 241, 0.1) 0%, transparent 70%);
		transform: translate(-50%, -50%) scale(0);
		transition: transform 0.4s ease;
		pointer-events: none;
	}

	.dropzone-wrapper.hovering .dropzone-label::before,
	.dropzone-wrapper.dragging .dropzone-label::before {
		transform: translate(-50%, -50%) scale(2);
	}

	.dropzone-wrapper.hovering .dropzone-label,
	.dropzone-wrapper.dragging .dropzone-label {
		border-color: var(--color-primary);
		background: rgba(99, 102, 241, 0.05);
	}

	.dropzone-wrapper.dragging .dropzone-label {
		border-style: solid;
		transform: scale(1.01);
	}

	.dropzone-wrapper.disabled .dropzone-label {
		opacity: 0.5;
		cursor: not-allowed;
	}

	.dropzone-content {
		display: flex;
		flex-direction: column;
		align-items: center;
		gap: 1.25rem;
		position: relative;
		z-index: 1;
	}

	/* Icon */
	.icon-wrapper {
		position: relative;
		width: 4.5rem;
		height: 4.5rem;
		display: flex;
		align-items: center;
		justify-content: center;
	}

	.icon-bg {
		position: absolute;
		inset: 0;
		background: var(--gradient-primary);
		border-radius: 50%;
		opacity: 0.15;
		transition: all 0.3s ease;
	}

	.dropzone-wrapper.hovering .icon-bg,
	.dropzone-wrapper.dragging .icon-bg {
		opacity: 0.25;
		transform: scale(1.1);
	}

	.icon-wrapper :global(.upload-icon) {
		color: var(--color-primary);
		transition: transform 0.3s ease;
	}

	.dropzone-wrapper.hovering .icon-wrapper :global(.upload-icon),
	.dropzone-wrapper.dragging .icon-wrapper :global(.upload-icon) {
		transform: translateY(-4px);
	}

	/* Text */
	.dropzone-text {
		text-align: center;
	}

	.primary-text {
		font-size: 1.125rem;
		font-weight: 600;
		color: var(--color-text);
		margin-bottom: 0.25rem;
	}

	.secondary-text {
		font-size: 0.875rem;
		color: var(--color-text-secondary);
	}

	.highlight {
		color: var(--color-primary-light);
		font-weight: 500;
	}

	/* Format chips */
	.format-chips {
		display: flex;
		gap: 0.5rem;
		flex-wrap: wrap;
		justify-content: center;
	}

	.chip {
		display: flex;
		align-items: center;
		gap: 0.35rem;
		padding: 0.35rem 0.75rem;
		font-size: 0.75rem;
		font-weight: 500;
		color: var(--color-text-secondary);
		background: var(--color-bg-tertiary);
		border-radius: 2rem;
		border: 1px solid var(--color-border);
		transition: all 0.2s ease;
	}

	.dropzone-wrapper.hovering .chip,
	.dropzone-wrapper.dragging .chip {
		border-color: var(--color-primary);
		color: var(--color-text);
	}

	/* Mobile */
	@media (max-width: 640px) {
		.dropzone-label {
			padding: 2rem 1.5rem;
		}

		.icon-wrapper {
			width: 3.5rem;
			height: 3.5rem;
		}

		.primary-text {
			font-size: 1rem;
		}

		.chip {
			padding: 0.25rem 0.5rem;
			font-size: 0.7rem;
		}

		.compact .dropzone-label {
			padding: 0.75rem 1rem;
		}
	}
</style>
