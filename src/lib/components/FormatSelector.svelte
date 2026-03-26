<script lang="ts">
	import { createEventDispatcher } from 'svelte';

	interface FormatOption {
		value: string;
		label: string;
		category: 'video' | 'audio' | 'image' | 'document';
	}

	interface Props {
		selectedFormat?: string;
		fileType?: 'video' | 'audio' | 'image' | 'document' | null;
	}

	let { selectedFormat = '', fileType = null }: Props = $props();

	const dispatch = createEventDispatcher<{ select: string }>();

	const formats: FormatOption[] = [
		// Video
		{ value: 'mp4', label: 'MP4', category: 'video' },
		{ value: 'webm', label: 'WebM', category: 'video' },
		{ value: 'avi', label: 'AVI', category: 'video' },
		{ value: 'mov', label: 'MOV', category: 'video' },
		{ value: 'mkv', label: 'MKV', category: 'video' },
		{ value: 'gif', label: 'GIF (アニメーション)', category: 'video' },
		// Audio
		{ value: 'mp3', label: 'MP3', category: 'audio' },
		{ value: 'wav', label: 'WAV', category: 'audio' },
		{ value: 'ogg', label: 'OGG', category: 'audio' },
		{ value: 'aac', label: 'AAC', category: 'audio' },
		{ value: 'flac', label: 'FLAC', category: 'audio' },
		{ value: 'm4a', label: 'M4A', category: 'audio' },
		// Image
		{ value: 'png', label: 'PNG', category: 'image' },
		{ value: 'jpg', label: 'JPG', category: 'image' },
		{ value: 'webp', label: 'WebP', category: 'image' },
		{ value: 'bmp', label: 'BMP', category: 'image' },
	];

	const categoryLabels = {
		video: '動画',
		audio: '音声',
		image: '画像',
		document: 'ドキュメント'
	};

	let filteredFormats = $derived(
		fileType ? formats.filter(f => f.category === fileType) : formats
	);

	let groupedFormats = $derived(
		filteredFormats.reduce((acc, format) => {
			if (!acc[format.category]) {
				acc[format.category] = [];
			}
			acc[format.category].push(format);
			return acc;
		}, {} as Record<string, FormatOption[]>)
	);

	function selectFormat(format: string) {
		dispatch('select', format);
	}
</script>

<div class="format-selector">
	<h3 class="selector-title">出力形式を選択</h3>

	{#each Object.entries(groupedFormats) as [category, categoryFormats]}
		<div class="format-category">
			<h4 class="category-label">{categoryLabels[category as keyof typeof categoryLabels]}</h4>
			<div class="format-grid">
				{#each categoryFormats as format}
					<button
						class="format-button"
						class:selected={selectedFormat === format.value}
						onclick={() => selectFormat(format.value)}
					>
						{format.label}
					</button>
				{/each}
			</div>
		</div>
	{/each}
</div>

<style>
	.format-selector {
		width: 100%;
	}

	.selector-title {
		font-size: 1rem;
		font-weight: 600;
		color: var(--color-text);
		margin-bottom: 1rem;
	}

	.format-category {
		margin-bottom: 1.5rem;
	}

	.category-label {
		font-size: 0.75rem;
		font-weight: 500;
		color: var(--color-text-secondary);
		text-transform: uppercase;
		letter-spacing: 0.05em;
		margin-bottom: 0.5rem;
	}

	.format-grid {
		display: flex;
		flex-wrap: wrap;
		gap: 0.5rem;
	}

	.format-button {
		padding: 0.5rem 1rem;
		border: 1px solid var(--color-border);
		border-radius: 0.5rem;
		background: var(--color-bg-secondary);
		color: var(--color-text);
		font-size: 0.875rem;
		font-weight: 500;
		cursor: pointer;
		transition: all 0.2s ease;
	}

	.format-button:hover {
		border-color: var(--color-primary);
		background: rgba(99, 102, 241, 0.1);
	}

	.format-button.selected {
		border-color: var(--color-primary);
		background: var(--color-primary);
		color: white;
	}
</style>
