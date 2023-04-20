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
	CString fantype = doc["fantype"].GetString();
    double pTot = doc["optionsdp"].GetDouble();
	double dens = doc["density"].GetDouble(); 
	double temp = doc["temperature"].GetDouble();
	int iqngn = doc["iqngn"].GetInt();

	//recupero le informazioni nel database interno LENNOX in base all'idmodello richiesto
	//ritorna un json creato con i dati di ritorno del recordset
	String^ JSONmodelspecification = SearchModel(model, fantype);
	std::string str1 = marshal_as<std::string>(JSONmodelspecification);
	doc.Parse(str1.c_str());
	Value& responseObj = doc["FANCODE"];
	CString fanmodel = responseObj.GetString();
	//fanmodel.Format ("%d", responseObj.GetInt());
	double maxWidth = 10000; //doc["maxWidth"].GetDouble();
	double maxHeigth = 10000;// doc["maxHeigth"].GetDouble();
	/*double euroventfactor = doc["euroventfactor"].GetDouble();
	double minport = doc["minairflow"].GetDouble();
	double maxport = doc["maxairflow"].GetDouble();
	double minpd = doc["mindp"].GetDouble();
	double maxpd = doc["maxdp"].GetDouble();*/
	
	int errorcode = 0;
	StringBuffer s;
	bool calcok = false;

	if (fanmodel.IsEmpty())
	{
		errorcode = 1;
		goto exit;
	}
	

	if (supplier == 1)
	{
		//LoadEBMDll();

		CString request;
		INT num = 0; 
		double press = pTot;
		char cMBBuffer[4000];
		sprintf_s(cMBBuffer, 4000, "%s;0;0;%.4f;;%.2f;%.4f;%.4f;%.2f;%.2f;ebmpapst;0;F;4000", fanmodel.GetString(), dens, temp, press, port, maxWidth, maxHeigth);

		char cMBOut[4001]; ZeroMemory(cMBOut, 4001 * sizeof(char)); char* pRis = &cMBOut[0];
		int err = 999;

		CString ventRis;
		errorcode = err = GET_CALCULATION_FAN_ALONE_PC(&cMBBuffer[0], &pRis);	// MV 10.06.2014. Per versione nuova DLL 3.0.1.0
		if (err == 0)
		{
			ventRis = cMBOut;
			int pos1 = 0;
			ventRis.Replace(_T(","), _T("."));
			double m_nfunzi = _tstof(ExtractString(ventRis, &pos1, _T(";")));
			double m_kwfunz = _tstof(ExtractString(ventRis, &pos1, _T(";"))) / 1000.0;
			double m_phifun = _tstof(ExtractString(ventRis, &pos1, _T(";")));//corrente ampere
			double m_etasta = _tstof(ExtractString(ventRis, &pos1, _T(";")));
			
			double m_ufunzi = _tstof(ExtractString(ventRis, &pos1, _T(";")));//voltaggio controllo
			ExtractString(ventRis, &pos1, _T(";")); // momento di inerzia
			double m_lw5Afu = _tstof(ExtractString(ventRis, &pos1, _T(";"))); // efficienza motore
			double m_lw3Afu = _tstof(ExtractString(ventRis, &pos1, _T(";"))); //potenza sonora db
			double m_lw7Afu = _tstof(ExtractString(ventRis, &pos1, _T(";")));//potenza sonora db ingr
			double m_lw6Afu = _tstof(ExtractString(ventRis, &pos1, _T(";")));//potenza sonora db out
			double m_l7w006 = _tstof(ExtractString(ventRis, &pos1, _T(";")));//potenza sonora db ingr
			double m_l7w012 = _tstof(ExtractString(ventRis, &pos1, _T(";")));//potenza sonora db ingr
			double m_l7w025 = _tstof(ExtractString(ventRis, &pos1, _T(";")));//potenza sonora db ingr
			double m_l7w050 = _tstof(ExtractString(ventRis, &pos1, _T(";")));//potenza sonora db ingr
			double m_l7w100 = _tstof(ExtractString(ventRis, &pos1, _T(";")));//potenza sonora db ingr
			double m_l7w200 = _tstof(ExtractString(ventRis, &pos1, _T(";")));//potenza sonora db ingr
			double m_l7w400 = _tstof(ExtractString(ventRis, &pos1, _T(";")));//potenza sonora db ingr
			double m_l7w800 = _tstof(ExtractString(ventRis, &pos1, _T(";")));//potenza sonora db ingr
			double m_l6w006 = _tstof(ExtractString(ventRis, &pos1, _T(";")));//potenza sonora db out
			double m_l6w012 = _tstof(ExtractString(ventRis, &pos1, _T(";")));//potenza sonora db out
			double m_l6w025 = _tstof(ExtractString(ventRis, &pos1, _T(";")));//potenza sonora db out
			double m_l6w050 = _tstof(ExtractString(ventRis, &pos1, _T(";")));//potenza sonora db out
			double m_l6w100 = _tstof(ExtractString(ventRis, &pos1, _T(";")));//potenza sonora db out
			double m_l6w200 = _tstof(ExtractString(ventRis, &pos1, _T(";")));//potenza sonora db out
			double m_l6w400 = _tstof(ExtractString(ventRis, &pos1, _T(";")));//potenza sonora db out
			double m_l6w800 = _tstof(ExtractString(ventRis, &pos1, _T(";")));//potenza sonora db out
			double m_ptotfu = _tstof(ExtractString(ventRis, &pos1, _T(";")));//pressione totale
			CString temp;
			temp = ExtractString(ventRis, &pos1, _T(";")); //pressione statica
			temp = ExtractString(ventRis, &pos1, _T(";")); //portata
			temp = ExtractString(ventRis, &pos1, _T(";")); //rendimento statico
			temp = ExtractString(ventRis, &pos1, _T(";")); //Po Watt
			temp = ExtractString(ventRis, &pos1, _T(";")); //eta R
			temp = ExtractString(ventRis, &pos1, _T(";")); //eta SR
			temp = ExtractString(ventRis, &pos1, _T(";")); //(safety factor rpm [%])
			temp = ExtractString(ventRis, &pos1, _T(";")); //(specific fan power[kW / (m3 / s)])
			double m_l5A050 = _tstof(ExtractString(ventRis, &pos1, _T(";")));//(pressure inlet nozzle [Pa] (rough estimate))
			temp = ExtractString(ventRis, &pos1, _T(";")); //(safety factor rpm [%])
			//pData->m_l5A050 = _tstof(ExtractString(ventRis, &pos1, _T(";")));//(pressure inlet nozzle [Pa] (rough estimate))
			temp = ExtractString(ventRis, &pos1, _T(";")); //(pd[Pa])
			//temp = ExtractString(ventRis, &pos1, _T(";")); //Pressure loss with respect to installation space [Pa])
			double m_pdynfu = m_ptotfu - pTot;
			double m_pstafu = pTot;
			sprintf_s(cMBBuffer, 4000, "%s", fanmodel.GetString());

			err = GET_TECHNICAL_DATA_PC(cMBBuffer, &pRis);	// MV 10.06.2014. Per utilizzare la nuova DLL 3.0.1.0
			errorcode = err * 100;
			CString ventRis2 = pRis;
			CString temp1;
			if (err >= 0 || err == -8)
			{
				int pos2 = 0;
				ventRis2.Replace(_T(","), _T("."));
				temp1 = ExtractString(ventRis2, &pos2, _T(";"));
				// potenza assorbita alla rete elettrica
				double m_kwmaxv = _tstof(ExtractString(ventRis2, &pos2, _T(";")));
				m_kwmaxv /= 1000;
				double m_l3A100 = m_kwmaxv;
				
				//corrente ampere
				double m_l4A800 = _tstof(ExtractString(ventRis2, &pos2, _T(";"))); // current draw
				double m_cuscfu = m_l4A800;
				double m_l4A025 = _tstof(ExtractString(ventRis2, &pos2, _T(";"))); // Nominal fan data speed
				double m_nlimit = m_l4A025;
				
				
				int pos3 = 9;
				CString pr = ExtractString(ventRis2, &pos3, _T(";ERP"), _T(";IP "));
				if (pr.Find(_T("54")) > -1)
					double m_l5A100 = 54;
				else
					double m_l5A100 = 55;
				for (int i = 0; i < 26 ; i++)
					(ExtractString(ventRis2, &pos2, _T(";")));
				double kfactor = _tstof(ExtractString(ventRis2, &pos2, _T(";")));



				char* SIZE = new char[2001]; char* TYP = new char[2001];	char* ISOCLASS = new char[2001];	char* PROTECTION = new char[2001];
				SIZE[0] = TYP[0] = ISOCLASS[0] = PROTECTION[0] = 0;
				int z1 = 0, z2 = 0;	double output[52]; double output2[51];
				double input[13];
				int s1 = 0, s2 = 0;


				err = GET_STANDARDS_FANMOTOR(cMBBuffer, SIZE, TYP, ISOCLASS, PROTECTION, z1, z2, &output[0]);	// MV 10.06.2014. Per utilizzare la nuova DLL 3.0.1.0

				int pos = 0;
				/*for (short i = 0; i < 52; i++)
				{
					output[i] = _tstof(ExtractString(ventRis, &pos));
				}*/
				err = 0;

				double m_l4A100 = 230;
				double m_l4A200 = 1;
				double peso = 0;
				double maxport = 0;
				double maxpd = 0;
				double minport = 0;
				double minpd = 0;
				if (err == 0 || err == -8)
				{
					m_l4A100 = output[6];		// Voltaggio
					m_l4A200 = output[7];		//Numero di fasi
					maxport = output[2];		// max airflow m3/h
					maxpd = output[4];		// pf max 
					peso = output[15];		// weight FS 29.02.2020

				}
				else
				{
					// Corrente nominale motore
					if (temp1.Find(_T("240")) > -1)
					{
						m_l4A100 = 230;
						m_l4A200 = 3;
					}
					if (temp1.Find(_T("277")) > -1)
					{
						m_l4A100 = 230;
						m_l4A200 = 1;
					}
					if (temp1.Find(_T("380")) > -1)
					{
						m_l4A100 = 400;
						m_l4A200 = 3;
					}
					if (temp1.Find(_T("200")) > -1)
					{
						m_l4A100 = 200;
						m_l4A200 = 1;
					}
					//gf 9-5-17 tensione ebm uscita su ventservice e prima non gestita
					if (temp1.Find(_T("400")) > -1)
					{
						m_l4A100 = 400;
						m_l4A200 = 3;
					}
				}


				SAFE_ARRAY_DELETE(SIZE);
				SAFE_ARRAY_DELETE(TYP);
				SAFE_ARRAY_DELETE(ISOCLASS);
				SAFE_ARRAY_DELETE(PROTECTION);

				//cerco la massima pressione per la portata passata
				double pressh = 10000;
				double presss = 5000;
				double pressl = 0;
				int errore = 0;
				do
				{
					sprintf_s(cMBBuffer, 4000, "%s;0;0;%.4f;;%.2f;%.4f;%.4f;%.2f;%.2f;ebmpapst;0;F;4000", fanmodel.GetString(), dens, temp, presss, port, maxWidth, maxHeigth);
					char cMBOut[4001]; ZeroMemory(cMBOut, 4001 * sizeof(char)); char* pRisr = &cMBOut[0];
					errore = GET_CALCULATION_FAN_ALONE_PC(&cMBBuffer[0], &pRisr);
					if (errore == 0)
					{
						ventRis = cMBOut;
						::MessageBox(NULL, ventRis, _T(""), MB_OK);
						int pos1 = 0;
						ventRis.Replace(_T(","), _T("."));
						double nfunzi = _tstof(ExtractString(ventRis, &pos1, _T(";")));
						if (nfunzi > 0)
						{
							pressl = presss;
							presss = (presss + pressh) / 2.0;
						}
						else
						{
							pressh = presss;
							presss = (presss + pressl) / 2.0;
						}
					}
				} while (pressh - pressl > 1);
				calcok = true;
				
				//json output:
				Writer<StringBuffer> writer(s);
				writer.StartObject();
					writer.Key("result"); 
					writer.StartObject();
						writer.Key("performance");
						writer.StartObject();
						if (pTot > 0)
						{
							writer.Key("motorabspower"); writer.Double(m_kwfunz);
							writer.Key("motorelecpower"); writer.Double(m_kwmaxv);
							writer.Key("Rpm"); writer.Double(m_nfunzi);
							double sfp = (m_kwmaxv / port) * 1000.0; //sfp W / m3 / s
							writer.Key("sfp_class"); writer.Int(GetSFPClass(sfp));
							writer.Key("sfp_value"); writer.Double(sfp);
							writer.Key("supplynoise"); writer.Double(m_lw3Afu);
						}
						writer.EndObject();
						writer.Key("configuration");
						writer.StartObject();
							writer.Key("fanname"); writer.String(fanmodel.GetString());
							double formula = 0;
							writer.Key("fanfactor"); writer.Double(formula);
							writer.Key("keyfactor"); writer.Double(kfactor); // kfactor
							writer.Key("weight"); writer.Double(peso);
							writer.Key("minairflow"); writer.Double(minport);
							writer.Key("maxaiflow"); writer.Double(maxport);
							writer.Key("mindp"); writer.Double(minpd);
							writer.Key("maxdp"); writer.Double(maxpd);
							
						writer.EndObject();
						writer.Key("noise");
						writer.StartObject();
							CString in, out;
							in.Format("%.0f;%.0f;%.0f;%.0f;%.0f;%.0f;%.0f;%.0f", m_l7w006, m_l7w012, m_l7w025, m_l7w050, m_l7w100, m_l7w200, m_l7w400, m_l7w800);
							out.Format("%.0f;%.0f;%.0f;%.0f;%.0f;%.0f;%.0f;%.0f", m_l6w006, m_l6w012, m_l6w025, m_l6w050, m_l6w100, m_l6w200, m_l6w400, m_l6w800);
							writer.Key("inlet"); writer.String(in);
							writer.Key("outlet"); writer.String(out);
						writer.EndObject();
					writer.EndObject();
					writer.Key("erroid"); writer.Int(errorcode);
					writer.Key("version"); writer.String(VERSION);
				writer.EndObject();
			}

			
			
			
		
		}
	}
exit:
	String^ JSONout;
	if (!calcok)
	{
		//json output:
		Writer<StringBuffer> writer(s);
		writer.StartObject();
		writer.Key("erroid"); writer.Int(errorcode);
		writer.Key("version"); writer.String(VERSION);
		writer.EndObject();
	}
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
	String^ err;
	return  err;
}
String^ Rooftop::GetNoiseData(String^ jSONIN) 
{
	String^ err;
	return  err;
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

	
	if (port[1] * port[0] == 0)
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
			
			//se type � > 0 si devono applicare delle formule diverse per il calcolo delle perdite di carico
			if (tipo == 1)
			{
				double Nfiltri = 1;
				std::string filter1 = "";
				g_ModelTable.AddFilterField("Nomcomm", "=", model, filter1);
				CGenTableRecord filtri = g_ModelTable.Lookup(filter1);
				long n1 = 1, n2 = 0;
				filtri.GetColumn("Filter_quantity", n1);
				filtri.GetColumn("Filter_quantity2", n2);
				if (n1 + n2 == 0)
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
String^ Rooftop::SearchModel(CString model, CString fantype)
{
	String^ jsoinrecordset;

	//g_ModelTable.ResetFilter();
	std::string filter;
	g_ModelTable.AddFilterField("Nomcomm", "=", model, filter);
	CGenTableRecord modello = g_ModelTable.Lookup(filter);

	if (!modello.IsValid())
		return jsoinrecordset;

	long nfan = 1;
	CString test;
	if (fantype.IsEmpty() || fantype == _T("STD"))
	{
		modello.GetColumn("Indoor_Fan_STD", test);
		modello.GetColumn("Indoor_Qty_Fan_STD", nfan);
	}
	if (fantype == _T("SFHC"))
	{
		modello.GetColumn("Indoor_Fan_SFHC", test);
		modello.GetColumn("Indoor_Qty_Fan_SFHC", nfan);
	}
	if (fantype == _T("SFLA"))
	{
		modello.GetColumn("Indoor_Fan_SFLA", test);
		modello.GetColumn("Indoor_Qty_Fan_SFLA", nfan);
	}
	if (fantype == _T("SFHA"))
	{
		modello.GetColumn("Indoor_Fan_SFHA", test);
		modello.GetColumn("Indoor_Qty_Fan_SFHA", nfan);
	}
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

		//::MessageBox(NULL, fan1, _T("modello fan"), MB_OK);
		//::MessageBox(NULL, fan, _T("modello fan"), MB_OK);
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
	if (!GET_STANDARDS_FANMOTOR)
	{

		return false;
	}
	
	return true;
}


