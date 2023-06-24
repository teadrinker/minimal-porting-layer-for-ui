/*

	OS_LAYER - Operating system dependency layer (plain C API)

	WARNING, this is code old, don't use in production!

	By Martin Eklund 2004
	License: MIT 
	https://en.wikipedia.org/wiki/MIT_License

	This was designed to be as thin as possible for porting,
	it's not supposed to be used directly from higher level
	application code...

	Also, you probably don't want to you modal loops like
	it's the 90s...

*/

#ifndef OS_LAYER
#define OS_LAYER

#ifdef  __cplusplus
extern "C" {
#endif

	typedef struct _OSI_TIMER* OS_TIMER;
	typedef struct _OSI_WINDOW* OS_WINDOW;
	typedef void* OS_NATIVEWINDOW;

	typedef struct _OS_COORD{ int x; int y; } OS_COORD;
	typedef unsigned int OS_COLOR;
	typedef unsigned int uint;


	typedef char OS_CHAR;



	// OS file path string code point type:
	
	#define OS_PCHAR char
	//#define OS_PCHAR unsigned short int
	//#define OS_PCHAR unsigned int

	#define OS_PATH_MAX 2048   // max length for file paths


	typedef struct _OS_RECT
	{
		int x,y,w,h;
	} OS_RECT;

	typedef struct _OS_BACKBUFFER_PIXELS
	{
		int w, h;
		unsigned char* data;
	} OS_BACKBUFFER_PIXELS;


	enum _OS_MSG_KEY_ID
	{
	//	OS_KEY__RESERVED0	= 0,
		OS_KEY_INSERT		= 1,
		OS_KEY_HOME			= 2,
		OS_KEY_END			= 3,
		OS_KEY_SHIFT		= 4,
		OS_KEY_CONTROL		= 5,
		OS_KEY_ALT			= 6,
	//	OS_KEY__RESERVED1	= 7,
		OS_KEY_BACKSPACE	= 8,
		OS_KEY_TAB			= 9,
	//	OS_KEY__RESERVED2	= 10,
		OS_KEY_COMMAND		= 11, // (mac only)
	//	OS_KEY__RESERVED4	= 12,
		OS_KEY_RETURN		= 13,
		OS_KEY_PAGE_UP		= 14,
		OS_KEY_PAGE_DOWN	= 15,
		OS_KEY_ESCAPE		= 27,
		OS_KEY_LEFT			= 28,
		OS_KEY_RIGHT		= 29,
		OS_KEY_UP			= 30,
		OS_KEY_DOWN			= 31,

		OS_KEY_DELETE		= 127,
	};
	typedef unsigned int OS_MSG_KEY_ID;


	#define OS_KEYFLAGS_RIGHT					(1)			// the key is to the right (for shift/control/alt)
	#define OS_KEYFLAGS_SPECIAL					(2|4|8|16)
	#define OS_KEYFLAGS_SPECIAL_UNDO			(2)			// ctrl-z	
	#define OS_KEYFLAGS_SPECIAL_REDO			(4)			// ctrl-y
	#define OS_KEYFLAGS_SPECIAL_CUT				(4|2)		// ctrl-x
	#define OS_KEYFLAGS_SPECIAL_COPY			(8)			// ctrl-c	ctrl-insert
	#define OS_KEYFLAGS_SPECIAL_PASTE			(8|2)		// ctrl-v	shift-insert
	#define OS_KEYFLAGS_SPECIAL_SELECTALL		(8|4)		// ctrl-a
	#define OS_KEYFLAGS_SPECIAL_DESELECTALL		(8|4|2)		// ctrl-d
	#define OS_KEYFLAGS_SPECIAL_FIND			(16)		// ctrl-f
	#define OS_KEYFLAGS_SPECIAL_FINDNEXT		(16|2)		// F3
	#define OS_KEYFLAGS_SPECIAL_OPEN			(16|4)		// ctrl-o
	#define OS_KEYFLAGS_SPECIAL_SAVE			(16|4|2)	// ctrl-s
	#define OS_KEYFLAGS_SPECIAL_HELP			(16|8)		// F1
	#define OS_KEYFLAGS_SPECIAL_EXIT			(16|8|2)	// alt+F4

	#define OS_MOUSE_BUTTON_LEFT 0
	#define OS_MOUSE_BUTTON_RIGHT 1
	#define OS_MOUSE_BUTTON_MIDDLE 2

	typedef union _OS_MSGPARAM
	{
		struct OSS_MSG_WINDOW
		{
			OS_COORD p;
		} window;

		struct OSS_MSG_DROPFILES
		{
			OS_COORD p;
			int num;
			OS_PCHAR ** p_path;
		} dropfiles;

		struct OSS_MSG_DROPTEXT
		{
			OS_COORD p;
			char* p_text;
		} droptext;

		struct OSS_MSG_MOUSE
		{
			OS_COORD p;
			int which_button;
			int wheel_dir;
		} mouse;

		struct OSS_MSG_KEY
		{
			unsigned int id;
			unsigned int* unicode_chars;
			unsigned int flags;
		} key;
    
		struct OSS_MSG_TIMER
		{
			unsigned int* user_id;
		} timer;

		struct OSS_MSG_OSPAINT
		{
			OS_RECT invalid_rect;
			OS_BACKBUFFER_PIXELS* pixel_data;
		} ospaint;

	} OS_MSGPARAM;



	enum _OS_MSG
	{
		OS_MSG_NONE			=0,
	
		// Window Notify
		OS_MSG_WINDOW_CREATE_NOTIFY,
		OS_MSG_WINDOW_MOVE_NOTIFY,
		OS_MSG_WINDOW_SIZE_NOTIFY,
		OS_MSG_WINDOW_MINIMIZE_NOTIFY,
		OS_MSG_WINDOW_MINIMIZE_RESTORE_NOTIFY,
		OS_MSG_WINDOW_KEY_FOCUS_NOTIFY,
		OS_MSG_WINDOW_KEY_FOCUSLOST_NOTIFY,
		OS_MSG_WINDOW_WIN_FOCUS_NOTIFY,
		OS_MSG_WINDOW_WIN_FOCUSLOST_NOTIFY,
		OS_MSG_WINDOW_SELFDESTRUCT,

		OS_MSG_WINDOW_TIMER		,

		// ClientRect
		OS_MSG_DROPFILES	,
	
		OS_MSG_DROPTEXT		,

		OS_MSG_MOUSE_MOVE	,
		OS_MSG_MOUSE_BUTTONDOWN,
		OS_MSG_MOUSE_BUTTONUP,
		OS_MSG_MOUSE_WHEEL,
	
		OS_MSG_KEY_DOWN		,
		OS_MSG_KEY_UP		,
		OS_MSG_KEY_REPEAT	,


		OS_MSG_OSPAINT,



		// User Messages Start
		OS_MSG_USER = 4096
	
	};
	typedef int OS_MSG;


	typedef void* OS_WINDOW_ID;

	typedef void ((*HFUNC_OS_MSG)(OS_WINDOW_ID userid, OS_MSG msg, OS_MSGPARAM* p));


	#define OS_WIN_FLAG_TITLE				1
	#define OS_WIN_FLAG_RESIZEABLE			2
	#define OS_WIN_FLAG_MINIMIZEABLE		4
	#define OS_WIN_FLAG_CLOSEABLE			8	
	#define OS_WIN_FLAG_HIDDEN				16	
	#define OS_WIN_FLAG_KEYBOARD_AUTOFOCUS	32	

	#define OS_WIN_FLAG_MACOSX_EXTERNAL_DRAW		1024
	#define OS_WIN_FLAG_WIN32_MOUSEHOOK		2048
	#define OS_WIN_FLAG_TEST_NATIVE_MODE	4096



	// Window Functions
	void		os_window_create(OS_WINDOW* p_out, HFUNC_OS_MSG msgfunc, int x, int y, int w, int h, const OS_CHAR* name, OS_WINDOW_ID userid, uint flags);
	void		os_window_create_from_native(OS_WINDOW* p_out, HFUNC_OS_MSG msgfunc, int x, int y, int w, int h, const OS_CHAR* name, OS_WINDOW_ID userid, uint flags, OS_NATIVEWINDOW win);
	void        os_window_delete(OS_WINDOW);

	int         os_window_get_content_width(OS_WINDOW);
	int         os_window_get_content_height(OS_WINDOW);

	void*		os_window_get_native(OS_WINDOW);

	void        os_window_invalidate_rect(OS_WINDOW, int x, int y, int w, int h);

	int			os_window_modal_runloop(OS_WINDOW);
	void        os_window_modal_runloop_return(OS_WINDOW,int return_value);

	void		os_window_get_clientrect(OS_WINDOW nw, OS_RECT* r);

	// Misc
	void        os_init();
	void        os_shutdown();

	void        os_poll_msg(int time);
	int			os_runloop();
	void        os_runloop_return(int return_value);
	void        os_sleep(int); /// let the os breath


	// Timer callbacks
	OS_TIMER	os_window_timer_create(OS_WINDOW, int time_in_ms, uint* user_id); // Note: Is OS  timers really a good idea?
	void		os_window_timer_delete(OS_WINDOW, OS_TIMER);


	// Clipboard handling
	typedef struct _OSI_CLIPBOARD* OS_CLIPBOARD;

	OS_CLIPBOARD	os_clipboard_create(const char * type_name);
	void			os_clipboard_set(OS_CLIPBOARD,const char * txt, int bytesize, void * bindata );
	const char*		os_clipboard_get_text(OS_CLIPBOARD);
	void*			os_clipboard_get_data(OS_CLIPBOARD);
	void			os_clipboard_delete(OS_CLIPBOARD);



	// Debug-only
	void        os_fastdebug(int); /// pcspeaker beep or system errorsound (for debugging use only)


	// Execute / Show in finder/explorer / Goto internet URL
	typedef void ((*HFUNC_OS_EXECUTE_CALLBACK)(void* userdata, int returnvalue));
	#define OS_EXECUTE_FLAG_VIEW_IN_OS_FILEBROWSER 1
	#define OS_EXECUTE_FLAG_INTERNET_URL 2
	int			os_execute(const OS_PCHAR* str,const OS_PCHAR* args,const OS_PCHAR* dir,int flags, HFUNC_OS_EXECUTE_CALLBACK f, void* userdata); // note: use posix style?? _spawnvp(int mode, const char *cmdname,const char *const *argv);


	// Drag n Drop functions (Start)
	int			os_window_drag_files(OS_WINDOW win, int count, OS_PCHAR** filename_array);
	int			os_window_drag_text(OS_WINDOW win, char *text);


	// Keyboard input
	int			os_key_is_down(OS_MSG_KEY_ID k);
	void		os_window_set_keyboard_focus(OS_WINDOW w,int state); // note: add flag for Sending/not sending windowmessage back to client?


	// File dialogs
	#define OS_FILEDLG_FLAG_FILEEXISTALERT 1
	int			os_filedlg_save(	OS_PCHAR* out_path, const OS_PCHAR* text, const OS_PCHAR* def_path, const OS_PCHAR* def_filename,  const OS_PCHAR* ext_filter, unsigned int flags, OS_WINDOW win);
	int			os_filedlg_open(	OS_PCHAR* out_path,  
									const OS_PCHAR* text, 
									const OS_PCHAR* def_path,
									const OS_PCHAR* def_filename,
									const OS_PCHAR* ext_filter,
									unsigned int flags,
									OS_WINDOW win);

	// Popup/Dropdown/Context menus

	#define OS_POPUP_SEPARATOR		1
	#define OS_POPUP_CHECKED		2
	#define OS_POPUP_DISABLED		4
	//#define OS_POPUP_SELECTED		8

	typedef struct _OS_WINDOW_POPUP_INFO
	{
		const char* name;
		int id;
		int sub_count;
		struct _OS_WINDOW_POPUP_INFO* sub;
		int flags;
		void* userdata;
	} OS_WINDOW_POPUP_INFO;

	OS_WINDOW_POPUP_INFO* os_window_popup(OS_WINDOW win, int x, int y, int count, OS_WINDOW_POPUP_INFO* i);  



// END
#ifdef  __cplusplus
}
#endif

#endif










#ifdef OS_LAYER_SELF_TEST


	static uint hash(uint a)
	{
		a = (a + 0x7ed55d16) + (a << 12);
		a = (a ^ 0xc761c23c) ^ (a >> 19);
		a = (a + 0x165667b1) + (a << 5);
		a = (a + 0xd3a2646c) ^ (a << 9);
		a = (a + 0xfd7046c5) + (a << 3);
		a = (a ^ 0xb55a4f09) ^ (a >> 16);
		return a;
	}

	static int g_drawCounter = 0;
	static OS_WINDOW g_win = NULL;

	static void message(OS_WINDOW_ID userid, OS_MSG msg, OS_MSGPARAM* p) {

		if (msg == OS_MSG_WINDOW_TIMER || 
			msg == OS_MSG_MOUSE_BUTTONDOWN && p->mouse.which_button == OS_MOUSE_BUTTON_LEFT)
		{
			int w = os_window_get_content_width(g_win);
			int h = os_window_get_content_height(g_win);
			os_window_invalidate_rect(g_win, 0, 0, w, h);
		}

		if (msg == OS_MSG_MOUSE_BUTTONUP && p->mouse.which_button == OS_MOUSE_BUTTON_RIGHT) {
			const int count = 4;
			OS_WINDOW_POPUP_INFO info[count];
			for (int i = 0; i < count; i++) {
				info[i].flags = i == 1 ? OS_POPUP_SEPARATOR : 0;
				info[i].sub_count = 0;
				info[i].userdata = (void*)(intptr_t)i;
			}
			info[0].name = "disabled";
			info[0].flags = OS_POPUP_DISABLED;

			info[2].name = "some option (checked)";
			info[2].flags = OS_POPUP_CHECKED;

			info[3].name = "another option";

			OS_WINDOW_POPUP_INFO* ret = os_window_popup(g_win, p->mouse.p.x, p->mouse.p.y, count, info);
			int breakhere = 1;
		}
		if (msg == OS_MSG_DROPFILES) {
			for (int i = 0; i < p->dropfiles.num; i++) {
				OS_PCHAR* debug = p->dropfiles.p_path[i];
				int breakhere = 1;
			}
		}
		if (msg == OS_MSG_OSPAINT) {
			g_drawCounter++;
			OS_BACKBUFFER_PIXELS* data = p->ospaint.pixel_data;
			unsigned int* pixelData = (unsigned int*)data->data;
			int w = data->w;
			for (int y = 0; y < data->h; y++)
				for (int x = 0; x < w; x++)
					pixelData[x + y * w] = hash(x | (y << 16) ^ (g_drawCounter << 24));
		}
		if (msg == OS_MSG_WINDOW_SELFDESTRUCT) {
			os_runloop_return(0);
			g_win = NULL;
		}
	}

	void os_window_self_test() {

		os_init();

		os_window_create(&g_win, message, 10, 10, 1280, 720, "test me", NULL, OS_WIN_FLAG_TITLE | OS_WIN_FLAG_RESIZEABLE | OS_WIN_FLAG_CLOSEABLE);
		OS_TIMER timer = os_window_timer_create(g_win, 100, NULL);


		os_runloop();

		if (g_win)
			os_window_delete(g_win);

		os_shutdown();
	}

#endif