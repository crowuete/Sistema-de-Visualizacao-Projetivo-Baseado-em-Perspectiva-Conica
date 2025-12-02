#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <fstream>

using namespace std;

// Estruturas básicas

struct Vetor3
{
    double x, y, z;
};

struct Vetor4
{
    double x, y, z, w;
};

struct Plano
{
    Vetor3 p1,p2,p3; // Três pontos que definem o plano
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

Vetor3 calcularNormalPlano(const Plano& plano) {
    Vetor3 v12 = plano.p1 - plano.p2;
    Vetor3 v32 = plano.p3 - plano.p2;
    Vetor3 N = produtoVetorial(v12, v32);
    return normalizacao(N);
}

vector<double> multMatrizVetor(const Mat4& M, const Vetor3& P) {
    vector<double> r(4); // Vetor homogêneo que será retornado
    r[0] = M.m[0][0]*P.x + M.m[0][1]*P.y + M.m[0][2]*P.z + M.m[0][3]*1;
    r[1] = M.m[1][0]*P.x + M.m[1][1]*P.y + M.m[1][2]*P.z + M.m[1][3]*1;
    r[2] = M.m[2][0]*P.x + M.m[2][1]*P.y + M.m[2][2]*P.z + M.m[2][3]*1;
    r[3] = M.m[3][0]*P.x + M.m[3][1]*P.y + M.m[3][2]*P.z + M.m[3][3]*1;
    return r;
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

    return 0;
}