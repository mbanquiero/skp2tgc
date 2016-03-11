
// skp2tgcDlg.h: archivo de encabezado
//

#pragma once
#include <slapi/slapi.h>
#include <slapi/geometry.h>
#include <slapi/initialize.h>
#include <slapi/unicodestring.h>
#include <slapi/model/model.h>
#include <slapi/model/entities.h>
#include <slapi/model/face.h>
#include <slapi/model/edge.h>
#include <slapi/model/vertex.h>
#include <slapi/model/mesh_helper.h>
#include <slapi/transformation.h>
#include <slapi/model/component_instance.h>
#include <slapi/model/component_definition.h>
#include <slapi/model/drawing_element.h>
#include <slapi/model/material.h>
#include <slapi/model/texture.h>
#include <slapi/model/group.h>

struct VERTEX
{
	float x,y,z;			// posicion
	float nx,ny,nz;			// normal
	float u,v;				// text coords
};

// helpers
void transformPosition(SUPoint3D *v , SUTransformation *T);
void transformNormal(SUVector3D *v , SUTransformation *T);
void transformMultiply(SUTransformation *A, SUTransformation *B , SUTransformation *C);
void transposeMatrix(SUTransformation *T);
void invertMatrix(SUTransformation *T);
SUVector3D cross(SUVector3D a, SUVector3D b);
double dot(SUVector3D p , SUVector3D q);
void Normalizar(SUVector3D &A);
#define SWAP(a,b) aux=a;a=b;b=aux
void crear_color_bitmap(char *fname,BYTE r,BYTE g,BYTE b);
char *replace(char *string,char c,char to);


// Cuadro de diálogo de Cskp2tgcDlg
class Cskp2tgcDlg : public CDialogEx
{
// Construcción
public:
	Cskp2tgcDlg(CWnd* pParent = NULL);	// Constructor estándar

// Datos del cuadro de diálogo
	enum { IDD = IDD_SKP2TGC_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// Compatibilidad con DDX/DDV

public:
	DWORD *face_attr;		
	VERTEX *vertex_buffer;
	int cant_v;
	int cant_faces;
	int cant_mat;
	char mat_names[MAX_PATH][256];
	char model_name[MAX_PATH];

	int nro_mat_instancia;		// global, nro de material de la instancia

// Implementación
protected:
	HICON m_hIcon;

	// Funciones de asignación de mensajes generadas
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedImportar();
	void exportar(char *fname);

	void writeEntities(SUEntitiesRef entities, SUTransformation T);
	void writeMaterial(FILE *fp,SUMaterialRef material,char *material_name, int nro_mat);

	int que_material(char *material_name);
	void resize(float height);

	afx_msg void OnBnClickedDownloadTest();
};
