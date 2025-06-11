#pragma once
#include <d3dx12.h> // ID3D12Resource,D3D12_INDEX_BUFFER_VIEW
#include <wrl.h>

class IndexBuffer {
public:
	// VertexBuffer生成
	void Create(const UINT size, const UINT stride);

	// ゲッター
	Microsoft::WRL::ComPtr<ID3D12Resource> Get(); // インデックスバッファー
	D3D12_INDEX_BUFFER_VIEW* GetView();           // インデックスバッファービュー

	// コンストラクタ
	IndexBuffer();
	// デストラクタ
	~IndexBuffer();

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> indexBuffer_ = nullptr; // インデックスバッファ
	D3D12_INDEX_BUFFER_VIEW indexBufferView_{};                  // インデックスバッファビュー
};
