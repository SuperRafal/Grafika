/*
     Prosty studencki program graficzny 
	 kodowany w jezyku C z uzyciem OpenGL 
*/

#include "szescian.h"
#include "objLoader.h"
#include <time.h>


LONG WINAPI WndProc( HWND, UINT, WPARAM, LPARAM );
HGLRC SetUpOpenGL( HWND hWnd );

// Wartosci poczatkowe
#define	DEFAULT_Z_DIST		25
#define DEFAULT_PIVOT_VERT_ANGLE	0
#define DEFAULT_PIVOT_HORIZ_ANGLE	0
#define DEFAULT_PIVOT_X				0
#define DEFAULT_PIVOT_Y				0


float z_dist=DEFAULT_Z_DIST;						// INSERT, PAGE UP
float pivot_vert_angle=DEFAULT_PIVOT_VERT_ANGLE;	// UP, DOWN
float pivot_horiz_angle=DEFAULT_PIVOT_HORIZ_ANGLE;	// LEFT, RIGHT
float pivot_x=DEFAULT_PIVOT_X;						// DELETE, PAGE DOWN
float pivot_y=DEFAULT_PIVOT_Y;						// HOME, END


// ************ Deklaracje funkcji ************************

void DrawOpenGLScene( void );  // glowna funkcja rysujaca

//   Skladowe funkcje obiektu graficznego

// FUNKCJE DO WCZYTYWANIA MODELU

void FreeModel(struct obj_model_t *mdl);
int MallocModel(struct obj_model_t *mdl);
int FirstPass(FILE *fp, struct obj_model_t *mdl);
int SecondPass(FILE *fp, struct obj_model_t *mdl);
int ReadOBJModel(const char *filename, struct obj_model_t *mdl, const char *texname);
void RenderOBJModel(struct obj_model_t *mdl);
int LoadGLTextures(struct obj_model_t *mdl, const char* texname);
AUX_RGBImageRec *LoadBMP(const char *Filename);
void SetPosition(struct obj_model_t *mdl, float x, float y, float z);
void SetRotate(struct obj_model_t *mdl, float angle);


// ..............................

//******** Fukcje skladowe ********************************

struct obj_model_t *gasienice1 = &gasienice;
struct obj_model_t *kadlub1 = &kadlub;
struct obj_model_t *lufa1 = &lufa;
struct obj_model_t *wieza1 = &wieza;
struct obj_model_t *trawa1 = &trawa;
struct obj_model_t *ziemia1 = &ziemia;
struct obj_model_t *box1 = &box;
struct obj_model_t *kadlub2 = &kadlub;
struct obj_model_t *lufa2 = &lufa;
struct obj_model_t *wieza2 = &wieza;

/*
*  KOD DO WCZYTYWANIA MODELI W OBJ
*
*/

void SetPosition(struct obj_model_t *mdl, float x, float y, float z)
{
	mdl->pos_x = x;
	mdl->pos_y = y;
	mdl->pos_z = z;
	
}

void SetRotate(struct obj_model_t *mdl, float angle)
{
	mdl->angle = angle;
}

void FreeModel(struct obj_model_t *mdl)
{
	int i;

	if (mdl)
	{
		if (mdl->vertices)
		{
			free(mdl->vertices);
			mdl->vertices = NULL;
		}

		if (mdl->texCoords)
		{
			free(mdl->texCoords);
			mdl->texCoords = NULL;
		}

		if (mdl->normals)
		{
			free(mdl->normals);
			mdl->normals = NULL;
		}

		if (mdl->faces)
		{
			for (i = 0; i < mdl->num_faces; ++i)
			{
				if (mdl->faces[i].vert_indices)
					free(mdl->faces[i].vert_indices);

				if (mdl->faces[i].uvw_indices)
					free(mdl->faces[i].uvw_indices);

				if (mdl->faces[i].norm_indices)
					free(mdl->faces[i].norm_indices);
			}

			free(mdl->faces);
			mdl->faces = NULL;
		}
	}
}

int MallocModel(struct obj_model_t *mdl)
{
	if (mdl->num_verts)
	{
		mdl->vertices = (struct obj_vertex_t *)
			malloc(sizeof(struct obj_vertex_t) * mdl->num_verts);
		if (!mdl->vertices)
			return 0;
	}

	if (mdl->num_texCoords)
	{
		mdl->texCoords = (struct obj_texCoord_t *)
			malloc(sizeof(struct obj_texCoord_t) * mdl->num_texCoords);
		if (!mdl->texCoords)
			return 0;
	}

	if (mdl->num_normals)
	{
		mdl->normals = (struct obj_normal_t *)
			malloc(sizeof(struct obj_normal_t) * mdl->num_normals);
		if (!mdl->normals)
			return 0;
	}

	if (mdl->num_faces)
	{
		mdl->faces = (struct obj_face_t *)
			calloc(mdl->num_faces, sizeof(struct obj_face_t));
		if (!mdl->faces)
			return 0;
	}

	mdl->texture[0] = (GLuint*)malloc(sizeof(GLuint));

	return 1;
}

int FirstPass(FILE *fp, struct obj_model_t *mdl)
{
	int v, t, n;
	char buf[256];

	while (!feof(fp))
	{
		/* Read whole line */
		fgets(buf, sizeof(buf), fp);

		switch (buf[0])
		{
		case 'v':
		{
			if (buf[1] == ' ')
			{
				/* Vertex */
				mdl->num_verts++;
			}
			else if (buf[1] == 't')
			{
				/* Texture coords. */
				mdl->num_texCoords++;
			}
			else if (buf[1] == 'n')
			{
				/* Normal vector */
				mdl->num_normals++;
			}
			else
			{
				printf("Warning: unknown token \"%s\"! (ignoring)\n", buf);
			}

			break;
		}

		case 'f':
		{
			/* Face */
			if (sscanf(buf + 2, "%d/%d/%d", &v, &n, &t) == 3)
			{
				mdl->num_faces++;
				mdl->has_texCoords = 1;
				mdl->has_normals = 1;
			}
			else if (sscanf(buf + 2, "%d//%d", &v, &n) == 2)
			{
				mdl->num_faces++;
				mdl->has_texCoords = 0;
				mdl->has_normals = 1;
			}
			else if (sscanf(buf + 2, "%d/%d", &v, &t) == 2)
			{
				mdl->num_faces++;
				mdl->has_texCoords = 1;
				mdl->has_normals = 0;
			}
			else if (sscanf(buf + 2, "%d", &v) == 1)
			{
				mdl->num_faces++;
				mdl->has_texCoords = 0;
				mdl->has_normals = 0;
			}
			else
			{
				/* Should never be there or the model is very crappy */
				fprintf(stderr, "Error: found face with no vertex!\n");
			}

			break;
		}

		case 'g':
		{
			/* Group */
			/*	fscanf (fp, "%s", buf); */
			break;
		}

		default:
			break;
		}
	}

	/* Check if informations are valid */
	if ((mdl->has_texCoords && !mdl->num_texCoords) ||
		(mdl->has_normals && !mdl->num_normals))
	{
		fprintf(stderr, "error: contradiction between collected info!\n");
		return 0;
	}

	if (!mdl->num_verts)
	{
		fprintf(stderr, "error: no vertex found!\n");
		return 0;
	}

	printf("first pass results: found\n");
	printf("   * %i vertices\n", mdl->num_verts);
	printf("   * %i texture coords.\n", mdl->num_texCoords);
	printf("   * %i normal vectors\n", mdl->num_normals);
	printf("   * %i faces\n", mdl->num_faces);
	printf("   * has texture coords.: %s\n", mdl->has_texCoords ? "yes" : "no");
	printf("   * has normals: %s\n", mdl->has_normals ? "yes" : "no");

	return 1;
}

int SecondPass(FILE *fp, struct obj_model_t *mdl)
{
	struct obj_vertex_t *pvert = mdl->vertices;
	struct obj_texCoord_t *puvw = mdl->texCoords;
	struct obj_normal_t *pnorm = mdl->normals;
	struct obj_face_t *pface = mdl->faces;
	char buf[128], *pbuf;
	int i;

	while (!feof(fp))
	{
		/* Read whole line */
		fgets(buf, sizeof(buf), fp);

		switch (buf[0])
		{
		case 'v':
		{
			if (buf[1] == ' ')
			{
				/* Vertex */
				if (sscanf(buf + 2, "%f %f %f %f",
					&pvert->xyzw[0], &pvert->xyzw[1],
					&pvert->xyzw[2], &pvert->xyzw[3]) != 4)
				{
					if (sscanf(buf + 2, "%f %f %f", &pvert->xyzw[0],
						&pvert->xyzw[1], &pvert->xyzw[2]) != 3)
					{
						fprintf(stderr, "Error reading vertex data!\n");
						return 0;
					}
					else
					{
						pvert->xyzw[3] = 1.0;
					}
				}

				pvert++;
			}
			else if (buf[1] == 't')
			{
				/* Texture coords. */
				if (sscanf(buf + 2, "%f %f %f", &puvw->uvw[0],
					&puvw->uvw[1], &puvw->uvw[2]) != 3)
				{
					if (sscanf(buf + 2, "%f %f", &puvw->uvw[0],
						&puvw->uvw[1]) != 2)
					{
						if (sscanf(buf + 2, "%f", &puvw->uvw[0]) != 1)
						{
							fprintf(stderr, "Error reading texture coordinates!\n");
							return 0;
						}
						else
						{
							puvw->uvw[1] = 0.0;
							puvw->uvw[2] = 0.0;
						}
					}
					else
					{
						puvw->uvw[2] = 0.0;
					}
				}

				puvw++;
			}
			else if (buf[1] == 'n')
			{
				/* Normal vector */
				if (sscanf(buf + 2, "%f %f %f", &pnorm->ijk[0],
					&pnorm->ijk[1], &pnorm->ijk[2]) != 3)
				{
					fprintf(stderr, "Error reading normal vectors!\n");
					return 0;
				}

				pnorm++;
			}

			break;
		}

		case 'f':
		{
			pbuf = buf;
			pface->num_elems = 0;

			/* Count number of vertices for this face */
			while (*pbuf)
			{
				if (*pbuf == ' ')
					pface->num_elems++;

				pbuf++;
			}

			/* Select primitive type */
			if (pface->num_elems < 3)
			{
				fprintf(stderr, "Error: a face must have at least 3 vertices!\n");
				return 0;
			}
			else if (pface->num_elems == 3)
			{
				pface->type = GL_TRIANGLES;
			}
			else if (pface->num_elems == 4)
			{
				pface->type = GL_QUADS;
			}
			else
			{
				pface->type = GL_POLYGON;
			}

			/* Memory allocation for vertices */
			pface->vert_indices = (int *)malloc(sizeof(int) * pface->num_elems);

			if (mdl->has_texCoords)
				pface->uvw_indices = (int *)malloc(sizeof(int) * pface->num_elems);

			if (mdl->has_normals)
				pface->norm_indices = (int *)malloc(sizeof(int) * pface->num_elems);

			/* Read face data */
			pbuf = buf;
			i = 0;

			for (i = 0; i < pface->num_elems; ++i)
			{
				pbuf = strchr(pbuf, ' ');
				pbuf++; /* Skip space */

				/* Try reading vertices */
				if (sscanf(pbuf, "%d/%d/%d",
					&pface->vert_indices[i],
					&pface->uvw_indices[i],
					&pface->norm_indices[i]) != 3)
				{
					if (sscanf(pbuf, "%d//%d", &pface->vert_indices[i],
						&pface->norm_indices[i]) != 2)
					{
						if (sscanf(pbuf, "%d/%d", &pface->vert_indices[i],
							&pface->uvw_indices[i]) != 2)
						{
							sscanf(pbuf, "%d", &pface->vert_indices[i]);
						}
					}
				}

				/* Indices must start at 0 */
				pface->vert_indices[i]--;

				if (mdl->has_texCoords)
					pface->uvw_indices[i]--;

				if (mdl->has_normals)
					pface->norm_indices[i]--;
			}

			pface++;
			break;
		}
		}
	}

	printf("second pass results: read\n");
	printf("   * %li vertices\n", pvert - mdl->vertices);
	printf("   * %li texture coords.\n", puvw - mdl->texCoords);
	printf("   * %li normal vectors\n", pnorm - mdl->normals);
	printf("   * %li faces\n", pface - mdl->faces);

	return 1;
}

int ReadOBJModel(const char *filename, struct obj_model_t *mdl, const char *texname)
{
	FILE *fp;

	fp = fopen(filename, "r");
	if (!fp)
	{
		fprintf(stderr, "Error: couldn't open \"%s\"!\n", filename);
		return 0;
	}

	/* reset model data */
	memset(mdl, 0, sizeof(struct obj_model_t));

	/* first pass: read model info */
	if (!FirstPass(fp, mdl))
	{
		fclose(fp);
		return 0;
	}

	rewind(fp);

	/* memory allocation */
	if (!MallocModel(mdl))
	{
		fclose(fp);
		FreeModel(mdl);
		return 0;
	}

	/* second pass: read model data */
	if (!SecondPass(fp, mdl))
	{
		fclose(fp);
		FreeModel(mdl);
		return 0;
	}

	fclose(fp);
	if(!LoadGLTextures(mdl, texname))
		return 0;
	return 1;
}

void RenderOBJModel(struct obj_model_t *mdl)
{
	int i, j;
	glBindTexture(GL_TEXTURE_2D, mdl->texture[0]);
	glPushMatrix();
	glColor3f(1,1,1);
	glRotatef(mdl->angle, 0, 1, 0);
	glTranslatef(mdl->pos_x, mdl->pos_y, mdl->pos_z);

	for (i = 0; i < mdl->num_faces; ++i)
	{
		glBegin(mdl->faces[i].type);
		for (j = 0; j < mdl->faces[i].num_elems; ++j)
		{
			if (mdl->has_texCoords)
				glTexCoord2fv(mdl->texCoords[mdl->faces[i].uvw_indices[j]].uvw);

			if (mdl->has_normals)
				glNormal3fv(mdl->normals[mdl->faces[i].norm_indices[j]].ijk);

			glVertex4fv(mdl->vertices[mdl->faces[i].vert_indices[j]].xyzw);
		}
		glEnd();
		glPopMatrix();
	}
}

AUX_RGBImageRec *LoadBMP(char *Filename)				// Loads A Bitmap Image
{
	FILE *File=NULL;									// File Handle

	if (!Filename)										// Make Sure A Filename Was Given
	{
		return NULL;									// If Not Return NULL
	}

	File=fopen(Filename,"r");							// Check To See If The File Exists

	if (File)											// Does The File Exist?
	{
		fclose(File);									// Close The Handle
		return auxDIBImageLoad(Filename);				// Load The Bitmap And Return A Pointer
	}

	return NULL;										// If Load Failed Return NULL
}

int LoadGLTextures(struct obj_model_t *mdl, const char *texname)									// Load Bitmaps And Convert To Textures
{
	int Status=FALSE;									// Status Indicator

	AUX_RGBImageRec *TextureImage[1];					// Create Storage Space For The Texture

	memset(TextureImage,0,sizeof(void *)*1);           	// Set The Pointer To NULL

	// Load The Bitmap, Check For Errors, If Bitmap's Not Found Quit
	if (TextureImage[0]=LoadBMP(texname))
	{
		Status=TRUE;									// Set The Status To TRUE

		glGenTextures(1, &mdl->texture[0]);					// Create The Texture

		// Typical Texture Generation Using Data From The Bitmap
		glBindTexture(GL_TEXTURE_2D, mdl->texture[0]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TextureImage[0]->sizeX, TextureImage[0]->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, TextureImage[0]->data);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	}

	if (TextureImage[0])									// If Texture Exists
	{
		if (TextureImage[0]->data)							// If Texture Image Exists
		{
			free(TextureImage[0]->data);					// Free The Texture Image Memory
		}

		free(TextureImage[0]);								// Free The Image Structure
	}

	return Status;										// Return The Status
}






//******************************************************** 
//  Glowna funkcja WINDOWS
//******************************************************** 

int WINAPI WinMain (HINSTANCE hInstance,
					HINSTANCE hPrevInstance,
                    LPSTR lpszCmdLine,
					int nCmdShow)
{
    static char szAppName[] = "OpenGL";                    
    static char szTitle[]=    "SZESCIAN"; // Nazwa okna
    WNDCLASS wc;   // zmienna klasy okna
    MSG      msg;  // zmienna komunikatu 
    HWND     hWnd; // uchwyt okna

    // wypelnianie klasy okna 

    wc.style         =
        CS_HREDRAW | CS_VREDRAW;// styl okna
    wc.lpfnWndProc   = 
        (WNDPROC)WndProc;       // procedura okna
    wc.cbClsExtra    = 0;       // dodatkowe dane
    wc.cbWndExtra    = 0;       
    wc.hInstance     =
        hInstance;              // instancja 
    wc.hIcon         = NULL;    // nazwa ikony 
    wc.hCursor       =
        LoadCursor(NULL, IDC_ARROW);// kursor
    wc.hbrBackground = 
        (HBRUSH)(COLOR_WINDOW+1);// domyslny kolor okna
    wc.lpszMenuName  = NULL;     
    wc.lpszClassName =
        szAppName;               // nazwa klasy

    // Rejestracja klasy okna

    RegisterClass( &wc );
  
    // Konstrukcja glownego okna 

    hWnd = CreateWindow(
                szAppName, // app name
                szTitle,   // Text for window title bar
                WS_OVERLAPPEDWINDOW// Window style 
                  // NEED THESE for OpenGL calls to work!
                 | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
                CW_USEDEFAULT, 0, CW_USEDEFAULT, 0,
                NULL,     // no parent window
                NULL,     // Use the window class menu.
                hInstance,// This instance owns this window
                NULL      // We don't use any extra data
        );

    // Jezeli okno nie zostanie utworzone, zwrot 0
    if ( !hWnd )
        {
        return( 0 );
        }

	
	ShowWindow( hWnd, nCmdShow );// Wyswietlanie okna
    UpdateWindow( hWnd );        // Aktualizacja okna

    // Uruchiomienie petli komunikatow
    while (GetMessage(&msg, NULL, 0, 0)) 
	{
        TranslateMessage( &msg ); // Przetwarzanie wiadomosci
        DispatchMessage( &msg );  // Zwolnienie wiadomosci
    }

    return( msg.wParam ); 
}


//******************************************************** 
//    Procedura okna
//******************************************************** 
 
LONG WINAPI WndProc(HWND hWnd,
					UINT msg,
                    WPARAM wParam,
					LPARAM lParam)
{
    HDC hDC;

    static HGLRC hRC; // Note this is STATIC!
    
	PAINTSTRUCT ps;

    GLdouble gldAspect;

    GLsizei glnWidth, glnHeight;

	float change;


    // Petla komunikatow 
	
    switch (msg)
      {
       case WM_CREATE:
           hRC = SetUpOpenGL( hWnd );	// Inicjalizacja OpenGL
           return 0;
    
       case WM_SIZE:
            hDC = GetDC (hWnd);			// Kontekst urzadzenia
            wglMakeCurrent (hDC, hRC);
        
            // Usawienie rozmiarow okna graficznego
            
			glnWidth = (GLsizei) LOWORD (lParam);
            glnHeight = (GLsizei) HIWORD (lParam);
            
			// Aspekt DC
			gldAspect = (GLdouble)glnWidth/(GLdouble)glnHeight;
       
            glMatrixMode( GL_PROJECTION );// Ustawienie macierzy projekcji
            
			glLoadIdentity();
            
			// Rodzaj transformacji ekranowej
            gluPerspective(
                30.0,      // kat 
                gldAspect, // aspekt
                1.0,       // polozenie plaszczyzny blizszej
                1000.0 );  // polozenie plaszczyzny dalszej

            glViewport( 0, 0, glnWidth, glnHeight );
            
			// czyszczenie
			glClearColor(0.5f,0.5f,0.5f,1.0f);

			glEnable(GL_COLOR_MATERIAL);
			wglMakeCurrent( NULL, NULL );
            ReleaseDC( hWnd, hDC );
            return 0;

        case WM_PAINT:
            // rysowanie sceny

            hDC = BeginPaint( hWnd, &ps );
            wglMakeCurrent( hDC, hRC );
            
            DrawOpenGLScene();
			
			SwapBuffers(hDC);
            
            wglMakeCurrent( NULL, NULL );

            EndPaint( hWnd, &ps );
            return 0;       
            
        case WM_DESTROY:
            // zamykanie 
            wglDeleteContext( hRC );
            PostQuitMessage( 0 );
            return 0;

		case WM_KEYDOWN: //************ OBSLUGA KLAWISZY ***********************
			if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
				change=1.0f;
			else
				change=0.1f;

			switch ((int)wParam)
			{
				// obrot w pionie
				case VK_UP:
					pivot_vert_angle+=5;
					if (pivot_vert_angle>=360)
						pivot_vert_angle-=360;
					InvalidateRect(hWnd, NULL, FALSE);
					break;
				case VK_DOWN:
					pivot_vert_angle-=5;
					if (pivot_vert_angle<0)
						pivot_vert_angle+=360;
					InvalidateRect(hWnd, NULL, FALSE);
					break;

				// obrot w poziomie
				case VK_RIGHT:
					pivot_horiz_angle+=5;
					if (pivot_horiz_angle>=360)
						pivot_horiz_angle-=360;
					InvalidateRect(hWnd, NULL, FALSE);
					break;
				case VK_LEFT:
					pivot_horiz_angle-=5;
					if (pivot_horiz_angle<0)
						pivot_horiz_angle+=360;
					InvalidateRect(hWnd, NULL, FALSE);
					break;

				// przesuniecia w poziomie
				case VK_NEXT:
					if ((pivot_x+change)<400)
						pivot_x+=change;
					InvalidateRect(hWnd, NULL, FALSE);
					break;
				case VK_DELETE:
					if ((pivot_x-change)>-400)
						pivot_x-=change;
					InvalidateRect(hWnd, NULL, FALSE);
					break;

				// przesuniecia w pionie
				case VK_HOME:
					if ((pivot_y+change)<400)
						pivot_y+=change;
					InvalidateRect(hWnd, NULL, FALSE);
					break;
				case VK_END:
					if ((pivot_y-change)>-400)
						pivot_y-=change;
					InvalidateRect(hWnd, NULL, FALSE);
					break;

				// odleglosc
				case VK_INSERT:
					if ((z_dist+change)<600)
						z_dist+=change;
					InvalidateRect(hWnd, NULL, FALSE);
					break;
				case VK_PRIOR:
					if ((z_dist-change)>1)
						z_dist-=change;
					InvalidateRect(hWnd, NULL, FALSE);
					break;

				// wyjscie z programu
				case VK_ESCAPE:
					PostMessage(hWnd, WM_CLOSE, 0, 0);
					return 0;
			}
        }

    
    return DefWindowProc( hWnd, msg, wParam, lParam );
}



//*******************************************************
//  Uruchomienie OpenGL w srodowisku Windows wymaga 
//  wlaczenie trybu pikselowego oraz ustawienia 
//  kontekstu renderowania
//*******************************************************

HGLRC SetUpOpenGL( HWND hWnd )
{
    static PIXELFORMATDESCRIPTOR pfd =
	{
        sizeof (PIXELFORMATDESCRIPTOR), // rozmiar struktury 
        1,                              // numer wersji
        PFD_DRAW_TO_WINDOW |            // flaga rysowania w oknie,
            PFD_SUPPORT_OPENGL |        // uzycie OpenGL
		    PFD_DOUBLEBUFFER,	        // uzycie podwojnego buforowania
        PFD_TYPE_RGBA,                  // model kolorow - RGBA 
        24,                             // kolor 24-bitowy
        0, 0, 0,                        
        0, 0, 0,                        // trezerwa
        0, 0,                           // brak bufora alpha 
        0, 0, 0, 0, 0,                  // brak bufora akumulacji
        32,                             // bufor glebokosci 32-bitowy
        0,                              // brak bufora szablonu
        0,                              // brak bufora pomocniczego
        PFD_MAIN_PLANE,					// warstwa
        0,                              // rezerwa - 0 
        0,                              // maska
        0,                              // maska widoczna
        0                               // maska bledow
    };

	// zmienne
    int nMyPixelFormatID;

    HDC hDC;
    
	HGLRC hRC;

    hDC = GetDC( hWnd );
    
	// Inicjalizacja trybu pikselowego 

	nMyPixelFormatID = ChoosePixelFormat( hDC, &pfd );
    SetPixelFormat( hDC, nMyPixelFormatID, &pfd );
	
	// Inicjalizacja kontekstu renderowania
    
	hRC = wglCreateContext( hDC );
    ReleaseDC( hWnd, hDC );


	srand(time(0));

    return hRC;
}


int dupa=0;
//******************************************************** 
//  Glowna funkcja rysujaca.
//******************************************************** 
void DrawOpenGLScene( )
{
	GLfloat position[4]={10.0f, 10.0, 100.0f, 0.0f};
    
	// flagi czynnosci pomocniczych
	glEnable(GL_TEXTURE_2D);
    glEnable( GL_DEPTH_TEST );

    
	// czyszczenie buforow
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	//glEnable(GL_CULL_FACE);
	glEnable(GL_COLOR_MATERIAL);

	auxInitDisplayMode(AUX_SINGLE|AUX_RGBA);
	
	// wlaczenie oswietlenia
	glLightfv(GL_LIGHT0, GL_POSITION, position);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHTING);

    
    // transformacja obserwacji
    
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();

    // umiejscowienie kamery w odleglym miejscu
  	glTranslatef( pivot_x, pivot_y, -z_dist );
	glRotatef(pivot_vert_angle, 1, 0, 0);
	glRotatef(pivot_horiz_angle, 0, 1, 0);
	
	//glPushMatrix();
		//szescian !!!!!!!!!!!!!!!!!!!!!!!!!!
		if(dupa==0)
		{
			ReadOBJModel("woodenbox.obj", box1, "woodenbox.bmp");
			SetPosition(box1, rand()%5, 100, rand()%6);
			SetRotate(box1, rand()%360);
			ReadOBJModel("czolg.obj", kadlub1, "kadlub.bmp");
			ReadOBJModel("lufa.obj", lufa1, "lufa.bmp");
			ReadOBJModel("wieza.obj", wieza1, "wieza.bmp");
			ReadOBJModel("ziemia.obj", ziemia1, "trawa.bmp");
			ReadOBJModel("czolg.obj", kadlub2, "kadlub.bmp");
			ReadOBJModel("lufa.obj", lufa2, "lufa.bmp");
			ReadOBJModel("wieza.obj", wieza2, "wieza.bmp");
			dupa++;
		}
		glTranslatef(-20,0,0);
		
		RenderOBJModel(box1);
		RenderOBJModel(lufa1);
		RenderOBJModel(kadlub1);
		RenderOBJModel(wieza1);
		RenderOBJModel(ziemia1);
		glTranslatef(20,0,-4);
		glRotatef(180,0,1.0,0);
		RenderOBJModel(lufa2);
		
		RenderOBJModel(kadlub2);
		RenderOBJModel(wieza2);

		glTranslatef(10,0,0);

		
	//glPopMatrix();

    glFlush ();
}


