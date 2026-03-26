<script lang="ts">
	import { createEventDispatcher } from 'svelte';
	import { getOutputFormats } from '$lib/converter';

	interface FormatOption {
		value: string;
		label: string;
		description?: string;
	}

	interface Props {
		selectedFormat?: string;
		fileType?: 'video' | 'audio' | 'image' | 'document' | null;
	}

	let { selectedFormat = '', fileType = null }: Props = $props();

	const dispatch = createEventDispatcher<{ select: string }>();

	const formatLabels: Record<string, { label: string; description?: string }> = {
		// Image
		png: { label: 'PNG', description: '高品質・透過対応' },
		jpeg: { label: 'JPEG', description: '写真向け・軽量' },
		jpg: { label: 'JPG', description: '写真向け・軽量' },
		webp: { label: 'WebP', description: '最新・高圧縮' },
		gif: { label: 'GIF', description: 'アニメーション対応' },
		bmp: { label: 'BMP', description: '無圧縮' },
		// Audio
		wav: { label: 'WAV', description: '無圧縮・高音質' },
		mp3: { label: 'MP3', description: '汎用・互換性高' },
		webm: { label: 'WebM', description: '高圧縮' },
		ogg: { label: 'OGG', description: 'オープン形式' },
		m4a: { label: 'M4A', description: 'Apple互換' },
		// Video
		mp4: { label: 'MP4', description: '汎用・互換性高' },
		// Document
		txt: { label: 'TXT', description: 'プレーンテキスト' },
		pdf: { label: 'PDF', description: 'ドキュメント' }
	};

	let availableFormats = $derived.by(() => {
		if (!fileType) return [];

		const formats = getOutputFormats(fileType);
		return formats.map(f => ({
			value: f,
			label: formatLabels[f]?.label || f.toUpperCase(),
			description: formatLabels[f]?.description
		}));
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
		<div class="format-grid">
			{#each availableFormats as format}
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

	.format-grid {
		display: grid;
		grid-template-columns: repeat(auto-fill, minmax(120px, 1fr));
		gap: 0.75rem;
	}

	.format-button {
		display: flex;
		flex-direction: column;
		align-items: center;
		justify-content: center;
		padding: 1rem 0.75rem;
		border: 2px solid var(--color-border);
		border-radius: 0.75rem;
		background: var(--color-bg-secondary);
		color: var(--color-text);
		cursor: pointer;
		transition: all 0.2s ease;
		min-height: 70px;
	}

	.format-button:hover {
		border-color: var(--color-primary);
		background: rgba(99, 102, 241, 0.1);
		transform: translateY(-2px);
	}

	.format-button:active {
		transform: translateY(0);
	}

	.format-button.selected {
		border-color: var(--color-primary);
		background: linear-gradient(135deg, var(--color-primary), var(--color-secondary));
		color: white;
	}

	.format-label {
		font-size: 1rem;
		font-weight: 600;
	}

	.format-desc {
		font-size: 0.7rem;
		color: var(--color-text-secondary);
		margin-top: 0.25rem;
	}

	.format-button.selected .format-desc {
		color: rgba(255, 255, 255, 0.8);
	}
</style>
