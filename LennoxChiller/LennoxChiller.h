#pragma once

#include <atlstr.h>
#include <string>
#include <msclr\marshal_cppstd.h>
#include "rapidjson\document.h"     // rapidjson's DOM-style API


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

namespace LennoxChiller {
	public ref class CalculateFan
	{
	public:
		int GetFanPerformance(String^ jSONIN);
	private:
		bool LoadEBMDll();

	};
}
