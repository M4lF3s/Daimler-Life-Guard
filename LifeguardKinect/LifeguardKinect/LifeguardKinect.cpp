// Defines the entry point for the application.

#include "stdafx.h"
#include "LifeguardKinect.h"
#include <FaceTrackLib.h>
#include "FTHelper.h"
#include <Shellapi.h>
#include <crtdbg.h>
#include <winhttp.h>
#include <strsafe.h>

namespace {
	const WCHAR* SERVER_NAME = L"127.0.0.1"; //L"192.168.10.1";
	const INTERNET_PORT SERVER_PORT = 1337; //INTERNET_DEFAULT_HTTP_PORT;
	const WCHAR* SERVER_PATH = L"/measurements";
}

class LifeguardKinect
{
public:
	LifeguardKinect()
		: m_hInst(NULL)
		, m_hWnd(NULL)
		, m_hAccelTable(NULL)
		, m_pImageBuffer(NULL)
		, m_pVideoBuffer(NULL)
		, m_depthType(NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX)
		, m_colorType(NUI_IMAGE_TYPE_COLOR)
		, m_depthRes(NUI_IMAGE_RESOLUTION_320x240)
		, m_colorRes(NUI_IMAGE_RESOLUTION_640x480)
		, m_bNearMode(TRUE)
		, m_bSeatedSkeletonMode(FALSE)
	{}

	int Run(HINSTANCE hInst, PWSTR lpCmdLine, int nCmdShow);

protected:
	BOOL                        InitInstance(HINSTANCE hInst, PWSTR lpCmdLine, int nCmdShow);
	void                        ParseCmdString(PWSTR lpCmdLine);
	void                        UninitInstance();
	ATOM                        RegisterClass(PCWSTR szWindowClass);
	static LRESULT CALLBACK     WndProcStatic(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK            WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK     About(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
	BOOL                        PaintWindow(HDC hdc, HWND hWnd);
	BOOL                        ShowVideo(HDC hdc, int width, int height, int originX, int originY);
	BOOL						PostDataToServer(IFTResult * pResult, FLOAT* eyesOpen);
	static void                 FTHelperCallingBack(LPVOID lpParam);
	static int const            MaxLoadStringChars = 100;

	HINSTANCE                   m_hInst;
	HWND                        m_hWnd;
	HACCEL                      m_hAccelTable;
	FTHelper                    m_FTHelper;
	IFTImage*                   m_pImageBuffer;
	IFTImage*                   m_pVideoBuffer;

	NUI_IMAGE_TYPE              m_depthType;
	NUI_IMAGE_TYPE              m_colorType;
	NUI_IMAGE_RESOLUTION        m_depthRes;
	NUI_IMAGE_RESOLUTION        m_colorRes;
	BOOL                        m_bNearMode;
	BOOL                        m_bSeatedSkeletonMode;

	FLOAT*						m_auCoefficients;
	UINT						m_auCount;
	FT_VECTOR2D*				m_facePoints;
	UINT						m_facePointCount;

	HINTERNET					m_hSession = NULL;
	HINTERNET					m_hConnect = NULL;
};

// Run the SingleFace application.
int LifeguardKinect::Run(HINSTANCE hInst, PWSTR lpCmdLine, int nCmdShow)
{
	MSG msg = { static_cast<HWND>(0), static_cast<UINT>(0), static_cast<WPARAM>(-1) };
	if(InitInstance(hInst, lpCmdLine, nCmdShow))
	{
		// Main message loop:
		while(GetMessage(&msg, NULL, 0, 0))
		{
			if(!TranslateAccelerator(msg.hwnd, m_hAccelTable, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}
	UninitInstance();

	return (int)msg.wParam;
}

// In this function, we save the instance handle, then create and display the main program window.
BOOL LifeguardKinect::InitInstance(HINSTANCE hInstance, PWSTR lpCmdLine, int nCmdShow)
{
	m_hInst = hInstance; // Store instance handle in our global variable

	ParseCmdString(lpCmdLine);

	WCHAR szTitle[MaxLoadStringChars];                  // The title bar text
	LoadString(m_hInst, IDS_APP_TITLE, szTitle, ARRAYSIZE(szTitle));

	static const PCWSTR RES_MAP[] = { L"80x60", L"320x240", L"640x480", L"1280x960" };
	static const PCWSTR IMG_MAP[] = { L"PLAYERID", L"RGB", L"YUV", L"YUV_RAW", L"DEPTH" };

	// Add mode params in title
	WCHAR szTitleComplete[MAX_PATH];
	swprintf_s(szTitleComplete, L"%s -- Depth:%s:%s Color:%s:%s NearMode:%s, SeatedSkeleton:%s", szTitle,
		IMG_MAP[m_depthType], (m_depthRes < 0) ? L"ERROR" : RES_MAP[m_depthRes], IMG_MAP[m_colorType], (m_colorRes < 0) ? L"ERROR" : RES_MAP[m_colorRes], m_bNearMode ? L"ON" : L"OFF",
		m_bSeatedSkeletonMode ? L"ON" : L"OFF");

	WCHAR szWindowClass[MaxLoadStringChars];            // the main window class name
	LoadString(m_hInst, IDC_LIFEGUARDKINECT, szWindowClass, ARRAYSIZE(szWindowClass));

	RegisterClass(szWindowClass);

	m_hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_LIFEGUARDKINECT));

	m_pImageBuffer = FTCreateImage();
	m_pVideoBuffer = FTCreateImage();

	m_hWnd = CreateWindow(szWindowClass, szTitleComplete, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, 660, 520, NULL, NULL, m_hInst, this);
	if(!m_hWnd)
	{
		return FALSE;
	}

	ShowWindow(m_hWnd, nCmdShow);
	UpdateWindow(m_hWnd);

	// Setup HTTP basics
	// Open Session
	m_hSession = WinHttpOpen(L"Lifeguard Kinect App/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);

	// Setup connection information
	if(m_hSession) {
		m_hConnect = WinHttpConnect(m_hSession, SERVER_NAME, SERVER_PORT, 0);
	}

	return SUCCEEDED(m_FTHelper.Init(m_hWnd,
		FTHelperCallingBack,
		this,
		m_depthType,
		m_depthRes,
		m_bNearMode,
		TRUE, // if near mode doesn't work, fall back to default mode
		m_colorType,
		m_colorRes,
		m_bSeatedSkeletonMode));
}

void LifeguardKinect::UninitInstance()
{
	// Clean up the memory allocated for Face Tracking and rendering.
	m_FTHelper.Stop();

	if(m_hAccelTable)
	{
		DestroyAcceleratorTable(m_hAccelTable);
		m_hAccelTable = NULL;
	}

	DestroyWindow(m_hWnd);
	m_hWnd = NULL;

	if(m_pImageBuffer)
	{
		m_pImageBuffer->Release();
		m_pImageBuffer = NULL;
	}

	if(m_pVideoBuffer)
	{
		m_pVideoBuffer->Release();
		m_pVideoBuffer = NULL;
	}

	if(m_hConnect) {
		WinHttpCloseHandle(m_hConnect);
	}
	if(m_hSession) {
		WinHttpCloseHandle(m_hSession);
	}
}


// Register the window class.
ATOM LifeguardKinect::RegisterClass(PCWSTR szWindowClass)
{
	WNDCLASSEX wcex = { 0 };

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = &LifeguardKinect::WndProcStatic;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = m_hInst;
	wcex.hIcon = LoadIcon(m_hInst, MAKEINTRESOURCE(IDC_LIFEGUARDKINECT));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCE(IDC_LIFEGUARDKINECT);
	wcex.lpszClassName = szWindowClass;

	return RegisterClassEx(&wcex);
}

LRESULT CALLBACK LifeguardKinect::WndProcStatic(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static LifeguardKinect* pThis = NULL; // cheating, but since there is just one window now, it will suffice.
	if(WM_CREATE == message)
	{
		pThis = reinterpret_cast<LifeguardKinect*>(reinterpret_cast<CREATESTRUCT*>(lParam)->lpCreateParams);
	}
	return pThis ? pThis->WndProc(hWnd, message, wParam, lParam) : DefWindowProc(hWnd, message, wParam, lParam);
}

//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_KEYUP    - Exit in response to ESC key
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
LRESULT CALLBACK LifeguardKinect::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	UINT wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch(message)
	{
	case WM_COMMAND:
		wmId = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		return DefWindowProc(hWnd, message, wParam, lParam);
		break;
	case WM_KEYUP:
		if(wParam == VK_ESCAPE)
		{
			PostQuitMessage(0);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// Draw the avatar window and the video window
		PaintWindow(hdc, hWnd);
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK LifeguardKinect::About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch(message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if(LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

// Drawing the video window
BOOL LifeguardKinect::ShowVideo(HDC hdc, int width, int height, int originX, int originY)
{
	BOOL ret = TRUE;

	// Now, copy a fraction of the camera image into the screen.
	IFTImage* colorImage = m_FTHelper.GetColorImage();
	if(colorImage)
	{
		int iWidth = colorImage->GetWidth();
		int iHeight = colorImage->GetHeight();
		if(iWidth > 0 && iHeight > 0)
		{
			int iTop = 0;
			int iBottom = iHeight;
			int iLeft = 0;
			int iRight = iWidth;

			// Keep a separate buffer.
			if(m_pVideoBuffer && SUCCEEDED(m_pVideoBuffer->Allocate(iWidth, iHeight, FTIMAGEFORMAT_UINT8_B8G8R8A8)))
			{
				// Copy do the video buffer while converting bytes
				colorImage->CopyTo(m_pVideoBuffer, NULL, 0, 0);

				// Compute the best approximate copy ratio.
				float w1 = (float)iHeight * (float)width;
				float w2 = (float)iWidth * (float)height;
				if(w2 > w1 && height > 0)
				{
					// video image too wide
					float wx = w1 / height;
					iLeft = (int)max(0, m_FTHelper.GetXCenterFace() - wx / 2);
					iRight = iLeft + (int)wx;
					if(iRight > iWidth)
					{
						iRight = iWidth;
						iLeft = iRight - (int)wx;
					}
				}
				else if(w1 > w2 && width > 0)
				{
					// video image too narrow
					float hy = w2 / width;
					iTop = (int)max(0, m_FTHelper.GetYCenterFace() - hy / 2);
					iBottom = iTop + (int)hy;
					if(iBottom > iHeight)
					{
						iBottom = iHeight;
						iTop = iBottom - (int)hy;
					}
				}

				int const bmpPixSize = m_pVideoBuffer->GetBytesPerPixel();
				SetStretchBltMode(hdc, HALFTONE);
				BITMAPINFO bmi = { sizeof(BITMAPINFO), iWidth, iHeight, 1, static_cast<WORD>(bmpPixSize * CHAR_BIT), BI_RGB, m_pVideoBuffer->GetStride() * iHeight, 5000, 5000, 0, 0 };
				if(0 == StretchDIBits(hdc, originX, originY, width, height,
					iLeft, iBottom, iRight - iLeft, iTop - iBottom, m_pVideoBuffer->GetBuffer(), &bmi, DIB_RGB_COLORS, SRCCOPY))
				{
					ret = FALSE;
				}
			}
		}
	}
	return ret;
}

// Draw the camera video with the mask superimposed.
BOOL LifeguardKinect::PaintWindow(HDC hdc, HWND hWnd)
{
	static int errCount = 0;
	BOOL ret = FALSE;
	RECT rect;
	GetClientRect(hWnd, &rect);
	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;
	int halfWidth = width / 2;

	// Show the video
	errCount += !ShowVideo(hdc, width, height, 0, 0);

	WCHAR buffer[256];

	FLOAT eyesOpen = m_FTHelper.GetEyesOpen();
	wsprintf(buffer, L"Eyes Open: %d", int(eyesOpen * 100));
	rect.top = 20;
	rect.bottom = 40;
	rect.left = 10;
	rect.right = 600;
	DrawText(hdc, buffer, -1, &rect, 0);

	return ret;
}

/*
* The "Face Tracker" helper class is generic. It will call back this function
* after a face has been successfully tracked. The code in the call back passes the parameters
* to the Egg Avatar, so it can be animated.
*/
void LifeguardKinect::FTHelperCallingBack(PVOID pVoid)
{
	LifeguardKinect* pApp = reinterpret_cast<LifeguardKinect*>(pVoid);
	if(pApp)
	{
		IFTResult* pResult = pApp->m_FTHelper.GetResult();
		IFTResult* resultParam = nullptr;
		FLOAT* pEyesOpen = nullptr;
		FLOAT fEyesOpen;
		if(pResult && SUCCEEDED(pResult->GetStatus()))
		{
			resultParam = pResult;
		}
		if(pApp->m_FTHelper.WasFaceSdkSuccessful()) {
			fEyesOpen = pApp->m_FTHelper.GetEyesOpen();
			pEyesOpen = &fEyesOpen;
		}

		pApp->PostDataToServer(pResult, pEyesOpen);
	}
}

BOOL LifeguardKinect::PostDataToServer(IFTResult* pResult, FLOAT* eyesOpen)
{
	BOOL bResult = FALSE;
	HINTERNET hRequest = NULL;
	BOOL contentAvailable = false;
	DWORD contentLength = 0;
	const int CONTENT_BUFFER_SIZE = 2048;
	CHAR content[CONTENT_BUFFER_SIZE] = {0};
	strcat_s<CONTENT_BUFFER_SIZE>(content, "{");

	// Prepare the Data to Send
	// Head pose
	FLOAT scale, rotation[3], translation[3];
	if(pResult && SUCCEEDED(pResult->Get3DPose(&scale, rotation, translation))) {
		if(rotation[0] != 0.f && rotation[1] != 0.f && rotation[2] != 0.f &&
			translation[0] != 0.f && translation[1] != 0.f && translation[2] != 0.f) {
			CHAR buffer[2048];
			StringCbPrintfA(buffer, sizeof(buffer),
				"\"pose\": {"
					"\"rotation\": {"
						"\"x\": %f,"
						"\"y\": %f,"
						"\"z\": %f"
					"},"
					"\"translation\": {"
						"\"x\": %f,"
						"\"y\": %f,"
						"\"z\": %f"
					"}"
				"}",
				rotation[0], rotation[1], rotation[2],
				translation[0], translation[1], translation[2]);

			strcat_s<CONTENT_BUFFER_SIZE>(content, buffer);
			contentAvailable = true;
		}
	}

	// Eyes open
	if(eyesOpen) {
		if(contentAvailable) { // If a pose is available, add a comma
			strcat_s<CONTENT_BUFFER_SIZE>(content, ",");
		}
		CHAR buffer[2048];
		StringCbPrintfA(buffer, sizeof(buffer),
			"\"eyesOpen\": %f", *eyesOpen);
		strcat_s<CONTENT_BUFFER_SIZE>(content, buffer);
		contentAvailable = true;
	}

	strcat_s<CONTENT_BUFFER_SIZE>(content, "}");
	contentLength = strlen(content);

	if(contentAvailable)
	{
		// Create HTTP Request
		if(m_hConnect) {
			hRequest = WinHttpOpenRequest(m_hConnect, L"POST", SERVER_PATH, NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0);
		}

		bResult = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, content, contentLength, contentLength, NULL);

		// End the request
		if(bResult) {
			bResult = WinHttpReceiveResponse(hRequest, NULL);
		}

		// Receive Response
		if(bResult) {
			DWORD dwSize = 0;
			WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_RAW_HEADERS_CRLF, WINHTTP_HEADER_NAME_BY_INDEX, 
				NULL, &dwSize, WINHTTP_NO_HEADER_INDEX);

			// Allocate memory to receive header
			if(GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
				WCHAR* headers = new WCHAR[dwSize / sizeof(WCHAR)];
				bResult = WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_RAW_HEADERS_CRLF, WINHTTP_HEADER_NAME_BY_INDEX, 
					headers, &dwSize, WINHTTP_NO_HEADER_INDEX);

				// Maybe inspect headers? Nah... :D

				delete[] headers;
			}
		}

		if(!bResult) {
			MessageBox(NULL, L"HTTP POST Error, no idea which one!", L"Error during HTTP POST", MB_OK);
		}

		// Clean up
		if(hRequest) {
			WinHttpCloseHandle(hRequest);
		}
	}

	return bResult;
}

void LifeguardKinect::ParseCmdString(PWSTR lpCmdLine)
{
	const WCHAR KEY_DEPTH[] = L"-Depth";
	const WCHAR KEY_COLOR[] = L"-Color";
	const WCHAR KEY_NEAR_MODE[] = L"-NearMode";
	const WCHAR KEY_DEFAULT_DISTANCE_MODE[] = L"-DefaultDistanceMode";
	const WCHAR KEY_SEATED_SKELETON_MODE[] = L"-SeatedSkeleton";

	const WCHAR STR_NUI_IMAGE_TYPE_DEPTH[] = L"DEPTH";
	const WCHAR STR_NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX[] = L"PLAYERID";
	const WCHAR STR_NUI_IMAGE_TYPE_COLOR[] = L"RGB";
	const WCHAR STR_NUI_IMAGE_TYPE_COLOR_YUV[] = L"YUV";

	const WCHAR STR_NUI_IMAGE_RESOLUTION_80x60[] = L"80x60";
	const WCHAR STR_NUI_IMAGE_RESOLUTION_320x240[] = L"320x240";
	const WCHAR STR_NUI_IMAGE_RESOLUTION_640x480[] = L"640x480";
	const WCHAR STR_NUI_IMAGE_RESOLUTION_1280x960[] = L"1280x960";

	enum TOKEN_ENUM
	{
		TOKEN_ERROR,
		TOKEN_DEPTH,
		TOKEN_COLOR,
		TOKEN_NEARMODE,
		TOKEN_DEFAULTDISTANCEMODE,
		TOKEN_SEATEDSKELETON
	};

	int argc = 0;
	LPWSTR *argv = CommandLineToArgvW(lpCmdLine, &argc);

	for(int i = 0; i < argc; i++)
	{
		NUI_IMAGE_TYPE* pType = NULL;
		NUI_IMAGE_RESOLUTION* pRes = NULL;

		TOKEN_ENUM tokenType = TOKEN_ERROR;
		PWCHAR context = NULL;
		PWCHAR token = wcstok_s(argv[i], L":", &context);
		if(0 == wcsncmp(token, KEY_DEPTH, ARRAYSIZE(KEY_DEPTH)))
		{
			tokenType = TOKEN_DEPTH;
			pType = &m_depthType;
			pRes = &m_depthRes;
		}
		else if(0 == wcsncmp(token, KEY_COLOR, ARRAYSIZE(KEY_COLOR)))
		{
			tokenType = TOKEN_COLOR;
			pType = &m_colorType;
			pRes = &m_colorRes;
		}
		else if(0 == wcsncmp(token, KEY_NEAR_MODE, ARRAYSIZE(KEY_NEAR_MODE)))
		{
			tokenType = TOKEN_NEARMODE;
			m_bNearMode = TRUE;
		}
		else if(0 == wcsncmp(token, KEY_DEFAULT_DISTANCE_MODE, ARRAYSIZE(KEY_DEFAULT_DISTANCE_MODE)))
		{
			tokenType = TOKEN_DEFAULTDISTANCEMODE;
			m_bNearMode = FALSE;
		}
		else if(0 == wcsncmp(token, KEY_SEATED_SKELETON_MODE, ARRAYSIZE(KEY_SEATED_SKELETON_MODE)))
		{
			tokenType = TOKEN_SEATEDSKELETON;
			m_bSeatedSkeletonMode = TRUE;
		}

		if(tokenType == TOKEN_DEPTH || tokenType == TOKEN_COLOR)
		{
			_ASSERT(pType != NULL && pRes != NULL);

			while((token = wcstok_s(NULL, L":", &context)) != NULL)
			{
				if(0 == wcsncmp(token, STR_NUI_IMAGE_TYPE_DEPTH, ARRAYSIZE(STR_NUI_IMAGE_TYPE_DEPTH)))
				{
					*pType = NUI_IMAGE_TYPE_DEPTH;
				}
				else if(0 == wcsncmp(token, STR_NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX, ARRAYSIZE(STR_NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX)))
				{
					*pType = NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX;
				}
				else if(0 == wcsncmp(token, STR_NUI_IMAGE_TYPE_COLOR, ARRAYSIZE(STR_NUI_IMAGE_TYPE_COLOR)))
				{
					*pType = NUI_IMAGE_TYPE_COLOR;
				}
				else if(0 == wcsncmp(token, STR_NUI_IMAGE_TYPE_COLOR_YUV, ARRAYSIZE(STR_NUI_IMAGE_TYPE_COLOR_YUV)))
				{
					*pType = NUI_IMAGE_TYPE_COLOR_YUV;
				}
				else if(0 == wcsncmp(token, STR_NUI_IMAGE_RESOLUTION_80x60, ARRAYSIZE(STR_NUI_IMAGE_RESOLUTION_80x60)))
				{
					*pRes = NUI_IMAGE_RESOLUTION_80x60;
				}
				else if(0 == wcsncmp(token, STR_NUI_IMAGE_RESOLUTION_320x240, ARRAYSIZE(STR_NUI_IMAGE_RESOLUTION_320x240)))
				{
					*pRes = NUI_IMAGE_RESOLUTION_320x240;
				}
				else if(0 == wcsncmp(token, STR_NUI_IMAGE_RESOLUTION_640x480, ARRAYSIZE(STR_NUI_IMAGE_RESOLUTION_640x480)))
				{
					*pRes = NUI_IMAGE_RESOLUTION_640x480;
				}
				else if(0 == wcsncmp(token, STR_NUI_IMAGE_RESOLUTION_1280x960, ARRAYSIZE(STR_NUI_IMAGE_RESOLUTION_1280x960)))
				{
					*pRes = NUI_IMAGE_RESOLUTION_1280x960;
				}
			}
		}
	}

	m_bNearMode = true;
	m_bSeatedSkeletonMode = true;

	if(argv) LocalFree(argv);
}


// Program's main entry point
int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR lpCmdLine, int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	LifeguardKinect app;

	HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

	return app.Run(hInstance, lpCmdLine, nCmdShow);
}
