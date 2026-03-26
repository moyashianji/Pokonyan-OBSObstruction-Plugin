<script lang="ts">
	import { createEventDispatcher } from 'svelte';
	import { getOutputFormats } from '$lib/converter';
	import { VIDEO_FORMATS, AUDIO_FORMATS, IMAGE_FORMATS, type FormatInfo } from '$lib/formats';

	interface FormatOption {
		value: string;
		label: string;
		description?: string;
		category: 'video' | 'audio' | 'image';
	}

	interface Props {
		selectedFormat?: string;
		fileType?: 'video' | 'audio' | 'image' | 'document' | null;
	}

	let { selectedFormat = '', fileType = null }: Props = $props();

	const dispatch = createEventDispatcher<{ select: string }>();

	const allFormats: Record<string, FormatInfo> = {
		...VIDEO_FORMATS,
		...AUDIO_FORMATS,
		...IMAGE_FORMATS,
		txt: { label: 'TXT', description: 'プレーンテキスト', category: 'image', mimeType: 'text/plain' }
	};

	const categoryLabels = {
		video: '動画形式',
		audio: '音声形式',
		image: '画像形式'
	};

	const categoryOrder = ['video', 'audio', 'image'];

	let availableFormats = $derived.by(() => {
		if (!fileType) return [];

		const formats = getOutputFormats(fileType);
		return formats.map(f => ({
			value: f,
			label: allFormats[f]?.label || f.toUpperCase(),
			description: allFormats[f]?.description || '',
			category: allFormats[f]?.category || 'image'
		}));
	});

	let groupedFormats = $derived.by(() => {
		const groups: Record<string, FormatOption[]> = {};

		for (const format of availableFormats) {
			if (!groups[format.category]) {
				groups[format.category] = [];
			}
			groups[format.category].push(format);
		}

		// Sort by category order
		const sorted: [string, FormatOption[]][] = [];
		for (const cat of categoryOrder) {
			if (groups[cat]) {
				sorted.push([cat, groups[cat]]);
			}
		}

		return sorted;
	});

	function selectFormat(format: string) {
		dispatch('select', format);
	}
</script>

<div class="format-selector">
	<h3 class="selector-title">変換先の形式を選択</h3>

	{#if availableFormats.length === 0}
		<p class="no-formats">利用可能な形式がありません</p>
	{:else}
		{#each groupedFormats as [category, formats]}
			<div class="format-group">
				<h4 class="group-title">{categoryLabels[category as keyof typeof categoryLabels] || category}</h4>
				<div class="format-grid">
					{#each formats as format}
						<button
							class="format-button"
							class:selected={selectedFormat === format.value}
							onclick={() => selectFormat(format.value)}
							type="button"
							title={format.description}
						>
							<span class="format-label">{format.label}</span>
							{#if format.description}
								<span class="format-desc">{format.description}</span>
							{/if}
						</button>
					{/each}
				</div>
			</div>
		{/each}
	{/if}
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

	.no-formats {
		color: var(--color-text-secondary);
		font-size: 0.875rem;
	}

	.format-group {
		margin-bottom: 1.25rem;
	}

	.group-title {
		font-size: 0.75rem;
		font-weight: 600;
		color: var(--color-text-secondary);
		text-transform: uppercase;
		letter-spacing: 0.05em;
		margin-bottom: 0.5rem;
	}

	.format-grid {
		display: grid;
		grid-template-columns: repeat(auto-fill, minmax(85px, 1fr));
		gap: 0.4rem;
	}

	.format-button {
		display: flex;
		flex-direction: column;
		align-items: center;
		justify-content: center;
		padding: 0.5rem 0.25rem;
		border: 2px solid var(--color-border);
		border-radius: 0.5rem;
		background: var(--color-bg-secondary);
		color: var(--color-text);
		cursor: pointer;
		transition: all 0.15s ease;
		min-height: 50px;
	}

	.format-button:hover {
		border-color: var(--color-primary);
		background: rgba(99, 102, 241, 0.1);
	}

	.format-button:active {
		transform: scale(0.97);
	}

	.format-button.selected {
		border-color: var(--color-primary);
		background: linear-gradient(135deg, var(--color-primary), var(--color-secondary));
		color: white;
	}

	.format-label {
		font-size: 0.8rem;
		font-weight: 600;
	}

	.format-desc {
		font-size: 0.55rem;
		color: var(--color-text-secondary);
		margin-top: 0.1rem;
		text-align: center;
		line-height: 1.2;
	}

	.format-button.selected .format-desc {
		color: rgba(255, 255, 255, 0.8);
	}

	@media (max-width: 480px) {
		.format-grid {
			grid-template-columns: repeat(4, 1fr);
		}
	}
</style>
