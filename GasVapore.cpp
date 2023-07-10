// gas.cpp : implementation file
//

#include "pch.h"

//#include "psicro.h"
//#include "..\hhsd\calc.h"
#include "gasvapore.h"
#include "math.h"




#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGas

CGas::CGas()
{
	m_frazionemolare = 0;
	m_kgfinali= a = b = c = m_peso=0;
}

CGas::~CGas()
{
}
 
double CGas::Densita(double temp, double um, double pressione)  const
{              
	/*
		CALCOLO DENSITA
		pressione in bar
	*/

	double d = pressione*100*3.485/(temp+273.15); // ricavata da PV=nRT ->ro = 1/Vm = P*PM/(RT)
	if(um == 0)	return d;					// PesoMolecolare aria= 28.98 R = 8.314

	double w = ClUmAss(temp, um, pressione)/1000;
	if (w < 0) return -999.0;
	
	d = d* ClRappPesi()*(1+w) / (ClRappPesi()+w);
	return d;
};

double CGas::Conducivita(double temp) const
{
	temp = temp + 273.16;
	double cta = (-0.00000003381*pow(temp,2)) + (0.00009814*temp)-0.0001308;
	if (temp > 600) cta=(0.05227 * temp + 15.27)*0.001;
	
	return cta;

};

double CGas::CaloreSpecifico(double temp, double um, double press)  const
{
	double ta = 273.15+temp;
	double cpa = -0.000003798*pow(ta,3)+0.00417*pow(ta,2)-1.45*ta+1168.8;
	if (ta >450) cpa = 1022*(1+(temp-200)*0.00022);

	if (um == 0) return cpa;
   

	double cpv = -0.000000367*temp+1835;
	double w = ClUmAss( temp, um, press)/1000;     // Umidita assoluta
	cpa=(cpa+(w*cpv))/(1+w);

	return cpa;
}	

	

double CGas::Viscosita(double temp, double um) const
{                        

	double tm= temp+273.15;
	double va;
	if (tm < 600)
   		va = -0.00000000002858*pow(tm,2) + 0.00000006605*tm +0.0000012;
	else
   		va = 0.00000002717*tm+0.000014277;
	

	return va;
};


/*FLUIDO CGas::Proprieta(double temp, double um, double pressione)
{
	
	fluido.visc  = Viscosita(temp,um);
	fluido.cond  = Conducivita(temp);
	fluido.calsp = CaloreSpecifico(temp,um,pressione);
	fluido.dens  = Densita(temp,um,pressione);
	
			
	return fluido;
}*/		
    
double CGas::ClUmAss(double temp, double um, double pressione, bool graph) const
{
	// calcolo umidita assoluta da temp. e umidità relativa
    // gr/Kg
	double w;

	
	if (um == 0)
		return 0;
	//ASSERT(um < 101);
	if (um > 100 && !graph) um = 100;

	if (um < 0)
	{
		//ASSERT(FALSE);
		return -99.0;
	}		
			
    double pg = ClPresSat(temp);  	// pressione vapore saturo a Tm
	double ps = pg * um / 100;   	// Umidità rel. entrata aria
	w  = ClRappPesi() * ps / (pressione * 100 - ps);     // Umidita assoluta
	ASSERT(w>=0);
	
	return w*1000;
}	

double CGas::ClPresSat(double temp) const
{
	// calcolo pressione vapore saturo a temperatura temp.
	// Il valore ritornato (Pres. vapore saturo) è in KN/m2 (Kpa = bar*100)
	// da CETIAT
    // Valida da 0 a 300 øC

	double pve = (16.826686 * temp)/(228.73733+temp)+6.4075;
	pve = exp(pve)/1000;
	return pve;
}	           

double CGas::ClEntalpia(double tm, double um, double press) const
{
	//ASSERT (tm > -50);
	#ifdef _DEBUG
	if (tm < -50)
	{
		//AfxMessageBox(L"Entalpia: tm < -50");
   		tm = -50.0;
	}
	#endif
	double w = ClUmAss(tm , um , press)/1000;  // Umidita assoluta
	
    return ClEntalpia1(tm, w, press);

}	

double CGas::ClEntalpia2(double tm, double trug, double press) const
{
	/*
	   	Calcolo entalpia in J/Kg
	*/

	ASSERT (tm>=trug);

	double um = ClPresSat(trug)/ClPresSat(tm)*100;
	
    return ClEntalpia(tm, um, press);

}	


double CGas::ClEntalpia1(double tm, double w, double press) const
{

	double htm = CaloreSpecifico(tm,0, press)*tm + w*(595.0 + (0.46*tm))*4185;
	//htm = ClCalSpGas(tm,0)*tm + w*1000*(1.846*tm-0.0006699*tm^2 + 2500.83)
	//hah=cpa*t+1000.*r1*((1.846*t-6.699e-04*t*t)+2500.83)
    return htm;

}


double CGas::ClTRug2(double x, double press)
{
	double tmin = -50;
	double tmax = 80; // o un numero che ti piace

	for (;;)
	{

		double tp = (tmin+tmax)/2.0;
		double w = ClUmAss(tp,100,press);
		if (fabs(w-x)< 0.01 || fabs(tmax - tmin) < 0.001)
			return tp;


		if (x < w)
			tmax = tp;

		else
			tmin = tp;
	}

	return 0;
}
	

double CGas::ClTRug1(double t, double um, double press)
{
	double h = ClEntalpia(t,um,press);
	return ClTRug(t,h,press);
}

double CGas::ClTRug(double t, double h, double press)
{
	double wout = (h-CaloreSpecifico(t, 0.0, press) * t)/((595.0+0.46*t)*4185.0);
	double ptr = 1000*(wout*press*100.0/(ClRappPesi()+wout));
	if (ptr < 0.00001)	return -50.0;
	

	double trose = 228.73733*(6.4075-log(ptr))/(log(ptr)-23.2344186);
	if (fabs(trose - t) < 0.001) trose=t;
	return trose;
}	
	
	
double CGas::ClUmSpec(double tb, double ts, double press)
{
	double xs = ClPresSat(tb);
	if (xs > press*100) return -999.0 ;

	xs = ClRappPesi() * xs / (press * 100-xs);  // Umidita specifica

	xs  = 1.006*(tb-ts)+xs*(2501-2.364*tb);
	xs /= 2501+1.83*ts-4.195*tb;

	return xs*1000;
}	
	
double CGas::ClUm(double ent, double temp, double press)
{
	//if (ent == 0) return 0;  // Entalpia nulla
   
	double wout = (ent-CaloreSpecifico(temp,0.0, press)*temp)/((595.0+0.46*temp)*4185.0);
	//ASSERT (wout>0);
	double ps = wout * press * 100/(ClRappPesi()+wout);
	double pg = ClPresSat(temp);
	double um = ps*100/pg;
		
	return um;
}

double CGas::ClUmRelWB(double drybulb, double wetBulb, double press) const noexcept
{
	double pg = ClPresSat(drybulb);
	double umass = CGas().ClUmSpec(wetBulb, drybulb, press) / 1000;
	double ps = umass * press * 100 / (umass + 0.622);
	return ps * 100 / pg;
}


double CGas:: ClTEntUm(double ent, double um, double tmin, double tmax, double press)
{
	/*
	   	calcola la temperatura alla quale l'entalpia calcolata con umidità rel. pari a um
   		è pari a Ent)
	*/

	for (;;)
	{
   		double tp = (tmin+tmax)/2.0;
   		double htp = ClEntalpia(tp,um,press);
	   	if (fabs(htp-ent)< 50.0 || fabs(tmax - tmin) < 0.001)
    	  	return tp;
   		
   		if (ent < htp)
      		tmax = tp;
   		else
      		tmin = tp;
   		
	}

	return 0;
}



double CGas::ClTBulboUmido(double ts, double um, double press)
{

	if (um == 0)
		return 0.0;

	double w  = ClUmAss(ts,um,press);

	if (w == -99) return -99;
	double tp;


	double tmin = -30;
	double tmax = ts;

	while (-1)
	{
		tp = (tmin+tmax)/2;
		double xc = ClUmSpec(tp, ts,press);
		if (fabs(xc-w) < 0.001 || tmax == tmin) break;
      
		if (w < xc || xc == -999)
			tmax = tp;
		else
			tmin = tp;
   	}

	return tp;
}



double CGas::ClUmRel(double tg, double x, double pres)
{
	x/=1000;
	double pg = ClPresSat(tg);  // pressione vapore saturo a Tin
	double ps = pres*100*x/(ClRappPesi()+x);
	return ps/pg*100;
	

}

double CGas::ClTEnt(double hsat, double t1, double t2, double x1, double x2, double pres)
{
	double tp;

	double tmin = -30;
	double tmax = __max(t1, t2)+30;

	while(true)
	{
		tp = (tmin+tmax)/2;
		double xout = x1 + (tp - t1)/(t2 - t1)*(x2 - x1);
		double um = ClUmRel(tp, xout, pres);
		if (um < 0)
		{
			tmin = tp;
			continue;
		}
		if (um>100) um = 100;
        double htp = ClEntalpia(tp,um,pres);
		if (fabs(htp-hsat) < 5 || fabs(tmax - tmin) < 0.00001) break;
      
		if (hsat < htp || htp == -999)
			tmax = tp;
		else
			tmin = tp;
   
	}

	return tp;

}

///////////////////////////////////////
// Azoto


double CAzoto::Densita(double temp, double um, double pressione)  const
{              
	/*
		CALCOLO DENSITA
		pressione in bar
	*/


	double d = pressione*100*3.3694/(temp+273.15); // ricavata da PV=nRT ->ro = 1/Vm = P*PM/(RT)
	if(um == 0)	return d;					// PesoMolecolare azoto= 28.0134 R = 8.314
	
	double w = ClUmAss(temp, um, pressione)/1000;
	if (w < 0) return -999.0;
	
	d = d* 0.6432*(1+w) / (0.6432+w);
	return d;
};

double CAzoto::Conducivita(double temp) const
{
	double cta = -2e-8*pow(temp,2) + 7e-5*temp+0.0242;
	return cta;

};


double CAzoto::CaloreSpecifico(double temp, double um, double press) const
{ 
	
	double cpa;

	
	if (temp <-60) 
		cpa = 0.005*pow(temp, 2) + 0.65*temp+1061;
	else
		cpa = 1038;

	if (um == 0) return cpa;
   
	double cpv = -0.000000367*temp+1835;
	double w = ClUmAss( temp, um, press)/1000;     // Umidita assoluta
	cpa=(cpa+(w*cpv))/(1+w);

	return cpa;
}	

	

double CAzoto::Viscosita(double temp, double um) const
{
	double va = (-5e-8*pow(temp,2) + 5e-5*temp + 0.0166)/1000;

	return va;
};

double CAzoto::ClUmAss(double temp, double um, double pressione, bool) const
{

	double w;

	if (um == 0)
		return 0;
	if (um > 100.1 || um < 0)
	{
		ASSERT(FALSE);
		return -99.0;
	}		
			
    double pg = ClPresSat(temp);  	// pressione vapore saturo a Tm
	double ps = pg * um / 100;   	// Umidità rel. entrata aria
	w  = 0.6432 * ps / (pressione * 100 - ps);     // Umidita assoluta
	ASSERT(w>=0);
	
	return w*1000;
}


CMiscela::CMiscela()
{
	m_componenti.SetSize(5);
	m_nComponenti = 0;
	m_pGas = NULL;
	m_pVapore = NULL;
}


CMiscela::~CMiscela()
{
	for (int i = 0; i < m_componenti.GetSize();i++)
	{
		delete m_componenti[i];
	}
	m_componenti.RemoveAll();

}
double 	CMiscela::Conducivita(double temp)
{
	
	return m_pGas->Conducivita(temp);
	//return 0;
}

double 	CMiscela::CaloreSpecifico(double temp,double um, double press)
{
	// J/Kg°C
	double x = ClUmAss(temp,um,press)/1000;
	double cp1 = m_pGas->CaloreSpecifico(temp,0,press);
	double cp2 = m_pVapore->CaloreSpecifico(temp,0,press);
	return (cp1+cp2*x)/(1+x);
	/*double calsp = 0;
	for (int i =0;i<m_nComponenti;
		calsp+=m_componenti[i]->m_frazionemolare*m_componenti[i]->CaloreSpecifico(temp,0,press));

	return calsp;*/

}

double 	CMiscela::Densita(double temp, double um, double pressione)
{
	// Kg/m3
	double rapp = ClRappPesi();
	double x = ClUmAss(temp,um,pressione)/1000;
	return m_pGas->Densita(temp,0,pressione)*rapp*(1+x)/(rapp+x);

	//return pressione*100*PesoMolecolare()/(8.314*temp);
	
}

double 	CMiscela::Viscosita(double temp, double um)
{
	return m_componenti[0]->Viscosita(temp,0);
	
}

double 	CMiscela::ClEntalpia1(double t, double w, double press)
{
	//J/Kg
	return m_pGas->CaloreSpecifico(t,0,press)*t + w*( m_pVapore->CaloreLatenteEvap() + 
		m_pVapore->CaloreSpecifico(t,0,press)*t);
	/*double entalpia = 0;
	for (int i =0;i<m_nComponenti;
		entalpia+=m_componenti[i]->m_frazionemolare*m_componenti[i]->ClEntalpia(t,0,press));

	return entalpia;*/
}

double 	CMiscela::ClPresSat(double temp)
{
	ASSERT(m_pVapore);
	return m_pVapore->ClPresSat(temp);
	//return 0;
}


double 	CMiscela::ClUmAss(double t, double um, double pres)
{
	// Pres è in bar
	// Ritorna g/kg
	if (um== 0) return 0;
	ASSERT( um <= 100);
	double ps = m_pVapore->ClPresSat(t); // kPa
	double pv = um/100*ps; // kPa
	double w = m_pVapore->PesoMolecolare()/m_pGas->PesoMolecolare()*(pv / (pres*100 - pv));
	ASSERT(w>0);
	return w*1000;
	//return 0;

}

double	CMiscela::ClTRug(double t, double ent, double press)
{

	double wout  = (ent-m_pGas->CaloreSpecifico(t, 0.0, press) * t)/(
		(m_pVapore->CaloreLatenteEvap()+m_pVapore->CaloreSpecifico(t,0,press)*t));


	double ptr   = (wout*press*100.0/( m_pVapore->PesoMolecolare()/m_pGas->PesoMolecolare()+wout));
	if (ptr < 0.00001)	return -50.0;

	return m_pVapore->ClTempSat(ptr);
	//return 0;
	
}


double 	CEtilAcetato::CaloreSpecifico(double temp,double um, double press)
{
	// 40.4 cal/mole/°C

	return 40.4/88.12*4185;
}

double 	CEtilAcetato::ClPresSat(double temp)
{
	//ASSERT(temp< TempBoll());
	// Perchè l'equazione di Antoine non dà per t=tboll P=101.325 esattamente
	//if (temp > (p=TempBoll()-1))
	//	temp = p;
	// L'equazione di Antoine dà pressione in mmHg
	return pow(10.,(7.10179-1244.950/(217.881+temp)))*13.596/100;
}

double 	CEtilAcetato::ClTempSat(double pres)
{
	// E' l'equazione inversa della precedente
	double temp = 1244.950/(7.10179  - log(pres*100/13.596)/log(10.)) - 217.881;
	return temp;
}
double	CEtilAcetato::CaloreLatenteEvap()
{
	// 7744 cal/mol
	return 7744/88.12*4185;
	
}

double 	CDMF::CaloreSpecifico(double temp,double um, double press)
{
	// 40.4 cal/mole/°C

	return 35.5/73*4185.0;
}

double 	CDMF::ClPresSat(double temp)
{
	//ASSERT(temp< TempBoll());
	//double p;
	// Perchè l'equazione di Antoine non dà per t=tboll P=101.325 esattamente
	//if (temp > (p=TempBoll()-1))
	//	temp = p;
	// L'equazione di Antoine dà pressione in mmHg
	return pow(10.,(7.1085-1537.78/(210.39+temp)))*13.596/100;
}

double 	CDMF::ClTempSat(double pres)
{
	// E' l'equazione inversa della precedente
	double temp = 1537.78/(7.1085  - log(pres*100/13.596)/log(10.)) - 210.39;
	return temp;
}

double	CDMF::CaloreLatenteEvap()
{
	// 10074 cal/mol
	return 10074.0/73.0*4185.0;
	
}

double	CMiscela::ClUm(double ent, double t, double press)
{
	double wout;

	if (ent == 0) return 0;  // Entalpia nulla
   
	wout  = (ent-m_pGas->CaloreSpecifico(t, 0.0, press) * t)/(
		(m_pVapore->CaloreLatenteEvap()+m_pVapore->CaloreSpecifico(t,0,press)*t));
	ASSERT (wout>0);
	double ps = (wout*press*100.0/( ClRappPesi()+wout));
	//ps    = wout * press * 100/(0.622+wout);
	double pg = ClPresSat(t);
	double um = ps*100/pg;
		
	return um;
	
	//return 0;
}


CString	CMiscela::GetComponentName(int pos)
{
	ASSERT(pos >= 0 && pos < m_nComponenti);
	return m_componenti[pos]->nome;
}

void CMiscela::AddComponent(int tipo,double peso)
{
	switch (tipo)
	{
	case ACETATODIETILE:
		m_componenti.SetAtGrow(m_nComponenti++, new CEtilAcetato());
		break;
	case METILACETATO:
		m_componenti.SetAtGrow(m_nComponenti++, new CMetilAcetato());
		break;
	case METANOLO:
		m_componenti.SetAtGrow(m_nComponenti++, new CMetanolo());
		break;
	case ETANOLO:
		m_componenti.SetAtGrow(m_nComponenti++, new CEtanolo());
		break;
	case DIMETILFORMAMIDE:
		m_componenti.SetAtGrow(m_nComponenti++, new CDMF());
		break;
	case ACETONE:
		m_componenti.SetAtGrow(m_nComponenti++, new CAcetone());
		break;
	case AZOTO:
		m_componenti.SetAtGrow(m_nComponenti++, new CAzoto());
		break;
	case ARIA:
		m_componenti.SetAtGrow(m_nComponenti++, new CGas());
		break;
	case PERCLOROETILENE:
		m_componenti.SetAtGrow(m_nComponenti++, new CPercloroEtilene());
		break;

	default:
		ASSERT(FALSE);
	}
	m_componenti[m_nComponenti-1]->m_peso = peso;
	m_componenti[m_nComponenti-1]->m_nMoli = peso/m_componenti[m_nComponenti-1]->PesoMolecolare();

	if (m_nComponenti == 1)
		m_pGas = m_componenti[m_nComponenti-1];
	if (m_nComponenti == 2)
		m_pVapore = m_componenti[m_nComponenti-1];

	
	CalcolaFrazioniMolari();

	
}

double CMiscela::ClRappPesi() const
{
	return m_pVapore->PesoMolecolare()/m_pGas->PesoMolecolare();
	//return 0;
}

double CGas::ClRappPesi() const
{
	return 0.622;
}

double CAzoto::ClRappPesi() const
{
	return 0.6432;
}


double 	CMetilAcetato::CaloreSpecifico(double temp,double um, double press)
{
	// 40.4 cal/mole/°C

	return 40.4/74.0*4185;
}

double 	CMetilAcetato::ClPresSat(double temp)
{
	//ASSERT(temp< TempBoll());
	// Perchè l'equazione di Antoine non dà per t=tboll P=101.325 esattamente
	//if (temp > (p=TempBoll()-1))
	//	temp = p;
	// L'equazione di Antoine dà pressione in mmHg
	return pow(10.,(7.06524-1157.63/(219.726+temp)))*13.596/100;
}

double 	CMetilAcetato::ClTempSat(double pres)
{
	// E' l'equazione inversa della precedente
	double temp = 1157.63/(7.06524  - log(pres*100/13.596)/log(10.)) - 219.726;
	return temp;
}
double	CMetilAcetato::CaloreLatenteEvap()
{
	// 7178 cal/mol
	return 7178.0/78.0*4185;
	
}

double 	CAcetone::CaloreSpecifico(double temp,double um, double press)
{
	// 40.4 cal/mole/°C

	return 23.78/58.0*4185;
}

double 	CAcetone::ClPresSat(double temp)
{
	//ASSERT(temp< TempBoll());
	// Perchè l'equazione di Antoine non dà per t=tboll P=101.325 esattamente
	//if (temp > (p=TempBoll()-1))
	//	temp = p;
	// L'equazione di Antoine dà pressione in mmHg
	return pow(10.,(7.11714-1210.596/(229.664+temp)))*13.596/100;
}

double 	CAcetone::ClTempSat(double pres)
{
	// E' l'equazione inversa della precedente
	double temp = 1210.596/(7.11714  - log(pres*100/13.596)/log(10.)) - 229.664;
	return temp;
}
double	CAcetone::CaloreLatenteEvap()
{
	// 7076 cal/mol
	return 7076.0/58.0*4185;
	
}

double 	CMetanolo::CaloreSpecifico(double temp,double um, double press)
{
	// 40.4 cal/mole/°C

	return 19.5/32.0*4185;
}

double 	CMetanolo::ClPresSat(double temp)
{
	//ASSERT(temp< TempBoll());
	// Perchè l'equazione di Antoine non dà per t=tboll P=101.325 esattamente
	//if (temp > (p=TempBoll()-1))
	//	temp = p;
	// L'equazione di Antoine dà pressione in mmHg
	return pow(10.,(8.08097-1582.271/(239.726+temp)))*13.596/100;
}

double 	CMetanolo::ClTempSat(double pres)
{
	// E' l'equazione inversa della precedente
	double temp = 1582.271/(8.08097  - log(pres*100/13.596)/log(10.)) - 239.726;
	return temp;
}
double	CMetanolo::CaloreLatenteEvap()
{
	// 8426 cal/mol
	return 8426.0/32.0*4185;
	
}


double 	CEtanolo::CaloreSpecifico(double temp,double um, double press)
{
	// 40.4 cal/mole/°C

	return 27.14/46.0*4185;
}

double 	CEtanolo::ClPresSat(double temp)
{
	//ASSERT(temp< TempBoll());
	// Perchè l'equazione di Antoine non dà per t=tboll P=101.325 esattamente
	//if (temp > (p=TempBoll()-1))
	//	temp = p;
	// L'equazione di Antoine dà pressione in mmHg
	return pow(10.,(8.1122-1592.864/(226.184+temp)))*13.596/100;
}

double 	CEtanolo::ClTempSat(double pres)
{
	// E' l'equazione inversa della precedente
	double temp = 1592.864/(8.1122  - log(pres*100/13.596)/log(10.)) - 226.184;
	return temp;
}
double	CEtanolo::CaloreLatenteEvap()
{
	// 8426 cal/mol
	return 9200.0/46.0*4185;
	
}


double 	CPercloroEtilene::CaloreSpecifico(double temp,double um, double press)
{
	// 40.4 cal/mole/°C

	return 34.9/166.0*4185;
}

double 	CPercloroEtilene::ClPresSat(double temp)
{
	//ASSERT(temp< TempBoll());
	// Perchè l'equazione di Antoine non dà per t=tboll P=101.325 esattamente
	//if (temp > (p=TempBoll()-1))
	//	temp = p;
	// L'equazione di Antoine dà pressione in mmHg
	return pow(10.,(7.6293-1803.96/(258.976+temp)))*13.596/100;
}

double 	CPercloroEtilene::ClTempSat(double pres)
{
	// E' l'equazione inversa della precedente
	double temp = 1803.96/(7.6293  - log(pres*100/13.596)/log(10.)) - 258.976;
	return temp;
}
double	CPercloroEtilene::CaloreLatenteEvap()
{
	// 8426 cal/mol
	return 8316.0/46.0*4185;
	
}



double CMiscela::ClPesoTotale()
{
	double pesotot=0;
	for (int i = 0; i<m_nComponenti; pesotot+=m_componenti[i++]->m_peso);
	return pesotot;
}

double CMiscela::PesoMolecolare()
{
	// peso molecolare miscela 
	double pesomol=0;
	for (int i = 0; i<m_nComponenti; pesomol+=m_componenti[i++]->PesoMolecolare()*m_componenti[i]->m_frazionemolare );

	return pesomol;
}

double CMiscela::ClMoliTotali()
{
	double nMoli = 0;
	for (int i = 0; i<m_nComponenti; ++i)
	{
		ASSERT(m_componenti[i]->m_nMoli > 0);
		nMoli += m_componenti[i]->m_nMoli;
	}
	return nMoli;
}

void CMiscela::CalcolaFrazioniMolari()
{
	double nmoli = ClMoliTotali();
	
	for ( int i = 0; i<m_nComponenti; 
		m_componenti[i]->m_frazionemolare=m_componenti[i]->m_nMoli/nmoli, 
			TRACE(L"%s: Ni=%.3f\n", m_componenti[i]->nome,m_componenti[i]->m_frazionemolare),
			++i);
	return;

}


double CVapore::CaloreSpecifico(double temp, double um, double press) const
{
	// Valori per 1bar Fonte CANUT
	double cpv=-0.17158e-7*pow(temp,3) + 0.15141e-4*pow(temp,2) -
		0.39187e-2*temp + 2.288;

	return cpv*1000;
	//0.46*4185;
}

double 	CVapore::ClPresSat(double temp)const
{
	double pve = (16.826686 * temp)/(228.73733+temp)+6.4075;
	pve = exp(pve)/1000;
	return pve;
}

double 	CVapore::ClTempSat(double pres)
{
	// E' l'equazione inversa della precedente
	// Pres in kPa
	double trose = 228.73733*(6.4075-log(pres))/(log(pres)-23.2344186);
	return trose;
	
}

double	CVapore::CaloreLatenteEvap()
{
	
	return 595*4185;
	
}

double 	CVapore::Densita(double temp, double um, double pressione)const
{
	double d = pressione*100*2.1674/(temp+273.15); // ricavata da PV=nRT ->ro = 1/Vm = P*PM/(RT)
	if(um == 0)	return d;					// PesoMolecolare acqua= 18.02 R = 8.314
	
	return d;
}
double 	CVapore::Viscosita(double temp, double um)const
{
	// ritorna mPa.s
	// 1.013 bar: dati CANUT

	double mu = 0.0413*temp+7.95;
	return mu/1000000;
}

double CVapore::Conducivita(double temp) const
{
	double cta = 0.71167e-4*pow(temp,2) + 6.49e-2*temp+17.41;

	return cta/1000;

}

double  CGas::ClTParete(double hsat, double tmax, double tmin, double press) const
{

	double 	tp, htp;
	if (tmin >= tmax)
	{
		//	ASSERT(false);
		return tmax;
	}

	while (-1)
	{
		tp = (tmin + tmax) / 2;
		htp = ClEntalpia(tp, 100.0, press);
		if (fabs(htp - hsat) < 20.0 || fabs(tmax - tmin) < 0.005)
		{
			return tp;
		}
		if (hsat < htp)
			tmax = tp;
		else
			tmin = tp;
	}

}



