
// skp2tgcDlg.cpp: archivo de implementación
//

#include "stdafx.h"
#include "skp2tgc.h"
#include "skp2tgcDlg.h"
#include "afxdialogex.h"
#include "direct.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// Cuadro de diálogo de Cskp2tgcDlg
char *sacarPath(char *filepath,char *filename);
void extension(char *file,char *ext);

char _texturas_dir[MAX_PATH];


Cskp2tgcDlg::Cskp2tgcDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(Cskp2tgcDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void Cskp2tgcDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(Cskp2tgcDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_IMPORTAR, &Cskp2tgcDlg::OnBnClickedImportar)
END_MESSAGE_MAP()


// Controladores de mensaje de Cskp2tgcDlg

BOOL Cskp2tgcDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Establecer el icono para este cuadro de diálogo. El marco de trabajo realiza esta operación
	//  automáticamente cuando la ventana principal de la aplicación no es un cuadro de diálogo
	SetIcon(m_hIcon, TRUE);			// Establecer icono grande
	SetIcon(m_hIcon, FALSE);		// Establecer icono pequeño

	strcpy(_texturas_dir , "out/Textures");

	return TRUE;  // Devuelve TRUE  a menos que establezca el foco en un control
}

// Si agrega un botón Minimizar al cuadro de diálogo, necesitará el siguiente código
//  para dibujar el icono. Para aplicaciones MFC que utilicen el modelo de documentos y vistas,
//  esta operación la realiza automáticamente el marco de trabajo.

void Cskp2tgcDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // Contexto de dispositivo para dibujo

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Centrar icono en el rectángulo de cliente
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Dibujar el icono
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// El sistema llama a esta función para obtener el cursor que se muestra mientras el usuario arrastra
//  la ventana minimizada.
HCURSOR Cskp2tgcDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void Cskp2tgcDlg::OnBnClickedImportar()
{
	char fname[MAX_PATH];
	strcpy(fname,"");
	OPENFILENAME ofn;
	char szFile[256], szFileTitle[256];
	unsigned int i;
	char Path[32000];			// 32k para permitir agregar varios nombres de archivos
	char  szFilter[256];
	strcpy(szFilter,"Archivos skp(*.skp)|*.skp|");
	szFile[0] = '\0';
	for(i=0;szFilter[i]!='\0';++i)
		if(szFilter[i]=='|')
			szFilter[i]='\0';

	Path[0] = '\0';
	// Set all structure members to zero. 
	memset(&ofn, 0, sizeof(OPENFILENAME));
	strcpy(szFileTitle,fname);
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hInstance = AfxGetApp()->m_hInstance;
	ofn.hwndOwner = AfxGetMainWnd()->m_hWnd;
	ofn.lpstrFilter = szFilter;
	ofn.nFilterIndex = 1;
	ofn.lpstrFile= Path;
	ofn.nMaxFile = sizeof(Path);
	ofn.lpstrFileTitle = szFileTitle;
	ofn.nMaxFileTitle = sizeof(szFileTitle);
	ofn.lpstrInitialDir = "c:\\test_sketchup";
	ofn.Flags = OFN_SHOWHELP | OFN_PATHMUSTEXIST | OFN_ALLOWMULTISELECT | 
		OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR | OFN_EXPLORER;
	if (GetOpenFileName( &ofn ))
	{
		// Parseo el string devuelto
		int cant_files = 0;
		char* NextFileName = Path;
		char tempFile[MAX_PATH];
		//If multiselect was used, Path is a 0 seperated list of dir and filenames. "strlen" assumes that Path is terminated by 0. So "strlen(Path)+1" is the first char after the first 0 in Path. If this is 0 again, there was no multiselect. Otherwise there comes the next filename.
		if(*(Path + strlen(Path) + 1) == 0) 
		{ 
			//Just one file was selected, so handle Path like one connected path string:
			sacarPath(Path,model_name);
			extension(model_name,"");
			exportar(Path);
			cant_files++;
		}
		else 
		{
			//Multiple files were selected so extract them. Again "strlen"'s behaviour to assume string is 0 terminated will be used here.
			NextFileName = NextFileName + strlen(NextFileName) + 1; //Do first iteration already to jump to first filename and skip dir
			while(*NextFileName!=0)
			{
				strcpy_s(tempFile, sizeof(tempFile), Path);
				strcat_s(tempFile, sizeof(tempFile), "\\\0");
				strcat_s(tempFile, sizeof(tempFile), NextFileName);

				sacarPath(tempFile,model_name);
				extension(model_name,"");
				exportar(tempFile);
				cant_files++;

				NextFileName = NextFileName + strlen(NextFileName) + 1;
			}
		}

		char saux[255];
		sprintf(saux,"%d archivos importados",cant_files);
		AfxMessageBox(saux);
	}
}


void Cskp2tgcDlg::exportar(char *fname)
{

	// Convierto las caras en una sopa de triangulos
	vertex_buffer = new VERTEX[1024*1024*2];
	face_attr = new DWORD[1024*1024*2/3];

	cant_v = 0;
	cant_faces = 0;
	cant_mat = 0;
	nro_mat_instancia = -1;

	// Always initialize the API before using it
	SUInitialize();
	// Load the model from a file
	SUModelRef model = SU_INVALID;
	SUResult res = SUModelCreateFromFile(&model, fname);
	// It's best to always check the return code from each SU function call.
	// Only showing this check once to keep this example short.
	if (res != SU_ERROR_NONE)
	{
		AfxMessageBox("Error al abrir el archivo");
		return;
	}

	// armo el nombre del mesh con el nombre del archivo sin path
	char mesh_name[255];
	char *s = strrchr(fname,'\\');
	if(s!=NULL)
		strcpy(mesh_name,s+1);
	else
		strcpy(mesh_name,fname);
	// y sin extension
	char *p =strchr(mesh_name,'.');
	if(p!=NULL)
		*p = '\0';
	
	char fname_out[MAX_PATH];
	sprintf(fname_out,"out\\%s-TgcScene.xml",mesh_name);

	FILE *fp = fopen(fname_out,"wt");
	fprintf(fp,"<tgcScene>\n");


	fprintf(fp,"	<name>%s</name>\n",mesh_name);
	fprintf(fp,"	<texturesExport enabled='true' dir='Textures'/>\n");
	fprintf(fp,"	<lightmapExport enabled='false' dir='LightMaps'/>\n");

	// materiales
	size_t count= 0;
	res = SUModelGetNumMaterials(model, &count);
	cant_mat = count;

	if (count> 0) 
	{
		fprintf(fp,"	<materials count='%d'>\n",1);
		fprintf(fp,"	<m name='material' type='Multimaterial'>\n");

		std::vector<SUMaterialRef> materials(count);
		res = SUModelGetMaterials(model, count, &materials[0], &count);
		for (size_t i=0; i<count; i++) 
		{
			writeMaterial(fp,materials[i],mat_names[i],i);
		}
		fprintf(fp,"	</m>\n");
		fprintf(fp,"	</materials>\n");
	}


	// Get the entity container of the model.
	SUEntitiesRef entities = SU_INVALID;
	res = SUModelGetEntities(model, &entities);
	SUTransformation transform = {1,0,0,0, 0,1,0,0, 0,0,1,0 , 0,0,0,1};
	writeEntities(entities,transform);

	// que ocupe 100unidades
	resize(100);

	int cant_mesh = 1;
	int max_faces_x_mesh = 10000;
	if(cant_faces>max_faces_x_mesh)
	{
		cant_mesh = cant_faces / max_faces_x_mesh + 1;
	}


	fprintf(fp,"<meshes count='%d' >\n",cant_mesh);

	for(int nro_mesh = 0;nro_mesh<cant_mesh;++nro_mesh)
	{
		char saux[255];
		if(cant_mesh>1)
			sprintf(saux,"%s_%d",mesh_name,nro_mesh+1);
		else
			strcpy(saux,mesh_name);
		fprintf(fp,"<mesh name='%s' layer='0' type='Original' matId='0' color='[134.0,6.0,6.0]' visibility='1.0' lightmap=''>\n",saux);

		int mesh_cant_v = nro_mesh<cant_mesh-1?max_faces_x_mesh*3:cant_v%(3*max_faces_x_mesh);
		int f0 = max_faces_x_mesh*nro_mesh;			// primer cara
		int v0 = f0*3;								// primer vertice
		int mesh_cant_faces = mesh_cant_v/3;

		// vertices
		fprintf(fp,"<coordinatesIdx count='%d'>",mesh_cant_v);
		for(int i=0;i<mesh_cant_v;++i)
			fprintf(fp,"%d ",i);
		fprintf(fp,"</coordinatesIdx>\n");
	
		fprintf(fp,"<vertices count='%d'>",mesh_cant_v*3);
		for(int i=0;i<mesh_cant_v;++i)
			fprintf(fp,"%.5f %.5f %.5f ",vertex_buffer[v0+i].x , vertex_buffer[v0+i].z , vertex_buffer[v0+i].y);
		fprintf(fp,"</vertices>\n");

		// normales por vertice
		fprintf(fp,"<normals count='%d'>",mesh_cant_v*3);
		for(int i=0;i<mesh_cant_v;++i)
			fprintf(fp,"%.5f %.5f %.5f ",vertex_buffer[v0+i].nx , vertex_buffer[v0+i].nz , vertex_buffer[v0+i].ny);
		fprintf(fp,"</normals>\n");

		// coordenadas de textura
		fprintf(fp,"<textCoordsIdx count='%d'>",mesh_cant_v);
		for(int i=0;i<mesh_cant_v;++i)
			fprintf(fp,"%d ",i);
		fprintf(fp,"</textCoordsIdx>\n");

		fprintf(fp,"<texCoords count='%d'>",mesh_cant_v*2);
		for(int i=0;i<mesh_cant_v;++i)
			fprintf(fp,"%.5f %.5f ",vertex_buffer[v0+i].u , vertex_buffer[v0+i].v);
		fprintf(fp,"</texCoords>\n");

		fprintf(fp,"<matIds count='%d'>",mesh_cant_faces);
		for(int i=0;i<mesh_cant_faces;++i)
			fprintf(fp,"%d ",face_attr[f0+i]);
		fprintf(fp,"</matIds>\n");

		// color: no soportado
		fprintf(fp,"<colorsIdx count='%d'>",mesh_cant_v);
		for(int i=0;i<mesh_cant_v;++i)
			fprintf(fp,"%d ",0);
		fprintf(fp,"</colorsIdx>\n");
		fprintf(fp,"<colors count='3'>255 255 255 </colors>\n");

		fprintf(fp,"	</mesh>\n");

		// paso al siguiente mesh
	}
	fprintf(fp,"</meshes>\n");
	fprintf(fp,"</tgcScene>\n");

	// cierro y libero el archivo
	fclose(fp);

	// libero los vertices
	delete vertex_buffer;
	// libero los atributos
	delete face_attr;

	// Must release the model or there will be memory leaks
	SUModelRelease(&model);

	// Always terminate the API when done using it
	SUTerminate();

}


// helper devuelve el nombre del material
char *getMaterialName(SUMaterialRef material,char *material_name)
{
	SUStringRef name = SU_INVALID;
	SUResult res = SUStringCreate(&name);
	res = SUMaterialGetName(material, &name);
	size_t bytes;
	SUStringGetUTF8(name,MAX_PATH,material_name,&bytes);
	SUStringRelease(&name);
	// devuelvo el material name 
	return material_name;
}

// escribe el material, devuelve el material name
void Cskp2tgcDlg::writeMaterial(FILE *fp,SUMaterialRef material,char *material_name, int nro_mat)
{
	// tomo el nombre del material
	getMaterialName(material,material_name);

	SUMaterialType mat_type;
	SUResult res = SUMaterialGetType(material,&mat_type);

	BYTE r = 255;
	BYTE g = 255;
	BYTE b = 255;
	BYTE a = 255;

	if(mat_type!=SUMaterialType_Textured)
	{
		SUColor color;
		res = SUMaterialGetColor(material,&color);
		if(res==SU_ERROR_NONE)
		{
			r = (BYTE)color.red;
			g = (BYTE)color.green;
			b = (BYTE)color.blue;
			a = (BYTE)color.alpha;
		}
	}

	double alpha = 0;
	res = SUMaterialGetOpacity(material,&alpha);
	if(res==SU_ERROR_NONE && alpha<1)
		a = 255 * alpha;

	if(a<=0)
		a = 255;		// debe ser algun error


	// filename de la texutra 
	char fname[MAX_PATH];
	memset(fname,0,sizeof(fname));

	if(mat_type!=SUMaterialType_Colored)
	{
		// grabo el jpg de la textura en un archivo externo
		SUTextureRef texture = SU_INVALID;
		res = SUMaterialGetTexture(material,&texture);
		if(res==SU_ERROR_NONE)
		{

			int len = strlen(material_name);
			replace(material_name,'>','_');
			strcpy(fname,"");
			if(material_name[0]=='<')
			{
				strcat(fname,material_name+1);
				strcat(fname,".png");
			}
			else
				if(material_name[0]=='*')
				{
					strcat(fname,material_name+1);
					strcat(fname,".png");
				}
				else
				{
					strcat(fname,material_name);
					strcat(fname,".jpg");
				}

				char fname_aux[MAX_PATH];
				sprintf(fname_aux,"%s\\%s",_texturas_dir,fname);

				// x las dudas borro el archivo si es que ya existia
				DeleteFile(fname_aux);
				// y ahora graba un jpg con la imagen del a textura 
				res = SUTextureWriteToFile(texture, fname_aux);


		}
	}
	else
	{
		// no tiene textura, tengo que crear un bitmap con este color 
		sprintf(fname,"clr%d-%d-%d.bmp",r,g,b);
		char fname_aux[MAX_PATH];
		sprintf(fname_aux,"%s\\%s",_texturas_dir,fname);
		crear_color_bitmap(fname_aux,r,g,b);

	}


	fprintf(fp,"	<subM name='mat_%d' type='Standardmaterial'>\n",nro_mat);
	fprintf(fp,"		<ambient>[51.0,51.0,51.0,255.0]</ambient>\n");
	fprintf(fp,"		<diffuse>[%d,%d,%d,%d]</diffuse>\n",r,g,b,a);
	fprintf(fp,"		<specular>[51.0,51.0,51.0,255.0]</specular>\n");
	fprintf(fp,"		<opacity>100.0</opacity>\n");
	fprintf(fp,"		<alphaBlendEnable>false</alphaBlendEnable>\n");
	fprintf(fp,"		<bitmap uvTiling='[1.0,1.0]' uvOffset='[0.0,0.0]'>%s</bitmap>\n",fname);
	fprintf(fp,"	</subM>\n");
}

int Cskp2tgcDlg::que_material(char *material_name)
{
	int rta = -1;
	int i=0;
	while(i<cant_mat && rta==-1)
		if(strcmp(mat_names[i],material_name)==0)
			rta = i;
		else
			++i;
	return rta;
}


void Cskp2tgcDlg::writeEntities(SUEntitiesRef entities, SUTransformation T)
{

	SUTransformation TNorm = T;
	TNorm.values[3] = 0;
	TNorm.values[7] = 0;
	TNorm.values[11] = 0;
	TNorm.values[12] = 0;
	TNorm.values[13] = 0;
	TNorm.values[14] = 0;
	TNorm.values[15] = 1;
	//invertMatrix(&TNorm);
	//transposeMatrix(&TNorm);

	// Component instances
	SUResult res;
	size_t num_instances = 0;
	SUEntitiesGetNumInstances(entities, &num_instances);
	if (num_instances > 0) 
	{
		std::vector<SUComponentInstanceRef> instances(num_instances);
		SUEntitiesGetInstances(entities, num_instances,&instances[0], &num_instances);
		for (size_t c = 0; c < num_instances; c++) 
		{
			SUComponentInstanceRef instance = instances[c];
			SUTransformation transform;
			SUComponentInstanceGetTransform(instance,&transform);
			SUComponentDefinitionRef definition = SU_INVALID;
			SUComponentInstanceGetDefinition(instance, &definition);
			SUEntitiesRef entities_child = SU_INVALID;
			SUComponentDefinitionGetEntities(definition, &entities_child);

			int ant_mat_instancia = nro_mat_instancia;
			SUDrawingElementRef drawing_element = SUComponentInstanceToDrawingElement(instance);
			if(drawing_element.ptr!=0)
			{
				// tomo el material de todo el componente
				SUMaterialRef material = SU_INVALID;
				SUResult res = SUDrawingElementGetMaterial(drawing_element , &material);
				if(res==SU_ERROR_NONE)
				{
					// tomo el nombre del material
					char material_name[MAX_PATH];
					nro_mat_instancia = que_material(getMaterialName(material , material_name));
				}
			}


			// compongo la transformacion
			SUTransformation transform_out;
			// transform_out = T * transform
			transformMultiply(&T, &transform , &transform_out);
			writeEntities(entities_child,transform_out);

			nro_mat_instancia = ant_mat_instancia;

		}
	}

	// Groups
	size_t num_groups = 0;
	res = SUEntitiesGetNumGroups(entities, &num_groups);
	if (num_groups > 0) 
	{
		std::vector<SUGroupRef> groups(num_groups);
		res = SUEntitiesGetGroups(entities, num_groups, &groups[0], &num_groups);
		for (size_t g = 0; g < num_groups; g++) 
		{
			int ant_mat_instancia = nro_mat_instancia;

			SUGroupRef group = groups[g];
			SUDrawingElementRef drawing_element = SUGroupToDrawingElement(group);
			if(drawing_element.ptr!=0)
			{
				// tomo el material de todo el componente
				SUMaterialRef material = SU_INVALID;
				SUResult res = SUDrawingElementGetMaterial(drawing_element , &material);
				if(res==SU_ERROR_NONE)
				{
					// tomo el nombre del material
					char material_name[MAX_PATH];
					nro_mat_instancia = que_material(getMaterialName(material , material_name));
				}
			}


			SUComponentDefinitionRef group_component = SU_INVALID;
			SUEntitiesRef group_entities = SU_INVALID;
			res = SUGroupGetEntities(group, &group_entities);
			SUTransformation transform;
			res = SUGroupGetTransform(group, &transform);
			// compongo la transformacion
			SUTransformation transform_out;
			// transform_out = T * transform
			transformMultiply(&T, &transform , &transform_out);
			writeEntities(group_entities,transform_out);

			nro_mat_instancia = ant_mat_instancia;
		}
	}


	// Get all the faces from the entities object
	size_t faceCount = 0;
	res = SUEntitiesGetNumFaces(entities, &faceCount);
	if (faceCount > 0) 
	{
		std::vector<SUFaceRef> faces(faceCount);
		SUEntitiesGetFaces(entities, faceCount, &faces[0], &faceCount);

		// Get all the edges in this face
		for (size_t i = 0; i < faceCount; i++) 
		{
			int nro_mat = -1;
			bool back_face = false;
			SUMaterialRef material;
			SUMaterialRef material_front = SU_INVALID , material_back = SU_INVALID;
			SUResult res_front = SUFaceGetFrontMaterial(faces[i],&material_front);
			SUResult res_back = SUFaceGetBackMaterial(faces[i],&material_back);

			if(res_front==SU_ERROR_NONE && res_back!=SU_ERROR_NONE)
			{
				//tiene cara de adelante
				material = material_front;
				res==SU_ERROR_NONE;
			}
			else
			if(res_front!=SU_ERROR_NONE && res_back==SU_ERROR_NONE)
			{
				//tiene cara de atras
				material = material_back;
				res=SU_ERROR_NONE;
				back_face = true;
			}
			else
			if(res_front==SU_ERROR_NONE && res_back==SU_ERROR_NONE)
			{
				// tiene las 2 caras, me quedo con que tiene textura
				SUMaterialType mat_type;
				if(SUMaterialGetType(material_back,&mat_type)==SU_ERROR_NONE && mat_type==SUMaterialType_Textured)
				{
					// me quedo con la cara de atras
					material = material_back;
					back_face = true;
				}
				else
				{
					// me quedo con la cara de adelante
					material = material_front;
				}
				res=SU_ERROR_NONE;
			}
			else
			{
				// error, no tiene material en ninguna cara
				res=res_front;
			}

			if(res==SU_ERROR_NONE)
			{
				// tomo el nombre del material
				char material_name[MAX_PATH];
				nro_mat = que_material(getMaterialName(material , material_name));
			}

			if(nro_mat==-1)
			{
				// uso x defecto el material de la instancia, o cero si no hay instanaci
				nro_mat = nro_mat_instancia!=-1 ? nro_mat_instancia : 0;
			}

			SUMeshHelperRef mesh_ref = SU_INVALID;
			res = SUMeshHelperCreate(&mesh_ref,faces[i]);

			//Get vertices.
			size_t num_vertices = 0;
			SUMeshHelperGetNumVertices(mesh_ref, &num_vertices);

			std::vector<SUPoint3D> vertices(num_vertices);
			SUMeshHelperGetVertices(mesh_ref, num_vertices, &vertices[0], &num_vertices);
			for(int t=0;t<num_vertices;++t)
				transformPosition(&vertices[t],&T);

			std::vector<SUPoint3D> uv_coords(num_vertices);

			if(back_face)
				SUMeshHelperGetBackSTQCoords(mesh_ref, num_vertices, &uv_coords[0], &num_vertices);
			else
				SUMeshHelperGetFrontSTQCoords(mesh_ref, num_vertices, &uv_coords[0], &num_vertices);

			// Get normals
			std::vector<SUVector3D> normals(num_vertices);
			SUMeshHelperGetNormals(mesh_ref, num_vertices, &normals[0], &num_vertices);

			// Transform normals
			for(int t=0;t<num_vertices;++t)
				transformNormal(&normals[t],&TNorm);


			if(back_face && 0)
			{
				for(int t=0;t<num_vertices;++t)
				{
					normals[t].x *= -1;
					normals[t].y *= -1;
					normals[t].z *= -1;
				}
			}

			// Get triangle indices.
			size_t num_triangles = 0;
			SUMeshHelperGetNumTriangles(mesh_ref, &num_triangles);
			const size_t num_indices = 3 * num_triangles;
			size_t num_retrieved = 0;
			std::vector<size_t> indices(num_indices);
			SUMeshHelperGetVertexIndices(mesh_ref, num_indices, &indices[0], &num_retrieved);

			for (size_t i_triangle = 0; i_triangle < num_triangles; i_triangle++) 
			{
				// Three points in each triangle
				SUVector3D Q[3], N;
				N.x = 0;
				N.y = 0;
				N.z = 0;
				for (size_t i = 0; i < 3; i++) 
				{
					size_t index = indices[i_triangle * 3 + i];
					// agrego el vertice en el vertex buffer
					vertex_buffer[cant_v].x = vertices[index].x * 25.4f;
					vertex_buffer[cant_v].y = -vertices[index].y* 25.4f;
					vertex_buffer[cant_v].z = vertices[index].z * 25.4f;
					vertex_buffer[cant_v].nx = normals[index].x;
					vertex_buffer[cant_v].ny = -normals[index].y;
					vertex_buffer[cant_v].nz = normals[index].z;
					vertex_buffer[cant_v].u = uv_coords[index].x / uv_coords[index].z;
					vertex_buffer[cant_v].v = (1 - uv_coords[index].y) / uv_coords[index].z;

					Q[i].x = vertices[index].x;
					Q[i].y = -vertices[index].y;
					Q[i].z = vertices[index].z;

					// normal promedio de la cara
					N.x += normals[index].x;
					N.y += -normals[index].y;
					N.z += normals[index].z;

					++cant_v;
				}

				// verifico que el triangulo este en el sentido correcto
				// computo la normal U*V

				SUVector3D U;
				// U = Q1 - Q0
				U.x = Q[1].x - Q[0].x;
				U.y = Q[1].y - Q[0].y;
				U.z = Q[1].z - Q[0].z;
				Normalizar(U);
				SUVector3D V;
				// V = Q2 - Q0
				V.x = Q[2].x - Q[0].x;
				V.y = Q[2].y - Q[0].y;
				V.z = Q[2].z - Q[0].z;
				Normalizar(V);

				Normalizar(N);
				SUVector3D N0 = cross(U,V);
				SUVector3D N1 = cross(V,U);
				double dot_p0 = dot(N0, N);
				double dot_p1 = dot(N1, N);
				if(dot_p0<0)
				{
					// la cara esta al reves
					// invierto el vertice 1 con el 2
					double aux;
					int v1 = cant_v-2;
					int v2 = cant_v-1;
					SWAP(vertex_buffer[v1].x , vertex_buffer[v2].x);
					SWAP(vertex_buffer[v1].y , vertex_buffer[v2].y);
					SWAP(vertex_buffer[v1].z , vertex_buffer[v2].z);
					SWAP(vertex_buffer[v1].nx , vertex_buffer[v2].nx);
					SWAP(vertex_buffer[v1].ny , vertex_buffer[v2].ny);
					SWAP(vertex_buffer[v1].nz , vertex_buffer[v2].nz);
					SWAP(vertex_buffer[v1].u , vertex_buffer[v2].u);
					SWAP(vertex_buffer[v1].v , vertex_buffer[v2].v);
				}
				face_attr[cant_faces++] = nro_mat;
			}
			SUMeshHelperRelease(&mesh_ref);
		}
	}
}

void Cskp2tgcDlg::resize(float height)
{
	// computo el bounding box
	float min_x , min_y , min_z;
	float max_x , max_y , max_z;

	min_x = max_x = vertex_buffer[0].x;
	min_y = max_y = vertex_buffer[0].y;
	min_z = max_z = vertex_buffer[0].z;
	for(int i=1;i<cant_v;++i)
	{
		float x = vertex_buffer[i].x;
		float y = vertex_buffer[i].y;
		float z = vertex_buffer[i].z;
		if(x<min_x)
			min_x = x;
		if(y<min_y)
			min_y = y;
		if(z<min_z)
			min_z = z;

		if(x>max_x)
			max_x = x;
		if(y>max_y)
			max_y = y;
		if(z>max_z)
			max_z = z;
	}

	// determino la escala
	if(max_y - min_y >0)
	{
		float k = height / (max_y - min_y);
		for(int i=0;i<cant_v;++i)
		{
			vertex_buffer[i].x = (vertex_buffer[i].x - min_x)*k;
			vertex_buffer[i].y = (vertex_buffer[i].y - min_y)*k;
			vertex_buffer[i].z = (vertex_buffer[i].z - min_z)*k;
		}
	}
}




// helpers
void transformMultiply(SUTransformation *A, SUTransformation *B , SUTransformation *C)	
{
	for(int i=0;i<4;++i)
	{
		for(int j=0;j<4;++j)
		{
			C->values[i+j*4] = 0;
			for(int t=0;t<4;++t)
			{
				C->values[i+j*4] += A->values[i+t*4] * B->values[t+j*4];
			}
		}
	}
}



void transformPosition(SUPoint3D *v , SUTransformation *T)
{
	double pos[4] = {v->x, v->y,v->z,1};
	double vt[3];
	for(int i=0;i<3;++i)
	{
		vt[i] = 0;
		for(int j=0;j<4;++j)
		{
			vt[i] += pos[j] * T->values[i+j*4];
		}
	}
	v->x = vt[0];
	v->y = vt[1];
	v->z = vt[2];
}

void transformNormal(SUVector3D *v , SUTransformation *T)
{
	double pos[4] = {v->x, v->y,v->z,0};
	double vt[3];
	for(int i=0;i<3;++i)
	{
		vt[i] = 0;
		for(int j=0;j<3;++j)
		{
			vt[i] += pos[j] * T->values[i+j*4];
		}
	}
	v->x = vt[0];
	v->y = vt[1];
	v->z = vt[2];
}

SUVector3D cross(SUVector3D a, SUVector3D b)
{
	SUVector3D c;
	c.x = a.y*b.z-a.z*b.y;
	c.y = a.z*b.x-a.x*b.z;
	c.z = a.x*b.y-a.y*b.x;
	return c;
}

double dot(SUVector3D p , SUVector3D q)
{
	return p.x*q.x+p.y*q.y+p.z*q.z;
}

void Normalizar(SUVector3D &A)
{
	double len = A.x*A.x + A.y*A.y + A.z*A.z;
	if(fabs(len)<0.000001)
		return;
	float k = 1 / sqrt(len);
	A.x *= k;
	A.y *= k;
	A.z *= k;
}

void transposeMatrix(SUTransformation *T)
{
	double aux;
	SWAP(T->values[1],T->values[4]);
	SWAP(T->values[2],T->values[8]);
	SWAP(T->values[3],T->values[12]);
	SWAP(T->values[6],T->values[9]);
	SWAP(T->values[7],T->values[13]);
	SWAP(T->values[11],T->values[14]);
}

void invertMatrix(SUTransformation *T)
{
#define MATRIX_INVERSE_EPSILON		1e-14
	// 84+4+16 = 104 multiplications
	//			   1 division
	double det, invDet;

	float m[4][4];
	int s = 0;
	for(int j=0;j<4;++j)
		for(int i=0;i<4;++i)
			m[i][j] = T->values[s++];

	// 2x2 sub-determinants required to calculate 4x4 determinant
	float det2_01_01 = m[0][0] * m[1][1] - m[1][0] * m[0][1];
	float det2_01_02 = m[0][0] * m[2][1] - m[2][0] * m[0][1];
	float det2_01_03 = m[0][0] * m[3][1] - m[3][0] * m[0][1];
	float det2_01_12 = m[1][0] * m[2][1] - m[2][0] * m[1][1];
	float det2_01_13 = m[1][0] * m[3][1] - m[3][0] * m[1][1];
	float det2_01_23 = m[2][0] * m[3][1] - m[3][0] * m[2][1];

	// 3x3 sub-determinants required to calculate 4x4 determinant
	float det3_201_012 = m[0][2] * det2_01_12 - m[1][2] * det2_01_02 + m[2][2] * det2_01_01;
	float det3_201_013 = m[0][2] * det2_01_13 - m[1][2] * det2_01_03 + m[3][2] * det2_01_01;
	float det3_201_023 = m[0][2] * det2_01_23 - m[2][2] * det2_01_03 + m[3][2] * det2_01_02;
	float det3_201_123 = m[1][2] * det2_01_23 - m[2][2] * det2_01_13 + m[3][2] * det2_01_12;

	det = ( - det3_201_123 * m[0][3] + det3_201_023 * m[1][3] - det3_201_013 * m[2][3] + det3_201_012 * m[3][3] );

	if ( fabs( det ) < MATRIX_INVERSE_EPSILON ) {
		return;
	}

	invDet = 1.0f / det;

	// remaining 2x2 sub-determinants
	float det2_03_01 = m[0][0] * m[1][3] - m[1][0] * m[0][3];
	float det2_03_02 = m[0][0] * m[2][3] - m[2][0] * m[0][3];
	float det2_03_03 = m[0][0] * m[3][3] - m[3][0] * m[0][3];
	float det2_03_12 = m[1][0] * m[2][3] - m[2][0] * m[1][3];
	float det2_03_13 = m[1][0] * m[3][3] - m[3][0] * m[1][3];
	float det2_03_23 = m[2][0] * m[3][3] - m[3][0] * m[2][3];

	float det2_13_01 = m[0][1] * m[1][3] - m[1][1] * m[0][3];
	float det2_13_02 = m[0][1] * m[2][3] - m[2][1] * m[0][3];
	float det2_13_03 = m[0][1] * m[3][3] - m[3][1] * m[0][3];
	float det2_13_12 = m[1][1] * m[2][3] - m[2][1] * m[1][3];
	float det2_13_13 = m[1][1] * m[3][3] - m[3][1] * m[1][3];
	float det2_13_23 = m[2][1] * m[3][3] - m[3][1] * m[2][3];

	// remaining 3x3 sub-determinants
	float det3_203_012 = m[0][2] * det2_03_12 - m[1][2] * det2_03_02 + m[2][2] * det2_03_01;
	float det3_203_013 = m[0][2] * det2_03_13 - m[1][2] * det2_03_03 + m[3][2] * det2_03_01;
	float det3_203_023 = m[0][2] * det2_03_23 - m[2][2] * det2_03_03 + m[3][2] * det2_03_02;
	float det3_203_123 = m[1][2] * det2_03_23 - m[2][2] * det2_03_13 + m[3][2] * det2_03_12;

	float det3_213_012 = m[0][2] * det2_13_12 - m[1][2] * det2_13_02 + m[2][2] * det2_13_01;
	float det3_213_013 = m[0][2] * det2_13_13 - m[1][2] * det2_13_03 + m[3][2] * det2_13_01;
	float det3_213_023 = m[0][2] * det2_13_23 - m[2][2] * det2_13_03 + m[3][2] * det2_13_02;
	float det3_213_123 = m[1][2] * det2_13_23 - m[2][2] * det2_13_13 + m[3][2] * det2_13_12;

	float det3_301_012 = m[0][3] * det2_01_12 - m[1][3] * det2_01_02 + m[2][3] * det2_01_01;
	float det3_301_013 = m[0][3] * det2_01_13 - m[1][3] * det2_01_03 + m[3][3] * det2_01_01;
	float det3_301_023 = m[0][3] * det2_01_23 - m[2][3] * det2_01_03 + m[3][3] * det2_01_02;
	float det3_301_123 = m[1][3] * det2_01_23 - m[2][3] * det2_01_13 + m[3][3] * det2_01_12;

	m[0][0] =	- det3_213_123 * invDet;
	m[1][0] = + det3_213_023 * invDet;
	m[2][0] = - det3_213_013 * invDet;
	m[3][0] = + det3_213_012 * invDet;

	m[0][1] = + det3_203_123 * invDet;
	m[1][1] = - det3_203_023 * invDet;
	m[2][1] = + det3_203_013 * invDet;
	m[3][1] = - det3_203_012 * invDet;

	m[0][2] = + det3_301_123 * invDet;
	m[1][2] = - det3_301_023 * invDet;
	m[2][2] = + det3_301_013 * invDet;
	m[3][2] = - det3_301_012 * invDet;

	m[0][3] = - det3_201_123 * invDet;
	m[1][3] = + det3_201_023 * invDet;
	m[2][3] = - det3_201_013 * invDet;
	m[3][3] = + det3_201_012 * invDet;

	s = 0;
	for(int j=0;j<4;++j)
		for(int i=0;i<4;++i)
			T->values[s++] = m[i][j];

}

void Cskp2tgcDlg::OnBnClickedDownloadTest()
{
	HRESULT br = URLDownloadToFile(NULL,"http://3dwarehouse.sketchup.com/warehouse/getbinary?subjectId=ea8d250e3363377c36fb70296e45483&subjectClass=entity&name=s8"
		,"TEST.DAT",0,NULL);

	int a = 0;


}



char *replace(char *string,char c,char to)
{
	int i=0;
	while(string[i])
	{
		if(string[i]==c)
			string[i]=to;
		++i;
	}
	return string;
}

char *sacarPath(char *filepath,char *filename)
{
	int i=0;
	int j=-1;
	while(filepath[i]!='\0')
	{
		if(filepath[i]=='\\')
			j=i;
		++i;
	}

	strcpy(filename,filepath+j+1);
	return(filename);
}

void extension(char *file,char *ext)
{
	int i=0;

	CString sFileName = (CString)file;
	// Le saco la extension que tenia antes
	// si es que tiene extension
	if(sFileName.Find('.')!=-1)
		sFileName = sFileName.Left(sFileName.ReverseFind('.'));

	if(ext && ext[0])
	{
		// Ahora no tiene mas extension, le agrego la nueva
		sFileName+=".";
		sFileName+=ext;
	}

	// y lo almaceno en *file 
	strcpy(file,(LPCSTR)sFileName);
}

void crear_color_bitmap(char *fname,BYTE r,BYTE g,BYTE b)
{
	CFile *fp = new CFile(fname, CFile::modeCreate | CFile::modeWrite );
	BYTE bm[] = {	0x42,0x4d,0x3a,0x00,0x00,0x00,0x00,0x00,	0x00,0x00,0x36,0x00,0x00,0x00,0x28,0x00,
					0x00,0x00,0x01,0x00,0x00,0x00,0x01,0x00,	0x00,0x00,0x01,0x00,0x18,0x00,0x00,0x00,
					0x00,0x00,0x04,0x00,0x00,0x00,0x13,0x0b,	0x00,0x00,0x13,0x0b,0x00,0x00,0x00,0x00,
					0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xff,	0xff,0x00};
	bm[54] = b;
	bm[55] = g;
	bm[56] = r;

	fp->Write(&bm,sizeof(bm));
	fp->Close();
	delete fp;

}