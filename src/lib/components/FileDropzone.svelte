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
		if (files.length > 0) dispatch('files', files);
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
	class="dropzone"
	class:dragging={isDragging}
	class:disabled
	class:compact
	role="button"
	tabindex="0"
	ondragover={handleDragOver}
	ondragleave={handleDragLeave}
	ondrop={handleDrop}
>
	<input
		type="file"
		id="file-input-{compact ? 'compact' : 'main'}"
		{accept}
		{multiple}
		onchange={handleChange}
		{disabled}
		class="input"
	/>

	<label for="file-input-{compact ? 'compact' : 'main'}" class="label">
		{#if compact}
			<span class="compact-text">+ Add files</span>
		{:else}
			<Icon name="upload" size={24} />
			<span class="main-text">Drop files here or click to select</span>
			<span class="hint">Video, audio, or images</span>
		{/if}
	</label>
</div>

<style>
	.dropzone {
		position: relative;
	}

	.input {
		position: absolute;
		inset: 0;
		opacity: 0;
		cursor: pointer;
	}

	.input:disabled {
		cursor: not-allowed;
	}

	.label {
		display: flex;
		flex-direction: column;
		align-items: center;
		justify-content: center;
		gap: 8px;
		padding: 40px 20px;
		border: 1px dashed var(--c-border);
		border-radius: var(--radius);
		color: var(--c-text-3);
		transition: all 0.15s;
		cursor: pointer;
	}

	.dropzone:hover:not(.disabled) .label,
	.dropzone.dragging .label {
		border-color: var(--c-accent);
		color: var(--c-text-2);
		background: var(--c-accent-subtle);
	}

	.dropzone.disabled .label {
		opacity: 0.5;
		cursor: not-allowed;
	}

	.main-text {
		font-size: 14px;
		font-weight: 500;
	}

	.hint {
		font-size: 12px;
	}

	/* Compact mode */
	.compact .label {
		padding: 12px 16px;
		flex-direction: row;
	}

	.compact-text {
		font-size: 13px;
		font-weight: 500;
	}
</style>
