// comando: g++ projecao_sfml.cpp -o camera -lsfml-graphics -lsfml-window -lsfml-system
// ./camera < input.txt

#include <iostream>
#include <vector>
#include <cmath>
#include <limits>

// SFML
#include <SFML/Graphics.hpp>

using namespace std;
const double ZOOM_FACTOR = 2;
// =================== Estruturas básicas ===================

struct Vetor3 {
    double x, y, z;
};

struct Vetor4 {
    double x, y, z, w;
};

struct Plano {
    Vetor3 p1, p2, p3; // pontos que definem o plano
    Vetor3 r0; // ponto sobre o plano (pode ser p1, p2 ou p3)
};

struct Superficie {
    vector<int> indicesVertices;
};

struct Objeto3D {
    vector<Vetor3> vertices;
    vector<Superficie> faces;
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

// =================== Operadores auxiliares ===================

Vetor3 operator-(const Vetor3& a, const Vetor3& b) {
    return {a.x - b.x, a.y - b.y, a.z - b.z};
}

Vetor3 operator+(const Vetor3& a, const Vetor3& b) {
    return {a.x + b.x, a.y + b.y, a.z + b.z};
}

Vetor3 operator*(const Vetor3& v, double s) {
    return {v.x * s, v.y * s, v.z * s};
}

// =================== Funções matemáticas ===================

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
    if (m == 0.0) return {0.0, 0.0, 0.0};
    return {v.x / m, v.y / m, v.z / m};
}

Vetor4 multMatrizVetor(const Matriz4& M, const Vetor4& P) {
    Vetor4 r;
    r.x = M.m[0][0]*P.x + M.m[0][1]*P.y + M.m[0][2]*P.z + M.m[0][3]*P.w;
    r.y = M.m[1][0]*P.x + M.m[1][1]*P.y + M.m[1][2]*P.z + M.m[1][3]*P.w;
    r.z = M.m[2][0]*P.x + M.m[2][1]*P.y + M.m[2][2]*P.z + M.m[2][3]*P.w;
    r.w = M.m[3][0]*P.x + M.m[3][1]*P.y + M.m[3][2]*P.z + M.m[3][3]*P.w;
    return r;
}

Vetor3 calcularNormalPlano(const Plano& plano) {
    Vetor3 v12 = plano.p1 - plano.p2;
    Vetor3 v32 = plano.p3 - plano.p2;
    Vetor3 N = produtoVetorial(v12, v32);
    return normalizacao(N);
}

// =================== Perspectiva conforme enunciado ===================
// C = (a,b,c), N = (nx,ny,nz), R0 = (x0,y0,z0)
// d0 = x0*nx + y0*ny + z0*nz
// d1 = a*nx + b*ny + c*nz
// d  = d0 - d1
// Mper = 4x4 conforme especificação

Matriz4 construirMatrizPerspectiva(const Vetor3& C, const Plano& plano) {
    Matriz4 M{};

    Vetor3 N = calcularNormalPlano(plano);
    double nx = N.x, ny = N.y, nz = N.z;

    double x0 = plano.r0.x;
    double y0 = plano.r0.y;
    double z0 = plano.r0.z;

    double a = C.x;
    double b = C.y;
    double c = C.z;

    double d0 = x0*nx + y0*ny + z0*nz;
    double d1 = a*nx + b*ny + c*nz;
    double d  = d0 - d1;

    M.m[0][0] = d + a*nx;
    M.m[0][1] = a*ny;
    M.m[0][2] = a*nz;
    M.m[0][3] = -a*d0;

    M.m[1][0] = b*nx;
    M.m[1][1] = d + b*ny;
    M.m[1][2] = b*nz;
    M.m[1][3] = -b*d0;

    M.m[2][0] = c*nx;
    M.m[2][1] = c*ny;
    M.m[2][2] = d + c*nz;
    M.m[2][3] = -c*d0;

    M.m[3][0] = nx;
    M.m[3][1] = ny;
    M.m[3][2] = nz;
    M.m[3][3] = 1.0;

    return M;
}

// Projeta um ponto 3D usando a matriz de perspectiva
// Retorna (XP, YP, Zc) já em coordenadas cartesianas no plano
Vetor3 projetarPonto(const Vetor3& P, const Matriz4& Mper) {
    Vetor4 Ph{P.x, P.y, P.z, 1.0};
    Vetor4 Pp = multMatrizVetor(Mper, Ph);

    // Evitar divisão por zero
    if (abs(Pp.w) < 1e-9) {
        return {0.0, 0.0, 0.0};
    }

    double Xc = Pp.x / Pp.w;
    double Yc = Pp.y / Pp.w;
    double Zc = Pp.z / Pp.w;

    // No plano: XP = Xc, YP = Yc
    return {Xc, Yc, Zc};
}

// =================== Janela / Viewport ===================

// Calcula a janela mínima que contém todos os pontos projetados
Janela calcularJanela(const vector<Vetor3>& proj) {
    Janela j;

    if (proj.empty()) {
        j.xmin = j.ymin = -1.0;
        j.xmax = j.ymax =  1.0;
        return j;
    }

    double xmin = numeric_limits<double>::infinity();
    double xmax = -numeric_limits<double>::infinity();
    double ymin = numeric_limits<double>::infinity();
    double ymax = -numeric_limits<double>::infinity();

    for (const auto& p : proj) {
        if (p.x < xmin) xmin = p.x;
        if (p.x > xmax) xmax = p.x;
        if (p.y < ymin) ymin = p.y;
        if (p.y > ymax) ymax = p.y;
    }

    // caso degenerado (objeto praticamente um ponto)
    if (abs(xmax - xmin) < 1e-6) {
        xmin -= 1.0;
        xmax += 1.0;
    }
    if (abs(ymax - ymin) < 1e-6) {
        ymin -= 1.0;
        ymax += 1.0;
    }

    double cx = (xmin + xmax) / 2.0;
    double cy = (ymin + ymax) / 2.0;

    double halfWidth  = (xmax - xmin) / 2.0 * ZOOM_FACTOR;
    double halfHeight = (ymax - ymin) / 2.0 * ZOOM_FACTOR;

    j.xmin = cx - halfWidth;
    j.xmax = cx + halfWidth;
    j.ymin = cy - halfHeight;
    j.ymax = cy + halfHeight;

    return j;
}

// Transforma (XP, YP) da janela para coordenada de tela (u,v) na viewport
sf::Vector2f janelaParaViewport(double xp, double yp, const Janela& j, const Viewport& v) {
    double windowWidth  = j.xmax - j.xmin;
    double windowHeight = j.ymax - j.ymin;

    double vpWidth  = v.xmax - v.xmin;
    double vpHeight = v.ymax - v.ymin;

    double Rw = windowWidth / windowHeight;
    double Rv = vpWidth / vpHeight;

    double sx, sy;
    double offsetX, offsetY;

    // Mantém proporção igual, centralizando
    if (Rw > Rv) {
        sx = vpWidth / windowWidth;
        sy = sx;
        double newHeight = windowHeight * sy;

        offsetX = v.xmin;
        offsetY = v.ymin + (vpHeight - newHeight) / 2.0;
    } else {
        sy = vpHeight / windowHeight;
        sx = sy;
        double newWidth = windowWidth * sx;

        offsetX = v.xmin + (vpWidth - newWidth) / 2.0;
        offsetY = v.ymin;
    }

    // xp cresce pra direita, yp cresce pra cima
    // em tela, y cresce pra baixo -> inverte
    double u = offsetX + (xp - j.xmin) * sx;
    double vcoord = offsetY + (j.ymax - yp) * sy;

    return sf::Vector2f(static_cast<float>(u), static_cast<float>(vcoord));
}

// =================== Leitura dos dados ===================

void lerEntrada(Objeto3D& objeto, Vetor3& C, Plano& plano) {
    cin >> C.x >> C.y >> C.z;

    cin >> plano.p1.x >> plano.p1.y >> plano.p1.z;
    cin >> plano.p2.x >> plano.p2.y >> plano.p2.z;
    cin >> plano.p3.x >> plano.p3.y >> plano.p3.z;

    cin >> plano.r0.x >> plano.r0.y >> plano.r0.z;

    int NV;
    cin >> NV;
    objeto.vertices.resize(NV);
    for (int i = 0; i < NV; ++i) {
        cin >> objeto.vertices[i].x >> objeto.vertices[i].y >> objeto.vertices[i].z;
    }

    int NS;
    cin >> NS;
    objeto.faces.resize(NS);

    for (int i = 0; i < NS; ++i) {
        int NVPS;
        cin >> NVPS;
        objeto.faces[i].indicesVertices.resize(NVPS);
        for (int j = 0; j < NVPS; ++j) {
            int idx;
            cin >> idx;
            // idx--; // descomente se o arquivo for 1-based
            objeto.faces[i].indicesVertices[j] = idx;
        }
    }
}

// =================== main ===================

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    Objeto3D objeto;
    Vetor3 cameraPos;
    Plano planoProjecao;

    lerEntrada(objeto, cameraPos, planoProjecao);

    const int WIDTH  = 800;
    const int HEIGHT = 600;

    sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "Camera Virtual - CG");
    window.setFramerateLimit(60);

    Viewport viewport{0, WIDTH, 0, HEIGHT};

    const double cameraStep = 0.1;

    // ==== CONTROLE DE MOUSE ====
    bool dragging = false;
    sf::Vector2i lastMousePos;
    const double mouseSensitivity = 0.01;
    const double zoomStep = 0.5;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Escape) {
                    window.close();
                }
            }

            // Botão do mouse pressionado: começar a arrastar
            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Right) {
                    dragging = true;
                    lastMousePos = sf::Mouse::getPosition(window);
                }
            }

            // Soltou o botão direito: parar de arrastar
            if (event.type == sf::Event::MouseButtonReleased) {
                if (event.mouseButton.button == sf::Mouse::Right) {
                    dragging = false;
                }
            }

            // Movimento do mouse enquanto arrasta
            if (event.type == sf::Event::MouseMoved) {
                if (dragging) {
                    sf::Vector2i currentPos = sf::Mouse::getPosition(window);
                    int dx = currentPos.x - lastMousePos.x;
                    int dy = currentPos.y - lastMousePos.y;

                    cameraPos.x += dx * mouseSensitivity;  // move esquerda/direita
                    cameraPos.y -= dy * mouseSensitivity;  // inverte Y pra ficar intuitivo

                    lastMousePos = currentPos;
                }
            }

            // Scroll do mouse para zoom (mexer em Z)
            if (event.type == sf::Event::MouseWheelScrolled) {
                if (event.mouseWheelScroll.wheel == sf::Mouse::VerticalWheel) {
                    // delta > 0: scroll para cima
                    cameraPos.z -= event.mouseWheelScroll.delta * zoomStep;
                }
            }
        }

        // ===== CONTROLES POR TECLADO (opcional, pode remover) =====
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
            cameraPos.x -= cameraStep;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
            cameraPos.x += cameraStep;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
            cameraPos.y += cameraStep;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
            cameraPos.y -= cameraStep;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q)) {
            cameraPos.z += cameraStep;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::E)) {
            cameraPos.z -= cameraStep;
        }

        // 1) Recalcular matriz de perspectiva com a posição atual da câmera
        Matriz4 Mper = construirMatrizPerspectiva(cameraPos, planoProjecao);

        // 2) Projetar todos os vértices
        vector<Vetor3> projVertices(objeto.vertices.size());
        for (size_t i = 0; i < objeto.vertices.size(); ++i) {
            projVertices[i] = projetarPonto(objeto.vertices[i], Mper);
        }

        // 3) Calcular janela a partir dos pontos projetados
        Janela janela = calcularJanela(projVertices);

        // 4) Desenhar
        window.clear();

        for (const auto& face : objeto.faces) {
            size_t n = face.indicesVertices.size();
            if (n < 2) continue;

            for (size_t i = 0; i < n; ++i) {
                int idx1 = face.indicesVertices[i];
                int idx2 = face.indicesVertices[(i + 1) % n]; // liga último ao primeiro

                if (idx1 < 0 || idx1 >= (int)projVertices.size()) continue;
                if (idx2 < 0 || idx2 >= (int)projVertices.size()) continue;

                Vetor3 p1 = projVertices[idx1];
                Vetor3 p2 = projVertices[idx2];

                sf::Vector2f s1 = janelaParaViewport(p1.x, p1.y, janela, viewport);
                sf::Vector2f s2 = janelaParaViewport(p2.x, p2.y, janela, viewport);

                sf::Vertex linha[] = {
                    sf::Vertex(s1),
                    sf::Vertex(s2)
                };
                window.draw(linha, 2, sf::Lines);
            }
        }

        window.display();
    }

    return 0;
}
