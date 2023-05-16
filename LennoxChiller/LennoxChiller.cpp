#include "pch.h"
#include "LennoxChiller.h"

////////////////////////////////////////////////////////////////////////////////////////////////

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
PEBMPAPSTFAN_FNCT30 SET_DECIMALSEPARATOR = NULL;
 
using namespace LennoxRooftop;

CString g_elencoEBMJson;

double Round(double val, int dec = 0)
{
	long a;
	double esp = 1.0, valDec = 0;
	int pos = 0;
	for (int i = 1; i <= dec; esp *= 10, i++);

	int meno = val < 0 ? -1.0 : 1.0;
	val = fabs(val);
	CString f, f1;
	f1.Format(_T("%d"), dec);
	f.Format(_T("%s%sf"), _T("%."), f1);
	f1 = f;
	f.Format(f, val);
	if ((pos = f.Find(_T("."))) != -1)
	{
		double cip = _tstof(f.Mid(pos + 1)) / pow(10., dec);
		double b = _tstof(f) + 0.001;
		a = (long)(b - cip);
		valDec = _tstof(f.Mid(pos + 1)) / pow(10., dec);
		//a = val - valDec;

	}
	else
	{
		a = _tstol(f);
		valDec = 0;
	}
	//a*=pow(10,dec);
	double b = val * pow(10., dec);
	//a = (unsigned long) (b);


	if (val < 0)
	{
		if (b + (a + valDec) * pow(10., dec) < -0.5)
		{
			return (a + valDec);
		}
		else
			return valDec + a - 1.0 / pow(10., dec);
	}
	else
	{
		if (meno < 0)
		{
			return meno * (a + valDec);
		}
		else
		{
			if ((b - (a + valDec) * pow(10., dec)) > 0.5 && meno > 0)
			{
				return 1.0 * a + valDec + 1.0 / pow(10., dec);
			}
			else
				return 1.0 * a + valDec;
		}
	}

}
CString ExtractString(const CString& str, int* pos, const CString& c2seek = _T(";"), const CString& start = _T(""))
{

	if (*pos >= str.GetLength())
		return _T("");

	int pos1 = str.Find(c2seek, *pos);
	if (!start.IsEmpty())
	{
		*pos = str.Find(start, *pos) + 1;
	}
	if (pos1 == -1)
	{
		if (*pos == 0)
		{
			*pos = str.GetLength() + 1;
			return str;
		}
		else
		{
			pos1 = str.GetLength() + 1;
		}

	}
	CString ris = str.Mid(*pos, pos1 - *pos);
	*pos = pos1 + 1;
	return ris;
};

void _cdecl OLEDBErrorMessageBox(LPCTSTR lpszFormat, ...)
{
	va_list args;
	va_start(args, lpszFormat);

	int nBuf;

#ifdef _UNICODE
	WCHAR szBuffer[2048];
#else
	CHAR szBuffer[2048];
#endif



	nBuf = _vsntprintf_s(szBuffer, sizeof(szBuffer), lpszFormat, args);
	ATLASSERT(nBuf < sizeof(szBuffer)); //Output truncated as it was > sizeof(szBuffer)

	::MessageBox(NULL, szBuffer, _T("OLE DB Error Message"), MB_OK);
	va_end(args);
}

void DisplayOLEDBErrorRecords(HRESULT hrErr = S_OK)
{
	CDBErrorInfo ErrorInfo;
	ULONG        cRecords;
	HRESULT      hr;
	ULONG        i;
	CComBSTR     bstrDesc, bstrHelpFile, bstrSource;
	GUID         guid;
	DWORD        dwHelpContext;
	WCHAR        wszGuid[40];
	USES_CONVERSION;

	// If the user passed in an HRESULT then trace it
	if (hrErr != S_OK)
		OLEDBErrorMessageBox(_T("OLE DB Error Record dump for hr = 0x%x\n"), hrErr);

	LCID lcLocale = GetSystemDefaultLCID();

	hr = ErrorInfo.GetErrorRecords(&cRecords);
	if (FAILED(hr) && ErrorInfo.m_spErrorInfo == NULL)
	{
		OLEDBErrorMessageBox(_T("No OLE DB Error Information found: hr = 0x%x\n"), hr);
	}
	else
	{
		for (i = 0; i < cRecords; i++)
		{
			hr = ErrorInfo.GetAllErrorInfo(i, lcLocale, &bstrDesc, &bstrSource, &guid,
				&dwHelpContext, &bstrHelpFile);
			if (FAILED(hr))
			{
				OLEDBErrorMessageBox(
					_T("OLE DB Error Record dump retrieval failed: hr = 0x%x\n"), hr);
				return;
			}
			StringFromGUID2(guid, wszGuid, sizeof(wszGuid) / sizeof(WCHAR));
			OLEDBErrorMessageBox(
				_T("Source:\"%s\"\nDescription:\"%s\"\nHelp File:\"%s\"\nHelp Context:%4d\nGUID:%s\n"),
				OLE2T(bstrSource), OLE2T(bstrDesc), OLE2T(bstrHelpFile), dwHelpContext, OLE2T(wszGuid));
			bstrSource.Empty();
			bstrDesc.Empty();
			bstrHelpFile.Empty();
		}
	}
}


String^ Rooftop::GetFanPerformance(String^ jSONIN)
{
    std::string str = marshal_as<std::string>(jSONIN);

    // lettura dei risultati
    Document doc;
    doc.Parse(str.c_str());
    CString model = doc["modelid"].GetString();
    int supplier = doc["supplierid"].GetInt();
    double port = doc["airflow"].GetDouble()/3600.0;
	int fantype = doc["fantype"].GetInt();
	CString fanopt = doc["fanoption"].GetString();
    double pTot = doc["optionsdp"].GetDouble();
	double dens = doc["density"].GetDouble(); 
	double temperature = doc["temperature"].GetDouble();
	int iqngn = doc["iqngn"].GetInt();

	int errorcode = 0;
	StringBuffer s;
	CString fanmodel = "";
	CString fanname = "";
	std::string str1 = "";
	std::string filter = "";
	CString casingcode = "BAUN";
	CGenTableRecord option;
	double a = 0, b = 0, c = 0, d = 0, e = 0;

	//recupero le informazioni nel database interno LENNOX in base all'idmodello richiesto
	//ritorna un json creato con i dati di ritorno del recordset
	String^ JSONmodelspecification = SearchModel(model, fanopt, port*3600.0, fantype);
	if (CString(JSONmodelspecification).IsEmpty())
	{
		errorcode = 2;
		goto exit;
	}
	str1 = marshal_as<std::string>(JSONmodelspecification);
	doc.Parse(str1.c_str());
	Value& responseObj = doc["FANCODE"];
	fanmodel = responseObj.GetString();
	int nfan = doc["FANNUMBER"].GetInt();
	 fanname = doc["FANMODEL"].GetString();
	
	//da recuperare in tabella ALTEC_definition?? gf 21-04-23
	double maxWidth = 10000; //doc["maxWidth"].GetDouble();
	double maxHeigth = 10000;// doc["maxHeigth"].GetDouble();
	


	if (fanmodel.IsEmpty() || nfan <= 0)
	{
		errorcode = 1;
		goto exit;
	}
	
	
	g_CoeffPdc.AddFilterField("Nomcomm", "=", model, filter);
	g_CoeffPdc.AddFilterField("Flow_stream", "=", 1, filter);
	g_CoeffPdc.AddFilterField("SupplyDP_code", "=", casingcode, filter);
	//ricerco il modello e mandato o ripresa
	option = g_CoeffPdc.Lookup(filter);
	
	option.GetColumn("a", a);
	option.GetColumn("b", b);
	option.GetColumn("c", c);
	option.GetColumn("d", d);
	option.GetColumn("e", e);
	double pdcCasing = a * pow(port, 4) + b * pow(port, 3) + c * pow(port, 2) + d * port + e;
	pdcCasing = Round(pdcCasing, 1);
	if (pdcCasing <= 0)
		errorcode = 3;
	if (pdcCasing > 300)
		errorcode = 3;
	

	double nfunzi = 0;
	double kwfunz = 0;
	double phifun = 0;//corrente ampere
	double etasta = 0;
	double ufunzi = 0;//voltaggio controllo
	double lw5Afu = 0; // efficienza motore
	double lw3Afu = 0; //potenza sonora db
	double lw7Afu = 0;//potenza sonora db ingr
	double lw6Afu = 0;//potenza sonora db out
	double l7w006 = 0;//potenza sonora db ingr
	double l7w012 = 0;//potenza sonora db ingr
	double l7w025 = 0;//potenza sonora db ingr
	double l7w050 = 0;//potenza sonora db ingr
	double l7w100 = 0;//potenza sonora db ingr
	double l7w200 = 0;//potenza sonora db ingr
	double l7w400 = 0;//potenza sonora db ingr
	double l7w800 = 0;//potenza sonora db ingr
	double l6w006 = 0;//potenza sonora db out
	double l6w012 = 0;//potenza sonora db out
	double l6w025 = 0;//potenza sonora db out
	double l6w050 = 0;//potenza sonora db out
	double l6w100 = 0;//potenza sonora db out
	double l6w200 = 0;//potenza sonora db out
	double l6w400 = 0;//potenza sonora db out
	double l6w800 = 0;//potenza sonora db out
	double ptotfu = 0;//pressione totale
	double l5A050 = 0;
	double pdynfu = 0;
	double pstafu = 0;
	double m_kwmaxv = 0;
	double m_l4A800 = 0;
	double m_l4A025 = 0;
	double kfactor = 0;
	double m_l5A100 = 0;
	double m_l4A100 = 230;
	double m_l4A200 = 1;
	double peso = 0;
	double maxport = 0;
	double maxpd = 0;
	double minport = 0;
	double minpd = 0;
	double sfp = 0;
	double efftot = 0;
	double effsta = 0;
	if (supplier == 1) // EBM
	{

		

		CString request;
		INT num = 0;
		double press = pTot;
		char cMBBuffer[4000];
		char cMBOut[4001]; ZeroMemory(cMBOut, 4001 * sizeof(char)); char* pRis = &cMBOut[0];
		int err = 999;

		CString ventRis, ventRis1;
		sprintf_s(cMBBuffer, 4000, "%s", fanmodel.GetString());

		err = GET_TECHNICAL_DATA_PC(cMBBuffer, &pRis);	// MV 10.06.2014. Per utilizzare la nuova DLL 3.0.1.0
		errorcode = err * 100;
		CString ventRis2 = pRis;
		CString temp1;

		if (err == 0 || err == -8)
		{
			int pos2 = 0;
			ventRis2.Replace(_T(","), _T("."));
			temp1 = ExtractString(ventRis2, &pos2, _T(";"));
			// potenza motore
			m_kwmaxv = _tstof(ExtractString(ventRis2, &pos2, _T(";")));
			m_kwmaxv /= 1000;

			//corrente motore
			m_l4A800 = _tstof(ExtractString(ventRis2, &pos2, _T(";"))); // current draw
			m_l4A025 = _tstof(ExtractString(ventRis2, &pos2, _T(";"))); // Nominal fan data speed



			int pos3 = 9;
			CString pr = ExtractString(ventRis2, &pos3, _T(";ERP"), _T(";IP "));
			if (pr.Find(_T("54")) > -1)
				m_l5A100 = 54;
			else
				m_l5A100 = 55;
			for (int i = 0; i < 26; i++)
				(ExtractString(ventRis2, &pos2, _T(";")));
			kfactor = _tstof(ExtractString(ventRis2, &pos2, _T(";")));



			char* SIZE = new char[2001]; char* TYP = new char[2001];	char* ISOCLASS = new char[2001];	char* PROTECTION = new char[2001];
			SIZE[0] = TYP[0] = ISOCLASS[0] = PROTECTION[0] = 0;
			int z1 = 0, z2 = 0;	double output[52]; double output2[51];
			double input[13];
			int s1 = 0, s2 = 0;
			err = 0;

			err = GET_STANDARDS_FANMOTOR(cMBBuffer, SIZE, TYP, ISOCLASS, PROTECTION, z1, z2, &output[0]);	// MV 10.06.2014. Per utilizzare la nuova DLL 3.0.1.0
			errorcode = err * 1000;
			int pos = 0;



			if (err == 0 || err == -8)
			{
				m_l4A100 = output[6];		// Voltaggio
				m_l4A200 = output[7];		//Numero di fasi
				maxport = output[2];		// max airflow m3/h
				maxpd = output[4];		// pf max 
				peso = output[15];		// weight FS 29.02.2020

			}

			SAFE_ARRAY_DELETE(SIZE);
			SAFE_ARRAY_DELETE(TYP);
			SAFE_ARRAY_DELETE(ISOCLASS);
			SAFE_ARRAY_DELETE(PROTECTION);

			//cerco la massima pressione per la portata passata
			if (port > 0)
			{
				double port1 = port / nfan;
				double pressh = maxpd;
				if (fantype == 1 || fantype == 2)
					pressh -= pdcCasing;

				double pressl = 0;
				double presss = (pressl + pressh) / 2.0;

				int errore = 0;
				long cont = 0;
				do
				{
					sprintf_s(cMBBuffer, 4000, "%s;0;0;%.4f;;%.2f;%.4f;%.4f;%.2f;%.2f;ebmpapst;0;F;4000", fanmodel.GetString(), dens, temperature, presss, port, maxWidth, maxHeigth);
					char cMBOut[4001]; ZeroMemory(cMBOut, 4001 * sizeof(char)); char* pRisr = &cMBOut[0];

					errore = GET_CALCULATION_FAN_ALONE_PC(&cMBBuffer[0], &pRisr);
					ventRis = cMBOut;
					if (errore == 0)
					{
						pressl = presss;
						presss = (presss + pressh) / 2.0;
					}
					else
					{
						pressh = presss;
						presss = (presss + pressl) / 2.0;
					}
					cont++;
				} while (pressh - pressl > 1 && cont < 1000);
				maxpd = Round(presss, 0);
			}
		}
		if (pTot > 0 && port > 0)
		{
			bool fangrid = false;
			if (nfan > 1)
			{
				short backFlow = 1;
				short oneSizePerGroup = 0;
				short filterWH = 0;
				short ignoreTooLarge = 1;
				short json = 1;
				long hoursperyear = 100000;
				double portataS = port*3600.0;
				short numRedFan = 0;
				INT num = 0; char cMBBufferS[4000];	cMBBufferS[3999] = 0;
				sprintf_s(cMBBufferS, 4000, "%.0f;%.0f;%d;;;;;;;;;;;;;%d;%.0f;%.0f;%s;%d;%d;%d;;%s;%s;%s;%.2f;0;%.2f;%s;%d;%s;;;2;0;", 
					portataS,
					pTot,
					hoursperyear,
					numRedFan,
					maxWidth,
					maxHeigth,
					backFlow == 1 ? _T("T") : _T("F"),
					nfan, // 20
					nfan, // 21
					nfan, //22
					oneSizePerGroup == 1 ? _T("T") : _T("F"),// 24
					filterWH == 1 ? _T("T") : _T("F"),// 25
					ignoreTooLarge == 1 ? _T("T") : _T("F"),// 26
					dens, temperature,
					json == 1 ? _T("T") : _T("F"),// 26
					lenRis,
					"");
				char* pSelectionResult = new(char[lenRis]);
				if (pSelectionResult)
				{
					memset(pSelectionResult, 0, lenRis * sizeof(char));
					num = CALCULATE_FANGRID(cMBBufferS, &pSelectionResult);
					if (num > 0)
					{
						ventRis1 = pSelectionResult;
						
						std::string strEBMFANWALL = ventRis1;
						// lettura dei risultati
						Document docEbm;
						docEbm.Parse(strEBMFANWALL.c_str());
						CString chiave = fanmodel + " " + fanname;
						Value& responseObj = docEbm[chiave.GetString()];
						if (responseObj.IsObject())
						{
							Value& responseObj1 = responseObj["OP_0"];
							ASSERT(responseObj1.IsArray());
							if (responseObj1.IsArray())
							{
								Value& performance = responseObj1[0]; //prendo solo la posizione 0
								nfunzi = _tstof(performance["n [1/min]"].GetString());
								
								kwfunz = _tstof(performance["P_ed [W]"].GetString())/1000.0;
								etasta = _tstof(performance["ETA_esd [%]"].GetString());
								ufunzi = _tstof(performance["Uctrl [V]"].GetString());//voltaggio controllo
								ptotfu = _tstof(performance["PTOT [Pa]"].GetString());//pressione totale
								efftot = _tstof(performance["ETA_ed [%]"].GetString()); //eta R
								effsta = _tstof(performance["ETA_esd [%]"].GetString()); //eta SR
								sfp = _tstof(performance["SFP [kW/(m^3/s)]"].GetString()); 
								pdynfu = ptotfu - pTot;
								pstafu = pTot;

								Value& responseObj2 = performance["Lwss"];
								lw7Afu = _tstof(responseObj2["LwA"].GetString());
								l7w006 = _tstof(responseObj2["63"].GetString());
								l7w012 = _tstof(responseObj2["125"].GetString());
								l7w025 = _tstof(responseObj2["250"].GetString());
								l7w050 = _tstof(responseObj2["500"].GetString());
								l7w100 = _tstof(responseObj2["1000"].GetString());
								l7w200 = _tstof(responseObj2["2000"].GetString());
								l7w400 = _tstof(responseObj2["4000"].GetString());
								l7w800 = _tstof(responseObj2["8000"].GetString());

								responseObj2 = performance["Lwps"];

								lw6Afu = _tstof(responseObj2["LwA"].GetString());
								l6w006 = _tstof(responseObj2["63"].GetString());
								l6w012 = _tstof(responseObj2["125"].GetString());
								l6w025 = _tstof(responseObj2["250"].GetString());
								l6w050 = _tstof(responseObj2["500"].GetString());
								l6w100 = _tstof(responseObj2["1000"].GetString());
								l6w200 = _tstof(responseObj2["2000"].GetString());
								l6w400 = _tstof(responseObj2["4000"].GetString());
								l6w800 = _tstof(responseObj2["8000"].GetString());

								fangrid = true;
							}
							else
							{
								errorcode = 93;
							}
						}
						else
						{
							errorcode = 92;
						}
					}
					else
					{
						errorcode = 91;
					}
				}
				else
				{
					errorcode = 90;
				}
				delete[] pSelectionResult;
			}
			

			if (!fangrid)
			{
				double port1 = port / nfan;

				sprintf_s(cMBBuffer, 4000, "%s;0;0;%.4f;;%.2f;%.4f;%.4f;%.2f;%.2f;ebmpapst;0;F;4000", fanmodel.GetString(), dens, temperature, press, port1, maxWidth, maxHeigth);


				errorcode = err = GET_CALCULATION_FAN_ALONE_PC(&cMBBuffer[0], &pRis);	// MV 10.06.2014. Per versione nuova DLL 3.0.1.0
				if (err == 0 || err == -8 || err == -7)
				{
					ventRis = cMBOut;
					int pos1 = 0;
					ventRis.Replace(_T(","), _T("."));
					nfunzi = _tstof(ExtractString(ventRis, &pos1, _T(";")));
					kwfunz = _tstof(ExtractString(ventRis, &pos1, _T(";"))) / 1000.0 * nfan;
					phifun = _tstof(ExtractString(ventRis, &pos1, _T(";")));//corrente ampere
					efftot = _tstof(ExtractString(ventRis, &pos1, _T(";")));

					ufunzi = _tstof(ExtractString(ventRis, &pos1, _T(";")));//voltaggio controllo
					ExtractString(ventRis, &pos1, _T(";")); // momento di inerzia
					lw5Afu = _tstof(ExtractString(ventRis, &pos1, _T(";"))); // efficienza motore
					lw3Afu = _tstof(ExtractString(ventRis, &pos1, _T(";"))); //potenza sonora db
					lw7Afu = _tstof(ExtractString(ventRis, &pos1, _T(";")));//potenza sonora db ingr
					lw6Afu = _tstof(ExtractString(ventRis, &pos1, _T(";")));//potenza sonora db out
					l7w006 = _tstof(ExtractString(ventRis, &pos1, _T(";")));//potenza sonora db ingr
					l7w012 = _tstof(ExtractString(ventRis, &pos1, _T(";")));//potenza sonora db ingr
					l7w025 = _tstof(ExtractString(ventRis, &pos1, _T(";")));//potenza sonora db ingr
					l7w050 = _tstof(ExtractString(ventRis, &pos1, _T(";")));//potenza sonora db ingr
					l7w100 = _tstof(ExtractString(ventRis, &pos1, _T(";")));//potenza sonora db ingr
					l7w200 = _tstof(ExtractString(ventRis, &pos1, _T(";")));//potenza sonora db ingr
					l7w400 = _tstof(ExtractString(ventRis, &pos1, _T(";")));//potenza sonora db ingr
					l7w800 = _tstof(ExtractString(ventRis, &pos1, _T(";")));//potenza sonora db ingr
					l6w006 = _tstof(ExtractString(ventRis, &pos1, _T(";")));//potenza sonora db out
					l6w012 = _tstof(ExtractString(ventRis, &pos1, _T(";")));//potenza sonora db out
					l6w025 = _tstof(ExtractString(ventRis, &pos1, _T(";")));//potenza sonora db out
					l6w050 = _tstof(ExtractString(ventRis, &pos1, _T(";")));//potenza sonora db out
					l6w100 = _tstof(ExtractString(ventRis, &pos1, _T(";")));//potenza sonora db out
					l6w200 = _tstof(ExtractString(ventRis, &pos1, _T(";")));//potenza sonora db out
					l6w400 = _tstof(ExtractString(ventRis, &pos1, _T(";")));//potenza sonora db out
					l6w800 = _tstof(ExtractString(ventRis, &pos1, _T(";")));//potenza sonora db out
					ptotfu = _tstof(ExtractString(ventRis, &pos1, _T(";")));//pressione totale
					CString temp;
					temp = ExtractString(ventRis, &pos1, _T(";")); //pressione statica
					temp = ExtractString(ventRis, &pos1, _T(";")); //portata
					effsta = _tstof(ExtractString(ventRis, &pos1, _T(";"))); //rendimento statico
					temp = ExtractString(ventRis, &pos1, _T(";")); //Po Watt
					temp = ExtractString(ventRis, &pos1, _T(";")); //eta R
					temp = ExtractString(ventRis, &pos1, _T(";")); //eta SR
					temp = ExtractString(ventRis, &pos1, _T(";")); //(safety factor rpm [%])
					sfp = _tstof(ExtractString(ventRis, &pos1, _T(";"))); //(specific fan power[kW / (m3 / s)])
					l5A050 = _tstof(ExtractString(ventRis, &pos1, _T(";")));//(pressure inlet nozzle [Pa] (rough estimate))
					temp = ExtractString(ventRis, &pos1, _T(";")); //(safety factor rpm [%])
					//pData->m_l5A050 = _tstof(ExtractString(ventRis, &pos1, _T(";")));//(pressure inlet nozzle [Pa] (rough estimate))
					temp = ExtractString(ventRis, &pos1, _T(";")); //(pd[Pa])
					//temp = ExtractString(ventRis, &pos1, _T(";")); //Pressure loss with respect to installation space [Pa])
					pdynfu = ptotfu - pTot;
					pstafu = pTot;

				}
			}
		}
	}
	else
	{
		errorcode = 11;
	}
exit:
	//json output:
	Writer<StringBuffer> writer(s);
	writer.StartObject();
		writer.Key("result"); 
		writer.StartObject();
			writer.Key("performance");
			writer.StartObject();
			if (pTot > 0 && port > 0)
			{
				writer.Key("motorabspower"); writer.Double(0.0);
				writer.Key("motorelecpower"); writer.Double(kwfunz);
				writer.Key("rpm"); writer.Double(nfunzi);
				writer.Key("sfp_class"); writer.Int(GetSFPClass(sfp*1000));
				writer.Key("sfp_value"); writer.Double(sfp);
				writer.Key("supplynoise"); writer.Double(lw3Afu);
				writer.Key("effsta"); writer.Double(effsta);
				writer.Key("efftot"); writer.Double(efftot);
			}
			writer.EndObject();
			writer.Key("configuration");
			writer.StartObject();
			//if (errorcode == 0)
			{
				writer.Key("fanname"); writer.String(fanmodel.GetString());
				double formula = 0;
				writer.Key("fanfactor"); writer.Double(formula);
				writer.Key("keyfactor"); writer.Double(kfactor); // kfactor
				writer.Key("weight"); writer.Double(peso);
				writer.Key("minairflow"); writer.Double(minport);
				writer.Key("maxaiflow"); writer.Double(Round(maxport, 2));
				writer.Key("mindp"); writer.Double(minpd);
				writer.Key("maxdp"); writer.Double(maxpd);
			}
			writer.EndObject();
			writer.Key("noise");
			writer.StartObject();
			if (pTot > 0 && port > 0)
			{
				CString in, out;
				in.Format("%.0f;%.0f;%.0f;%.0f;%.0f;%.0f;%.0f;%.0f", l7w006, l7w012, l7w025, l7w050, l7w100, l7w200, l7w400, l7w800);
				out.Format("%.0f;%.0f;%.0f;%.0f;%.0f;%.0f;%.0f;%.0f", l6w006, l6w012, l6w025, l6w050, l6w100, l6w200, l6w400, l6w800);
				writer.Key("inlet"); writer.String(in);
				writer.Key("outlet"); writer.String(out);
			}
			writer.EndObject();
		writer.EndObject();
		writer.Key("erroid"); writer.Int(errorcode);
		writer.Key("version"); writer.String(VERSION);
	writer.EndObject();


	String^ JSONout;
	JSONout = gcnew String(s.GetString());
    return  JSONout;
}
String^ Rooftop::GetModelPerformance(String^ jSONIN)
{
	String^ err;
	return  err;
} 

String^ Rooftop::GetWaterCoilPerformance(String^ jSONIN)
{

	std::string str = marshal_as<std::string>(jSONIN);
	Document doc,doc1;
	doc.Parse(str.c_str());
	int errorcode = 0;

	//lettura json input
	Value& conf = doc["configuration"];
	int iqngn = doc["iqngn"].GetInt();
	CString sigla = "";
	short supp = 0, coiltype = 0;
	if (conf.IsObject())
	{
		CString model = conf["modelid"].GetString();
		String^ JSONmodelspecification = SearchModel(model,"",0,1);
		if (CString(JSONmodelspecification).IsEmpty())
		{
			errorcode = 1;
		}
		std::string str1 = "";
		str1 = marshal_as<std::string>(JSONmodelspecification);
		doc1.Parse(str1.c_str());
		sigla = doc1["COIL"].GetString();
		supp = conf["supplierid"].GetInt();
		coiltype = conf["coiltype"].GetInt();
	}

	if (sigla.IsEmpty())
		errorcode = 2;

	double airflow = 0;
	double airdb = 0;
	double airwb = 0;
	double waterin = 0;
	double waterout = 0;
	double waterflow = 0;
	int fluidtype = 0;
	int glycoletype = 0;
	double glycoleperc = 0;

	Value& responseObj = doc["conditions"];
	//if (responseObj.IsArray()) // nella documentazione era Array, ma se ci fossero n condizioni in ingresso, dovrei aver en condizioni in uscita, e non ci sono.
	if (responseObj.IsObject())
	{
		//for (int i = 0; i < responseObj.GetArray().Size(); i++)
		//{
		//	Value& opt = responseObj[i];
		Value& opt = responseObj;
			if (opt.IsObject())
			{
				airflow = opt["airflow"].GetDouble();
				airdb = opt["airdb"].GetDouble();
				airwb = opt["airwb"].GetDouble();
				waterin = opt["waterin"].GetDouble();
				waterout = opt["waterout"].GetDouble();
				waterflow = opt["waterflow"].GetDouble();
				fluidtype = opt["fluidtype"].GetInt();
				glycoletype = opt["glycoletype"].GetInt();
				glycoleperc = opt["glycoleperc"].GetDouble();


			}

		//}
	}
	if (airflow == 0)
		errorcode = 3;
	
	//risultati
	double capacity = 0, outdb = 0, outwb = 0, outhumrel = 0, airpressuredp = 0, waterinlet = 0, wateroutlet = 0, waterpressuredp = 0;
	double valvepressuredp = 0, weight = 0, overalllength = 0, overallheight = 0;
	CString coilname = "";

	if (errorcode == 0)
	{

		leelcoilsDLL::Calculation calc;
		std::string output;
		std::vector<std::string> single_result;
		std::string strJSON;
		/*JObject JO;
		JObject JOmeasure;
		JObject JOres;


		JO.Add("PARAM3", 1); // calculation - fluid
		JO.Add("PARAM4", true); // verify coil

		// coil
		JO.Add("PARAM9", -999999); // max fin height
		JO.Add("PARAM26", 84); // number tubes
		JO.Add("PARAM57", -999999); // fin height
		JO.Add("PARAM10", 900); // length fin block
		JO.Add("PARAM70", 0); // Number of tubes not used in circuit
		JO.Add("PARAM25", 6); // number rows
		JO.Add("PARAM29", 2.12); // spacing
		JO.Add("PARAM22", 1); // number of circuits
		JO.Add("PARAM41", 1); // Type of fluid arrangement (1 = counterflow ; 2 = Parallel flow)
		JO.Add("PARAM27", 31); // number of injections (total Number of Tubes feeded from headers)
		JO.Add("PARAM33", "G"); // surface
		JO.Add("PARAM20", "4S6"); // pattern
		JO.Add("OP_01", 1); // tube material
		JO.Add("OP_09", 2); // tube thickness
		JO.Add("OP_03", 1); // fin thickness
		JO.Add("OP_02", 1); // fin material

		// air type input
		JO.Add("PARAM49", 7); // air conditions type
		JO.Add("PARAM50", -999999); // Moist air density
		JO.Add("PARAM51", -999999); // barometric pressure
		JO.Add("PARAM52", 0); // altitude

		// air temperatures
		JO.Add("PARAM36", 42); // Entering Air Temperature Dry Bulb
		JO.Add("PARAM37", -999999); // Entering Air Temperature Wet Bulb
		JO.Add("PARAM38", -999999); // Entering Air Absolute Humidity
		JO.Add("PARAM39", 30); // Entering Air Relative Humidity

		// air flow
		JO.Add("PARAM54", 18612); // Volumetric Air Flow
		JO.Add("PARAM46", -999999); // Massic Air Flow - Moist air
		JO.Add("PARAM34", -999999); // Air velocity
		JO.Add("PARAM97", -999999); // fan select
		JO.Add("PARAM99", 0); // fan direction

		//fluid
		JO.Add("PARAM43", 1); //type fluid

			JO.Add("PARAM89", -999999); //Superheated water Pressure
			JO.Add("PARAM90", -999999); //Custom fluid density
			JO.Add("PARAM91", -999999); //Custom fluid viscosity
			JO.Add("PARAM92", -999999); //Custom fluid concuctivity
			JO.Add("PARAM93", -999999); //Custom fluid specific heat

			JO.Add("PARAM40", 6); //Entering fluid temperature
			JO.Add("PARAM45", 12); //Leaving fluid temperature
			JO.Add("PARAM55", -999999); //Volumetric Fluid Flow
			JO.Add("PARAM47", -999999); //Mass Fluid Flow

			JO.Add("PARAM95", -999999); //diameter of header

			//duty
			JO.Add("PARAM87", 0); //Oversurface Requested

		Object JOO;
		JO.ToObject(Object);
		String^ strJSONCoil = Newtonsoft::Json::JsonConvert::SerializeObject();*/
		StringBuffer s;
		Writer<StringBuffer> writer(s);
		writer.StartObject();

		writer.Key("PARAM3"); writer.Int(1); // calculation - fluid coil 
		writer.Key("PARAM4"); writer.String("True"); // verify coil

		// coil
		short ntubi = atoi(sigla.Left(2));
		short pos = 0;

		pos = sigla.Find("-",pos);
		short ranghi = atoi(sigla.Mid(pos+1,1));
		pos = sigla.Find("-", pos);
		double lencoil = atof(sigla.Mid(pos + 1, 4));
		double pal = atof(sigla.Mid(pos + 6, 4));
		int ncircuiti = atof(sigla.Mid(pos + 11, 2)); // quà ci sarebbe da capire se la sigla 2*ncircuiti o altro...


		writer.Key("PARAM9"); writer.Int(-999999); // max fin height
		writer.Key("PARAM26"); writer.Int(ntubi); // number tubes
		writer.Key("PARAM57"); writer.Int(-999999); // fin height
		writer.Key("PARAM10"); writer.Double(lencoil); // length fin block
		writer.Key("PARAM70"); writer.Int(0); // Number of tubes not used in circuit
		writer.Key("PARAM25"); writer.Int(ranghi); // number rows
		writer.Key("PARAM29"); writer.Double(pal); // spacing
		writer.Key("PARAM22"); writer.Int(ncircuiti); // number of circuits
		writer.Key("PARAM41"); writer.Int(1); // Type of fluid arrangement (1 = counterflow ; 2 = Parallel flow)
		writer.Key("PARAM27"); writer.Int(31); // number of injections (total Number of Tubes feeded from headers)
		writer.Key("PARAM33"); writer.String("G"); // surface
		writer.Key("PARAM20"); writer.String("4S6"); // pattern
		writer.Key("OP_01"); writer.Int(1); // tube material
		writer.Key("OP_09"); writer.Int(2); // tube thickness
		writer.Key("OP_03"); writer.Int(1); // fin thickness
		writer.Key("OP_02"); writer.Int(1); // fin material

		// air type input
		writer.Key("PARAM49"); writer.Int(1); // air conditions type
		writer.Key("PARAM50"); writer.Int(-999999); // Moist air density
		writer.Key("PARAM51"); writer.Int(-999999); // barometric pressure
		writer.Key("PARAM52"); writer.Double(-999999); // altitude

		// air temperatures
		writer.Key("PARAM36"); writer.Double(airdb); // Entering Air Temperature Dry Bulb
		writer.Key("PARAM37"); writer.Int(airwb); // Entering Air Temperature Wet Bulb
		writer.Key("PARAM38"); writer.Int(-999999); // Entering Air Absolute Humidity
		writer.Key("PARAM39"); writer.Double(-999999); // Entering Air Relative Humidity

		// air flow
		writer.Key("PARAM54"); writer.Double(airflow); // Volumetric Air Flow
		writer.Key("PARAM46"); writer.Int(-999999); // Massic Air Flow - Moist air
		writer.Key("PARAM34"); writer.Int(-999999); // Air velocity
		writer.Key("PARAM97"); writer.Int(-999999); // fan select
		writer.Key("PARAM99"); writer.Int(0); // fan direction

		//fluid
		

		writer.Key("PARAM43"); writer.Int(fluidtype+ glycoletype); //type fluid
		writer.Key("PARAM42"); writer.Int(glycoleperc); //type perc in kg (kg fluid / kg mixture)

		writer.Key("PARAM89"); writer.Int(-999999); //Superheated water Pressure
		writer.Key("PARAM90"); writer.Int(-999999); //Custom fluid density
		writer.Key("PARAM91"); writer.Int(-999999); //Custom fluid viscosity
		writer.Key("PARAM92"); writer.Int(-999999); //Custom fluid concuctivity
		writer.Key("PARAM93"); writer.Int(-999999); //Custom fluid specific heat

		writer.Key("PARAM40");
		if (waterin == -1 && waterflow > 0)
			 writer.Double(-999999); //Entering fluid temperature
		else
			writer.Double(waterin); //Entering fluid temperature

		writer.Key("PARAM45");
		if (waterout == -1 && waterflow > 0)
			writer.Double(-999999); //Leaving fluid temperature
		else
			writer.Double(waterout); //Leaving fluid temperature
		
		writer.Key("PARAM55");
		if (waterflow > 0)
			 writer.Double(Round(waterflow/1000.0,3)); //Volumetric Fluid Flow
		else
			writer.Double(-999999); //Volumetric Fluid Flow

		writer.Key("PARAM47"); writer.Int(-999999); //Mass Fluid Flow

		writer.Key("PARAM95"); writer.Int(-999999); //diameter of header

		//duty
		writer.Key("PARAM87"); writer.Double(0); //Oversurface Requested

		writer.EndObject();



		String^ strJSONCoil;
		strJSONCoil = gcnew String(s.GetString());

		//strJSONCoil = "{""PARAM3"":1,""PARAM4"":true,""PARAM9"":-999999,""PARAM26"":84,""PARAM57"":-999999,""PARAM10"":900,""PARAM70"":0,""PARAM25"":6,""PARAM29"":2.12,""PARAM22"":1,""PARAM41"":1,""PARAM27"":31,""PARAM33"":""G"",""PARAM20"":""4S6"",""OP_01"":1,""OP_09"":2,""OP_03"":1,""OP_02"":1,""PARAM49"":7,""PARAM50"":-999999,""PARAM51"":-999999,""PARAM52"":0,""PARAM36"":42,""PARAM37"":-999999,""PARAM38"":-999999,""PARAM39"":30,""PARAM54"":18612,""PARAM46"":-999999,""PARAM34"":-999999,""PARAM97"":-999999,""PARAM99"":0,""PARAM43"":1,""PARAM89"":-999999,""PARAM90"":-999999,""PARAM91"":-999999,""PARAM92"":-999999,""PARAM93"":-999999,""PARAM40"":6.5,""PARAM45"":12.8,""PARAM55"":-999999,""PARAM47"":-999999,""PARAM95"":-999999,""PARAM87"":0}";
		//CString temp = CString(strJSONCoil);
		//::MessageBox(NULL, temp, _T(""), MB_OK);
		leelcoilsDLL::Calculation^ calcLeel = gcnew leelcoilsDLL::Calculation();

		String^ error = calcLeel->StartCalculation(strJSONCoil)->Trim();

		CString d = error;
		::MessageBox(NULL, d, _T(""), MB_OK);
		//if (atoi(d) > 0)
		//{
		System::Collections::Generic::List<System::String^>^ results;
		String^ result = "";
		std::array <String^, 100>;
		
		if (error->IsNullOrEmpty(error))
		{
			results = calcLeel->ReadResults();
			result = calcLeel->ReadResult(0);
			d = result;
			::MessageBox(NULL, d, _T(""), MB_OK);

			std::string str = marshal_as<std::string>(result);
			Document res;
			res.Parse(str.c_str());
			
			capacity = res["OUT_91"].GetDouble();
			outdb = res["OUT_05"].GetDouble();
			outwb = res["OUT_06"].GetDouble();
			outhumrel = res["OUT_08"].GetDouble();
			airpressuredp = res["OUT_224"].GetDouble(); //wet
			waterinlet = res["OUT_15"].GetDouble();
			wateroutlet = res["OUT_16"].GetDouble();
			waterflow = res["OUT_17"].GetDouble()*1000.0;
			waterpressuredp = res["OUT_21"].GetDouble();
			//valvepressuredp ?????  Fluid pressure Drop - Headers
			coilname = res["OUT_46"].GetString(); 
			weight = 0; //??????
			//sono alettati...non ci sono i fuori tutto
			overalllength  = res["OUT_31"].GetDouble();
			overallheight = res["OUT_27"].GetDouble();
		}
		else
			errorcode = atoi(d);

		
		//d = nresults;
		//::MessageBox(NULL, d, _T(""), MB_OK);
		//String^ outputCoil = calcLeel->ReadResults();
		//d = outputCoil;
		//::MessageBox(NULL, d, _T(""), MB_OK);
		//}
		
		/*
				if (output == "") {
					array<System::String^>^ single_result = calcLeel->ReadResults();

					if (single_result->Length > 0) {
						JOres = Newtonsoft::Json::JsonConvert::DeserializeObject(single_result[0]);
					}
				}

		*/
	}

	StringBuffer s;
	Writer<StringBuffer> writer(s);

	writer.StartObject();
		writer.Key("result");
		writer.StartObject();
			writer.Key("performance");
			writer.StartObject();
				writer.Key("capacity"); writer.Double(capacity);
				writer.Key("outdb"); writer.Double(outdb);
				writer.Key("outwb"); writer.Double(outwb);
				writer.Key("outhumrel"); writer.Double(outhumrel);
				writer.Key("airpressuredp"); writer.Double(airpressuredp);
				writer.Key("waterinlet"); writer.Double(waterinlet);
				writer.Key("wateroutlet"); writer.Double(wateroutlet);
				writer.Key("waterflow"); writer.Double(waterflow);
				writer.Key("waterpressuredp"); writer.Double(waterpressuredp);
				writer.Key("valvepressuredp"); writer.Double(valvepressuredp);
			writer.EndObject();
			writer.Key("configuration");
			writer.StartObject();
				writer.Key("coilname"); writer.String(coilname);
				writer.Key("weight"); writer.Double(weight);
				writer.Key("overalllength"); writer.Double(overalllength);
				writer.Key("overallheight"); writer.Double(overallheight);
			writer.EndObject();
		writer.Key("errorid"); writer.Int(errorcode);
		writer.Key("version");  writer.String(VERSION);

	writer.EndObject();
	writer.EndObject();


	String^ jsoinrecordset;
	jsoinrecordset = gcnew String(s.GetString());
	return  jsoinrecordset;
}
String^ Rooftop::GetNoiseData(String^ jSONIN)
{
	std::string str = marshal_as<std::string>(jSONIN);
	Document doc;
	doc.Parse(str.c_str());
 	int errorcode = 0;

	//lettura json input
	double port[3] = { 0,0,0 };
	double pdc[3] = { 0,0,0 };
	CString model = doc["modelid"].GetString();
	int supplier = doc["supplierid"].GetInt();

	//not used

/*	CString fantypesupply = doc["fantypesupply"].GetString();
	CString fantypeexhaust = doc["fantypeexhaust"].GetString();
	CString fantypeoutdoor = doc["fantypeoutdoor"].GetString();

	port[1] = doc["airflowexhaust"].GetDouble() / 3600.0;
	port[0] = doc["airflowsupply"].GetDouble() / 3600.0;
	port[2] = doc["airflowoutdoor"].GetDouble() / 3600.0;

	pdc[1] = doc["staticdpexh"].GetDouble();
	pdc[0] = doc["staticdpsup"].GetDouble();
	pdc[2] = doc["staticdpout"].GetDouble();*/

	double attenuazioni[8][8];
	//banda di ottava per:
	//1 before supply fan
	//2 after supply fan
	//3 before ex.fan
	//4 before ex.fan
	//5 after compressore
	//6 before condeser fan
	//7 after condenser fan
	//8 casing

	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
			attenuazioni[i][j] = 0;
	}

	Value& responseObj = doc["options"];
	if (responseObj.IsArray())
	{
		for (int i = 0; i < responseObj.GetArray().Size(); i++)
		{
			Value& opt = responseObj[i];

			if (opt.IsObject())
			{
				CString optcode = opt["option"].GetString();
				double value = opt["value"].GetDouble();
				if (value == 1)
				{
					double att[8];
					short tipo = GetAttenuazioni(optcode, att);
					if (tipo >= 1 && tipo < 9)
					{
						for (int j = 0; j < 8; j++)
							attenuazioni[tipo-1][j] += att[j];
					}
				}
			}

		}	
	}

	double distance = doc["distance"].GetDouble();
	double dens = doc["density"].GetDouble();
	double temperature = doc["temperature"].GetDouble();

	CString noisesupplyin = doc["noisesupplyin"].GetString();
	CString noisesupplyout = doc["noisesupplyout"].GetString();
	CString noiseretin = doc["noiseretin"].GetString();
	CString noiseretout = doc["noiseretout"].GetString();
	CString noiseoutin = doc["noiseoutin"].GetString();
	CString noiseoutout = doc["noiseoutout"].GetString();

	int iqngn = doc["iqngn"].GetInt();

	int pos1 = 0;
	int pos2 = 0;
	int pos3 = 0;
	double noisesupplyinV[9];
	double noisesupplyoutV[9];
	double noiseoutoutV[9];
	double filtroA[8] = { 26.2,16.1,8.6,3.2,0,-1.2,-1,1.1 };
	
	//DA LEGGERE DA TABELLA
	//double coilatt[8] = {1,1,1,1,1,1,1,1};
	//double mitigationcasing[8] = { 8,8,8,8,8,8,8,8 };
	//double jacket[8] = { 0,0,0,3.5,14.3,18.2,21.7,0.0 };

	for (int i = 1; i < 9; i++)
	{
		noisesupplyinV[i] = _tstof(ExtractString(noisesupplyin, &pos1, _T(";")));
		noisesupplyoutV[i] = _tstof(ExtractString(noisesupplyout, &pos2, _T(";")));
		noiseoutoutV[i] = _tstof(ExtractString(noiseoutout, &pos3, _T(";")));
	}
	
	//CALCULATION
	double outdoorbandV[9], supinbandV[9], supoutbandV[9], retinbandV[9], retoutbandV[9], Outdoorband_noexV[9], supinband_noexV[9], supoutband_noexV[9], retinband_noexV[9], retoutband_noexV[9];
	double sumlog[10];
	//////
	for (int i = 1; i < 9; i++)
	{
		supoutband_noexV[i] = noisesupplyoutV[i] - filtroA[i - 1] - attenuazioni[1][i - 1];
		supinband_noexV[i] = noisesupplyinV[i] - filtroA[i - 1] - attenuazioni[0][i - 1];
		sumlog[0] += pow(10, supoutband_noexV[i] / 10.0);
		sumlog[1] += pow(10, supinband_noexV[i] / 10.0);

	}
	supoutband_noexV[0] = 10 * log10(sumlog[0]); //Sound Power Levels SUPPLY(dBA) = SUPPLY FAN (out) * SUPPLY FAN NUMBER - EAR ATTENUATION
	supinband_noexV[0] = 10 * log10(sumlog[1]); //Sound Power Levels RETURN(dBA) = SUPPLY FAN (in) * SUPPLY FAN NUMBER - INDOOR COIL ATTENUATION - EAR ATTENUATION
	////////
	
	//non sono sicuro dell'associazione tra risultati calcolati in excel e output definiti nella documentazione.
	////////////
	double NoiseSupplyTot[9]; 
	for (int i = 1; i < 9; i++)
	{
		NoiseSupplyTot[i] = 10 * log10(pow(10, noisesupplyoutV[i] / 10.0) + (pow(10, noisesupplyinV[i] / 10.0))) - filtroA[i - 1] - attenuazioni[7][i - 1];//mitigationcasing[i-1];
		sumlog[2] += pow(10, NoiseSupplyTot[i] / 10.0);
	}
	NoiseSupplyTot[0] = 10 * log10(sumlog[2]);//Sound Power Levels OUT OF UNIT with only supply fan (dBA) = (SUPPLY fan (in) + SUPPLY fan (out)) * SUPPLY fan NUMBER - TREATMENT BOX ATTENUATION - EAR ATTENUATION

	//////////
	double NoiseOutdoorTot[9];
	for (int i = 1; i < 9; i++)
	{
		NoiseOutdoorTot[i] = noiseoutoutV[i] - filtroA[i - 1];
		sumlog[3] += pow(10, NoiseOutdoorTot[i] / 10.0);

	}
	NoiseOutdoorTot[0] = 10 * log10(sumlog[3]); //Sound Power Levels OUT OF UNIT with only condensing fan (dBA) = CONDENSER fan (out) - EAR ATTENUATION
	////////////

	double noiseCompV[9];
	//double noiseCompCOJAV[9];

	pos1 = 0;
	pos2 = 0;
	String^ jSONComp = GetCondeserNoise(); //input //WIth/Without jacket (COJA)??portata? pressione? 
	std::string strComp = marshal_as<std::string>(jSONIN);
	Document docComp;
	docComp.Parse(strComp.c_str());
	if (docComp.IsObject())
	{
		//lettura json output delle bande d'ottava di rumoristà del compressore, nella verisione con e senza jacket
		CString noiseComp = docComp["noise"].GetString();
		//CString noiseCompCOJA = docComp["noisecoja"].GetString();


		for (int i = 0; i < 9; i++)
		{
			noiseCompV[i] = _tstof(ExtractString(noiseComp, &pos1, _T(";")));
			//noiseCompCOJAV[i] = _tstof(ExtractString(noiseCompCOJA, &pos2, _T(";")));
		}
		//////
		for (int i = 1; i < 9; i++)
		{
			noiseCompV[i] = noiseCompV[i] - filtroA[i - 1] - attenuazioni[6][i - 1];
			//noiseCompCOJAV[i] = noiseCompCOJAV[i] - filtroA[i - 1] - coilatt[i - 1] - jacket[i - 1];
			sumlog[4] += pow(10, noiseCompV[i] / 10.0);
			//sumlog[5] += pow(10, noiseCompCOJAV[i] / 10.0);

		}
		noiseCompV[0] = 10 * log10(sumlog[4]); //Sound Power Levels SUPPLY(dBA) = SUPPLY FAN (out) * SUPPLY FAN NUMBER - EAR ATTENUATION
		//noiseCompCOJAV[0] = 10 * log10(sumlog[5]); //Sound Power Levels RETURN(dBA) = SUPPLY FAN (in) * SUPPLY FAN NUMBER - INDOOR COIL ATTENUATION - EAR ATTENUATION
	}

	///////
	for (int i = 1; i < 9; i++)
	{
		Outdoorband_noexV[i] = 10 * log10(pow(10, noiseCompV[i] / 10.0) + pow(10, NoiseSupplyTot[i] / 10.0) + pow(10, NoiseOutdoorTot[i] / 10.0));
		//Outdoorband_noexV[i] = 10 * log10(pow(10, noiseCompV[i] / 10.0) + pow(10, NoiseSupplyTot[i] / 10.0) + pow(10, NoiseOutdoorTot[i] / 10.0));
		sumlog[6] += pow(10, Outdoorband_noexV[i] / 10.0);
		//sumlog[7] += pow(10, Outdoorband[i] / 10.0);
	}
	Outdoorband_noexV[0] = 10 * log10(sumlog[6]); //Total Sound Power Levels OUT OF UNIT (dBA)= SUPPLY FAN + CONDENSER FAN + COMPRESSOR
	//Outdoorband_noexV[0] = 10 * log10(sumlog[7]); //Total Sound Power Levels OUT OF UNIT (dBA) [COJA] = SUPPLY FAN + CONDENSER FAN + COMPRESSOR WITH JACKET

	double presnoiselevel = Round(Outdoorband_noexV[0] + 10.0 * log10(1 / (2 * PIGRECO * pow(distance,2))), 1); //è la pressione sonora totale all'esterno con compressore 
	//telefonata con pino 12-05-23 , cambiato in 2 * PIGRECO, è semisferico e non sferico.
	
	//OUTPUT

	
	CString outdoorband = "", supinband = "", supoutband = "", retinband = "", retoutband = "", Outdoorband_noex = "", supinband_noex = "", supoutband_noex = "", retinband_noex = "", retoutband_noex = "";
	
	for (int i = 0; i < 9; i++)
	{
		CString temp,temp1,temp2;
		temp.Format("%.1f;", Outdoorband_noexV[i]);
		temp1.Format("%.1f;", NoiseSupplyTot[i]);
		Outdoorband_noex += temp;
		supoutband_noex += temp1;
	}

	StringBuffer s;
	Writer<StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("result");
	writer.StartObject();
	
	writer.Key("outdoorband"); writer.String(outdoorband);
	writer.Key("supinband"); writer.String(supinband);
	writer.Key("supoutband"); writer.String(supoutband);
	writer.Key("retinband"); writer.String(retinband);
	writer.Key("retoutband"); writer.String(retoutband);
	writer.Key("Outdoorband_noex"); writer.String(Outdoorband_noex);
	writer.Key("supinband_noex"); writer.String(supinband_noex);
	writer.Key("supoutband_noex"); writer.String(supoutband_noex);
	writer.Key("retinband_noex"); writer.String(retinband_noex);
	writer.Key("retoutband_noex"); writer.String(retoutband_noex);
	writer.Key("presnoiselevel"); writer.Double(presnoiselevel); // è la pressione sonora totale all'esterno con compressore senza jacket
	
	writer.Key("errorid"); writer.Int(errorcode);
	writer.Key("version");  writer.String(VERSION);
	
	writer.EndObject();
	writer.EndObject();


	String^ jsoinrecordset;
	jsoinrecordset = gcnew String(s.GetString());
	return  jsoinrecordset;
}

String^ Rooftop::GetCondeserNoise()
{
	String^ err;
	return  err;
}
short Rooftop::GetAttenuazioni(CString code, double att[])
{
	std::string filter = "";
	g_NoiseAtt.AddFilterField("OptionCode", "=", code, filter);
	//ricerco il modello e mandato o ripresa
	CGenRecordList options = g_NoiseAtt.GetRecordList(filter);
	CGenTableRecord noise = g_NoiseAtt.Lookup(filter);
	short tipo = -1;
	if (noise.IsValid())
	{
		noise.GetColumn("INOUT", tipo);
		noise.GetColumn("63HZ", att[0]);
		noise.GetColumn("125HZ", att[1]);
		noise.GetColumn("250HZ", att[2]);
		noise.GetColumn("500HZ", att[3]);
		noise.GetColumn("1000HZ", att[4]);
		noise.GetColumn("2000HZ", att[5]);
		noise.GetColumn("4000HZ", att[6]);
		noise.GetColumn("8000HZ", att[7]);
	}
	//tipo:
	//banda di ottava per:
	//1 before supply fan
	//2 after supply fan
	//3 before ex.fan
	//4 before ex.fan
	//5 after compressore
	//6 before condeser fan
	//7 after condenser fan
	//8 casing
	
	return  tipo;
}

String^ Rooftop::GetNoiseData1(String^ jSONIN) 
{
	String^ err;
	return  err;
}
String^ Rooftop::GetOptionsPressureDrop(String^ jSONIN) 
{
	std::string str = marshal_as<std::string>(jSONIN);
	Document doc;
	doc.Parse(str.c_str());
	int errorcode = 0;
	//lettura json input
	double port[2] = { 0,0 };
	CString model = doc["modelid"].GetString();
	port[1] = doc["airflowexhaust"].GetDouble()/3600.0;
	port[0] = doc["airflowsupply"].GetDouble()/3600.0;
	double td1 = doc["coiltempdb"].GetDouble();
	double tw1 = doc["coiltempwb"].GetDouble();
	double td2 = doc["coiltempposthdb"].GetDouble();
	double tw2 = doc["coiltempposthwb"].GetDouble();
	double td3 = doc["coiltemppostcdb"].GetDouble();
	double tw3 = doc["coiltemppostcwb"].GetDouble();
	int iqngn = doc["iqngn"].GetInt();

	
	if (port[1] * port[0] <= 0)
		errorcode = 1; //portata = 0

	String^ jsoinrecordset;
	std::string filter;
	

	CString code = L"";
	long ripresa = 0,tipo = 0;
	double pdc = 0;
	double a = 0, b=0, c=0, d=0, e=0;

	//inizio scrittura json output
	StringBuffer s;
	Writer<StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("result");
	writer.StartObject();
	writer.Key("supply");
	writer.StartArray();
	

	for (int j = 0; j < 2; j++)
	{
		filter = "";
		g_CoeffPdc.AddFilterField("Nomcomm", "=", model, filter);
		g_CoeffPdc.AddFilterField("Flow_stream", "=", j, filter);
		//ricerco il modello e mandato o ripresa
		CGenRecordList options = g_CoeffPdc.GetRecordList(filter);
		if (j == 1)
		{
			writer.Key("return");
			writer.StartArray();	
		}
		for (const auto currDLL : options)
		{
			CGenTableRecord option = currDLL.c_str();
			//per ogni record aggiungo un oggetto nell'array di mandata o ripresa nel json output
			option.GetColumn("SupplyDP_code", code);
			option.GetColumn("a", a);
			option.GetColumn("b", b);
			option.GetColumn("c", c);
			option.GetColumn("d", d);
			option.GetColumn("e", e);
			option.GetColumn("Type", tipo);
			
			//se type è > 0 si devono applicare delle formule diverse per il calcolo delle perdite di carico
			if (tipo == 1)
			{
				double Nfiltri = 1;
				std::string filter1 = "";
				g_ModelTable.AddFilterField("Nomcomm", "=", model, filter1);
				CGenTableRecord filtri = g_ModelTable.Lookup(filter1);
				long n1 = 1, n2 = 0;
				filtri.GetColumn("Filter_quantity", n1);
				filtri.GetColumn("Filter_quantity2", n2);
				if (n1 + n2 <= 0)
				{
					errorcode = 5;
					n1 = 1;
				}
				port[j] /= (n1+n2);
				
			}
			pdc = a * pow(port[j], 4) + b * pow(port[j], 3) + c * pow(port[j], 2) + d * port[j] + e;
			pdc = Round(pdc, 1);
			if (pdc <= 0)
				errorcode = 2; 
			if (pdc > 300)
				errorcode = 3; 
			CGas gas;
			if (tipo == 2) //batteria calda preheating
			{
				double um = gas.ClUmRel(td1, tw1, 1.013);
				double dens = gas.Densita(td1, um, 1.013);
				double tmpDens = dens / 1.20433;
				if (tmpDens <= 0)
					errorcode = 6;
				pdc = Round(pdc/ tmpDens, 1);
			}
			if (tipo == 4) //batteria fredda post heating (verifica estiva)
			{
				bool deumidificazione = 0;
				if (deumidificazione)
				{
					double um = gas.ClUmRel(td3, tw3, 1.013);
					double dens = gas.Densita(td3, um, 1.013);
					double tmpDens = dens / 1.20433;
					if (tmpDens <= 0)
						errorcode = 6;
					pdc = Round(pdc / tmpDens, 1);
				}
				else
				{
					//cerco la calda post heating 
					std::string filter1 = "";
					g_CoeffPdc.AddFilterField("Nomcomm", "=", model, filter1);
					g_CoeffPdc.AddFilterField("Flow_stream", "=", j, filter1);
					g_CoeffPdc.AddFilterField("Type", "=", 3, filter1);
					//ricerco il modello e mandato o ripresa e batteria
					CGenTableRecord caldapost = g_CoeffPdc.Lookup(filter1);
					caldapost.GetColumn("a", a);
					caldapost.GetColumn("b", b);
					caldapost.GetColumn("c", c);
					caldapost.GetColumn("d", d);
					caldapost.GetColumn("e", e);

					pdc = a * pow(port[j], 4) + b * pow(port[j], 3) + c * pow(port[j], 2) + d * port[j] + e;
					pdc = Round(pdc, 1);
					if (pdc <= 0)
						errorcode = 2;
					if (pdc > 300)
						errorcode = 3;
					tipo = 3;
				}

			}
			if (tipo == 3) //batteria calda post heating
			{	
				double um = gas.ClUmRel(td2, tw2, 1.013);
				double dens = gas.Densita(td2, um, 1.013);
				double tmpDens = dens / 1.20433;
				if (tmpDens <= 0)
					errorcode = 6;
				pdc = Round(pdc / tmpDens, 1);
			}
			
			writer.StartObject();
			writer.Key("options"); writer.String(code);
			writer.Key("pressure");  writer.Double(pdc);
			writer.EndObject();
		}
		writer.EndArray();
		
	}
	writer.Key("errorid"); writer.Int(errorcode);
	writer.Key("version");  writer.String(VERSION);
	writer.EndObject();
	writer.EndObject();

	jsoinrecordset = gcnew String(s.GetString());
	
	return  jsoinrecordset;
}
String^ Rooftop::GetDrawing(String^ jSONIN) 
{
	String^ err;
	return  err;
}
String^ Rooftop::GetBIMModel(String^ jSONIN) 
{
	String^ err;
	return  err;
}

bool Rooftop::Init()
{
	bool l;
	TCHAR szPathProgramData[1024];
	GetCurrentDirectory(1024, szPathProgramData);
	CString path = CString(szPathProgramData);
	//::MessageBox(NULL, path, _T(""), MB_OK);
	//String^ str = marshal_as <std::string> path;
	//String^ boh = str;
	l = OpenConnection();
	l &= LoadModel();
	l &= LoadCoeffPdc();
	l &= LoadNoiseAttenuation();
	l &= LoadEBMDll();

	SetCurrentDirectory(path); //setting path
	//::MessageBox(NULL, path, _T(""), MB_OK);
	return l;
}
bool Rooftop::OpenConnection()
{
	//HRESULT		hr;
	CString provider = "";
	if (!OpenDataSource(CString("elencal.udl"), g_Sql, CString(""), CString("")))
	{
		MessageBox(NULL, _T("Unable to open session to SQL SERVER - Elencal.udl not found"), "ALTEC", MB_ICONWARNING);
		return false;
	}

	try
	{
		g_session.Open(g_Sql);
	}
	catch (HRESULT hr)
	{
		DisplayOLEDBErrorRecords(hr);
		return false;
	}
	return true;

	//VERSIONE CON ADO E ACCESSOR
	/*HRESULT		hr;
	CString varUserId = _T("");
	CString varPwd = _T("");
	CString strConn = _T("File Name=elencal.udl;");
	ADOConnection* pConnSQL = NULL;
	try
	{
		CoCreateInstance(CONGUID, NULL, CLSCTX_INPROC_SERVER, CONINTGUID, (LPVOID*)&pConnSQL);
		hr = pConnSQL->Open(strConn.AllocSysString(), varUserId.AllocSysString(), varPwd.AllocSysString(), adOpenUnspecified);
	}
	catch (HRESULT hr)
	{

	}
	ADORecordset* pModelRs = NULL;

	CModelAccessor modelAccessor;
	IADORecordBinding* picRs = NULL;   // Interface Pointer declared.

	_variant_t vNull;
	CoCreateInstance(RECGUID, NULL, CLSCTX_INPROC_SERVER, RECINTGUID, (LPVOID*)&pModelRs);

	CString strFmt = _T("RT2_B6_altec_definition");
	hr = pModelRs->put_Source(strFmt.AllocSysString());
	hr = pModelRs->Open((_variant_t)strFmt.AllocSysString(), _variant_t((IDispatch*)pConnSQL, true),
		adOpenStatic, adLockReadOnly, adCmdTable);
	hr = pModelRs->QueryInterface(__uuidof(IADORecordBinding), (LPVOID*)&picRs);
	hr = picRs->BindToRecordset(&modelAccessor);

	picRs->Release();
	HRESULT hrpr = pModelRs->MoveFirst();
	VARIANT_BOOL vbEOF;
	pModelRs->get_EOF(&vbEOF);

	while (!vbEOF)
	{
		pModelRs->MoveNext();

		CString test = modelAccessor.m_Nomecomm;

		pModelRs->get_EOF(&vbEOF);
	}
	pModelRs->Close();*/
}

bool Rooftop::LoadNoiseAttenuation()
{
	CString datastr = L"SELECT RT2_ALTEC_ATTENUATION.* FROM RT2_ALTEC_ATTENUATION order by IDNOISE";
	if (g_NoiseAtt.LoadFromDB(g_session, datastr.AllocSysString(), false))
		return true;
	else
		return false;
}

bool Rooftop::LoadCoeffPdc()
{
	
	CString datastr = L"SELECT RT2_Altec_dpcomposants.* FROM RT2_Altec_dpcomposants order by Nomcomm, Flow_stream";
	if (g_CoeffPdc.LoadFromDB(g_session, datastr.AllocSysString(), false))
		return true;
	else
		return false;

}

bool Rooftop::LoadModel()
{

	CString datastr = L"SELECT RT2_definition.iddefinition, RT2_B6_altec_definition.* \
 FROM RT2_definition INNER JOIN RT2_B6_altec_definition ON RT2_definition.nomcomm = RT2_B6_altec_definition.Nomcomm order by Nomcomm";
	if (g_ModelTable.LoadFromDB(g_session, datastr.AllocSysString(), false))
		return true;
	else
		return false;


	
}

bool Rooftop::OpenDataSource(CString fileName, CDataSource& ds, CString provider, CString pwd)
{

	// Default provider = JET
	if (provider.IsEmpty())
		provider = _T("Microsoft.JET.OLEDB.4.0");

	CSession session;
	CDBPropSet dbps;
	dbps.SetGUID(DBPROPSET_JETOLEDB_DBINIT);

	CString con;
	if (fileName.Find(_T(".udl")) != -1)
		con.Format(_T("File Name=%s;"), fileName);
	else
	{
		con = _T("Provider=");
		con += provider + _T("; Data Source=") + fileName;
		if (!pwd.IsEmpty())
		{
			con += _T("; Jet OLEDB:Database Password=") + pwd;
		}
	}
	_bstr_t bstrConnect;
	_bstr_t bstrFile;
	bstrConnect = con;
	bstrFile = fileName;

	HRESULT hr;

	if (fileName.Find(_T(".udl")) != -1)
		hr = ds.OpenFromFileName(bstrFile);
	else
		hr = ds.OpenFromInitializationString(bstrConnect);
	
	if (FAILED(hr))
	{
		//DisplayOLEDBErrorRecords(hr);
		//PopupErrorMessage(hr);
		return false;
	}


	return true;

}
String^ Rooftop::SearchModel(CString model, CString fanopt, double portata, int fantype)
{
	String^ jsoinrecordset;

	//g_ModelTable.ResetFilter();
	std::string filter;
	g_ModelTable.AddFilterField("Nomcomm", "=", model, filter);
	CGenTableRecord modello = g_ModelTable.Lookup(filter);

	if (!modello.IsValid())
		return jsoinrecordset;

	long nfan = 1;
	CString test = "";
	//da riceve conferma mappatura con colonne db gf 21-04-23 -> segnate nella documentazione
	if (fantype == 1)
	{
		if (fanopt.IsEmpty() || fanopt == _T("SFLC"))
		{
			modello.GetColumn("Indoor_Fan_STD", test);
			modello.GetColumn("Indoor_Qty_Fan_STD", nfan);
		}
		if (fanopt == _T("SFHC"))
		{
			modello.GetColumn("Indoor_Fan_SFHC", test);
			modello.GetColumn("Indoor_Qty_Fan_SFHC", nfan);
		}
		if (fanopt == _T("SFLA"))
		{
			modello.GetColumn("Indoor_Fan_SFLA", test);
			modello.GetColumn("Indoor_Qty_Fan_SFLA", nfan);
		}
		if (fanopt == _T("SFHA"))
		{
			modello.GetColumn("Indoor_Fan_SFHA", test);
			modello.GetColumn("Indoor_Qty_Fan_SFHA", nfan);
		}
	}
	//return fan
	if (fantype == 2)
	{
		if (fanopt.IsEmpty() || fanopt == _T("EFLC"))
		{
			modello.GetColumn("Return_fan_EFLC", test);
			modello.GetColumn("Return_Qty_fan", nfan);
		}
		if (fanopt == _T("EFHC"))
		{
			modello.GetColumn("Return_fan_EFHC", test);
			modello.GetColumn("Return_Qty_fan", nfan);
		}
		if (fanopt == _T("EFLA"))
		{
			modello.GetColumn("Return_fan_EFLA", test);
			modello.GetColumn("Return_Qty_fan", nfan);
		}
		if (fanopt == _T("EFHA"))
		{
			modello.GetColumn("Return_fan_EFHA", test);
			modello.GetColumn("Return_Qty_fan", nfan);
		}
	}
	double portmin = 0, portmax = 0;
	modello.GetColumn("Indoor_fan_airflow_min", portmin);
	modello.GetColumn("Indoor_fan_airflow_max_STD", portmax);

	//ATTENZIONE QUESTI SONO QUELLI ESTERNI ASSIALI, sono EBM?
	if (fantype == 3)
	{
		if (fanopt == _T("EAFA"))
		{
			modello.GetColumn("C1_Outdoor_fan_STD", test);
			modello.GetColumn("C1_Outdoor_fan_qty", nfan);
			modello.GetColumn("C1_Outdoor_fan_airflow", portmax);
			portmin = 0;
		}
		if (fanopt == _T("OFLN"))
		{
			modello.GetColumn("C2_Outdoor_fan_STD", test);
			modello.GetColumn("C2_Outdoor_fan_qty", nfan);
			modello.GetColumn("C2_Outdoor_fan_airflow", portmax);
			portmin = 0;

		}
		if (fanopt == _T("OFSP"))
		{
			modello.GetColumn("Outdoor_fan_OFSP", test);
			modello.GetColumn("C2_Outdoor_fan_qty", nfan);
			modello.GetColumn("C2_Outdoor_fan_airflow", portmax);
			portmin = 0;
		}
	}
	


	if ((portata < portmin || portata > portmax) && portata > 0)
		return jsoinrecordset;

	CString coil;
	modello.GetColumn("Indoor_Exchanger", coil);

	//cerco corrispondenza tra nome modello e codice articolo preso da dll:
	std::string str = (g_elencoEBMJson);
	//::MessageBox(NULL, g_elencoEBMJson, _T("modello fan"), MB_OK);
	Document doc;
	doc.Parse(str.c_str());
	CString codart = doc[test.GetString()].GetString();

	//test.Format(_T("%d"), val);
	//::MessageBox(NULL, test, _T("modello fan"), MB_OK);

	//CString test = "176408";
	
	StringBuffer s;
	Writer<StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("FANMODEL"); writer.String(test);
	writer.Key("FANCODE"); writer.String(codart);
	writer.Key("FANNUMBER"); writer.Int(nfan);
	writer.Key("COIL");  writer.String(coil);
	
	writer.EndObject();
	
	jsoinrecordset = gcnew String(s.GetString());
	//::MessageBox(NULL, s.GetString(), _T("modello fan"), MB_OK);
	//"{\"FANMODEL\": \"176408\",""maxWidth"": 10000, ""maxHeigth"": 10000, ""euroventfactor"": 0, ""minairflow"": 0.2, ""maxairflow"": 4, ""mindp"": 20, ""maxdp"": 800}";
	
	return jsoinrecordset;
}

// sfp va passato in W/m3/s
short Rooftop::GetSFPClass(double sfp)
{
	short classe = 7;
	if (sfp < 500)
		classe = 1;
	else if (sfp < 750)
		classe = 2;
	else if (sfp < 1250)
		classe = 3;
	else if (sfp < 2000)
		classe = 4;
	else if (sfp < 3000)
		classe = 5;
	else if (sfp < 4500)
		classe = 6;
	else
		classe = 7;
	return classe;
}


bool Rooftop::LoadEBMDll()
{

	USES_CONVERSION;
	
	CString fanEBM;
	CString temp = ("\\data\\plug_fans");
	//char path33[MAX_PATH + 1];


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

	TCHAR szPathProgramData[1024];
	GetCurrentDirectory(1024, szPathProgramData);
	CString path = CString(szPathProgramData);
	
	temp.Format(_T("%s\\data\\plug_fans\\"), path);
	//::MessageBox(NULL, temp, _T(""), MB_OK);
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
		fanEBM = A2W(cMBBuffer2);
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
	int pos1 = 0;
	char* cMBOut = new char[2001];
	g_elencoEBMJson = _T("");
	StringBuffer s;
	Writer<StringBuffer> writer(s);
	writer.StartObject();

	while (true)
	{
		CString fan = ExtractString(fanEBM, &pos1, _T(";"));
		if (fan.IsEmpty())
			break;
		err = GET_TECHNICAL_DATA_PC((fan.GetBuffer(0)), &cMBOut);	// MV 13.05.2014. Per utilizzare la nuova DLL 3.0.1.0
		//CString cip = cMBOut;
		// La riga dopo crea i dati tecnici per singolo ventilatore
		//TRACE(_T("%s;%s"), fan,cip);TRACE(_T("\n"));
		CString fan1 = ExtractString(fanEBM, &pos1, _T(";"));

	//	::MessageBox(NULL, fan1, _T("modello fan"), MB_OK);
	//	::MessageBox(NULL, fan, _T("modello fan"), MB_OK);
		// la riga dopo crea i codici articolo
		//TRACE(_T("%s;%s-%s-%s ;%s\n"), fan, fan1.Left(6), fan1.Mid(6, 4), fan1.Mid(10, 2), fan1);
		
		
		writer.Key(fan1); writer.String(fan);
		
	}

	writer.EndObject();
	g_elencoEBMJson = gcnew String(s.GetString());

	SAFE_ARRAY_DELETE(cMBOut);

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
	SET_DECIMALSEPARATOR = (PEBMPAPSTFAN_FNCT30)GetProcAddress(g_EbmPapstFanDLL, "SET_DECIMALSEPARATOR");
	int test1 = 0;
	temp.Format(_T("."));
	
	if (SET_DECIMALSEPARATOR)
	{
		test1 = SET_DECIMALSEPARATOR(A2W(temp));
	}
	if (!GET_STANDARDS_FANMOTOR)
	{

		return false;
	}
	
	return true;
}


