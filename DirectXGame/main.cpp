#include "KamataEngine.h"
#include <Windows.h>


using namespace KamataEngine;
// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {
	// エンジンの初期化
	Initialize(L"LE3D_16_チバ_ダイチ");


	// DirectXCommonインスタンスの取得
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	// メインループ
	while (true) {
		// エンジンの更新
		if (Update()) {

			break;
		}

		// 描画開始
		dxCommon->PreDraw();


		// 描画終了
		dxCommon->PostDraw();
	}

	// エンジンの終了処理
	Finalize();

	return 0;
}
