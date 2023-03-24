#include "pch.h"
#include "LennoxChiller.h"

////////////////////////////////////////////////////////////////////////////////////////////////
CString m_fanEBM;

HMODULE		g_EbmPapstFanDLL = NULL;
PEBMPAPSTFAN_FNCT1	GET_PRODUCTS_PC = NULL;
PEBMPAPSTFAN_FNCT6	GET_TECHNICAL_DATA_PC = NULL;
PEBMPAPSTFAN_FNCT6	GET_CALCULATION_FAN_ALONE_PC = NULL;
PEBMPAPSTFAN_FNCT6	SEARCH_PRODUCTS = NULL;
PEBMPAPSTFAN_FNCT7	GET_CALCULATION_FANMOTOR = NULL;
PEBMPAPSTFAN_FNCT6	GET_GRAPH_PFA_PC = NULL;
PEBMPAPSTFAN_FNCT18	GET_FAN_CURVE = NULL;
PEBMPAPSTFAN_FNCT6	GET_GRAPH_N_PC = NULL;
PEBMPAPSTFAN_FNCT6	GET_GRAPH_ETA_PC = NULL;
PEBMPAPSTFAN_FNCT6	GET_GRAPH_POWER_PC = NULL;
PEBMPAPSTFAN_FNCT6	GET_GRAPH_SOUND_PC = NULL;
PEBMPAPSTFAN_FNCT17	GET_STANDARDS_FANMOTOR = NULL;
PEBMPAPSTFAN_FNCT6	CALCULATE_FANGRID = NULL;
PEBMPAPSTFAN_FNCT2	GET_DLL_VERSION = NULL;
PEBMPAPSTFAN_FNCT1  SET_XML_PATH = NULL;
PEBMPAPSTFAN_FNCT20 SET_XML_PATH_WS = NULL;

using namespace LennoxChiller;
constexpr long lenRis = 40000;


        int CalculateFan::GetFanPerformance(String^ jSONIN)
        {
            std::string str = marshal_as<std::string>(jSONIN);

            // lettura dei risultati
            Document doc;
            doc.Parse(str.c_str());
            int model = doc["MODELID"].GetInt();
            int supplier = doc["SUPPLIERID"].GetInt();
            double port = doc["airflow"].GetDouble();
            int opt = doc["fantypeoption"].GetInt();
            double pTot = doc["totaloptionpressuredrop"].GetDouble();
			LoadEBMDll();
			CString request;
			INT num = 0; char cMBBufferS[4000];	cMBBufferS[3999] = 0;
			char* pSelectionResult = new(char[lenRis]);;
			CString maxW, maxH; maxW.Format(_T("%.0f"), 10000); maxH.Format(_T("%.0f"), 10000);
			CString tollStr; tollStr.Format(_T("%.2f"), 20); tollStr.Replace(_T("."), _T(","));

			sprintf_s(cMBBufferS, 4000, "%.0f;%.0f;%s;%.2f;%s;%s;%s;;ebmpapst;0;F;40000;F", port/3600.0, pTot, tollStr, 1.2, maxW, maxH, tollStr);
			num = SEARCH_PRODUCTS(cMBBufferS, &pSelectionResult);
            int err = 0; 
            return  err;
        }; 

		bool CalculateFan::LoadEBMDll()
		{

			USES_CONVERSION;
			CString temp = ("\\data\\plug_fans");
			char path33[MAX_PATH + 1];


			//typedef int (__stdcall *PEBMPAPSTFAN_FNCT1)( char** buffer );
			typedef int(__stdcall* PEBMPAPSTFAN_FNCT1)(char** buffer);
			typedef int(__stdcall* PEBMPAPSTFAN_FNCT11)(char* buffer);
			PEBMPAPSTFAN_FNCT1	SET_XML_PATH = NULL;
			// Load dll
			g_EbmPapstFanDLL = LoadLibrary(temp + _T("\\EbmPapstFan.dll"));
			if (!g_EbmPapstFanDLL)
			{
				// Load dll
				g_EbmPapstFanDLL = LoadLibrary("data\\EbmPapstFan.dll");
				if (!g_EbmPapstFanDLL)
				{
					return false;
				}
			}

			GET_DLL_VERSION = (PEBMPAPSTFAN_FNCT2)GetProcAddress(g_EbmPapstFanDLL, "GetDLLVersion");

			SET_XML_PATH = (PEBMPAPSTFAN_FNCT1)GetProcAddress(g_EbmPapstFanDLL, "SET_XML_PATH");
			if (!SET_XML_PATH)
			{
				return false;
			}

			SET_XML_PATH_WS = (PEBMPAPSTFAN_FNCT20)GetProcAddress(g_EbmPapstFanDLL, "SET_XML_PATH_WS");
			if (!SET_XML_PATH_WS)
			{
				return false;
			}

			int version = GET_DLL_VERSION();
	
			temp = _T("d:\\data\\plug_fans\\");  

			A2W(temp);
			short err = SET_XML_PATH_WS(A2W(temp));
			

			//SAFE_ARRAY_DELETE(cMBBuffer);		//MV 10.06.2014. Non serve per la DLL versione 3.0.1.0

			if (err < 0)
			{
				return false;
			}

			GET_PRODUCTS_PC = (PEBMPAPSTFAN_FNCT1)GetProcAddress(g_EbmPapstFanDLL, "GET_PRODUCTS_PC");
			if (!GET_PRODUCTS_PC)
			{
			
				return false;
			}
			{
				char cMBBuffer2[4001];
				ZeroMemory(&cMBBuffer2, 4001 * sizeof(char));	char* pBuffer = &cMBBuffer2[0];
				int num2 = GET_PRODUCTS_PC(&pBuffer);
				m_fanEBM = A2W(cMBBuffer2);
			}

			GET_CALCULATION_FAN_ALONE_PC = (PEBMPAPSTFAN_FNCT6)GetProcAddress(g_EbmPapstFanDLL, "GET_CALCULATION_FAN_ALONE_PC");
			if (!GET_CALCULATION_FAN_ALONE_PC)
			{
				
				return false;
			}
			GET_TECHNICAL_DATA_PC = (PEBMPAPSTFAN_FNCT6)GetProcAddress(g_EbmPapstFanDLL, "GET_TECHNICAL_DATA_PC");
			if (!GET_TECHNICAL_DATA_PC)
			{
				
				return false;
			}
	
			SEARCH_PRODUCTS = (PEBMPAPSTFAN_FNCT6)GetProcAddress(g_EbmPapstFanDLL, "SEARCH_PRODUCTS");

			if (!SEARCH_PRODUCTS)
			{
				
				return false;
			}

			GET_CALCULATION_FANMOTOR = (PEBMPAPSTFAN_FNCT7)GetProcAddress(g_EbmPapstFanDLL, "GET_CALCULATION_FANMOTOR");
			if (!GET_CALCULATION_FANMOTOR)
			{
				
				return false;
			}

			GET_GRAPH_PFA_PC = (PEBMPAPSTFAN_FNCT6)GetProcAddress(g_EbmPapstFanDLL, "GET_GRAPH_PFA_PC");
			if (!GET_GRAPH_PFA_PC)
			{
				
				return false;
			}

			GET_FAN_CURVE = (PEBMPAPSTFAN_FNCT18)GetProcAddress(g_EbmPapstFanDLL, "GET_FAN_CURVE");
			if (!GET_FAN_CURVE)
			{
				
				return false;
			}
			CALCULATE_FANGRID = (PEBMPAPSTFAN_FNCT6)GetProcAddress(g_EbmPapstFanDLL, "CALCULATE_FANGRID");
			if (!CALCULATE_FANGRID)
			{
				
				return false;
			}

			GET_GRAPH_N_PC = (PEBMPAPSTFAN_FNCT6)GetProcAddress(g_EbmPapstFanDLL, "GET_GRAPH_N_PC");
			GET_GRAPH_ETA_PC = (PEBMPAPSTFAN_FNCT6)GetProcAddress(g_EbmPapstFanDLL, "GET_GRAPH_ETA_PC");
			GET_GRAPH_POWER_PC = (PEBMPAPSTFAN_FNCT6)GetProcAddress(g_EbmPapstFanDLL, "GET_GRAPH_POWER_PC");
			GET_GRAPH_SOUND_PC = (PEBMPAPSTFAN_FNCT6)GetProcAddress(g_EbmPapstFanDLL, "GET_GRAPH_SOUND_PC");
			GET_STANDARDS_FANMOTOR = (PEBMPAPSTFAN_FNCT17)GetProcAddress(g_EbmPapstFanDLL, "GET_STANDARDS_FANMOTOR");;
			return true;
		}


