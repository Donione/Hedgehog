#include <Renderer/DirectX12RendererAPI.h>

#include <glm/gtc/type_ptr.hpp>


void DirectX12RendererAPI::Init(RenderContext* renderContext)
{
	this->renderContext = dynamic_cast<DirectX12Context*>(renderContext);
}

void DirectX12RendererAPI::BeginFrame()
{
	// TODO ensure that BeginFrame and EndFrame are always called in pairs and BeginFrame is called first.

	frameCtxt = frameCtxt = renderContext->WaitForNextFrameResources();
	backBufferIdx = renderContext->g_pSwapChain->GetCurrentBackBufferIndex();
	frameCtxt->CommandAllocator->Reset();

	barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = renderContext->g_mainRenderTargetResource[backBufferIdx];
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

	renderContext->g_pd3dCommandList->Reset(frameCtxt->CommandAllocator, NULL);
	renderContext->g_pd3dCommandList->ResourceBarrier(1, &barrier);
	renderContext->g_pd3dCommandList->ClearRenderTargetView(renderContext->g_mainRenderTargetDescriptor[backBufferIdx],
															glm::value_ptr(clearColor),
															0,
															NULL);
	renderContext->g_pd3dCommandList->OMSetRenderTargets(1, &renderContext->g_mainRenderTargetDescriptor[backBufferIdx], FALSE, NULL);
	renderContext->g_pd3dCommandList->SetDescriptorHeaps(1, &renderContext->g_pd3dSrvDescHeap);
}

void DirectX12RendererAPI::EndFrame()
{
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;

	renderContext->g_pd3dCommandList->ResourceBarrier(1, &barrier);
	renderContext->g_pd3dCommandList->Close();

	renderContext->g_pd3dCommandQueue->ExecuteCommandLists(1, (ID3D12CommandList* const*)&renderContext->g_pd3dCommandList);

	renderContext->SwapBuffers();

	UINT64 fenceValue = renderContext->g_fenceLastSignaledValue + 1;
	renderContext->g_pd3dCommandQueue->Signal(renderContext->g_fence, fenceValue);
	renderContext->g_fenceLastSignaledValue = fenceValue;
	frameCtxt->FenceValue = fenceValue;
}
