#pragma once
#include <wrl.h>
#include <d3dx12.h> // ID3D12RootSignature

class RootSignature {
public:
	// 生成
	void Create();

	// ゲッター
	Microsoft::WRL::ComPtr<ID3D12RootSignature> Get();


	// コンストラクタ
	RootSignature();
	// デストラクタ
	~RootSignature();

private:
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_ = nullptr;
};
