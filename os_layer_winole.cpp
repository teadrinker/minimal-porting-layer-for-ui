/*

 OLE drag'n drop implementation:

	int os_window_drag_text(OS_WINDOW win, char *text);
	int os_window_drag_files(OS_WINDOW win, int count, OS_PCHAR **filename_array);

	typedef struct OLE_WRAPPER_STRUCT* OLE_WRAPPER_HANDLE;

	OLE_WRAPPER_HANDLE   osi_win32_ole_drop_target_create(HWND win, int wm_user_msg_for_dropped_file, int wm_user_msg_for_dropped_text);
	void                 osi_win32_ole_drop_target_delete(HWND win, OLE_WRAPPER_HANDLE s);

 I've made the GUIDS static to avoid dependencys to uuid.lib

 Based on samplecode from the excellent tutorial: 
 "OLE Drag and Drop" by James Brown, www.catch22.net

*/

#include "os_layer.h"

#ifndef CF_OS_NO_OLE

#define M_NEW(classname,constr)		(new classname constr)
#define M_NEW_ARRAY(classname,count)(new classname[count])

#define M_SAFE_DELETE_ARRAY(p)		{ if(p) { delete [] p; p=0; } }
#define M_SAFE_DELETE(p)			{ if(p) { delete p; p=0; } }
#define M_DELETE_ARRAY(p)			{ delete [] p; }
#define M_DELETE(p)					{ delete p; }

#ifndef M_ASSERT
#ifdef _DEBUG
//	#define M_ASSERT(a)		assert(a)
#include <assert.h>
#define M_DO(a)			if(!(a))assert(0)
void asser(int i)
{
	if (!i)
	{
		int w = 0;
		assert(0);
	}
}
#define M_ASSERT(a) asser(((int)a))
#define M_ASSERT_MSG(a,b) asser(((int)a))
#define M_ASSERT_MSG1(a,b,c) asser(((int)a))

#else
#define M_ASSERT(a)
#define M_ASSERT_MSG(a,b)
#define M_ASSERT_MSG1(a,b,c)
#define M_DO(a)			a
#endif
#endif

#define STRICT

#include <ole2.h>


static const GUID sIID_IUnknown = {0x00000000, 0x0000, 0x0000, {0xc0,0x00, 0x00,0x00,0x00,0x00,0x00,0x46}};
static const GUID sIID_IDataObject = {0x0000010E, 0x0000, 0x0000, {0xc0,0x00, 0x00,0x00,0x00,0x00,0x00,0x46}};
static const GUID sIID_IDropSource = {0x00000121, 0x0000, 0x0000, {0xc0,0x00, 0x00,0x00,0x00,0x00,0x00,0x46}};
static const GUID sIID_IEnumFORMATETC = {0x00000103, 0x0000, 0x0000, {0xc0,0x00, 0x00,0x00,0x00,0x00,0x00,0x46}};



// defined in enumformat.cpp
HRESULT CreateEnumFormatEtc(UINT nNumFormats, FORMATETC *pFormatEtc, IEnumFORMATETC **ppEnumFormatEtc);

class CDataObject : public IDataObject
{
public:
	//
    // IUnknown members
	//
    HRESULT __stdcall QueryInterface (REFIID iid, void ** ppvObject);
    ULONG   __stdcall AddRef (void);
    ULONG   __stdcall Release (void);
		
    //
	// IDataObject members
	//
    HRESULT __stdcall GetData				(FORMATETC *pFormatEtc,  STGMEDIUM *pMedium);
    HRESULT __stdcall GetDataHere			(FORMATETC *pFormatEtc,  STGMEDIUM *pMedium);
    HRESULT __stdcall QueryGetData			(FORMATETC *pFormatEtc);
	HRESULT __stdcall GetCanonicalFormatEtc (FORMATETC *pFormatEct,  FORMATETC *pFormatEtcOut);
    HRESULT __stdcall SetData				(FORMATETC *pFormatEtc,  STGMEDIUM *pMedium,  BOOL fRelease);
	HRESULT __stdcall EnumFormatEtc			(DWORD      dwDirection, IEnumFORMATETC **ppEnumFormatEtc);
	HRESULT __stdcall DAdvise				(FORMATETC *pFormatEtc,  DWORD advf, IAdviseSink *pAdvSink, DWORD *pdwConnection);
	HRESULT __stdcall DUnadvise				(DWORD      dwConnection);
	HRESULT __stdcall EnumDAdvise			(IEnumSTATDATA **ppEnumAdvise);
	
	//
    // Constructor / Destructor
	//
    CDataObject(FORMATETC *fmt, STGMEDIUM *stgmed, int count);
    ~CDataObject();
	
private:

	int LookupFormatEtc(FORMATETC *pFormatEtc);

    //
	// any private members and functions
	//
    LONG	   m_lRefCount;

	FORMATETC *m_pFormatEtc;
	STGMEDIUM *m_pStgMedium;
	LONG	   m_nNumFormats;

};

//
//	Constructor
//
CDataObject::CDataObject(FORMATETC *fmtetc, STGMEDIUM *stgmed, int count) 
{
	m_lRefCount  = 1;
	m_nNumFormats = count;
	
	m_pFormatEtc  = M_NEW_ARRAY(FORMATETC,count);
	m_pStgMedium  = M_NEW_ARRAY(STGMEDIUM,count);

	for(int i = 0; i < count; i++)
	{
		m_pFormatEtc[i] = fmtetc[i];
		m_pStgMedium[i] = stgmed[i];
	}
}

//
//	Destructor
//
CDataObject::~CDataObject()
{
	// cleanup
	M_SAFE_DELETE_ARRAY(m_pFormatEtc);
	M_SAFE_DELETE_ARRAY(m_pStgMedium);
}

//
//	IUnknown::AddRef
//
ULONG __stdcall CDataObject::AddRef(void)
{
    // increment object reference count
    return InterlockedIncrement(&m_lRefCount);
}

//
//	IUnknown::Release
//
ULONG __stdcall CDataObject::Release(void)
{
    // decrement object reference count
	LONG count = InterlockedDecrement(&m_lRefCount);
		
	if(count == 0)
	{
		M_DELETE(this);
		return 0;
	}
	else
	{
		return count;
	}
}

//
//	IUnknown::QueryInterface
//
HRESULT __stdcall CDataObject::QueryInterface(REFIID iid, void **ppvObject)
{
    // check to see what interface has been requested
    if(iid == sIID_IDataObject || iid == sIID_IUnknown)
    {
        AddRef();
        *ppvObject = this;
        return S_OK;
    }
    else
    {
        *ppvObject = 0;
        return E_NOINTERFACE;
    }
}

HGLOBAL DupMem(HGLOBAL hMem)
{
	// lock the source memory object
	SIZE_T  len    = GlobalSize(hMem);
	PVOID   source = GlobalLock(hMem);
	
	// create a fixed "global" block - i.e. just
	// a regular lump of our process heap
	PVOID   dest   = GlobalAlloc(GMEM_FIXED, len);

	if (dest != NULL) {
		memcpy(dest, source, len);
		GlobalUnlock(hMem);
	}


	return dest;
}

int CDataObject::LookupFormatEtc(FORMATETC *pFormatEtc)
{
	for(int i = 0; i < m_nNumFormats; i++)
	{
		if((pFormatEtc->tymed    &  m_pFormatEtc[i].tymed)   &&
			pFormatEtc->cfFormat == m_pFormatEtc[i].cfFormat && 
			pFormatEtc->dwAspect == m_pFormatEtc[i].dwAspect)
		{
			return i;
		}
	}

	return -1;

}

//
//	IDataObject::GetData
//
HRESULT __stdcall CDataObject::GetData (FORMATETC *pFormatEtc, STGMEDIUM *pMedium)
{
	int idx;

	//
	// try to match the requested FORMATETC with one of our supported formats
	//
	if((idx = LookupFormatEtc(pFormatEtc)) == -1)
	{
		return DV_E_FORMATETC;
	}

	//
	// found a match! transfer the data into the supplied storage-medium
	//
	pMedium->tymed			 = m_pFormatEtc[idx].tymed;
	pMedium->pUnkForRelease  = 0;
	
	switch(m_pFormatEtc[idx].tymed)
	{
	case TYMED_HGLOBAL:

		pMedium->hGlobal = DupMem(m_pStgMedium[idx].hGlobal);
		//return S_OK;
		break;

	default:
		return DV_E_FORMATETC;
	}

	return S_OK;
}

//
//	IDataObject::GetDataHere
//
HRESULT __stdcall CDataObject::GetDataHere (FORMATETC *pFormatEtc, STGMEDIUM *pMedium)
{
	// GetDataHere is only required for IStream and IStorage mediums
	// It is an error to call GetDataHere for things like HGLOBAL and other clipboard formats
	//
	//	OleFlushClipboard 
	//
	return DATA_E_FORMATETC;
}

//
//	IDataObject::QueryGetData
//
//	Called to see if the IDataObject supports the specified format of data
//
HRESULT __stdcall CDataObject::QueryGetData (FORMATETC *pFormatEtc)
{
	return (LookupFormatEtc(pFormatEtc) == -1) ? DV_E_FORMATETC : S_OK;
}

//
//	IDataObject::GetCanonicalFormatEtc
//
HRESULT __stdcall CDataObject::GetCanonicalFormatEtc (FORMATETC *pFormatEct, FORMATETC *pFormatEtcOut)
{
	// Apparently we have to set this field to NULL even though we don't do anything else
	pFormatEtcOut->ptd = NULL;
	return E_NOTIMPL;
}

//
//	IDataObject::SetData
//
HRESULT __stdcall CDataObject::SetData (FORMATETC *pFormatEtc, STGMEDIUM *pMedium,  BOOL fRelease)
{
	return E_NOTIMPL;
}

//
//	IDataObject::EnumFormatEtc
//
HRESULT __stdcall CDataObject::EnumFormatEtc (DWORD dwDirection, IEnumFORMATETC **ppEnumFormatEtc)
{
	if(dwDirection == DATADIR_GET)
	{
		// for Win2k+ you can use the SHCreateStdEnumFmtEtc API call, however
		// to support all Windows platforms we need to implement IEnumFormatEtc ourselves.
		return CreateEnumFormatEtc(m_nNumFormats, m_pFormatEtc, ppEnumFormatEtc);
	}
	else
	{
		// the direction specified is not support for drag+drop
		return E_NOTIMPL;
	}
}


//
//	IDataObject::DAdvise
//
HRESULT __stdcall CDataObject::DAdvise (FORMATETC *pFormatEtc, DWORD advf, IAdviseSink *pAdvSink, DWORD *pdwConnection)
{
	return OLE_E_ADVISENOTSUPPORTED;
}

//
//	IDataObject::DUnadvise
//
HRESULT __stdcall CDataObject::DUnadvise (DWORD dwConnection)
{
	return OLE_E_ADVISENOTSUPPORTED;
}

//
//	IDataObject::EnumDAdvise
//
HRESULT __stdcall CDataObject::EnumDAdvise (IEnumSTATDATA **ppEnumAdvise)
{
	return OLE_E_ADVISENOTSUPPORTED;
}

//
//	Helper function
//
HRESULT CreateDataObject (FORMATETC *fmtetc, STGMEDIUM *stgmeds, UINT count, IDataObject **ppDataObject)
{
	if(ppDataObject == 0)
		return E_INVALIDARG;

	*ppDataObject = M_NEW( CDataObject ,(fmtetc, stgmeds, count));

	return (*ppDataObject) ? S_OK : E_OUTOFMEMORY;
}


class CDropSource : public IDropSource
{
public:
	//
    // IUnknown members
	//
    HRESULT __stdcall QueryInterface	(REFIID iid, void ** ppvObject);
    ULONG   __stdcall AddRef			(void);
    ULONG   __stdcall Release			(void);
		
    //
	// IDropSource members
	//
    HRESULT __stdcall QueryContinueDrag	(BOOL fEscapePressed, DWORD grfKeyState);
	HRESULT __stdcall GiveFeedback		(DWORD dwEffect);
	
	//
    // Constructor / Destructor
	//
    CDropSource();
    ~CDropSource();
	
private:

    //
	// private members and functions
	//
    LONG	   m_lRefCount;
};

//
//	Constructor
//
CDropSource::CDropSource() 
{
	m_lRefCount = 1;
}

//
//	Destructor
//
CDropSource::~CDropSource()
{
}

//
//	IUnknown::AddRef
//
ULONG __stdcall CDropSource::AddRef(void)
{
    // increment object reference count
    return InterlockedIncrement(&m_lRefCount);
}

//
//	IUnknown::Release
//
ULONG __stdcall CDropSource::Release(void)
{
    // decrement object reference count
	LONG count = InterlockedDecrement(&m_lRefCount);
		
	if(count == 0)
	{
		M_DELETE(this);
		return 0;
	}
	else
	{
		return count;
	}
}

//
//	IUnknown::QueryInterface
//
HRESULT __stdcall CDropSource::QueryInterface(REFIID iid, void **ppvObject)
{
    // check to see what interface has been requested
    if(iid == sIID_IDropSource || iid == sIID_IUnknown)
    {
        AddRef();
        *ppvObject = this;
        return S_OK;
    }
    else
    {
        *ppvObject = 0;
        return E_NOINTERFACE;
    }
}

//
//	CDropSource::QueryContinueDrag
//
//	Called by OLE whenever Escape/Control/Shift/Mouse buttons have changed
//
HRESULT __stdcall CDropSource::QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState)
{
	// if the <Escape> key has been pressed since the last call, cancel the drop
	if(fEscapePressed == TRUE)
		return DRAGDROP_S_CANCEL;	

	// if the <LeftMouse> button has been released, then do the drop!
	if((grfKeyState & MK_LBUTTON) == 0)
		return DRAGDROP_S_DROP;

	// continue with the drag-drop
	return S_OK;
}

//
//	CDropSource::GiveFeedback
//
//	Return either S_OK, or DRAGDROP_S_USEDEFAULTCURSORS to instruct OLE to use the
//  default mouse cursor images
//
HRESULT __stdcall CDropSource::GiveFeedback(DWORD dwEffect)
{
	return DRAGDROP_S_USEDEFAULTCURSORS;
}

//
//	Helper routine to create an IDropSource object
//	
HRESULT CreateDropSource(IDropSource **ppDropSource)
{
	if(ppDropSource == 0)
		return E_INVALIDARG;

	*ppDropSource = M_NEW( CDropSource,());

	return (*ppDropSource) ? S_OK : E_OUTOFMEMORY;

}


class CEnumFormatEtc : public IEnumFORMATETC
{
public:

	//
	// IUnknown members
	//
	HRESULT __stdcall  QueryInterface (REFIID iid, void ** ppvObject);
	ULONG	__stdcall  AddRef (void);
	ULONG	__stdcall  Release (void);

	//
	// IEnumFormatEtc members
	//
	HRESULT __stdcall  Next  (ULONG celt, FORMATETC * rgelt, ULONG * pceltFetched);
	HRESULT __stdcall  Skip  (ULONG celt); 
	HRESULT __stdcall  Reset (void);
	HRESULT __stdcall  Clone (IEnumFORMATETC ** ppEnumFormatEtc);

	//
	// Construction / Destruction
	//
	CEnumFormatEtc(FORMATETC *pFormatEtc, int nNumFormats);
	~CEnumFormatEtc();

private:

	LONG		m_lRefCount;		// Reference count for this COM interface
	ULONG		m_nIndex;			// current enumerator index
	ULONG		m_nNumFormats;		// number of FORMATETC members
	FORMATETC * m_pFormatEtc;		// array of FORMATETC objects
};

//
//	"Drop-in" replacement for SHCreateStdEnumFmtEtc. Called by CDataObject::EnumFormatEtc
//
HRESULT CreateEnumFormatEtc(UINT nNumFormats, FORMATETC *pFormatEtc, IEnumFORMATETC **ppEnumFormatEtc)
{
	if(nNumFormats == 0 || pFormatEtc == 0 || ppEnumFormatEtc == 0)
		return E_INVALIDARG;

	*ppEnumFormatEtc = M_NEW( CEnumFormatEtc,(pFormatEtc, nNumFormats));

	return (*ppEnumFormatEtc) ? S_OK : E_OUTOFMEMORY;
}

//
//	Helper function to perform a "deep" copy of a FORMATETC
//
static void DeepCopyFormatEtc(FORMATETC *dest, FORMATETC *source)
{
	// copy the source FORMATETC into dest
	*dest = *source;
	
	if(source->ptd)
	{
		// allocate memory for the DVTARGETDEVICE if necessary
		dest->ptd = (DVTARGETDEVICE*)CoTaskMemAlloc(sizeof(DVTARGETDEVICE));

		// copy the contents of the source DVTARGETDEVICE into dest->ptd
		*(dest->ptd) = *(source->ptd);
	}
}

//
//	Constructor 
//
CEnumFormatEtc::CEnumFormatEtc(FORMATETC *pFormatEtc, int nNumFormats)
{
	m_lRefCount   = 1;
	m_nIndex      = 0;
	m_nNumFormats = nNumFormats;
	m_pFormatEtc  = M_NEW_ARRAY( FORMATETC,nNumFormats);
	
	// copy the FORMATETC structures
	for(int i = 0; i < nNumFormats; i++)
	{	
		DeepCopyFormatEtc(&m_pFormatEtc[i], &pFormatEtc[i]);
	}
}

//
//	Destructor
//
CEnumFormatEtc::~CEnumFormatEtc()
{
	if(m_pFormatEtc)
	{
		for(ULONG i = 0; i < m_nNumFormats; i++)
		{
			if(m_pFormatEtc[i].ptd)
				CoTaskMemFree(m_pFormatEtc[i].ptd);
		}

		M_DELETE_ARRAY(m_pFormatEtc);
	}
}

//
//	IUnknown::AddRef
//
ULONG __stdcall CEnumFormatEtc::AddRef(void)
{
    // increment object reference count
    return InterlockedIncrement(&m_lRefCount);
}

//
//	IUnknown::Release
//
ULONG __stdcall CEnumFormatEtc::Release(void)
{
    // decrement object reference count
	LONG count = InterlockedDecrement(&m_lRefCount);
		
	if(count == 0)
	{
		M_DELETE(this);
		return 0;
	}
	else
	{
		return count;
	}
}

//
//	IUnknown::QueryInterface
//
HRESULT __stdcall CEnumFormatEtc::QueryInterface(REFIID iid, void **ppvObject)
{
    // check to see what interface has been requested
    if(iid == sIID_IEnumFORMATETC || iid == sIID_IUnknown)
    {
        AddRef();
        *ppvObject = this;
        return S_OK;
    }
    else
    {
        *ppvObject = 0;
        return E_NOINTERFACE;
    }
}

//
//	IEnumFORMATETC::Next
//
//	If the returned FORMATETC structure contains a non-null "ptd" member, then
//  the caller must free this using CoTaskMemFree (stated in the COM documentation)
//
HRESULT __stdcall CEnumFormatEtc::Next(ULONG celt, FORMATETC *pFormatEtc, ULONG * pceltFetched)
{
	ULONG copied  = 0;

	// validate arguments
	if(celt == 0 || pFormatEtc == 0)
		return E_INVALIDARG;

	// copy FORMATETC structures into caller's buffer
	while(m_nIndex < m_nNumFormats && copied < celt)
	{
		DeepCopyFormatEtc(&pFormatEtc[copied], &m_pFormatEtc[m_nIndex]);
		copied++;
		m_nIndex++;
	}

	// store result
	if(pceltFetched != 0) 
		*pceltFetched = copied;

	// did we copy all that was requested?
	return (copied == celt) ? S_OK : S_FALSE;
}

//
//	IEnumFORMATETC::Skip
//
HRESULT __stdcall CEnumFormatEtc::Skip(ULONG celt)
{
	m_nIndex += celt;
	return (m_nIndex <= m_nNumFormats) ? S_OK : S_FALSE;
}

//
//	IEnumFORMATETC::Reset
//
HRESULT __stdcall CEnumFormatEtc::Reset(void)
{
	m_nIndex = 0;
	return S_OK;
}

//
//	IEnumFORMATETC::Clone
//
HRESULT __stdcall CEnumFormatEtc::Clone(IEnumFORMATETC ** ppEnumFormatEtc)
{
	HRESULT hResult;

	// make a duplicate enumerator
	hResult = CreateEnumFormatEtc(m_nNumFormats, m_pFormatEtc, ppEnumFormatEtc);

	if(hResult == S_OK)
	{
		// manually set the index state
		((CEnumFormatEtc *) *ppEnumFormatEtc)->m_nIndex = m_nIndex;
	}

	return hResult;
}


//
//	This is our definition of a class which implements
//  the IDropTarget interface
//
class CDropTarget : public IDropTarget
{
public:
	// IUnknown implementation
	HRESULT __stdcall QueryInterface (REFIID iid, void ** ppvObject);
	ULONG	__stdcall AddRef (void);
	ULONG	__stdcall Release (void);

	// IDropTarget implementation
	HRESULT __stdcall DragEnter (IDataObject * pDataObject, DWORD grfKeyState, POINTL pt, DWORD * pdwEffect);
	HRESULT __stdcall DragOver (DWORD grfKeyState, POINTL pt, DWORD * pdwEffect);
	HRESULT __stdcall DragLeave (void);
	HRESULT __stdcall Drop (IDataObject * pDataObject, DWORD grfKeyState, POINTL pt, DWORD * pdwEffect);

	// Constructor
	CDropTarget(HWND hwnd);
	~CDropTarget();

private:

	// internal helper function
	DWORD DropEffect(DWORD grfKeyState, POINTL pt, DWORD dwAllowed);
	bool  QueryDataObject(IDataObject *pDataObject);


	// Private member variables
	LONG	m_lRefCount;
	HWND	m_hWnd;
	bool    m_fAllowDrop;

	IDataObject *m_pDataObject;

	void DropDataInternal(IDataObject *pDataObject, POINTL pt);
public:
	int wm_user_msg_for_dropped_file;
	int wm_user_msg_for_dropped_text;

};

//
//	Constructor for the CDropTarget class
//
CDropTarget::CDropTarget(HWND hwnd)
{
	m_lRefCount  = 1;
	m_hWnd       = hwnd;
	m_fAllowDrop = false;
}

//
//	Destructor for the CDropTarget class
//
CDropTarget::~CDropTarget()
{
	
}

//
//	Position the edit control's caret under the mouse
//
void PositionCursor(HWND hwndEdit, POINTL pt)
{
	LRESULT curpos;
		
	// get the character position of mouse
	ScreenToClient(hwndEdit, (POINT *)&pt);
	curpos = SendMessage(hwndEdit, EM_CHARFROMPOS, 0, MAKELPARAM(pt.x, pt.y));

	// set cursor position
	SendMessage(hwndEdit, EM_SETSEL, LOWORD(curpos), LOWORD(curpos));
}

//
//	IUnknown::QueryInterface
//
HRESULT __stdcall CDropTarget::QueryInterface (REFIID iid, void ** ppvObject)
{
	if(iid == IID_IDropTarget || iid == IID_IUnknown)
	{
		AddRef();
		*ppvObject = this;
		return S_OK;
	}
	else
	{
		*ppvObject = 0;
		return E_NOINTERFACE;
	}
}

//
//	IUnknown::AddRef
//
ULONG __stdcall CDropTarget::AddRef(void)
{
	return InterlockedIncrement(&m_lRefCount);
}	

//
//	IUnknown::Release
//
ULONG __stdcall CDropTarget::Release(void)
{
	LONG count = InterlockedDecrement(&m_lRefCount);
		
	if(count == 0)
	{
		delete this;
		return 0;
	}
	else
	{
		return count;
	}
}

//
//	QueryDataObject private helper routine
//
bool CDropTarget::QueryDataObject(IDataObject *pDataObject)
{
	FORMATETC fmtetc = { CF_TEXT, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
	if(pDataObject->QueryGetData(&fmtetc) == S_OK)
		return true;

	fmtetc.cfFormat=CF_HDROP;
	if(pDataObject->QueryGetData(&fmtetc) == S_OK)
		return true;

	return false;

}

//
//	DropEffect private helper routine
//
DWORD CDropTarget::DropEffect(DWORD grfKeyState, POINTL pt, DWORD dwAllowed)
{
	DWORD dwEffect = 0;

	// 1. check "pt" -> do we allow a drop at the specified coordinates?
	
	// 2. work out that the drop-effect should be based on grfKeyState
	if(grfKeyState & MK_CONTROL)
	{
		dwEffect = dwAllowed & DROPEFFECT_COPY;
	}
	else if(grfKeyState & MK_SHIFT)
	{
		dwEffect = dwAllowed & DROPEFFECT_MOVE;
	}
	
	// 3. no key-modifiers were specified (or drop effect not allowed), so
	//    base the effect on those allowed by the dropsource
	if(dwEffect == 0)
	{
		if(dwAllowed & DROPEFFECT_COPY) dwEffect = DROPEFFECT_COPY;
		if(dwAllowed & DROPEFFECT_MOVE) dwEffect = DROPEFFECT_MOVE;
	}
	
	return dwEffect;
}


//
//	IDropTarget::DragEnter
//
//
//
HRESULT __stdcall CDropTarget::DragEnter(IDataObject * pDataObject, DWORD grfKeyState, POINTL pt, DWORD * pdwEffect)
{
	// does the dataobject contain data we want?
	m_fAllowDrop = QueryDataObject(pDataObject);
	
	if(m_fAllowDrop)
	{
		// get the dropeffect based on keyboard state
		*pdwEffect = DropEffect(grfKeyState, pt, *pdwEffect);

		SetFocus(m_hWnd);

		PositionCursor(m_hWnd, pt);
	}
	else
	{
		*pdwEffect = DROPEFFECT_NONE;
	}

	return S_OK;
}

//
//	IDropTarget::DragOver
//
//
//
HRESULT __stdcall CDropTarget::DragOver(DWORD grfKeyState, POINTL pt, DWORD * pdwEffect)
{
	if(m_fAllowDrop)
	{
		*pdwEffect = DropEffect(grfKeyState, pt, *pdwEffect);
		PositionCursor(m_hWnd, pt);
	}
	else
	{
		*pdwEffect = DROPEFFECT_NONE;
	}

	return S_OK;
}

//
//	IDropTarget::DragLeave
//
HRESULT __stdcall CDropTarget::DragLeave(void)
{
	return S_OK;
}

//
//	IDropTarget::Drop
//
//
HRESULT __stdcall CDropTarget::Drop(IDataObject * pDataObject, DWORD grfKeyState, POINTL pt, DWORD * pdwEffect)
{
	PositionCursor(m_hWnd, pt);

	if(m_fAllowDrop)
	{
		DropDataInternal(pDataObject, pt);
		*pdwEffect = DropEffect(grfKeyState, pt, *pdwEffect);
	}
	else
	{
		*pdwEffect = DROPEFFECT_NONE;
	}

	return S_OK;
}

void CDropTarget::DropDataInternal(IDataObject *pDataObject, POINTL pt)
{
	// construct a FORMATETC object
	int msg[2]={wm_user_msg_for_dropped_text,wm_user_msg_for_dropped_file};
	FORMATETC fmtetc[2] = {	{ CF_TEXT, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL },
							{ CF_HDROP, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL }	};
	STGMEDIUM stgmed;

	int i;
	for(i=0;i<2;i++)
	// See if the dataobject contains any TEXT stored as a HGLOBAL
	if(pDataObject->QueryGetData(&fmtetc[i]) == S_OK)
	{
		// Yippie! the data is there, so go get it!
		if(pDataObject->GetData(&fmtetc[i], &stgmed) == S_OK)
		{
			// we asked for the data as a HGLOBAL, so access it appropriately
			PVOID data = GlobalLock(stgmed.hGlobal);

			SendMessage(m_hWnd,msg[i],(WPARAM)data,0);

			GlobalUnlock(stgmed.hGlobal);

			// release the data using the COM API
			ReleaseStgMedium(&stgmed);
		}
	}
}
/*
void RegisterDropWindow(HWND hwnd, IDropTarget **ppDropTarget)
{
	CDropTarget *pDropTarget = new CDropTarget(hwnd);

	// acquire a strong lock
	CoLockObjectExternal(pDropTarget, TRUE, FALSE);

	// tell OLE that the window is a drop target
	RegisterDragDrop(hwnd, pDropTarget);

	*ppDropTarget = pDropTarget;
}

void UnregisterDropWindow(HWND hwnd, IDropTarget *pDropTarget)
{
	// remove drag+drop
	RevokeDragDrop(hwnd);

	// remove the strong lock
	CoLockObjectExternal(pDropTarget, FALSE, TRUE);

	// release our own reference
	pDropTarget->Release();
}
*/












typedef struct _DROPFILESHACK
{
    DWORD pFiles;    // offset of file list
    POINT pt;        // drop point (client coords)
    BOOL fNC;        // is it on NonClient area and pt is in screen coords
    BOOL fWide;      // wide character flag
}DROPFILESHACK;


static int os_strlen(OS_PCHAR* str)
{
	int i=0;
	while(str[i]) i++;
	return i;
}


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
	MessageBox( NULL, (LPCTSTR)lpMsgBuf, L"Error", MB_OK | MB_ICONINFORMATION );
	// Free the buffer.
	LocalFree( lpMsgBuf );
}

extern "C" int  os_window_drag_files(OS_WINDOW win, int count, OS_PCHAR **filename_array)
{
	IDataObject *pDataObject;
	IDropSource *pDropSource;
	DWORD		 dwEffect;
	DWORD		 dwResult;


	FORMATETC fmtetc = { CF_HDROP, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
	STGMEDIUM stgmed = { TYMED_HGLOBAL, { 0 }, 0 };

	int j,i,len=0;
	for(j=0;j<count;j++)
		len+=os_strlen(filename_array[j])+1;

	unsigned short *wstr;
	HGLOBAL    hgDrop;
	DROPFILESHACK* pDrop;
	HWND dest=0;
	POINT cp;
/*	RECT cleantr;

	GetCursorPos(&cp);
	dest=WindowFromPoint(cp);
	if(!dest)
		return;

	GetClientRect(dest,&cleantr);
	cp.x-=cleantr.left;
	cp.y-=cleantr.top;
*/
	cp.x=0;
	cp.y=0;


    // Allocate memory from the heap for the DROPFILES struct.
    hgDrop=GlobalAlloc ( GHND, sizeof(DROPFILESHACK)+(len+2)*sizeof(unsigned short) );

    if(!hgDrop)
	{
        return 0;
	}

    pDrop=(DROPFILESHACK*) GlobalLock ( hgDrop );
    if(!pDrop)
    {
		GlobalFree(hgDrop);
		return 0;
    }

    // Fill in the DROPFILES struct.
    pDrop->pFiles=sizeof(DROPFILESHACK);
	pDrop->pt=cp;
    pDrop->fWide=TRUE;
	wstr=(unsigned short *) &(((char*)pDrop)[pDrop->pFiles]);
	for(j=0;j<count;j++)
	{
		for(i=0;i<len;i++)
			wstr[i]=filename_array[j][i];

		for(i=0;i<len;i++) if(wstr[i]=='/') wstr[i]='\\';

	}
	wstr[i++]=0;
	wstr[i++]=0;

	GlobalUnlock(hgDrop);
	stgmed.hGlobal = hgDrop;


	// Create IDataObject and IDropSource COM objects
	CreateDropSource(&pDropSource);
	CreateDataObject(&fmtetc, &stgmed, 1, &pDataObject);

	//
	//	** ** ** The drag-drop operation starts here! ** ** **
	//
	dwResult = DoDragDrop(pDataObject, pDropSource, DROPEFFECT_COPY/*|DROPEFFECT_MOVE*/, &dwEffect);

	// success!
	if(dwResult == DRAGDROP_S_DROP)
	{
		if(dwEffect & DROPEFFECT_MOVE)
		{
			// remove selection from edit control
		}
	}
	// cancelled
	else if(dwResult == DRAGDROP_S_CANCEL)
	{
	}
	else if(dwResult == E_OUTOFMEMORY)
	{
		//MDEBUG_PRINTW(2,"\nDoDragDrop returned E_OUTOFMEMORY in file \n",unsigned short,wstr);
		M_ASSERT(0);
	}
	else
	{
		M_ASSERT_MSG1(0,"\nDoDragDrop returned %d",dwResult);
		//MDEBUG_PRINTW(2," in file \n",unsigned short,wstr);
	}

	pDataObject->Release();
	pDropSource->Release();

	ReleaseCapture();



//this needs testing :

	ReleaseStgMedium(&stgmed);


//	fMouseDown = FALSE;
//	fDidDragDrop = TRUE;
	return 1;
}









extern "C" int os_window_drag_text(OS_WINDOW win, char *text)
{
	IDataObject *pDataObject;
	IDropSource *pDropSource;
	DWORD		 dwEffect;
	DWORD		 dwResult;

	FORMATETC fmtetc = { CF_TEXT, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
	STGMEDIUM stgmed = { TYMED_HGLOBAL, { 0 }, 0 };

	HGLOBAL hMem;
	int datalen = (int) strlen(text) + 1;
	hMem = GlobalAlloc(GHND, datalen);
	if (hMem != NULL) {
		char *ptr  = (char *)GlobalLock(hMem);

		// copy the selected text and nul-terminate
		if(ptr != NULL)
			strcpy_s(ptr, datalen, text);

		GlobalUnlock(hMem);
	}

	stgmed.hGlobal = hMem;

	// Create IDataObject and IDropSource COM objects
	CreateDropSource(&pDropSource);
	CreateDataObject(&fmtetc, &stgmed, 1, &pDataObject);

	//
	//	** ** ** The drag-drop operation starts here! ** ** **
	//
	dwResult = DoDragDrop(pDataObject, pDropSource, DROPEFFECT_COPY|DROPEFFECT_MOVE, &dwEffect);

	// success!
	if(dwResult == DRAGDROP_S_DROP)
	{
		if(dwEffect & DROPEFFECT_MOVE)
		{
			// remove selection from edit control
		}
	}
	// cancelled
	else if(dwResult == DRAGDROP_S_CANCEL)
	{
	}

	pDataObject->Release();
	pDropSource->Release();

	ReleaseCapture();
//	fMouseDown = FALSE;
//	fDidDragDrop = TRUE;
	return 1;
}


struct OLE_WRAPPER_STRUCT
{
	void* pDropTarget;
};

typedef struct OLE_WRAPPER_STRUCT* OLE_WRAPPER_HANDLE;

extern "C" OLE_WRAPPER_HANDLE osi_win32_ole_drop_target_create(HWND hwnd, int wm_user_msg_for_dropped_file, int wm_user_msg_for_dropped_text)
{
	CDropTarget *pDropTarget = new CDropTarget(hwnd);

	// acquire a strong lock
	CoLockObjectExternal(pDropTarget, TRUE, FALSE);

	// tell OLE that the window is a drop target
	RegisterDragDrop(hwnd, pDropTarget);

	pDropTarget->wm_user_msg_for_dropped_file=wm_user_msg_for_dropped_file;
	pDropTarget->wm_user_msg_for_dropped_text=wm_user_msg_for_dropped_text;
	return (OLE_WRAPPER_HANDLE) pDropTarget;
}

extern "C" void osi_win32_ole_drop_target_delete(HWND hwnd, OLE_WRAPPER_HANDLE s)
{
	CDropTarget*pDropTarget=(CDropTarget*)s;
	// remove drag+drop
	RevokeDragDrop(hwnd);

	// remove the strong lock
	CoLockObjectExternal(pDropTarget, FALSE, TRUE);

	// release our own reference
	pDropTarget->Release();
}


/*extern "C" void os_drag_files(int count, char **filename_array)
{
	IDataObject *pIDataObject=CoCreateInstance(	CLSID_IDataObject,// __uuidof(IDataObject),
												0,
												CLSCTX_INPROC_SERVER,
												sIID_IDataObject);
	IDropSource *pIDropSource;

	DWORD dwEffect;
	HRESULT hr = DoDragDrop(pIDataObject,
                            pIDropSource,
                            DROPEFFECT_COPY,
                            &dwEffect);

}*/


#endif

