#pragma once
#include "rapidjson/document.h"     // rapidjson's DOM-style API
#include <string>
using namespace rapidjson;

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
	
	bool		 LoadFromDB(const CDBCmdBase& m_dbMacchineSession, const wchar_t* sqlExpression, bool checkExist=true);
	virtual bool LoadFromDB(const CDBCmdBase& m_dbMacchineSession) { return false; };
	virtual bool LoadFromExcel(libxl::Book* book) { return false; };
	const wchar_t*	Lookup(long idtable, const char* tagName) noexcept; // retistuisce una stringa col valore del campo <tagname> per il record che ha chiave idtable
	CGenTableRecord	Lookup();// cerca i campi specificati nel filtro, assegna il valore del record trovato a result
	CGenTableRecord Lookup(long idtable);
	void		ResetFilter() noexcept { filter.clear(); };
	void		AddFilterField(const char* fieldName, const char* oper, double value);
	
private:
	Value	emptyValue;
	std::vector<std::string> m_columnName;
	//std::unordered_map<long, std::string> m_tableDataset; //Salvo il contenuto di un record in un documento json
	std::vector<std::string> m_tableDataset; //Salvo il contenuto di un record in un documento json
	std::map<long, long> m_tableID_Pos_Dataset; //Salvo il contenuto di un record in un documento json
	std::string filter;
	Document currRecord;
	
}; 
