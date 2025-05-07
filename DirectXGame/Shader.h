#pragma once
#include <string>
#include <d3d12.h>
#include <Windows.h>
#include <wrl.h>
// Shaderクラス
class Shader {
public:
	// シェイダーファイルを読み込み、コンパイル済みデータを生成する
	void Load(const std::wstring& filePath, const std::string& shaderModel);
	//生成したコンパイル済みデータを取得する
	Microsoft::WRL::ComPtr<ID3DBlob> GetBlob();

	//コンストラクタ
	Shader();
	//デストラクタ
	~Shader();

private:
	Microsoft::WRL::ComPtr<ID3DBlob> blob_ = nullptr;
};
