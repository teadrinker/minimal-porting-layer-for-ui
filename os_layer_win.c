
/*

	OS dependent functions
	
	See "os_layer.h" for more information.

*/

#include "os_layer.h"


#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#include <windows.h>

#ifdef  __cplusplus
extern "C" {
#endif

#include <stdio.h>   // swprintf_S

#ifndef M_MALLOC
	#include <memory.h>
	#define mstd_malloc(a) ((unsigned char*)malloc(a))
	#define mstd_free(a) free(a)
	static unsigned char* mstd_malloc_z( unsigned int uiSize )		{ unsigned char* m=mstd_malloc(uiSize); memset(m,0,uiSize); return m; }

	#define M_MALLOC(type,size)			( (type *) mstd_malloc(		sizeof(type) * (size) ) )
	#define M_MALLOC_Z(type,size)		( (type *) mstd_malloc_z(	sizeof(type) * (size) ) )
	#define M_MALLOC_BYTES(size)		mstd_malloc(size)

	#define M_SAFE_FREE(p)				{ if(p) { mstd_free(p); p=0; } }
	#define M_FREE(p)					mstd_free(p);
#endif

#ifndef M_ASSERT
	#include <assert.h>
	#define M_ASSERT(a) assert(a)
#endif

#ifndef M_STRLEN
	// M_CONVERT is not nice...
#define M_CONVERT(t,a) ((t)(   (unsigned int)   ( (a<0)?((unsigned char)(a)):(a) )   ))
#define M_STRLEN(dst_count,string,datatype)				{ const datatype* m_strlen_tmp=string; dst_count=0; while(m_strlen_tmp[dst_count]) dst_count++; }
#define M_STRCPY(strdst,strsrc,datatypedst,datatypesrc) { datatypedst* m_strcpy_strdst=strdst; const datatypesrc* m_strcpy_strsrc=strsrc; while(*m_strcpy_strsrc) { *m_strcpy_strdst=M_CONVERT(datatypedst,*m_strcpy_strsrc); m_strcpy_strdst++; m_strcpy_strsrc++; } *m_strcpy_strdst=0; }
#define M_STRCAT(strdst,strsrc,datatypedst,datatypesrc) { datatypedst* m_strcpy_strdst=strdst; const datatypesrc* m_strcpy_strsrc=strsrc; while(*m_strcpy_strdst) m_strcpy_strdst++; while(*m_strcpy_strsrc) { *m_strcpy_strdst=M_CONVERT(datatypedst,*m_strcpy_strsrc); m_strcpy_strdst++; m_strcpy_strsrc++; } *m_strcpy_strdst=0; }
#define M_STRFLR(dst_count,string,query,datatype)		{ const datatype* m_strflr_tmp=string; dst_count=0; while(m_strflr_tmp[dst_count]) dst_count++; while(dst_count && m_strflr_tmp[dst_count]!=query) dst_count--; if(m_strflr_tmp[dst_count]!=query) dst_count=-1;  }
#endif

/*
void showlasterror()
{
	void * lpMsgBuf;
	FormatMessage( 
		FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		FORMAT_MESSAGE_FROM_SYSTEM | 
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPTSTR) &lpMsgBuf,
		0,
		NULL 
	);
	// Process any inserts in lpMsgBuf.
	// ...
	// Display the string.
	MessageBox( NULL, (LPCTSTR)lpMsgBuf, "Error", MB_OK | MB_ICONINFORMATION );
	// Free the buffer.
	LocalFree( lpMsgBuf );
}
*/

int WindowClassAddRef();
void WindowClassRelease();

HWND g_hwnd=0;
HINSTANCE g_hinstance;


/*	Globals
*******************************************************************************/
WNDCLASSEX		g_wcex;
BOOL			g_bReady = FALSE;
WCHAR			g_szAppName[256] = {L"ost"};


LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);


BOOL g_bIsActive=TRUE;

void os_init()
{

}

void os_shutdown()
{

}

void os_sleep(int a)
{
	Sleep(a);
}

void os_fastdebug(int a)
{
	if(a==0)
		Beep(4000,30);
	else
		Beep(a,30);
}



// BACKBUF
/*BITMAPINFO info;
info.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
info.bmiHeader.biWidth=port_w; 
info.bmiHeader.biHeight=-port_h; 
info.bmiHeader.biPlanes=1; 
info.bmiHeader.biBitCount=32;
info.bmiHeader.biCompression=BI_RGB; 
info.bmiHeader.biSizeImage=port_w*port_h*4;
info.bmiHeader.biXPelsPerMeter=0;
info.bmiHeader.biYPelsPerMeter=0;
info.bmiHeader.biClrUsed=0;
info.bmiHeader.biClrImportant=0;

screen_hdc=GetDC(port_hWnd);
backbuf_hdc=CreateCompatibleDC(screen_hdc); 
h_bm=CreateDIBSection(
    backbuf_hdc,	// handle to device context
    &info,			// pointer to structure containing bitmap size, format, and color data
    DIB_RGB_COLORS,	// color data type indicator: RGB values or palette indices
    &destscreen,	// pointer to variable to receive a pointer to the bitmap's bit values
    NULL,			// optional handle to a file mapping object
    0				// offset to the bitmap bit values within the file mapping object
   );
SelectObject(backbuf_hdc,h_bm);
screen_hdc=GetDC(port_hWnd);



BitBlt(screen_hdc,0,0,port_w,port_h,backbuf_hdc,0,0,SRCCOPY);
*/


// windows
typedef struct OLE_WRAPPER_STRUCT* OLE_WRAPPER_HANDLE;
OLE_WRAPPER_HANDLE osi_win32_ole_drop_target_create(HWND win, int wm_user_msg_for_dropped_file, int wm_user_msg_for_dropped_text);
void osi_win32_ole_drop_target_delete(HWND win, OLE_WRAPPER_HANDLE s);

#define WM_USER_MOUSEWHEEL		WM_USER+99999+1
#define WM_USER_KEYUP			WM_USER+99999+2
#define WM_USER_KEYDOWN			WM_USER+99999+3
//#define WM_USER_FORCEINVALIDATE	WM_USER+99999+4
#define WM_USER_DROPFILES		WM_USER+99999+5
#define WM_USER_DROPTEXT		WM_USER+99999+6

static HHOOK s_key_hook=0;
static OS_WINDOW s_key_hook_targetwin=0;
static HWND s_key_focus_prevwin=0;

typedef struct _OSI_WINDOW
{
    HWND			win32win;
    HFUNC_OS_MSG	fp_msg;
	OS_BACKBUFFER_PIXELS surface;
    OS_WINDOW_ID	id;	
	volatile int	bb_lock;
	int				global_array_id;
	int				send_paint_msg;

    uint			mcap;
    uint			mbutt[3];
//	int				popuphack;


	HDC				win32hdc_backbuffer;
	HDC				win32hdc_screen;
	HBITMAP			win32hbitmap_backbuffer;
	HGDIOBJ			old_gdiobj;
	HCURSOR			hcursor;

	OLE_WRAPPER_HANDLE ole_handle;
} OSI_WINDOW;


static void _add_to_global_array(struct _OSI_WINDOW * nw);
static void _remove_from_global_array(struct _OSI_WINDOW * nw);


static void _deletebackbuffer(OSI_WINDOW * nw)
{
	if(nw->old_gdiobj)
		SelectObject(nw->win32hdc_backbuffer,nw->old_gdiobj);
	if(nw->win32hbitmap_backbuffer)
		DeleteObject(nw->win32hbitmap_backbuffer);
	if(nw->win32hdc_backbuffer)
		DeleteDC(nw->win32hdc_backbuffer);
	if(nw->win32hdc_screen)
		ReleaseDC(nw->win32win,nw->win32hdc_screen);
}

static OS_BACKBUFFER_PIXELS* _os_window_lock_backbuffer(OS_WINDOW win)
{
	// volatileblock causes standard assert() to freeze if it is call between lock and unlock!
	int i = 1200;
	while (win->bb_lock && i)
	{
		Sleep(1);
		i--;
	}
	if (win->bb_lock)
	{
		M_ASSERT(0); // this never happens
	}

	win->bb_lock = 1;
	return &win->surface;
}

static void _os_window_unlock_backbuffer(OS_WINDOW win)
{
	win->bb_lock = 0;
}


static void _createbackbuffer(OSI_WINDOW * nw, int w, int h)
{
	// CreateBackBuffer
	void * destscreen;
	BITMAPINFO info;

	_os_window_lock_backbuffer(nw);


	if(nw->win32hdc_backbuffer)
		_deletebackbuffer(nw);

	info.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
	info.bmiHeader.biWidth=w; 
	info.bmiHeader.biHeight=-h; 
	info.bmiHeader.biPlanes=1; 
	info.bmiHeader.biBitCount=32;
	info.bmiHeader.biCompression=BI_RGB; 
	info.bmiHeader.biSizeImage=w*h*4;
	info.bmiHeader.biXPelsPerMeter=0;
	info.bmiHeader.biYPelsPerMeter=0;
	info.bmiHeader.biClrUsed=0;
	info.bmiHeader.biClrImportant=0;

	nw->win32hdc_screen=GetDC(nw->win32win);
	nw->win32hdc_backbuffer=CreateCompatibleDC(nw->win32hdc_screen); 
	nw->win32hbitmap_backbuffer=CreateDIBSection(
		nw->win32hdc_backbuffer,	// handle to device context
		&info,			// pointer to structure containing bitmap size, format, and color data
		DIB_RGB_COLORS,	// color data type indicator: RGB values or palette indices
		&destscreen,	// pointer to variable to receive a pointer to the bitmap's bit values
		NULL,			// optional handle to a file mapping object
		0				// offset to the bitmap bit values within the file mapping object
	   );

	nw->old_gdiobj=SelectObject(nw->win32hdc_backbuffer,nw->win32hbitmap_backbuffer);
//		nw->win32hdc_screen=GetDC(nw->win32win);

	nw->surface.w = w;
	nw->surface.h = h;
	nw->surface.data = (unsigned char*) destscreen;

	_os_window_unlock_backbuffer(nw);
}
/*
HHOOK g_oldhook=0;
HWND g_reallyuglyhack=0;
LRESULT CALLBACK KeyboardProc( int code, WPARAM wParam, LPARAM lParam)
{
	OS_MSGPARAM p;
	OSI_WINDOW * ww = (OSI_WINDOW *) GetWindowLong(g_reallyuglyhack,GWL_USERDATA);

	if(code==HC_ACTION)
	{
		if(lParam&(1<<31))
			WindowProc(g_reallyuglyhack,WM_KEYUP,wParam,lParam);
		else
			WindowProc(g_reallyuglyhack,WM_KEYDOWN,wParam,lParam);
	}

	return CallNextHookEx(g_oldhook,code,wParam,lParam);
}
*/

void os_window_create_from_native(OS_WINDOW* p_out, HFUNC_OS_MSG msgfunc, int x, int y, int w, int h, const char* name, OS_WINDOW_ID userid, uint flags, OS_NATIVEWINDOW win)
{
    OSI_WINDOW * nw = M_MALLOC_Z(OSI_WINDOW,1);
    nw->fp_msg = msgfunc;
    nw->id = userid;
	nw->send_paint_msg=1;
	*p_out=nw;

	WindowClassAddRef(flags);
	nw->hcursor=LoadCursor(0,IDC_ARROW);
/*
	nw->win32win = CreateWindowEx (0, g_szAppName, "Window",
			 WS_CHILD | WS_VISIBLE, 
			 0, 0, w, h, 
			 (HWND)win, NULL, g_hinstance, NULL);
*/

	// oh, this is a nice hack...
	nw->win32win = CreateWindowEx ( 0
		|WS_EX_ACCEPTFILES
		,L"EDIT", L"ostkaka", ES_LEFT | WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
		x, y, w, h, 
		(HWND)win, NULL, g_hinstance, 0);

	SetWindowLongPtr ((HWND)nw->win32win, GWLP_WNDPROC, (LONG_PTR)WindowProc);


    M_ASSERT(nw->win32win!=NULL);

	SetWindowLongPtr(nw->win32win,GWLP_USERDATA,(LONG_PTR)nw);

//	g_reallyuglyhack=nw->win32win;
//	g_oldhook=SetWindowsHookEx(WH_KEYBOARD,KeyboardProc,g_hinstance,0);

	nw->ole_handle=osi_win32_ole_drop_target_create(nw->win32win,WM_USER_DROPFILES,WM_USER_DROPTEXT);

	_createbackbuffer(nw,w,h);
   ShowWindow(nw->win32win, 1);
   UpdateWindow(nw->win32win);

	_add_to_global_array(nw);

}


// Function to convert const char* to WCHAR*
static WCHAR* ConvertCharToWChar(const char* str)
{
	int size = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);
	if (size == 0)
		return NULL;

	WCHAR* wstr = (WCHAR*)malloc(size * sizeof(WCHAR));
	if (wstr == NULL)
		return NULL;

	if (MultiByteToWideChar(CP_UTF8, 0, str, -1, wstr, size) == 0)
	{
		free(wstr);
		return NULL;
	}

	return wstr;
}

static WCHAR* ToWCHAR(const char* str, WCHAR* valueIfFail) {
	WCHAR* r = ConvertCharToWChar(str);
	if (r == NULL) return valueIfFail;
	return r;
}

int os_window_get_content_width( OS_WINDOW win) { return win->surface.w; }
int os_window_get_content_height(OS_WINDOW win) { return win->surface.h; }


void os_window_create(OS_WINDOW* p_out, HFUNC_OS_MSG msgfunc, int x, int y, int w, int h, const char* name, OS_WINDOW_ID userid, uint flags)
{
    OSI_WINDOW * nw = M_MALLOC_Z(OSI_WINDOW,1);
	DWORD dwFlags=WS_POPUP;
    nw->fp_msg = msgfunc;
    nw->id = userid;
	nw->send_paint_msg=1;
	*p_out=nw;

	nw->hcursor=LoadCursor(0,IDC_ARROW);


	if(!(	flags & OS_WIN_FLAG_HIDDEN))
		dwFlags |= WS_VISIBLE;

	if(		flags & OS_WIN_FLAG_TITLE)
		dwFlags |= WS_CAPTION;

	if(		flags & OS_WIN_FLAG_RESIZEABLE)
		dwFlags |= WS_SIZEBOX;

	if(		flags & OS_WIN_FLAG_MINIMIZEABLE)
		dwFlags |= WS_MINIMIZEBOX|WS_MINIMIZE|WS_SYSMENU|WS_CAPTION;

	if(		flags & OS_WIN_FLAG_CLOSEABLE)
		dwFlags |= WS_SYSMENU|WS_CAPTION;

	//WS_EX_TOPMOST
	WindowClassAddRef(flags);
	
	WCHAR* wname = ConvertCharToWChar(name);
	nw->win32win = CreateWindowEx( 0
								|WS_EX_ACCEPTFILES
								,g_szAppName, wname == NULL ? L"" : wname, dwFlags,
		 					   x, y, 
							   w + GetSystemMetrics(SM_CXFIXEDFRAME)*2, 
							   h + GetSystemMetrics(SM_CYFIXEDFRAME)*2+GetSystemMetrics(SM_CYCAPTION),
							   (HWND)NULL, (HMENU)NULL, g_hinstance, (LPVOID)NULL);

	if (wname != NULL)
		free(wname);

	{
		int tmp=0;
		RECT rr;
		GetClientRect(nw->win32win,&rr);
		tmp=1;
	}
//    nw->win32win = CreateWindowEx(WS_EX_ACCEPTFILES, g_szAppName, g_szAppName, WS_POPUP|WS_CAPTION|/*WS_SYSMENU|WS_MINIMIZEBOX|*/WS_THICKFRAME|WS_VISIBLE, x, y, w,h, NULL, NULL, g_hinstance, NULL);								   

//	nw->win32win = CreateWindow(g_szAppName, g_szAppName, WS_DLGFRAME |WS_VISIBLE,
//											CW_USEDEFAULT, CW_USEDEFAULT, w+GetSystemMetrics(SM_CXFIXEDFRAME)*2, h+GetSystemMetrics(SM_CYCAPTION)+GetSystemMetrics(SM_CXFIXEDFRAME)*2,
//											NULL, NULL, g_hinstance, NULL);

	SetWindowLongPtr((HWND)nw->win32win, GWLP_WNDPROC, (LONG_PTR)WindowProc);
//	showlasterror();
    M_ASSERT(nw->win32win!=NULL);

    SetWindowLongPtr(nw->win32win,GWLP_USERDATA,(LONG_PTR)nw);

	nw->ole_handle=osi_win32_ole_drop_target_create(nw->win32win,WM_USER_DROPFILES,WM_USER_DROPTEXT);

	_createbackbuffer(nw,w,h);

   ShowWindow(nw->win32win, 1);
   UpdateWindow(nw->win32win);
	if(flags&OS_WIN_FLAG_KEYBOARD_AUTOFOCUS)
		os_window_set_keyboard_focus(nw,1);
	_add_to_global_array(nw);

}


OS_TIMER os_window_timer_create(OS_WINDOW w, int time_in_ms, uint* userid)
{
//	s_timerf=f;
//	s_timer=SetTimer(0,0,time_in_ms,TimerProc);
//	return (OS_TIMER) s_timer;

	return (OS_TIMER)SetTimer(w->win32win,(UINT_PTR)userid,time_in_ms,0);
}


void os_window_timer_delete(OS_WINDOW w, OS_TIMER t)
{
	KillTimer(w->win32win,(UINT_PTR)t);
//	KillTimer(0,(uint)t);
}



/*LRESULT CALLBACK _os_key_proc(int code,WPARAM wParam,LPARAM lParam)
{
	M_ASSERT(s_key_targetwin);
//	PostMessage(s_key_targetwin->win32win,WM_USER_KEY,p->mouseData,lParam);

	if(code<0)
		return CallNextHookEx(0,code,wParam,lParam);
	return code;
}*/
static LRESULT CALLBACK _os_key_proc_ll(int code, WPARAM wParam, LPARAM lParam)
{
	BOOL fEatKeystroke = FALSE;
	if (code == HC_ACTION) 
	{
		switch (wParam) 
		{
		case WM_KEYDOWN:
//		case WM_SYSKEYDOWN:
		case WM_KEYUP:
//		case WM_SYSKEYUP:
			{
			KBDLLHOOKSTRUCT* p = (KBDLLHOOKSTRUCT*) lParam;
//			if(p->flags&(1<<4)) // don't care about injected
//				break;

			if(p->flags&(1<<7))
				PostMessage(s_key_hook_targetwin->win32win,WM_USER_KEYUP,p->vkCode,p->scanCode);
			else
				PostMessage(s_key_hook_targetwin->win32win,WM_USER_KEYDOWN,p->vkCode,p->scanCode);
			}

/*			fEatKeystroke =
			((p->vkCode == VK_TAB) && ((p->flags & LLKHF_ALTDOWN) != 0)) ||
			((p->vkCode == VK_ESCAPE) &&
			((p->flags & LLKHF_ALTDOWN) != 0)) ||
			((p->vkCode == VK_ESCAPE) && ((GetKeyState(VK_CONTROL) &
			0x8000) != 0));
			break;
			*/
			break;
		}
	}
//	if(code<0)
		return CallNextHookEx(0,code,wParam,lParam);
//	return code;
//	return (fEatKeystroke) ? 1 : CallNextHookEx(NULL, nCode, wParam, lParam);
}
void os_window_set_keyboard_focus(OS_WINDOW w,int state)
{
	if(state)
	{
		if(w!=s_key_hook_targetwin)
		{
			s_key_focus_prevwin=SetFocus(w->win32win);
			s_key_hook_targetwin=w;
		}
		if(s_key_hook==0)
			s_key_hook=SetWindowsHookEx(WH_KEYBOARD_LL,_os_key_proc_ll,g_hinstance,0);
	}
	else
	{
		if(s_key_hook)
		{
			UnhookWindowsHookEx(s_key_hook);
			s_key_hook=0;
		}
		if(s_key_hook_targetwin)
		{
			if(s_key_focus_prevwin) // this is check is needed because KILLFOCUS calls this func
			{
				SetFocus(s_key_focus_prevwin);
				s_key_focus_prevwin=0;
			}
			s_key_hook_targetwin=0;
		}
	}
}


void os_window_set_cursor(OS_WINDOW w, int id)
{
	switch(id)
	{
	default:
		w->hcursor=LoadCursor(0,IDC_ARROW);
		SetCursor(w->hcursor);
		return;

	case 2:
	//	w->hcursor=LoadCursor(0,IDC_WAIT);
	//	w->hcursor=LoadCursor(0,IDC_IBEAM);
	//	w->hcursor=LoadCursor(0,IDC_CROSS);
	//	w->hcursor=LoadCursor(0,IDC_SIZEALL);
		w->hcursor=LoadCursor(0,IDC_HAND);
		SetCursor(w->hcursor);
		return;
	};

}

int os_execute(const OS_PCHAR* str,const OS_PCHAR* args,const OS_PCHAR* dir,int flags, HFUNC_OS_EXECUTE_CALLBACK f, void* userdata)
{
	int i,len;
	WCHAR* wstr=0;
	WCHAR* wargs=0;
	WCHAR* wdir=0;
	WCHAR wopn[64];

	M_ASSERT(f==0); // not yet implemented

	if(str)
	{
		M_STRLEN(len,str,OS_PCHAR);
		wstr=M_MALLOC(WCHAR,len+1);
		M_STRCPY(wstr,str,WCHAR,OS_PCHAR);
		for(i=0;wstr[i];i++) if(wstr[i]=='/') wstr[i]='\\';
	}

	if(args)
	{
		M_STRLEN(len,args,OS_PCHAR);
		wargs=M_MALLOC(WCHAR,len+1);
		M_STRCPY(wargs,args,WCHAR,OS_PCHAR);
		for(i=0;wargs[i];i++) if(wargs[i]=='/') wargs[i]='\\';
	}

	if(dir)
	{
		M_STRLEN(len,dir,OS_PCHAR);
		wdir=M_MALLOC(WCHAR,len+1);
		M_STRCPY(wdir,dir,WCHAR,OS_PCHAR);
		for(i=0;wdir[i];i++) if(wdir[i]=='/') wdir[i]='\\';
	}

	M_STRCPY(wopn,"open",WCHAR,char);

//	if(flags&OS_EXECUTE_FLAG_VIEW_IN_OS_FILEBROWSER)
	ShellExecuteW(0,wopn,wstr,wargs,wdir, SW_SHOW);
//	else
	

	M_SAFE_FREE(wstr);
	M_SAFE_FREE(wargs);
	M_SAFE_FREE(wdir);

	return 1;
}

int os_key_is_down(OS_MSG_KEY_ID k)
{
	switch(k)
	{
	case OS_KEY_ALT:
		return (GetAsyncKeyState(VK_MENU)&0x8000)?1:0;
	case OS_KEY_SHIFT:
		return (GetAsyncKeyState(VK_SHIFT)&0x8000)?1:0;
	case OS_KEY_CONTROL:
		return (GetAsyncKeyState(VK_CONTROL)&0x8000)?1:0;
	case OS_KEY_UP:
		return (GetAsyncKeyState(VK_UP)&0x8000)?1:0;
	case OS_KEY_DOWN:
		return (GetAsyncKeyState(VK_DOWN)&0x8000)?1:0;
	case OS_KEY_LEFT:
		return (GetAsyncKeyState(VK_LEFT)&0x8000)?1:0;
	case OS_KEY_RIGHT:
		return (GetAsyncKeyState(VK_RIGHT)&0x8000)?1:0;
	}
	return 0;
}




void os_window_get_clientrect(OS_WINDOW nw, OS_RECT* r)
{
	POINT pt;
	RECT Rect;
	pt.x=0;
	pt.y=0;
	ClientToScreen(nw->win32win,&pt);
	r->x=pt.x;
	r->y=pt.y;
	GetClientRect(nw->win32win,&Rect);
	r->w=Rect.right;
	r->h=Rect.bottom;
}






void os_window_invalidate_rect(OS_WINDOW win, int x, int y, int w, int h)
{
	RECT r;
	r.left=x;
	r.top=y;
	r.right=x+w;
	r.bottom=y+h;

								//MDEBUG_PRINT(MDEBUG_DG_VERBOSE,"\nos_window_invalidate_rect START");

	InvalidateRect(win->win32win,&r,0);
	win->send_paint_msg=1;

								//MDEBUG_PRINT(MDEBUG_DG_VERBOSE,"\nos_window_invalidate_rect END");
//	BitBlt(win->win32hdc_screen,x,y,w,h,win->win32hdc_backbuffer,x,y,SRCCOPY);
}






static volatile int s_loop;
static int s_looprval;
static int _os_poll_msg(int time,HWND wnd)
{
    MSG msg;
	int r;

	if(time==-1)
		Sleep(1);
	else if(time!=0)
		Sleep(time);
        
    while(PeekMessage(&msg, wnd,0,0,PM_NOREMOVE))
	{
		r=GetMessage (&msg, wnd, 0, 0);
		if(r>0)
		{
			TranslateMessage(&msg);
		 	DispatchMessage(&msg);  
		}
		else if(r==0)
		{
			PostQuitMessage((int)msg.wParam);
			return 0;
		}
	}
	return 1;
}
static int _os_runloop(HWND w)
{
	s_loop=1;
	while(s_loop)
	{
		if(!_os_poll_msg(-1,w))
		{
			s_looprval=-1; // we got the quit message
			break;
		}
	}
	s_loop=1;

	return s_looprval;
}

int os_runloop() { return _os_runloop(0); }

void os_runloop_return(int val)
{
	s_looprval=val;
	s_loop=0;
}



int os_window_modal_runloop(OS_WINDOW w)
{
//	return _os_runloop(w->win32win); 
	return _os_runloop(0); // no modal
}

void os_window_modal_runloop_return(OS_WINDOW w,int return_value)
{
	os_runloop_return(return_value);
}

void os_poll_msg(int time)
{
	_os_poll_msg(time,0);
}

void os_window_delete(OS_WINDOW p)
{
	_remove_from_global_array(p);

	if(s_key_hook && s_key_hook_targetwin==p)
	{
		UnhookWindowsHookEx(s_key_hook);
		s_key_hook=0;
	}

	if(p->win32hdc_backbuffer)
	{
		_os_window_lock_backbuffer(p);
		_deletebackbuffer(p);
	}

	osi_win32_ole_drop_target_delete(p->win32win,p->ole_handle);

	// TODO ? this may generate calls (focuslost) to the user callback function, is this good?
	if(p->win32win)
		DestroyWindow(p->win32win);

    M_SAFE_FREE(p);

	WindowClassRelease();
}


void* os_window_get_native(OS_WINDOW w)
{
	return w->win32win;
}


/*
typedef struct _DROPFILESHACK
{
    DWORD pFiles;    // offset of file list
    POINT pt;        // drop point (client coords)
    BOOL fNC;        // is it on NonClient area and pt is in screen coords
    BOOL fWide;      // wide character flag
}DROPFILESHACK;

void os_drop_file_at_cursor(char*filename)
{
	int i,len=strlen(filename);
	unsigned short *wstr;
	HGLOBAL    hgDrop;
	DROPFILESHACK* pDrop;
	HWND dest=0;
	POINT cp;
	RECT cleantr;

	GetCursorPos(&cp);
	dest=WindowFromPoint(cp);
	if(!dest)
		return;

	GetClientRect(dest,&cleantr);
	cp.x-=cleantr.left;
	cp.y-=cleantr.top;

    // Allocate memory from the heap for the DROPFILES struct.
    hgDrop=GlobalAlloc ( GHND | GMEM_SHARE, sizeof(DROPFILESHACK)+(len+2)*2 );

    if(!hgDrop)
        return;

    pDrop=(DROPFILESHACK*) GlobalLock ( hgDrop );
    if(!pDrop)
    {
		GlobalFree(hgDrop);
		return;
    }

    // Fill in the DROPFILES struct.
    pDrop->pFiles=sizeof(DROPFILESHACK);
	pDrop->pt=cp;
    pDrop->fWide=TRUE;
	wstr=(unsigned short *) &(((char*)pDrop)[pDrop->pFiles]);
	for(i=0;i<len;i++)
		wstr[i]=filename[i];
	wstr[i++]=0;
	wstr[i++]=0;

	GlobalUnlock(hgDrop);

	PostMessage(dest,WM_DROPFILES,(WPARAM)hgDrop,0);

}
*/

#ifdef CF_OS_NO_OLE

int os_window_drag_files(OS_WINDOW win, int count, OS_PCHAR **filename_array)
{
	return 1;
}


int os_window_drag_text(OS_WINDOW win, char *text)
{
	return 1;
}
	
#endif



HMENU _createmenu_rec(int count, OS_WINDOW_POPUP_INFO* dat)
{
	HWND hWnd=0;//GetForegroundWindow();

    HMENU hMenu;
    MENUITEMINFO info;
	WCHAR tmpp[OS_PATH_MAX];
    int i;


	hMenu = CreatePopupMenu();

    if ( hMenu == 0 ) return 0;

    for( i = 0; i < count; i++ ) {

        memset( &info, 0, sizeof( info ) );

        info.cbSize = sizeof( info );

        info.fMask = MIIM_ID | MIIM_STATE | MIIM_TYPE;

        if ( dat[i].flags&OS_POPUP_SEPARATOR )
            info.fType = MFT_SEPARATOR;
        else 
		{
			info.fType = MFT_STRING;
			if(dat[i].name)
			{
				int j,k;
				k=0;
				for(j=0;dat[i].name[j];j++)
				{
					tmpp[k++]=dat[i].name[j];
					if(dat[i].name[j]=='&')
						tmpp[k++]='&';
				}
				tmpp[k]=0;

				info.dwTypeData = tmpp;
				info.cch = (unsigned int)(k+1);
			}
			else
			{
				info.dwTypeData = 0;
				info.cch = 0;
			}
        }

		info.fState = MFS_UNCHECKED;

		if ( dat[i].flags&OS_POPUP_CHECKED ) 
            info.fState |= MFS_CHECKED;

        if ( dat[i].flags&OS_POPUP_DISABLED ) 
            info.fState |= MFS_DISABLED;

//        if ( dat[i].flags&OS_POPUP_SELECTED ) 
//            info.fState |= MFS_HILITE;

        if ( dat[i].sub_count ) 
		{
            info.fMask = MIIM_ID | MIIM_STATE | MIIM_TYPE | MIIM_SUBMENU ;
            info.hSubMenu = _createmenu_rec( dat[i].sub_count, dat[i].sub );
        }

        info.wID = (unsigned int) (uintptr_t) &dat[i];//dat[i].id;

	   if ( !InsertMenuItem( hMenu, i, TRUE, &info ) ) 
		{
			int debug=0;
		}
    }

    return hMenu;

}

static OS_WINDOW_POPUP_INFO* _os_window_popup_finditem(int count, OS_WINDOW_POPUP_INFO* dat, unsigned int wIdSearch)
{
	int i;
	unsigned int wID;
	OS_WINDOW_POPUP_INFO *p;

    for (i = 0; i < count; i++ ) {

		if ( dat[i].sub_count )  {
            p = _os_window_popup_finditem( dat[i].sub_count, dat[i].sub, wIdSearch );
			if (p) return p;
        }

		wID = (unsigned int) (uintptr_t) &dat[i];
		if (wID == wIdSearch) return &dat[i];
    }

    return NULL;
}


OS_WINDOW_POPUP_INFO* os_window_popup(OS_WINDOW win, int x, int y, int count, OS_WINDOW_POPUP_INFO* dat)
{
//	HWND hwnd=GetForegroundWindow();
	HWND hwnd=win->win32win;
	HMENU hMenu=_createmenu_rec(count,dat);
	unsigned int wId;

	POINT pt;
	pt.x = 0;
	pt.y = 0;
	ClientToScreen(win->win32win, &pt);
	x += pt.x;
	y += pt.y;

	wId = (unsigned int)TrackPopupMenuEx( hMenu,
		TPM_LEFTALIGN | TPM_TOPALIGN | TPM_NONOTIFY | TPM_RETURNCMD | TPM_NOANIMATION // TPM_RIGHTBUTTON is BAD
		,x,y,hwnd,0);

//	PostMessage(hwnd, WM_NULL, 0, 0);

	DestroyMenu(hMenu);
//	PostMessage(hwnd,WM_USER_FORCEINVALIDATE,0,0);

	return _os_window_popup_finditem(count, dat, wId);
}




static unsigned int next_po2(unsigned int  x) { x |= (x >> 1); x |= (x >> 2); x |= (x >> 4); x |= (x >> 8); x |= (x >> 16); return x+1; }

static void po2arrayresize(int need, int num, void**a, int itemsize)
{
	unsigned int nsize=next_po2(need);
	if( nsize != next_po2(num) )
	{
		// TODO: add realloc support

		unsigned char *na=(unsigned char*)M_MALLOC_BYTES(nsize*itemsize);
		unsigned char *oa=(unsigned char*)(*a);
		int i,numis=num*itemsize;
		if(oa)
		{
			for(i=0;i<numis;i++)
				na[i]=oa[i];
			M_FREE(oa);
		}
		(*a)=na;
	}
}
#define M_ARRAY_ADD_ITEM(v,num,a,type)			{ po2arrayresize(  (num)+1,num,(void**)&(a),sizeof(type)); a[num]=v; (num)++; }



#ifndef CF_OS_LAYER_WIN32_NO_MOUSEHOOK
#define CF_OS_LAYER_WIN32_MOUSEHOOK
#endif

#ifdef CF_OS_LAYER_WIN32_MOUSEHOOK


#ifndef WH_MOUSE_LL	
#define WH_MOUSE_LL	14
#endif 

#ifndef WM_MOUSEWHEEL	
#define WM_MOUSEWHEEL 0x020A
#endif 
/*
#if !defined(PMSLLHOOKSTRUCT)
typedef struct {
	POINT pt;
	DWORD mouseData;
	DWORD flags;
	DWORD time;
	ULONG_PTR dwExtraInfo;
} MSLLHOOKSTRUCT, *PMSLLHOOKSTRUCT;
#endif
*/


static HHOOK g_mouse_hook = 0;
static struct _OSI_WINDOW ** g_window_array = 0;
static int g_window_array_count = 0;
static CRITICAL_SECTION cs;
static DWORD g_MouseHookThreadID=0;
static HANDLE g_MouseHookThread=0;
static volatile int g_ExitMouseHookThread=0;

static void _add_to_global_array(struct _OSI_WINDOW * nw)
{
	EnterCriticalSection(&cs);
	nw->global_array_id=g_window_array_count;
	M_ARRAY_ADD_ITEM(nw,g_window_array_count,g_window_array,OS_WINDOW*);
	LeaveCriticalSection(&cs);
}

static void _remove_from_global_array(struct _OSI_WINDOW * nw)
{
	EnterCriticalSection(&cs);
	g_window_array[nw->global_array_id]=g_window_array[--g_window_array_count];
	g_window_array[nw->global_array_id]->global_array_id=nw->global_array_id;
	if(g_window_array_count==0)
	{
		M_SAFE_FREE(g_window_array);
	}
	LeaveCriticalSection(&cs);
}

LRESULT CALLBACK low_level_mouse(int nCode, WPARAM wParam, LPARAM lParam)
{
	MSLLHOOKSTRUCT* p= (MSLLHOOKSTRUCT*) lParam;
	if (nCode==HC_ACTION)
	{
		if (wParam == WM_MOUSEWHEEL)
		{
			int i;
			EnterCriticalSection(&cs);
			for(i=0;i<g_window_array_count;i++)
				PostMessage(g_window_array[i]->win32win,WM_USER_MOUSEWHEEL,p->mouseData,lParam);
			LeaveCriticalSection(&cs);
		}
	}
	return CallNextHookEx(g_mouse_hook, nCode, wParam, lParam);
}

static void s_InitHook(int flags)
{
		if(flags&OS_WIN_FLAG_WIN32_MOUSEHOOK)
		{

			g_mouse_hook = SetWindowsHookEx(
					WH_MOUSE_LL,						// Hook in before msg reaches app
					(HOOKPROC) low_level_mouse,			// Hook procedure
					g_hinstance,						// This DLL instance
					0									// Hook in to all apps
					);
		}
}
static void s_DeInitHook()
{
		if(g_mouse_hook)
		{
			UnhookWindowsHookEx(g_mouse_hook);
			g_mouse_hook=0;
		}
		DeleteCriticalSection(&cs);
}
unsigned int WINAPI MouseHookThreadPump( void * lpParam ) 
{
	HWND wnd=NULL;
    MSG msg;
	s_InitHook((int)(uintptr_t)lpParam);
	while(!g_ExitMouseHookThread)
	{
		Sleep(1);
	    while(PeekMessage(&msg, wnd,0,0,PM_NOREMOVE))
		{
			if( GetMessage (&msg, wnd, 0, 0)>0 )
			{
				TranslateMessage(&msg);
			 	DispatchMessage(&msg);  
			}
		}
	}
	s_DeInitHook();
	return 0;
}


#include "process.h"
static void InitMouseHook(int flags)
{
	g_ExitMouseHookThread=0;

	InitializeCriticalSection(&cs);
    g_MouseHookThread=(HANDLE)_beginthreadex(NULL,0,MouseHookThreadPump,(void*)(intptr_t)flags,0,&g_MouseHookThreadID);   // returns the thread identifier 
	SetThreadPriority(g_MouseHookThread,THREAD_PRIORITY_HIGHEST);
}


static void DeinitMouseHook()
{
	g_ExitMouseHookThread=1;
	CloseHandle(g_MouseHookThread);
	g_MouseHookThread=0;
}

#else

static void _add_to_global_array(struct _OSI_WINDOW * nw){}
static void _remove_from_global_array(struct _OSI_WINDOW * nw){}

#endif

static long useCount = 0;

//-----------------------------------------------------------------------------
int WindowClassAddRef(unsigned int flags)
{
	useCount++;
	if (useCount == 1)
	{
		WNDCLASS windowClass;
		swprintf_s (g_szAppName, 256, L"winclass%p", g_hinstance);

#ifndef CF_OS_NO_OLE
		OleInitialize(0);
#endif	
		
		windowClass.style = CS_GLOBALCLASS;//|CS_OWNDC; // add Private-DC constant 

		windowClass.lpfnWndProc = WindowProc; 
		windowClass.cbClsExtra = 0; 
		windowClass.cbWndExtra = 0; 
		windowClass.hInstance = g_hinstance; 
		windowClass.hIcon = 0; 

		windowClass.hCursor = LoadCursor (NULL, IDC_ARROW);
		windowClass.hbrBackground = GetSysColorBrush (COLOR_BTNFACE); 
		windowClass.lpszMenuName = 0; 
		windowClass.lpszClassName = g_szAppName; 
		RegisterClass (&windowClass);

#ifdef CF_OS_LAYER_WIN32_MOUSEHOOK
		InitMouseHook(flags);
#endif
	}
	return 1;
}

//-----------------------------------------------------------------------------
void WindowClassRelease()
{
	useCount--;
	if (useCount == 0)
	{
		UnregisterClass (g_szAppName, g_hinstance);

#ifdef CF_OS_LAYER_WIN32_MOUSEHOOK
		DeinitMouseHook();
#endif
#ifndef CF_OS_NO_OLE
		OleUninitialize();
#endif	

	}
}

/*
BOOL WINAPI DllMain (HINSTANCE hInst, DWORD dwReason, LPVOID lpvReserved)
{
	g_hinstance = hInst;
	return 1;
}
*/

//#ifndef CF_OS_LAYER_DLL
#ifdef CF_OS_MAIN










int os_main(int argc, char **argv);

//extern int D3D_IsDeviceValid();

int MsgProcess()
{
	do
	{
		MSG msg;
		
		while(PeekMessage(&msg, NULL,0,0,PM_NOREMOVE))
		{
			if( GetMessage (&msg, NULL, 0, 0) )
			{
				TranslateMessage(&msg);
			 	DispatchMessage(&msg);  
			}
			if(msg.message==WM_QUIT)
			return 0;
		}

		// Check For Valid Device
//		if( D3D_IsDeviceValid() == FALSE )
//		{
//			Sleep( 10 );
//			continue;
//		}

		if( !g_bIsActive )
		{
			Sleep( 10 );
			continue;
		}
			

	}while(0);

	return 1;
}





//
//Some Windowsshit to emulate the original main...
//

static char    *fakeName = "_Reserved!";
static char    *argvbuf[32];
static char    cmdLineBuffer[1024];
char **
commandLineToArgv(LPSTR lpCmdLine, int *pArgc)
{
  char    *p, *pEnd;
  int     argc = 0;
  
  argvbuf[argc++] = fakeName;
  
  if (lpCmdLine == NULL) {
    *pArgc = argc;
    return argvbuf;
  }
  
  strcpy(cmdLineBuffer, lpCmdLine);
  p = cmdLineBuffer;
  pEnd = p + strlen(cmdLineBuffer);
  if (pEnd >= &cmdLineBuffer[1022]) pEnd = &cmdLineBuffer[1022];
  
  fflush(stdout);
  
  while (1) {
    // skip over white space 
    fflush(stdout);

    while (*p == ' ') p++;
    if (p >= pEnd) break;

    argvbuf[argc++] = p;
    if (argc >= 32) break;

    // skip till there's a 0 or a white space 
    while (*p && (*p != ' ')) p++;

    if (*p == ' ') *p++ = 0;
  }
  
  *pArgc = argc;
  return argvbuf;
}
//--------------------------------*/




#ifdef CF_MEM_TRACKER
	int memtracker_CurrentlyAllocated_GetInfo(char * buf, int max);
#endif

/*	WinMain:
*******************************************************************************/
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	int ret;
    int     argc;
    char    **argv;

	g_hinstance		= hInstance;
	/*

	// Workaround for level 4 varnings
	g_hinstance		= hInstance;
	hPrevInstance	= hPrevInstance;
	lpCmdLine		= lpCmdLine;
	nCmdShow		= nCmdShow;

	g_wcex.cbSize			= sizeof(WNDCLASSEX); 
	
	g_wcex.style			= CS_HREDRAW | CS_VREDRAW;
	g_wcex.lpfnWndProc		= (WNDPROC)WindowProc;
	g_wcex.cbClsExtra		= 0;
	g_wcex.cbWndExtra		= 0;
	g_wcex.hInstance		= hInstance;
	g_wcex.hIcon			= 0;//LoadIcon(hInstance, (LPCTSTR)IDI_MYICON);
	g_wcex.hCursor			= LoadCursor (NULL, IDC_ARROW);
	g_wcex.hbrBackground	= (HBRUSH)GetStockObject(BLACK_BRUSH);
	g_wcex.lpszClassName	= g_szAppName;
	g_wcex.hIconSm			= 0;//LoadIcon(g_wcex.hInstance, (LPCTSTR)IDI_MYICON);

	// -------------------------------------------------------------- RegisterClass
	if ( RegisterClassEx(&g_wcex) == FALSE )
	{
		return FALSE;
	}
*/

//	WindowClassAddRef();

    argv = commandLineToArgv(lpCmdLine, &argc);
	
//	MDEBUG_INIT(0,1,1);

	ret=os_main(argc,argv);

#ifdef CF_OS_LAYER_WIN32_MOUSEHOOK
	M_SAFE_FREE(g_window_array);
#endif

//	MDEBUG_SHUTDOWN();

//	WindowClassRelease();


#ifdef CF_MEM_TRACKER
	{
		char txt[80*20];
		int c=memtracker_CurrentlyAllocated_GetInfo(txt,20);
		OS_PCHAR filename[OS_PATH_MAX];
		M_STRCPY(filename,"C:\\mem.txt",OS_PCHAR,char);
		mf_save_textfile(filename,txt,0);
	}
#endif

//	CoUninitialize();

	return ret;

} // End Of WinMain


#endif













//SHGetFolderPathW



#define		OS_FILEDLG_FLAG_OPEN (1<<30)
#define		OS_FILEDLG_FLAG_SAVE (1<<29)


static int _filedlg(
					OS_PCHAR* out_path, 
					const OS_PCHAR* text, 
					const OS_PCHAR* def_path, 
					const OS_PCHAR* def_filename, 
					const OS_PCHAR* ext_filter, 
					unsigned int flags, 
					OS_WINDOW win)
{
	WCHAR text_w[OS_PATH_MAX]; 
	WCHAR def_path_w[OS_PATH_MAX]; 
	WCHAR def_filename_w[OS_PATH_MAX]; 
	WCHAR ext_filter_w[OS_PATH_MAX]; 
	WCHAR out_path_w[OS_PATH_MAX]; 
	HWND hwnd=0;//GetForegroundWindow();
	OPENFILENAMEW ofn;
	int i;
	int ext_index;

	if(win)
		hwnd=(HWND)os_window_get_native(win);

	M_STRCPY(text_w,text,WCHAR,OS_PCHAR);
	M_STRCPY(def_path_w,def_path,WCHAR,OS_PCHAR);
	M_STRCPY(def_filename_w,def_filename,WCHAR,OS_PCHAR);
	M_STRCPY(ext_filter_w,ext_filter,WCHAR,OS_PCHAR);

	M_STRCPY(out_path_w,def_path_w,WCHAR,WCHAR);
//	M_STRCAT(out_path_w,"\\",WCHAR,char);
	M_STRCAT(out_path_w,def_filename_w,WCHAR,WCHAR);

	// find extension
	M_STRFLR(ext_index,def_filename_w,'.',WCHAR);
	if(ext_index!=-1)
		ext_index++;

	for(i=0;def_path_w[i];i++) if(def_path_w[i]=='/') def_path_w[i]='\\';
	for(i=0;out_path_w[i];i++) if(out_path_w[i]=='/') out_path_w[i]='\\';

	// replace '|' with 0 for double termination of ext_filter_w
	i=0;
	while(ext_filter_w[i]) 
	{
		if(ext_filter_w[i]=='|')
			ext_filter_w[i]=0;
		i++;
	}

// debug:
// M_STRCPY(out_path,out_path_w,OS_PCHAR,WCHAR);
	
	

/* -------------------------------------------------------------------------*/
	memset(&ofn, '\0', sizeof(OPENFILENAMEW));

	ofn.lStructSize = sizeof(OPENFILENAMEW);
	ofn.hwndOwner = hwnd;
	ofn.hInstance = NULL;
	ofn.lpstrFilter = ext_filter_w;
	ofn.lpstrCustomFilter = NULL;
	ofn.nMaxCustFilter = 0;
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = out_path_w; // out full path
	ofn.nMaxFile = OS_PATH_MAX;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = OS_PATH_MAX;
	ofn.lpstrInitialDir = def_path_w;
	ofn.lpstrTitle = text_w;
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR | OFN_HIDEREADONLY | ((flags&OS_FILEDLG_FLAG_FILEEXISTALERT)?OFN_OVERWRITEPROMPT:0);
	ofn.nFileOffset = 0;
	ofn.nFileExtension = 0;

	if(ext_index!=-1)
		ofn.lpstrDefExt = &def_filename_w[ext_index];

	ofn.lCustData = 0;
	ofn.lpfnHook = NULL;
	ofn.lpTemplateName = NULL;



	if(flags&OS_FILEDLG_FLAG_SAVE)
		i=GetSaveFileNameW(&ofn);
	else
		i=GetOpenFileNameW(&ofn);

	if(i==0)
		return 0;

	M_STRCPY(out_path,out_path_w,OS_PCHAR,WCHAR);
	for(i=0;out_path[i];i++) if(out_path[i]=='\\') out_path[i]='/';

 //  istat = GetOpenFileName(&ofn);
 //  if (istat == 0) return(0);


 //  strncpy(dirBuffer, tsFn, ofn.nFileOffset);
 //  *(dirBuffer + ofn.nFileOffset) = '\0';
	return 1;
}

int os_filedlg_open( OS_PCHAR* out_path, const OS_PCHAR* text, const OS_PCHAR* def_path, const OS_PCHAR* def_filename, const OS_PCHAR* def_ext, unsigned int flags, OS_WINDOW win)
{
	return _filedlg(out_path,text,def_path,def_filename,def_ext,flags|OS_FILEDLG_FLAG_OPEN,win);
}

int os_filedlg_save( OS_PCHAR* out_path, const OS_PCHAR* text, const OS_PCHAR* def_path, const OS_PCHAR* def_filename, const OS_PCHAR* def_ext, unsigned int flags, OS_WINDOW win)
{
	return _filedlg(out_path,text,def_path,def_filename,def_ext,flags|OS_FILEDLG_FLAG_SAVE,win);
}











static int s_mxPos=0;
static int s_myPos=0;

unsigned int GetMouseX()
{
	return s_mxPos;
}

unsigned int GetMouseY()
{
	return s_myPos;
}
//static HCURSOR s_hCursor;

BOOL g_bMouseEnable=FALSE;
void SetMouseEnable(BOOL b)
{
	g_bMouseEnable=b;
//	SetCursor(s_hCursor);
	if(b)
		while(ShowCursor( TRUE )<=0);
	else
		while(ShowCursor( FALSE )>0);
}

#ifdef _DEBUG

const char * get_msg_name(int msg)
{
	static char tmp[32];
	if(msg>0x8000)
	{
		sprintf_s(tmp, 32, "WM_APP%d",msg);
		return tmp;
	}
	if(msg>0x0400)
	{
		sprintf_s(tmp, 32, "WM_USER%d",msg);
		return tmp;
	}

	switch(msg)
	{
	case 0x0000: return "WM_NULL";
	case 0x0001: return "WM_CREATE";                       
	case 0x0002: return "WM_DESTROY";                      
	case 0x0003: return "WM_MOVE";                         
	case 0x0005: return "WM_SIZE";                         
	case 0x0006: return "WM_ACTIVATE";                     
	case 0x0007: return "WM_SETFOCUS";                     
	case 0x0008: return "WM_KILLFOCUS";                    
	case 0x000A: return "WM_ENABLE";                       
	case 0x000B: return "WM_SETREDRAW";                    
	case 0x000C: return "WM_SETTEXT";                      
	case 0x000D: return "WM_GETTEXT";                      
	case 0x000E: return "WM_GETTEXTLENGTH";                
	case 0x000F: return "WM_PAINT";                        
	case 0x0010: return "WM_CLOSE";                        
	case 0x0011: return "WM_QUERYENDSESSION";              
	case 0x0013: return "WM_QUERYOPEN";                    
	case 0x0016: return "WM_ENDSESSION";                   
	case 0x0012: return "WM_QUIT";                         
	case 0x0014: return "WM_ERASEBKGND";                   
	case 0x0015: return "WM_SYSCOLORCHANGE";               
	case 0x0018: return "WM_SHOWWINDOW";                   
	case 0x001A: return "WM_WININICHANGE";                 
	case 0x001B: return "WM_DEVMODECHANGE";                
	case 0x001C: return "WM_ACTIVATEAPP";                  
	case 0x001D: return "WM_FONTCHANGE";                   
	case 0x001E: return "WM_TIMECHANGE";                   
	case 0x001F: return "WM_CANCELMODE";                   
	case 0x0020: return "WM_SETCURSOR";                    
	case 0x0021: return "WM_MOUSEACTIVATE";                
	case 0x0022: return "WM_CHILDACTIVATE";                
	case 0x0023: return "WM_QUEUESYNC";                    
	case 0x0024: return "WM_GETMINMAXINFO";                
	case 0x0026: return "WM_PAINTICON";                    
	case 0x0027: return "WM_ICONERASEBKGND";               
	case 0x0028: return "WM_NEXTDLGCTL";                   
	case 0x002A: return "WM_SPOOLERSTATUS";                
	case 0x002B: return "WM_DRAWITEM";                     
	case 0x002C: return "WM_MEASUREITEM";                  
	case 0x002D: return "WM_DELETEITEM";                   
	case 0x002E: return "WM_VKEYTOITEM";                   
	case 0x002F: return "WM_CHARTOITEM";                   
	case 0x0030: return "WM_SETFONT";                      
	case 0x0031: return "WM_GETFONT";                      
	case 0x0032: return "WM_SETHOTKEY";                    
	case 0x0033: return "WM_GETHOTKEY";                    
	case 0x0037: return "WM_QUERYDRAGICON";                
	case 0x0039: return "WM_COMPAREITEM";                  
	case 0x003D: return "WM_GETOBJECT";                    
	case 0x0041: return "WM_COMPACTING";                   
	case 0x0044: return "WM_COMMNOTIFY";
	case 0x0046: return "WM_WINDOWPOSCHANGING";            
	case 0x0047: return "WM_WINDOWPOSCHANGED";             
	case 0x0048: return "WM_POWER";                        
	case 0x004A: return "WM_COPYDATA";                     
	case 0x004B: return "WM_CANCELJOURNAL";                
	case 0x004E: return "WM_NOTIFY";                       
	case 0x0050: return "WM_INPUTLANGCHANGEREQUEST";       
	case 0x0051: return "WM_INPUTLANGCHANGE";              
	case 0x0052: return "WM_TCARD";                        
	case 0x0053: return "WM_HELP";                         
	case 0x0054: return "WM_USERCHANGED";                  
	case 0x0055: return "WM_NOTIFYFORMAT";                 
	case 0x007B: return "WM_CONTEXTMENU";                  
	case 0x007C: return "WM_STYLECHANGING";                
	case 0x007D: return "WM_STYLECHANGED";                 
	case 0x007E: return "WM_DISPLAYCHANGE";                
	case 0x007F: return "WM_GETICON";                      
	case 0x0080: return "WM_SETICON";                      
	case 0x0081: return "WM_NCCREATE";                     
	case 0x0082: return "WM_NCDESTROY";                    
	case 0x0083: return "WM_NCCALCSIZE";                   
	case 0x0084: return "WM_NCHITTEST";                    
	case 0x0085: return "WM_NCPAINT";                      
	case 0x0086: return "WM_NCACTIVATE";                   
	case 0x0087: return "WM_GETDLGCODE";                   
	case 0x00A0: return "WM_NCMOUSEMOVE";                  
	case 0x00A1: return "WM_NCLBUTTONDOWN";                
	case 0x00A2: return "WM_NCLBUTTONUP";                  
	case 0x00A3: return "WM_NCLBUTTONDBLCLK";              
	case 0x00A4: return "WM_NCRBUTTONDOWN";                
	case 0x00A5: return "WM_NCRBUTTONUP";                  
	case 0x00A6: return "WM_NCRBUTTONDBLCLK";              
	case 0x00A7: return "WM_NCMBUTTONDOWN";                
	case 0x00A8: return "WM_NCMBUTTONUP";                  
	case 0x00A9: return "WM_NCMBUTTONDBLCLK";              
	case 0x00AB: return "WM_NCXBUTTONDOWN";                
	case 0x00AC: return "WM_NCXBUTTONUP";                  
	case 0x00AD: return "WM_NCXBUTTONDBLCLK";              
//	case 0x0100: return "WM_KEYFIRST";                     
	case 0x0100: return "WM_KEYDOWN";                      
	case 0x0101: return "WM_KEYUP";                        
	case 0x0102: return "WM_CHAR";                         
	case 0x0103: return "WM_DEADCHAR";                     
	case 0x0104: return "WM_SYSKEYDOWN";                   
	case 0x0105: return "WM_SYSKEYUP";                     
	case 0x0106: return "WM_SYSCHAR";                      
	case 0x0107: return "WM_SYSDEADCHAR";                  
	case 0x0109: return "WM_UNICHAR";                      
//	case 0x0109: return "WM_KEYLAST";                      
	case 0x0108: return "WM_KEYLAST";                      
	case 0x010D: return "WM_IME_STARTCOMPOSITION";         
	case 0x010E: return "WM_IME_ENDCOMPOSITION";           
	case 0x010F: return "WM_IME_COMPOSITION";              
//	case 0x010F: return "WM_IME_KEYLAST";                  
	case 0x0110: return "WM_INITDIALOG";                   
	case 0x0111: return "WM_COMMAND";                      
	case 0x0112: return "WM_SYSCOMMAND";
	case 0x0113: return "WM_TIMER";                        
	case 0x0114: return "WM_HSCROLL";                      
	case 0x0115: return "WM_VSCROLL";                      
	case 0x0116: return "WM_INITMENU";                     
	case 0x0117: return "WM_INITMENUPOPUP";                
	case 0x011F: return "WM_MENUSELECT";                   
	case 0x0120: return "WM_MENUCHAR";                     
	case 0x0121: return "WM_ENTERIDLE";                    
	case 0x0122: return "WM_MENURBUTTONUP";                
	case 0x0123: return "WM_MENUDRAG";                     
	case 0x0124: return "WM_MENUGETOBJECT";                
	case 0x0125: return "WM_UNINITMENUPOPUP";              
	case 0x0126: return "WM_MENUCOMMAND";
	case 0x0127: return "WM_CHANGEUISTATE";
	case 0x0128: return "WM_UPDATEUISTATE";                
	case 0x0129: return "WM_QUERYUISTATE";                 
	case 0x0132: return "WM_CTLCOLORMSGBOX";               
	case 0x0133: return "WM_CTLCOLOREDIT";                 
	case 0x0134: return "WM_CTLCOLORLISTBOX";              
	case 0x0135: return "WM_CTLCOLORBTN";                  
	case 0x0136: return "WM_CTLCOLORDLG";                  
	case 0x0137: return "WM_CTLCOLORSCROLLBAR";            
	case 0x0138: return "WM_CTLCOLORSTATIC";               
	case 0x01E1: return "MN_GETHMENU";                     
//	case 0x0200: return "WM_MOUSEFIRST";                   
	case 0x0200: return "WM_MOUSEMOVE";                    
	case 0x0201: return "WM_LBUTTONDOWN";                  
	case 0x0202: return "WM_LBUTTONUP";                    
	case 0x0203: return "WM_LBUTTONDBLCLK";                
	case 0x0204: return "WM_RBUTTONDOWN";                  
	case 0x0205: return "WM_RBUTTONUP";                    
	case 0x0206: return "WM_RBUTTONDBLCLK";                
	case 0x0207: return "WM_MBUTTONDOWN";                  
	case 0x0208: return "WM_MBUTTONUP";                    
	case 0x0209: return "WM_MBUTTONDBLCLK";                
	case 0x020A: return "WM_MOUSEWHEEL";                   
	case 0x020B: return "WM_XBUTTONDOWN";                  
	case 0x020C: return "WM_XBUTTONUP";                    
	case 0x020D: return "WM_XBUTTONDBLCLK";                
//	case 0x020D: return "WM_MOUSELAST";                    
//	case 0x020A: return "WM_MOUSELAST 1";                   
//	case 0x0209: return "WM_MOUSELAST 2";                    
	case 0x0210: return "WM_PARENTNOTIFY";                 
	case 0x0211: return "WM_ENTERMENULOOP";                
	case 0x0212: return "WM_EXITMENULOOP";                 
	case 0x0213: return "WM_NEXTMENU";                     
	case 0x0214: return "WM_SIZING";                       
	case 0x0215: return "WM_CAPTURECHANGED";               
	case 0x0216: return "WM_MOVING";                       
	case 0x0218: return "WM_POWERBROADCAST";               
	case 0x0220: return "WM_MDICREATE";                    
	case 0x0221: return "WM_MDIDESTROY";                   
	case 0x0222: return "WM_MDIACTIVATE";                  
	case 0x0223: return "WM_MDIRESTORE";                   
	case 0x0224: return "WM_MDINEXT";                      
	case 0x0225: return "WM_MDIMAXIMIZE";                  
	case 0x0226: return "WM_MDITILE";                      
	case 0x0227: return "WM_MDICASCADE";                   
	case 0x0228: return "WM_MDIICONARRANGE";               
	case 0x0229: return "WM_MDIGETACTIVE";                 
	case 0x0230: return "WM_MDISETMENU";                   
	case 0x0231: return "WM_ENTERSIZEMOVE";                
	case 0x0232: return "WM_EXITSIZEMOVE";                 
	case 0x0233: return "WM_DROPFILES";                    
	case 0x0234: return "WM_MDIREFRESHMENU";               
	case 0x0281: return "WM_IME_SETCONTEXT";               
	case 0x0282: return "WM_IME_NOTIFY";                   
	case 0x0283: return "WM_IME_CONTROL";                  
	case 0x0284: return "WM_IME_COMPOSITIONFULL";          
	case 0x0285: return "WM_IME_SELECT";                   
	case 0x0286: return "WM_IME_CHAR";                     
	case 0x0288: return "WM_IME_REQUEST";                  
	case 0x0290: return "WM_IME_KEYDOWN";                  
	case 0x0291: return "WM_IME_KEYUP";                    
	case 0x02A1: return "WM_MOUSEHOVER";                   
	case 0x02A3: return "WM_MOUSELEAVE";                   
	case 0x02A0: return "WM_NCMOUSEHOVER";                 
	case 0x02A2: return "WM_NCMOUSELEAVE";                 
	case 0x02B1: return "WM_WTSSESSION_CHANGE";            
	case 0x02c0: return "WM_TABLET_FIRST";                 
	case 0x02df: return "WM_TABLET_LAST";                  
	case 0x0300: return "WM_CUT";                          
	case 0x0301: return "WM_COPY";                         
	case 0x0302: return "WM_PASTE";                        
	case 0x0303: return "WM_CLEAR";                        
	case 0x0304: return "WM_UNDO";                         
	case 0x0305: return "WM_RENDERFORMAT";                 
	case 0x0306: return "WM_RENDERALLFORMATS";             
	case 0x0307: return "WM_DESTROYCLIPBOARD";             
	case 0x0308: return "WM_DRAWCLIPBOARD";                
	case 0x0309: return "WM_PAINTCLIPBOARD";               
	case 0x030A: return "WM_VSCROLLCLIPBOARD";             
	case 0x030B: return "WM_SIZECLIPBOARD";                
	case 0x030C: return "WM_ASKCBFORMATNAME";              
	case 0x030D: return "WM_CHANGECBCHAIN";                
	case 0x030E: return "WM_HSCROLLCLIPBOARD";             
	case 0x030F: return "WM_QUERYNEWPALETTE";              
	case 0x0310: return "WM_PALETTEISCHANGING";            
	case 0x0311: return "WM_PALETTECHANGED";               
	case 0x0312: return "WM_HOTKEY";                       
	case 0x0317: return "WM_PRINT";                        
	case 0x0318: return "WM_PRINTCLIENT";                  
	case 0x0319: return "WM_APPCOMMAND";                   
	case 0x031A: return "WM_THEMECHANGED";                 
	case 0x0358: return "WM_HANDHELDFIRST";                
	case 0x035F: return "WM_HANDHELDLAST";                 
	case 0x0360: return "WM_AFXFIRST";                     
	case 0x037F: return "WM_AFXLAST";                      
	case 0x0380: return "WM_PENWINFIRST";                  
	case 0x038F: return "WM_PENWINLAST";                   
	}
	sprintf_s(tmp, 32,"WM_reserved_%d",msg);
	return tmp;
}

#endif


/*	Main Window Procedure
*******************************************************************************/
/*LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if(message==WM_PAINT)
	{
			Beep(444,41);
			Beep(6444,41);
			assert(0);
	}
    return DefWindowProc(hWnd, message, wParam, lParam);
}*/
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
//#ifdef _DEBUG
//	const char *msgname=get_msg_name(message);
//#endif

    switch (message)
	{
		case 15:// paint
		case 20:// WM_ERASEBKGND
		case 133:// ncpaint
		case 71:
		case 32:
		case 129:
		case 132:
		case 160:
		case 70:
		case 28:
		case 134:
		case 33:
			break;
		default:
			{
			int debug=message;
			}
	}

    switch (message)
    {
//		case 7:
//			return 1;

		case WM_SETCURSOR:
			{
			OSI_WINDOW * ww = (OSI_WINDOW *) GetWindowLongPtr(hWnd,GWLP_USERDATA);
			SetCursor(ww->hcursor);
			}
			return 1;

		case WM_GETDLGCODE:
		{
			long flags = DLGC_WANTALLKEYS;
			return flags;
		}

/*		case WM_USER_FORCEINVALIDATE:
			{
			OSI_WINDOW * ww = (OSI_WINDOW *) GetWindowLong(hWnd,GWL_USERDATA);
			MDEBUG_PRINT(0,"\nWM_USER_FORCEINVALIDATE");
			os_window_invalidate_rect(ww,0,0,ww->surface.w,ww->surface.h);
			}
			break;
*/
//		case WM_KEYDOWN:
//		case WM_KEYUP:

		case WM_USER_KEYDOWN:
		case WM_USER_KEYUP:
			{
			OS_MSGPARAM p;
			OSI_WINDOW * ww = (OSI_WINDOW *) GetWindowLongPtr(hWnd,GWLP_USERDATA);
			if(ww)
			{
				#define MAX_KEY_UNICHARS 8
				unsigned int unicode_c[MAX_KEY_UNICHARS];
				int i,uni_count=0;
				int thekey=0;
				p.key.flags=0;
				p.key.unicode_chars=unicode_c;
				switch((int)wParam)
				{
				case VK_INSERT:		thekey=OS_KEY_INSERT; break;
				case VK_HOME:		thekey=OS_KEY_HOME; break;
				case VK_END:		thekey=OS_KEY_END; break;
				case VK_SHIFT:		
					thekey=OS_KEY_SHIFT;
					if( ((lParam>>16)&511) > 50 )
						p.key.flags=1;
					break;
				case VK_CONTROL:	
					thekey=OS_KEY_CONTROL; 
					if( ((lParam>>16)&511) > 50 )
						p.key.flags=1;
					break;
				case VK_MENU:	
					thekey=OS_KEY_ALT; 
					if( ((lParam>>16)&511) > 50 )
						p.key.flags=1;
					break;
				case VK_BACK:		thekey=OS_KEY_BACKSPACE; break;
				case VK_TAB:		thekey=OS_KEY_TAB; break;
				case VK_RETURN:		thekey=OS_KEY_RETURN; break;
				case VK_SPACE:		thekey=' '; break;
				case VK_NEXT:		thekey=OS_KEY_PAGE_UP; break;
				case VK_PRIOR:		thekey=OS_KEY_PAGE_DOWN; break;
				case VK_UP:			thekey=OS_KEY_UP; break;
				case VK_DOWN:		thekey=OS_KEY_DOWN; break;
				case VK_LEFT:		thekey=OS_KEY_LEFT; break;
				case VK_RIGHT:		thekey=OS_KEY_RIGHT; break;
				case VK_ESCAPE:		thekey=OS_KEY_ESCAPE; break;
				case VK_DELETE:		thekey=OS_KEY_DELETE; break;
				default:
					{
						WORD out[MAX_KEY_UNICHARS];
						unsigned char st[256];
						unsigned int scancode=(lParam>>16)&255;
						out[0]=0;
						GetKeyboardState(st);

		//				thekey=MapVirtualKeyW(wParam,2);
						uni_count=ToAscii((unsigned int)wParam,scancode,st,out,0);
		//				uni_count=ToUnicode(wParam,scancode,st,out,MAX_KEY_UNICHARS,0);
		//				uni_count=0;
						if(uni_count<0)
							uni_count=0;
						for(i=0;i<uni_count;i++)
							unicode_c[i]=out[i];
						unicode_c[i]=0;
					}
					break;
				}

				if(thekey)
				{
					unicode_c[0]=thekey;
					unicode_c[1]=0;
				}
				else
				{
					if((wParam>='A' && wParam<='Z') || (wParam>='0' && wParam<='9'))
						thekey=(int)wParam;
				}

				if(os_key_is_down(OS_KEY_CONTROL))
				{
					switch(thekey)
					{
					case 'Z':
						if(os_key_is_down(OS_KEY_SHIFT))
								p.key.flags|=OS_KEYFLAGS_SPECIAL_REDO; 
						else
								p.key.flags|=OS_KEYFLAGS_SPECIAL_UNDO; 
						break;
					case 'X':	p.key.flags|=OS_KEYFLAGS_SPECIAL_CUT; break;
					case 'C':	p.key.flags|=OS_KEYFLAGS_SPECIAL_COPY; break;
					case 'V':	p.key.flags|=OS_KEYFLAGS_SPECIAL_PASTE; break;
					case 'A':	p.key.flags|=OS_KEYFLAGS_SPECIAL_SELECTALL; break;
					}
				}

				if(thekey || uni_count)
				{
					p.key.id=thekey;
					if( message==WM_USER_KEYDOWN && ((1<<30)&lParam )) // KeyRepeat
						ww->fp_msg(ww->id,OS_MSG_KEY_REPEAT,&p);
					else if(message==WM_USER_KEYDOWN)
						ww->fp_msg(ww->id,OS_MSG_KEY_DOWN,&p);
					else if(message==WM_USER_KEYUP)
						ww->fp_msg(ww->id,OS_MSG_KEY_UP,&p);
					else
					{
						M_ASSERT(0);
					}
				}
			}
			}
			break;

/*
		case WM_DEADCHAR:
			{
				int ost=2;
			}
			break;

		case WM_CHAR:
			{
			OS_MSGPARAM p;
			OSI_WINDOW * ww = (OSI_WINDOW *) GetWindowLongPtr(hWnd,GWL_USERDATA);
			p.keyascii.char_value=wParam;
//			if(p.keyascii.char_value<27 && p.keyascii.char_value!=13 && p.keyascii.char_value!=8)
//				p.keyascii.char_value+='A'-1;
			ww->fp_msg(ww->id,OS_MSG_KEYASCII,&p);
//			SendMessage(hWnd,WM_PAINT,0,0);
			break;
			}
*/
		case WM_SIZE:
			{
			OS_MSGPARAM p;
			OSI_WINDOW * ww = (OSI_WINDOW *) GetWindowLongPtr(hWnd,GWLP_USERDATA);
			if(ww)
			{
				p.window.p.x = LOWORD(lParam); 
				p.window.p.y = HIWORD(lParam);

				_createbackbuffer(ww,p.window.p.x,p.window.p.y);

				ww->fp_msg(ww->id,OS_MSG_WINDOW_SIZE_NOTIFY,&p);
			}
			}
			break;

		case WM_TIMER:
			{
			OS_MSGPARAM p;
			OSI_WINDOW * ww = (OSI_WINDOW *) GetWindowLongPtr(hWnd,GWLP_USERDATA);
			if(ww)
			{
				p.timer.user_id = (unsigned int*) wParam; 
				ww->fp_msg(ww->id,OS_MSG_WINDOW_TIMER,&p);
			}
			}
			break;

		case WM_MOVE:
			{
			OS_MSGPARAM p;
			OSI_WINDOW * ww = (OSI_WINDOW *) GetWindowLongPtr(hWnd,GWLP_USERDATA);
			if(ww)
			{
				p.window.p.x = LOWORD(lParam); 
				p.window.p.y = HIWORD(lParam);
				ww->fp_msg(ww->id,OS_MSG_WINDOW_MOVE_NOTIFY,&p);
			}
			}
			break;

#ifdef CF_OS_LAYER_WIN32_MOUSEHOOK
		case WM_USER_MOUSEWHEEL: // special case
			{
			OS_MSGPARAM p;
			OSI_WINDOW * ww = (OSI_WINDOW *) GetWindowLongPtr(hWnd,GWLP_USERDATA);
			if(ww)
			{
				POINT ptc;
				POINT pt;
				//GetForegroundWindow(VOID);
				GetCursorPos(&ptc);
				if(WindowFromPoint(ptc)==ww->win32win)
				{
					pt.x=0;
					pt.y=0;
					ClientToScreen(ww->win32win,&pt);
					p.mouse.p.x = ptc.x-pt.x; 
					p.mouse.p.y = ptc.y-pt.y;
					p.mouse.which_button = 0;
					p.mouse.wheel_dir = (((int)wParam)>0)?-1:1;
					ww->fp_msg(ww->id,OS_MSG_MOUSE_WHEEL,&p);
				}
			}
			}
			break;
#endif

		case WM_MOUSEMOVE:
			{
			OS_MSGPARAM p;
			OSI_WINDOW * ww = (OSI_WINDOW *) GetWindowLongPtr(hWnd,GWLP_USERDATA);
			s_mxPos = (short) LOWORD(lParam); 
			s_myPos = (short) HIWORD(lParam);
			p.mouse.p.x=s_mxPos;
			p.mouse.p.y=s_myPos;
			ww->fp_msg(ww->id,OS_MSG_MOUSE_MOVE,&p);
			break;
			}

		case WM_CAPTURECHANGED:
			{
				int i;
				OS_MSGPARAM p;
				OSI_WINDOW * ww = (OSI_WINDOW *) GetWindowLongPtr(hWnd,GWLP_USERDATA);
				p.mouse.p.x=s_mxPos;
				p.mouse.p.y=s_myPos;
				ww->mcap=0;
				for(i=0;i<3;i++)
					if(ww->mbutt[i])
					{
						ww->mbutt[i]=0;
						p.mouse.which_button=i;
						ww->fp_msg(ww->id,OS_MSG_MOUSE_BUTTONUP,&p);
					}
			}
			break;

		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
		case WM_MBUTTONUP:
			{
			OS_MSGPARAM p;
			OSI_WINDOW * ww = (OSI_WINDOW *) GetWindowLongPtr(hWnd,GWLP_USERDATA);

//			if(ww->popuphack)
//			{
//				PostMessage(ww->win32win,BM_CLICK,0,0);
//				ww->popuphack=0;
//			}

			switch(message)
			{
			case WM_LBUTTONUP: 
				p.mouse.which_button=OS_MOUSE_BUTTON_LEFT;
				break;
			case WM_RBUTTONUP: 
				p.mouse.which_button=OS_MOUSE_BUTTON_RIGHT;
				break;
			case WM_MBUTTONUP: 
				p.mouse.which_button=OS_MOUSE_BUTTON_MIDDLE;
				break;
			}
			if(ww->mbutt[p.mouse.which_button]==0) // out of sync, this button is not down... alt-tab etc
				break;

			ww->mbutt[p.mouse.which_button]=0;
			s_mxPos = (short) LOWORD(lParam); 
			s_myPos = (short) HIWORD(lParam);
			p.mouse.p.x=s_mxPos;
			p.mouse.p.y=s_myPos;

			M_ASSERT(ww->mcap);
			ww->mcap--;
			if(ww->mcap==0)
				ReleaseCapture();
			ww->fp_msg(ww->id,OS_MSG_MOUSE_BUTTONUP,&p);
			break;
			}

		case WM_LBUTTONDBLCLK:
		case WM_RBUTTONDBLCLK:
		case WM_MBUTTONDBLCLK:
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN:
			{
			OS_MSGPARAM p;
			OSI_WINDOW * ww = (OSI_WINDOW *) GetWindowLongPtr(hWnd,GWLP_USERDATA);

			switch(message)
			{
			case WM_LBUTTONDBLCLK:
			case WM_LBUTTONDOWN: 
				p.mouse.which_button=OS_MOUSE_BUTTON_LEFT;
				break;
			case WM_RBUTTONDBLCLK:
			case WM_RBUTTONDOWN: 
				p.mouse.which_button=OS_MOUSE_BUTTON_RIGHT;
				break;
			case WM_MBUTTONDBLCLK:
			case WM_MBUTTONDOWN: 
				p.mouse.which_button=OS_MOUSE_BUTTON_MIDDLE;
				break;
			}
			if(ww->mbutt[p.mouse.which_button]==1)
			{
				M_ASSERT(0); // This never happens?
				break;
			}
			ww->mbutt[p.mouse.which_button]=1;
			s_mxPos = (short) LOWORD(lParam); 
			s_myPos = (short) HIWORD(lParam);
			p.mouse.p.x=s_mxPos;
			p.mouse.p.y=s_myPos;
//			SetFocus(ww->win32win);
			if(ww->mcap==0)
				SetCapture(hWnd);
			ww->mcap++;
			ww->fp_msg(ww->id,OS_MSG_MOUSE_BUTTONDOWN,&p);
			break;
			}

		case WM_SETFOCUS:
			{
				OSI_WINDOW * ww = (OSI_WINDOW *) GetWindowLongPtr(hWnd,GWLP_USERDATA);
				if(ww)
					ww->fp_msg(ww->id,OS_MSG_WINDOW_KEY_FOCUS_NOTIFY,0);
				g_bIsActive=TRUE;
				return 1;
			}
			break;

		case WM_KILLFOCUS:
			{
				OSI_WINDOW * ww = (OSI_WINDOW *) GetWindowLongPtr(hWnd,GWLP_USERDATA);
				s_key_focus_prevwin=0;
				os_window_set_keyboard_focus(ww,0);

				ww->fp_msg(ww->id,OS_MSG_WINDOW_KEY_FOCUSLOST_NOTIFY,0);
				g_bIsActive=FALSE;
			}
			break;

		case WM_ACTIVATE:
			if(wParam&65535)
			{
//				OS_MSGPARAM p;
				OSI_WINDOW * ww = (OSI_WINDOW *) GetWindowLongPtr(hWnd,GWLP_USERDATA);
				if(ww)
					ww->fp_msg(ww->id,OS_MSG_WINDOW_WIN_FOCUS_NOTIFY,0);
				g_bIsActive=TRUE;
			}
			else
			{
//				OS_MSGPARAM p;
				OSI_WINDOW * ww = (OSI_WINDOW *) GetWindowLongPtr(hWnd,GWLP_USERDATA);
				if(ww)
				ww->fp_msg(ww->id,OS_MSG_WINDOW_WIN_FOCUSLOST_NOTIFY,0);
				g_bIsActive=FALSE;
			}
		break;

	case WM_USER_DROPTEXT:
		{
			POINT pt;
			OS_MSGPARAM p;
			OSI_WINDOW * ww = (OSI_WINDOW *) GetWindowLongPtr(hWnd,GWLP_USERDATA);
			OS_RECT rr;
			os_window_get_clientrect(ww,&rr);
			GetCursorPos(&pt);
			pt.x-=rr.x;
			pt.y-=rr.y;
			p.droptext.p_text = (char*)wParam;
			p.droptext.p.x = pt.x;
			p.droptext.p.y = pt.y;
			ww->fp_msg(ww->id,OS_MSG_DROPTEXT,&p);
		}
		break;
		
	case WM_USER_DROPFILES:
//	case WM_DROPFILES:
		{
			HDROP hd = (HDROP)wParam;

			int i,j,num_files = DragQueryFileW(hd, 0xFFFFFFFF, NULL, 0);

			WCHAR char16[OS_PATH_MAX];

			OS_PCHAR ** txt = M_MALLOC(OS_PCHAR*,num_files);
			OS_PCHAR * txtbuf = M_MALLOC(OS_PCHAR,OS_PATH_MAX*num_files);

			for(i=0;i<num_files;i++)
			{
				int len = DragQueryFileW(hd, i, char16, 1023);
				txt[i]=&txtbuf[i*1024];

				for(j=0;char16[j];j++) if(char16[j]=='\\') char16[j]='/';

				j=0;
				while(char16[j]) { txt[i][j]=(OS_PCHAR)char16[j]; j++; }
				txt[i][j]=0;
			}

			{
			POINT pt;
			OS_MSGPARAM p;
			OSI_WINDOW * ww = (OSI_WINDOW *) GetWindowLongPtr(hWnd,GWLP_USERDATA);
			OS_RECT rr;
			os_window_get_clientrect(ww,&rr);
			GetCursorPos(&pt);
			pt.x-=rr.x;
			pt.y-=rr.y;

//			DragQueryPoint(hd,&pt);
	
			p.dropfiles.p.x=pt.x;
			p.dropfiles.p.y=pt.y;
			p.dropfiles.num=num_files;
			p.dropfiles.p_path=txt;
			
			ww->fp_msg(ww->id,OS_MSG_DROPFILES,&p);

			M_SAFE_FREE(txt);
			M_SAFE_FREE(txtbuf);
			}
		}
		return 0;


		case WM_CLOSE:
			{
			LRESULT retu;
			OS_MSGPARAM p;
			OSI_WINDOW * ww = (OSI_WINDOW *) GetWindowLongPtr(hWnd,GWLP_USERDATA);

		    retu=DefWindowProc(hWnd, message, wParam, lParam);

			ww->fp_msg(ww->id,OS_MSG_WINDOW_SELFDESTRUCT,&p);
			ww->win32win=0;
			os_window_delete(ww);
			return retu;
			}
		break;
		
		case WM_ERASEBKGND:
		{
			return TRUE; // This is not needed?
		}
		break;

		case WM_PAINT:
		{
			OS_MSGPARAM p;
			PAINTSTRUCT ps; 
			HDC hdc;
			OSI_WINDOW * win = (OSI_WINDOW *) GetWindowLongPtr(hWnd,GWLP_USERDATA);

			RGNDATA *rgndata;
//			HDC paintdc;
			int gotregion;
			int i,l,x,y,w,h;
			RECT r;
			HRGN hrgn;
//			HWND hWnd;
			//MDEBUG_PRINT(MDEBUG_DG_VERBOSE,"\nWM_PAINT START");


			GetUpdateRect(hWnd, &ps.rcPaint, 0);
			if(win->send_paint_msg)
			{
				p.ospaint.invalid_rect.x=ps.rcPaint.left;
				p.ospaint.invalid_rect.y=ps.rcPaint.top;
				p.ospaint.invalid_rect.w=ps.rcPaint.right-ps.rcPaint.left;
				p.ospaint.invalid_rect.h=ps.rcPaint.bottom-ps.rcPaint.top;
				//_os_window_lock_backbuffer(win); 
				p.ospaint.pixel_data = &win->surface;
				win->send_paint_msg=0;
				win->fp_msg(win->id,OS_MSG_OSPAINT,&p);
				//_os_window_unlock_backbuffer(win);
			}

			hrgn=CreateRectRgn(0,0,0,0);
			gotregion=GetUpdateRgn(hWnd,hrgn,0);
			if(gotregion!=NULLREGION&&gotregion!=ERROR)
			{
				if(gotregion==COMPLEXREGION)
				{
					rgndata=(RGNDATA*)M_MALLOC_BYTES(i=GetRegionData(hrgn,0,0));
					GetRegionData(hrgn,i,rgndata);
			//		InvalidateRect(hWnd,&ps.rcPaint,0); // Needed? ("dont regionclip my BitBlt calls")

					DeleteObject(hrgn);

					hdc=BeginPaint(hWnd, &ps); 
					for(l=0;l<(int)rgndata->rdh.nCount;l++)
					{
						r=((RECT *)(rgndata->Buffer))[l];
						x=r.left;
						y=r.top;
						w=r.right-r.left;
						h=r.bottom-r.top;
	//					ValidateRect(hWnd,&r);

	//					os_window_sendcolordata(win, x, y, w, h, win->surface.w, &win->surface.data[x+y*win->surface.w]);

						BitBlt(hdc,x,y,w,h,win->win32hdc_backbuffer,x,y,SRCCOPY);
					}
					EndPaint(hWnd, &ps); 
					M_FREE(rgndata);
				}
				else if(gotregion==SIMPLEREGION)
				{
					DeleteObject(hrgn);

					hdc=BeginPaint(hWnd, &ps); 

					BitBlt(hdc,ps.rcPaint.left,ps.rcPaint.top,ps.rcPaint.right-ps.rcPaint.left,ps.rcPaint.bottom-ps.rcPaint.top,win->win32hdc_backbuffer,ps.rcPaint.left,ps.rcPaint.top,SRCCOPY);
					EndPaint(hWnd, &ps); 
				}
				else
				DeleteObject(hrgn);
			}
			else
				DeleteObject(hrgn);


/*
			GetUpdateRect(hWnd, &ps.rcPaint, 0);

			p.ospaint.x=ps.rcPaint.left;
			p.ospaint.y=ps.rcPaint.top;
			p.ospaint.w=ps.rcPaint.right-ps.rcPaint.left;
			p.ospaint.h=ps.rcPaint.bottom-ps.rcPaint.top;
			win->fp_msg(win->id,OS_MSG_OSPAINT,&p);

			hdc=BeginPaint(hWnd, &ps); 
			BitBlt(hdc,ps.rcPaint.left,ps.rcPaint.top,ps.rcPaint.right-ps.rcPaint.left,ps.rcPaint.bottom-ps.rcPaint.top,win->win32hdc_backbuffer,ps.rcPaint.left,ps.rcPaint.top,SRCCOPY);
			EndPaint(hWnd, &ps); 
*/

			//MDEBUG_PRINT(MDEBUG_DG_VERBOSE,"\nWM_PAINT END");
			return FALSE;
		}
		
		case WM_DESTROY:
			{
			// Clean up and close the app
	//		PostQuitMessage(0);
			return 0L;
			}
		break;
		

		case WM_SYSCOMMAND:
		{
			// Prevent moving/sizing and power loss in fullscreen mode
			switch( wParam )
			{
				case SC_MOVE:
				case SC_KEYMENU:
					{
					int ost=0;
					}

				case SC_SIZE:
				case SC_MAXIMIZE:
				case SC_MONITORPOWER:
					return 1;
				break;
			}

		}
		break;

	}

    return DefWindowProc(hWnd, message, wParam, lParam);

}













//--------------------------------------------------------
//--                     CLIPBOARD                      --
//--------------------------------------------------------

typedef struct _OSI_CLIPBOARD
{
	HWND hwnd;
	unsigned int format;
	char * txt;
	int bytesize;
	void * data;
} OSI_CLIPBOARD;

OS_CLIPBOARD os_clipboard_create(const char * type_name)
{
	OSI_CLIPBOARD* c=M_MALLOC(OSI_CLIPBOARD,1);
	WCHAR* type_name_w = ConvertCharToWChar(type_name);
	if (c != NULL && type_name != NULL) {
		c->hwnd = FindWindow(0,0);
		c->txt=0;
		c->bytesize=0;
		c->data=0;
		c->format = RegisterClipboardFormat(type_name_w);
		free(type_name_w);
	}
	return c;
}

void os_clipboard_delete(OS_CLIPBOARD c)
{
	if(c->txt)  M_FREE(c->txt);
	if(c->data) M_FREE(c->data);
	M_FREE(c);
}

void os_clipboard_set(OS_CLIPBOARD c, const char * txt, int bytesize, void * bindata )
{
	HANDLE hMem=0;
	HANDLE hStr=0;

    if (bytesize) 
	{
		int * pMem;
		hMem = GlobalAlloc(GMEM_MOVEABLE, bytesize + 4 + sizeof(int) );
		if (hMem != NULL) {
			pMem = (int *) GlobalLock(hMem);
			if (pMem != NULL) {
				*pMem = bytesize;
				memcpy(pMem+1,bindata,bytesize);
				GlobalUnlock(hMem);					
			}
			else { /* log error? */ }
		}
		else { /* log error? */ }
	}

    if (txt && *txt) 
	{
		char * pStr;
		int len = (int)(strlen(txt) + 1);
		hStr = GlobalAlloc(GMEM_MOVEABLE, len + 4L);
		if (hStr != NULL) {
			pStr = (char*)GlobalLock(hStr);
			if(pStr != NULL) {
				memcpy(pStr, txt, len);
				GlobalUnlock(hStr);			
			}
			else { /* log error? */ }	
		}
		else { /* log error? */ }	
	}

    if (OpenClipboard(c->hwnd)) 
	{
		EmptyClipboard();

		if(hMem)
		{
			SetClipboardData(c->format, hMem);
		}

		if(hStr)
		{
			SetClipboardData(CF_TEXT, hStr);
		}

		CloseClipboard();
	}
}

const char* os_clipboard_get_text(OS_CLIPBOARD c)
{
    if(OpenClipboard(c->hwnd)) 
	{
		HANDLE hStr=GetClipboardData(CF_TEXT);
		if(hStr!=0)
		{
			char * pStr = (char *) GlobalLock(hStr);
			if(pStr!=0)
			{
				int datasize = (int)(strlen(pStr) + 1);
				if(c->txt) M_FREE(c->txt);
				c->txt=(char*)M_MALLOC_BYTES(datasize);
				if (c->txt != NULL) {
					strcpy_s(c->txt, datasize, pStr);
					GlobalUnlock(hStr);
					CloseClipboard();
					return c->txt;				
				}
				else {
					// todo log error?
				}
			}
		}
	}
	return "";
}

void* os_clipboard_get_data(OS_CLIPBOARD c)
{
    if(OpenClipboard(c->hwnd)) 
	{
		HANDLE hMem=GetClipboardData(c->format);
		if(hMem)
		{
			int * pMem = (int *) GlobalLock(hMem);
			if (pMem != NULL) {
				int len = *pMem;
				if(c->data) M_FREE(c->data);
				c->data=(char*)M_MALLOC_BYTES(len);
				memcpy(c->data,pMem+1,len);
				GlobalUnlock(hMem);
				CloseClipboard();
				return c->data;			
			}
		}
	}
	return 0;
}




// END
#ifdef  __cplusplus
}
#endif

