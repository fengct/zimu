// ZimuDoc.cpp : implementation of the CZimuDoc class
//

#include "stdafx.h"
#include "Zimu.h"

#include "ZimuDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CZimuDoc

IMPLEMENT_DYNCREATE(CZimuDoc, CDocument)

BEGIN_MESSAGE_MAP(CZimuDoc, CDocument)
	//{{AFX_MSG_MAP(CZimuDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CZimuDoc construction/destruction

CZimuDoc::CZimuDoc()
{
	// TODO: add one-time construction code here

}

CZimuDoc::~CZimuDoc()
{
}

BOOL CZimuDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CZimuDoc serialization

void CZimuDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CZimuDoc diagnostics

#ifdef _DEBUG
void CZimuDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CZimuDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CZimuDoc commands
