#include "pch.h"
#include "GenTable.h"
#include "rapidjson\rapidjson.h"
#include "rapidjson\stringbuffer.h"
#include "rapidjson\writer.h"

extern  void DisplayOLEDBErrorRecords(HRESULT hrErr = S_OK);
bool CGenTable::LoadFromDB(const CSession& session, const wchar_t* sqlExpression, bool checkExist )
{
	
	CCommand<CDynamicAccessor> acc1;
	 
	HRESULT hr = acc1.Open(session, sqlExpression);
	if (!SUCCEEDED(hr) || hr == DB_E_NOTABLE)
	{
		DisplayOLEDBErrorRecords(hr);
		return !checkExist;
	}
	m_columnName.clear();
	USES_CONVERSION;
	// FS recupero i nomi dei campi, il primo campo della query deve sempre essere la chiave della tabella
	long colCount = acc1.GetColumnCount();
	for (short i = 1; i <= colCount; i++)
	{
		m_columnName.push_back(CString(acc1.GetColumnName(i)).GetString());
	}
	
	hr = acc1.MoveFirst();
	
	long IDtable;
	while (hr == S_OK)
	{
		rapidjson::StringBuffer buffer;
		rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
		writer.StartObject();
		acc1.GetValue(1, &IDtable);
		//Value jsonRecord(kObjectType);
		//Document document;
		for (short i = 1; i <= colCount; i++)
		{
			DBTYPE type; acc1.GetColumnType(i, &type);
			//Value name; name.SetString(m_columnName[i - 1].c_str(), m_columnName[i - 1].length(), document.GetAllocator());
			writer.Key(m_columnName[i - 1].c_str());
			DBSTATUS st; acc1.GetStatus(i, &st);
			switch (type)
			{
			case DBTYPE_BOOL:
			{
				if (st != DBSTATUS_S_ISNULL)
				{
					bool value; acc1.GetValue(i, &value);
					writer.Int(value);
				}
				else
					writer.Int(0);
				break;
			}
			case DBTYPE_I2:
			{
				if (st != DBSTATUS_S_ISNULL)
				{
					short value; acc1.GetValue(i, &value);
					writer.Int(value);
				}
				else
					writer.Int(0);

				break;
			}
			case DBTYPE_I4:
			{
				if (st != DBSTATUS_S_ISNULL)
				{
					long value; acc1.GetValue(i, &value);
					writer.Int64(value);
				}
				else
					writer.Int64(0);
				break;
			}
			case DBTYPE_R4:
			{
				if (st != DBSTATUS_S_ISNULL)
				{
					float value; acc1.GetValue(i, &value);
					writer.Double(value);
				}
				else
					writer.Double(0.0);
			}
			break;
			case DBTYPE_R8:
			{
				if (st != DBSTATUS_S_ISNULL)
				{
					double value; acc1.GetValue(i, &value);
					writer.Double(value);
				}else
					writer.Double(0.0);
					
			}
			break;
			case DBTYPE_WSTR:
			{
					
				if (st != DBSTATUS_S_ISNULL)
				{
					std::wstring value = (wchar_t*)acc1.GetValue(i);
					writer.String(W2A(value.c_str()));
				}
				else
					writer.String("");

				break;
			}
			case DBTYPE_STR:
			{
				DBSTATUS st; acc1.GetStatus(i, &st);
				if (st != DBSTATUS_S_ISNULL)
					writer.String((char*)acc1.GetValue(i));
				else
					writer.String("");
				break;
			}
			break;
			case DBTYPE_DATE:
			{
				if (st != DBSTATUS_S_ISNULL)
				{
					DATE date; acc1.GetValue(i, &date);
					//COleDateTime dat = date;
					std::string value = marshal_as<std::string>(date.ToString());
					writer.String(value.c_str());
					//document.AddMember(name, date, document.GetAllocator());
				}
				else
					writer.String("");
				break;
			}
			break;
			default:
				ASSERT(FALSE);
				break;
			}
			
		}
		writer.EndObject();
		std::string output = buffer.GetString();
		m_tableDataset.push_back(output); // sonRecord.; // .CopyFrom(jsonRecord, document.GetAllocator());
		m_tableID_Pos_Dataset[IDtable] = m_tableDataset.size()-1;
		hr = acc1.MoveNext();
	}
	
	acc1.Close();
	return true;
}


const wchar_t* CGenTable::Lookup(long idtable, const char* tagName) noexcept
{
	
	auto record = m_tableID_Pos_Dataset.find(idtable);
	if (record == m_tableID_Pos_Dataset.end())
		return nullptr;
	
	USES_CONVERSION;
	currRecord.Parse(m_tableDataset[record->second].c_str());
	ASSERT(currRecord.HasMember(tagName));
	return A2W(currRecord[tagName].GetString());
	
}

// ritorna una stringa json con il record che corrisponde alla ricerca effettuata
CGenTableRecord CGenTable::Lookup(long idtable)
{
	auto record = m_tableID_Pos_Dataset.find(idtable);
	if (record == m_tableID_Pos_Dataset.end())
		return CGenTableRecord();

	return CGenTableRecord(m_tableDataset[record->second].c_str());
}

// ritorna una stringa json con il record che corrisponde alla ricerca effettuata
CGenTableRecord CGenTable::Lookup()
{
	if (filter.size() == 0)
		return CGenTableRecord();

	Document record, document;
	document.Parse(filter.c_str());
	// trovo il primo record, si assume che l'ordinamento sia corretto in base all'elenco dei campi indicati nel filtro
	// FS 29-08-2022 se l'operatore >= ha operando pari a -9999 allora cerco il record conil massimo valore del campo a parità delle altre condizioni, 
	// vale sempre l'assunzione sul criterio sull'ordinamento
	short cont = 0;

	for (const auto& recordJson : m_tableDataset)
	{
		cont++;
		record.Parse(recordJson.c_str());
		bool cond = true;
		const rapidjson::Value& k = document["fields"];
		for (rapidjson::Value::ConstValueIterator field = k.Begin(); field != k.End(); ++field)
		{
			const rapidjson::Value& field1 = *field;
			// Scorre l'elenco dei campi sui quali c'è una condizione nel filtro di input
			//for (rapidjson::Value::ConstMemberIterator m_iter = field1.MemberBegin();m_iter != field1.MemberEnd(); ++m_iter)//kf pair
			rapidjson::Value::ConstMemberIterator m_iter = field1.MemberBegin();//kf pair
			if (m_iter->value.IsDouble())
			{
				double compareTo = m_iter->value.GetDouble();
				std::string fieldName = m_iter->name.GetString();
				m_iter++; // operatore
				std::string oper = m_iter->value.GetString();
				// Assumo che il cmap oesista non faccio controllo sulla validità di fieldName
				ASSERT(record.HasMember(fieldName.c_str()));
				double toCompare = record[fieldName.c_str()].GetDouble();
				if (oper.compare("=") == 0)
				{
					if (toCompare != compareTo)
						cond = false;
				}
				else
				{
					if (oper.compare(">=") == 0)
					{
						if (toCompare < compareTo || compareTo == -9999)
						{
							cond = false;
							if (compareTo == -9999 && cont == m_tableDataset.size())
								cond = true;//l'ultimo è quello che cerco se voglio il record con il valore massimo del campo
						}
					}
					else
					{
						if (oper.compare("<=") == 0)
						{
							if (toCompare > compareTo)
								cond = false;
						}
					}
				}
			}
			else
			{
				CString compareTo = m_iter->value.GetString();
				std::string fieldName = m_iter->name.GetString();
				m_iter++; // operatore
				std::string oper = m_iter->value.GetString();
				// Assumo che il cmap oesista non faccio controllo sulla validità di fieldName
				ASSERT(record.HasMember(fieldName.c_str()));
				CString toCompare = record[fieldName.c_str()].GetString();
				if (oper.compare("=") == 0)
				{
					if (toCompare != compareTo)
						cond = false;
				}
			}
			if (!cond)
				break;
		}
		if (cond)
		{
			return CGenTableRecord(recordJson.c_str());
		}
	}
	
	return CGenTableRecord();
}

void CGenTable::AddFilterField(const char* fieldName, const char* oper, double value)
{
	// aggiungo il primo elemento
	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	if (filter.size() == 0)
	{
		writer.StartObject();
		writer.Key("fields");
		writer.StartArray();
		writer.StartObject(); writer.Key(fieldName); writer.Double(value); writer.Key("OP"); writer.String(oper); writer.EndObject();
		writer.EndArray();
		writer.EndObject();
		filter = buffer.GetString();
		return;
	}
	// Aggiunge un elemento all'array dei campi su cui filtrare
	Document doc; doc.Parse(filter.c_str());
	Value object(kObjectType);
	object.AddMember(StringRef(fieldName),value, doc.GetAllocator());
	object.AddMember("OP", Value().SetString(oper, doc.GetAllocator()), doc.GetAllocator());
	
	Value& currArray = doc["fields"];
	ASSERT(currArray.IsArray());
	currArray.PushBack(object,doc.GetAllocator());
	doc.Accept(writer);
	filter = buffer.GetString();

}
void CGenTable::AddFilterField(const char* fieldName, const char* oper, const char* value)
{
	// aggiungo il primo elemento
	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	if (filter.size() == 0)
	{
		writer.StartObject();
		writer.Key("fields");
		writer.StartArray();
		writer.StartObject(); writer.Key(fieldName); writer.String(value); writer.Key("OP"); writer.String(oper); writer.EndObject();
		writer.EndArray();
		writer.EndObject();
		filter = buffer.GetString(); 
		return;
	}
	// Aggiunge un elemento all'array dei campi su cui filtrare
	Document doc; 
	doc.Parse(filter.c_str());
	Value object(kObjectType);
	object.AddMember(StringRef(fieldName), StringRef(value), doc.GetAllocator());
	object.AddMember("OP", Value().SetString(oper, doc.GetAllocator()), doc.GetAllocator());

	Value& currArray = doc["fields"];
	ASSERT(currArray.IsArray());
	currArray.PushBack(object, doc.GetAllocator());
	doc.Accept(writer);
	filter = buffer.GetString();

}


bool CGenTableRecord::GetColumn(short columnIndex, double& doubleval) const noexcept
{
	rapidjson::Value::ConstMemberIterator field = resultDoc.MemberBegin()+ columnIndex-1;
	const rapidjson::Value& field1 = field->value;
	doubleval = field1.GetDouble();
	//std::string name = field->name.GetString();
	return true;
}

bool CGenTableRecord::GetColumn(short columnIndex, short& shortval) const noexcept
{
	rapidjson::Value::ConstMemberIterator field = resultDoc.MemberBegin() + columnIndex - 1;
	const rapidjson::Value& field1 = field->value;
	shortval = field1.GetInt();
	//std::string name = field->name.GetString();
	return true;
}

bool CGenTableRecord::GetColumn(const char* fieldName, double& doubleval) const noexcept
{
	ASSERT(resultDoc.HasMember(fieldName));
	if (!resultDoc.HasMember(fieldName))
		return false;
	doubleval = resultDoc[fieldName].GetDouble();
	return true;
}
bool CGenTableRecord::GetColumn(const char* fieldName, long& doubleval) const noexcept
{
	ASSERT(resultDoc.HasMember(fieldName));
	if (!resultDoc.HasMember(fieldName))
		return false;
	doubleval = (long) resultDoc[fieldName].GetDouble();
	return true;
}

bool CGenTableRecord::GetColumn(const char* fieldName, int& doubleval) const noexcept
{
	ASSERT(resultDoc.HasMember(fieldName));
	if (!resultDoc.HasMember(fieldName))
		return false;
	doubleval = (int)resultDoc[fieldName].GetDouble();
	return true;
}

bool CGenTableRecord::GetColumn(const char* fieldName, short& doubleval) const noexcept
{
	ASSERT(resultDoc.HasMember(fieldName));
	if (!resultDoc.HasMember(fieldName))
		return false;
	doubleval = (short)resultDoc[fieldName].GetDouble();
	return true;
}

bool CGenTableRecord::GetColumn(const char* fieldName, std::string& stringVal) const noexcept
{
	ASSERT(resultDoc.HasMember(fieldName));
	if (!resultDoc.HasMember(fieldName))
		return false;
	stringVal = resultDoc[fieldName].GetString();
	return true;
}

bool CGenTableRecord::GetColumn(const char* fieldName, CString& stringVal) const noexcept
{
	ASSERT(resultDoc.HasMember(fieldName));
	if (!resultDoc.HasMember(fieldName))
		return false;
	USES_CONVERSION;
	stringVal = A2W(resultDoc[fieldName].GetString());
	return true;
}
