// projetor_svg.cpp
// Compilar: g++ projetor_svg.cpp -o projetor_svg -std=c++17
// Executar: ./projetor_svg
// Gera: output.svg

#include <bits/stdc++.h>
using namespace std;

struct Vetor3 { double x=0, y=0, z=0; };
struct Plano { Vetor3 p1, p2, p3; };
struct Superficie { vector<int> indicesVertices; };
struct Mat4 { double m[4][4]; };

double dotp(const Vetor3 &a, const Vetor3 &b) { return a.x*b.x + a.y*b.y + a.z*b.z; }
Vetor3 crossp(const Vetor3 &a, const Vetor3 &b) {
    return { a.y*b.z - a.z*b.y,
            a.z*b.x - a.x*b.z,
            a.x*b.y - a.y*b.x };
}
Vetor3 operator-(const Vetor3 &a, const Vetor3 &b) { return {a.x-b.x, a.y-b.y, a.z-b.z}; }
double norm(const Vetor3 &v) { return sqrt(dotp(v,v)); }
Vetor3 normalize(const Vetor3 &v) { double n=norm(v); if(n==0) return {0,0,0}; return {v.x/n, v.y/n, v.z/n}; }

vector<double> multMatVec(const Mat4 &M, const Vetor3 &P) {
    vector<double> r(4,0.0);
    r[0] = M.m[0][0]*P.x + M.m[0][1]*P.y + M.m[0][2]*P.z + M.m[0][3]*1.0;
    r[1] = M.m[1][0]*P.x + M.m[1][1]*P.y + M.m[1][2]*P.z + M.m[1][3]*1.0;
    r[2] = M.m[2][0]*P.x + M.m[2][1]*P.y + M.m[2][2]*P.z + M.m[2][3]*1.0;
    r[3] = M.m[3][0]*P.x + M.m[3][1]*P.y + M.m[3][2]*P.z + M.m[3][3]*1.0;
    return r;
}


// g++ projetor_svg.cpp -o projetor_svg -std=c++17
// ./projetor_svg

/*
5 5 5
1 1 0
-1 1 0
-1 -1 0
8
-1 -1 -1
 1 -1 -1
 1  1 -1
-1  1 -1
-1 -1  1
 1 -1  1
 1  1  1
-1  1  1
6
4 0 1 2 3
4 4 5 6 7
4 0 1 5 4
4 1 2 6 5
4 2 3 7 6
4 3 0 4 7
*/

int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    setlocale(LC_ALL, "pt_BR.UTF-8");
    cout << "Projeção Perspectiva -> Gerador de SVG\n";

    // Entrada: ponto de vista
    Vetor3 C;
    cout << "Ponto de vista C (a b c): ";
    if(!(cin >> C.x >> C.y >> C.z)) return 0;

    // Entrada: plano - três pontos
    Plano plano;
    cout << "P1 (X1 Y1 Z1): "; cin >> plano.p1.x >> plano.p1.y >> plano.p1.z;
    cout << "P2 (X2 Y2 Z2): "; cin >> plano.p2.x >> plano.p2.y >> plano.p2.z;
    cout << "P3 (X3 Y3 Z3): "; cin >> plano.p3.x >> plano.p3.y >> plano.p3.z;

    // Número de vértices
    int NV;
    cout << "Numero de vertices NV: "; cin >> NV;
    vector<Vetor3> vertices(NV);
    for(int i=0;i<NV;i++){
        cout << "Vertice " << i << " (X Y Z): ";
        cin >> vertices[i].x >> vertices[i].y >> vertices[i].z;
    }

    // Número de superfícies
    int NS;
    cout << "Numero de superficies NS: "; cin >> NS;
    vector<Superficie> faces(NS);
    for(int i=0;i<NS;i++){
        int nvps;
        cout << "Num vertices da superficie " << i << ": "; cin >> nvps;
        faces[i].indicesVertices.resize(nvps);
        cout << "Indices (0 a " << NV-1 << "): ";
        for(int j=0;j<nvps;j++) cin >> faces[i].indicesVertices[j];
    }

    // --- Cálculos conforme enunciado ---
    // Vetor normal do plano N = (P1 - P2) x (P3 - P2)
    Vetor3 v12 = plano.p2 - plano.p1;   // vetor P1→P2
    Vetor3 v13 = plano.p3 - plano.p1;   // vetor P1→P3
    Vetor3 N = crossp(v12, v13); // não normalizado aqui, pois enunciado usa componentes Nx,Ny,Nz
    double nx = N.x, ny = N.y, nz = N.z;

    // d0 = X0*nx + Y0*ny + Z0*nz  (R0 pode ser P1, por exemplo)
    // vamos usar P1 como R0 (conforme enunciado)
    double d0 = plano.p1.x * nx + plano.p1.y * ny + plano.p1.z * nz;

    // d1 = a*nx + b*ny + c*nz
    double d1 = C.x * nx + C.y * ny + C.z * nz;

    // d = d0 - d1
    double d = d0 - d1;

    // Monta matriz perspectiva 4x4 conforme enunciado:
    // | d + a*nx   a*ny    a*nz   -a*d0 |
    // | b*nx     d + b*ny  b*nz   -b*d0 |
    // | c*nx      c*ny   d + c*nz -c*d0 |
    // | nx         ny      nz     1    |
    Mat4 M;
    for(int i=0;i<4;i++) for(int j=0;j<4;j++) M.m[i][j] = 0.0;
    M.m[0][0] = d + C.x * nx; M.m[0][1] = C.x * ny;     M.m[0][2] = C.x * nz;     M.m[0][3] = -C.x * d0;
    M.m[1][0] = C.y * nx;     M.m[1][1] = d + C.y * ny; M.m[1][2] = C.y * nz;     M.m[1][3] = -C.y * d0;
    M.m[2][0] = C.z * nx;     M.m[2][1] = C.z * ny;     M.m[2][2] = d + C.z * nz; M.m[2][3] = -C.z * d0;
    M.m[3][0] = nx;           M.m[3][1] = ny;           M.m[3][2] = nz;           M.m[3][3] = 1.0;

    // Aplica M em cada vértice -> coordenadas homogêneas (X',Y',Z',W')
    struct Proj { double xc, yc, zc; /*cartesian*/ };
    vector<Proj> projVertices(NV);
    for(int i=0;i<NV;i++){
        auto r = multMatVec(M, vertices[i]);
        double Xp = r[0], Yp = r[1], Zp = r[2], Wp = r[3];
        if (Wp == 0) Wp = 1e-12; // evitar divisão por zero
        double XC = Xp / Wp;
        double YC = Yp / Wp;
        double ZC = Zp / Wp;
        projVertices[i] = {XC, YC, ZC};
    }

    // Determinar caixa de bounding da projeção no plano (XP = XC, YP = YC)
    double xmin = 1e300, xmax=-1e300, ymin=1e300, ymax=-1e300;
    for(auto &p : projVertices){
        xmin = min(xmin, p.xc);
        xmax = max(xmax, p.xc);
        ymin = min(ymin, p.yc);
        ymax = max(ymax, p.yc);
    }
    if (xmin==xmax) { xmin -= 0.5; xmax += 0.5; }
    if (ymin==ymax) { ymin -= 0.5; ymax += 0.5; }

    // Viewport SVG tamanho fixo — você pode mudar
    int VW = 1000, VH = 800; // pixels
    // Ajuste de aspecto: centralizar e preservar aspecto (janela->viewport)
    double rw = (xmax - xmin);
    double rh = (ymax - ymin);
    double aspectWin = rw / rh;
    double aspectView = double(VW) / double(VH);

    // Calcular escala e offset para centrar
    double scale;
    double tx=0, ty=0; // translação em pixels
    if (aspectView > aspectWin) {
        // viewport mais largo -> limitar por altura
        scale = VH / rh;
        double totalWidthPixels = rw * scale;
        tx = (VW - totalWidthPixels) / 2.0;
        ty = 0;
    } else {
        // viewport mais alto -> limitar por largura
        scale = VW / rw;
        double totalHeightPixels = rh * scale;
        tx = 0;
        ty = (VH - totalHeightPixels) / 2.0;
    }

    // Função para mapear XP,YP -> pixel SVG (inverte y para coordenadas de tela)
    auto mapToPixel = [&](double xp, double yp)->pair<double,double>{
        double px = (xp - xmin) * scale + tx;
        // invertendo Y para sistema de pixels (0 no topo)
        double py = VH - ((yp - ymin) * scale + ty);
        return {px, py};
    };

    // Ordenar faces por profundidade média (painter's algorithm)
    struct FaceDraw { int idx; double zavg; string svgPath; };
    vector<FaceDraw> toDraw;
    toDraw.reserve(NS);
    for(int i=0;i<NS;i++){
        double zsum = 0;
        for(int idx : faces[i].indicesVertices) zsum += projVertices[idx].zc;
        double zavg = zsum / faces[i].indicesVertices.size();

        // Criar caminho SVG da face (polígono)
        ostringstream oss;
        oss.setf(std::ios::fixed); oss<<setprecision(2);
        oss << "<polygon points=\"";
        for(size_t k=0;k<faces[i].indicesVertices.size();k++){
            int vid = faces[i].indicesVertices[k];
            auto pr = mapToPixel(projVertices[vid].xc, projVertices[vid].yc);
            oss << pr.first << "," << pr.second;
            if(k+1 < faces[i].indicesVertices.size()) oss << " ";
        }
        oss << "\" style=\"fill:rgba(150,150,250,0.6);stroke:black;stroke-width:1\" />";
        string path = oss.str();
        toDraw.push_back({i, zavg, path});
    }

    // Pintar da face mais distante (maior zavg?) -> depende da convenção: maiores z = mais distante?
    // Aqui assumimos que valores Z maiores significam mais distantes do observador no espaço projetado.
    // Ordenamos por zavg decrescente (maiores primeiro -> pintamos primeiro).
    sort(toDraw.begin(), toDraw.end(), [](const FaceDraw &a, const FaceDraw &b){
        return a.zavg > b.zavg;
    });

    // Gerar SVG
    string fname = "output.svg";
    ofstream svg(fname);
    svg << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n";
    svg << "<svg xmlns=\"http://www.w3.org/2000/svg\" ";
    svg << "width=\"" << VW << "\" height=\"" << VH << "\" ";
    svg << "viewBox=\"0 0 " << VW << " " << VH << "\">\n";
    svg << "<rect width=\"100%\" height=\"100%\" fill=\"white\" />\n";

    // Opcional: desenhar borda da janela projetada (por debugging)
    // converter os 4 cantos da janela em pixels e desenhar retângulo
    auto p1 = mapToPixel(xmin, ymin);
    auto p2 = mapToPixel(xmax, ymin);
    auto p3 = mapToPixel(xmax, ymax);
    auto p4 = mapToPixel(xmin, ymax);
    svg << "<polyline points=\""
        << p1.first << "," << p1.second << " "
        << p2.first << "," << p2.second << " "
        << p3.first << "," << p3.second << " "
        << p4.first << "," << p4.second
        << " \" style=\"fill:none;stroke:gray;stroke-dasharray:4\" />\n";

    // Desenhar faces
    for(auto &fd : toDraw) svg << fd.svgPath << "\n";

    // Desenhar vértices como pontos
    for(int i=0;i<NV;i++){
        auto pr = mapToPixel(projVertices[i].xc, projVertices[i].yc);
        svg << "<circle cx=\"" << pr.first << "\" cy=\"" << pr.second << "\" r=\"3\" fill=\"red\" />\n";
    }

    svg << "</svg>\n";
    svg.close();

    cout << "Arquivo gerado: " << fname << " (abra no navegador)\n";
    cout << "Resumo:\n";
    cout << "  d0 (R0.N) = " << d0 << "\n";
    cout << "  d1 (C.N)  = " << d1 << "\n";
    cout << "  d = " << d << "\n";
    cout << "Caixa da projecao: xmin="<<xmin<<" xmax="<<xmax<<" ymin="<<ymin<<" ymax="<<ymax<<"\n";
    cout << "Viewport: " << VW << "x" << VH << "\n";
    return 0;
}