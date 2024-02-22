#include "Window.h"

#include "Game.h"
#include "InputDevice.h"

LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam);

void Window::Initialize(RenderContext& renderCtx) {
	LPCWSTR applicationName = L"My3DApp";
	hInstance_ = GetModuleHandle(nullptr);

	wc_.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc_.lpfnWndProc = WndProc;
	wc_.cbClsExtra = 0;
	wc_.cbWndExtra = 0;
	wc_.hInstance = hInstance_;
	wc_.hIcon = LoadIcon(nullptr, IDI_WINLOGO);
	wc_.hIconSm = wc_.hIcon;
	wc_.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc_.hbrBackground = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
	wc_.lpszMenuName = nullptr;
	wc_.lpszClassName = applicationName;
	wc_.cbSize = sizeof(WNDCLASSEX);

	RegisterClassEx(&wc_);

	RECT windowRect = { 0, 0, static_cast<LONG>(width_), static_cast<LONG>(height_) };
	AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

	auto dwStyle = WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX | WS_THICKFRAME;

	auto posX = (GetSystemMetrics(SM_CXSCREEN) - width_) / 2;
	auto posY = (GetSystemMetrics(SM_CYSCREEN) - height_) / 2;

	hWnd_ = CreateWindowEx(
		WS_EX_APPWINDOW,
		applicationName,
		applicationName,
		dwStyle,
		posX, posY,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		nullptr, nullptr, hInstance_, nullptr
	);

	DXGI_SWAP_CHAIN_DESC1 swapDesc = {};
	swapDesc.BufferCount = 2;
	swapDesc.Width = width_;
	swapDesc.Height = height_;
	swapDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	swapDesc.Scaling = DXGI_SCALING_STRETCH;
	swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	swapDesc.SampleDesc.Count = 1;
	swapDesc.SampleDesc.Quality = 0;
	swapDesc.Stereo = false;
	swapDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;

	renderCtx.GetFactory()->CreateSwapChainForHwnd(renderCtx.GetDevice(), hWnd_, &swapDesc, nullptr, nullptr, &swapchain_);
	swapchain_->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer_);
	renderCtx.GetDevice()->CreateRenderTargetView(backBuffer_, nullptr, &renderView_);
}

void Window::Show() {
	ShowWindow(hWnd_, SW_SHOW);
	SetForegroundWindow(hWnd_);
	SetFocus(hWnd_);
	ShowCursor(true);
}

void Window::ProcessEvent() {
	MSG msg = {};

	while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam) {
	switch (umessage)
	{
	case WM_INPUT:
	{
		UINT dwSize = 0;
		GetRawInputData(reinterpret_cast<HRAWINPUT>(lparam), RID_INPUT, nullptr, &dwSize, sizeof(RAWINPUTHEADER));
		LPBYTE lpb = new BYTE[dwSize];
		if (lpb == nullptr) {
			return 0;
		}

		if (GetRawInputData((HRAWINPUT)lparam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)) != dwSize)
			OutputDebugString(TEXT("GetRawInputData does not return correct size !\n"));

		RAWINPUT* raw = reinterpret_cast<RAWINPUT*>(lpb);

		if (raw->header.dwType == RIM_TYPEKEYBOARD) {
			Game::GetSingleton().GetInputDevice().OnKeyDown({
				raw->data.keyboard.MakeCode,
				raw->data.keyboard.Flags,
				raw->data.keyboard.VKey,
				raw->data.keyboard.Message
				});
		}
		else if (raw->header.dwType == RIM_TYPEMOUSE) {
			Game::GetSingleton().GetInputDevice().OnMouseMove({
				raw->data.mouse.usFlags,
				raw->data.mouse.usButtonFlags,
				static_cast<int>(raw->data.mouse.ulExtraInformation),
				static_cast<int>(raw->data.mouse.ulRawButtons),
				static_cast<short>(raw->data.mouse.usButtonData),
				raw->data.mouse.lLastX,
				raw->data.mouse.lLastY
				});
		}

		delete[] lpb;
		return DefWindowProc(hwnd, umessage, wparam, lparam);
	}
	case WM_CLOSE:
	{
		Game::GetSingleton().Exit();
	}
	default:
	{
		return DefWindowProc(hwnd, umessage, wparam, lparam);
	}
	}
}
