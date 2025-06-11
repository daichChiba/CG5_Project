#include "IndexBuffer.h"
#include "KamataEngine.h"

#include <cassert>

using namespace KamataEngine;

void IndexBuffer::Create(const UINT size, const UINT stride) {
	// strideの値によって、1つのインデックスのフォーマットを決める
	assert(stride == 2 || stride == 4); // 2byte or 4byte のみ受け付ける
	DXGI_FORMAT format = (stride == 2) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;

	// クラス内で取得するために追加
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	// インデックスリソースの作成 -----------
	// インデックスリソース用のヒープの設定
	D3D12_HEAP_PROPERTIES uploadHeapProperties{};
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD; // CPUにアップロードするヒープ
	// インデックスリソースの設定
	D3D12_RESOURCE_DESC indexResourceDesc{};
	indexResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER; // バッファ
	indexResourceDesc.Width = size;                 // リソースのサイズ。今回はVector4を3頂点分
	// バッファの場合はこれらは1にする決まり
	indexResourceDesc.Height = 1;
	indexResourceDesc.DepthOrArraySize = 1;
	indexResourceDesc.MipLevels = 1;
	indexResourceDesc.SampleDesc.Count = 1;
	// バッファの場合はこれにする決まり
	indexResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	// 実際に頂点リソースを生成する
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource = nullptr;
	HRESULT hr = dxCommon->GetDevice()->CreateCommittedResource(
	    &uploadHeapProperties, D3D12_HEAP_FLAG_NONE,           // ヒープの設定
	    &indexResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, // リソースの設定
	    nullptr, IID_PPV_ARGS(&indexResource));                // リソースの生成
	assert(SUCCEEDED(hr));                                     // うまくいかなかったときは起動できない
	// 生成した頂点リソースを取っておく
	indexBuffer_ = indexResource;

	// インデックスリソースを作成する -----------
	D3D12_INDEX_BUFFER_VIEW indexBufferView{};
	// リソースの先頭アドレスから使う
	indexBufferView.BufferLocation = indexResource->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点size分のサイズ
	indexBufferView.SizeInBytes = size; // 頂点リソースの全サイズ
	// インデックスのフォーマット
	indexBufferView.Format = format;

	// IndexBufferViewを取っておく
	indexBufferView_ = indexBufferView;
}

Microsoft::WRL::ComPtr<ID3D12Resource> IndexBuffer::Get() { return indexBuffer_; }

D3D12_INDEX_BUFFER_VIEW* IndexBuffer::GetView() { return &indexBufferView_; }

IndexBuffer::IndexBuffer() {}

IndexBuffer::~IndexBuffer() {}
