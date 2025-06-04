#include "PipelineState.h"
#include "KamataEngine.h"

using namespace KamataEngine;

void PipelineState::Create(D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc) {
	// クラス内で取得するために追加
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState = nullptr;
	HRESULT hr = dxCommon->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&graphicsPipelineState));
	assert(SUCCEEDED(hr));

	// 生成したpipelineStateを取っておく
	pipelineState_ = graphicsPipelineState;

}

Microsoft::WRL::ComPtr<ID3D12PipelineState> PipelineState::Get() {
	return pipelineState_;
}

PipelineState::PipelineState() {}

PipelineState::~PipelineState() {
}
