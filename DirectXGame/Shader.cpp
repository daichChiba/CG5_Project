#include "Shader.h"
#include "KamataEngine.h"
#include "MiscUtility.h"
#include "d3dcompiler.h"
#include <dxcapi.h>
#pragma comment(lib, "dxcompiler.lib") // d3dcompiler.libをリンクする

using namespace KamataEngine;

void Shader::Load(const std::wstring& filePath, const std::wstring& shaderModel) {

	MiscUtility miscUtility_;

	// wstring => string 文字列変換
	std::string mdShaderModel = miscUtility_.ConvertString(shaderModel);

	Microsoft::WRL::ComPtr<ID3DBlob> shaderBlob = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3DCompileFromFile(
		filePath.c_str(),                                // シェーダファイル名
		nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,      // インクルード可能になる
		"main", mdShaderModel.c_str(),                   // エントリーポイント名、シェーダモデル
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

void Shader::LoadDxc(const std::wstring& filePath, const std::wstring& shaderModel) {
	// DXC(DirectX Shader Compiler)を初期化
	static Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils = nullptr;
	static Microsoft::WRL::ComPtr<IDxcCompiler3> dxcCompiler = nullptr;
	static Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler = nullptr;

	HRESULT hr;
	if (dxcUtils == nullptr) {
		hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils));
		assert(SUCCEEDED(hr)); // うまくいかなかった場合は起動できない
	}
	if (dxcCompiler == nullptr) {
		hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler));
		assert(SUCCEEDED(hr)); // うまくいかなかった場合は起動できない
	}
	if (includeHandler == nullptr) {
		hr = dxcUtils->CreateDefaultIncludeHandler(&includeHandler);
		assert(SUCCEEDED(hr)); // うまくいかなかった場合は起動できない
	}
	// 1.hlslファイルを読み込む
	Microsoft::WRL::ComPtr<IDxcBlobEncoding> shaderSource = nullptr;
	hr = dxcUtils->LoadFile(filePath.c_str(), nullptr, &shaderSource);
	assert(SUCCEEDED(hr)); // うまくいかなかった場合は起動できない

	// 読み込んだファイルの内容をDxcBufferに変換する
	DxcBuffer shaderSourceBuffer{};
	shaderSourceBuffer.Ptr = shaderSource->GetBufferPointer();
	shaderSourceBuffer.Size = shaderSource->GetBufferSize();
	shaderSourceBuffer.Encoding = DXC_CP_UTF8;

	//	2.	Compileする
	//		Compileに必要なコンパイルオプションの準備
	LPCWSTR arguments[] = {
		filePath.c_str(), // コンパイル対象のhlslファイル名
		L"-E",
		L"main", // エントリーポイントの指定。基本的にmain以外にはしない
		L"-T",
		shaderModel.c_str(), // ShaderProfileの設定
		L"-Zi",
		L"-Qembed_debug", // デバッグ情報を埋め込む
		L"-Od",           // 最適化を外しておく
		L"-Zpc",          // メモリレイアウトは行優先
	};
	//	実際にShaderをCompileする
	Microsoft::WRL::ComPtr<IDxcResult> shaderResult = nullptr;
	hr = dxcCompiler->Compile(
		&shaderSourceBuffer,        // 読み込んだファイル
		arguments,                  // コンパイルオプション
		_countof(arguments),        // コンパイルオプションの数
		includeHandler.Get(),       // includeが含まれた諸々
		IID_PPV_ARGS(&shaderResult) // コンパイル結果
	);
	// コンパイルエラーではなくdxcが起動できないなど致命的な状況
	assert(SUCCEEDED(hr));

	// 3.警告・エラーが出ていないか確認する
	Microsoft::WRL::ComPtr<IDxcBlobUtf8> shaderError = nullptr;
	Microsoft::WRL::ComPtr<IDxcBlobWide> nameBlob = nullptr;
	shaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&shaderError), &nameBlob);
	if (shaderError != nullptr && shaderError->GetStringLength() != 0) {
		OutputDebugStringA(shaderError->GetStringPointer());
		assert(false);
	}

	// 4. Compile結果を受け取る
	Microsoft::WRL::ComPtr<IDxcBlob> shaderBlob = nullptr;
	hr = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), &nameBlob);
	assert(SUCCEEDED(hr)); // うまくいかなかった場合は起動できない

	dxcBlob_ = shaderBlob;
}

Microsoft::WRL::ComPtr<ID3DBlob> Shader::GetBlob() { return blob_; }

Microsoft::WRL::ComPtr<IDxcBlob> Shader::GetDxcBlob() { return dxcBlob_; }

Shader::Shader() {}

Shader::~Shader() {}
