#ifndef GASFILE

#define GASFILE

// gas.h : header file
//

#define ARIA	1
#define AZOTO	2
#define MISCELA 3
#define VAPOREA 4

#define ACETATODIETILE		100
#define DIMETILFORMAMIDE	101
#define METILACETATO		102
#define METANOLO			103
#define ETANOLO				104
#define ACETONE				105
#define VAPOREACQUA			106
#define PERCLOROETILENE		107

class  CGas
{   

public:
	double ClTBulboUmido(double ts, double um, double press);
	CGas();			

// Attributes
public:
	
// Operations
public:

// Implementation
public:

	double m_kgfinali;
	virtual double PesoMolecolare(){return 29.8;};
	double ClTEnt(double ent, double t1, double t2, double x1, double x2, double pres);
	double ClUmRel(double tg, double x, double pres);
	double			ClTParete(double hsat, double tmax, double tmin, double press) const;
	virtual ~CGas();
	virtual double 	Conducivita(double temp) const;
	virtual double 	CaloreSpecifico(double temp,double um, double press) const;
	virtual double 	Densita(double temp, double um, double pressione) const;
	virtual double 	Viscosita(double temp, double um) const;
	virtual double 	ClEntalpia1(double t, double w, double press) const;
	virtual double 	ClPresSat(double temp) const;
	virtual double 	ClTempSat(double pres){ASSERT(false);return -99;};
	virtual double 	ClUmAss(double t, double um, double pres, bool graph = false) const;
	virtual double	ClTRug(double t, double ent, double press);
	virtual double  ClTRug2(double x, double press);
	virtual double	CaloreLatenteEvap(){ASSERT(false);return -99;};
	virtual double	ClUm(double ent, double t, double press);
	virtual double	TempBoll(){return 100;};
	double ClUmRelWB(double drybulb, double wetbulb, double pressure) const noexcept;
	virtual double  ClRappPesi() const;

//	FLUIDO   		Proprieta(double temp, double um, double press);
    double 			ClEntalpia(double t, double um, double press) const;
	double			ClEntalpia2(double tm, double trug, double press) const;
	double			ClTRug1(double t, double um, double press);
	
    double 			ClUmSpec(double tb, double ts, double press);
	double 			ClTEntUm(double ent, double um, double tmin, double tmax, double press);

	// coefficienti Legge di Antoine
	double a, b, c;
	double m_frazionemolare;
	double m_peso;
	double m_nMoli;
	CString nome;
    
protected:
//	FLUIDO	fluido;
	
};

class  CAzoto: public CGas
{   

public:
	double PesoMolecolare(){return 28.0134;};
	double 	Conducivita(double temp)const;
	double 	CaloreSpecifico(double temp,double um, double press)const;
	double 	Densita(double temp, double um, double pressione)const;
	double 	Viscosita(double temp, double um)const;
	double 	ClUmAss(double t, double um, double pres, bool graph = false) const;
	double  ClRappPesi() const override;
};

class  CVapore: public CGas
{   

public:
	double PesoMolecolare(){return 18.0;};
	double	TempBoll(){return 500.0;};
	double 	ClPresSat(double temp)const;
	double 	ClTempSat(double pres);
	double	CaloreLatenteEvap();
	double  Conducivita(double temp)const;
	double 	Densita(double temp, double um, double pressione)const;
	double 	Viscosita(double temp, double um)const;
	double 	CaloreSpecifico(double temp,double um, double press)const;
};



class  CEtilAcetato: public CGas
{   

public:
	double PesoMolecolare(){return 88.12;};
	double	TempBoll(){return 77.06;};
	double 	CaloreSpecifico(double temp,double um, double press);
	double 	ClPresSat(double temp);
	double 	ClTempSat(double pres);
	double	CaloreLatenteEvap();
	
};

class  CMetilAcetato: public CGas
{   

public:
	double PesoMolecolare(){return 74.0;};
	double	TempBoll(){return 57.0;};
	double 	CaloreSpecifico(double temp,double um, double press);
	double 	ClPresSat(double temp);
	double 	ClTempSat(double pres);
	double	CaloreLatenteEvap();
	
};


class  CDMF: public CGas
{   
	// Dimetilformamide
public:
	double PesoMolecolare(){return 73.0;};
	double	TempBoll(){return 153.0;};
	double 	CaloreSpecifico(double temp,double um, double press);
	double 	ClPresSat(double temp);
	double 	ClTempSat(double pres);
	double	CaloreLatenteEvap();
	
};

class  CMetanolo: public CGas
{   

public:
	double PesoMolecolare(){return 32.0;};
	double	TempBoll(){return 64.0;};
	double 	CaloreSpecifico(double temp,double um, double press);
	double 	ClPresSat(double temp);
	double 	ClTempSat(double pres);
	double	CaloreLatenteEvap();
	
};


class  CEtanolo: public CGas
{   

public:
	double  PesoMolecolare(){return 46;};
	double	TempBoll(){return 78.0;};
	double 	CaloreSpecifico(double temp,double um, double press);
	double 	ClPresSat(double temp);
	double 	ClTempSat(double pres);
	double	CaloreLatenteEvap();
	
};

class  CPercloroEtilene: public CGas
{   

public:
	double  PesoMolecolare(){return 166;};
	double	TempBoll(){return 121.0;};
	double 	CaloreSpecifico(double temp,double um, double press);
	double 	ClPresSat(double temp);
	double 	ClTempSat(double pres);
	double	CaloreLatenteEvap();
	
};


class  CAcetone: public CGas
{   

public:
	double PesoMolecolare(){return 58.0;};
	double	TempBoll(){return 56.0;};
	double 	CaloreSpecifico(double temp,double um, double press);
	double 	ClPresSat(double temp);
	double 	ClTempSat(double pres);
	double	CaloreLatenteEvap();
	
};

class  CMiscela: public CGas
{   

public:
	
	double ClPesoTotale();
	virtual ~CMiscela();
	CMiscela();

	double 	Conducivita(double temp);
	double 	CaloreSpecifico(double temp,double um, double press);
	double 	Densita(double temp, double um, double pressione);
	double 	Viscosita(double temp, double um);
	double 	ClEntalpia1(double t, double w, double press);
	double 	ClPresSat(double temp);
	double 	ClUmAss(double t, double um, double pres);
	double	ClTRug(double t, double ent, double press);
	double	ClUm(double ent, double t, double press);
	double	TempBoll(){ASSERT(m_pVapore);return m_pVapore->TempBoll();};
	double  ClRappPesi() const override;
	double PesoMolecolare();
	CTypedPtrArray<CPtrArray, CGas*> m_componenti;
	void	AddComponent(int tipo,double peso=0);
	CString	GetComponentName(int pos);
	int m_nComponenti;

protected:
	void CalcolaFrazioniMolari();
	double ClMoliTotali();
	CGas* m_pVapore;
	CGas* m_pGas;

};





#endif // ifndef GASFILE
/////////////////////////////////////////////////////////////////////////////
