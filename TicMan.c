#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <windows.h>
#include <gl/gl.h>
#include "SOIL.h"
#include "TicMan.h"

/// Tamaño de cada bloque de la matriz del juego
#define bloque 70
/// Tamaño matriz
#define N 20
/// Tamaño de cada bloque de la matriz en la pantalla
#define TAM 0.1f
///Convierten una columna y una fila de una matriz en una coordenada de [-1,1]
#define MAT2X(j) ((j)*0.1f-1)
#define MAT2Y(i) (0.9-(i)*0.1f)



///cantidad de texturas que tendra cada sprite

struct Direcciones
{
    int x;
    int y;
};

const struct Direcciones direc[4] = {{1,0},{0,1},{-1,0},{0,-1}};

GLuint ticmanTex[12];
GLuint fantasmaTex[12];
GLuint mapaTex[14];
GLuint Start;
GLuint GameOver;


///Funciones para cargar las texturas

GLuint cargarTexDelArchi(char *str)  ///Carga las texturas del archivo dejandolas en png
{
     GLuint tex = SOIL_load_OGL_texture(
            str,
            SOIL_LOAD_AUTO,
            SOIL_CREATE_NEW_ID,
            SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y |
            SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
        );
     ///check for an error during the load process
    if(0 == tex){
        printf( "SOIL loading error: '%s'\n", SOIL_last_result() );
    }

    return tex;
}


void cargarTexturas ()
{
    int i=0;
    char string [50]; ///Cantidad de texturas a cargar

     for(i=0; i<12; i++)
    {
        sprintf(string,".//Sprites//phantom%d.png", i); ///lugar donde se encuentran las texturas.
        fantasmaTex[i] = cargarTexDelArchi(string);
    }

    for (i=0; i<12; i++)
    {
        sprintf(string,".//Sprites//pacman%d.png", i);
        ticmanTex[i] = cargarTexDelArchi(string);
    }

    for(i=0; i<14; i++){
        sprintf(string,".//Sprites//mapa%d.png", i);
        mapaTex[i] = cargarTexDelArchi(string);
    }

    Start = cargarTexDelArchi(".//Sprites//titulo.png");
    GameOver = cargarTexDelArchi(".//Sprites//gameover.png");

}

///Donde se le pase una posicion crea un cuadrado con esa textura generando las fisicas de la misma
void poneSprites(float colum,float fila, GLuint tex)
{
    glColor3f(1.0, 1.0, 1.0);///negro
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBegin(GL_QUADS);
    glTexCoord2f(0.0f,0.0f);
    glVertex2f(colum, fila);
    glTexCoord2f(1.0f,0.0f);
    glVertex2f(colum+TAM, fila);
    glTexCoord2f(1.0f,1.0f);
    glVertex2f(colum+TAM, fila+TAM);
    glTexCoord2f(0.0f,1.0f);
    glVertex2f(colum, fila+TAM);
    glEnd();

    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);

}

///Posiciona los sprites
void posicionaTitulos(float x, float y, float tam, GLuint tex)
{

    glPushMatrix();

    glColor3f(1.0, 1.0, 1.0); ///negro
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBegin(GL_QUADS);
        glTexCoord2f(0.0f,1.0f); glVertex2f(x - tam, y + tam);
        glTexCoord2f(1.0f,1.0f); glVertex2f(x + tam, y + tam);
        glTexCoord2f(1.0f,0.0f); glVertex2f(x + tam, y - tam);
        glTexCoord2f(0.0f,0.0f); glVertex2f(x - tam, y - tam);
    glEnd();

    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);

    glPopMatrix();
}

void titulos(int tipo)
{
    if(tipo == 0)
        posicionaTitulos(0, 0, 1.0, Start);
    else
        posicionaTitulos(0, 0, 1.0, GameOver);
}

///Funciones para cargar el escenario


///carga los datos del escenario que estan en un archivo de texto

Escenario * cargarEscenario(char *archivo)
{
    int i,j;

    FILE *archi = fopen(archivo,"r");

    if(archi == NULL)
    {
        printf("Erro al cargar escenario\n");
        system("pause");
        exit(1);
    }

    Escenario* escen = malloc(sizeof(Escenario));
    escen->cantPastillas = 0;

    for(i=0; i<N; i++)
        for(j=0; j<N; j++)
        {
            fscanf(archi,"%d",&escen->mapa[i][j]);
            if(escen->mapa[i][j] == 1 || escen->mapa[i][j] == 2)
                escen->cantPastillas++;
        }

    fclose(archi);

    caminoFantasmasMuertos(escen);
    return escen;
}


void destruyeEscenario(Escenario* cen)
{
    free(cen->grafo); ///pq es puntero
    free(cen);
}

void diseniaEscenario(Escenario* escen)
{
    int i,j;
    for(i=0; i<N; i++)
        for(j=0; j<N; j++)
            poneSprites(MAT2X(j),MAT2Y(i),mapaTex[escen->mapa[i][j]]); ///va pasando los puntos de la matriz a coordenadas
}

///Funciones para marcar los caminos validos
/// Verifica si una posicion es una interseccion o una pared
 int caminosEscenario(int x, int y, Escenario* escen)
{
    int i;
    int cont = 0;
    int v[4];
    int h=0;

    for(i=0; i<4; i++)
    {
        if(escen->mapa[y + direc[i].y][x + direc[i].x] <=2) ///si es menor o igual a dos significa que es: un espacio vacio o un orbe de puntos
        {
            cont++;
            v[i] = 1;
        }
        else
        {
            v[i] = 0;
        }
    }
    if(cont > 1)
    {
        if(cont == 2)
        {
            if((v[0] == v[2] && v[0]) || (v[1] == v[3] && v[1]))
            {
               h=0; /// camino recto
            }
            else
            {
                  h=1;/// camino L
            }
        }else
        {
             h=1;/// camino T
        }
    }else
    {
         h=0;/// camino sin salida
    }

  return h;
}

/// Se fija si es posible andar en la direccion deseeada
 int verificaDireccion(int matriz[N][N], int y, int x, int direccion)
{
    int xt = x;
    int yt = y;
    while(matriz[yt + direc[direccion].y][xt + direc[direccion].x] == 0)
    {
        yt = yt + direc[direccion].y;
        xt = xt + direc[direccion].x;
    }

    if(matriz[yt + direc[direccion].y][xt + direc[direccion].x] < 0)
        return -1;
    else
        return matriz[yt + direc[direccion].y][xt + direc[direccion].x] - 1;
}

/// Esta funcion hara que los fantasmas vuelvan al punto de partida una vez muertos
 void caminoFantasmasMuertos(Escenario* escen)
{
    int matriz[N][N];
    int i,j,k,idx;
    int cont = 0;

    for(i=1; i<N-1; i++)
    {
        for(j=1; j<N-1; j++)
        {
            if(escen->mapa[i][j] <= 2) ///si no hay pared
            {
                if(caminosEscenario(j,i,escen))
                {
                    cont++;
                    matriz[i][j] = cont;
                }else
                    matriz[i][j] = 0;
            }else
                matriz[i][j] = -1;
        }
    }

    for(i=0; i < N; i++) ///fantasmas no se vayan del escenario
    {
        matriz[0][i] = -1;
        matriz[i][0] = -1;
        matriz[N-1][i] = -1;
        matriz[i][N-1] = -1;
    }

    escen->NV = cont;
    escen->grafo = malloc(cont * sizeof(Vertices));

    for(i=1; i<N-1; i++)
    {
        for(j=1; j<N-1; j++)
        {
            if(matriz[i][j] > 0)
            {
                idx = matriz[i][j] - 1;
                escen->grafo[idx].x = j;
                escen->grafo[idx].y = i;
                for(k=0; k<4; k++)
                {
                    escen->grafo[idx].direcciones[k] = verificaDireccion(matriz,i,j,k);
                }

            }
        }
    }

}

///Funciones para el control del pacman

TicMan* CrearTicMan(int x, int y)
{
    TicMan* tic = malloc(sizeof(TicMan));
    if(tic != NULL)
    {
        tic->invencible= 0;
        tic->puntos = 0;
        tic->paso = 4;///4 direcciones
        tic->vivo = 1;
        tic->status = 0;
        tic->direcc= 0;
        tic->parcial = 0;
        tic->xi = x;
        tic->yi = y;
        tic->x = x;
        tic->y = y;
    }
    return tic;
}

///Libera los datos del ticman
void DestruirTicMan(TicMan *tic)
{
    free(tic);
}

///Chequea si el ticman esta vivo o no
int TicManVivo(TicMan *tic)
{
    if(tic->vivo)
        return 1;
    else
    {
        if(tic->animacion > 60)
            return 0;
        else
            return 1;
    }
}

///Esta funcion verificara si es valida la nueva direccion que tome el ticman
void nuevaDirecTicMan(TicMan *tic, int direccion, Escenario *escen)
{
    int di=0;
    if(escen->mapa[tic->y + direc[direccion].y][tic->x + direc[direccion].x] <=2)///no es pared es punto
    {
        di = abs(direccion - tic->direcc); ///La funcion abs es una funcion de la biblioteca stlib que devuelve el valor absoluto del valor q se le pase
        if(di != 2 && di != 0)
            tic->parcial = 0;

        tic->direcc = direccion;
    }
}
///Aumenta su posicion de una celda a otra
void MovimientoTicMan(TicMan *tic, Escenario * escen)
{
    if(tic->vivo == 0)///ta muerto
        return;


    if(escen->mapa[tic->y + direc[tic->direcc].y][tic->x + direc[tic->direcc].x] <=2)
    {
        if(tic->direcc < 2)
        {
            tic->parcial += tic->paso; /// += es equivalente a x = x+exp
            if(tic->parcial >= bloque)
            {
                tic->x += direc[tic->direcc].x;
                tic->y += direc[tic->direcc].y;
                tic->parcial = 0;
            }
        }
        else
            {
            tic->parcial -= tic->paso;
            if(tic->parcial <= -bloque)
            {
                tic->x += direc[tic->direcc].x;
                tic->y += direc[tic->direcc].y;
                tic->parcial = 0;
            }
        }
    }

    /// si se come una pastilla del mapa suma puntos
    if(escen->mapa[tic->y][tic->x] == 1)///pastilla chica
    {
        tic->puntos += 10;
        escen->cantPastillas--;
    }
    if(escen->mapa[tic->y][tic->x] == 2)///pastilla grande
    {
        tic->puntos += 50;
        tic->invencible = 1000;
        escen->cantPastillas--;
    }
    ///saca la pastilla comida
    escen->mapa[tic->y][tic->x] = 0;
}

void animacionMuerteTicMan(float columna,float linia,TicMan* tic)
{
    tic->animacion++;

    /// Se fija q combinacion de animaciones usar para dar el efecto de muerte
    if(tic->animacion < 15)
        poneSprites(MAT2X(columna),MAT2Y(linia), ticmanTex[8]);
    else
        if(tic->animacion < 30)
            poneSprites(MAT2X(columna),MAT2Y(linia), ticmanTex[9]);
        else
            if(tic->animacion < 45)
                poneSprites(MAT2X(columna),MAT2Y(linia), ticmanTex[10]);
            else
                poneSprites(MAT2X(columna),MAT2Y(linia), ticmanTex[11]);
}

///Funcion que diseña el mapa
void DisenioTicMan(TicMan *tic)
{
    float linia, columna;
    float passo = (tic->parcial/(float)bloque);
    int idx=0;

    if(tic->direcc == 0 || tic->direcc == 2) ///Verifica la pocision
    {
        linia = tic->y;
        columna = tic->x + passo;
    }
    else
    {
        linia = tic->y + passo;
        columna = tic->x;
    }
    if(tic->vivo){
        /// Elige el sprite del ticMan segun la direccion a la q apunte
        idx = 2 * tic->direcc;

        /// se fija si lo dibuja con la boca abierta o cerrada
        if(tic->status < 15)
            poneSprites(MAT2X(columna),MAT2Y(linia), ticmanTex[idx]);
        else
            poneSprites(MAT2X(columna),MAT2Y(linia), ticmanTex[idx+1]);

        /// Va alternando entre el sprite de boca abierta y cerrada para q genere la animacion
        tic->status = (tic->status+1) % 30;

        if(tic->invencible > 0)
            tic->invencible--;
    }
    else
    {
        /// Mostrar animacion de muerte
        animacionMuerteTicMan(columna,linia,tic);
    }
}

static int ticManInvencible(TicMan *tic)
{
    return tic->invencible > 0;
}

static void muerteTicMan(TicMan *tic)
{
    if(tic->vivo)
    {
        tic->vivo = 0;
        tic->animacion = 0;
    }
}

static void puntosFantasmas(TicMan *tic)
{
    tic->puntos += 100;
}

///Fantasmas

Fantasma* crearFantasma(int x, int y)
{
    Fantasma* fan = malloc(sizeof(Fantasma));///Re fan el fantasma
    if(fan != NULL){
        fan->paso = 3;
        fan->cruce = 0;
        fan->empezarDeVuelta = 0;
        fan->indiceActual = 0;
        fan->status = 0;
        fan->direcc = 0;
        fan->parcial = 0;
        fan->xi = x;
        fan->yi = y;
        fan->x = x;
        fan->y = y;
        fan->camino = NULL;
    }
    return fan;
}

/// Libera los datos asociados al fantasma
void DestruirFantasma(Fantasma *fan)
{
    if(fan->camino != NULL)
    {
          free(fan->camino);
    }

    free(fan);
}

/// Diseña un fantasma
void diseniaFantasma(Fantasma * fan)
{
    int idx=0;
    float linia, columna;
    float paso = (fan->parcial/(float)bloque);

    ///Verifica a posição
    if(fan->direcc == 0 || fan->direcc == 2)///Se verifica la posicion
    {
        linia = fan->y;
        columna = fan->x + paso;
    }
    else
    {
        linia = fan->y + paso;
        columna = fan->x;
    }
    /// Escoge los sprites para ponerlos en las direcciones y el estatus
    idx = 3*fan->direcc + fan->status;
    poneSprites(MAT2X(columna),MAT2Y(linia), fantasmaTex[idx]);
}


void movimientoFantasma(Fantasma *fan, Escenario *escen, TicMan * tic)
{
    int d=0;

    if(fan->status == 1)///si el estatus del fantasma es 1 significa q esta muerto
        {
           d = movimientoFantasmaMuerto(fan, escen);
    }else
    {///Si el estatus es 2 huira del ticman, si es 0 lo perseguira
            if(ticManInvencible(tic))
            {
                fan->status = 2;
            }else{
                fan->status = 0;
            }
       ///Llama a la funcion que le indicara a donde moverse segun la posicion del pacman
        d = movimientoFantasmaVivo(fan, tic, escen);

        if(tic->x == fan->x && tic->y == fan->y) /// Que pasa si el pacman toca al fantasma?
        {
            if(ticManInvencible(tic))
            {
                fan->status = 1; ///RIP fantasma
                puntosFantasmas(tic);
                fan->empezarDeVuelta = 0;
            }else{
                if(TicManVivo(tic))
                    muerteTicMan(tic);
            }
        }
    }
    moverFantasma(fan,d,escen); /// Va moviendo y dibujando el fantasma en la pantalla
}

/// Esta funcion va actualizando la posicion del fantasma, esta funcion es similar a la del ticman
void moverFantasma(Fantasma*fan, int direccion, Escenario *escen)
{
    int xt = fan->x;
    int yt = fan->y;


    if(escen->mapa[fan->y + direc[direccion].y][fan->x + direc[direccion].x] <=2)
    {
        if(direccion == fan->direcc)
        {
            if(fan->direcc < 2)
            {
                fan->parcial += fan->paso;
                if(fan->parcial >= bloque)
                {
                    fan->x += direc[direccion].x;
                    fan->y += direc[direccion].y;
                    fan->parcial = 0;
                }
            }else{
                fan->parcial -= fan->paso;
                if(fan->parcial <= -bloque)
                    {
                        fan->x += direc[direccion].x;
                        fan->y += direc[direccion].y;
                        fan->parcial = 0;
                    }
            }
        }else{
            if(abs(direccion - fan->direcc) != 2)///Cambiar direccion
                fan->parcial = 0;

            fan->direcc = direccion;
        }
    }

    if(xt != fan->x || yt != fan->y)
        fan->cruce = 0;
}

///Ayuda a elegir el camino del fantasma cuando muere
int fantasmaDireccionMuerte(Fantasma*fan, Escenario *escen)
{
    if(escen->grafo[fan->indiceActual].x == escen->grafo[fan->camino[fan->indiceActual]].x)
    {
        if(escen->grafo[fan->indiceActual].y > escen->grafo[fan->camino[fan->indiceActual]].y)
        {
            return 3;
        }
        else
        {
            return 1;
        }

    }
    else
    {
        if(escen->grafo[fan->indiceActual].x > escen->grafo[fan->camino[fan->indiceActual]].x)
        {
            return 2;
        }
        else
        {
               return 0;
        }

    }

}

/// Ayuda a elegir el camino del fantasma cuando muere parte 2
static int fantasmaDistanciaMuerte(Escenario *escen, int noA, int noB)
{
    return fabs(escen->grafo[noA].x - escen->grafo[noB].x) + fabs(escen->grafo[noA].y - escen->grafo[noB].y); ///La funcion fabs al igual que la funcion abs devuelve el valor absoluto de un n umero pero es tipo double
}

///Si el fantasma se encuentra con una interseccion dibuja una nueva direccion
///Tambien hace que no vuelvan por el mismo camino casi nunca
int dibujarDireccionFantasma(Fantasma*fan, Escenario *escen)
{
    int i,j,k,mayor;
    int peso[4], dir[4];

    for(i=0; i<4; i++)
    {
         peso[i] = rand() % 10 + 1;
    }


    peso[fan->direcc] = 7;
    peso[(fan->direcc + 2) % 4] = 3;

    /// A cada direccion le pondra un "peso" y las ordenara
    for(j=0; j<4; j++)
        {
            mayor = 0;
            for(i=0; i<4; i++)
            {
                if(peso[i] > mayor)
                {
                    mayor = peso[i];
                    k = i;
                }
            }
            dir[j] = k;
            peso[k] = 0;
        }

    /// Escoge la primer direc valida
    i = 0;

    while(escen->mapa[fan->y + direc[dir[i]].y][fan->x + direc[dir[i]].x] > 2)
    {
        i++;
    }


    return dir[i];
}


int movimientoFantasmaVivo(Fantasma * fan, TicMan * tic, Escenario *escen)
{
    int d, i;
    if(caminosEscenario(fan->x, fan->y, escen))
    {
        if(!fan->cruce)
        {
            /// viste al ticman? seguilo
            d = -1;
            for(i=0; i<4; i++)
                if(fantasmaVeAlPacMan(fan, tic, escen, i))
                    d = i;

            /// no viste al ticman? elegi una direc
            if(d == -1)
                d = dibujarDireccionFantasma(fan, escen);
            else{
                /// el ticman es invencible? Huir en nueva direcc
                if(ticManInvencible(tic))
                {
                    i = d;
                    while(i == d)
                        d = dibujarDireccionFantasma(fan, escen);
                }
            }
            fan->cruce = 1;
        }else
            d = fan->direcc;
    }else
    {
        ///no se cruzan? segui en la misma direc
        fan->cruce = 0;
        d = fan->direcc;
        if(ticManInvencible(tic))
        {
            ///Si viste al pacman y es invensible ir en direc opuesta
            if(fantasmaVeAlPacMan(fan, tic, escen, d))
                d = (d + 2) % 4;
        }


        if(escen->mapa[fan->y + direc[d].y][fan->x + direc[d].x] >2)
            d = (d + 2) % 4;
    }
    return d;
}

/// Funcion que arma la interaccion cuando el fantrasma ve al pacman
int fantasmaVeAlPacMan(Fantasma*fan, TicMan*tic, Escenario * escen, int direccion)
{
    int continua = 0;
    int xt=0;
    int yt=0;

    if(direccion == 0 || direccion == 2)
    {
        if(tic->y == fan->y)
            continua = 1;
    }else
    {
        if(tic->x == fan->x)
            continua = 1;
    }

    if(continua)
    {
        xt = fan->x;
        yt = fan->y;
        while(escen->mapa[yt + direc[direccion].y][xt + direc[direccion].x] <= 2)
        {
            yt = yt + direc[direccion].y;
            xt = xt + direc[direccion].x;

            if(xt == tic->x && yt == tic->y)
                return 1;
        }
    }
    return 0;
}

/// Ayuda a escojer el menor camino cuando el fantasma muere
void fantasmaMenorCamino(Fantasma*fan, Escenario *escen)
{
    int i, k, indice;
    int continua, d;
    int *dist;

    dist = malloc(escen->NV*sizeof(int));
    if(fan->camino == NULL)
        fan->camino = malloc(escen->NV*sizeof(int));


    for(i=0; i<escen->NV; i++) /// Empieza a calcular el menor camino...
    {
        dist[i] = 10000;
        fan->camino[i] = -1;
        if(escen->grafo[i].x == fan->xi && escen->grafo[i].y == fan->yi)
            indice = i;

        if(escen->grafo[i].x == fan->x && escen->grafo[i].y == fan->y)
            fan->indiceActual = i;
    }

    dist[indice] = 0;

    ///calcula el menor camino...

    continua = 1;
    while(continua)
    {
        continua = 0;
        for(i=0; i<escen->NV; i++)
        {
            for(k=0; k<4; k++)
            {
                if(escen->grafo[i].direcciones[k] >= 0)
                {
                    d = fantasmaDistanciaMuerte(escen, i, escen->grafo[i].direcciones[k]);
                    if(dist[escen->grafo[i].direcciones[k]] > (dist[i] + d))
                    {
                        dist[escen->grafo[i].direcciones[k]] = (dist[i] + d);
                        fan->camino[escen->grafo[i].direcciones[k]] = i;
                        continua = 1;
                    }
                }
            }
        }
    }
    free(dist);
}


/// Ayuda a elegir el camino cuando muere el fantasma
int movimientoFantasmaMuerto(Fantasma * fan, Escenario * escen)
{
    int d;

    if(!fan->empezarDeVuelta)
    {
        if(caminosEscenario(fan->x, fan->y, escen))
        {
            fan->empezarDeVuelta = 1;
            fantasmaMenorCamino(fan, escen);
            fan->cruce = 1;
            d = fantasmaDireccionMuerte(fan, escen);
        }else
        {
            d = fan->direcc;
            if(escen->mapa[fan->y + direc[d].y][fan->x + direc[d].x] > 2)///si es pared
                d = (d + 2) % 4;
        }
    }else{///vuelve
        if(fan->x != fan->xi || fan->y != fan->yi)
        {
            if(caminosEscenario(fan->x, fan->y, escen))
            {
                if(fan->cruce)
                    d = fan->direcc;
                else{///se fija que direccion tomar
                    fan->indiceActual = fan->camino[fan->indiceActual];
                    d = fantasmaDireccionMuerte(fan, escen);
                    fan->cruce = 1;
                }
            }else{
                d = fan->direcc;
                fan->cruce = 0;
            }
        }else{
            fan->status = 0;
            d = fan->direcc;
        }
    }
    return d;
}

