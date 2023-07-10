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
		//::MessageBox(NULL, CString(acc1.GetColumnName(i)), _T("modello fan"), MB_OK);
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
		//::MessageBox(NULL, buffer.GetString(), _T("modello fan"), MB_OK);
		std::string output = buffer.GetString();
		m_tableDataset.push_back(output); // sonRecord.; // .CopyFrom(jsonRecord, document.GetAllocator());
		m_tableID_Pos_Dataset[IDtable] = m_tableDataset.size()-1;
		hr = acc1.MoveNext();
	}
	
	acc1.Close();
	return true;
}


const wchar_t* CGenTable::Lookup(long idtable, const char* tagName) const noexcept
{
	
	auto record = m_tableID_Pos_Dataset.find(idtable);
	if (record == m_tableID_Pos_Dataset.end())
		return nullptr;
	
	USES_CONVERSION;
	/*currRecord.Parse(m_tableDataset[record->second].c_str());
	ASSERT(currRecord.HasMember(tagName));
	return A2W(currRecord[tagName].GetString());*/
	Document recordJSON; recordJSON.Parse(m_tableDataset[record->second].c_str());
	ASSERT(recordJSON.HasMember(tagName));
	return A2W(recordJSON[tagName].GetString());
	
}

// ritorna una stringa json con il record che corrisponde alla ricerca effettuata
CGenTableRecord CGenTable::Lookup(long idtable) const noexcept
{
	auto record = m_tableID_Pos_Dataset.find(idtable);
	if (record == m_tableID_Pos_Dataset.end())
		return CGenTableRecord();

	return CGenTableRecord(m_tableDataset[record->second].c_str());
}

// ritorna una stringa json con il record che corrisponde alla ricerca effettuata
CGenTableRecord CGenTable::Lookup(const std::string& filter, std::vector<std::string>* pList)  const noexcept
{
	if (filter.size() == 0)
		return CGenTableRecord();

	Document record, document;
	document.Parse(filter.c_str());
	// trovo il primo record, si assume che l'ordinamento sia corretto in base all'elenco dei campi indicati nel filtro
	// FS 29-08-2022 se l'operatore >= ha operando pari a -9999 allora cerco il record conil massimo valore del campo a parità delle altre condizioni, 
	// vale sempre l'assunzione sul criterio sull'ordinamento
	short cont = 0;
	std::string lastGood;
	double toCompare, compareTo, compareTo2;
	const char* compareToS = nullptr;
	for (const auto& recordJson : m_tableDataset)
	{
		cont++;
		record.Parse(recordJson.c_str());
		bool cond = true;
		const rapidjson::Value& k = document["fields"];

		for (rapidjson::Value::ConstValueIterator field = k.Begin(); field != k.End(); ++field)
		{
			const rapidjson::Value& field1 = *field; // Singolo oggetto del JSON
			// Scorre l'elenco dei campi sui quali c'è una condizione nel filtro di input
			rapidjson::Value::ConstMemberIterator m_iter = field1.MemberBegin();// primo campo dell'oggetto
			compareTo = -99;
			if (m_iter->value.IsDouble())
				compareTo = m_iter->value.GetDouble();
			if (m_iter->value.IsString())
				compareToS = m_iter->value.GetString();

			std::string fieldName = m_iter->name.GetString();
			m_iter++; std::string oper = m_iter->value.GetString();
			m_iter++; ASSERT(m_iter->value.IsBool());  bool isNullValid = m_iter->value.GetBool(); // null valore valido ?
			m_iter++; long OrValue = m_iter->value.GetInt(); // FS 16-10-2022 Valore in or
			compareTo2 = OrValue != -99 ? OrValue : compareTo;
			bool isNull = false;
			// Assumo che il campo oesista non faccio controllo sulla validità di fieldName
			ASSERT(record.HasMember(fieldName.c_str()));
			ASSERT(record.FindMember(fieldName.c_str()) != record.MemberEnd());
			const rapidjson::Value& tcValue = record[fieldName.c_str()]; //; itr->value.GetDouble();

			if (!tcValue.IsString())
			{
				if (tcValue.IsNull())
					isNull = true;
				else
					toCompare = tcValue.GetDouble();
			}

			if (oper.compare("=") == 0)
			{
				if (tcValue.IsString())
				{
					if (strcmp(compareToS, tcValue.GetString()) != 0)
						cond = false;
				}
				else // Double 
				{
					if (isNull && !isNullValid || (!isNull && (toCompare != compareTo && toCompare != compareTo2)))
						cond = false;
				}
			}
			else
			{
				if (oper.compare(">=") == 0)
				{
					if ((toCompare < compareTo || compareTo == -9999) && (toCompare < compareTo2))
					{
						cond = false;
						if (compareTo == -9999 && cont == m_tableDataset.size())
							cond = true;//l'ultimo è quello che cerco se voglio il record con il valore massimo del campo
					}
				}
				else
				{
					if (oper.compare(">") == 0)
					{
						if ((toCompare <= compareTo || compareTo == -9999) && (toCompare <= compareTo2))
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
							if (toCompare > fabs(compareTo) && (toCompare > fabs(compareTo2)))
							{
								cond = compareTo < 0; //false;
							}
							else
							{
								if (compareTo < 0) // Cerco il record con il valore massimo del campo inferiore al valore del filtro 
								{
									lastGood = recordJson;
									cond = false;
								}
							}
						}
					}
				}
			}
			if (!cond)
				break;
		}
		if (cond)
		{
			if (!pList)
			{
				if (lastGood.length() > 0)
					return CGenTableRecord(lastGood.c_str());
				//m_lastRecord = record;
				return CGenTableRecord(recordJson.c_str());
			}

			pList->push_back(recordJson.c_str());
		}
	}
	if (lastGood.length() > 0)
		return CGenTableRecord(lastGood.c_str());

	return CGenTableRecord();
}

const CGenTableRecord& CGenTable::Lookup1(const std::string& filter, std::vector<std::string>* pList)  noexcept
{
	if (filter.size() == 0)
		return CGenTableRecord();

	Document record, document;
	document.Parse(filter.c_str());
	// trovo il primo record, si assume che l'ordinamento sia corretto in base all'elenco dei campi indicati nel filtro
	// FS 29-08-2022 se l'operatore >= ha operando pari a -9999 allora cerco il record conil massimo valore del campo a parità delle altre condizioni, 
	// vale sempre l'assunzione sul criterio sull'ordinamento
	short cont = 0;
	std::string lastGood;
	double toCompare, compareTo, compareTo2;
	const char* compareToS = nullptr;
	for (const auto& recordJson : m_tableDataset)
	{
		cont++;
		record.Parse(recordJson.c_str());
		bool cond = true;
		const rapidjson::Value& k = document["fields"];

		for (rapidjson::Value::ConstValueIterator field = k.Begin(); field != k.End(); ++field)
		{
			const rapidjson::Value& field1 = *field; // Singolo oggetto del JSON
			// Scorre l'elenco dei campi sui quali c'è una condizione nel filtro di input
			rapidjson::Value::ConstMemberIterator m_iter = field1.MemberBegin();// primo campo dell'oggetto
			if (m_iter->value.IsDouble())
				compareTo = m_iter->value.GetDouble();
			if (m_iter->value.IsString())
				compareToS = m_iter->value.GetString();

			std::string fieldName = m_iter->name.GetString();
			m_iter++; std::string oper = m_iter->value.GetString();
			m_iter++; ASSERT(m_iter->value.IsBool());  bool isNullValid = m_iter->value.GetBool(); // null valore valido ?
			m_iter++; long OrValue = m_iter->value.GetInt(); // FS 16-10-2022 Valore in or
			compareTo2 = OrValue != -99 ? OrValue : compareTo;
			bool isNull = false;
			// Assumo che il campo oesista non faccio controllo sulla validità di fieldName
			ASSERT(record.HasMember(fieldName.c_str()));
			ASSERT(record.FindMember(fieldName.c_str()) != record.MemberEnd());
			const rapidjson::Value& tcValue = record[fieldName.c_str()]; //; itr->value.GetDouble();

			if (!tcValue.IsString())
			{
				if (tcValue.IsNull())
					isNull = true;
				else
					toCompare = tcValue.GetDouble();
			}

			if (oper.compare("=") == 0)
			{
				if (tcValue.IsString())
				{
					if (strcmp(compareToS, tcValue.GetString()) != 0)
						cond = false;
				}
				else // Double 
				{
					if (isNull && !isNullValid || (!isNull && (toCompare != compareTo && toCompare != compareTo2)))
						cond = false;
				}
			}
			else
			{
				if (oper.compare(">=") == 0)
				{
					if ((toCompare < compareTo || compareTo == -9999) && (toCompare != compareTo2))
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
						if (toCompare > fabs(compareTo) && (toCompare != compareTo2))
						{
							cond = compareTo < 0; //false;
						}
						else
						{
							if (compareTo < 0) // Cerco il record con il valore massimo del campo inferiore al valore del filtro 
							{
								lastGood = recordJson;
								cond = false;
							}
						}
					}
				}
			}
			if (!cond)
				break;
		}
		if (cond)
		{
			if (!pList)
			{
				if (lastGood.length() > 0)
					return CGenTableRecord(lastGood.c_str());
				m_lastRecord = record;
				return m_lastRecord; // CGenTableRecord(recordJson.c_str());
			}

			pList->push_back(recordJson.c_str());
		}
	}
	if (lastGood.length() > 0)
		return CGenTableRecord(lastGood.c_str());

	return CGenTableRecord();
}

std::vector<std::string> CGenTable::GetRecordList(const std::string& filter) const noexcept
{
	std::vector<std::string> recordList;
	Lookup(filter, &recordList);
	return recordList;

}
void CGenTable::AddFilterField(const char* fieldName, const char* oper, const char* value, std::string& filter, bool IsNullValid, long id1) const noexcept
{
	// aggiungo il primo elemento
	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	if (filter.size() == 0)
	{
		writer.StartObject();
		writer.Key("fields");
		writer.StartArray();
		writer.StartObject();
		writer.Key(fieldName); writer.String(value);
		writer.Key("OP"); writer.String(oper);
		writer.Key("ISNULL"); writer.Bool(IsNullValid);
		writer.Key("VALOREOR"); writer.Int(id1);
		writer.EndObject();
		writer.EndArray();
		writer.EndObject();
		filter = buffer.GetString();
		return;
	}
	// Aggiunge un elemento all'array dei campi su cui filtrare
	Document doc; doc.Parse(filter.c_str());
	Value object(kObjectType);
	object.AddMember(StringRef(fieldName), StringRef(value), doc.GetAllocator());
	object.AddMember("OP", Value().SetString(oper, doc.GetAllocator()), doc.GetAllocator());
	object.AddMember("ISNULL", IsNullValid, doc.GetAllocator());
	object.AddMember("VALOREOR", id1, doc.GetAllocator());

	Value& currArray = doc["fields"];
	ASSERT(currArray.IsArray());
	currArray.PushBack(object, doc.GetAllocator());
	doc.Accept(writer);
	filter = buffer.GetString();
}

void CGenTable::AddFilterField(const char* fieldName, const char* oper, double value, std::string& filter, bool IsNullValid, long id1) const noexcept
{
	// aggiungo il primo elemento
	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	if (filter.size() == 0)
	{
		writer.StartObject();
		writer.Key("fields");
		writer.StartArray();
		writer.StartObject();
		writer.Key(fieldName); writer.Double(value);
		writer.Key("OP"); writer.String(oper);
		writer.Key("ISNULL"); writer.Bool(IsNullValid);
		writer.Key("VALOREOR"); writer.Int(id1);
		writer.EndObject();
		writer.EndArray();
		writer.EndObject();
		filter = buffer.GetString();
		return;
	}
	// Aggiunge un elemento all'array dei campi su cui filtrare
	Document doc; doc.Parse(filter.c_str());
	Value object(kObjectType);
	object.AddMember(StringRef(fieldName), value, doc.GetAllocator());
	object.AddMember("OP", Value().SetString(oper, doc.GetAllocator()), doc.GetAllocator());
	object.AddMember("ISNULL", IsNullValid, doc.GetAllocator());
	object.AddMember("VALOREOR", id1, doc.GetAllocator());
	Value& currArray = doc["fields"];
	ASSERT(currArray.IsArray());
	currArray.PushBack(object, doc.GetAllocator());
	doc.Accept(writer);
	filter = buffer.GetString();

}

/*
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
*/

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
const Value& CGenTableRecord::GetColumn(short columnIndex) const noexcept
{
	rapidjson::Value::ConstMemberIterator m_iter = resultDoc.MemberBegin();// primo campo dell'oggetto
	m_iter += columnIndex - 1;
	return m_iter->value;


}

const Value& CGenTableRecord::GetColumn(const char* fieldName) const noexcept
{
	return resultDoc[fieldName];
	//rapidjson::Value::ConstMemberIterator m_iter = resultDoc.MemberBegin();// primo campo dell'oggetto
	//m_iter += columnIndex - 1;
	//return m_iter->value;


}