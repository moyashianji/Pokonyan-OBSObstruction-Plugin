<script lang="ts">
	import { createEventDispatcher } from 'svelte';

	interface Props {
		accept?: string;
		multiple?: boolean;
		disabled?: boolean;
	}

	let { accept = '*', multiple = true, disabled = false }: Props = $props();

	const dispatch = createEventDispatcher<{ files: File[] }>();

	function handleChange(e: Event) {
		const input = e.target as HTMLInputElement;
		if (input.files && input.files.length > 0) {
			dispatch('files', Array.from(input.files));
			input.value = '';
		}
	}
</script>

<div class="upload-container">
	<p class="upload-text">ファイルを選択してください</p>
	<input
		type="file"
		accept={accept}
		multiple={multiple}
		onchange={handleChange}
		disabled={disabled}
		class="file-input"
	/>
	<p class="upload-hint">動画・音声・画像・ドキュメントに対応</p>
</div>

<style>
	.upload-container {
		display: flex;
		flex-direction: column;
		align-items: center;
		gap: 1.5rem;
		padding: 2rem;
		border: 2px dashed var(--color-border, #475569);
		border-radius: 1rem;
		background: rgba(255, 255, 255, 0.02);
	}

	.upload-text {
		font-size: 1.1rem;
		color: var(--color-text, #f8fafc);
	}

	.file-input {
		font-size: 1rem;
		color: var(--color-text, #f8fafc);
		padding: 1rem;
		background: var(--color-primary, #6366f1);
		border: none;
		border-radius: 0.5rem;
		cursor: pointer;
	}

	.file-input::file-selector-button {
		padding: 0.75rem 1.5rem;
		margin-right: 1rem;
		background: white;
		color: var(--color-primary, #6366f1);
		border: none;
		border-radius: 0.5rem;
		font-weight: 600;
		cursor: pointer;
	}

	.upload-hint {
		font-size: 0.875rem;
		color: var(--color-text-secondary, #94a3b8);
	}
</style>
