#include <iostream>
#include <vector>
#include <cmath>
#include <SDL2/SDL.h>

using namespace std;

// estruturas basicas

struct Vetor3 {
    double x, y, z;
    // sobrecarga de operadores para facilitar a matematica vetorial no codigo
    Vetor3 operator-(const Vetor3& b) const { return {x - b.x, y - b.y, z - b.z}; }
    Vetor3 operator+(const Vetor3& b) const { return {x + b.x, y + b.y, z + b.z}; }
    Vetor3 operator*(double s) const { return {x * s, y * s, z * s}; }
    Vetor3 operator/(double s) const { return {x / s, y / s, z / s}; }
};

struct Vetor2 {
    double u, v; 
};

struct Matriz4 {
    double m[4][4];
};

struct Plano {
    Vetor3 p1, p2, p3, r0;
    // vetores unitarios que definem a direcao x e y localmente dentro do plano
    Vetor3 eixoX, eixoY; 
};

struct Superficie {
    vector<int> indicesVertices;
};

struct Objeto3D {
    vector<Vetor3> vertices;
    vector<Superficie> faces;
};

struct Janela {
    double minX, maxX, minY, maxY;
};

struct Viewport {
    int x, y, w, h;
};

struct Camera {
    double anguloX, anguloY, distancia;
};

// funcoes de matematica vetorial

double produtoEscalar(const Vetor3& a, const Vetor3& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

// calcula o vetor perpendicular a dois outros vetores usado para achar normais
Vetor3 produtoVetorial(const Vetor3& a, const Vetor3& b) {
    return { a.y * b.z - a.z * b.y, -(a.x * b.z - a.z * b.x), a.x * b.y - a.y * b.x };
}

// ajusta o tamanho do vetor para 1 mantendo a direcao
Vetor3 normalizacao(const Vetor3& v) {
    double m = sqrt(produtoEscalar(v, v));
    if (m < 1e-9) return {0,0,0};
    return v / m;
}

Vetor3 normalPlano(const Plano& p) {
    return produtoVetorial(p.p1 - p.p2, p.p3 - p.p2); 
}

// leitura de entrada
bool lerArquivoEntrada(istream& input, Objeto3D& objeto, Vetor3& centroProjecao, Plano& planoProjecao) {    
    if (!(input >> centroProjecao.x >> centroProjecao.y >> centroProjecao.z)) return false;
    cout << "Centro de Projeção: " << centroProjecao.x << " " << centroProjecao.y << " " << centroProjecao.z << endl;
    
    input >> planoProjecao.p1.x >> planoProjecao.p1.y >> planoProjecao.p1.z;
    input >> planoProjecao.p2.x >> planoProjecao.p2.y >> planoProjecao.p2.z;
    input >> planoProjecao.p3.x >> planoProjecao.p3.y >> planoProjecao.p3.z;
    input >> planoProjecao.r0.x >> planoProjecao.r0.y >> planoProjecao.r0.z;
    
    // define o eixo x local usando dois pontos do plano
    planoProjecao.eixoX = normalizacao(planoProjecao.p2 - planoProjecao.p1);
    
    // calculo para garantir que o eixo y seja ortogonal ao x e a normal do plano
    Vetor3 v12 = planoProjecao.p2 - planoProjecao.p1;
    Vetor3 v13 = planoProjecao.p3 - planoProjecao.p1;
    Vetor3 normal = normalizacao(produtoVetorial(v12, v13));
    planoProjecao.eixoY = normalizacao(produtoVetorial(normal, planoProjecao.eixoX));

    int nv; 
    input >> nv; 
    objeto.vertices.resize(nv);
    for(int i=0; i<nv; i++) input >> objeto.vertices[i].x >> objeto.vertices[i].y >> objeto.vertices[i].z;
    
    int ns; 
    input >> ns; 
    objeto.faces.resize(ns);
    for(int i=0; i<ns; i++) {
        int ni; 
        input >> ni; 
        objeto.faces[i].indicesVertices.resize(ni);
        for(int j=0; j<ni; j++) input >> objeto.faces[i].indicesVertices[j];
    }
    return true;
}

// funcoes de calculo da projecao
Matriz4 matrizPerspectiva(const Vetor3& C, const Plano& p) {
    Vetor3 N = normalPlano(p); 
    double d0 = produtoEscalar(p.p1, N);
    double d1 = produtoEscalar(C, N);
    double d = d0 - d1;

    // preenche a matriz de transformacao baseada na geometria projetiva
    Matriz4 M;
    M.m[0][0] = d + C.x*N.x; M.m[0][1] = C.x*N.y;     M.m[0][2] = C.x*N.z;     M.m[0][3] = -C.x*d0;
    M.m[1][0] = C.y*N.x;     M.m[1][1] = d + C.y*N.y; M.m[1][2] = C.y*N.z;     M.m[1][3] = -C.y*d0;
    M.m[2][0] = C.z*N.x;     M.m[2][1] = C.z*N.y;     M.m[2][2] = d + C.z*N.z; M.m[2][3] = -C.z*d0;
    M.m[3][0] = N.x;         M.m[3][1] = N.y;         M.m[3][2] = N.z;         M.m[3][3] = -d1;
    return M;
}

Vetor3 aplicarMatriz(const Matriz4& M, const Vetor3& p) {
    // multiplicacao de matriz 4x4 por vetor 4x1
    double x = M.m[0][0]*p.x + M.m[0][1]*p.y + M.m[0][2]*p.z + M.m[0][3];
    double y = M.m[1][0]*p.x + M.m[1][1]*p.y + M.m[1][2]*p.z + M.m[1][3];
    double z = M.m[2][0]*p.x + M.m[2][1]*p.y + M.m[2][2]*p.z + M.m[2][3];
    double w = M.m[3][0]*p.x + M.m[3][1]*p.y + M.m[3][2]*p.z + M.m[3][3];
    
    // divisao homogenea para trazer as coordenadas de volta para o espaco euclidiano
    if (abs(w) < 1e-9) return {0,0,0};
    return {x/w, y/w, z/w};
}

Vetor2 converterParaPlanoUV(const Vetor3& ponto3D, const Plano& p) {
    Vetor3 vec = ponto3D - p.p1;
    return { produtoEscalar(vec, p.eixoX), produtoEscalar(vec, p.eixoY) };  // projeta o ponto 3d nos eixos 2d locais do plano usando produto escalar
}

Vetor2 transformarViewport(Vetor2 pontoUV, const Janela& j, const Viewport& v) {
    // normaliza as coordenadas entre 0 e 1 e mapeia para o tamanho em pixels da tela
    double nx = (pontoUV.u - j.minX) / (j.maxX - j.minX);
    double ny = (pontoUV.v - j.minY) / (j.maxY - j.minY);
    // o eixo y e invertido pois telas desenham de cima para baixo
    return { v.x + nx * v.w, v.y + (1.0 - ny) * v.h };
}

// visualização do observador

struct PontoTela { int x, y; };

PontoTela projetarObservador(Vetor3 p, const Camera& cam, int w, int h) {
    double cx = cos(cam.anguloX), sx = sin(cam.anguloX);
    double cy = cos(cam.anguloY), sy = sin(cam.anguloY);
    
    // aplica rotacao orbital para visualizar a cena de fora
    double tx = p.x * cy - p.z * sy;
    double tz = p.x * sy + p.z * cy;
    p.x = tx; p.z = tz;
    double ty = p.y * cx - p.z * sx;
    tz = p.y * sx + p.z * cx;
    p.y = ty; p.z = tz;
    
    // projecao perspectiva simples apenas para visualizacao da cena 3d
    double f = 1000.0 / (p.z + cam.distancia);
    return { (int)(w/2 + p.x * f), (int)(h/2 - p.y * f) };
}

void desenharLinha(SDL_Renderer* r, PontoTela p1, PontoTela p2) {
    SDL_RenderDrawLine(r, p1.x, p1.y, p2.x, p2.y);
}

// função principal
int main(int argc, char* argv[]) {
    Objeto3D obj;
    Vetor3 CP;
    Plano plano;
    
    if (!lerArquivoEntrada(cin, obj, CP, plano)) {
        cerr << "Erro leitura!" << endl;
        return 1;
    }

    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* win = SDL_CreateWindow("Projeção Perspectiva", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1200, 600, SDL_WINDOW_SHOWN);
    SDL_Renderer* ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);

    Camera camera = {-0.02, 2.5, 35.0}; 
    Viewport viewport = {650, 50, 500, 500};

    bool rodando = true;
    SDL_Event ev;
    bool mouse = false;
    int mx, my;

    while(rodando) {
        while(SDL_PollEvent(&ev)) {
            // logica de input para movimentacao e controle
            if(ev.type == SDL_QUIT) rodando = false;
            if(ev.type == SDL_MOUSEBUTTONDOWN) { mouse = true; mx = ev.button.x; my = ev.button.y; }
            if(ev.type == SDL_MOUSEBUTTONUP) mouse = false;
            if(ev.type == SDL_MOUSEMOTION && mouse) {
                camera.anguloY += (ev.motion.x - mx) * 0.005;
                camera.anguloX += (ev.motion.y - my) * 0.005;
                mx = ev.motion.x; my = ev.motion.y;
            }
            if(ev.type == SDL_MOUSEWHEEL) camera.distancia -= ev.wheel.y * 2.0;
            if(ev.type == SDL_KEYDOWN) {
                double vel = 1.0;
                if(ev.key.keysym.sym == SDLK_LEFT) CP.x -= vel;
                if(ev.key.keysym.sym == SDLK_RIGHT) CP.x += vel;
                if(ev.key.keysym.sym == SDLK_UP) CP.y += vel;
                if(ev.key.keysym.sym == SDLK_DOWN) CP.y -= vel;
                if(ev.key.keysym.sym == SDLK_w) CP.z += vel;
                if(ev.key.keysym.sym == SDLK_s) CP.z -= vel;
            }
        }

        SDL_SetRenderDrawColor(ren, 20, 20, 20, 255);
        SDL_RenderClear(ren);

        // calculos de projecao
        Matriz4 M = matrizPerspectiva(CP, plano);
        vector<Vetor3> projs3D;
        vector<Vetor2> projsUV;
        Janela janela = {INFINITY, -INFINITY, INFINITY, -INFINITY}; // inicializa a janela com os limites minimos e maximos

        // projeta os vertices e calcula a janela bounding box
        for(auto& v : obj.vertices) {
            Vetor3 p3d = aplicarMatriz(M, v);
            projs3D.push_back(p3d);
            Vetor2 uv = converterParaPlanoUV(p3d, plano);
            projsUV.push_back(uv);
            
            // atualiza os limites da janela baseado na sombra do objeto
            if(uv.u < janela.minX) janela.minX = uv.u;
            if(uv.u > janela.maxX) janela.maxX = uv.u;
            if(uv.v < janela.minY) janela.minY = uv.v;
            if(uv.v > janela.maxY) janela.maxY = uv.v;
        }

        // adiciona margem para visualizacao
        double margem = 5.0; 
        janela.minX -= margem; janela.maxX += margem;
        janela.minY -= margem; janela.maxY += margem;

        // calcula aspect ratio para ajustar o desenho na viewport
        double ratioJanela = (janela.maxX - janela.minX) / (max(1e-5, janela.maxY - janela.minY));
        double ratioView = (double)viewport.w / viewport.h;
        if(ratioJanela > ratioView) {
            double hNovo = (janela.maxX - janela.minX) / ratioView;
            double cy = (janela.minY + janela.maxY)/2;
            janela.minY = cy - hNovo/2; janela.maxY = cy + hNovo/2;
        } else {
            double wNovo = (janela.maxY - janela.minY) * ratioView;
            double cx = (janela.minX + janela.maxX)/2;
            janela.minX = cx - wNovo/2; janela.maxX = cx + wNovo/2;
        }

        // desenho esquerdo
        int W_MUNDO = 600, H_MUNDO = 600;

        // desenha centro de projecao
        SDL_SetRenderDrawColor(ren, 255, 0, 0, 255);
        PontoTela pCP = projetarObservador(CP, camera, W_MUNDO, H_MUNDO);
        SDL_Rect rCP = {pCP.x-3, pCP.y-3, 6, 6};
        SDL_RenderFillRect(ren, &rCP);

        // desenho do plano fixo com base nos pontos hardcoded
        SDL_SetRenderDrawColor(ren, 50, 50, 150, 255);
        double tamanhoPlano = 10.0;
        
        // utilizacao de p1 como centro do desenho
        Vetor3 centroPlano = plano.p1;
        
        // calcula os 4 cantos usando os eixos ortogonais do plano
        Vetor3 c1 = centroPlano - plano.eixoX * tamanhoPlano - plano.eixoY * tamanhoPlano;
        Vetor3 c2 = centroPlano + plano.eixoX * tamanhoPlano - plano.eixoY * tamanhoPlano;
        Vetor3 c3 = centroPlano + plano.eixoX * tamanhoPlano + plano.eixoY * tamanhoPlano;
        Vetor3 c4 = centroPlano - plano.eixoX * tamanhoPlano + plano.eixoY * tamanhoPlano;

        PontoTela tc1 = projetarObservador(c1, camera, W_MUNDO, H_MUNDO);
        PontoTela tc2 = projetarObservador(c2, camera, W_MUNDO, H_MUNDO);
        PontoTela tc3 = projetarObservador(c3, camera, W_MUNDO, H_MUNDO);
        PontoTela tc4 = projetarObservador(c4, camera, W_MUNDO, H_MUNDO);

        // desenha o plano
        desenharLinha(ren, tc1, tc2); 
        desenharLinha(ren, tc2, tc3);
        desenharLinha(ren, tc3, tc4); 
        desenharLinha(ren, tc4, tc1);

        // desenha objetos
        for(const auto& face : obj.faces) {
            for(size_t i=0; i<face.indicesVertices.size(); i++) {
                int ia = face.indicesVertices[i];
                int ib = face.indicesVertices[(i+1)%face.indicesVertices.size()];
                
                // objeto 3D
                SDL_SetRenderDrawColor(ren, 0, 255, 0, 255);
                desenharLinha(ren, projetarObservador(obj.vertices[ia], camera, W_MUNDO, H_MUNDO),
                                   projetarObservador(obj.vertices[ib], camera, W_MUNDO, H_MUNDO));

                // raios projetores
                SDL_SetRenderDrawColor(ren, 255, 255, 0, 40);
                desenharLinha(ren, pCP, projetarObservador(projs3D[ia], camera, W_MUNDO, H_MUNDO));

                // projecao 2d no plano
                SDL_SetRenderDrawColor(ren, 255, 0, 255, 255);
                desenharLinha(ren, projetarObservador(projs3D[ia], camera, W_MUNDO, H_MUNDO),
                                   projetarObservador(projs3D[ib], camera, W_MUNDO, H_MUNDO));
            }
        }

        // desenho direito
        SDL_SetRenderDrawColor(ren, 255, 255, 255, 255);
        SDL_Rect rVP = {viewport.x, viewport.y, viewport.w, viewport.h};
        SDL_RenderDrawRect(ren, &rVP);

        // desenha objeto final
        SDL_SetRenderDrawColor(ren, 0, 255, 255, 255);
        for(const auto& face : obj.faces) {
            for(size_t i=0; i<face.indicesVertices.size(); i++) {
                int ia = face.indicesVertices[i];
                int ib = face.indicesVertices[(i+1)%face.indicesVertices.size()];
                Vetor2 va = transformarViewport(projsUV[ia], janela, viewport);
                Vetor2 vb = transformarViewport(projsUV[ib], janela, viewport);
                SDL_RenderDrawLine(ren, (int)va.u, (int)va.v, (int)vb.u, (int)vb.v);
            }
        }
        
        // divisoria de tela
        SDL_SetRenderDrawColor(ren, 100, 100, 100, 255);
        SDL_RenderDrawLine(ren, 600, 0, 600, 600);

        SDL_RenderPresent(ren);
        SDL_Delay(16);
    }

    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}