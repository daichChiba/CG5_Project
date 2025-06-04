#include "RootSignature.h"
#include "KamataEngine.h"
#include "PipelineState.h"

using namespace KamataEngine;

// RootSignatureを生成する
void RootSignature::Create() {
	// クラス内で取得するために追加
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	// RootSignature作成-----------
	// 構造体にデータを用意する
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr)) {
		DebugText::GetInstance()->ConsolePrintf(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}

	//  バイナリを基に作成
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = nullptr;
	hr = dxCommon->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
	assert(SUCCEEDED(hr));

	// 生成したRootSignatureを取っておく
	rootSignature_ = rootSignature;
}

Microsoft::WRL::ComPtr<ID3D12RootSignature> RootSignature::Get() {
	return rootSignature_; }


RootSignature::RootSignature() {
}

RootSignature::~RootSignature() {

}
