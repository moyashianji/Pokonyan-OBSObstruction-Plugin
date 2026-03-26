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
		txt: { label: 'TXT', description: 'テキスト', category: 'image', mimeType: 'text/plain' }
	};

	const categoryConfig = {
		video: { label: '動画', icon: '🎬', color: '#8b5cf6' },
		audio: { label: '音声', icon: '🎵', color: '#06b6d4' },
		image: { label: '画像', icon: '🖼️', color: '#10b981' }
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
	<h3 class="selector-title">
		<span class="title-icon">🔄</span>
		変換先の形式
	</h3>

	{#if availableFormats.length === 0}
		<p class="no-formats">利用可能な形式がありません</p>
	{:else}
		{#each groupedFormats as [category, formats], i}
			{@const config = categoryConfig[category as keyof typeof categoryConfig]}
			<div class="format-group" style="animation-delay: {i * 50}ms">
				<h4 class="group-header">
					<span class="group-icon">{config.icon}</span>
					<span class="group-label">{config.label}</span>
					<span class="group-count">{formats.length}</span>
				</h4>
				<div class="format-grid">
					{#each formats as format, j}
						<button
							class="format-btn"
							class:selected={selectedFormat === format.value}
							onclick={() => selectFormat(format.value)}
							type="button"
							title={format.description}
							style="animation-delay: {(i * 50) + (j * 20)}ms"
						>
							<span class="format-name">{format.label}</span>
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
		display: flex;
		align-items: center;
		gap: 0.5rem;
		font-size: 1rem;
		font-weight: 600;
		color: var(--color-text);
		margin-bottom: 1rem;
	}

	.title-icon {
		font-size: 1.125rem;
	}

	.no-formats {
		color: var(--color-text-secondary);
		font-size: 0.875rem;
		text-align: center;
		padding: 1rem;
	}

	.format-group {
		margin-bottom: 1.25rem;
		animation: fadeInUp 0.3s ease-out backwards;
	}

	@keyframes fadeInUp {
		from {
			opacity: 0;
			transform: translateY(10px);
		}
		to {
			opacity: 1;
			transform: translateY(0);
		}
	}

	.format-group:last-child {
		margin-bottom: 0;
	}

	.group-header {
		display: flex;
		align-items: center;
		gap: 0.5rem;
		margin-bottom: 0.5rem;
		padding-bottom: 0.35rem;
		border-bottom: 1px solid var(--color-border);
	}

	.group-icon {
		font-size: 0.875rem;
	}

	.group-label {
		font-size: 0.75rem;
		font-weight: 600;
		color: var(--color-text-secondary);
		text-transform: uppercase;
		letter-spacing: 0.05em;
	}

	.group-count {
		margin-left: auto;
		font-size: 0.65rem;
		font-weight: 500;
		color: var(--color-text-muted);
		background: var(--color-bg-tertiary);
		padding: 0.1rem 0.4rem;
		border-radius: 1rem;
	}

	.format-grid {
		display: grid;
		grid-template-columns: repeat(auto-fill, minmax(70px, 1fr));
		gap: 0.4rem;
	}

	.format-btn {
		display: flex;
		flex-direction: column;
		align-items: center;
		justify-content: center;
		padding: 0.6rem 0.4rem;
		min-height: 52px;
		border: 1px solid var(--color-border);
		border-radius: var(--radius-sm);
		background: var(--color-bg-glass);
		color: var(--color-text);
		cursor: pointer;
		transition: all 0.15s ease;
		animation: fadeInUp 0.3s ease-out backwards;
	}

	.format-btn:hover {
		border-color: var(--color-primary);
		background: rgba(99, 102, 241, 0.1);
		transform: translateY(-2px);
	}

	.format-btn:active {
		transform: translateY(0) scale(0.98);
	}

	.format-btn.selected {
		border-color: var(--color-primary);
		background: var(--gradient-primary);
		color: white;
		box-shadow: 0 4px 12px rgba(99, 102, 241, 0.3);
	}

	.format-name {
		font-size: 0.8rem;
		font-weight: 600;
		line-height: 1;
	}

	.format-desc {
		font-size: 0.55rem;
		color: var(--color-text-secondary);
		margin-top: 0.2rem;
		text-align: center;
		line-height: 1.2;
		max-width: 100%;
		overflow: hidden;
		text-overflow: ellipsis;
		white-space: nowrap;
	}

	.format-btn.selected .format-desc {
		color: rgba(255, 255, 255, 0.8);
	}

	/* Mobile */
	@media (max-width: 480px) {
		.format-grid {
			grid-template-columns: repeat(4, 1fr);
		}

		.format-btn {
			padding: 0.5rem 0.25rem;
			min-height: 46px;
		}

		.format-name {
			font-size: 0.75rem;
		}

		.format-desc {
			display: none;
		}
	}
</style>
