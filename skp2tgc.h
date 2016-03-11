
// skp2tgc.h: archivo de encabezado principal para la aplicación PROJECT_NAME
//

#pragma once

#ifndef __AFXWIN_H__
	#error "incluir 'stdafx.h' antes de incluir este archivo para PCH"
#endif

#include "resource.h"		// Símbolos principales


// Cskp2tgcApp:
// Consulte la sección skp2tgc.cpp para obtener información sobre la implementación de esta clase
//

class Cskp2tgcApp : public CWinApp
{
public:
	Cskp2tgcApp();

// Reemplazos
public:
	virtual BOOL InitInstance();

// Implementación

	DECLARE_MESSAGE_MAP()
};

extern Cskp2tgcApp theApp;