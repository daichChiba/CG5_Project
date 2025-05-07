#include "Shader.h"
#include "KamataEngine.h"
#include "d3dcompiler.h"

using namespace KamataEngine;

void Shader::Load(const std::wstring& filePath, const std::string& shaderModel) {
	Microsoft::WRL::ComPtr<ID3DBlob> shaderBlob = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3DCompileFromFile(
	    filePath.c_str(),                                // シェーダファイル名
	    nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,      // インクルード可能になる
	    "main", shaderModel.c_str(),                     // エントリーポイント名、シェーダモデル
	    D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, // デバッグ用設定
	    0, &shaderBlob, &errorBlob);
	// エラーが発生した場合、止める
	if (FAILED(hr)) {
		if (errorBlob) {
			DebugText::GetInstance()->ConsolePrintf(reinterpret_cast<const char*>(errorBlob->GetBufferPointer()));
		}
		assert(false);
	}
	blob_ = shaderBlob;
}

Microsoft::WRL::ComPtr<ID3DBlob> Shader::GetBlob() { return blob_; }

Shader::Shader() {}

Shader::~Shader() {}
