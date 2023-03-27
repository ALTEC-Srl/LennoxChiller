#pragma once

#include <atlstr.h>
#include <string>
#include <msclr\marshal_cppstd.h>
#include "rapidjson\document.h"     // rapidjson's DOM-style API
#include "rapidjson\rapidjson.h"
#include "rapidjson\stringbuffer.h"
#include "rapidjson\writer.h"


// Metodi per EBM 
//typedef int (__stdcall *PEBMPAPSTFAN_FNCT6)(	char** in, char** out);
typedef int(__stdcall* PEBMPAPSTFAN_FNCT1)	(char** in);
typedef int(__stdcall* PEBMPAPSTFAN_FNCT20)	(wchar_t* in);
typedef int(__stdcall* PEBMPAPSTFAN_FNCT2)();
typedef int(__stdcall* PEBMPAPSTFAN_FNCT6)	(char* in, char** out);		// MV 10.06.2014. Per utilizzare la DLL nuova 3.0.1.0
typedef int(__stdcall* PEBMPAPSTFAN_FNCT18)(char* in, char** out);		// MV 10.06.2014. Per utilizzare la DLL nuova 3.0.1.0
typedef int(__stdcall* PEBMPAPSTFAN_FNCT7)	(int s1, int s2, double* input, char* key, int z1, int z2, double* output);
typedef int(__stdcall* PEBMPAPSTFAN_FNCT17)(char* KEY, char* SIZE, char* TYP, char* ISOCLASS, char* PROTECTION, int z1, int z2, double* OUTPUT);

using namespace System;
using namespace msclr::interop;
using namespace rapidjson;

namespace LennoxRooftop {

	class CModelAccessor : public CADORecordBinding {
		BEGIN_ADO_BINDING(CModelAccessor)

			// Column fname is the 2nd field in the table     
			ADO_VARIABLE_LENGTH_ENTRY2(3, adVarChar, m_Nomecomm,
				sizeof(m_Nomecomm), m_NomecommStatus, FALSE)

			/*/ Column lname is the 4th field in the table.  
			ADO_VARIABLE_LENGTH_ENTRY2(4, adVarChar, m_sze_lname,
				sizeof(m_sze_lname), le_lnameStatus, FALSE)

			// Column hiredate is the 8th field in the table.  
			ADO_VARIABLE_LENGTH_ENTRY2(8, adDBDate, m_sze_hiredate,
				sizeof(m_sze_hiredate), le_hiredateStatus, TRUE)
				*/
			END_ADO_BINDING()

	public:
		CHAR   m_Nomecomm[255];
		ULONG   m_NomecommStatus;
		
	};

	public ref class Rooftop
	{
	public:
		String^ GetModelPerformance(String^ jSONIN);
		String^ GetWaterCoilPerformance(String^ jSONIN);
		String^ GetFanPerformance(String^ jSONIN);
		String^ GetNoiseData(String^ jSONIN);
		String^ GetNoiseData1(String^ jSONIN);
		String^ GetOptionsPressureDrop(String^ jSONIN);
		String^ GetDrawing(String^ jSONIN);
		String^ GetBIMModel(String^ jSONIN);

	private:
		bool LoadEBMDll();
		short GetSFPClass(double sfp);
		//recupero le informazioni nel database interno LENNOX in base all'idmodello richiesto
		//ritorna un json creato con i dati di ritorno del recordset	
		String^ SearchModel(int model);
	};
	///////////////////////////////////////////////////////////////////////////

}
