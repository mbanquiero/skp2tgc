
// skp2tgc.h: archivo de encabezado principal para la aplicaci�n PROJECT_NAME
//

#pragma once

#ifndef __AFXWIN_H__
	#error "incluir 'stdafx.h' antes de incluir este archivo para PCH"
#endif

#include "resource.h"		// S�mbolos principales


// Cskp2tgcApp:
// Consulte la secci�n skp2tgc.cpp para obtener informaci�n sobre la implementaci�n de esta clase
//

class Cskp2tgcApp : public CWinApp
{
public:
	Cskp2tgcApp();

// Reemplazos
public:
	virtual BOOL InitInstance();

// Implementaci�n

	DECLARE_MESSAGE_MAP()
};

extern Cskp2tgcApp theApp;