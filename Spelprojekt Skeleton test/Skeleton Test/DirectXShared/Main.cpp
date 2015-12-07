#include <windows.h>

#include "D3D.h"
#include "TimerClass.h"
#include <sstream>

#pragma comment (lib, "winmm.lib")

TimerClass timer;
HWND InitWindow(HINSTANCE hInstance);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

void calculateAverageFrames(HWND wndHandle)
{
	static int frameCount = 0;
	static float timeElapsed = 0.0f;

	frameCount++;

	if ((timer.time() - timeElapsed) > 1.0f)
	{
		float fps = (float)frameCount;
		float mspf = 1000.0f / fps;

		std::wostringstream outs;
		outs.precision(6);
		outs << L"Skeletal Animation" << L"    " << L"FPS:  " << fps << L"    " << L"Frame Time: " << mspf
			<< L"  (ms)";
		SetWindowText(wndHandle, outs.str().c_str());

		frameCount = 0;
		timeElapsed += 1.0f;
	}

}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	MSG msg = { 0 };
	HWND wndHandle = InitWindow(hInstance); //1. Skapa fönster
	float startTime = timeGetTime();
	float time = 0.0f;

	if (wndHandle)
	{
		D3D d3d(wndHandle);

		ShowWindow(wndHandle, nCmdShow);

		while (WM_QUIT != msg.message)
		{
			if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
			{
				float deltaTime = timeGetTime();
				if (deltaTime - startTime >= 16)
				{
					if (time <= 7.0f) {
						d3d.UpdateBones(time, 0);
						time += 0.05f;
					}
					else
						time = 0.0f;

					d3d.Update();
					d3d.Render();

					timer.Tick();
					calculateAverageFrames(wndHandle);
					startTime = deltaTime;
				}
			}
		}
		DestroyWindow(wndHandle);
	}
	return (int)msg.wParam;
}

HWND InitWindow(HINSTANCE hInstance)
{
	WNDCLASSEX wcex = { 0 };
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.hInstance = hInstance;
	wcex.lpszClassName = L"BTH_D3D_DEMO";
	if (!RegisterClassEx(&wcex))
		return false;

	RECT rc = { 0, 0, 640, 480 };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

	HWND handle = CreateWindow(
		L"BTH_D3D_DEMO",
		L"BTH Direct3D Demo",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		rc.right - rc.left,
		rc.bottom - rc.top,
		nullptr,
		nullptr,
		hInstance,
		nullptr);

	timer.Reset();

	return handle;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}