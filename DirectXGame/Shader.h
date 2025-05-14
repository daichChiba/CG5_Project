#pragma once
#include <Windows.h>
#include <d3d12.h>
#include <d3dcompiler.h> //	dxcapi.hが依存している
#include <dxcapi.h>      //	IDxcBlobを利用するため
#include <string>
#include <wrl.h>

// Shaderクラス
class Shader {
public:
	// シェイダーファイルを読み込み、コンパイル済みデータを生成する
	void Load(const std::wstring& filePath, const std::wstring& shaderModel);
	void LoadDxc(const std::wstring& filePath, const std::wstring& shaderModel);

	// 生成したコンパイル済みデータを取得する
	Microsoft::WRL::ComPtr<ID3DBlob> GetBlob();
	Microsoft::WRL::ComPtr<IDxcBlob> GetDxcBlob();

	// コンストラクタ
	Shader();
	// デストラクタ
	~Shader();

private:
	Microsoft::WRL::ComPtr<ID3DBlob> blob_ = nullptr;

	Microsoft::WRL::ComPtr<IDxcBlob> dxcBlob_ = nullptr;
};
