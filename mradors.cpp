#include "pch.h"
#include "mradors.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

_COM_SMARTPTR_TYPEDEF(_Command, __uuidof(_Command));
/*_COM_SMARTPTR_TYPEDEF(_Parameters, __uuidof(_Parameters));
_COM_SMARTPTR_TYPEDEF(_Parameter, __uuidof(_Parameter));

*/
#define vtNull COleVariant((long)NULL)

///////////////////////////////////////////////////////////////////////

// myDisplayErrorRecord
// Summary of Routines
//
// This function displays the error information for a single error
// record, including information from ISQLErrorInfo, if supported.
//
////////////////////////////////////////////////////////////////////////
#include "oledb.h"         // OLE DB Header
#include "oledberr.h"      // OLE DB Errors

//#include "msdasc.h"        // OLE DB Service Component header
#include "msdaguid.h"      // OLE DB Root Enumerator
#include "msdasql.h"       // MSDASQL - Default provider

#define __LONGSTRING(string) L##string
#define LONGSTRING(string) __LONGSTRING(string)

// Goes to CLEANUP on Failure_
#define CHECK_HR(hr)      \
   if(FAILED(hr))         \
      goto CLEANUP

// Goes to CLEANUP on Failure, and displays any ErrorInfo
#define XCHECK_HR(hr)                                               \
{                                                                   \
   if( g_dwFlags & DISPLAY_METHODCALLS )                            \
      fwprintf(stderr, LONGSTRING(#hr) L"\n");                      \
   if(FAILED(myHandleResult(hr, LONGSTRING(__FILE__), __LINE__)))   \
      goto CLEANUP;                                                 \
}
#define CHECK_MEMORY(hr, pv)   \
{                              \
   if(!pv)                     \
   {                           \
      hr = E_OUTOFMEMORY;      \
      CHECK_HR(hr);            \
   }                           \
}

HRESULT myGetSqlErrorInfo(ULONG iRecord, IErrorRecords* pIErrorRecords, 
                          BSTR* pBstr, LONG* plNativeError);


#define MAX_COL_SIZE         5000
#define MAX_NAME_LEN         256

#define MAX_ROWS              10
#define MAX_DISPLAY_SIZE      20
#define MIN_DISPLAY_SIZE      3


HRESULT myDisplayErrorRecord
   (
   HRESULT           hrReturned,
   ULONG             iRecord,
   IErrorRecords *   pIErrorRecords,
   LPCTSTR           pwszFile,
   ULONG             ulLine
   )
{
   HRESULT           hr;
   IErrorInfo *      pIErrorInfo       = NULL;
   BSTR              bstrDescription   = NULL;
   BSTR              bstrSource        = NULL;
   BSTR              bstrSQLInfo       = NULL;

   static LCID       lcid              = GetUserDefaultLCID();

   LONG              lNativeError      = 0;
   ERRORINFO         ErrorInfo;
   CString p;

   // Get the IErrorInfo interface pointer for this error record.
   CHECK_HR(hr = pIErrorRecords->GetErrorInfo(iRecord, lcid, &pIErrorInfo));
   
   // Get the description of this error.
   CHECK_HR(hr = pIErrorInfo->GetDescription(&bstrDescription));
      
   // Get the source of this error.
   CHECK_HR(hr = pIErrorInfo->GetSource(&bstrSource));

   // Get the basic error information for this record.
   CHECK_HR(hr = pIErrorRecords->GetBasicErrorInfo(iRecord, &ErrorInfo));

   // If the error object supports ISQLErrorInfo, get this information.
   myGetSqlErrorInfo(iRecord, pIErrorRecords, &bstrSQLInfo, &lNativeError);

   // Display the error information to the user.
   if( bstrSQLInfo )
   {
      p.Format(_T("\nErrorRecord:  HResult: 0x%08x\nDescription: %s\n")
         _T("SQLErrorInfo: %s\nSource: %s\nFile: %s, Line: %d\n"),
         ErrorInfo.hrError,
         bstrDescription,
         bstrSQLInfo,
         bstrSource,
         pwszFile,
         ulLine);

   }
   else
   {
      p.Format(_T("\nErrorRecord:  HResult: 0x%08x\nDescription: %s\n")
         _T("Source: %s\nFile: %s, Line: %d\n"),
         ErrorInfo.hrError,
         bstrDescription,
         bstrSource,
         pwszFile,
         ulLine);
   }
   ::MessageBox(NULL, p,NULL,0);

CLEANUP:
   if( pIErrorInfo )
      pIErrorInfo->Release();
   SysFreeString(bstrDescription);
   SysFreeString(bstrSource);
   SysFreeString(bstrSQLInfo);
   return hr;
}


////////////////////////////////////////////////////////////////////////

// myDisplayErrorInfo
// Summary of Routines
//
// This function displays basic error information for an error object
// that doesn't support the IErrorRecords interface.
//
////////////////////////////////////////////////////////////////////////
HRESULT myDisplayErrorInfo
   (
   HRESULT        hrReturned,
   IErrorInfo *   pIErrorInfo,
   LPCTSTR        pwszFile,
   ULONG          ulLine
   )
{
   HRESULT        hr;
   BSTR           bstrDescription   = NULL;
   BSTR           bstrSource        = NULL;
	CString p;
   // Get the description of the error.
   CHECK_HR(hr = pIErrorInfo->GetDescription(&bstrDescription));
      
   // Get the source of the error -- this will be the window title.
   CHECK_HR(hr = pIErrorInfo->GetSource(&bstrSource));

   // Display this error information.
   
   p.Format(_T("\nErrorInfo:  HResult: 0x%08x, Description: %s\nSource:%s\n File: %s, Line: %d\n"),
            hrReturned,
            bstrDescription,
            bstrSource,
            pwszFile,
            ulLine);
   //MessageBox::Show(p);

CLEANUP:
   SysFreeString(bstrDescription);
   SysFreeString(bstrSource);
   return hr;
}


////////////////////////////////////////////////////////////////////////

// myGetSqlErrorInfo
// Summary of Routines
//
// If the error object supports ISQLErrorInfo, get the SQL error
// string and native error code for this error.
//
////////////////////////////////////////////////////////////////////////
HRESULT myGetSqlErrorInfo
   (
   ULONG             iRecord,
   IErrorRecords *   pIErrorRecords,
   BSTR *            pBstr,
   LONG *            plNativeError
   )
{
   HRESULT           hr;
   ISQLErrorInfo *   pISQLErrorInfo   = NULL;
   LONG              lNativeError     = 0;

   // Attempt to get the ISQLErrorInfo interface for this error
   // record through GetCustomErrorObject. Note that ISQLErrorInfo
   // is not mandatory, so failure is acceptable here.
   // See ISQLErrorInfo
   CHECK_HR(hr = pIErrorRecords->GetCustomErrorObject(
            iRecord,                               // iRecord
            IID_ISQLErrorInfo,                     // riid
            (IUnknown**)&pISQLErrorInfo            // ppISQLErrorInfo
            ));

   // If we obtained the ISQLErrorInfo interface, get the SQL
   // error string and native error code for this error.
   if( pISQLErrorInfo )
      hr = pISQLErrorInfo->GetSQLInfo(pBstr, &lNativeError);

CLEANUP:
   if( plNativeError )
      *plNativeError = lNativeError;
   if( pISQLErrorInfo )
      pISQLErrorInfo->Release();
   return hr;
}

void PopupErrorMessage(HRESULT hr1)
{
	
	HRESULT           hr;
   IErrorInfo *      pIErrorInfo      = NULL;
   IErrorRecords *   pIErrorRecords   = NULL;
   ULONG             cRecords;
   ULONG             iErr;
#ifdef _UNICODE
   LPCWSTR pwszFile = LONGSTRING(__FILE__); 
#else
   LPCTSTR pwszFile = __FILE__; 
#endif
   ULONG   ulLine = __LINE__;
   // If the method called as part of the XCHECK_HR macro failed,
   // we will attempt to get extended error information for the call.
   if( FAILED(hr1) )
   {
      // Obtain the current error object, if any, by using the
      // Automation GetErrorInfo function, which will give
      // us back an IErrorInfo interface pointer if successful.
      hr = GetErrorInfo(0, &pIErrorInfo);

      // We've got the IErrorInfo interface pointer on the error object.
      if( SUCCEEDED(hr) && pIErrorInfo )
      {
         // OLE DB extends the Automation error model by allowing
         // error objects to support the IErrorRecords interface. This
         // interface can expose information on multiple errors.
         // See IErrorRecords
         hr = pIErrorInfo->QueryInterface(IID_IErrorRecords,
                  (void**)&pIErrorRecords);
         if( SUCCEEDED(hr) )
         {
            // Get the count of error records from the object.
            CHECK_HR(hr = pIErrorRecords->GetRecordCount(&cRecords));
            
            // Loop through the set of error records, and
            // display the error information for each one.
            for( iErr = 0; iErr < cRecords; iErr++ )
            {
               myDisplayErrorRecord(hr1, iErr, pIErrorRecords,
                  pwszFile, ulLine);
            }
         }
         // The object didn't support IErrorRecords; display
         // the error information for this single error.
         else
         {
            myDisplayErrorInfo(hr1, pIErrorInfo, pwszFile, ulLine);
         }
      }
      // There was no error object, so just display the HRESULT
      // to the user.
      else
      {
         TRACE(L"\nNo Error Info posted; HResult: 0x%08x\n"
            L"File: %s, Line: %d\n", hr1, pwszFile, ulLine);
      }
   }

CLEANUP:
   if( pIErrorInfo )
      pIErrorInfo->Release();
   if( pIErrorRecords )
      pIErrorRecords->Release();

}



ADOConn::ADOConn()
{
	//CoCreateInstance(L"ADODB.Connection", NULL, CLSCTX_INPROC_SERVER, IID_IADOConnection, (LPVOID *)&m_pConn);
	CoInitialize(NULL);
	CoCreateInstance(CLSID_CADOConnection, NULL, CLSCTX_INPROC_SERVER, IID_IADOConnection, (LPVOID *)&m_pConn);
	
	//m_pConn.CreateInstance(__uuidof(Connection));
};



ADOConn::~ADOConn()
{
	m_pConn->Close();
	//m_pConn->Release();
	m_pConn = NULL;
};

_Connection* ADOConn::GetConnection()
{
	return m_pConn;
};

HRESULT ADOConn::QueryInterface( REFIID iid, void ** ppvObject)
{
	return m_pConn->QueryInterface(iid, ppvObject);
};

bool ADOConn::IsOpen()
{
	LONG state;
	m_pConn->get_State(&state);
	return state == adStateOpen;
};

bool ADOConn::Open(CString db, CString pwd, CString provider, CString extra, bool azure)
{
	if (provider.IsEmpty())
	{
		//CString str = "Data Source=" + path + "calclang.mdb;Provider=Microsoft.JET.OLEDB.3.51; Jet OLEDB:Database Password=rossomch";
		//provider = "Microsoft.JET.OLEDB.4.0";
		provider = _DEF_PROVIDER;
		//pConnection1->ConnectionString = 
		//"Provider='sqloledb';Data Source='MySqlServer';"
		//"Initial Catalog='Pubs';Integrated Security='SSPI';";
	}

	CString conn;
	if (db.Find(_T(".udl")) != -1)// IsEmpty() && !extra.IsEmpty())
		conn.Format(_T("File Name=%s;"),  db);
	else
	{
		conn.Format(_T("Data Source = %s;Provider=%s;"), db, provider);
		if (!extra.IsEmpty())
			conn += extra;
	}
	
	if (!pwd.IsEmpty() && provider == _T("Microsoft.JET.OLEDB.4.0") )
	{
		conn += _T(";Jet OLEDB:Database Password=") + pwd;
	}
	ASSERT(m_pConn);
	try
	{
		_bstr_t strB = conn;
		//CVar varUserId, varPwd;
		//m_pConn->ConnectionString = strB;
		_bstr_t varUserId, varPwd;
		HRESULT hr = m_pConn->Open(strB, varUserId, varPwd, adOpenUnspecified ) ;
//#if defined(_DEBUG)
		if (!SUCCEEDED(hr))
		{
			
			PopupErrorMessage(hr);
			return false;

		}
//#endif
		ADOX::_CatalogPtr cat;
		cat.CreateInstance(__uuidof(ADOX::Catalog));
		cat->PutActiveConnection(m_pConn);
		m_proc = cat->GetProcedures();
		if (!azure)
			m_proc->Refresh();
		return true;
	}
	catch( _com_error &e)
	{
		_bstr_t bstrSource(e.Source());
		_bstr_t bs =  _bstr_t(_T(" Error: ")) + _bstr_t(e.Error()) + _bstr_t(_T(" Msg: ")) 
			+ _bstr_t(e.ErrorMessage()) + _bstr_t(_T(" Description: ")) 
			+ _bstr_t(e.Description());

		::MessageBox(0,bs,bstrSource, MB_OK);
		return false;

	} 
};


void ADOConn::BeginTrans()
{
	LONG transLevel =0;
	m_pConn->BeginTrans(&transLevel);
};
void ADOConn::Rollback()
{
	m_pConn->RollbackTrans();
};
void ADOConn::CommitTrans()
{
	m_pConn->CommitTrans();
};

bool ADOConn::Execute(CString command)
{
	_bstr_t commB = command;
	try
	{
		HRESULT hr = m_pConn->Execute(commB,NULL,adCmdText,NULL);
#ifdef _DEBUG
		if (hr != S_OK)
		{
			PopupErrorMessage(hr);
		}
#endif

	}
	catch( _com_error &e)
	{
		_bstr_t bstrSource(e.Source());
		_bstr_t bs =  _bstr_t(_T(" Error: ")) + _bstr_t(e.Error()) + _bstr_t(_T(" Msg: ")) 
			+ _bstr_t(e.ErrorMessage()) + _bstr_t(_T(" Description: ")) 
			+ _bstr_t(e.Description());

		::MessageBox(0,bs,bstrSource, MB_OK);
		return false;

	}
	return true;
};

ADORecSet::ADORecSet(ADOConn* pConn)
{

	try
	{
		//m_rs.CreateInstance(__uuidof(Recordset));
		//m_rs->CreateInstance(L"ADODB.Recordset");
		CoCreateInstance(CLSID_CADORecordset, NULL, CLSCTX_INPROC_SERVER, IID_IADORecordset, (LPVOID *)&m_rs);
		m_rs->putref_ActiveConnection(pConn->GetConnection());
		cat.CreateInstance(__uuidof(ADOX::Catalog));
		COleVariant conn;
		m_rs->get_ActiveConnection(&conn);
		cat->PutActiveConnection(conn);
		m_pConn = pConn;
		//pConn->GetConnection()->Release();
	}

	catch( _com_error &e)
	{
		_bstr_t bstrSource(e.Source());
		_bstr_t bs =  _bstr_t(_T(" Error: ")) + _bstr_t(e.Error()) + _bstr_t(_T(" Msg: ")) 
			+ _bstr_t(e.ErrorMessage()) + _bstr_t(_T(" Description: ")) 
			+ _bstr_t(e.Description());

		::MessageBox(0,bs,bstrSource, MB_OK);

	}

};
ADORecSet::ADORecSet(_Recordset*	rs)
{
	if (rs == NULL)
	{
		CoCreateInstance(CLSID_CADORecordset, NULL, CLSCTX_INPROC_SERVER, IID_IADORecordset, (LPVOID *)&m_rs);
	}
	m_rs = rs;
	m_fCount = GetFieldsCount();
};
ADORecSet::~ADORecSet()
{
	if (IsOpen())
	{
		m_rs->Close();
	}
}

HRESULT ADORecSet::QueryInterface( REFIID iid, void ** ppvObject)
{
	return m_rs->QueryInterface(iid, ppvObject);
};


bool ADORecSet::IsEOF()
{

	VARIANT_BOOL eof;
	m_rs->get_EOF(&eof);
	return eof == FALSE ? false: true;
}
	
bool ADORecSet::IsBOF()
{
	VARIANT_BOOL bof;
	m_rs->get_BOF(&bof);
	return bof == FALSE ? false: true;
}

bool ADORecSet::IsOpen()
{
	if (m_rs == NULL)
		return false;
	LONG state;

	m_rs->get_State(&state);
	return state == adStateOpen;
};

variant_t ADORecSet::GetFieldValue(LPTSTR fields)
{
	ASSERT(IsOpen());
	//VARIANT result;
	//VARIANT index;
	//index.bstrVal = _bstr_t(fields); //CString(fields).AllocSysString();
	//index.vt = VT_BSTR;

	Field	*pField = NULL;
	Fields	*pFields = NULL;

	_variant_t findex ;
	findex = _bstr_t(fields);

	m_rs->get_Fields(&pFields);
	HRESULT hr = pFields->get_Item(findex, &pField );
	if (hr != S_OK)
	{
		BSTR db;
		m_pConn->GetConnection()->get_ConnectionString(&db);
		CString f;
		f.Format(_T("Field %s not found, incorrect database version.\n\n"),
			fields);
//		AfxMessageBox(f);
		return vtNull;
	
	}
	_variant_t val ;
	pField->get_Value(&val);
	if (val.vt == VT_NULL)
	{
		val.iVal = 0;
		val.lVal = 0;
		val.dblVal = 0;
		val.bstrVal = 0;
	}

	//m_rs->get_Collect( index, &result);
	
	return val; //m_rs->get_CollectFields->Item[field]->Value;
};

variant_t ADORecSet::GetFieldValue(int field)
{
	ASSERT(IsOpen());
	if (m_fCount <= field)
	{
		BSTR db;
		try
		{
		m_pConn->GetConnection()->get_ConnectionString(&db);
		CString f;
		f.Format(_T("Field %d not found, incorrect database version.\n\n"),field);
		//AfxMessageBox(f);
		}
		catch (...)
		{
			CString f;
			f.Format(_T("Field not found, incorrect database version."));
		//	AfxMessageBox(f);
		}
		return variant_t();
	}
	_variant_t findex ;
	findex = (short) field;
	
	Field	*pField = NULL;
	Fields	*pFields = NULL;
	m_rs->get_Fields(&pFields);
	HRESULT hr = pFields->get_Item(findex, &pField );
	_variant_t val ;
	hr = pField->get_Value(&val);
	if (val.vt == VT_NULL)
	{
		val.iVal = 0;
		val.lVal = 0;
		val.dblVal = 0;
		val.bstrVal = 0;
	}
	pField->Release();
	pFields->Release();
	//m_rs->get_Collect( index, &result);
	
	//return m_rs->Fields->Item[vtIndex]->Value;
	return val;
};

ADORecordset* ADORecSet::GetRS()
{
	return m_rs;
};

bool ADORecSet::OpenFromQuery(CursorTypeEnum openType, const CString& sqlCmd, CArray<COleVariant, COleVariant> *param, 
							  LockTypeEnum lockType , CommandTypeEnum options )
{

	// Query senza parametri
	DWORD t = GetTickCount();
	m_fCount = 0;
	if (!param)
	{
		bool ris = Open(openType, sqlCmd, lockType, adCmdStoredProc);
		m_fCount = GetFieldsCount();
		return ris;
	}
	// Query con parametri, ho bisogno di ADOX
	//try
	{
		/*_CommandPtr  spCMD;
		CoCreateInstance(CLSID_CADOCommand, NULL, CLSCTX_INPROC_SERVER, IID_IADOCommand, (LPVOID *)&spCMD);
		COleVariant conn;
		m_rs->get_ActiveConnection(&conn);
		spCMD->put_ActiveConnection(conn);
        spCMD->put_CommandText(_bstr_t(sqlCmd)) ;
        spCMD->put_CommandType(adCmdStoredProc);
		
		ADOParameters* par=NULL;
		spCMD->get_Parameters(&par);
		par->Refresh();
		_ADOParameter* pa=NULL;
		HRESULT hr;
		for (int i = 0; i < param->GetSize(); i++)
		{
			//findex.iVal = i;
			//pars->get_Item(findex,&par);
			COleVariant value = param->GetAt(i);
			//hr = par->put_Value(value);
			spCMD->CreateParameter(L"@IDCALC", adInteger,adParamInput,sizeof(adInteger), value,&pa);
			par->Append(pa);
			//if (!SUCCEEDED(hr))
			//	PopupErrorMessage(hr);

		}
		hr = m_rs->Open(_variant_t((IDispatch *)spCMD), vtMissing, openType,lockType, adCmdStoredProc);
		if (!SUCCEEDED(hr))
			PopupErrorMessage(hr);
		
		//spRS->Open(vtMissing,vtMissing,adOpenStatic,adLockBatchOptimistic,-1);
*/

		/*ADOX::_CatalogPtr cat=NULL;
		
		cat.CreateInstance(__uuidof(ADOX::Catalog));
		COleVariant conn;
		m_rs->get_ActiveConnection(&conn);
		cat->PutActiveConnection(conn);
		
		_bstr_t comB = sqlCmd;*/
#ifdef _DEBUG
		/*{
			
			ADOX::ProcedurePtr proc;
			for (int i = 0; i < cat->GetProcedures()->GetCount(); i++)
			{
				_variant_t findex ;
				findex.vt = VT_I2;
				findex.iVal= i;
				proc = cat->GetProcedures()->GetItem(findex);
				CString cazzo = (LPSTR) _bstr_t(proc->GetName());
				TRACE("%s\n ", (LPSTR) _bstr_t(proc->GetName()));
			}
		}*/
#endif

		//ADOX::ProcedurePtr proc2 = cat->GetProcedures()->GetItem(_variant_t(short(0)));
 		//ADOX::ProcedurePtr proc1 = cat->GetProcedures()->GetItem(comB);
		
		//_CommandPtr com = cat->GetProcedures()->GetItem(comB)->GetCommand();;
		/*ADOCommand* com2;
		CoCreateInstance(CLSID_CADOCommand, NULL, CLSCTX_INPROC_SERVER, IID_IADOCommand, (LPVOID *)&com2);
		com2->put_ActiveConnection(conn);

		com2->put_CommandText(comB);
		com2->put_CommandType(adCmdStoredProc);*/
		
		HRESULT hr;
		_bstr_t sParam;

		for (int i = 0; i < param->GetSize(); i++)
		{
			sParam += _bstr_t(param->GetAt(i));
			if (i < param->GetSize()-1)
				sParam+= ",";
		}
		_bstr_t comB = CString(_T("[")) + sqlCmd + _T("]");
		comB+= + L" (" + sParam + L")";
		Close();
		hr = m_rs->Open(COleVariant(comB.Detach()), vtMissing, openType,lockType, adCmdStoredProc);

		if (!SUCCEEDED(hr))
		{	
			// provo in altro modo, funziona per DB di ACCESS
			// PopupErrorMessage(hr);
			/*COleVariant conn;
			
			cat.CreateInstance(__uuidof(ADOX::Catalog));
			m_rs->get_ActiveConnection(&conn);
			cat->PutActiveConnection(conn);
			((IDispatch*) conn.pdispVal)->Release();*/
			
 			_CommandPtr com = m_pConn->m_proc->GetItem(_bstr_t(sqlCmd))->GetCommand();

			_Parameters* pars=NULL;
			ADOParameter* par=NULL;
			_variant_t findex ;
			findex.vt = VT_I2;
			com->get_Parameters(&pars);
			for (int i = 0; i < param->GetSize(); i++)
			{
				findex.iVal = i;
				pars->get_Item(findex,&par);
				COleVariant value = param->GetAt(i);
				hr = par->put_Value(value);
				par->Release();
				if (!SUCCEEDED(hr))
					PopupErrorMessage(hr);
			}
			
			hr = m_rs->Open(_variant_t((IDispatch *) com), vtMissing,openType,lockType, adCmdStoredProc);
			pars->Release();
#ifdef _DEBUG
			if (!SUCCEEDED(hr))
					PopupErrorMessage(hr);
#endif

		}
		
		/*m_rs->putref_Source(_variant_t((IDispatch *)com2));
		m_rs->put_CursorLocation(adUseClient);
		m_rs->Open(vtMissing,vtMissing,adOpenStatic,adLockBatchOptimistic,-1);*/
		
		MoveFirst();
	}
	/*catch( _com_error &e)
	{
		_bstr_t bstrSource(e.Source());
		_bstr_t bs =  _bstr_t(_T(" Error: ")) + _bstr_t(e.Error()) + _bstr_t(_T(" Msg: ")) 
			+ _bstr_t(e.ErrorMessage()) + _bstr_t(_T(" Description: ")) 
			+ _bstr_t(e.Description())+ _bstr_t("\nQuery:")+_bstr_t(sqlCmd);

		::MessageBox(0,bs,bstrSource, MB_OK);
		return false;

	}*/			
	TRACE(_T("tempo open query %s %d\n"), sqlCmd, GetTickCount()-t);;
	m_fCount = GetFieldsCount();
	return true;
};


bool ADORecSet::OpenFromQuery(CursorTypeEnum openType, short pos, CArray<COleVariant, COleVariant> *param, 
							  LockTypeEnum lockType , CommandTypeEnum options )
{

	// Query senza parametri
	m_fCount = 0;
	if (!param)
	{
		ASSERT(FALSE);
		return false;
		//return Open(openType, sqlCmd, lockType, adCmdStoredProc);
	}
	// Query con parametri, ho bisogno di ADOX
	try
	{
		_CommandPtr com = m_pConn->m_proc->GetItem(_variant_t(pos))->GetCommand();
		_Parameters* pars=NULL;
		ADOParameter* par=NULL;
		_variant_t findex ;
		findex.vt = VT_I2;
		com->get_Parameters(&pars);
		HRESULT hr;
		for (int i = 0; i < param->GetSize(); i++)
		{
			findex.iVal = i;
			pars->get_Item(findex,&par);
			COleVariant value = param->GetAt(i);
			hr = par->put_Value(value);
			if (!SUCCEEDED(hr))
				PopupErrorMessage(hr);
		}

		hr = m_rs->Open(_variant_t((IDispatch *)com), vtMissing, openType,lockType, adCmdStoredProc);
		MoveFirst();
	}
	catch( _com_error &e)
	{
		_bstr_t bstrSource(e.Source());
		_bstr_t bs =  _bstr_t(_T(" Error: ")) + _bstr_t(e.Error()) + _bstr_t(_T(" Msg: ")) 
			+ _bstr_t(e.ErrorMessage()) + _bstr_t(_T(" Description: ")) 
			+ _bstr_t(e.Description());

		::MessageBox(0,bs,bstrSource, MB_OK);
		return false;

	}			
	m_fCount = GetFieldsCount();
	return true;
};

bool ADORecSet::Open(CursorTypeEnum openType, const CString& sqlCmd, 
					 LockTypeEnum lockType , CommandTypeEnum options)
{
	//ASSERT(m_rs->GetActiveConnection());
	m_fCount = 0;
	HRESULT hr;
	try
	{
		Close();
		_bstr_t comB = sqlCmd;
		m_rs->put_Source(comB);
		hr = m_rs->Open(vtMissing, vtMissing, openType, lockType, options);
#ifdef _DEBUG
		if (hr != S_OK)
		{
			PopupErrorMessage(hr);
			BSTR db;
			m_pConn->GetConnection()->get_ConnectionString(&db);
			CString f;
			f.Format(_T("Unable to open recordset, incorrect database version.\n\n"));
			AfxMessageBox(f);
		}
		
#endif
		
		MoveFirst();
			
	}
	catch( _com_error &e)
	{
		_bstr_t bstrSource(e.Source());
		_bstr_t bs =  _bstr_t(_T(" Error: ")) + _bstr_t(e.Error()) + _bstr_t(_T(" Msg: ")) 
			+ _bstr_t(e.ErrorMessage()) + _bstr_t(_T(" Description: ")) 
			+ _bstr_t(e.Description());

		::MessageBox(0,bs,bstrSource, MB_OK);
		return false;

	}
	m_fCount = GetFieldsCount();
	return hr == S_OK;

};

LONG ADORecSet::GetFieldsCount()
{

	Fields	*pFields = NULL;
	m_rs->get_Fields(&pFields);
	LONG count;
	pFields->get_Count(&count);
	return count;
};

Field* ADORecSet::GetField(int col)
{

	Field	*pField = NULL;
	Fields	*pFields = NULL;

	_variant_t findex ;
	findex.vt = VT_I2;
	findex.iVal = col;

	m_rs->get_Fields(&pFields);
	pFields->get_Item(findex, &pField );

	return pField;

};

WCHAR* ADORecSet::GetFieldName(int col, CString &name )
{

	Field	*pField = GetField(col);
	BSTR	str;
	pField->get_Name(&str);
#ifdef _UNICODE
	name = (LPWSTR) (_bstr_t) str;
	return name.GetBuffer(0);
#else
	name = (LPTSTR) (_bstr_t) str;
	return (LPWSTR) name.GetBuffer(0);
#endif

};

void ADORecSet::MoveNext()
{
	if (!IsEOF())
		m_rs->MoveNext();
};
void ADORecSet::Move(long nrec)
{
	if (!IsEOF() && !IsBOF())
		m_rs->Move(nrec, vtNull);
};
void ADORecSet::MoveFirst()
{
	HRESULT hr;
	if (!IsBOF())
	{
		hr = m_rs->MoveFirst();
	}
};
void ADORecSet::MoveLast()
{
	m_rs->MoveLast();
};
void ADORecSet::Close()
{
	HRESULT hr ;

	if (IsOpen())
	{
		m_rs->put_Filter(vtNull);
		hr = m_rs->Close();
#ifdef _DEBUG
		if (!SUCCEEDED(hr))
		{
			
			PopupErrorMessage(hr);
			ASSERT(FALSE);
		}
#endif
	}
	
}
LONG_PTR ADORecSet::GetRecordCount()
{
	ASSERT(IsOpen());
	LONG_PTR count;
	m_rs->get_RecordCount(&count);
	return count;
};

void ADORecSet::SetFieldValue(CString field, VARIANT var)
{

	ADOField	*pField = NULL;
	ADOFields	*pFields = NULL;
	_variant_t fname = field;
	m_rs->get_Fields(&pFields);
	pFields->get_Item(fname, &pField);
	if (pField == NULL)
	{
		ASSERT(FALSE);
		BSTR db;
		m_pConn->GetConnection()->get_ConnectionString(&db);
		CString f;
		f.Format(_T("Field %s not found, incorrect database version.\n\n"),	field);
		AfxMessageBox(f);
		return ;
	}
	pField->put_Value(var);

};

void ADORecSet::SetFieldValue(CString field, long var)
{

	COleVariant var1(var);
	SetFieldValue(field, var1);
	

};

void ADORecSet::SetFieldValue(CString field, short var)
{
	COleVariant var1(var);
	SetFieldValue(field, var1);

};
void ADORecSet::SetFieldValue(CString field, double var)
{

	COleVariant var1(var);
	SetFieldValue(field, var1);
};

void ADORecSet::SetFieldValue(CString field, WCHAR* var)
{
#ifdef _UNICODE
	COleVariant var1(var);
	SetFieldValue(field, var1);
#else
	// VEDREMO POI
	ASSERT(FALSE);
#endif
};

void ADORecSet::SetFieldValue(int field, VARIANT var)
{
	ADOField	*pField = NULL;
	ADOFields	*pFields = NULL;
	m_rs->get_Fields(&pFields);
	_variant_t fname = (short)field;
	m_rs->get_Fields(&pFields);
	pFields->get_Item(fname, &pField);
	if (pField == NULL)
	{
		ASSERT(FALSE);
		BSTR db;
		m_pConn->GetConnection()->get_ConnectionString(&db);
		CString f;
		f.Format(_T("Field %s not found, incorrect database version.\n\n"),field);
		AfxMessageBox(f);
		return ;
	}
	pField->put_Value(var);
};

void ADORecSet::SetFieldValue(int field, double var)
{
	COleVariant var1(var);
	SetFieldValue(field, var1);
};

void ADORecSet::SetFieldValue(int field, long var)
{
	COleVariant var1(var);
	SetFieldValue(field, var1);
};

void ADORecSet::SetFieldValue(int field, short var)
{
	COleVariant var1(var);
	SetFieldValue(field, var1);
};

void ADORecSet::SetFieldValue(int field, WCHAR* var)
{
#ifdef _UNICODE
	COleVariant var1(var);
	SetFieldValue(field, var1);
#else
	ASSERT(FALSE);
#endif

	//m_rs->Fields->Item[vtIndex]->Value = string;
};

void ADORecSet::Requery()
{
	m_rs->Requery(adCmdUnspecified);
};

void ADORecSet::AddNew()
{
#if defined(_DEBUG)
	VARIANT_BOOL sup;
	m_rs->Supports(adAddNew, &sup);
	ASSERT(sup);
#endif
	HRESULT hr = m_rs->AddNew(vtMissing, vtMissing);
};


void ADORecSet::Edit()
{

};

bool ADORecSet::Update()
{
	
	HRESULT Hr = m_rs->Update(vtMissing, vtMissing);
#ifdef _DEBUG
		if (!SUCCEEDED(Hr))
		{
			PopupErrorMessage(Hr);
		}
#endif
	return Hr == S_OK;
};

bool ADORecSet::FindFirst(CString command)
{
	_bstr_t comB = command;
	ASSERT(IsOpen());
	HRESULT hr;
	m_rs->put_Filter(vtNull);

	MoveFirst();
	if (command.Find(_T("AND")) != -1 || command.Find(_T("OR")) != -1)
	{
		hr = m_rs->put_Filter(COleVariant(command));
		if (hr != S_OK)
		{
			PopupErrorMessage(hr);
		}
	}
	else
	{
		hr = m_rs->Find (comB,0,adSearchForward,vtNull);
#ifdef _DEBUG
		if (!SUCCEEDED(hr))
		{
			PopupErrorMessage(hr);
		}
#endif
	}
	bool ris = !IsEOF();
	
	return ris;


}

void ADORecSet::ResetFilter()
{
	CString command=_T("");
	ASSERT(IsOpen());
	VARIANT filter;
	filter.vt = VT_BSTR;;
	filter.bstrVal = command.AllocSysString();
	m_rs->put_Filter(filter);

}

void ADORecSet::SetBookmark(COleVariant bookmark)
{
	m_rs->put_Bookmark(bookmark);
}

/*
void _cdecl OLEDBErrorMessageBox(LPCTSTR lpszFormat, ...)
{
	va_list args;
	va_start(args, lpszFormat);

	int nBuf;
	TCHAR szBuffer[2048];

	nBuf = _vsntprintf_s(szBuffer, sizeof(szBuffer), lpszFormat, args);
	ATLASSERT(nBuf < sizeof(szBuffer)); //Output truncated as it was > sizeof(szBuffer)

	::MessageBox( NULL, szBuffer, _T("OLE DB Error Message"), MB_OK );
	va_end(args);
}
*/
