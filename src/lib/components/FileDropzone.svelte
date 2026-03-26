<script lang="ts">
	import { createEventDispatcher } from 'svelte';

	interface Props {
		accept?: string;
		multiple?: boolean;
		disabled?: boolean;
	}

	let { accept = '*', multiple = true, disabled = false }: Props = $props();

	let isDragging = $state(false);

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

	function handleFileInput(e: Event) {
		const target = e.target as HTMLInputElement;
		if (target.files && target.files.length > 0) {
			dispatch('files', Array.from(target.files));
			target.value = '';
		}
	}
</script>

<label
	class="dropzone"
	class:dragging={isDragging}
	class:disabled={disabled}
	ondragover={handleDragOver}
	ondragleave={handleDragLeave}
	ondrop={handleDrop}
>
	<input
		type="file"
		{accept}
		{multiple}
		onchange={handleFileInput}
		class="file-input"
		disabled={disabled}
	/>
	<div class="dropzone-content">
		<svg class="upload-icon" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
			<path d="M21 15v4a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2v-4" />
			<polyline points="17 8 12 3 7 8" />
			<line x1="12" y1="3" x2="12" y2="15" />
		</svg>
		<p class="dropzone-text">
			ファイルをドラッグ&ドロップ<br />
			または<span class="highlight">クリックして選択</span>
		</p>
		<p class="dropzone-hint">
			動画・音声・画像・ドキュメントに対応
		</p>
	</div>
</label>

<style>
	.file-input {
		position: absolute;
		width: 100%;
		height: 100%;
		top: 0;
		left: 0;
		opacity: 0;
		cursor: pointer;
	}

	.dropzone {
		position: relative;
		display: block;
		border: 2px dashed var(--color-border);
		border-radius: 1rem;
		padding: 3rem 2rem;
		text-align: center;
		cursor: pointer;
		transition: all 0.3s ease;
		background: rgba(255, 255, 255, 0.02);
	}

	.dropzone:hover,
	.dropzone.dragging {
		border-color: var(--color-primary);
		background: rgba(99, 102, 241, 0.1);
		transform: scale(1.01);
	}

	.dropzone.disabled {
		opacity: 0.5;
		cursor: not-allowed;
	}

	.dropzone.disabled:hover {
		transform: none;
		border-color: var(--color-border);
		background: rgba(255, 255, 255, 0.02);
	}

	.dropzone-content {
		display: flex;
		flex-direction: column;
		align-items: center;
		gap: 1rem;
	}

	.upload-icon {
		width: 4rem;
		height: 4rem;
		color: var(--color-primary);
	}

	.dropzone-text {
		font-size: 1.1rem;
		color: var(--color-text);
		line-height: 1.6;
	}

	.highlight {
		color: var(--color-primary);
		font-weight: 600;
	}

	.dropzone-hint {
		font-size: 0.875rem;
		color: var(--color-text-secondary);
	}
</style>
