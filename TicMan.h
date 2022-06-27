#ifndef TICMAN_H_INCLUDED
#define TICMAN_H_INCLUDED


/// Tamaño matriz
#define N 20


///Estructuras

///---- Structs para control de juego ----


///---- Fin structs control de juego---

typedef struct
{
    int x,y;
    int direcciones[4]; ///cantidad de direcciones que puede tomar el pac man
}Vertices;

typedef struct
{
    int mapa[N][N];
    int cantPastillas;
    int NV;
    Vertices *grafo; ///Aux caminos, (los caminos estan definidos con un grafo)
}Escenario;

typedef struct
{
    int status;
    int xi,yi,x,y;
    int direcc,paso,parcial;
    int puntos;
    int invencible;
    int vivo;
    int animacion;
}TicMan;

typedef struct
{
    int status;
    int xi,yi,x,y;
    int direcc,paso,parcial;
	int cruce,empezarDeVuelta;
	int indiceActual;
	int *camino;
}Fantasma;

///Funciones

void cargarTexturas ();
void poneSprites(float colum,float fila, GLuint tex);
void posicionaTitulos(float x, float y, float tam, GLuint tex);
void titulos(int tipo);
GLuint cargarTexDelArchi(char *str);
Escenario * cargarEscenario(char *archivo);
void destruyeEscenario(Escenario* cen);
void diseniaEscenario(Escenario* escen);

///Funciones del pacman

void animacionMuerteTicMan(float columna,float linia,TicMan* tic);
TicMan* CrearTicMan(int x, int y);
void DestruirTicMan(TicMan *tic);
int TicManVivo(TicMan *tic);
void nuevaDirecTicMan(TicMan *tic, int direccion, Escenario *escen);
void MovimientoTicMan(TicMan *tic, Escenario * escen);
void DisenioTicMan(TicMan *tic);
static int ticManInvencible(TicMan *tic);
static void muerteTicMan(TicMan *tic);
static void puntosFantasmas(TicMan *tic);

///Funciones del fantasma

Fantasma* crearFantasma(int x, int y);
void DestruirFantasma(Fantasma *fan);
void diseniaFantasma(Fantasma * fan);
void movimientoFantasma(Fantasma *fan, Escenario *escen, TicMan * tic);
void moverFantasma(Fantasma*fan, int direccion, Escenario *escen);
int fantasmaDireccionMuerte(Fantasma*fan, Escenario *escen);
static int fantasmaDistanciaMuerte(Escenario *escen, int noA, int noB);
int dibujarDireccionFantasma(Fantasma*fan, Escenario *escen);
int movimientoFantasmaVivo(Fantasma * fan, TicMan * tic, Escenario *escen);
int fantasmaVeAlPacMan(Fantasma*fan, TicMan*tic, Escenario * escen, int direccion);
void fantasmaMenorCamino(Fantasma*fan, Escenario *escen);
int movimientoFantasmaMuerto(Fantasma * fan, Escenario * escen);


#endif
