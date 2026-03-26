<script lang="ts">
	import { createEventDispatcher } from 'svelte';
	import { getOutputFormats } from '$lib/converter';

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

	const formatInfo: Record<string, { label: string; description: string; category: 'video' | 'audio' | 'image' }> = {
		// Video
		mp4: { label: 'MP4', description: '最も互換性が高い', category: 'video' },
		webm: { label: 'WebM', description: 'Web最適化', category: 'video' },
		avi: { label: 'AVI', description: 'レガシー互換', category: 'video' },
		mov: { label: 'MOV', description: 'Apple形式', category: 'video' },
		mkv: { label: 'MKV', description: '高品質コンテナ', category: 'video' },
		gif: { label: 'GIF', description: 'アニメーション', category: 'video' },
		flv: { label: 'FLV', description: 'Flash形式', category: 'video' },
		// Audio
		mp3: { label: 'MP3', description: '最も普及', category: 'audio' },
		wav: { label: 'WAV', description: '無圧縮高音質', category: 'audio' },
		ogg: { label: 'OGG', description: 'オープン形式', category: 'audio' },
		aac: { label: 'AAC', description: '高効率圧縮', category: 'audio' },
		flac: { label: 'FLAC', description: '可逆圧縮', category: 'audio' },
		m4a: { label: 'M4A', description: 'Apple音声', category: 'audio' },
		opus: { label: 'Opus', description: '最新高効率', category: 'audio' },
		// Image
		png: { label: 'PNG', description: '透過対応', category: 'image' },
		jpg: { label: 'JPG', description: '写真向け', category: 'image' },
		jpeg: { label: 'JPEG', description: '写真向け', category: 'image' },
		webp: { label: 'WebP', description: '次世代形式', category: 'image' },
		bmp: { label: 'BMP', description: '無圧縮', category: 'image' },
		// Document
		txt: { label: 'TXT', description: 'プレーンテキスト', category: 'image' },
	};

	const categoryLabels = {
		video: '動画形式',
		audio: '音声形式',
		image: '画像形式'
	};

	let availableFormats = $derived.by(() => {
		if (!fileType) return [];

		const formats = getOutputFormats(fileType);
		return formats.map(f => ({
			value: f,
			label: formatInfo[f]?.label || f.toUpperCase(),
			description: formatInfo[f]?.description || '',
			category: formatInfo[f]?.category || 'image'
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

		return groups;
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
		{#each Object.entries(groupedFormats) as [category, formats]}
			<div class="format-group">
				<h4 class="group-title">{categoryLabels[category as keyof typeof categoryLabels] || category}</h4>
				<div class="format-grid">
					{#each formats as format}
						<button
							class="format-button"
							class:selected={selectedFormat === format.value}
							onclick={() => selectFormat(format.value)}
							type="button"
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
		grid-template-columns: repeat(auto-fill, minmax(100px, 1fr));
		gap: 0.5rem;
	}

	.format-button {
		display: flex;
		flex-direction: column;
		align-items: center;
		justify-content: center;
		padding: 0.75rem 0.5rem;
		border: 2px solid var(--color-border);
		border-radius: 0.5rem;
		background: var(--color-bg-secondary);
		color: var(--color-text);
		cursor: pointer;
		transition: all 0.2s ease;
		min-height: 60px;
	}

	.format-button:hover {
		border-color: var(--color-primary);
		background: rgba(99, 102, 241, 0.1);
	}

	.format-button:active {
		transform: scale(0.98);
	}

	.format-button.selected {
		border-color: var(--color-primary);
		background: linear-gradient(135deg, var(--color-primary), var(--color-secondary));
		color: white;
	}

	.format-label {
		font-size: 0.875rem;
		font-weight: 600;
	}

	.format-desc {
		font-size: 0.65rem;
		color: var(--color-text-secondary);
		margin-top: 0.125rem;
	}

	.format-button.selected .format-desc {
		color: rgba(255, 255, 255, 0.8);
	}

	@media (max-width: 480px) {
		.format-grid {
			grid-template-columns: repeat(3, 1fr);
		}
	}
</style>
