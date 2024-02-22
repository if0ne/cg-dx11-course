#include "InputDevice.h"

#include <iostream>
#include <unordered_map>

#include "Game.h"

void InputDevice::Initialize() {
	RAWINPUTDEVICE Rid[2];

	Rid[0].usUsagePage = 0x01;
	Rid[0].usUsage = 0x02;
	Rid[0].dwFlags = 0;
	Rid[0].hwndTarget = ctx_.GetWindow().GetDescriptor();

	Rid[1].usUsagePage = 0x01;
	Rid[1].usUsage = 0x06;
	Rid[1].dwFlags = 0;
	Rid[1].hwndTarget = ctx_.GetWindow().GetDescriptor();

	if (RegisterRawInputDevices(Rid, 2, sizeof(Rid[0])) == FALSE)
	{
		auto errorCode = GetLastError();
		std::cout << "ERROR: " << errorCode << std::endl;
	}
}

void InputDevice::OnKeyDown(KeyboardInputEventArgs args) {
	bool Break = args.Flags & 0x01;

	auto key = static_cast<Keys>(args.VKey);

	if (args.MakeCode == 42) key = Keys::LeftShift;
	if (args.MakeCode == 54) key = Keys::RightShift;

	if (Break) {
		if (keys_->count(key)) RemovePressedKey(key);
	}
	else {
		if (!keys_->count(key)) AddPressedKey(key);
	}
}

void InputDevice::OnMouseMove(RawMouseEventArgs args) {
	if (args.ButtonFlags & static_cast<int>(MouseButtonFlags::LeftButtonDown))
		AddPressedKey(Keys::LeftButton);
	if (args.ButtonFlags & static_cast<int>(MouseButtonFlags::LeftButtonUp))
		RemovePressedKey(Keys::LeftButton);
	if (args.ButtonFlags & static_cast<int>(MouseButtonFlags::RightButtonDown))
		AddPressedKey(Keys::RightButton);
	if (args.ButtonFlags & static_cast<int>(MouseButtonFlags::RightButtonUp))
		RemovePressedKey(Keys::RightButton);
	if (args.ButtonFlags & static_cast<int>(MouseButtonFlags::MiddleButtonDown))
		AddPressedKey(Keys::MiddleButton);
	if (args.ButtonFlags & static_cast<int>(MouseButtonFlags::MiddleButtonUp))
		RemovePressedKey(Keys::MiddleButton);

	POINT p;
	GetCursorPos(&p);
	ScreenToClient(ctx_.GetWindow().GetDescriptor(), &p);

	MousePosition = DirectX::XMFLOAT2(p.x, p.y);
	MouseOffset = DirectX::XMFLOAT2(args.X, args.Y);
	MouseWheelDelta = args.WheelDelta;

	MouseMoveEventArgs moveArgs = { MousePosition, MouseOffset, MouseWheelDelta };

	for (const auto& evt : *mouseMoveEvents_) {
		evt.second(moveArgs);
	}
}
