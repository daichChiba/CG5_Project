#include "Programs/IndexBuffer.h"
#include "KamataEngine.h"
#include "Programs/PipelineState.h"
#include "Programs/RootSignature.h"
#include "Programs/Shader.h"
#include "Programs/VertexBuffer.h"
#include "Programs/WorldTransformEx.h"
// #include "d3dcompiler.h"
#include <Windows.h>
#include <cassert>

using namespace KamataEngine;

// 関数プロトタイプ宣言
void SetupPipelineState(PipelineState& pipelineState, RootSignature& rs, Shader& vs, Shader ps);
// RenderTextureResourceの生成
Microsoft::WRL::ComPtr<ID3D12Resource> CreateRenderTextureResource(ID3D12Device* device, uint32_t width, uint32_t height, DXGI_FORMAT format, const FLOAT* clearColor);
Microsoft::WRL::ComPtr<ID3D12Resource> CreateDepthStencilTextureResource(ID3D12Device* device, uint32_t width, uint32_t height);

// インプットレイアウト、ブレンドステート、ラスタライザステート
// 引数として空のpipelineState、RootSignature、頂点シェーダーvs、ピクセルシェイダーpsを参照で受け取る
void SetupPipelineState(PipelineState& pipelineState, RootSignature& rs, Shader& vs, Shader ps) {

	// InputLayout
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[2] = {};
	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs[1].SemanticName = "TEXCOORD";
	inputElementDescs[1].SemanticIndex = 0;
	inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = _countof(inputElementDescs);

	// BlendState------------今回は不透明
	D3D12_BLEND_DESC blendDesc{};
	// 全ての色要素を書き込む
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	// RasterizersState ----------------
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	// 裏面(反時計回り)をカリングする
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	// 塗りつぶしモードをソリッドにする(ワイヤーフレームならD3D12_FILL_MODE_WIREFRAME)
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	// PSO(PipelineStateObject)の作成 -----------
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	graphicsPipelineStateDesc.pRootSignature = rs.Get();                                                    // RootSignature
	graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;                                                // InputLayout
	graphicsPipelineStateDesc.VS = {vs.GetDxcBlob()->GetBufferPointer(), vs.GetDxcBlob()->GetBufferSize()}; // VertexShader
	graphicsPipelineStateDesc.PS = {ps.GetDxcBlob()->GetBufferPointer(), ps.GetDxcBlob()->GetBufferSize()}; // PixelShader
	graphicsPipelineStateDesc.BlendState = blendDesc;                                                       // BlendState
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;                                             // RasterizerState

	// 書き込むRTVの情報
	graphicsPipelineStateDesc.NumRenderTargets = 1; // 書き込むRTVの数 ※2つ同時にしようと思えば行ける
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	// 利用するトポロジ(形状)のタイプ。三角形
	graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	// どのように画面に色を打ち込むかの設定(今は気にしなくてもいい)
	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	// 準備は整ったので、PSOを作成する
	pipelineState.Create(graphicsPipelineStateDesc);
}

// RenderTextureResourceの生成
Microsoft::WRL::ComPtr<ID3D12Resource> CreateRenderTextureResource(ID3D12Device* device, uint32_t width, uint32_t height, DXGI_FORMAT format, const FLOAT* clearColor) {

	// 1.生成するRenderTextureResourceのDescの設定
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = UINT(width);                             // RenderTextureの幅
	resourceDesc.Height = UINT(height);                           // Textureの高さ
	resourceDesc.MipLevels = 1;                                   // mipmapの数
	resourceDesc.DepthOrArraySize = 1;                            // 奥行 or 配列Textureの配列数
	resourceDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;        // TextureのFormat
	resourceDesc.SampleDesc.Count = 1;                            // サンプリングカウント 1固定
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;  // Textureの時限数。普段使っているのは、2次元
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET; // RenderTargetとして使う通知

	// 2.利用するHeapの設定
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT; // VRAM上に作る

	// 3.ClearValueの用意
	D3D12_CLEAR_VALUE clearValue;
	clearValue.Format = format;
	clearValue.Color[0] = clearColor[0];
	clearValue.Color[1] = clearColor[1];
	clearValue.Color[2] = clearColor[2];
	clearValue.Color[3] = clearColor[3];

	// 4.RenderTextureResourceの生成
	Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
	[[maybe_unused]] HRESULT hr = device->CreateCommittedResource(
	    &heapProperties,                            // Heapの設定
	    D3D12_HEAP_FLAG_NONE,                       // Heapの特殊な設定
	    &resourceDesc,                              // Resourceの設定
	    D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, // PixelShaderでアクセスできるようにする
	    &clearValue,                                // Clear最適値
	    IID_PPV_ARGS(&resource));
	assert(SUCCEEDED(hr));

	return resource;
}

Microsoft::WRL::ComPtr<ID3D12Resource> CreateDepthStencilTextureResource(ID3D12Device* device, uint32_t width, uint32_t height) {
	// 1.生成するDepthStancilTextureのDescの設定
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = width;                                   // Textureの幅
	resourceDesc.Height = height;                                 // Textureの高さ
	resourceDesc.MipLevels = 1;                                   // mipmapの数 DepthStencilなので1つでいい
	resourceDesc.DepthOrArraySize = 1;                            // Textureの配列数 DepthStencilなので1つでいい
	resourceDesc.Format = DXGI_FORMAT_D32_FLOAT;                  // DepthStencilとして利用可能なフォーマット
	                                                              // ※KamataEngineと合わせる
	resourceDesc.SampleDesc.Count = 1;                            // サンプリングカウント 1固定
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;  // 2次元
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL; // DepthStencilとして使う通知

	// 2.利用するHeapの設定
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT; // VRAM上に作る

	// 深度値のクリア設定
	D3D12_CLEAR_VALUE depthClearValue{};
	depthClearValue.DepthStencil.Depth = 1.0f;      // 1.0f(最大値)でクリア
	depthClearValue.Format = DXGI_FORMAT_D32_FLOAT; // Zバッファ形式、resourceと合わせる
	                                                // ※KamataEngineと合わせた

	Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
	[[maybe_unused]] HRESULT hr = device->CreateCommittedResource(
	    &heapProperties,                  // Heapの設定
	    D3D12_HEAP_FLAG_NONE,             // Heapの特殊な設定★後で変更?
	    &resourceDesc,                    // Resourceの設定
	    D3D12_RESOURCE_STATE_DEPTH_WRITE, // 深度値書き込み状態にしておく
	    &depthClearValue,                 // Clear最適値
	    IID_PPV_ARGS(&resource));         // 作成Resourceポインタへのポインタ
	assert(SUCCEEDED(hr));

	return resource;
}

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {
	// エンジンの初期化
	Initialize(L"LE3D_16_チバ_ダイチ");

	// DirectXCommonインスタンスの取得
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	// DirectXCommonクラスが管理している、ウィンドウの幅と高さの値の取得
	int32_t width = dxCommon->GetBackBufferWidth();
	int32_t height = dxCommon->GetBackBufferHeight();
	DebugText::GetInstance()->ConsolePrintf(std::format("width:{},heigth:{}\n", width, height).c_str());

	// DirectXCommonクラスが管理している、コマンドリストの取得
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList = dxCommon->GetCommandList();

	// RootSignature生成-----------------
	RootSignature rs;
	rs.Create();

	// 頂点シェイダーの読み込みとコンパイル
	Shader vs;
	vs.LoadDxc(L"Resources/shaders/TestVS.hlsl", L"vs_6_0");
	assert(vs.GetDxcBlob() != nullptr);

	// ピクセルシェイダーの読み込みとコンパイル
	Shader TestPs, VignettePs, BoxFilterPs, BoxFilter5x5Ps, GaussianFilterPs, LuminanceBasedOutlinePs, RadialBlurPs;
	TestPs.LoadDxc(L"Resources/shaders/TestPS.hlsl", L"ps_6_0");
	assert(TestPs.GetDxcBlob() != nullptr);
	VignettePs.LoadDxc(L"Resources/shaders/vignettePS.hlsl", L"ps_6_0");
	assert(VignettePs.GetDxcBlob() != nullptr);
	BoxFilterPs.LoadDxc(L"Resources/shaders/BoxFilterPS.hlsl", L"ps_6_0");
	assert(BoxFilterPs.GetDxcBlob() != nullptr);
	BoxFilter5x5Ps.LoadDxc(L"Resources/shaders/BoxFilter5x5PS.hlsl", L"ps_6_0");
	assert(BoxFilter5x5Ps.GetDxcBlob() != nullptr);
	GaussianFilterPs.LoadDxc(L"Resources/shaders/GaussianFilterPS.hlsl", L"ps_6_0");
	assert(GaussianFilterPs.GetDxcBlob() != nullptr);
	LuminanceBasedOutlinePs.LoadDxc(L"Resources/shaders/LuminanceBasedOutlinePS.hlsl", L"ps_6_0");
	assert(LuminanceBasedOutlinePs.GetDxcBlob() != nullptr);
	RadialBlurPs.LoadDxc(L"Resources/shaders/RadialBlurPS.hlsl", L"ps_6_0");
	assert(RadialBlurPs.GetDxcBlob() != nullptr);

	PipelineState pipelineStateTest, pipelineStateVignette, pipelineStateBoxFilter, pipelineStateBoxFilter5x5, pipelineStateGaussianFilter, pipelineStateLuminanceBasedOutline,
	    pipelineStateRadialBlur;
	SetupPipelineState(pipelineStateTest, rs, vs, TestPs);
	SetupPipelineState(pipelineStateVignette, rs, vs, VignettePs);
	SetupPipelineState(pipelineStateBoxFilter, rs, vs, BoxFilterPs);
	SetupPipelineState(pipelineStateBoxFilter5x5, rs, vs, BoxFilter5x5Ps);
	SetupPipelineState(pipelineStateGaussianFilter, rs, vs, GaussianFilterPs);
	SetupPipelineState(pipelineStateLuminanceBasedOutline, rs, vs, LuminanceBasedOutlinePs);
	SetupPipelineState(pipelineStateRadialBlur, rs, vs, RadialBlurPs);



	const int changeSetPipelineStateFirst = 1;
	int changeSetPipelineState = changeSetPipelineStateFirst;
	const int changeSetPipelineStateMax = 7;

	// リソースの確保含め、頂点情報を柔軟に対応できるようにVertexData構造体を新たに作成する
	// Vertex4 ⇒ VertexDate に変更して利用する
	struct VertexData {
		Vector4 pos;
		Vector2 texCoord;
	};

	// 頂点データの準備
	VertexData vertices[] = {
	    {{-1.0f, 1.0f, 0.0f, 1.0f},  {0.0f, 0.0f}}, // 左上
	    {{1.0f, 1.0f, 0.0f, 1.0f},   {1.0f, 0.0f}}, // 右上
	    {{-1.0f, -1.0f, 0.0f, 1.0f}, {0.0f, 1.0f}}, // 左下
	    {{1.0f, -1.0f, 0.0f, 1.0f},  {1.0f, 1.0f}}, // 右下
	};

	uint16_t indices[] = {
	    0, 1, 2, 1, 3, 2,
	};
	// IndexBuffer(IndexResource, IndexResourceView)の生成
	IndexBuffer ib;
	ib.Create(sizeof(indices), sizeof(indices[0]));

	// 頂点インデックスリソースにデータを書き込む -----------
	uint16_t* pGpuIndices = nullptr;
	ib.Get()->Map(0, nullptr, reinterpret_cast<void**>(&pGpuIndices));

	for (int i = 0; i < _countof(indices); ++i) {
		pGpuIndices[i] = indices[i];
	}

	//============================================================
	// Resource生成、heap生成、View生成で再利用される変数の準備

	Microsoft::WRL::ComPtr<ID3D12Device> device_ = dxCommon->GetDevice();
	HRESULT hr;

	//============================================================
	// RenderTexture関係

	//------------------------------------------------------------
	// 0.RenderTextureResourceの作成

	// 画面クリア色※わかりやすいように赤とする
	const FLOAT kRenderTargetClearColor[4] = {1.0f, 0.0f, 0.0f, 1.0f};

	Microsoft::WRL::ComPtr<ID3D12Resource> renderTextureResource =
	    CreateRenderTextureResource(device_.Get(), WinApp::kWindowWidth, WinApp::kWindowHeight, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, kRenderTargetClearColor);
	//------------------------------------------------------------
	// 1.RTV用のDescriptorHeapの作成
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap_ = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC rtvDescriptorHeapDesc{};
	rtvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV; // RTV
	rtvDescriptorHeapDesc.NumDescriptors = 1;                    // Descriprorの個数は1

	hr = device_->CreateDescriptorHeap(&rtvDescriptorHeapDesc, IID_PPV_ARGS(&rtvDescriptorHeap_));
	assert(SUCCEEDED(hr));

	// CPU側から見たHANDLEを取得しておく
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandleCPU = rtvDescriptorHeap_->GetCPUDescriptorHandleForHeapStart();

	//------------------------------------------------------------
	// 2.RTV用のViewの生成
	device_->CreateRenderTargetView(
	    renderTextureResource.Get(), // Viewと関連付けたいリソース
	    nullptr,                     // RTVの詳細情報(Desc:Description、構成内容の記述)
	                                 // ※RTVの場合nullptrにするとDirectX12が自動で推測してくれる
	    rtvHandleCPU                 // RTV用のディスクリプタヒープのCPUHandle
	);

	//============================================================
	// DepthStancilTexture関係

	//------------------------------------------------------------
	// 0.DepthStancilTextureResourceの作成
	Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResource_ = CreateDepthStencilTextureResource(device_.Get(), WinApp::kWindowWidth, WinApp::kWindowHeight);

	//------------------------------------------------------------
	// 1.DSV用のDescriptorHeapの作成
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap_ = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC dsvDescriptorHeapDesc{};
	dsvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;   // Heap Type
	dsvDescriptorHeapDesc.NumDescriptors = 1;                      // Heap Type の個数
	dsvDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE; // DSVはShaderで触らないようにする

	hr = device_->CreateDescriptorHeap(&dsvDescriptorHeapDesc, IID_PPV_ARGS(&dsvDescriptorHeap_));
	assert(SUCCEEDED(hr));

	// CPU側から見たHANDLEを取得しておく
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandleCPU = dsvDescriptorHeap_->GetCPUDescriptorHandleForHeapStart();

	//------------------------------------------------------------
	// 2.DSV用のViewの生成
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;                // 基本的にResouceに合わせる
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D; // 2DTexture

	// DSVHeapの先頭にDSVを作る
	device_->CreateDepthStencilView(depthStencilResource_.Get(), &dsvDesc, dsvHandleCPU);

	//============================================================
	// SRV(Shader Resource View)を準備する ※PixelShaderと連携を取るようにするため

	//------------------------------------------------------------
	// 1.SRV用のDescriptorHeapの作成
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap_ = nullptr;

	D3D12_DESCRIPTOR_HEAP_DESC srvDescriptorHeapDesc = {};
	srvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;     // SRV
	srvDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE; // PixelShaderから見える
	srvDescriptorHeapDesc.NumDescriptors = 1;

	hr = device_->CreateDescriptorHeap(&srvDescriptorHeapDesc, IID_PPV_ARGS(srvDescriptorHeap_.GetAddressOf()));
	assert(SUCCEEDED(hr));

	// CPU側から見たHANDLE、GPU側から見たHANDLEを取得しておく
	D3D12_CPU_DESCRIPTOR_HANDLE srvHandleCPU = srvDescriptorHeap_->GetCPUDescriptorHandleForHeapStart();
	D3D12_GPU_DESCRIPTOR_HANDLE srvHandleGPU = srvDescriptorHeap_->GetGPUDescriptorHandleForHeapStart();

	//------------------------------------------------------------
	// 2.SRV(Shader Resource View)の作成
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;                           // RenderTargetResourceと同じにする
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING; // RGBA値をそのままShaderに対応させる
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;                      // 2Dテクスチャ
	srvDesc.Texture2D.MipLevels = 1;                                            // MipLevelは1しかない

	device_->CreateShaderResourceView(
	    renderTextureResource.Get(), // Viewと関連付けたいリソース
	    &srvDesc,                    // SRVの詳細情報(Desc:Description、構成内容の記述)
	    srvHandleCPU                 // SRV用のディスクリプタヒープのCPUHandle
	);

	// VertexBuffer(VertexResource, VertexResourceView)の生成
	VertexBuffer vb;
	vb.Create(sizeof(vertices) * 3, sizeof(vertices[0]));

	// 頂点リソースにデータを書き込む -----------
	VertexData* pGpuVertices = nullptr;
	vb.Get()->Map(0, nullptr, reinterpret_cast<void**>(&pGpuVertices));

	for (int i = 0; i < _countof(vertices); ++i) {
		pGpuVertices[i] = vertices[i];
	}
	// アプリで利用する3Dモデル=====================================================
	// 被写体の準備
	Model* model = Model::CreateFromOBJ("terrain");

	WorldTransformEx worldtransform;
	worldtransform.Initialize();
	worldtransform.scale_ = Vector3(1.0f, 1.0f, 1.0f);

	const Vector3 worldtransformRotation = worldtransform.rotation_;

	// カメラの準備
	Camera camera;
	camera.Initialize();
	camera.translation_ = Vector3(0.0f, 1.0f, 0.0f);

	bool isStop = false;
	const Vector3 cameraPos = Vector3(0.0f, 1.0f, 0.0f);

	// メインループ
	while (true) {
		// エンジンの更新
		if (Update()) {
			break;
		}

		if (Input::GetInstance()->TriggerKey(DIK_LEFT)) {
			if (changeSetPipelineState <= changeSetPipelineStateFirst) {
				changeSetPipelineState = changeSetPipelineStateMax;
			} else {
				changeSetPipelineState -= 1;
			}
		}
		if (Input::GetInstance()->TriggerKey(DIK_RIGHT)) {
			if (changeSetPipelineState >= changeSetPipelineStateMax) {
				changeSetPipelineState = changeSetPipelineStateFirst;
			} else {
				changeSetPipelineState += 1;
			}
		}
		// world変換行列の定数バッファへの転送
		worldtransform.UpdateMatrix();

		if (isStop) {
			worldtransform.rotation_ = worldtransformRotation;
			camera.translation_ = {0.0f,12.0f,-10.0f};
			camera.rotation_.x = 0.8f;
		} else {
			camera.translation_ = cameraPos;
			camera.rotation_.x = 0.0f;
		}

		// cameraの更新と定数バッファへの転送
		camera.UpdateMatrix();

		// ここに描画処理を記述する

		// TransitionBarrierをSRV⇒RTVに設定する
		D3D12_RESOURCE_BARRIER barrier{};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;                       // TranslationBarrierの設定
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;                            // フラグはNoneにしておく
		barrier.Transition.pResource = renderTextureResource.Get();                  // バリアを張る対象のリソース
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE; // 遷移前
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;          // 遷移後
		commandList->ResourceBarrier(1, &barrier);                                   // バリアを張る

		// 描画先のRTVとDSVを設定する
		commandList->OMSetRenderTargets(1, &rtvHandleCPU, false, &dsvHandleCPU);

		// Viewportの設定
		D3D12_VIEWPORT viewport{};
		viewport.Width = WinApp::kWindowWidth;
		viewport.Height = WinApp::kWindowHeight;
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.MinDepth = 0.0f; // 深度の最小値
		viewport.MaxDepth = 1.0f; // 深度の最大値

		commandList->RSSetViewports(1, &viewport);

		// Scissorの設定
		D3D12_RECT scissorRect{};
		// 基本的にビューポートと同じ矩形が構成されるようにする
		scissorRect.left = 0;
		scissorRect.right = WinApp::kWindowWidth;
		scissorRect.top = 0;
		scissorRect.bottom = WinApp::kWindowHeight;

		commandList->RSSetScissorRects(1, &scissorRect);

		// 画面クリア
		commandList->ClearRenderTargetView(rtvHandleCPU, kRenderTargetClearColor, 0, nullptr);
		// 指定した震度で画面全体をクリアにする
		commandList->ClearDepthStencilView(dsvHandleCPU, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

		// 描画
		Model::PreDraw(commandList.Get());
		model->Draw(worldtransform, camera);
		Model::PostDraw();

		// TransitionBarrierをもとに戻し、PixelShaderが扱えるようにする
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;                      // TranslationBarrierの設定
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;                           // フラグはNONEにしておく
		barrier.Transition.pResource = renderTextureResource.Get();                 // バリアを張る対象のリソース
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;        // 還移前
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE; // 還移後
		commandList->ResourceBarrier(1, &barrier);

		// 描画開始
		dxCommon->PreDraw();

		// コマンドを積む
		commandList->SetGraphicsRootSignature(rs.Get()); // RootSignatureの設定
		// PSOの設定をする
		if (changeSetPipelineState == 1) {
			isStop = false;
			worldtransform.rotation_.y += 0.005f; // 適当な回転角度(ラジアン)
			commandList->SetPipelineState(pipelineStateTest.Get());//grayscaleのセピア色
		} else if (changeSetPipelineState == 2) {
			isStop = true;
			commandList->SetPipelineState(pipelineStateVignette.Get());//vignetting
		} else if (changeSetPipelineState == 3) {
			commandList->SetPipelineState(pipelineStateBoxFilter.Get()); // BoxFilter
		} else if (changeSetPipelineState == 4) {
			commandList->SetPipelineState(pipelineStateBoxFilter5x5.Get());// BoxFilter5x5
		} else if (changeSetPipelineState == 5) {
			isStop = true;
			commandList->SetPipelineState(pipelineStateGaussianFilter.Get()); // GaussianFilter
		} else if (changeSetPipelineState == 6) {
			isStop = true;
			commandList->SetPipelineState(pipelineStateLuminanceBasedOutline.Get()); // LuminanceBasedOutline
		} else if (changeSetPipelineState ==7) {
			isStop = true;
			commandList->SetPipelineState(pipelineStateRadialBlur.Get()); // RadialBlur
		}

		commandList->IASetVertexBuffers(0, 1, vb.GetView()); // VBVの設定をする
		commandList->IASetIndexBuffer(ib.GetView());         // IBVの設定をする
		// トポロジの設定
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		// 使用するディスクリプタヒープの設定
		commandList->SetDescriptorHeaps(srvDescriptorHeap_->GetDesc().NumDescriptors, srvDescriptorHeap_.GetAddressOf());

		// SRVのDescripterTableの先頭を設定※0はrootParameter[0]である
		commandList->SetGraphicsRootDescriptorTable(0, srvHandleGPU);

		//// 頂点数、インスタンス数、インデックスの開始位置、インデックスのオフセット
		// commandList->DrawInstanced(3, 1, 0, 0);
		commandList->DrawIndexedInstanced(_countof(indices), 1, 0, 0, 0);

		// 描画終了
		dxCommon->PostDraw();
	}

	delete model;

	// エンジンの終了処理
	Finalize();

	return 0;
}
