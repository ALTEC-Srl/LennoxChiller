#if	!defined(AFX_STDAFX_H__ADOCONN__INCLUDED_)
#define AFX_STDAFX_H__ADOCONN__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <comutil.h>
#include <comdef.h>

//#define _DEF_PROVIDER "sqloldb"
#define _DEF_PROVIDER "Microsoft.JET.OLEDB.4.0"
#define _DEF_DATASOURCE    "Data Source:'Altecserver';Initial Catalog='Circuiti';trusted connection = 'yes';"
//#define _DEF_STRING    "Provider = 'sqloledb';Data Source:'Altecserver';Initial Catalog='Circuiti';Integrated Security='SSPI';"
#define _DEF_STRING    "Provider = 'sqloledb';Data Source:'Altecserver';Initial Catalog='Circuiti DBM';Trusted Connection = 'Yes';"

#include <INITGUID.H>
#include <adoid.h>
#include <adoint.h>

//#import "c:\Programmi\File Comuni\system\ado\msado15.dll" rename ("EOF","ADOEOF")
#include "..\msadox.tlh" // calcwin2005\#import "c:\Programmi\File Comuni\system\ado\msadox.dll"  //rename("Properties","ADOProperties") rename ("Property","ADOProperty")
//#include "debug\msadox.tlh" 

#include <afxtempl.h>


/*
//Matteo 31.07.2018
AGGIUNTA CLASSE HELPFUNC.H
class CVar : public VARIANT
	{
public:
	CVar()
		{
		VariantInit(this);
		}
	CVar(VARTYPE vt, SCODE scode = 0)
		{
		VariantInit(this);
		this->vt = vt;
		this->scode = scode;
		}
	CVar(VARIANT var)
		{
		*this = var;
		}
	~CVar()
		{
		VariantClear(this);
		}

	// ASSIGNMENT OPS.
	CVar & operator=(PCWSTR pcwstr)
		{
		VariantClear(this);
		if (NULL == (this->bstrVal = SysAllocStringLen(pcwstr, wcslen(pcwstr))))
			throw E_OUTOFMEMORY;
		this->vt = VT_BSTR;
		return *this;
		}
	CVar & operator=(VARIANT var)
		{
		HRESULT hr;

		VariantClear(this);
		if (FAILED(hr = VariantCopy(this, &var)))
			throw hr;
		return *this;
		}

	// CAST OPS.
	// doesn't change type. only returns BSTR if variant is of type
	// bstr. asserts otherwise.
	operator BSTR() const
	{
		if(VT_BSTR == this->vt)
			return this->bstrVal;
		else
			return NULL;
	}

	HRESULT Clear()
	{
		return VariantClear(this);
	}
};
*/

class ADOConn
{

public:

 	ADOConn();
	~ADOConn();

	_Connection* GetConnection();
	
	HRESULT QueryInterface( REFIID iid, void ** ppvObject);
	
	bool IsOpen();
	bool Open(CString db, CString pwd=_T(""), CString provider=_T(""), CString extra = _T(""),bool azure=false);

	/*_ConnectionPtr operator->()
	{
		return m_pConn;
	};*/

	void BeginTrans();
	void Rollback();
	void CommitTrans();
	ADOX::ProceduresPtr m_proc ;
	bool Execute(CString command);

protected:
	
	_Connection* m_pConn;
	
};


class ADORecSet
{
public:
	ADORecSet(ADOConn* pConn);
	ADORecSet(_Recordset*	rs);
	~ADORecSet();

	HRESULT QueryInterface( REFIID iid, void ** ppvObject);
	bool IsEOF();
	
	bool IsBOF();

	bool IsOpen();

	variant_t GetFieldValue(LPTSTR fields);
	variant_t GetFieldValue(int field);

	_Recordset* GetRS();
	ADOX::_CatalogPtr cat;

	bool OpenFromQuery(CursorTypeEnum openType, const CString& sqlCmd, CArray<COleVariant, COleVariant> *param=NULL, 
		LockTypeEnum lockType = adLockOptimistic, CommandTypeEnum options=adCmdText );

	bool OpenFromQuery(CursorTypeEnum openType, short pos , CArray<COleVariant, COleVariant> *param=NULL, 
		LockTypeEnum lockType = adLockOptimistic, CommandTypeEnum options=adCmdText );

	bool Open(CursorTypeEnum openType, const CString& sqlCmd, 
		LockTypeEnum lockType = adLockOptimistic, CommandTypeEnum options=adCmdText);

	LONG GetFieldsCount();

	Field* GetField(int col);

	WCHAR* GetFieldName(int col, CString &name );
	
	void MoveNext();
	void Move(long nrec);
	void MoveFirst();
	void MoveLast();
	void Close();
	LONG_PTR GetRecordCount();

	void SetFieldValue(CString field, VARIANT var);
	void SetFieldValue(CString field, long var);
	void SetFieldValue(CString field, short var);
	void SetFieldValue(CString field, double var);
	void SetFieldValue(CString field, WCHAR* var);
	void SetFieldValue(int field, VARIANT var);
	void SetFieldValue(int field, double var);
	void SetFieldValue(int field, long var);
	void SetFieldValue(int field, short var);
	void SetFieldValue(int field, WCHAR* var);

	void Requery();

	void AddNew();
	void Edit();
	bool Update();

	bool FindFirst(CString command);

	void ResetFilter();
	void SetBookmark(COleVariant bookmark);

	
	
protected:
	//_RecordsetPtr	m_rs;
	_Recordset*	m_rs;
	ADOConn* m_pConn;
	long m_fCount;
};
#endif