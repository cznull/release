// mcd.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "mcd.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

HDC hdc1, hdc2;
HGLRC m_hrc;
int mx, my,cx,cy;
GLuint vbo[256], tbo[256], tex;
int (*block)[256][256];
int start;
movestate pe;
int isa = 0, isb = 0, isl = 0, isr = 0, isu = 0, isd = 0;
POINT clientpoint;
unsigned int ts, lts;
HBITMAP teximg;
elementtypes regionsupdate[256] = { 0 };

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
	
    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_MCD, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);
	teximg=LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_BITMAP1));

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MCD));
    MSG msg;
	int i;
	unsigned int time1, time2, dt = 0, count = 0;
	double lasttick, tickperiod;
	char s[32];
	tickperiod = 33.333;
	time1 = timeGetTime();
	lasttick = time1;
	lts = time1 - 201;
st:
	if (start) {
		for (;;) {
			if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				if (msg.message == WM_QUIT)
					break;
				if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
			}
			else if (start) {
				time2 = timeGetTime();
				dt += time2 - time1;
				while (lasttick + tickperiod < time2) {
					lasttick += tickperiod;
					move(&pe, lasttick - time1, block);
					updateintick(block, regionsupdate);
					time1 = lasttick;
				}
				move(&pe, time2 - time1, block);
				updatepoint(&pe, block);
				for (i = 0; i < 256; i++) {
					if (regionsupdate[i].opaque) {
						upregionsurface(block, i % 16, i / 16, 0, vbo, tbo);
						regionsupdate[i].opaque = 0;
					}
				}
				draw(&pe, vbo, tbo);
				count += 1;
				if (dt>= 1000) {
					sprintf_s(s, "%.2ffps ", count*1000.0 / dt);
					TextOutA(hdc2, 0, 0, s, strlen(s));
					dt = 0;
					count = 0;
				}
				time1 = time2;
			}
			else {
				goto st;
			}
		}
	}
	else {
		while (GetMessage(&msg, nullptr, 0, 0))
		{
			if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			if (start) {
				time1 = timeGetTime();
				goto st;
			}
		}
	}
    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MCD));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_MCD);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
	case WM_CREATE: {
		PIXELFORMATDESCRIPTOR pfd = {
			sizeof(PIXELFORMATDESCRIPTOR),
			1,
			PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER | PFD_STEREO,
			PFD_TYPE_RGBA,
			24,
			0,0,0,0,0,0,0,0,
			0,
			0,0,0,0,
			32,
			0,0,
			PFD_MAIN_PLANE,
			0,0,0,0
		};

		hdc1 = GetDC(hWnd);
		hdc2 = GetDC(NULL);
		int uds = ::ChoosePixelFormat(hdc1, &pfd);
		::SetPixelFormat(hdc1, uds, &pfd);
		m_hrc = ::wglCreateContext(hdc1);
		::wglMakeCurrent(hdc1, m_hrc);
		glewInit();

		glDisable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_ALPHA_TEST);
		//glEnable(GL_CULL_FACE);
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glAlphaFunc(GL_GREATER, 0.0f);

		pe.pos.x = 128.0;
		pe.pos.y = 130.0;
		pe.pos.z = 128.0;
		pe.v.x = 0.0;
		pe.v.y = 0.0;
		pe.v.z = 0.0;
		pe.ang1 = 0;
		pe.ang2 = 0;
		pe.flying = 1;
		pe.forward = 0;
		pe.left = 0;
		pe.inhand = 1;
		start = 0;

		((bool(_stdcall*)(int))wglGetProcAddress("wglSwapIntervalEXT"))(0);
		initblocks(&block, pe.pos);
		initview(&tex, vbo, tbo, block, teximg);

		break;
	}
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
			draw(&pe, vbo, tbo);
            EndPaint(hWnd, &ps);
        }
        break;
	case WM_SIZE: {
		cx = lParam & 0xffff;
		cy = (lParam & 0xffff0000) >> 16;
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glFrustum(-(float)cx / (cx + cy) / 5, (float)cx / (cx + cy) / 5, -(float)cy / (cx + cy) / 5, (float)cy / (cx + cy) / 5, 0.08, 1000.0);
		glViewport(0, 0, cx, cy);
		break;
	}
	case WM_MOUSEMOVE: {
		int x, y;
		x = (lParam & 0xffff);
		y = ((lParam & 0xffff0000) >> 16);
		if (start) {
			mx = (x - cx / 2);
			my = (y - cy / 2);
			if (mx || my) {
				pe.ang1 += mx * 0.004;
				pe.ang2 -= my * 0.004;
				pe.at.x = cos(pe.ang1)*cos(pe.ang2);
				pe.at.y = sin(pe.ang2);
				pe.at.z = sin(pe.ang1)*cos(pe.ang2);
				clientpoint.x = cx / 2;
				clientpoint.y = cy / 2;
				ClientToScreen(hWnd, &clientpoint);
				SetCursorPos(clientpoint.x, clientpoint.y);
			}
		}
		break;
	}
	case WM_KEYDOWN: {
		switch (wParam) {
		case(87):
			isa = 1;
			pe.forward = isa + isb;
			break;
		case(83):
			isb = -1;
			pe.forward = isa + isb;
			break;
		case(65):
			isl = 1;
			pe.left = isl + isr;
			break;
		case(68):
			isr = -1;
			pe.left = isl + isr;
			break;
		case(32):
			if (start) {
				ts = timeGetTime();
				if ((isu == 0) && (ts - lts) < 200) {
					pe.flying = !pe.flying;
				}
				lts = ts;
				if (pe.onlandstate.isonland) {
					pe.v.y = 7;
					pe.onlandstate.isonland = 0;
				}
			}
			isu = 1;
			pe.up = isu + isd;
			break;
		case(16):
			isd = -1;
			pe.up = isu + isd;
			break;
		case(66):
			/*block[0][0][0] = 1;
			upface();*/
			break;
		case(27):
			start = !start;
			if (start) {
				clientpoint.x = cx/2;
				clientpoint.y = cy/2;
				ClientToScreen(hWnd, &clientpoint);
				SetCursorPos(clientpoint.x, clientpoint.y);
			}
			break;
		case(114):
			//upface();
			break;
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
			pe.inhand = wParam - 48;
			break;
		}
		break;
	}
	case WM_KEYUP: {
		switch (wParam) {
		case(87):
			isa = 0;
			pe.forward = isa + isb;
			break;
		case(83):
			isb = 0;
			pe.forward = isa + isb;
			break;
		case(65):
			isl = 0;
			pe.left = isl + isr;
			break;
		case(68):
			isr = 0;
			pe.left = isl + isr;
			break;
		case(32):
			isu = 0;
			pe.up = isu + isd;
			break;
		case(16):
			isd = 0;
			pe.up = isu + isd;
			break;
		}
		break;
	}
	case WM_LBUTTONDBLCLK:
	case WM_LBUTTONDOWN: {
		if (start&&pe.point.ispoint) {
			removeblock(&pe, block, regionsupdate);
			updatepoint(&pe, block);
		}
		break;
	}
	case WM_RBUTTONDBLCLK:
	case WM_RBUTTONDOWN: {
		if (start&&pe.point.ispoint) {
			rightbutton(&pe, block, regionsupdate);
			updatepoint(&pe, block);
		}
		break;
	}
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

extern "C" {
	__declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}
extern "C" {
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}