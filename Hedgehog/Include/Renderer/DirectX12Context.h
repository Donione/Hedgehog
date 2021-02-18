#pragma once

#include <Renderer/RenderContext.h>

#include <Windows.h>

#include <d3d12.h>
#include <dxgi1_4.h>


class DirectX12Context : public RenderContext
{
public:
	struct FrameContext
	{
		ID3D12CommandAllocator* CommandAllocator;
		UINT64 FenceValue;
	};

public:
	DirectX12Context(HWND windowHandle);
	virtual ~DirectX12Context() override;

	void SetSwapInterval(int interval) override;

	void WaitForLastSubmittedFrame();
	FrameContext* WaitForNextFrameResources();

	void SwapBuffers() override;

private:
	void CreateRenderTarget();
	void CleanupRenderTarget();
	void ResizeSwapChain(HWND hWnd, int width, int height);


private:
	HWND windowHandle = NULL;

	int swapInterval = 0;


// TODO: everything is public for quick and dirty impementation, clean it up and abstract it
// NOTE: taken from ImGui examples for DX12
public:
	static int const NUM_FRAMES_IN_FLIGHT = 3;
	FrameContext g_frameContext[NUM_FRAMES_IN_FLIGHT] = {};
	UINT g_frameIndex = 0;

	// TODO maybe use ComPtr
	static int const NUM_BACK_BUFFERS = 3;
	ID3D12Device* g_pd3dDevice = NULL;
	ID3D12DescriptorHeap* g_pd3dRtvDescHeap = NULL;
	ID3D12CommandQueue* g_pd3dCommandQueue = NULL;
	ID3D12GraphicsCommandList* g_pd3dCommandList = NULL;
	ID3D12Fence* g_fence = NULL;
	HANDLE g_fenceEvent = NULL;
	UINT64 g_fenceLastSignaledValue = 0;
	IDXGISwapChain3* g_pSwapChain = NULL;
	HANDLE g_hSwapChainWaitableObject = NULL;
	ID3D12Resource* g_mainRenderTargetResource[NUM_BACK_BUFFERS] = {};
	D3D12_CPU_DESCRIPTOR_HANDLE  g_mainRenderTargetDescriptor[NUM_BACK_BUFFERS] = {};
};
