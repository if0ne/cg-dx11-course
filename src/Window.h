#pragma once
#include <stdint.h>
#include <windows.h>
#include <WinUser.h>

#include <d3d.h>
#include <d3d11.h>
#include <dxgi1_2.h>

#include "RenderContext.h"

class Game;

class Window
{
private:
	HWND hWnd_;
	HINSTANCE hInstance_;
	WNDCLASSEX wc_;

	uint32_t width_;
	uint32_t height_;

	IDXGISwapChain1* swapchain_;
	ID3D11Texture2D* backBuffer_;
	ID3D11RenderTargetView* renderView_;

	ID3D11Texture2D* depthStencilBuffer_;
	ID3D11DepthStencilView* depthStencilView_;

public:
    Window(uint32_t width, uint32_t height) : width_(width), height_(height) {}

	void Initialize(RenderContext& renderCtx);
	void ProcessEvent();
	void Show();

	void DestroyResources() {
		renderView_->Release();
		backBuffer_->Release();
		swapchain_->Release();
		depthStencilBuffer_->Release();
		depthStencilView_->Release();
	}

	HWND GetDescriptor() {
		return hWnd_;
	}

	uint32_t GetWidth() {
		return width_;
	}

	uint32_t GetHeight() {
		return height_;
	}

	float GetAspectRatio() {
		return (float)width_ / height_;
	}

	IDXGISwapChain1* GetSwapchain() {
		return swapchain_;
	}

	ID3D11Texture2D* GetBackBuffer() {
		return backBuffer_;
	}

	ID3D11RenderTargetView* GetRenderTarget() {
		return renderView_;
	}

	ID3D11DepthStencilView* GetDepthStencilView() {
		return depthStencilView_;
	}
};