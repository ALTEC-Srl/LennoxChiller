#pragma once

#include <atlstr.h>
#include <string>
#include <map>
#include <msclr\marshal_cppstd.h>
#include "rapidjson\document.h"     // rapidjson's DOM-style API
#include "rapidjson\rapidjson.h"
#include "rapidjson\stringbuffer.h"
#include "rapidjson\writer.h"
#include "msjetoledb.h"
#include "GenTable.h"
#include "Gasvapore.h"


// Metodi per EBM 
//typedef int (__stdcall *PEBMPAPSTFAN_FNCT6)(	char** in, char** out);
typedef int(__stdcall* PEBMPAPSTFAN_FNCT1)	(char** in);
typedef int(__stdcall* PEBMPAPSTFAN_FNCT20)	(wchar_t* in);
typedef int(__stdcall* PEBMPAPSTFAN_FNCT2)();
typedef int(__stdcall* PEBMPAPSTFAN_FNCT6)	(char* in, char** out);		// MV 10.06.2014. Per utilizzare la DLL nuova 3.0.1.0
typedef int(__stdcall* PEBMPAPSTFAN_FNCT18)(char* in, char** out);		// MV 10.06.2014. Per utilizzare la DLL nuova 3.0.1.0
typedef int(__stdcall* PEBMPAPSTFAN_FNCT7)	(int s1, int s2, double* input, char* key, int z1, int z2, double* output);
typedef int(__stdcall* PEBMPAPSTFAN_FNCT17)(char* KEY, char* SIZE, char* TYP, char* ISOCLASS, char* PROTECTION, int z1, int z2, double* OUTPUT);
typedef int(__stdcall* PEBMPAPSTFAN_FNCT30)(wchar_t* in);

using namespace System;
using namespace msclr::interop;
using namespace rapidjson;
using namespace std;

#using <leelcoilsDLL.dll>
using namespace leelcoilsDLL;


#//using <D:\lavoro 2022\LENNOX\TestAppVB\bin\Debug\Newtonsoft.Json.11.0.2\lib\net45\Newtonsoft.Json.dll>

//using namespace Newtonsoft::Json;
//using namespace Newtonsoft::Json::Linq;

CDataSource		g_Sql;
CSession		g_session;
CGenTable       g_ModelTable;
CGenTable       g_CoeffPdc;
CGenTable		g_NoiseAtt;
namespace LennoxRooftop {

	/*class CModelAccessor : public CADORecordBinding {
		BEGIN_ADO_BINDING(CModelAccessor)

			// Column fname is the 2nd field in the table     
			ADO_VARIABLE_LENGTH_ENTRY2(3, adVarChar, m_Nomecomm,
				sizeof(m_Nomecomm), m_NomecommStatus, FALSE)

			END_ADO_BINDING()

	public:
		CHAR   m_Nomecomm[255];
		ULONG   m_NomecommStatus;
		
	};*/

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
		
		int Init(short deb);
	private:
		bool LoadEBMDll();
		short GetSFPClass(double sfp);
		//recupero le informazioni nel database interno LENNOX in base all'idmodello richiesto
		//ritorna un json creato con i dati di ritorno del recordset	
		String^ SearchModel(CString model, CString fanopt, double portata, int fantype);
		bool LoadModel();
		bool LoadCoeffPdc();
		bool LoadNoiseAttenuation();
		bool OpenConnection();
		bool OpenDataSource(CString fileName, CDataSource& ds, CString provider, CString pwd);
		String^ GetCondeserNoise();
		short GetAttenuazioni(CString code, double attenuazioni[8][8]);
		double FormatString(CString temp);
		CString findnextstringcoil(CString sigla, short i);
		void LogFile(const CString& log);
		
	};
	///////////////////////////////////////////////////////////////////////////

}
