#include <windows.h>
#include <gl/gl.h>
#include "SOIL.h"
#include <stdio.h>
#include <time.h>
#include "TicMan.h"

LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
void EnableOpenGL(HWND hwnd, HDC*, HGLRC*);
void DisableOpenGL(HWND, HDC, HGLRC);



Escenario * escen;
TicMan * tic;
Fantasma * fan[4];      ///4 fantasmas
WNDCLASSEX wcex;
HWND hwnd;
HDC hDC;
HGLRC hRC;
MSG msg;
BOOL bQuit = FALSE;
float theta = 0.0f;
int inicioJuego=0;



/// Inicializacion de funciones ///

void iniciarJuego();
void diseniarJuego();
void terminarJuego();
void elegirResolucion(int * resolucionX, int * resolucionY);

///Funcion que configura y crea la  ventana grafica del OpenGL ///

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
/// MAIN ANTES DE LA PANTALLA DE OPENGL ///

    int resolucionX, resolucionY;
    elegirResolucion(&resolucionX, &resolucionY);











/// LINEAS DE APERTURA Y MANEJO DE PANTALLA ///


    /* register window class */
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_OWNDC;
    wcex.lpfnWndProc = WindowProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = "GLSample";
    wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);;


    if (!RegisterClassEx(&wcex))
        return 0;

    /* create main window */
    hwnd = CreateWindowEx(0,
                          "GLSample",
                          "Tic Man",
                          WS_OVERLAPPEDWINDOW,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          resolucionX,
                          resolucionY,
                          NULL,
                          NULL,
                          hInstance,
                          NULL);

    ShowWindow(hwnd, nCmdShow);

    /* enable OpenGL for the window */
    EnableOpenGL(hwnd, &hDC, &hRC);
    iniciarJuego();

    /* program main loop */
    while (!bQuit)
    {
        /* check for messages */
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            /* handle or dispatch messages */
            if (msg.message == WM_QUIT)
            {
                bQuit = TRUE;
            }
            else
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else
        {
            /* OpenGL animation code goes here */

            glClearColor(0.0f, 0.0f, 0.0f, 0.0f); ///Color del fondo
            glClear(GL_COLOR_BUFFER_BIT);///Limpia el buffer

            glPushMatrix();

            diseniarJuego();

            glPopMatrix();

            SwapBuffers(hDC);


            Sleep (8);
        }
    }

    terminarJuego();
    /* shutdown OpenGL */
    DisableOpenGL(hwnd, hDC, hRC);

    /* destroy the window explicitly */
    DestroyWindow(hwnd);



    return msg.wParam;
}
///Funcion que verifica si apretaste el teclado

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_CLOSE:
            PostQuitMessage(0);
        break;

        case WM_DESTROY:
            return 0;

        case WM_KEYDOWN:
        {
            switch (wParam)
            {
                 case VK_ESCAPE: ///ESC
                    PostQuitMessage(0);
                    break;
                case VK_RIGHT: ///Derecha
                    nuevaDirecTicMan(tic,0,escen);
                    break;
                case VK_DOWN: ///Abajo
                    nuevaDirecTicMan(tic,1,escen);
                    break;
                case VK_LEFT: ///Izquierda
                    nuevaDirecTicMan(tic,2,escen);
                    break;
                case VK_UP: ///Arriba
                    nuevaDirecTicMan(tic,3,escen);

                    break;
                case 'P':
                    if(inicioJuego == 2)
                    {
                        terminarJuego();
                        iniciarJuego();
                    }
                    inicioJuego = 1;
                    break;
                case 'N':
                    if(inicioJuego == 2)
                    {
                        terminarJuego();
                        DisableOpenGL(hwnd, hDC, hRC);
                        DestroyWindow(hwnd);

                    }
                    break;

                break;
            }
        }
        break;
        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}
///Funcion que configura el OPenGL
void EnableOpenGL(HWND hwnd, HDC* hDC, HGLRC* hRC)
{
    PIXELFORMATDESCRIPTOR pfd;

    int iFormat;

    /* get the device context (DC) */
    *hDC = GetDC(hwnd);

    /* set the pixel format for the DC */
    ZeroMemory(&pfd, sizeof(pfd));

    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW |
                  PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 16;
    pfd.iLayerType = PFD_MAIN_PLANE;

    iFormat = ChoosePixelFormat(*hDC, &pfd);

    SetPixelFormat(*hDC, iFormat, &pfd);

    /* create and enable the render context (RC) */
    *hRC = wglCreateContext(*hDC);

    wglMakeCurrent(*hDC, *hRC);
    cargarTexturas(); ///carga texturas
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); // Linear Filtering
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
}

void DisableOpenGL (HWND hwnd, HDC hDC, HGLRC hRC)
{
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hRC);
    ReleaseDC(hwnd, hDC);
}
void diseniarJuego()
{
    int i=0;
    diseniaEscenario(escen);

      if(inicioJuego == 0){
        titulos(0);
        return;
    }

    if(TicManVivo(tic))
    {
        MovimientoTicMan(tic, escen);
        DisenioTicMan(tic);

        for(i=0; i<4; i++)
        {
            movimientoFantasma(fan[i], escen, tic);
            diseniaFantasma(fan[i]);
        }
    }
    else
    {
        titulos(1);
        inicioJuego = 2;
    }

}

void iniciarJuego()
{
    int i =0;
    int mapaAleatorio =0;
    srand(time(NULL));
    mapaAleatorio = rand()%4+1;
    switch (mapaAleatorio)
    {
    case 1:
        escen = cargarEscenario("mapita1.txt");
        break;
    case 2:
        escen = cargarEscenario("mapita2.txt");
        break;
    case 3:
        escen = cargarEscenario("mapita3.txt");
        break;
    case 4:
        escen = cargarEscenario("mapita4.txt");
        break;
    }

    tic = CrearTicMan (9,13);

    for(i=0; i<4; i++)
        fan[i] = crearFantasma(9,9);

    inicioJuego = 0;
}

void terminarJuego()
{
    int i;
    for(i=0; i<4; i++)
    {
         DestruirFantasma(fan[i]);
    }
    DestruirTicMan(tic);
    destruyeEscenario(escen);
}

void elegirResolucion(int * resolucionX, int * resolucionY)
{
    int x;
    printf("Eliga una resolucion:   \n");                    ///1366 x 768 resolucion del profe.
    printf("1 - 800x600\n");
    printf("2 - 1240x720\n");
    printf("3 - 1366x768\n");
    printf("4 - 1920x1080\n");

    x=getche();

    switch (x)
    {
        case '1':
            *resolucionX=800;
            *resolucionY=600;
            break;
        case '2':
            *resolucionX=1240;
            *resolucionY=720;
            break;
        case '3':
            *resolucionX=1366;
            *resolucionY=768;
            break;
        case '4':
            *resolucionX=1920;
            *resolucionY=1080;
            break;
    }
}
