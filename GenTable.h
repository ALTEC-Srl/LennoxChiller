#pragma once
#include "rapidjson/document.h"     // rapidjson's DOM-style API
#include <string>
#include <map>
#include <vector>
#include <msclr\marshal_cppstd.h>

using namespace msclr::interop;
using namespace rapidjson;

typedef std::vector<std::string> CGenRecordList;
class CGenTableRecord
{
public:
	CGenTableRecord() {};
	CGenTableRecord(const char* jsonRecord)
	{
		resultDoc.Parse(jsonRecord);
	}
	void Swap(Document& doc)
	{
		doc.Swap(resultDoc);
	}
	void operator=(Document& doc)
	{
		resultDoc.Swap(doc);
	}
	bool	GetColumn(short columnIndex, double& doubleval) const noexcept;
	bool	GetColumn(short columnIndex, short& doubleval) const noexcept;
	bool	GetColumn(const char* fieldName, double& doubleval) const noexcept;
	bool	GetColumn(const char* fieldName, long& doubleval) const noexcept;
	bool	GetColumn(const char* fieldName, int& doubleval) const noexcept;
	bool	GetColumn(const char* fieldName, short& doubleval) const noexcept;
	bool	GetColumn(const char* fieldName, std::string& stringVal) const noexcept;
	bool	GetColumn(const char* fieldName, CString& stringVal) const noexcept;
	bool	IsValid() const noexcept { return resultDoc.IsObject(); };
	const	Value& GetColumn(short columnIndex) const noexcept;
	const	Value& GetColumn(const char* fieldName) const noexcept;
	const	Document& GetDocument() { return resultDoc; };
private:
	Document resultDoc;
};

const class CGenTable
{
public:

	CGenTable() {};

	bool		 LoadFromDB(const CSession& session, const wchar_t* sqlExpression, bool checkExist = true);
	//virtual bool			LoadFromDB(const CDBCmdBase& m_dbMacchineSession) { return false; };
	//virtual bool			LoadFromExcel(libxl::Book* book) { return false; };
	const wchar_t* Lookup(long idtable, const char* tagName) const noexcept; // retistuisce una stringa col valore del campo <tagname> per il record che ha chiave idtable
	CGenTableRecord	Lookup(const std::string& filter, std::vector<std::string>* pList = nullptr) const noexcept;// cerca i campi specificati nel filtro, assegna il valore del record trovato a result
	const CGenTableRecord& Lookup1(const std::string& filter, std::vector<std::string>* pList = nullptr) noexcept;// cerca i campi specificati nel filtro, assegna il valore del record trovato a result
	CGenTableRecord	Lookup(long idtable)  const noexcept;
	CGenRecordList	GetRecordList(const std::string& filter) const noexcept;
	const CGenRecordList& GetTable() const noexcept { return m_tableDataset; };
	long					GetRecordNumber() const noexcept { return m_tableDataset.size(); };
	void					AddFilterField(const char* fieldName, const char* oper, double value, std::string& filter, bool IsNullValid = false, long valoreor = -99) const noexcept;
	void					AddFilterField(const char* fieldName, const char* oper, const char* value, std::string& filter, bool IsNullValid = false, long valoreor = -99) const noexcept;
	bool					IsEmpty() const noexcept { return m_tableDataset.size() == 0; };
	
private:
	Value	emptyValue;
	std::vector<std::string> m_columnName;
	//std::unordered_map<long, std::string> m_tableDataset; //Salvo il contenuto di un record in un documento json
	CGenRecordList m_tableDataset; //Salvo il contenuto di un record in un documento json
	std::map<long, long> m_tableID_Pos_Dataset; //Salvo il contenuto di un record in un documento json
	CGenTableRecord m_lastRecord;
	//Document currRecord;

};

/*GF 14-04-23 VECCHIA VERSIONE
class CGenTableRecord
{
public:
	CGenTableRecord() {};
	CGenTableRecord(const char* jsonRecord)
	{
		resultDoc.Parse(jsonRecord);
	}
	bool	GetColumn(short columnIndex, double& doubleval) const noexcept;
	bool	GetColumn(short columnIndex, short& doubleval) const noexcept;
	bool	GetColumn(const char* fieldName, double& doubleval) const noexcept;
	bool	GetColumn(const char* fieldName, long &doubleval) const noexcept;
	bool	GetColumn(const char* fieldName, int& doubleval) const noexcept;
	bool	GetColumn(const char* fieldName, short& doubleval) const noexcept;
	bool	GetColumn(const char* fieldName, std::string& stringVal) const noexcept;
	bool	GetColumn(const char* fieldName, CString& stringVal) const noexcept;
	bool	IsValid() const noexcept { return resultDoc.IsObject(); };
private:
	Document resultDoc;
};
class CGenTable
{
public:

	CGenTable() {};
	
	bool		 LoadFromDB(const CSession& session, const wchar_t* sqlExpression, bool checkExist=true);
	//virtual bool LoadFromDB(const CDBCmdBase& m_dbMacchineSession) { return false; };
	//virtual bool LoadFromExcel(libxl::Book* book) { return false; };
	const wchar_t*	Lookup(long idtable, const char* tagName) noexcept; // retistuisce una stringa col valore del campo <tagname> per il record che ha chiave idtable
	CGenTableRecord	Lookup();// cerca i campi specificati nel filtro, assegna il valore del record trovato a result
	CGenTableRecord Lookup(long idtable);
	void		ResetFilter() noexcept { filter.clear(); };
	void		AddFilterField(const char* fieldName, const char* oper, double value);
	void		AddFilterField(const char* fieldName, const char* oper, const char* value);
	
private:
	Value	emptyValue;
	std::vector<std::string> m_columnName;
	//std::unordered_map<long, std::string> m_tableDataset; //Salvo il contenuto di un record in un documento json
	std::vector<std::string> m_tableDataset; //Salvo il contenuto di un record in un documento json
	std::map<long, long> m_tableID_Pos_Dataset; //Salvo il contenuto di un record in un documento json
	std::string filter;
	Document currRecord;
	
}; 
*/