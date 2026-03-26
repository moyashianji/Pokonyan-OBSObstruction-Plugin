<script lang="ts">
	import { createEventDispatcher } from 'svelte';
	import { getOutputFormats } from '$lib/converter';
	import { VIDEO_FORMATS, AUDIO_FORMATS, IMAGE_FORMATS, type FormatInfo } from '$lib/formats';

	interface FormatOption {
		value: string;
		label: string;
		category: 'video' | 'audio' | 'image';
	}

	interface Props {
		selectedFormat?: string;
		selectedFormats?: string[];
		multiSelect?: boolean;
		fileType?: 'video' | 'audio' | 'image' | 'document' | null;
	}

	let {
		selectedFormat = '',
		selectedFormats = [],
		multiSelect = false,
		fileType = null
	}: Props = $props();

	const dispatch = createEventDispatcher<{ select: string; toggle: string }>();

	const allFormats: Record<string, FormatInfo> = {
		...VIDEO_FORMATS,
		...AUDIO_FORMATS,
		...IMAGE_FORMATS,
		txt: { label: 'TXT', description: 'Text', category: 'image', mimeType: 'text/plain' }
	};

	const categories = {
		video: 'Video',
		audio: 'Audio',
		image: 'Image'
	};

	let availableFormats = $derived.by(() => {
		if (!fileType) return [];
		return getOutputFormats(fileType).map(f => ({
			value: f,
			label: allFormats[f]?.label || f.toUpperCase(),
			category: allFormats[f]?.category || 'image'
		}));
	});

	let groupedFormats = $derived.by(() => {
		const groups: Record<string, FormatOption[]> = {};
		for (const format of availableFormats) {
			if (!groups[format.category]) groups[format.category] = [];
			groups[format.category].push(format);
		}
		return Object.entries(groups).sort(([a], [b]) => {
			const order = ['video', 'audio', 'image'];
			return order.indexOf(a) - order.indexOf(b);
		});
	});

	function isSelected(value: string): boolean {
		return multiSelect ? selectedFormats.includes(value) : selectedFormat === value;
	}

	function handleClick(format: string) {
		dispatch(multiSelect ? 'toggle' : 'select', format);
	}
</script>

<div class="selector">
	{#if availableFormats.length === 0}
		<p class="empty">No formats available</p>
	{:else}
		{#each groupedFormats as [category, formats]}
			<div class="group">
				<div class="group-label">{categories[category as keyof typeof categories]} ({formats.length})</div>
				<div class="grid">
					{#each formats as format}
						<button
							type="button"
							class="format"
							class:selected={isSelected(format.value)}
							onclick={() => handleClick(format.value)}
						>
							{format.label}
						</button>
					{/each}
				</div>
			</div>
		{/each}
	{/if}
</div>

<style>
	.selector {
		display: flex;
		flex-direction: column;
		gap: 12px;
	}

	.empty {
		color: var(--c-text-3);
		font-size: 13px;
		text-align: center;
		padding: 12px;
	}

	.group-label {
		font-size: 11px;
		font-weight: 500;
		color: var(--c-text-3);
		text-transform: uppercase;
		letter-spacing: 0.04em;
		margin-bottom: 8px;
	}

	.grid {
		display: flex;
		flex-wrap: wrap;
		gap: 6px;
	}

	.format {
		padding: 6px 10px;
		font-size: 12px;
		font-weight: 500;
		color: var(--c-text-2);
		background: var(--c-surface);
		border: 1px solid var(--c-border-subtle);
		border-radius: var(--radius-sm);
		cursor: pointer;
		transition: all 0.12s;
	}

	.format:hover {
		color: var(--c-text);
		border-color: var(--c-border);
		background: var(--c-surface-raised);
	}

	.format.selected {
		color: var(--c-accent);
		border-color: var(--c-accent);
		background: var(--c-accent-subtle);
	}
</style>
