#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <fstream>

using namespace std;

// Estruturas básicas

struct PontoPixel {
    int u, v;
};

struct Vetor3
{
    double x, y, z;

    // Sobrecarga de operador para subtração
    Vetor3 operator-(const Vetor3& b) const {
        return {x - b.x, y - b.y, z - b.z};
    }
};

struct Vetor4
{
    double x, y, z, w;
};

struct Plano
{
    Vetor3 p1,p2,p3; // Três pontos que definem o plano
    Vetor3 eixoXLocal, eixoYLocal; // Base vetorial do plano
};


// Define uma superfície/face do objeto
// Armazena os ÍNDICES dos vértices, não as coordenadas em si (para economizar memória)
struct Superficie {
    vector<int> indicesVertices; // Ex: {0, 1, 2} forma um triângulo
};

// O Objeto 3D completo
struct Objeto3D {
    vector<Vetor3> vertices; // Lista de todos os vértices (X, Y, Z) lidos
    vector<Superficie> faces; // Lista de todas as superfícies lidas
    vector<Vetor3> verticesProjetados; // Onde guardaremos o resultado final (2D)
};

struct Matriz4 {
    double m[4][4];
};

struct Janela {
    double xmin, xmax;
    double ymin, ymax;
};

struct Viewport {
    int xmin, xmax;
    int ymin, ymax;
};

//Funções Auxiliares

double produtoEscalar(const Vetor3& a, const Vetor3& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

Vetor3 produtoVetorial(const Vetor3& a, const Vetor3& b) {
    return {
        a.y * b.z - a.z * b.y,
        -(a.x * b.z - a.z * b.x),
        a.x * b.y - a.y * b.x
    };
}

Vetor3 normalizacao(const Vetor3& v) {
    double m = sqrt(produtoEscalar(v, v));
    if (m == 0) return {0, 0, 0};
    return {v.x / m, v.y / m, v.z / m};
}

Vetor3 calculaNormalPlano(const Plano& plano) {
    Vetor3 v12 = plano.p1 - plano.p2;
    Vetor3 v32 = plano.p3 - plano.p2;
    Vetor3 N = produtoVetorial(v12, v32);
    return normalizacao(N);
}

vector<double> multMatrizVetor(const Matriz4& M, const Vetor3& P) {
    vector<double> r(4); // Vetor homogêneo que será retornado
    r[0] = M.m[0][0]*P.x + M.m[0][1]*P.y + M.m[0][2]*P.z + M.m[0][3]*1;
    r[1] = M.m[1][0]*P.x + M.m[1][1]*P.y + M.m[1][2]*P.z + M.m[1][3]*1;
    r[2] = M.m[2][0]*P.x + M.m[2][1]*P.y + M.m[2][2]*P.z + M.m[2][3]*1;
    r[3] = M.m[3][0]*P.x + M.m[3][1]*P.y + M.m[3][2]*P.z + M.m[3][3]*1;
    return r;
}

Matriz4 geraMatrizPerspectiva(const Vetor3& C, const Plano& plano) {
    Vetor3 N = calculaNormalPlano(plano);
    double d0 = produtoEscalar(plano.p1, N); 
    double d1 = produtoEscalar(C, N);
    double d = d0 - d1;

    Matriz4 M;
    M.m[0][0] = d + C.x * N.x;  M.m[0][1] = C.x * N.y;      M.m[0][2] = C.x * N.z;      M.m[0][3] = -C.x * d0;
    M.m[1][0] = C.y * N.x;      M.m[1][1] = d + C.y * N.y;  M.m[1][2] = C.y * N.z;      M.m[1][3] = -C.y * d0;
    M.m[2][0] = C.z * N.x;      M.m[2][1] = C.z * N.y;      M.m[2][2] = d + C.z * N.z;  M.m[2][3] = -C.z * d0;
    M.m[3][0] = N.x;            M.m[3][1] = N.y;            M.m[3][2] = N.z;            M.m[3][3] = -d1;

    return M;
}

Vetor3 projetaVertice(const Vetor3& original, const Matriz4& matrizPersp) {
    vector<double> h = multMatrizVetor(matrizPersp, original);

    double w = h[3];
    if (abs(w) < 1e-9) return {0, 0, 0}; 

    return {
        h[0] / w, // xc
        h[1] / w, // yc
        h[2] / w  // zc
    };
}

void ajustaJanela(Janela& j, const Viewport& v) {
    double j_largura = j.xmax - j.xmin;
    double j_altura = j.ymax - j.ymin;
    double Rj = j_largura / j_altura;

    double v_largura = (double)(v.xmax - v.xmin);
    double v_altura = (double)(v.ymax - v.ymin);
    double Rv = v_largura / v_altura;

    if (Rj > Rv) {
        double novaAltura = j_largura / Rv;
        double centroY = (j.ymax + j.ymin) / 2.0;

        j.ymin = centroY - (novaAltura / 2.0);
        j.ymax = centroY + (novaAltura / 2.0);
    } else {
        double novaLargura = j_altura * Rv;
        double centroX = (j.xmax + j.xmin) / 2.0;

        j.xmin = centroX - (novaLargura / 2.0);
        j.xmax = centroX + (novaLargura / 2.0);
    }
    
    cout << "--- Janela Ajustada para Aspect Ratio ---\n";
    cout << "X: " << j.xmin << " ate " << j.xmax << "\n";
    cout << "Y: " << j.ymin << " ate " << j.ymax << "\n\n";
}

Vetor3 converteParaSistemaDoPlano(const Vetor3& pontoProjetado, const Plano& plano) {
    Vetor3 vetorPonto = pontoProjetado - plano.p1;
    
    double u = produtoEscalar(vetorPonto, plano.eixoXLocal);
    double v = produtoEscalar(vetorPonto, plano.eixoYLocal);
    
    return {u, v, 0};
}

PontoPixel mundoParaViewport(const Vetor3& pMundo, const Janela& j, const Viewport& v) {
    PontoPixel pixel;
    double nx = (pMundo.x - j.xmin) / (j.xmax - j.xmin);
    double ny = (pMundo.y - j.ymin) / (j.ymax - j.ymin);

    pixel.u = v.xmin + (int)(nx * (v.xmax - v.xmin));
    pixel.v = v.ymin + (int)((1.0 - ny) * (v.ymax - v.ymin));

    return pixel;
}

int main() {
    cout << "Programa de Projeção 3D iniciado." << endl;
    
    // Leitura do Ponto de Vista
    Vetor3 pontoDeVista;
    cout << "Digite o ponto de vista (X Y Z): ";
    cin >> pontoDeVista.x >> pontoDeVista.y >> pontoDeVista.z;

    // Leitura do Plano de Projeção
    Plano planoProjecao;
    cout << "Digite as coordenadas (X Y Z) dos três pontos que definem o plano de projeção:" << endl;
    cout << "Ponto 1: ";
    cin >> planoProjecao.p1.x >> planoProjecao.p1.y >> planoProjecao.p1.z;
    cout << "Ponto 2: ";
    cin >> planoProjecao.p2.x >> planoProjecao.p2.y >> planoProjecao.p2.z;
    cout << "Ponto 3: ";
    cin >> planoProjecao.p3.x >> planoProjecao.p3.y >> planoProjecao.p3.z;
    // Calcular os eixos locais do plano
    Vetor3 eixoX = planoProjecao.p2 - planoProjecao.p1;
    planoProjecao.eixoXLocal = normalizacao(eixoX);
    Vetor3 N = calculaNormalPlano(planoProjecao);
    planoProjecao.eixoYLocal = normalizacao(produtoVetorial(N, planoProjecao.eixoXLocal));
    
    // Leitura do Numero de Vértices do Objeto
    int numVertices;
    cout << "Digite o número de vértices do Objeto: ";
    cin >> numVertices;

    // Leitura das Coordenadas dos Vértices do Objeto
    vector<Vetor3> vertices;
    for (int i = 0; i < numVertices; i++) {
        cout << "Vértice " << (i + 1) << " (X Y Z): ";
        Vetor3 v;
        cin >> v.x >> v.y >> v.z;
        vertices.push_back(v);
    }

    // Leitura do Número de Superfícies do Objeto
    int numSuperficies;
    cout << "Digite o número de superfícies do Objeto: ";
    cin >> numSuperficies;

    // Leitura das Superfícies do Objeto
    vector<Superficie> superficies;
    for (int i = 0; i < numSuperficies; i++) {
        int numIndices;
        cout << "Digite o número de vértices da superfície " << (i + 1) << ": ";
        cin >> numIndices;
        Superficie s;
        cout << "Digite os índices dos vértices (0 a " << (numVertices - 1) << "): ";
        for (int j = 0; j < numIndices; j++) {
            int idx;
            cin >> idx;
            s.indicesVertices.push_back(idx);
        }
        superficies.push_back(s);
    }

    // Leitura da Janela de Projeção e Viewport
    Janela janela;
    cout << "Digite os valores da Janela de Projeção (xmin xmax ymin ymax): ";
    cin >> janela.xmin >> janela.xmax >> janela.ymin >> janela.ymax;
    Viewport viewport;
    cout << "Digite os valores da Viewport (xmin xmax ymin ymax): ";
    cin >> viewport.xmin >> viewport.xmax >> viewport.ymin >> viewport.ymax;
    ajustaJanela(janela, viewport);
    // --- Fim da Leitura dos Dados ---

    // Criação do Objeto 3D
    Objeto3D objeto;
    objeto.vertices = vertices;
    objeto.faces = superficies;
    objeto.verticesProjetados.resize(numVertices); // Reserva espaço para os vértices projetados
    cout << "Objeto 3D carregado com sucesso." << endl;

    // Geração da Matriz de Projeção
    Matriz4 matrizPerspectiva = geraMatrizPerspectiva(pontoDeVista, planoProjecao);
    cout << "Matriz de Projeção gerada." << endl;

    // Projeção dos Vértices
    for (int i = 0; i < numVertices; i++) {
        objeto.verticesProjetados[i] = projetaVertice(objeto.vertices[i], matrizPerspectiva);
    }
    cout << "Vértices projetados com sucesso." << endl;

    // Conversão para o Sistema do Plano
    vector<Vetor3> verticesNoPlano(numVertices);
    for (int i = 0; i < numVertices; i++) {
        verticesNoPlano[i] = converteParaSistemaDoPlano(objeto.verticesProjetados[i], planoProjecao);
    }
    cout << "Vértices convertidos para o sistema do plano." << endl;

    // Mapeamento para a Viewport
    vector<PontoPixel> pixelsProjetados(numVertices);
    for (int i = 0; i < numVertices; i++) {
        pixelsProjetados[i] = mundoParaViewport(verticesNoPlano[i], janela, viewport);
    }
    cout << "Vértices mapeados para a Viewport." << endl;

    // Geração do arquivo SVG
    ofstream arquivoSVG("projecao.svg");
    if (!arquivoSVG.is_open()) {
        cerr << "Erro ao criar o arquivo SVG." << endl;
        return 1;
    }
    arquivoSVG << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n";
    arquivoSVG << "<svg xmlns=\"http://www.w3.org/2000/svg\" ";
    arquivoSVG << "width=\"" << (viewport.xmax - viewport.xmin) << "\" height=\"" << (viewport.ymax - viewport.ymin) << "\" ";
    arquivoSVG << "viewBox=\"0 0 " << (viewport.xmax - viewport.xmin) << " " << (viewport.ymax - viewport.ymin) << "\">\n";
    arquivoSVG << "<rect width=\"100%\" height=\"100%\" fill=\"white\" />\n";
    // Desenho das Superfícies
    for (const auto& face : objeto.faces) {
        arquivoSVG << "<polygon points=\"";
        for (size_t j = 0; j < face.indicesVertices.size(); j++) {
            int idx = face.indicesVertices[j];
            arquivoSVG << pixelsProjetados[idx].u << "," << pixelsProjetados[idx].v;
            if (j < face.indicesVertices.size() - 1) {
                arquivoSVG << " ";
            }
        }
        arquivoSVG << "\" style=\"fill:none;stroke:black;stroke-width:1\" />\n";
    }
    arquivoSVG << "</svg>\n";
    arquivoSVG.close();
    cout << "Arquivo SVG gerado com sucesso: projecao.svg" << endl;

    return 0;
}