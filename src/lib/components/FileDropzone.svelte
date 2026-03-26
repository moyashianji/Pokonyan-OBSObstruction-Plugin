<script lang="ts">
	import { createEventDispatcher } from 'svelte';

	interface Props {
		accept?: string;
		multiple?: boolean;
		disabled?: boolean;
	}

	let { accept = '*', multiple = true, disabled = false }: Props = $props();

	let isDragging = $state(false);
	let fileInputEl: HTMLInputElement;

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

	function triggerFileSelect() {
		if (!disabled && fileInputEl) {
			fileInputEl.click();
		}
	}
</script>

<div
	class="dropzone-wrapper"
	class:dragging={isDragging}
	class:disabled={disabled}
	ondragover={handleDragOver}
	ondragleave={handleDragLeave}
	ondrop={handleDrop}
>
	<input
		bind:this={fileInputEl}
		type="file"
		accept={accept}
		multiple={multiple}
		onchange={handleChange}
		disabled={disabled}
		style="display: none;"
	/>

	<button
		type="button"
		class="dropzone-button"
		onclick={triggerFileSelect}
		disabled={disabled}
	>
		<svg class="upload-icon" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
			<path d="M21 15v4a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2v-4" />
			<polyline points="17 8 12 3 7 8" />
			<line x1="12" y1="3" x2="12" y2="15" />
		</svg>
		<p class="dropzone-text">
			ファイルをドラッグ&ドロップ<br />
			または<span class="highlight">タップして選択</span>
		</p>
		<p class="dropzone-hint">
			動画・音声・画像・ドキュメントに対応
		</p>
	</button>
</div>

<style>
	.dropzone-wrapper {
		width: 100%;
	}

	.dropzone-button {
		display: flex;
		flex-direction: column;
		align-items: center;
		justify-content: center;
		gap: 1rem;
		width: 100%;
		min-height: 200px;
		padding: 2rem;
		border: 2px dashed var(--color-border, #475569);
		border-radius: 1rem;
		background: rgba(255, 255, 255, 0.02);
		cursor: pointer;
		transition: all 0.3s ease;
		-webkit-tap-highlight-color: rgba(99, 102, 241, 0.3);
		touch-action: manipulation;
	}

	.dropzone-button:hover,
	.dropzone-wrapper.dragging .dropzone-button {
		border-color: var(--color-primary, #6366f1);
		background: rgba(99, 102, 241, 0.1);
	}

	.dropzone-button:active {
		transform: scale(0.98);
	}

	.dropzone-button:disabled {
		opacity: 0.5;
		cursor: not-allowed;
	}

	.upload-icon {
		width: 4rem;
		height: 4rem;
		color: var(--color-primary, #6366f1);
	}

	.dropzone-text {
		font-size: 1.1rem;
		color: var(--color-text, #f8fafc);
		line-height: 1.6;
		text-align: center;
	}

	.highlight {
		color: var(--color-primary, #6366f1);
		font-weight: 600;
	}

	.dropzone-hint {
		font-size: 0.875rem;
		color: var(--color-text-secondary, #94a3b8);
		text-align: center;
	}
</style>
