// pch.h: questo è un file di intestazione precompilata.
// I file elencati di seguito vengono compilati una sola volta, in modo da migliorare le prestazioni per le compilazioni successive.
// Questa impostazione influisce anche sulle prestazioni di IntelliSense, incluso il completamento codice e molte altre funzionalità di esplorazione del codice.
// I file elencati qui vengono però TUTTI ricompilati se uno di essi viene aggiornato da una compilazione all'altra.
// Non aggiungere qui file soggetti a frequenti aggiornamenti; in caso contrario si perderanno i vantaggi offerti in termini di prestazioni.

#ifndef PCH_H
#define PCH_H
#define _AFXDLL

// aggiungere qui le intestazioni da precompilare
//#include <adoid.h>
#include <adoint.h>
#include "..\msadox.tlh"
#include <oledb.h> 
#include "icrsint.h"  
#include <atldbcli.h>
#include <afxtempl.h>		// MFC Template classes

#define VERSION "1.0.0.2"

#define PIGRECO			3.1415928
constexpr long lenRis = 1000000;

#define SAFE_DELETE(x)			{ if((x)){delete (x); (x)=NULL;} }  
#define SAFE_ARRAY_DELETE(x)	{ if((x)){delete[] (x); (x)=NULL;} }

#define THROW_ERR(exp)	if (FAILED(hr = (exp))) throw hr

static const GUID CONGUID =
{ 0x0000514, 0, 0x10, 0x80,0,0,0xAA,0,0x6D,0x2E,0xA4 };

static const GUID CONINTGUID =
{ 0x0000550, 0, 0x10, 0x80,0,0,0xAA,0,0x6D,0x2E,0xA4 };

static const GUID RECGUID =
{ 0x0000535, 0, 0x10, 0x80,0,0,0xAA,0,0x6D,0x2E,0xA4 };

static const GUID RECINTGUID =
{ 0x000054F, 0, 0x10, 0x80,0,0,0xAA,0,0x6D,0x2E,0xA4 };


#endif //PCH_H
