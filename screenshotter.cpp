#include <windows.h>
#include <string>
#include <wininet.h>
#include <sstream>

#pragma comment(lib,"wininet.lib")

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

HWND hTextField;
HWND hStartButton;
std::wstring ipAddress;
bool bStarted = false;

const int NUM_PIXELS = 10;
POINT pixelPositions[NUM_PIXELS] = {
    {60, 15},
    {70, 20},
    {80, 25},
    {80, 25},
    {80, 25},
    {80, 25},
    {80, 25},
    {80, 25},
    {80, 25},
    {80, 25},
};

void TakeScreenshotAndSendRequest(const std::wstring& ipAddress) {
    std::wostringstream urlStream;
    urlStream << L"http://" << ipAddress << L"/api/color?";
    for (int i = 0; i < NUM_PIXELS; ++i) {
        HDC hdcScreen = GetDC(NULL);
        COLORREF color = GetPixel(hdcScreen, pixelPositions[i].x, pixelPositions[i].y);
        ReleaseDC(NULL, hdcScreen);
        WCHAR hexBuffer[8];
        swprintf_s(hexBuffer, L"%02X%02X%02X", GetRValue(color), GetGValue(color), GetBValue(color));
        std::wstring hexColor(hexBuffer);
        urlStream << L"hex" << i + 1 << L"=" << hexColor;
        if (i < NUM_PIXELS - 1) {
            urlStream << L"&";
        }
    }
    std::wstring url = urlStream.str();

    HINTERNET hInternet = InternetOpen(L"HTTPGET", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    HINTERNET hConnect = InternetOpenUrl(hInternet, url.c_str(), NULL, 0, 0, 0);
    InternetCloseHandle(hConnect);
    InternetCloseHandle(hInternet);
}

void StartScreenshotAndSend(HWND hwnd) {
    if (!bStarted) {
        bStarted = true;
        SetWindowText(hStartButton, L"Stop");
        SetTimer(hwnd, 1, 20, NULL);
    }
    else {
        bStarted = false;
        SetWindowText(hStartButton, L"Start");
        KillTimer(hwnd, 1);
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const wchar_t CLASS_NAME[] = L"Screenshotter";

    WNDCLASS wc = { };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW);

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        L"Screenshotter",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 500, 400,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (hwnd == NULL) {
        return 0;
    }

    RECT rect;
    GetClientRect(hwnd, &rect);

    int windowWidth = rect.right - rect.left;
    int windowHeight = rect.bottom - rect.top;

    hTextField = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
        (windowWidth - 300) / 2, (windowHeight - 30) / 2, 300, 20, hwnd, NULL, NULL, NULL);

    hStartButton = CreateWindowEx(NULL, L"BUTTON", L"Start", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        (windowWidth - 100) / 2, (windowHeight + 50) / 2, 100, 30, hwnd, (HMENU)1, NULL, NULL);

    ShowWindow(hwnd, nCmdShow);

    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_COMMAND: {
        if (LOWORD(wParam) == 1) {
            StartScreenshotAndSend(hwnd);
        }
        break;
    }
    case WM_TIMER: {
        if (wParam == 1) {
            if (bStarted) {
                wchar_t buffer[256];
                GetWindowText(hTextField, buffer, 256);
                ipAddress = std::wstring(buffer);
                TakeScreenshotAndSendRequest(ipAddress);
            }
        }
        break;
    }
    case WM_DESTROY: {
        PostQuitMessage(0);
        break;
    }
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}
