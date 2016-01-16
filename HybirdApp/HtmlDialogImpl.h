// ***************************************************************
//  atldhtmldlg   version:  1.03   ·  date: 1/14/2008
//  -------------------------------------------------------------
//  Author: James
//
//  -------------------------------------------------------------
//  Copyright (C) 2007 - All Rights Reserved
// ***************************************************************
// 
// usage:
//     This file support ATL::CDHtmlDialogImpl and ATL::CMultiPageDHtmlDialogImpl like as MFC, and expands some features
// 
// features:
//     * show HTML resource/file/url on dialog
//				class CMainDlg : public CDHtmlDialogImpl<CMainDlg> {...}
//				CMainDlg dlg;
//				dlg.m_nHtmlResID = IDR_YOUR_HTML_RESOURCE_ID; // open your resource
//					or
//				dlg.m_szHtmlResID = _T("yourname.html"); // open your resource
//					or
//				dlg.m_strCurrentUrl = _T("www.microsoft.com"); // open local html file or external URL
//				dlg.Create(NULL); // or dlg.DoModal(NULL);
//					......
//					
//     * overidable DWebBrowser2Events sink implement
//				class CMainDlg : public CDHtmlDialogImpl<CMainDlg>
//				{
//					......
//					virtual void OnBeforeNavigate(LPDISPATCH pDisp, LPCTSTR szUrl)
//					{
//						// your code is here.
//					}
//					virtual void OnNavigateComplete(LPDISPATCH pDisp, LPCTSTR szUrl)
//					{
//						// your code is here.
//					}
//					virtual void OnDocumentComplete(LPDISPATCH pDisp, LPCTSTR szUrl)
//					{
//						// your code is here.
//					}
//					......
//				}
//					
//     * map-enabled IHTMLDocument2Events sink implement
//				class CMainDlg : public CDHtmlDialogImpl<CMainDlg>
//				{
//					......
//					BEGIN_DHTML_EVENT_MAP(CMainDlg)
//						DHTML_EVENT_ONCLICK(_T("elementid1"), OnClick)
//						DHTML_EVENT_ONMOUSEMOVE(_T("elementid2"), OnMouseMove)
//					END_DHTML_EVENT_MAP()
//					HRESULT OnClick(IHTMLElement *pElement)
//					{
//						// your code is here.
//						return S_OK;
//					}
//					HRESULT OnMouseMove(IHTMLElement *pElement)
//					{
//						// your code is here.
//						return S_OK;
//					}
//					......
//				}
//					
//     * map-enabled IHTMLElement events sink implement
//				class CMainDlg : public CDHtmlDialogImpl<CMainDlg>
//				{
//					......
//					BEGIN_DHTML_EVENT_MAP(CMainDlg)
//						DHTML_EVENT_ELEMENT(DISPID_HTMLELEMENTEVENTS_ONMOUSEOVER, _T("elementid"), OnMouseOver)
//						DHTML_EVENT_CLASS(DISPID_HTMLELEMENTEVENTS_ONMOUSEOVER, _T("classname"), OnMouseOver)
//						DHTML_EVENT_TAG(DISPID_HTMLELEMENTEVENTS_ONMOUSEOVER, _T("tagname"), OnMouseOver)
//					END_DHTML_EVENT_MAP()
//					HRESULT OnMouseOver(IHTMLElement *pElement)
//					{
//						// your code is here.
//						return S_OK;
//					}
//					......
//				}
//					
//     * map-enabled ActiveX control Events sink implement
//				class CMainDlg : public CDHtmlDialogImpl<CMainDlg>
//				{
//					......
//					BEGIN_DHTML_EVENT_MAP(CMainDlg)
//						DHTML_EVENT_AXCONTROL(controlMethodDISPID, _T("objectid"), OnControlMethod)
//					END_DHTML_EVENT_MAP()
//					HRESULT OnControlMethod(IHTMLElement *pElement)
//					{
//						// your code is here.
//						return S_OK;
//					}
//					......
//				}
//					
//     * map-enabled external dispatch implement
//				class CMainDlg : public CDHtmlDialogImpl<CMainDlg>
//				{
//					......
//					BEGIN_EXTERNAL_METHOD_MAP(CMainDlg)
//						EXTERNAL_METHOD(_T("about"), OnAbout)  //// html script calls "window.external.about(123, "abc");"
//					END_EXTERNAL_METHOD_MAP()
//					void OnAbout(VARIANT* para1, VARIANT* para2, VARIANT* para3)
//					{
//						// your code is here.
//					}
//					......
//				}
//					
//     * map-enabled multi pages HTML dialog
//				class CMainDlg : public CMultiPageDHtmlDialogImpl<CMainDlg>
//				{
//					......
//					BEGIN_URL_ENTRIES_MAP(CMainDlg)
//						BEGIN_DHTML_URL_EVENT_MAP(1)	/// map in page1.htm
//							DHTML_EVENT_ONCLICK(_T("elementid"), OnClick)
//							DHTML_EVENT_AXCONTROL(controlMethodDISPID, _T("objectid"), OnControlMethod)
//						END_DHTML_URL_EVENT_MAP()
//
//						BEGIN_DHTML_URL_EVENT_MAP(2)	/// map in page2.htm
//							DHTML_EVENT_ELEMENT(DISPID_HTMLELEMENTEVENTS_ONMOUSEOVER, _T("elementid"), OnMouseOver)
//						END_DHTML_URL_EVENT_MAP()
// 
//						BEGIN_URL_ENTRIES()
//							URL_EVENT_ENTRY(_T("page1.htm"), 1)
//							URL_EVENT_ENTRY(_T("page2.htm"), 2)
//						END_URL_ENTRIES()
//					END_URL_ENTRIES_MAP()
// 
//					HRESULT OnMouseOver(IHTMLElement *pElement)
//					{
//						// your code is here.
//						return S_OK;
//					}
//					......
//				}
//					
//     * IDocHostHandleDispatch implement
//     * dialog properties in HTML document
// 
// note:
//     DDX is not supported in this version, because ATL is not support that.
//     DDX is supported by WTL, next build will add WTL's DDX.
// 
//////////////////////////////////////////////////////////////////////////
// 
// dialog properties in HTML document sample:
// 
// <html>
//   <head>...</head>
//   <body &&& DIALOG PROPERTIES IS HERE &&& >...</body>
// </html>
// 
// dialog properties list:
//     dlg_autosize - dialog resizes webbrowser to real size, value of property is ignored
//     dlg_width=number - dialog resizes webbrowser's width to 'number' pixels, default is 300
//     dlg_height=number - dialog resizes webbrowser's height to 'number' pixels, default is 200
// 
// ***************************************************************
//
//////////////////////////////////////////////////////////////////////////
// Fix histroy:
// 1.03--------
//		* It's failed when dialog is recreated, because WebBrowser control isn't destroy in WM_DESTROY.
//		* #include <atlsafe.h> is deleted. atlsafe.h is not supported in ATL 3.0
//////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __ATLDHTMLDLG_H__
#define __ATLDHTMLDLG_H__

#ifndef __cplusplus
	#error ATL requires C++ compilation (use a .cpp suffix)
#endif

#ifndef __mshtml_h__
#include <mshtml.h>
#endif

#ifndef __MSHTMDID_H__
#include <mshtmdid.h>
#endif

#ifndef __mshtmhst_h__
#include <mshtmhst.h>
#endif

#ifndef __ATLBASE_H__
#include <atlbase.h>
#endif

#ifndef __ATLCOM_H__
#include <atlcom.h>
#endif

#ifndef __exdisp_h__
#include <exdisp.h>
#endif

#ifndef EXDISPID_H_
#include <exdispid.h>
#endif

//#ifndef __ATLAPP_H__
//	#error atldhtmldlg.h requires atlapp.h (WTL) to be included first
//#endif

#ifndef __ATLWIN_H__
#include <atlwin.h>
	//#error atldhtmldlg.h requires atlwin.h to be included first
#endif

#ifndef __ATLHOST_H__
#include <atlhost.h>
#endif

//#include <atlsafe.h>
#ifndef _OLEAUTO_H_
#include <Oleauto.h>
#endif // _OLEAUTO_H_

#ifdef __ATLDDX_H__
	#ifndef WTL::DDX_LOAD
		#error atldhtmldlg.h reqires atlddx.h(WTL) to be included first
	#endif
using namespace WTL;
#endif

//#define ID__WEBBROWSER	2345

namespace ATL
{

// Classes declared in this file
class CDHtmlSinkHandler;	// Events Sink Base
	class CDHtmlEventSink;	// IHTMLDocument2 Events Sink

// IDispatch
	class CDHtmlControlSink;	// ActiveX Control Events Sink

// IDispatch
	class CDHtmlElementEventSink;	// IHTMLElement Events Sink

// IDispatch
class CExternalDispatchBase;
	template<class T> class CExternalDispatchImpl; // External Dispatch Implement

// IDocHostUIHandleDispatch
	template<class T> class IDocHostUIHandlerDispatchImpl;

// IDispatch
	template<class T> class DWebBrowserEvent2Impl;

// ATL::CDialogImplBaseT
	template<class T,class TBase> class CDHtmlDialogImpl;
		template<class T, class TBas> class CMultiPageDHtmlDialogImpl;


enum DHtmlEventMapEntryType { DHTMLEVENTMAPENTRY_NAME, DHTMLEVENTMAPENTRY_CLASS, DHTMLEVENTMAPENTRY_TAG,
	DHTMLEVENTMAPENTRY_ELEMENT, DHTMLEVENTMAPENTRY_CONTROL, DHTMLEVENTMAPENTRY_END };

struct DHtmlEventMapEntry;

// Dummy base classes just to force pointers-to-member that work with multiple inheritance
class CDHtmlSinkHandlerBase1
{
};
class CDHtmlSinkHandlerBase2
{
};

class CDHtmlSinkHandler :
	public CDHtmlSinkHandlerBase1,
	public CDHtmlSinkHandlerBase2
{
public:
	STDMETHOD(CDHtmlSinkHandlerQueryInterface)(REFIID riid, void ** ppvObject) = 0;
	virtual ULONG STDMETHODCALLTYPE AddRef(void) = 0;
	virtual ULONG STDMETHODCALLTYPE Release(void) = 0;
	STDMETHOD(GetTypeInfoCount)(UINT *pctinfo) = 0;
	STDMETHOD(GetTypeInfo)(UINT iTInfo, LCID lcid, ITypeInfo **ppTInfo) = 0;
	STDMETHOD(GetIDsOfNames)(REFIID riid, OLECHAR **rgszNames, UINT cNames, LCID lcid, DISPID *rgDispId) = 0;
	STDMETHOD(Invoke)(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags,
		DISPPARAMS *pDispParams, VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr) = 0;
	virtual const DHtmlEventMapEntry* GetDHtmlEventMap() = 0;
};

// External Dispatch Method Map
struct DHtmlExternalMethodMapEntry 
{
	LPCTSTR szName; // external method name
	void (__stdcall CExternalDispatchBase::*pfnExternalFunc)();
};

typedef void (CExternalDispatchBase::*EXTFUNC)(VARIANT* para1,VARIANT* para2,VARIANT* para3);
typedef void (__stdcall CExternalDispatchBase::*EXTFUNCCONTROL)();

#define BEGIN_EXTERNAL_METHOD_MAP(className) \
public: \
virtual const DHtmlExternalMethodMapEntry* GetExternalMethodMapEntry(long* plNum) \
{ \
	typedef className theClass; \
	PTM_WARNING_DISABLE \
	static const DHtmlExternalMethodMapEntry _dhtmlExtMethodEntries[] = { \
	{ _T("resizeWindow"), (EXTFUNCCONTROL)(EXTFUNC) &theClass::OnResizeWindow },\
	{ _T("centerWindow"), (EXTFUNCCONTROL)(EXTFUNC) &theClass::OnCenterWindow },\
	{ _T("showTitleBar"), (EXTFUNCCONTROL)(EXTFUNC) &theClass::OnShowTitleBar },

#define END_EXTERNAL_METHOD_MAP() \
	/*{ NULL, (EXTFUNCCONTROL)NULL },*/ \
	}; \
	PTM_WARNING_RESTORE \
	if (plNum) \
		*plNum = (long)(sizeof(_dhtmlExtMethodEntries)/sizeof(DHtmlExternalMethodMapEntry)); \
	return _dhtmlExtMethodEntries;\
}

#define EXTERNAL_METHOD(methodName, memberFxn) \
{ methodName, (EXTFUNCCONTROL)(EXTFUNC) &theClass::memberFxn },\

// DHtml Event Map
struct DHtmlEventMapEntry
{
	DHtmlEventMapEntryType nType;
	DISPID dispId;
	LPCTSTR szName;	// name or class based on nType
	void (__stdcall CDHtmlSinkHandler::*pfnEventFunc)();
};

typedef HRESULT (CDHtmlSinkHandler::*DHEVTFUNC)(IHTMLElement *pElement);
typedef void (__stdcall CDHtmlSinkHandler::*DHEVTFUNCCONTROL)();

#define BEGIN_DHTML_EVENT_MAP(className)\
public:\
const DHtmlEventMapEntry* GetDHtmlEventMap()\
{\
	typedef className theClass;\
	PTM_WARNING_DISABLE \
	static const DHtmlEventMapEntry _dhtmlEventEntries[] = { \
	{ DHTMLEVENTMAPENTRY_NAME, DISPID_EVMETH_ONREADYSTATECHANGE, NULL, (DHEVTFUNCCONTROL) (DHEVTFUNC) &theClass::OnDocumentReadyStateChange },

#define END_DHTML_EVENT_MAP()\
	{ DHTMLEVENTMAPENTRY_END, 0, NULL, (DHEVTFUNCCONTROL) NULL },\
	};\
	PTM_WARNING_RESTORE \
	return _dhtmlEventEntries;\
}

#define BEGIN_DHTML_EVENT_MAP_INLINE(className) BEGIN_DHTML_EVENT_MAP(className)
#define END_DHTML_EVENT_MAP_INLINE() END_DHTML_EVENT_MAP()


#define DHTML_EVENT(dispid, elemName, memberFxn)\
{ DHTMLEVENTMAPENTRY_NAME, dispid, elemName, (DHEVTFUNCCONTROL) (DHEVTFUNC) &theClass::memberFxn },\

#define DHTML_EVENT_CLASS(dispid, elemName, memberFxn)\
{ DHTMLEVENTMAPENTRY_CLASS, dispid, elemName, (DHEVTFUNCCONTROL) (DHEVTFUNC) &theClass::memberFxn },\

#define DHTML_EVENT_TAG(dispid, elemName, memberFxn)\
{ DHTMLEVENTMAPENTRY_TAG, dispid, elemName, (DHEVTFUNCCONTROL) (DHEVTFUNC) &theClass::memberFxn },\

#define DHTML_EVENT_ELEMENT(dispid, elemName, memberFxn)\
{ DHTMLEVENTMAPENTRY_ELEMENT, dispid, elemName, (DHEVTFUNCCONTROL) (DHEVTFUNC) &theClass::memberFxn },\

#define DHTML_EVENT_AXCONTROL(dispid, controlName, memberFxn)\
{ DHTMLEVENTMAPENTRY_CONTROL, dispid, controlName, (DHEVTFUNCCONTROL) (static_cast<void (__stdcall theClass::*)()>(&theClass::memberFxn)) },\

// specific commonly used events
#define DHTML_EVENT_ONHELP(elemName, memberFxn)\
DHTML_EVENT(DISPID_HTMLELEMENTEVENTS_ONHELP, elemName, memberFxn)\

#define DHTML_EVENT_ONCLICK(elemName, memberFxn)\
DHTML_EVENT(DISPID_HTMLELEMENTEVENTS_ONCLICK, elemName, memberFxn)\

#define DHTML_EVENT_ONDBLCLICK(elemName, memberFxn)\
DHTML_EVENT(DISPID_HTMLELEMENTEVENTS_ONDBLCLICK, elemName, memberFxn)\

#define DHTML_EVENT_ONKEYPRESS(elemName, memberFxn)\
DHTML_EVENT(DISPID_HTMLELEMENTEVENTS_ONKEYPRESS, elemName, memberFxn)\

#define DHTML_EVENT_ONKEYDOWN(elemName, memberFxn)\
DHTML_EVENT(DISPID_HTMLELEMENTEVENTS_ONKEYDOWN, elemName, memberFxn)\

#define DHTML_EVENT_ONKEYUP(elemName, memberFxn)\
DHTML_EVENT(DISPID_HTMLELEMENTEVENTS_ONKEYUP, elemName, memberFxn)\

#define DHTML_EVENT_ONMOUSEOUT(elemName, memberFxn)\
DHTML_EVENT(DISPID_HTMLELEMENTEVENTS_ONMOUSEOUT, elemName, memberFxn)\

#define DHTML_EVENT_ONMOUSEOVER(elemName, memberFxn)\
DHTML_EVENT(DISPID_HTMLELEMENTEVENTS_ONMOUSEOVER, elemName, memberFxn)\

#define DHTML_EVENT_ONMOUSEMOVE(elemName, memberFxn)\
DHTML_EVENT(DISPID_HTMLELEMENTEVENTS_ONMOUSEMOVE, elemName, memberFxn)\

#define DHTML_EVENT_ONMOUSEDOWN(elemName, memberFxn)\
DHTML_EVENT(DISPID_HTMLELEMENTEVENTS_ONMOUSEDOWN, elemName, memberFxn)\

#define DHTML_EVENT_ONMOUSEUP(elemName, memberFxn)\
DHTML_EVENT(DISPID_HTMLELEMENTEVENTS_ONMOUSEUP, elemName, memberFxn)\

#define DHTML_EVENT_ONSELECTSTART(elemName, memberFxn)\
DHTML_EVENT(DISPID_HTMLELEMENTEVENTS_ONSELECTSTART, elemName, memberFxn)\

#define DHTML_EVENT_ONFILTERCHANGE(elemName, memberFxn)\
DHTML_EVENT(DISPID_HTMLELEMENTEVENTS_ONFILTERCHANGE, elemName, memberFxn)\

#define DHTML_EVENT_ONDRAGSTART(elemName, memberFxn)\
DHTML_EVENT(DISPID_HTMLELEMENTEVENTS_ONDRAGSTART, elemName, memberFxn)\

#define DHTML_EVENT_ONBEFOREUPDATE(elemName, memberFxn)\
DHTML_EVENT(DISPID_HTMLELEMENTEVENTS_ONBEFOREUPDATE, elemName, memberFxn)\

#define DHTML_EVENT_ONAFTERUPDATE(elemName, memberFxn)\
DHTML_EVENT(DISPID_HTMLELEMENTEVENTS_ONAFTERUPDATE, elemName, memberFxn)\

#define DHTML_EVENT_ONERRORUPDATE(elemName, memberFxn)\
DHTML_EVENT(DISPID_HTMLELEMENTEVENTS_ONERRORUPDATE, elemName, memberFxn)\

#define DHTML_EVENT_ONROWEXIT(elemName, memberFxn)\
DHTML_EVENT(DISPID_HTMLELEMENTEVENTS_ONROWEXIT, elemName, memberFxn)\

#define DHTML_EVENT_ONROWENTER(elemName, memberFxn)\
DHTML_EVENT(DISPID_HTMLELEMENTEVENTS_ONROWENTER, elemName, memberFxn)\

#define DHTML_EVENT_ONDATASETCHANGED(elemName, memberFxn)\
DHTML_EVENT(DISPID_HTMLELEMENTEVENTS_ONDATASETCHANGED, elemName, memberFxn)\

#define DHTML_EVENT_ONDATAAVAILABLE(elemName, memberFxn)\
DHTML_EVENT(DISPID_HTMLELEMENTEVENTS_ONDATAAVAILABLE, elemName, memberFxn)\

#define DHTML_EVENT_ONDATASETCOMPLETE(elemName, memberFxn)\
DHTML_EVENT(DISPID_HTMLELEMENTEVENTS_ONDATASETCOMPLETE, elemName, memberFxn)\

// non-bubbled element events
#define DHTML_EVENT_ONBLUR(elemName, memberFxn)\
DHTML_EVENT_ELEMENT(DISPID_EVMETH_ONBLUR, elemName, memberFxn)\

#define DHTML_EVENT_ONCHANGE(elemName, memberFxn)\
DHTML_EVENT_ELEMENT(DISPID_EVMETH_ONCHANGE, elemName, memberFxn)\

#define DHTML_EVENT_ONFOCUS(elemName, memberFxn)\
DHTML_EVENT_ELEMENT(DISPID_EVMETH_ONFOCUS, elemName, memberFxn)\

#define DHTML_EVENT_ONRESIZE(elemName, memberFxn)\
DHTML_EVENT_ELEMENT(DISPID_EVMETH_ONRESIZE, elemName, memberFxn)\

class CDHtmlEventSink : public CDHtmlSinkHandler
{
public:
	HRESULT ConnectToConnectionPoint(IUnknown *punkObj, REFIID riid, DWORD *pdwCookie)
	{
		return AtlAdvise(punkObj, (IDispatch *)(this), riid, pdwCookie);
	}

	void DisconnectFromConnectionPoint(IUnknown *punkObj, REFIID riid, DWORD& dwCookie)
	{
		AtlUnadvise(punkObj, riid, dwCookie);
	}

	STDMETHOD(CDHtmlSinkHandlerQueryInterface)(REFIID iid, LPVOID* ppvObj)
	{
		if (!ppvObj)
			return E_POINTER;

		*ppvObj = NULL;
		if (IsEqualIID(iid, __uuidof(IDispatch)) || IsEqualIID(iid, __uuidof(IUnknown)))
		{
			*ppvObj = (IDispatch *)(this);
			AddRef();
			return S_OK;
		}
		return E_NOINTERFACE;
	}

	STDMETHOD_(ULONG, AddRef)() { return 1; }
	STDMETHOD_(ULONG, Release)() { return 1; }

	STDMETHOD(GetTypeInfoCount)(UINT *pctinfo)
	{
		*pctinfo = 0;
		ATLTRACENOTIMPL(_T("CDHtmlEventSink::GetTypeInfoCount"));
	}

	STDMETHOD(GetTypeInfo)(UINT /*iTInfo*/, LCID /*lcid*/, ITypeInfo **ppTInfo)
	{
		*ppTInfo = NULL;
		ATLTRACENOTIMPL(_T("CDHtmlEventSink::GetTypeInfo"));
	}

	STDMETHOD(GetIDsOfNames)(REFIID /*riid*/, OLECHAR ** /*rgszNames*/, UINT /*cNames*/, LCID /*lcid*/, DISPID * /*rgDispId*/)
	{
		ATLTRACENOTIMPL(_T("CDHtmlEventSink::GetIDsOfNames"));
	}

	STDMETHOD(Invoke)(DISPID dispIdMember, REFIID /*riid*/, LCID /*lcid*/, WORD /*wFlags*/, DISPPARAMS *pDispParams, VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr)
	{
		IHTMLElement *psrcElement = NULL;
		HRESULT hr = S_OK;

		VariantInit(pVarResult);

		if (DHtmlEventHook(&hr, dispIdMember, pDispParams, pVarResult, pExcepInfo, puArgErr))
			return hr;

		const DHtmlEventMapEntry *pMap = GetDHtmlEventMap();

		int nIndex = FindDHtmlEventEntry(pMap, dispIdMember, &psrcElement);
		if (nIndex<0)
			return DISP_E_MEMBERNOTFOUND;

		// now call it
		if (pMap)
		{
			hr = (this->*((DHEVTFUNC) (GetDHtmlEventMap()[nIndex].pfnEventFunc)))(psrcElement);
			if (GetDHtmlEventMap()[nIndex].nType != DHTMLEVENTMAPENTRY_CONTROL && pVarResult)
			{
				pVarResult->vt = VT_BOOL;
				pVarResult->boolVal = (hr==S_OK) ? ATL_VARIANT_TRUE : ATL_VARIANT_FALSE;
			}
		}
		if (psrcElement)
			psrcElement->Release();

		return hr;
	}

	virtual BOOL DHtmlEventHook(HRESULT * /*phr*/, DISPID /*dispIdMember*/, DISPPARAMS * /*pDispParams*/, VARIANT * /*pVarResult*/, EXCEPINFO * /*pExcepInfo*/, UINT * /*puArgErr*/)
	{
		// stub base implementation
		return FALSE;
	}

	virtual const DHtmlEventMapEntry* GetDHtmlEventMap() = 0;
	virtual HRESULT GetDHtmlDocument(IHTMLDocument2 **pphtmlDoc) = 0;
	//virtual void _OnBeforeNavigate2(LPDISPATCH pDisp, VARIANT FAR* URL, VARIANT FAR* Flags, VARIANT FAR* TargetFrameName, VARIANT FAR* PostData, VARIANT FAR* Headers, BOOL FAR* Cancel) {}
	//virtual void _OnNavigateComplete2(LPDISPATCH pDisp, VARIANT FAR* URL) {}
	//virtual void _OnDocumentComplete(LPDISPATCH pDisp, VARIANT* URL) {}

	int FindDHtmlEventEntry(const DHtmlEventMapEntry *pEventMap, DISPID dispIdMember, IHTMLElement **ppsrcElement)
	{
		HRESULT hr = DISP_E_MEMBERNOTFOUND;
		CComPtr<IHTMLWindow2> sphtmlWnd;
		CComPtr<IHTMLEventObj> sphtmlEvent;
		CComPtr<IHTMLElement> sphtmlElement;
		CComPtr<IHTMLDocument2> sphtmlDoc;
		CComBSTR bstrId, bstrName;
		CComBSTR bstrClass;
		CComBSTR bstrTagName;

		int i;
		int nIndexFound = -1;

		if(ppsrcElement == NULL)
			return E_POINTER;

		*ppsrcElement = NULL;

		if (!pEventMap)
			goto Error;

		// get the html document
		hr = GetDHtmlDocument(&sphtmlDoc);
		if (sphtmlDoc == NULL)
			goto Error;

		// get the element that generated the event
		sphtmlDoc->get_parentWindow(&sphtmlWnd);
		if ((sphtmlWnd==NULL) || FAILED(sphtmlWnd->get_event(&sphtmlEvent)) || (sphtmlEvent==NULL))
		{
			hr = DISP_E_MEMBERNOTFOUND;
			goto Error;
		}
		sphtmlEvent->get_srcElement(&sphtmlElement);
		*ppsrcElement = sphtmlElement;
		if (sphtmlElement)
			sphtmlElement.p->AddRef();
		// look for the dispid in the map
		for (i=0; pEventMap[i].nType != DHTMLEVENTMAPENTRY_END; i++)
		{
			if (pEventMap[i].dispId == dispIdMember)
			{
				if (pEventMap[i].nType == DHTMLEVENTMAPENTRY_NAME)
				{
					if (!bstrId && sphtmlElement)
						sphtmlElement->get_id(&bstrId);
					if (bstrId && pEventMap[i].szName && !wcscmp(bstrId, CComBSTR(pEventMap[i].szName)) ||
						(!bstrId && !sphtmlElement))
					{
						nIndexFound = i;
						break;
					}
				}
				else if (pEventMap[i].nType == DHTMLEVENTMAPENTRY_CLASS)
				{
					if (!bstrClass && sphtmlElement)
						sphtmlElement->get_className(&bstrClass);
					if (bstrClass && !wcscmp(bstrClass, CComBSTR(pEventMap[i].szName)))
					{
						nIndexFound = i;
						break;
					}
				}
				else if (pEventMap[i].nType == DHTMLEVENTMAPENTRY_TAG)
				{
					if (!bstrTagName && sphtmlElement)
						sphtmlElement->get_tagName(&bstrTagName);
					if (bstrTagName && !_wcsicmp(bstrTagName, CComBSTR(pEventMap[i].szName)))
					{
						nIndexFound = i;
						break;
					}
				}
			}
		}
Error:
		if (nIndexFound==-1 && *ppsrcElement)
		{
			(*ppsrcElement)->Release();
			*ppsrcElement = NULL;
		}
		return nIndexFound;
	}
};

class CDHtmlControlSink : public IDispatch
{
public:
	LPCTSTR m_szControlId;
	DWORD m_dwCookie;
	ATL::CComPtr<IUnknown> m_spunkObj;
	IID m_iid;
	GUID m_libid;
	WORD m_wMajor;
	WORD m_wMinor;
	ATL::CComPtr<ITypeInfo> m_spTypeInfo;
	CDHtmlSinkHandler *m_pHandler;
	DWORD_PTR m_dwThunkOffset;

	CDHtmlControlSink()
	{
		m_dwCookie = 0;
		m_pHandler = NULL;
		m_dwThunkOffset = 0;
		memset(&m_iid, 0x00, sizeof(IID));
	}

	CDHtmlControlSink(IUnknown *punkObj, CDHtmlSinkHandler *pHandler, LPCTSTR szControlId, DWORD_PTR dwThunkOffset = 0)
	{
		m_dwCookie = 0;
		m_pHandler = pHandler;
		m_szControlId = szControlId;
		m_dwThunkOffset = dwThunkOffset;
		ConnectToControl(punkObj);
	}

	~CDHtmlControlSink()
	{
		if (m_dwCookie != 0)
			AtlUnadvise(m_spunkObj, m_iid, m_dwCookie);
	}

	HRESULT ConnectToControl(IUnknown *punkObj)
	{
		m_spunkObj = punkObj;
		HRESULT hr = AtlGetObjectSourceInterface(punkObj, &m_libid, &m_iid, &m_wMajor, &m_wMinor);
		if (FAILED(hr))
			return hr;

		CComPtr<ITypeLib> spTypeLib;

		hr = LoadRegTypeLib(m_libid, m_wMajor, m_wMinor, LOCALE_USER_DEFAULT, &spTypeLib);
		if (FAILED(hr))
			return hr;

		hr = spTypeLib->GetTypeInfoOfGuid(m_iid, &m_spTypeInfo);
		if (FAILED(hr))
			return hr;

		return AtlAdvise(punkObj, this, m_iid, &m_dwCookie);
	}

	STDMETHOD_(ULONG, AddRef)() { return 1; }
	STDMETHOD_(ULONG, Release)() { return 1; }

	STDMETHOD(QueryInterface)(REFIID iid, LPVOID* ppvObj)
	{
		if (!ppvObj)
			return E_POINTER;

		*ppvObj = NULL;
		if (IsEqualIID(iid, __uuidof(IUnknown)) || 
			IsEqualIID(iid, __uuidof(IDispatch)) || 
			IsEqualIID(iid, m_iid))
		{
			*ppvObj = this;
			return S_OK;
		}
		return E_NOINTERFACE;
	}

	STDMETHOD(GetTypeInfoCount)(UINT *pctinfo)
	{
		*pctinfo = 0;
		ATLTRACENOTIMPL(_T("CDHtmlControlSink::GetTypeInfoCount"));
	}

	STDMETHOD(GetTypeInfo)(UINT /*iTInfo*/, LCID /*lcid*/, ITypeInfo **ppTInfo)
	{
		*ppTInfo = NULL;
		ATLTRACENOTIMPL(_T("CDHtmlControlSink::GetTypeInfo"));
	}

	STDMETHOD(GetIDsOfNames)(REFIID /*riid*/, OLECHAR ** /*rgszNames*/, UINT /*cNames*/, LCID /*lcid*/, DISPID * /*rgDispId*/)
	{
		ATLTRACENOTIMPL(_T("CDHtmlControlSink::GetIDsOfNames"));
	}

	STDMETHOD(Invoke)(DISPID dispidMember, REFIID /*riid*/, LCID lcid, WORD /*wFlags*/, DISPPARAMS *pdispparams, VARIANT *pvarResult, EXCEPINFO * /*pExcepInfo*/, UINT * /*puArgErr*/)
	{
		ATLASSERT(m_pHandler);
		if (m_pHandler==NULL)
			return S_OK;

		_ATL_FUNC_INFO info;

		BOOL fFound = FALSE;
		DHEVTFUNCCONTROL pEvent = NULL;

		const DHtmlEventMapEntry *pEventMap = m_pHandler->GetDHtmlEventMap();
		for (int i=0; pEventMap[i].nType != DHTMLEVENTMAPENTRY_END; i++)
		{
			if (pEventMap[i].nType==DHTMLEVENTMAPENTRY_CONTROL &&
				pEventMap[i].dispId == dispidMember &&
				!_tcscmp(pEventMap[i].szName, m_szControlId))
			{
				// found the entry
				pEvent = pEventMap[i].pfnEventFunc;
				fFound = TRUE;
				break;
			}
		}
		if (!fFound)
			return DISP_E_MEMBERNOTFOUND;

		HRESULT hr = GetFuncInfoFromId(m_iid, dispidMember, lcid, info);
		if (FAILED(hr))
		{
			return S_OK;
		}
		return InvokeFromFuncInfo(pEvent, info, pdispparams, pvarResult);
	}

	//Helper for invoking the event
	HRESULT InvokeFromFuncInfo(DHEVTFUNCCONTROL pEvent, ATL::_ATL_FUNC_INFO& info, DISPPARAMS* pdispparams, VARIANT* pvarResult)
	{
		USES_ATL_SAFE_ALLOCA;
		if (info.nParams < 0)
		{
			return E_INVALIDARG;
		}
		if (info.nParams > size_t(-1) / sizeof(VARIANTARG*))
		{
			return E_OUTOFMEMORY;
		}
		VARIANTARG** pVarArgs = info.nParams ? (VARIANTARG**)_ATL_SAFE_ALLOCA(sizeof(VARIANTARG*)*info.nParams,_ATL_SAFE_ALLOCA_DEF_THRESHOLD) : 0;
		if(!pVarArgs)
		{
			return E_OUTOFMEMORY;
		}

		for (int i=0; i<info.nParams; i++)
		{
			pVarArgs[i] = &pdispparams->rgvarg[info.nParams - i - 1];
		}

		CComStdCallThunk<CDHtmlSinkHandler> thunk;
		if (m_pHandler)
			thunk.Init(pEvent, reinterpret_cast< CDHtmlSinkHandler* >((DWORD_PTR) m_pHandler - m_dwThunkOffset));

		CComVariant tmpResult;
		if (pvarResult == NULL)
			pvarResult = &tmpResult;

		HRESULT hr = DispCallFunc(
			&thunk,
			0,
			info.cc,
			info.vtReturn,
			info.nParams,
			info.pVarTypes,
			pVarArgs,
			pvarResult);
		ATLASSERT(SUCCEEDED(hr));
		return hr;
	}

	HRESULT GetFuncInfoFromId(const IID& iid, DISPID dispidMember, LCID lcid, ATL::_ATL_FUNC_INFO& info)
	{
		if (!m_spTypeInfo)
			return E_FAIL;
		return AtlGetFuncInfoFromId(m_spTypeInfo, iid, dispidMember, lcid, info);
	}

	VARTYPE GetUserDefinedType(ITypeInfo *pTI, HREFTYPE hrt)
	{
		return AtlGetUserDefinedType(pTI, hrt);
	}
};

class CDHtmlElementEventSink : public IDispatch
{
public:
	CDHtmlEventSink *m_pHandler;
	ATL::CComPtr<IUnknown> m_spunkElem;
	DWORD m_dwCookie;

	CDHtmlElementEventSink(CDHtmlEventSink *pHandler, IDispatch *pdisp)
	{
		m_pHandler = pHandler;
		pdisp->QueryInterface(__uuidof(IUnknown), (void **) &m_spunkElem);
		m_dwCookie = 0;
	}

	STDMETHOD_(ULONG, AddRef)() { return 1; }
	STDMETHOD_(ULONG, Release)() { return 1; }

	STDMETHOD(QueryInterface)(REFIID iid, LPVOID* ppvObj)
	{
		if (!ppvObj)
			return E_POINTER;

		*ppvObj = NULL;
		if (IsEqualIID(iid, __uuidof(IUnknown)) || 
			IsEqualIID(iid, __uuidof(IDispatch)))
		{
			*ppvObj = this;
			return S_OK;
		}
		return E_NOINTERFACE;
	}

	STDMETHOD(GetTypeInfoCount)(UINT *pctinfo)
	{
		*pctinfo = 0;
		ATLTRACENOTIMPL(_T("CDHtmlElementEventSink::GetTypeInfoCount"));
	}

	STDMETHOD(GetTypeInfo)(UINT /*iTInfo*/, LCID /*lcid*/, ITypeInfo **ppTInfo)
	{
		*ppTInfo = NULL;
		ATLTRACENOTIMPL(_T("CDHtmlElementEventSink::GetTypeInfo"));
	}

	STDMETHOD(GetIDsOfNames)(REFIID /*riid*/, OLECHAR ** /*rgszNames*/, UINT /*cNames*/, LCID /*lcid*/, DISPID * /*rgDispId*/)
	{
		ATLTRACENOTIMPL(_T("CDHtmlElementEventSink::GetIDsOfNames"));
	}

	STDMETHOD(Invoke)(DISPID dispIdMember, REFIID /*riid*/, LCID /*lcid*/, WORD /*wFlags*/, DISPPARAMS * /*pdispparams*/, VARIANT *pVarResult, EXCEPINFO * /*pExcepInfo*/, UINT * /*puArgErr*/)
	{
		HRESULT hr;
		CComPtr<IHTMLWindow2> sphtmlWnd;
		CComPtr<IHTMLDocument2> sphtmlDoc;
		CComPtr<IHTMLElement> sphtmlElem;
		CComPtr<IHTMLElement> spsrcElem;
		CComPtr<IHTMLEventObj> sphtmlEvent;

		CComBSTR bstrId;

		if (pVarResult)
			VariantInit(pVarResult);
		hr = m_spunkElem->QueryInterface(&sphtmlElem);
		if (!sphtmlElem)
			return hr;

		hr = sphtmlElem->get_id(&bstrId);
		if (FAILED(hr))
			return hr;

		hr = m_pHandler->GetDHtmlDocument(&sphtmlDoc);
		if (FAILED(hr))
			return hr;

		hr = sphtmlDoc->get_parentWindow(&sphtmlWnd);
		if (FAILED(hr))
			return hr;

		hr = sphtmlWnd->get_event(&sphtmlEvent);
		if (FAILED(hr))
			return hr;

		hr = sphtmlEvent->get_srcElement(&spsrcElem);
		if (FAILED(hr))
			return hr;

		const DHtmlEventMapEntry *pEventMap = m_pHandler->GetDHtmlEventMap();
		for (int i=0; pEventMap[i].nType != DHTMLEVENTMAPENTRY_END; i++)
		{
			if (pEventMap[i].nType!=DHTMLEVENTMAPENTRY_CONTROL &&
				pEventMap[i].dispId == dispIdMember)
			{
				if (pEventMap[i].szName && !wcscmp(CComBSTR(pEventMap[i].szName), bstrId))
				{
					// found the entry
					hr = (m_pHandler->*((DHEVTFUNC) (m_pHandler->GetDHtmlEventMap()[i].pfnEventFunc)))(spsrcElem);
					if (pVarResult)
					{
						pVarResult->vt = VT_BOOL;
						pVarResult->boolVal = (hr==S_OK) ? ATL_VARIANT_TRUE : ATL_VARIANT_FALSE;
					}
					return S_OK;
				}
			}
		}

		return DISP_E_MEMBERNOTFOUND;
	}

	HRESULT Advise(LPUNKNOWN pUnkObj, REFIID iid)
	{
		return AtlAdvise((LPUNKNOWN)pUnkObj, (LPDISPATCH)this, iid, &m_dwCookie);
	}

	HRESULT UnAdvise(LPUNKNOWN pUnkObj, REFIID /*iid*/)
	{
		return AtlUnadvise((LPUNKNOWN)pUnkObj, __uuidof(HTMLElementEvents), m_dwCookie);
	}
};

class CExternalDispatchBase {};

template<class T>
class CExternalDispatchImpl : public IDispatch, public CExternalDispatchBase
{
public:
	STDMETHOD_(ULONG, AddRef)() { return 1; }
	STDMETHOD_(ULONG, Release)() { return 1; }
	STDMETHOD(QueryInterface)(REFIID iid, LPVOID* ppvObj)
	{
		if (!ppvObj)
			return E_POINTER;

		*ppvObj = NULL;
		if (IsEqualIID(iid, __uuidof(IUnknown)) || 
			IsEqualIID(iid, __uuidof(IDispatch)))
		{
			*ppvObj = this;
			return S_OK;
		}
		return E_NOINTERFACE;
	}
	STDMETHOD(GetTypeInfoCount)(UINT *pctinfo)
	{
		*pctinfo = 0;
		ATLTRACENOTIMPL(_T("CExternalDispatchImpl::GetTypeInfoCount"));
	}
	STDMETHOD(GetTypeInfo)(UINT /*iTInfo*/, LCID /*lcid*/, ITypeInfo **ppTInfo)
	{
		*ppTInfo = NULL;
		ATLTRACENOTIMPL(_T("CExternalDispatchImpl::GetTypeInfo"));
	}
	STDMETHOD(GetIDsOfNames)(REFIID /*riid*/, OLECHAR ** rgszNames, UINT cNames, LCID /*lcid*/, DISPID * rgDispId)
	{
		for (UINT i=0; i<cNames; i++)
		{
			rgDispId[i] = DISPID_UNKNOWN;
		}

		ATLASSERT(rgszNames);
		if (cNames != 1)
			return DISP_E_UNKNOWNNAME; // named parameter has not been supported!!!

		long lMapNum = 0;
		const DHtmlExternalMethodMapEntry* pMap = (static_cast<T*>(this))->_GetExternalMethodMapEntry(&lMapNum);
		ATLASSERT(pMap);

		CComBSTR strName = rgszNames[0];
		for (long i=0; i<lMapNum; i++)
		{
			if (pMap[i].szName == NULL)
				continue;

			CComBSTR strMapName = pMap[i].szName;
			if (strMapName==strName)
			{
				*rgDispId = i;
				return S_OK;
			}
		}
		return DISP_E_UNKNOWNNAME;
	}
	STDMETHOD(Invoke)(DISPID dispIdMember, REFIID /*riid*/, LCID /*lcid*/, WORD /*wFlags*/, DISPPARAMS * pdispparams, VARIANT *pVarResult, EXCEPINFO * /*pExcepInfo*/, UINT * /*puArgErr*/)
	{
		//HRESULT hr;
		if (pVarResult)
			VariantClear(pVarResult);

		return (static_cast<T*>(this))->OnExternalInvoke(dispIdMember, pdispparams);
	}

	IDispatch* GetExternalDispatch()
	{
		return (IDispatch*)this;
	}
};

template<class T>
class IDocHostUIHandlerDispatchImpl : public IDocHostUIHandlerDispatch
{
public:
	// IUnknown methods
	STDMETHOD_(ULONG, AddRef)() { return 1; }
	STDMETHOD_(ULONG, Release)() { return 1; }
	STDMETHOD(QueryInterface)(REFIID iid, LPVOID* ppvObj)
	{
		if (!ppvObj)
			return E_POINTER;

		*ppvObj = NULL;
		if (IsEqualIID(iid, __uuidof(IUnknown)) || 
			IsEqualIID(iid, __uuidof(IDispatch)) || 
			IsEqualIID(iid, __uuidof(IDocHostUIHandlerDispatch)) ||
			IsEqualIID(iid, __uuidof(IOleCommandTarget)))
		{
			*ppvObj = this;
			return S_OK;
		}
		return E_NOINTERFACE;
	}

	// IDispatch methods
	STDMETHOD(GetTypeInfoCount)(UINT * /*pctinfo*/)
	{
		ATLTRACENOTIMPL(_T("IDocHostUIHandlerDispatchImpl::GetTypeInfoCount"));
	}
	STDMETHOD(GetTypeInfo)(UINT /*iTInfo*/, LCID /*lcid*/, ITypeInfo **ppTInfo)
	{
		*ppTInfo = NULL;
		ATLTRACENOTIMPL(_T("IDocHostUIHandlerDispatchImpl::GetTypeInfo"));
	}
	STDMETHOD(GetIDsOfNames)(REFIID /*riid*/, OLECHAR ** /*rgszNames*/, UINT /*cNames*/, LCID /*lcid*/, DISPID * /*rgDispId*/)
	{
		ATLTRACENOTIMPL(_T("IDocHostUIHandlerDispatchImpl::GetIDsOfNames"));
	}
	STDMETHOD(Invoke)(DISPID /*dispIdMember*/, REFIID /*riid*/, LCID /*lcid*/, WORD /*wFlags*/,
		DISPPARAMS * /*pdispparams*/, VARIANT * /*pVarResult*/, EXCEPINFO * /*pExcepInfo*/, UINT * /*puArgErr*/)
	{
		ATLTRACENOTIMPL(_T("IDocHostUIHandlerDispatchImpl::Invoke"));
	}

	// IDocHostUIHandlerDispatch methods
	STDMETHOD(ShowContextMenu)(DWORD /*dwID*/, DWORD /*x*/, DWORD /*y*/, IUnknown * /*pcmdtReserved*/, IDispatch * /*pdispReserved*/, HRESULT *dwRetVal)
	{
		*dwRetVal = S_OK;
		return S_OK;
	}
	STDMETHOD(GetHostInfo)(DWORD *pdwFlags, DWORD * /*pdwDoubleClick*/)
	{
		*pdwFlags = (static_cast<T*>(this))->m_dwHostFlags;
		return S_OK;
	}
	STDMETHOD(ShowUI)(DWORD /*dwID*/, IUnknown * /*pActiveObject*/, IUnknown * /*pCommandTarget*/, IUnknown * /*pFrame*/, IUnknown * /*pDoc*/, HRESULT *dwRetVal)
	{
		*dwRetVal = S_FALSE;
		return S_OK;
	}
	STDMETHOD(HideUI)(void)
	{
		return E_NOTIMPL;
	}
	STDMETHOD(UpdateUI)(void)
	{
		return E_NOTIMPL;
	}
	STDMETHOD(EnableModeless)(VARIANT_BOOL /*fEnable*/)
	{
		return E_NOTIMPL;
	}
	STDMETHOD(OnDocWindowActivate)(VARIANT_BOOL /*fActivate*/)
	{
		return E_NOTIMPL;
	}
	STDMETHOD(OnFrameWindowActivate)(VARIANT_BOOL /*fActivate*/)
	{
		return E_NOTIMPL;
	}
	STDMETHOD(ResizeBorder)(long /*left*/, long /*top*/, long /*right*/, long /*bottom*/, IUnknown * /*pUIWindow*/, VARIANT_BOOL /*fFrameWindow*/)
	{
		return E_NOTIMPL;
	}
#define KEY_DOWN(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 1 : 0)
#define CTRL_DOWN() KEY_DOWN(VK_CONTROL)
#define ALT_DOWN() KEY_DOWN(VK_MENU)
#define SHIFT_DOWN() KEY_DOWN(VK_SHIFT)
	STDMETHOD(TranslateAccelerator)(DWORD_PTR /*hWnd*/, DWORD nMessage, DWORD_PTR wParam, DWORD_PTR /*lParam*/, BSTR /*bstrGuidCmdGroup*/, DWORD /*nCmdID*/, HRESULT *dwRetVal)
	{
		if ((nMessage==WM_KEYDOWN || nMessage==WM_KEYUP))
		{
#ifdef _DEBUG
			if (wParam==VK_F5)
			{
				*dwRetVal = S_FALSE;
				return S_OK;
			}
#endif // _DEBUG
			if (wParam==VK_BACK ||
				(wParam>=VK_F1 && wParam<=VK_F12) ||
				wParam==VK_ESCAPE ||
				(ALT_DOWN() && (wParam==VK_HOME || wParam==VK_LEFT || wParam==VK_RIGHT)) ||
				(CTRL_DOWN()))
			{
				*dwRetVal = S_OK;
				return S_OK;
			}
		}
		else if ((nMessage==WM_SYSKEYDOWN || nMessage==WM_SYSKEYUP) && wParam!=VK_MENU)
		{
			if (wParam==VK_HOME || wParam==VK_LEFT || wParam==VK_RIGHT)
			{
				*dwRetVal = S_OK;
				return S_OK;
			}
		}
		*dwRetVal = S_FALSE;
		return S_OK;
	}
	STDMETHOD(GetOptionKeyPath)(BSTR *pbstrKey, DWORD /*dw*/)
	{
		*pbstrKey = NULL;
		return E_NOTIMPL;
	}
	STDMETHOD(GetDropTarget)(IUnknown * /*pDropTarget*/, IUnknown **ppDropTarget)
	{
		*ppDropTarget = NULL;
		return E_NOTIMPL;
	}
	STDMETHOD(GetExternal)(IDispatch **ppDispatch)
	{
		if(ppDispatch == NULL)
			return E_POINTER;

		*ppDispatch = NULL;
		T* pThis = static_cast<T*>(this);
		if (pThis->CanAccessExternal())
		{
			*ppDispatch = pThis->GetExternalDispatch();
			return S_OK;
		}
		return E_NOTIMPL;
	}
	STDMETHOD(TranslateUrl)(DWORD /*dwTranslate*/, BSTR /*bstrURLIn*/, BSTR *pbstrURLOut)
	{
		if(pbstrURLOut == NULL)
			return E_POINTER;
		*pbstrURLOut = NULL;
		return S_FALSE;
	}
	STDMETHOD(FilterDataObject)(IUnknown * /*pDO*/, IUnknown **ppDORet)
	{
		if(ppDORet == NULL)
			return E_POINTER;
		*ppDORet = NULL;
		return S_FALSE;
	}
};

template<class T>
class DWebBrowserEvent2Impl : public IDispatch
{
public:
	DWebBrowserEvent2Impl() : m_dwCookie(0) {}

public:
	// IUnknown methods
	STDMETHOD_(ULONG, AddRef)() { return 1; }
	STDMETHOD_(ULONG, Release)() { return 1; }
	STDMETHOD(QueryInterface)(REFIID iid, LPVOID* ppvObj)
	{
		if (!ppvObj)
			return E_POINTER;

		*ppvObj = NULL;
		if (IsEqualIID(iid, __uuidof(IUnknown)) || 
			IsEqualIID(iid, __uuidof(IDispatch)) || 
			IsEqualIID(iid, (DIID_DWebBrowserEvents2)))
		{
			*ppvObj = this;
			return S_OK;
		}
		return E_NOINTERFACE;
	}

	// IDispatch methods
	STDMETHOD(GetTypeInfoCount)(UINT * /*pctinfo*/)
	{
		ATLTRACENOTIMPL(_T("DWebBrowserEvent2Impl::GetTypeInfoCount"));
	}
	STDMETHOD(GetTypeInfo)(UINT /*iTInfo*/, LCID /*lcid*/, ITypeInfo **ppTInfo)
	{
		*ppTInfo = NULL;
		ATLTRACENOTIMPL(_T("DWebBrowserEvent2Impl::GetTypeInfo"));
	}
	STDMETHOD(GetIDsOfNames)(REFIID /*riid*/, OLECHAR ** /*rgszNames*/, UINT /*cNames*/, LCID /*lcid*/, DISPID * /*rgDispId*/)
	{
		ATLTRACENOTIMPL(_T("DWebBrowserEvent2Impl::GetIDsOfNames"));
	}
	STDMETHOD(Invoke)(DISPID dispIdMember, REFIID /*riid*/, LCID /*lcid*/, WORD wFlags, DISPPARAMS *pdispparams, VARIANT * /*pVarResult*/, EXCEPINFO * /*pExcepInfo*/, UINT * /*puArgErr*/)
	{
		ATLASSERT(wFlags==DISPATCH_METHOD);

		if (dispIdMember==DISPID_BEFORENAVIGATE2)
		{
			// BeforeNavigate2(LPDISPATCH pDisp, VARIANT FAR* URL, VARIANT FAR* Flags, VARIANT FAR* TargetFrameName, VARIANT FAR* PostData, VARIANT FAR* Headers, BOOL FAR* Cancel)
			ATLASSERT(pdispparams->cArgs==7);
			int npdisp = 6;
			int nURL = 5;
			ATLASSERT(V_VT(&pdispparams->rgvarg[npdisp])==VT_DISPATCH && V_VT(&pdispparams->rgvarg[nURL])==(VT_VARIANT|VT_BYREF));
			(static_cast<T*>(this))->_OnBeforeNavigate2(pdispparams->rgvarg[npdisp].pdispVal, pdispparams->rgvarg[nURL].pvarVal);
		}
		else if (dispIdMember==DISPID_NAVIGATECOMPLETE2)
		{
			// NavigateComplete2(LPDISPATCH pDisp, VARIANT FAR* URL)
			ATLASSERT(pdispparams->cArgs==2);
			int npdisp = 1;
			int nURL = 0;
			ATLASSERT(V_VT(&pdispparams->rgvarg[npdisp])==VT_DISPATCH && V_VT(&pdispparams->rgvarg[nURL])==(VT_VARIANT|VT_BYREF));
			(static_cast<T*>(this))->_OnNavigateComplete2(pdispparams->rgvarg[npdisp].pdispVal, pdispparams->rgvarg[nURL].pvarVal);
		}
		else if (dispIdMember==DISPID_DOCUMENTCOMPLETE)
		{
			// DocumentComplete(LPDISPATCH pDisp, VARIANT* URL)
			ATLASSERT(pdispparams->cArgs==2);
			int npdisp = 1;
			int nURL = 0;
			ATLASSERT(V_VT(&pdispparams->rgvarg[npdisp])==VT_DISPATCH && V_VT(&pdispparams->rgvarg[nURL])==(VT_VARIANT|VT_BYREF));
			(static_cast<T*>(this))->_OnDocumentComplete(pdispparams->rgvarg[npdisp].pdispVal, pdispparams->rgvarg[nURL].pvarVal);
		}

		return S_OK;
	}

	HRESULT ConnectToWebBrowser(IWebBrowser2* pBrowser)
	{
		if (pBrowser==NULL)
			return E_POINTER;

		HRESULT hr = S_OK;
		if (m_dwCookie==0)
		{
			hr = AtlAdvise(pBrowser, this, DIID_DWebBrowserEvents2, &m_dwCookie);
		}
		
		return hr;
	}

	HRESULT DisconnectToWebBrowser(IWebBrowser2* pBrowser)
	{
		if (pBrowser==NULL)
			return E_POINTER;

		HRESULT hr = S_OK;
		if (m_dwCookie!=0)
		{
			hr = AtlUnadvise(pBrowser, DIID_DWebBrowserEvents2, m_dwCookie);
			if (SUCCEEDED(hr))
				m_dwCookie = 0;
		}
		return hr;
	}

protected:
	DWORD m_dwCookie;
};

//// DDX_DHtml helper macros
//#define DDX_DHtml_ElementValue(dx, name, var) DDX_DHtml_ElementText(dx, name, DISPID_A_VALUE, var)
//#define DDX_DHtml_ElementInnerText(dx, name, var) DDX_DHtml_ElementText(dx, name, DISPID_IHTMLELEMENT_INNERTEXT, var)
//#define DDX_DHtml_ElementInnerHtml(dx, name, var) DDX_DHtml_ElementText(dx, name, DISPID_IHTMLELEMENT_INNERHTML, var)
//#define DDX_DHtml_Anchor_Href(dx, name, var) DDX_DHtml_ElementText(dx, name, DISPID_IHTMLANCHORELEMENT_HREF, var)
//#define DDX_DHtml_Anchor_Target(dx, name, var) DDX_DHtml_ElementText(dx, name, DISPID_IHTMLANCHORELEMENT_TARGET, var)
//#define DDX_DHtml_Img_Src(dx, name, var) DDX_DHtml_ElementText(dx, name, DISPID_IHTMLIMGELEMENT_SRC, var)
//#define DDX_DHtml_Frame_Src(dx, name, var) DDX_DHtml_ElementText(dx, name, DISPID_IHTMLFRAMEBASE_SRC, var)
//#define DDX_DHtml_IFrame_Src(dx, name, var) DDX_DHtml_ElementText(dx, name, DISPID_IHTMLFRAMEBASE_SRC, var)

template <class T, class TBase = ATL::CWindow>
class ATL_NO_VTABLE CDHtmlDialogImpl :
	public ATL::CDialogImplBaseT< TBase >,
	public CDHtmlEventSink,
	public CExternalDispatchImpl<CDHtmlDialogImpl<T,TBase> >,
	public IDocHostUIHandlerDispatchImpl<CDHtmlDialogImpl<T,TBase> >,
	public DWebBrowserEvent2Impl<CDHtmlDialogImpl<T,TBase> >
{
public:
	typedef CDHtmlDialogImpl<T,TBase> thisClass;
	typedef ATL::CDialogImplBaseT< TBase > baseDialog;
	typedef IDocHostUIHandlerDispatchImpl<CDHtmlDialogImpl<T,TBase> > DocHostUIHandlerDispatch;
	typedef DWebBrowserEvent2Impl<CDHtmlDialogImpl<T,TBase> > WebBrowserEvent2;

public:
	virtual DLGPROC GetDialogProc()
	{
		return DialogProc;
	}
	static INT_PTR CALLBACK DialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		thisClass* pThis = (thisClass*)hWnd;

		if (uMsg == WM_INITDIALOG)
		{
			pThis->m_wndBrowser.Create(pThis->m_hWnd, NULL, NULL, WS_CHILD|WS_VISIBLE);
			if (pThis->m_wndBrowser.IsWindow())
			{
				LPOLESTR szClsid = NULL;
				if (S_OK == StringFromCLSID(CLSID_WebBrowser, &szClsid))
				{
					if (SUCCEEDED(pThis->m_wndBrowser.CreateControl(szClsid)))
					{
						pThis->m_wndBrowser.SetExternalUIHandler((DocHostUIHandlerDispatch*)pThis);
						pThis->m_wndBrowser.QueryControl(IID_IWebBrowser2, (void**)&pThis->m_pBrowserApp);
						pThis->ConnectToWebBrowser(pThis->m_pBrowserApp);
					}
					CoTaskMemFree(szClsid);
				}
				//pThis->m_wndBrowser.ShowWindow(SW_SHOW);
				pThis->SendMessage(WM_SIZE);
			}

			USES_CONVERSION;
			if (pThis->m_strDlgCaption.Length()>0)
				pThis->SetWindowText(W2CT(pThis->m_strDlgCaption));

			if (pThis->m_strCurrentUrl)
				pThis->Navigate(OLE2CT(pThis->m_strCurrentUrl));
		}
		else if (uMsg == WM_DESTROY)
		{
			pThis->DisconnectDHtmlEvents();
			pThis->m_spHtmlDoc = NULL;

			// now tell the browser control we're shutting down
			if (pThis->m_pBrowserApp)
			{
				pThis->DisconnectToWebBrowser(pThis->m_pBrowserApp);

				CComPtr<IOleObject> spObject;
				pThis->m_pBrowserApp->QueryInterface(IID_IOleObject, (void **) &spObject);
				if (spObject != NULL)
				{
					spObject->Close(OLECLOSE_NOSAVE);
					spObject.Release();
				}
				pThis->m_pBrowserApp = NULL;
			}

			if (pThis->m_wndBrowser.IsWindow())
				pThis->m_wndBrowser.DestroyWindow();
		}
		else if (uMsg == WM_SIZE)
		{
			if (!pThis->m_bAttachedControl && pThis->m_wndBrowser.m_hWnd)
			{
				RECT rc;
				pThis->GetClientRect(&rc);
				pThis->m_wndBrowser.MoveWindow(0, 0, (int)(rc.right-rc.left), (int)(rc.bottom-rc.top));
			}
		}
		else if (uMsg == WM_GETMINMAXINFO)
		{
			if (!pThis->m_AutoSize)
			{
				MINMAXINFO* lpMMI = (MINMAXINFO*)lParam;
				lpMMI->ptMinTrackSize.x = pThis->m_Width;
				lpMMI->ptMinTrackSize.y = pThis->m_Height;
				lpMMI->ptMaxTrackSize.x = pThis->m_Width;
				lpMMI->ptMaxTrackSize.y = pThis->m_Height;
			}
		}
		return baseDialog::DialogProc(hWnd, uMsg, wParam, lParam);
	}
	// modal dialogs
	INT_PTR DoModal(HWND hWndParent = ::GetActiveWindow(), LPARAM dwInitParam = NULL)
	{
		BOOL result;

		ATLASSUME(m_hWnd == NULL);

		// Allocate the thunk structure here, where we can fail
		// gracefully.

		result = m_thunk.Init(NULL,NULL);
		if (result == FALSE) 
		{
			SetLastError(ERROR_OUTOFMEMORY);
			return -1;
		}

		_AtlWinModule.AddCreateWndData(&m_thunk.cd, (CDialogImplBaseT< TBase >*)this);

		return ::DialogBoxIndirectParam(_AtlBaseModule.GetResourceInstance(), ATL::_DialogSplitHelper::SplitDialogTemplate((LPDLGTEMPLATE)T::GetDialogTemplate(GetDialogStyle()), NULL),
			hWndParent, T::StartDialogProc, dwInitParam);
	}
	BOOL EndDialog(int nRetCode)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::EndDialog(m_hWnd, nRetCode);
	}
	// modeless dialogs
	HWND Create(HWND hWndParent, LPARAM dwInitParam = NULL)
	{
		BOOL result;

		ATLASSUME(m_hWnd == NULL);

		// Allocate the thunk structure here, where we can fail
		// gracefully.

		result = m_thunk.Init(NULL,NULL);
		if (result == FALSE) 
		{
			SetLastError(ERROR_OUTOFMEMORY);
			return NULL;
		}

		_AtlWinModule.AddCreateWndData(&m_thunk.cd, (CDialogImplBaseT< TBase >*)this);
		
		//它自定义的template
		//HWND hWnd = ::CreateDialogIndirectParam(
		//	_AtlBaseModule.GetResourceInstance(), 
		//	ATL::_DialogSplitHelper::SplitDialogTemplate((LPDLGTEMPLATE)T::GetDialogTemplate(static_cast<T*>(this)->GetDialogStyle()), NULL),
		//	hWndParent, 
		//	T::StartDialogProc, 
		//	dwInitParam);

		HWND hWnd = ::CreateDialogParam(
			_AtlBaseModule.GetResourceInstance(), 
			MAKEINTRESOURCE(static_cast<T*>(this)->IDD),
			hWndParent, 
			T::StartDialogProc, 
			dwInitParam);

		ATLASSUME(m_hWnd == hWnd);
		return hWnd;
	}
	// for CComControl
	HWND Create(HWND hWndParent, RECT&, LPARAM dwInitParam = NULL)
	{
		return Create(hWndParent, dwInitParam);
	}
	BOOL DestroyWindow()
	{
		ATLASSERT(::IsWindow(m_hWnd));

		return ::DestroyWindow(m_hWnd);

		//if (!thisClass::WM_DESTROYMODELESS)
		//	thisClass::WM_DESTROYMODELESS = RegisterWindowMessage(_T("WTLDHtmlDialogDestroy"));
		//return PostMessage(thisClass::WM_DESTROYMODELESS, 0, 0);
	}

public:
	CComBSTR m_strCurrentUrl;
	ATL::CComPtr<IWebBrowser2> m_pBrowserApp;
	ATL::CComPtr<IHTMLDocument2> m_spHtmlDoc;
	BOOL m_bUseHtmlTitle;
	BOOL	m_AutoSize;
	LONG	m_Height;
	LONG	m_Width;

	CDHtmlDialogImpl(UINT nHtmlResID = 0)
	{	
#ifdef _DEBUG
		m_bModal = false;
#endif // _DEBUG
		Initialize();
	}
	CDHtmlDialogImpl(LPCTSTR szHtmlResID)
	{
#ifdef _DEBUG
		m_bModal = false;
#endif // _DEBUG
		Initialize();
		m_szHtmlResID = const_cast<LPTSTR>(szHtmlResID);
	}
	virtual ~CDHtmlDialogImpl() {}

	//virtual BOOL CreateControlSite(COleControlContainer* pContainer, 
	//	COleControlSite** ppSite, UINT /* nID */, REFCLSID /* clsid */);
	HRESULT GetDHtmlDocument(IHTMLDocument2 **pphtmlDoc)
	{
		if(pphtmlDoc == NULL)
		{
			ATLASSERT(FALSE);
			return E_POINTER;
		}

		*pphtmlDoc = NULL;

		if (m_spHtmlDoc)
		{
			*pphtmlDoc = m_spHtmlDoc;
			(*pphtmlDoc)->AddRef();
			return S_OK;
		}
		return E_NOINTERFACE;
	}
	void GetCurrentUrl(CComBSTR& szUrl)
	{
		szUrl = m_strCurrentUrl;
	}
	virtual BOOL CanAccessExternal()
	{
		// if the dispatch we have is safe, 
		// we allow access
		if (IsExternalDispatchSafe())
			return TRUE;

		// the external dispatch is not safe, so we check
		// whether the current zone allows for scripting
		// of objects that are not safe for scripting
		if (m_spHtmlDoc == NULL)
			return FALSE;

		CComPtr<IInternetHostSecurityManager> spSecMan;
		m_spHtmlDoc->QueryInterface(IID_IInternetHostSecurityManager,
			(void **) &spSecMan);
		if (spSecMan == NULL)
			return FALSE;

		HRESULT hr = spSecMan->ProcessUrlAction(URLACTION_ACTIVEX_OVERRIDE_OBJECT_SAFETY,
			NULL, 0, NULL, 0, 0, PUAF_DEFAULT);
		if (hr == S_OK)
			return TRUE;
		return FALSE;
	}
	void OnBeforeNavigate(LPDISPATCH pDisp, LPCTSTR szUrl)
	{
		szUrl; // unused
	}
	void OnNavigateComplete(LPDISPATCH pDisp, LPCTSTR szUrl)
	{
	}
	void OnDocumentComplete(LPDISPATCH pDisp, LPCTSTR szUrl)
	{
		szUrl; // unused
	}

	BOOL LoadFromResource(LPCTSTR lpszResource)
	{
		HINSTANCE hInstance = _AtlBaseModule.GetResourceInstance();
		ATLASSERT(hInstance != NULL);

		TCHAR strResourceURL[300] = _T("");
		BOOL bRetVal = TRUE;
		LPTSTR lpszModule = new TCHAR[_MAX_PATH];

		int ret = GetModuleFileName(hInstance, lpszModule, _MAX_PATH);

		if (ret == 0 || ret == _MAX_PATH)
			bRetVal = FALSE;
		else
		{
			_stprintf_s<300>(strResourceURL, _T("res://%s/%s"), lpszModule, lpszResource);
			Navigate(strResourceURL, 0, 0, 0);
		}

		delete [] lpszModule;
		return bRetVal;
	}
	BOOL LoadFromResource(UINT nRes)
	{
		HINSTANCE hInstance = _AtlBaseModule.GetResourceInstance();
		ATLASSERT(hInstance != NULL);

		TCHAR strResourceURL[300] = _T("");
		BOOL bRetVal = TRUE;
		LPTSTR lpszModule = new TCHAR[_MAX_PATH];

		if (GetModuleFileName(hInstance, lpszModule, _MAX_PATH))
		{
			_stprintf_s<300>(strResourceURL, _T("res://%s/%d"), lpszModule, nRes);
			Navigate(strResourceURL, 0, 0, 0);
		}
		else
			bRetVal = FALSE;

		delete [] lpszModule;
		return bRetVal;
	}
	void Navigate(LPCTSTR lpszURL, DWORD dwFlags= 0, LPCTSTR lpszTargetFrameName = NULL,
		LPCTSTR lpszHeaders = NULL, LPVOID lpvPostData = NULL,
		DWORD dwPostDataLen = 0)
	{
		CComBSTR bstrURL = lpszURL;

		LPSAFEARRAY psa = NULL;
		//CComSafeArray<BYTE> saPostData;
		if (lpvPostData != NULL)
		{
			if (dwPostDataLen == 0)
				dwPostDataLen = lstrlen((LPCTSTR) lpvPostData);

			SAFEARRAYBOUND sab = {dwPostDataLen, 0};
			psa = SafeArrayCreate(VT_UI1, 1, &sab);
			if (psa)
			{
				SafeArrayLock(psa);
				memcpy(psa->pvData, lpvPostData, dwPostDataLen);
				SafeArrayUnlock(psa);
			}
			//saPostData.Add(dwPostDataLen, (LPCBYTE)lpvPostData);
		}

		CComVariant vFlags((long)dwFlags);
		CComVariant vTargetFrameName(lpszTargetFrameName);
		CComVariant vHeaders(lpszHeaders);

		if (psa)
		{
			CComVariant vPostData(psa);
			m_pBrowserApp->Navigate(bstrURL, &vFlags, &vTargetFrameName, &vPostData, &vHeaders);
			SafeArrayDestroy(psa);
		}
		else
		{
			m_pBrowserApp->Navigate(bstrURL, &vFlags, &vTargetFrameName, NULL, &vHeaders);
		}
	}

	// host handler
	void SetHostFlags(DWORD dwFlags)
	{
		m_dwHostFlags = dwFlags;
	}
	virtual BOOL IsExternalDispatchSafe()
	{
		return TRUE;
	}

	template<class Q>
	HRESULT GetElementByClass(LPCTSTR szElementClass, Q** ppElem)
	{
		if (!ppElem)
			return E_POINTER;
		if (szElementClass==NULL)
			return E_INVALIDARG;
		*ppElem = NULL;

		CComBSTR strElementClass = szElementClass;
		HRESULT hr = S_OK;
		CComPtr<IHTMLDocument2> sphtmlDoc;
		hr = GetDHtmlDocument(&sphtmlDoc);
		if (sphtmlDoc == NULL)
			return hr;

		CComPtr<IHTMLElementCollection> spElemAll;
		hr = sphtmlDoc->get_all(&spElemAll);
		if (spElemAll==NULL)
			return E_NOINTERFACE;

		long len = 0;
		hr = spElemAll->get_length(&len);
		for (long i=0; i<len; i++)
		{
			CComPtr<IDispatch> spdisp;
			hr = spElemAll->item(CComVariant(i), CComVariant(i), &spdisp);
			if (spdisp==NULL)
				continue;

			CComQIPtr<IHTMLElement> spElem = spdisp;
			if (spElem==NULL)
				continue;

			CComBSTR className;
			hr = spElem->get_className(&className);
			if (strElementClass==className)
			{
				hr = spElem.QueryInterface(ppElem);
				break;
			}
		}
		return hr;
	}
	HRESULT GetElementsByTag(LPCTSTR szElementTag, IHTMLElementCollection **ppElemColl)
	{
		if (!ppElemColl)
			return E_POINTER;
		if(szElementTag == NULL)
			return E_INVALIDARG;

		*ppElemColl = NULL;

		HRESULT hr = S_OK;
		CComPtr<IHTMLDocument2> sphtmlDoc;
		hr = GetDHtmlDocument(&sphtmlDoc);
		if (sphtmlDoc == NULL)
			return hr;

		CComQIPtr<IHTMLDocument3> spDoc3 = sphtmlDoc;
		if (spDoc3==NULL)
			return E_FAIL;

		CComBSTR strElementTag = szElementTag;
		return spDoc3->getElementsByTagName(strElementTag, ppElemColl);
	}
	template<class Q>
	HRESULT GetElementByTag(LPCTSTR szElementTag, Q** ppElem, long idx=0)
	{
		if (ppElem==NULL)
			return E_POINTER;
		*ppElem = NULL;
		HRESULT hr = S_OK;
		CComPtr<IHTMLElementCollection> spElemColl;
		hr = GetElementsByTag(szElementTag, &spElemColl);
		if (spElemColl==NULL)
			return E_NOINTERFACE;

		CComPtr<IDispatch> spdispElem;
		hr = spElemColl->item(CComVariant(idx), CComVariant(idx), &spdispElem);
		if (spdispElem)
			hr = spdispElem.QueryInterface(ppElem);
		return hr;
	}
	HRESULT GetElementsByName(LPCTSTR szElementId, IHTMLElementCollection **ppElemColl)
	{
		if (!ppElemColl)
			return E_POINTER;
		if(szElementId == NULL)
			return E_INVALIDARG;

		*ppElemColl = NULL;

		HRESULT hr = S_OK;
		CComPtr<IHTMLDocument2> sphtmlDoc;
		hr = GetDHtmlDocument(&sphtmlDoc);
		if (sphtmlDoc == NULL)
			return hr;

		CComQIPtr<IHTMLDocument3> spDoc3 = sphtmlDoc;
		if (spDoc3==NULL)
			return E_FAIL;

		CComBSTR strElementId = szElementId;
		return spDoc3->getElementsByName(strElementId, ppElemColl);
	}
	template<class Q>
	HRESULT GetElement(LPCTSTR szElementId, Q **ppElem, long idx=0)
	{
		if (ppElem==NULL)
			return E_POINTER;
		*ppElem = NULL;
		HRESULT hr = S_OK;
		CComPtr<IHTMLElementCollection> spElemColl;
		hr = GetElementsByName(szElementId, &spElemColl);
		if (spElemColl==NULL)
			return E_NOINTERFACE;

		CComPtr<IDispatch> spdispElem;
		hr = spElemColl->item(CComVariant(idx), CComVariant(idx), &spdispElem);
		if (spdispElem)
			hr = spdispElem.QueryInterface(ppElem);
		return hr;
	}

	BSTR GetElementText(LPCTSTR szElementId)
	{
		BSTR bstrText = NULL;
		CComPtr<IHTMLElement> sphtmlElem;
		GetElement(szElementId, &sphtmlElem);
		if (sphtmlElem)
			sphtmlElem->get_innerText(&bstrText);
		return bstrText;
	}
	void SetElementText(LPCTSTR szElementId, BSTR bstrText)
	{
		CComPtr<IHTMLElement> sphtmlElem;
		GetElement(szElementId, &sphtmlElem);
		if (sphtmlElem)
			sphtmlElem->put_innerText(bstrText);
	}
	void SetElementText(IUnknown *punkElem, BSTR bstrText)
	{
		if (punkElem==NULL)
			return;
		CComPtr<IHTMLElement> sphtmlElem;
		punkElem->QueryInterface(__uuidof(IHTMLElement), (void **) &sphtmlElem);
		if (sphtmlElem != NULL)
			sphtmlElem->put_innerText(bstrText);
	}
	BSTR GetElementHtml(LPCTSTR szElementId)
	{
		BSTR bstrText = NULL;
		CComPtr<IHTMLElement> sphtmlElem;
		GetElement(szElementId, &sphtmlElem);
		if (sphtmlElem)
			sphtmlElem->get_innerHTML(&bstrText);
		return bstrText;
	}
	void SetElementHtml(LPCTSTR szElementId, BSTR bstrText)
	{
		CComPtr<IHTMLElement> sphtmlElem;
		GetElement(szElementId, &sphtmlElem);
		if (sphtmlElem)
			sphtmlElem->put_innerHTML(bstrText);
	}
	void SetElementHtml(IUnknown *punkElem, BSTR bstrText)
	{
		if (punkElem==NULL)
			return;
		CComPtr<IHTMLElement> sphtmlElem;
		punkElem->QueryInterface(__uuidof(IHTMLElement), (void **) &sphtmlElem);
		if (sphtmlElem != NULL)
			sphtmlElem->put_innerHTML(bstrText);
	}
	VARIANT GetElementProperty(LPCTSTR szElementId, DISPID dispid)
	{
		VARIANT varRet;
		CComPtr<IDispatch> spdispElem;
		varRet.vt = VT_EMPTY;
		GetElement(szElementId, &spdispElem);
		if (spdispElem)
		{
			DISPPARAMS dispparamsNoArgs = { NULL, NULL, 0, 0 };
			spdispElem->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, &dispparamsNoArgs, &varRet, NULL, NULL);
		}
		return varRet;
	}
	void SetElementProperty(LPCTSTR szElementId, DISPID dispid, VARIANT *pVar)
	{
		CComPtr<IDispatch> spdispElem;
		GetElement(szElementId, &spdispElem);
		if (spdispElem)
		{
			DISPPARAMS dispparams = {NULL, NULL, 1, 1};
			dispparams.rgvarg = pVar;
			DISPID dispidPut = DISPID_PROPERTYPUT;
			dispparams.rgdispidNamedArgs = &dispidPut;
			spdispElem->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYPUT, &dispparams, NULL, NULL, NULL);
		}
	}

	BOOL GetDocAttribute(LPCTSTR Attribute, BOOL& value)
	{
		CComVariant vt;
		BOOL bSuccess = GetDocAttribute(Attribute, vt);
		if (!bSuccess || V_VT(&vt) == VT_EMPTY || V_VT(&vt) == VT_NULL || V_VT(&vt) == VT_ERROR)
		{
			return FALSE;
		}
		if (V_VT(&vt) == VT_BSTR )
		{
			value = vt == CComVariant(L"true");
			return TRUE;
		}
		if (V_VT(&vt) == VT_BOOL || SUCCEEDED(vt.ChangeType(VT_BOOL)))
		{
			value = V_BOOL(&vt);
			return TRUE;
		}
		return FALSE;
	}
	BOOL GetDocAttribute(LPCTSTR Attribute, LONG& value)
	{
		CComVariant vt;
		BOOL bSuccess = GetDocAttribute(Attribute, vt);
		if (!bSuccess || V_VT(&vt) == VT_EMPTY || V_VT(&vt) == VT_NULL || V_VT(&vt) == VT_ERROR)
		{
			return FALSE;
		}
		if (V_VT(&vt) == VT_I4 || SUCCEEDED(vt.ChangeType(VT_I4)))
		{
			value = V_I4(&vt);
			return TRUE;
		}
		return FALSE;
	}
	BOOL GetDocAttribute(LPCTSTR Attribute, CComBSTR& value)
	{
		CComVariant vt;
		BOOL bSuccess = GetDocAttribute(Attribute, vt);
		if (!bSuccess || V_VT(&vt) == VT_EMPTY || V_VT(&vt) == VT_NULL || V_VT(&vt) == VT_ERROR)
		{
			return FALSE;
		}
		if (V_VT(&vt) == VT_BSTR || SUCCEEDED(vt.ChangeType(VT_BSTR)))
		{
			value = V_BSTR(&vt);
			return TRUE;
		}
		return FALSE;
	}
	BOOL GetDocAttribute(LPCTSTR Attribute, VARIANT& value)
	{
		CComPtr<IHTMLDocument2> spDoc;
		GetDHtmlDocument(&spDoc);
		if (spDoc == NULL)
			return FALSE;

		CComPtr<IHTMLElement> spBody;
		spDoc->get_body(&spBody);
		if (spBody == NULL)
			return FALSE;

		CComBSTR strName = Attribute;
		HRESULT hr = spBody->getAttribute(strName, 0, &value);
		return SUCCEEDED(hr);
	}

	BOOL GetDocTitle(CComBSTR& value)
	{
		CComPtr<IHTMLDocument2> spDoc;
		GetDHtmlDocument(&spDoc);
		if (spDoc == NULL)
			return FALSE;
		spDoc->get_title(&value);
		return TRUE;
	}
	
	HRESULT GetControlDispatch(LPCTSTR szId, IDispatch **ppdisp)
	{
		HRESULT hr = S_OK;
		CComPtr<IDispatch> spdispElem;
		hr = GetElement(szId, &spdispElem);
		if (spdispElem)
		{
			CComPtr<IHTMLObjectElement> sphtmlObj;
			hr = spdispElem.QueryInterface(&sphtmlObj);
			if (sphtmlObj)
			{
				spdispElem.Release();
				hr = sphtmlObj->get_object(ppdisp);
			}
		}
		return hr;
	}
	VARIANT GetControlProperty(IDispatch *pdispControl, DISPID dispid)
	{
		VARIANT varRet;
		varRet.vt = VT_EMPTY;
		if (pdispControl)
		{
			DISPPARAMS dispparamsNoArgs = { NULL, NULL, 0, 0 };
			pdispControl->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, &dispparamsNoArgs, &varRet, NULL, NULL);
		}
		return varRet;
	}
	VARIANT GetControlProperty(LPCTSTR szId, DISPID dispid)
	{
		CComPtr<IDispatch> spdispElem;
		GetControlDispatch(szId, &spdispElem);
		return GetControlProperty(spdispElem, dispid);
	}
	VARIANT GetControlProperty(LPCTSTR szId, LPCTSTR szPropName)
	{
		CComVariant varEmpty;
		CComPtr<IDispatch> spdispElem;
		GetControlDispatch(szId, &spdispElem);
		if (!spdispElem)
			return varEmpty;

		DISPID dispid;
		USES_CONVERSION;
		LPOLESTR pPropName = (LPOLESTR)T2COLE(szPropName);
		HRESULT hr = spdispElem->GetIDsOfNames(IID_NULL, &pPropName, 1, LOCALE_USER_DEFAULT, &dispid);
		if (SUCCEEDED(hr))
			return GetControlProperty(spdispElem, dispid);
		return varEmpty;
	}
	void SetControlProperty(IDispatch *pdispControl, DISPID dispid, VARIANT *pVar)
	{
		if (pdispControl != NULL)
		{
			DISPPARAMS dispparams = {NULL, NULL, 1, 1};
			dispparams.rgvarg = pVar;
			DISPID dispidPut = DISPID_PROPERTYPUT;
			dispparams.rgdispidNamedArgs = &dispidPut;
			pdispControl->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYPUT, &dispparams, NULL, NULL, NULL);
		}
	}
	void SetControlProperty(LPCTSTR szElementId, DISPID dispid, VARIANT *pVar)
	{
		CComPtr<IDispatch> spdispElem;
		GetControlDispatch(szElementId, &spdispElem);
		SetControlProperty(spdispElem, dispid, pVar);
	}
	void SetControlProperty(LPCTSTR szElementId, LPCTSTR szPropName, VARIANT *pVar)
	{
		CComPtr<IDispatch> spdispElem;
		GetControlDispatch(szElementId, &spdispElem);
		if (!spdispElem)
			return;
		DISPID dispid;
		USES_CONVERSION;
		LPOLESTR pPropName = (LPOLESTR)T2COLE(szPropName);
		HRESULT hr = spdispElem->GetIDsOfNames(IID_NULL, &pPropName, 1, LOCALE_USER_DEFAULT, &dispid);
		if (SUCCEEDED(hr))
			SetControlProperty(spdispElem, dispid, pVar);
	}
	HRESULT GetEvent(IHTMLEventObj **ppEventObj)
	{
		if (ppEventObj==NULL)
			return E_POINTER;

		CComPtr<IHTMLWindow2> sphtmlWnd;
		CComPtr<IHTMLDocument2> sphtmlDoc;
		*ppEventObj = NULL;
		HRESULT hr = GetDHtmlDocument(&sphtmlDoc);
		if (sphtmlDoc == NULL)
			return hr;

		hr = sphtmlDoc->get_parentWindow(&sphtmlWnd);
		if (FAILED(hr))
			goto Error;
		hr = sphtmlWnd->get_event(ppEventObj);

Error:
		return hr;
	}

	//Implementation
public:
	HRESULT OnDocumentReadyStateChange(IHTMLElement *phtmlElem)
	{
		phtmlElem; // unused

		CComPtr<IHTMLDocument2> sphtmlDoc;
		GetDHtmlDocument(&sphtmlDoc);
		if (sphtmlDoc)
		{
			CComBSTR bstrState;
			sphtmlDoc->get_readyState(&bstrState);
			if (bstrState)
			{
				if (bstrState==TEXT("complete"))
					ConnectDHtmlElementEvents((((DWORD_PTR)static_cast< CDHtmlSinkHandler* >(this)) - (DWORD_PTR) this));
				else if (bstrState==TEXT("loading"))
					DisconnectDHtmlElementEvents();
			}
		}
		return S_OK;
	}

protected:
	BOOL m_bAttachedControl;
	CComBSTR m_strDlgCaption;
	ATL::CAxWindow m_wndBrowser;

	static UINT WM_DESTROYMODELESS;

public:

	BEGIN_MSG_MAP(thisClass)
		//MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		//MESSAGE_HANDLER(WM_SIZE, OnSize)
		////MESSAGE_HANDLER(WM_CREATE, OnCreate)
		//MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(thisClass::WM_DESTROYMODELESS, OnDestroyModeless)
	END_MSG_MAP()

	// Accelerators handling - needs to be called from a message loop
	BOOL IsDialogMessage(LPMSG pMsg)
	{
		if((pMsg->message < WM_KEYFIRST || pMsg->message > WM_KEYLAST) &&
			(pMsg->message < WM_MOUSEFIRST || pMsg->message > WM_MOUSELAST))
			return FALSE;

		//// find a direct child of the dialog from the window that has focus
		//HWND hWndCtl = ::GetFocus();
		//if(IsChild(hWndCtl) && ::GetParent(hWndCtl) != m_hWnd)
		//{
		//	do
		//	{
		//		hWndCtl = ::GetParent(hWndCtl);
		//	}
		//	while (::GetParent(hWndCtl) != m_hWnd);
		//}
		// give controls a chance to translate this message
		if (m_wndBrowser.IsWindow() && m_wndBrowser.SendMessage(WM_FORWARDMSG, 0, (LPARAM)pMsg) == 1)
			return TRUE;

		// do the Windows default thing
		return /*TRUE;*/ baseDialog::IsDialogMessage(pMsg);
	}

	LRESULT OnDestroyModeless(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		DestroyWindow();
		return 0;
	}

	void Initialize()
	{
		SetHostFlags(DOCHOSTUIFLAG_NO3DBORDER
					| DOCHOSTUIFLAG_SCROLL_NO
					| DOCHOSTUIFLAG_DIALOG
					| DOCHOSTUIFLAG_THEME
					);
		m_bUseHtmlTitle = FALSE;
		m_bAttachedControl = FALSE;
		m_pBrowserApp = NULL;
		m_dwDHtmlEventSinkCookie = 0;
		m_AutoSize = TRUE;
		m_Height = 0;
		m_Width = 0;
	}
	void _OnBeforeNavigate2(LPDISPATCH pDisp, VARIANT FAR* URL)
	{
		if (pDisp != m_pBrowserApp)
			return;
		// ignore script navigation
		ATLASSERT(V_VT(URL) == VT_BSTR);
		CComBSTR strUrl(11, V_BSTR(URL));
		if (lstrcmpiW(strUrl, L"javascript:") == 0) // is script navigation
			return;

		m_pBrowserApp->put_Silent(VARIANT_TRUE);

		DisconnectDHtmlEvents();
		m_spHtmlDoc = NULL;
		m_strCurrentUrl.Empty();

		USES_CONVERSION;
		((T*)(this))->OnBeforeNavigate(pDisp, W2CT(V_BSTR(URL)));
	}
	void _OnNavigateComplete2(LPDISPATCH pDisp, VARIANT FAR* URL)
	{
		if (pDisp != m_pBrowserApp)
			return;
		IDispatch *pdispDoc = NULL;
		m_pBrowserApp->get_Document(&pdispDoc);
		if (!pdispDoc)
			return;

		pdispDoc->QueryInterface(IID_IHTMLDocument2, (void **) &m_spHtmlDoc);
		if (m_bUseHtmlTitle)
		{
			CComBSTR title;
			GetDocTitle(title);
			::SetWindowTextW(m_hWnd, title);
		}

		m_strCurrentUrl = V_BSTR(URL);

		ConnectDHtmlEvents(pdispDoc);
		pdispDoc->Release();

		(static_cast<T*>(this))->OnNavigateComplete(pDisp, W2CT(V_BSTR(URL)));
	}
	void _OnDocumentComplete(LPDISPATCH pDisp, VARIANT* URL)
	{
		if (pDisp != m_pBrowserApp)
			return;
		ConnectDHtmlElementEvents((((DWORD_PTR)static_cast< CDHtmlSinkHandler* >(this)) - (DWORD_PTR)this));

		CComBSTR title;
		GetDocTitle(title);
		::SetWindowTextW(m_hWnd, title);

		m_AutoSize = TRUE;
		if (!GetDocAttribute(_T("dlg_autosize"), m_AutoSize) || !m_AutoSize)
		{
			m_Height = 0;
			m_Width = 0;
			if (GetDocAttribute(_T("dlg_width"), m_Width) &&
				GetDocAttribute(_T("dlg_height"), m_Height) &&
				m_Width &&
				m_Height)
			{
				ResizeHostWindow(m_Width, m_Height);
			}
		}

		(static_cast<T*>(this))->OnDocumentComplete(pDisp, W2CT(V_BSTR(URL)));
	}

	BEGIN_EXTERNAL_METHOD_MAP(thisClass)
	END_EXTERNAL_METHOD_MAP()

	const DHtmlExternalMethodMapEntry* _GetExternalMethodMapEntry(long* p)
	{
		return GetExternalMethodMapEntry(p);
	}
	HRESULT OnExternalInvoke(DISPID dispid, DISPPARAMS* pParams)
	{
		ATLASSERT(pParams);
		long lMapNum = 0;
		const DHtmlExternalMethodMapEntry* pMap = GetExternalMethodMapEntry(&lMapNum);
		ATLASSERT(pMap);
		if (dispid>=lMapNum || dispid<0)
			return DISP_E_MEMBERNOTFOUND;

		// get 3 parameters
		UINT c = pParams->cArgs;
		switch (c)
		{
		case 0:
			(static_cast<T*>(this)->*((EXTFUNC) (pMap[dispid].pfnExternalFunc)))(NULL, NULL, NULL);
			break;
		case 1:
			(static_cast<T*>(this)->*((EXTFUNC) (pMap[dispid].pfnExternalFunc)))(&pParams->rgvarg[c-1], NULL,NULL);
			break;
		case 2:
			(static_cast<T*>(this)->*((EXTFUNC) (pMap[dispid].pfnExternalFunc)))(&pParams->rgvarg[c-1], &pParams->rgvarg[c-2], NULL);
			break;
		default: // >= 3
			(static_cast<T*>(this)->*((EXTFUNC) (pMap[dispid].pfnExternalFunc)))(&pParams->rgvarg[c-1], &pParams->rgvarg[c-2], &pParams->rgvarg[c-3]);
		}
		return S_OK;
	}

	void OnCenterWindow(VARIANT* para1=NULL, VARIANT* =NULL , VARIANT* =NULL) 
	{ 
		CenterWindow(); 
	}
	void OnResizeWindow(VARIANT* para1=NULL, VARIANT* para2=NULL , VARIANT* =NULL)
	{
		long width = 0;
		long height = 0;
		if (para1)
		{
			CComVariant vWidth(*para1);
			if (V_VT(&vWidth)==VT_I4 || SUCCEEDED(vWidth.ChangeType(VT_I4)))
				width = V_I4(&vWidth);
		}
		if (para2)
		{
			CComVariant vHeight(*para2);
			if (V_VT(&vHeight) == VT_I4 || SUCCEEDED(vHeight.ChangeType(VT_I4)))
				height = V_I4(&vHeight);
		}
		ResizeHostWindow(width, height);
	}
	void OnShowTitleBar(VARIANT* para1 = NULL, VARIANT* para2 = NULL, VARIANT* = NULL)
	{
		long show = 0;
		if (para1)
		{
			CComVariant vshow(*para1);
			if (V_VT(&vshow) == VT_I4 || SUCCEEDED(vshow.ChangeType(VT_I4)))
				show = V_I4(&vshow);
		}
		if (show)
		{
			SetWindowLong(GWL_STYLE, GetWindowLong(GWL_STYLE) | WS_CAPTION | WS_BORDER);
		}
		else
		{
			DWORD keep_style = WS_OVERLAPPED | WS_VISIBLE | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
			SetWindowLong(GWL_STYLE, GetStyle() & keep_style);

			keep_style = WS_EX_LEFT | WS_EX_LTRREADING | WS_EX_RIGHTSCROLLBAR;
			SetWindowLong(GWL_EXSTYLE, GetExStyle() & keep_style);

			//SetWindowLong(GWL_STYLE, GetWindowLong(GWL_STYLE) & ~(WS_CAPTION | WS_BORDER));
		}
		SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED);
	}
	void ResizeHostWindow(long width, long height)
	{
		RECT rect = { 0 };
		GetWindowRect(&rect);
		rect.right = rect.left + width;
		rect.bottom = rect.top + height;
		MoveWindow(&rect);
		SendMessage(WM_SIZE);
	}

	//// DDX functions
	////void DDX_DHtml_ElementText(CDataExchange* pDX, LPCTSTR szId, DISPID dispid, CString& value);
	//void DDX_DHtml_ElementText(CDataExchange* pDX, LPCTSTR szId, DISPID dispid, short& value);
	//void DDX_DHtml_ElementText(CDataExchange* pDX, LPCTSTR szId, DISPID dispid, int& value);
	//void DDX_DHtml_ElementText(CDataExchange* pDX, LPCTSTR szId, DISPID dispid, long& value);
	//void DDX_DHtml_ElementText(CDataExchange* pDX, LPCTSTR szId, DISPID dispid, DWORD& value);
	//void DDX_DHtml_ElementText(CDataExchange* pDX, LPCTSTR szId, DISPID dispid, float& value);
	//void DDX_DHtml_ElementText(CDataExchange* pDX, LPCTSTR szId, DISPID dispid, double& value);
	//void DDX_DHtml_CheckBox(CDataExchange* pDX, LPCTSTR szId, int& value);
	//void DDX_DHtml_Radio(CDataExchange* pDX, LPCTSTR szId, long& value);
	//void DDX_DHtml_SelectValue(CDataExchange* pDX, LPCTSTR szId, CString& value);
	//void DDX_DHtml_SelectString(CDataExchange* pDX, LPCTSTR szId, CString& value);
	//void DDX_DHtml_SelectIndex(CDataExchange* pDX, LPCTSTR szId, long& value);
	//void DDX_DHtml_AxControl(CDataExchange *pDX, LPCTSTR szId, DISPID dispid, VARIANT& var);
	//void DDX_DHtml_AxControl(CDataExchange *pDX, LPCTSTR szId, LPCTSTR szPropName, VARIANT& var);

	//// DDX Functions
	////void DDX_DHtml_ElementText(LPCTSTR szId, DISPID dispid, CString& value, BOOL bSave);
	//void DDX_DHtml_ElementText(LPCTSTR szId, DISPID dispid, short& value, BOOL bSave);
	//void DDX_DHtml_ElementText(LPCTSTR szId, DISPID dispid, int& value, BOOL bSave);
	//void DDX_DHtml_ElementText(LPCTSTR szId, DISPID dispid, long& value, BOOL bSave);
	//void DDX_DHtml_ElementText(LPCTSTR szId, DISPID dispid, DWORD& value, BOOL bSave);
	//void DDX_DHtml_ElementText(LPCTSTR szId, DISPID dispid, float& value, BOOL bSave);
	//void DDX_DHtml_ElementText(LPCTSTR szId, DISPID dispid, double& value, BOOL bSave);

	///////////////////////////////////////////////////////////////////////////////
	//// Data exchange for special controls
	//void DDX_DHtml_CheckBox(LPCTSTR szId, int& value, BOOL bSave);
	//void DDX_DHtml_Radio(LPCTSTR szId, long& value, BOOL bSave);
	//void DDX_DHtml_SelectValue(LPCTSTR szId, CString& value, BOOL bSave);
	//void DDX_DHtml_SelectString(LPCTSTR szId, CString& value, BOOL bSave);
	//void DDX_DHtml_SelectIndex(LPCTSTR szId, long& value, BOOL bSave);
	//void DDX_DHtml_AxControl(LPCTSTR szId, DISPID dispid, VARIANT& var, BOOL bSave);
	//void DDX_DHtml_AxControl(LPCTSTR szId, LPCTSTR szPropName, VARIANT& var, BOOL bSave);
	//void OnDDXError(LPCTSTR szId, UINT nIDPrompt, BOOL bSave);

	//void __cdecl DDX_DHtml_ElementTextWithFormat(LPCTSTR szId,
	//	DISPID dispid, LPCTSTR lpszFormat, UINT nIDPrompt, BOOL bSave, ...);
	//void DDX_DHtml_ElementTextFloatFormat(LPCTSTR szId,
	//	DISPID dispid, void* pData, double value, int nSizeGcvt, BOOL bSave);
	//BOOL _AfxSimpleScanf(LPCTSTR lpszText,
	//	LPCTSTR lpszFormat, va_list pData);
	//BOOL _AfxSimpleFloatParse(LPCTSTR lpszText, double& d);
	//long Select_FindString(IHTMLSelectElement *pSelect, BSTR bstr, BOOL fExact);
	void SetFocusToElement(LPCTSTR szId)
	{
		// check if the element is a control element
		CComPtr<IHTMLControlElement> sphtmlCtrlElem;
		HRESULT hr = GetElementInterface(szId, &sphtmlCtrlElem);
		if (sphtmlCtrlElem)
		{
			sphtmlCtrlElem->focus();
			return;
		}

		// check if the element is an anchor element
		CComPtr<IHTMLAnchorElement> sphtmlAnchorElem;
		hr = GetElementInterface(szId, &sphtmlAnchorElem);
		if (sphtmlAnchorElem)
		{
			sphtmlAnchorElem->focus();
			return;
		}

		// otherwise all we can do is scroll the element into view
		CComPtr<IHTMLElement> sphtmlElem;
		hr = GetElementInterface(szId, &sphtmlElem);
		if (sphtmlElem)
		{
			CComVariant var;
			var.vt = VT_BOOL;
			var.boolVal = ATL_VARIANT_TRUE;
			sphtmlElem->scrollIntoView(var);
		}
	}

	// event sink
	DWORD m_dwDHtmlEventSinkCookie;
	ATL::CSimpleArray<CDHtmlControlSink *> m_ControlSinks;
	ATL::CSimpleArray<CDHtmlElementEventSink *> m_SinkedElements;

	virtual const DHtmlEventMapEntry* GetDHtmlEventMap() { return NULL; }

	HRESULT ElementSink(IDispatch* pdispElement)
	{
		if (pdispElement && !IsSinkedElement(pdispElement))
		{
			HRESULT hr;
			CDHtmlElementEventSink *pSink = NULL;
			ATLTRY(pSink = new CDHtmlElementEventSink(this, pdispElement));
			if (pSink == NULL)
				return E_OUTOFMEMORY;
			hr = AtlAdvise(pdispElement, pSink, __uuidof(IDispatch), &pSink->m_dwCookie);
			if (SUCCEEDED(hr))
				m_SinkedElements.Add(pSink);
			else
				delete pSink;
#ifdef _DEBUG
			if (FAILED(hr))
				ATLTRACE(_T("Warning: Failed to connect to ConnectionPoint!\n"));
#endif
		}
		return S_OK;
	}
	HRESULT ConnectDHtmlEvents(IUnknown *punkDoc)
	{
		return ConnectToConnectionPoint(punkDoc, __uuidof(HTMLDocumentEvents), &m_dwDHtmlEventSinkCookie);
	}
	void DisconnectDHtmlEvents()
	{
		CComPtr<IHTMLDocument2> sphtmlDoc;
		GetDHtmlDocument(&sphtmlDoc);

		if (sphtmlDoc == NULL)
			return;
		DisconnectFromConnectionPoint(sphtmlDoc, __uuidof(HTMLDocumentEvents), m_dwDHtmlEventSinkCookie);
		DisconnectDHtmlElementEvents();
	}
	HRESULT ConnectDHtmlElementEvents(DWORD_PTR dwThunkOffset = 0)
	{
		HRESULT hr = S_OK;
		const DHtmlEventMapEntry* pEventMap = GetDHtmlEventMap();
		if (!pEventMap)
			return hr;

		for (int i=0; pEventMap[i].nType != DHTMLEVENTMAPENTRY_END; i++)
		{
			//if (pEventMap[i].nType!=DHTMLEVENTMAPENTRY_CONTROL)
			if (pEventMap[i].nType==DHTMLEVENTMAPENTRY_ELEMENT)
			{
				// an element name must be specified when using element events
				ATLASSERT(pEventMap[i].szName);

				// connect to the element's event sink
				CComPtr<IDispatch> spdispElement;
				GetElement(pEventMap[i].szName, &spdispElement);
				ElementSink(spdispElement);
			}
			else if (pEventMap[i].nType==DHTMLEVENTMAPENTRY_CONTROL)
			{
				// check if we already have a sink connected to this control
				if (!FindSinkForObject(pEventMap[i].szName))
				{
					// create a new sink and
					// connect it to the element's event sink
					CComPtr<IDispatch> spdispElement;
					GetElement(pEventMap[i].szName, &spdispElement);
					if (spdispElement)
					{
						CComPtr<IHTMLObjectElement> sphtmlObj;
						spdispElement->QueryInterface(__uuidof(IHTMLObjectElement), (void **) &sphtmlObj);
						if (sphtmlObj)
						{
							CComPtr<IDispatch> spdispControl;
							sphtmlObj->get_object(&spdispControl);
							if (spdispControl)
							{
								// create a new control sink to connect to the control's events
								CDHtmlControlSink *pSink = NULL; 
								ATLTRY(pSink = new CDHtmlControlSink(spdispControl, this, pEventMap[i].szName, dwThunkOffset));
								if (pSink == NULL)
									return E_OUTOFMEMORY;
								m_ControlSinks.Add(pSink);
							}
						}
					}
				}
			}
		}
		return hr;
	}
	BOOL FindSinkForObject(LPCTSTR szName)
	{	
		if (!szName)
			return TRUE;
		int nLength = m_ControlSinks.GetSize();
		for (int i=0; i<nLength; i++)
		{
			if (!_tcscmp(szName, m_ControlSinks[i]->m_szControlId))
				return TRUE;
		}
		return FALSE;
	}
	BOOL IsSinkedElement(IDispatch *pdispElem)
	{
		if (!pdispElem)
			return TRUE;
		CComPtr<IUnknown> spunk;
		pdispElem->QueryInterface(__uuidof(IUnknown), (void **) &spunk);
		if (!spunk)
			return FALSE;
		for (int i=0; i<m_SinkedElements.GetSize(); i++)
		{
			if (spunk == m_SinkedElements[i]->m_spunkElem)
				return TRUE;
		}
		return FALSE;
	}
	void DisconnectDHtmlElementEvents()
	{
		const DHtmlEventMapEntry* pEventMap = GetDHtmlEventMap();

		if (!pEventMap)
			return;

		int i;

		// disconnect from element events
		for (i=0; i<m_SinkedElements.GetSize(); i++)
		{
			CDHtmlElementEventSink *pSink = m_SinkedElements[i];
			AtlUnadvise(pSink->m_spunkElem, __uuidof(IDispatch), pSink->m_dwCookie);
			delete pSink;
		}
		m_SinkedElements.RemoveAll();

		// disconnect from control events
		for (i=0; i<m_ControlSinks.GetSize(); i++)
		{
			DisconnectFromConnectionPoint(m_ControlSinks[i]->m_spunkObj, 
				m_ControlSinks[i]->m_iid, m_ControlSinks[i]->m_dwCookie);
			delete m_ControlSinks[i];
		}
		m_ControlSinks.RemoveAll();
		return;
	}

	// host handler
	DWORD m_dwHostFlags;
};

template<class T, class TBase> UINT __declspec(selectany) CDHtmlDialogImpl<T,TBase>::WM_DESTROYMODELESS = 0;

/////////////////////////////////////////////////////////////////////////////
// Multi Url event map support
struct DHtmlUrlEventMapEntry
{
	LPCTSTR szUrl;	// the url
	const DHtmlEventMapEntry *pEventMap; // the map associated with the Url
};

#define BEGIN_URL_ENTRIES_MAP(className) \
public: \
typedef className theClass; \
const DHtmlEventMapEntry *GetEventMapForUrl(LPCTSTR szUrl)\
{\

#define BEGIN_DHTML_URL_EVENT_MAP(mapName)\
	static const DHtmlEventMapEntry __EventMap##mapName [] = \
	{\

#define END_DHTML_DHTML_EVENT_MAP()\
	{ DHTMLEVENTMAPENTRY_END, 0, NULL, NULL },\
	};\

#define BEGIN_URL_ENTRIES()\
	static const DHtmlUrlEventMapEntry UrlEntries[] = \
	{\

#define URL_EVENT_ENTRY(url, mapName)\
		{ url, __EventMap##mapName },\

#define END_URL_ENTRIES()\
		{ NULL, NULL },\
	};\

#define END_URL_ENTRIES_MAP() \
	for (int i=0; UrlEntries[i].szUrl; i++)\
	{\
		if (!_tcsicmp(UrlEntries[i].szUrl, szUrl))\
			return UrlEntries[i].pEventMap;\
	}\
	return NULL;\
}\


template<class T, class TBase=ATL::CWindow>
class CMultiPageDHtmlDialogImpl : public CDHtmlDialogImpl<T,TBase>
{
public:
	typedef CDHtmlDialogImpl<T,TBase> baseClass;

public:
	CMultiPageDHtmlDialogImpl(UINT nHtmlResID = 0) : baseClass(nHtmlResID) { m_pCurrentMap = NULL; }
	CMultiPageDHtmlDialogImpl(LPCTSTR szHtmlResID) : baseClass(szHtmlResID) { m_pCurrentMap = NULL; }

	//implementation
protected:
	virtual const DHtmlEventMapEntry *GetEventMapForUrl(LPCTSTR /*szUrl*/)
	{ return NULL; }
	virtual const DHtmlEventMapEntry* GetDHtmlEventMap()
	{ return m_pCurrentMap; }
	virtual void OnNavigateComplete(LPDISPATCH pDisp, LPCTSTR szUrl)
	{
		if (pDisp != m_pBrowserApp)
			return;
		LPTSTR p = (LPTSTR)szUrl;
		CComBSTR str(4, szUrl);
		if (szUrl && lstrcmpiW(str, L"res:")==0)
		{
			p += lstrlen(szUrl) - 1;
			while (p>szUrl && *p!=_T('/')) p--;
			if (*p == _T('/'))
				p++;
			else
				p = szUrl;
		}
		m_pCurrentMap = GetEventMapForUrl((LPCTSTR)p);
	}

	const DHtmlEventMapEntry *m_pCurrentMap;
};

}; // namespace WTL

#endif // __ATLDHTMLDLG_H__