# Sistema de Visualização Projetiva 3D

Este projeto implementa um sistema de visualização gráfica baseado em **Projeção Perspectiva Cônica**. O software realiza todo o pipeline gráfico matemático (transformações, projeção, recorte e mapeamento de viewport) para renderizar objetos 3D em uma tela 2D.

A visualização é dividida em duas partes (Split-Screen):
* **Lado Esquerdo (Mundo 3D):** Visualização externa da cena, mostrando o Centro de Projeção (ponto vermelho), o Objeto (verde), os Raios de Luz (amarelo) e a Projeção no Plano (magenta).
* **Lado Direito (Viewport):** O resultado final da renderização 2D, como seria visto na tela do dispositivo, com o objeto ajustado automaticamente à janela.

## Dependências

O projeto é escrito em **C++** e utiliza a biblioteca **SDL2** para gerenciamento de janelas e desenho de primitivas.

### Instalação do SDL2

* **Linux (Ubuntu/Debian):**
    ```bash
    sudo apt-get update
    sudo apt-get install libsdl2-dev
    ```

* **Windows (MinGW/MSYS2):**
    ```bash
    pacman -S mingw-w64-x86_64-SDL2
    ```

* **macOS (Homebrew):**
    ```bash
    brew install sdl2
    ```

## Compilação

Para compilar o projeto, utilize o `g++` linkando a biblioteca SDL2.

**Comando genérico:**
```bash
g++ main.cpp -o projecao -lSDL2
````

**No Windows (se necessário especificar bibliotecas):**

```bash
g++ main.cpp -o projecao -lmingw32 -lSDL2main -lSDL2
```

## Arquivo de Entrada

O programa lê os dados da geometria e da configuração da cena via **entrada padrão (stdin)**. O arquivo deve seguir estritamente a estrutura abaixo, contendo coordenadas em ponto flutuante (float/double) e inteiros para contagens.

### Estrutura do Arquivo

1.  **Centro de Projeção ($C$):** `Cx Cy Cz`
2.  **Ponto 1 do Plano ($P_1$):** `P1x P1y P1z`
3.  **Ponto 2 do Plano ($P_2$):** `P2x P2y P2z`
4.  **Ponto 3 do Plano ($P_3$):** `P3x P3y P3z`
5.  **Ponto de Referência ($R_0$):** `R0x R0y R0z` *(Geralmente é repetido as coordenadas de $P_1$)*
6.  **Número de Vértices ($NV$):** `int`
7.  **Lista de Vértices:** $NV$ linhas, cada uma contendo `X Y Z`
8.  **Número de Superfícies ($NS$):** `int`
9.  **Lista de Superfícies:** $NS$ linhas, onde cada linha segue o formato:
      * `NumVertices indice1 indice2 indice3 ...`
      * *Nota: O primeiro número indica quantos vértices formam a face, seguido pelos índices (base 0) dos vértices.*

### Exemplo de Arquivo (`entrada.txt`)

O exemplo abaixo define um Cubo centralizado na origem:

```text
0 0 5
-2 -2 0
2 -2 0
2 2 0
-2 -2 0
8
-1 -1 -2
1 -1 -2
1 1 -2
-1 1 -2
-1 -1 -4
1 -1 -4
1 1 -4
-1 1 -4
6
4
0 1 2 3
4
4 5 6 7
4
1 2 6 5
4
0 3 7 4
4
3 2 6 7
4
0 1 5 4
```

## Como executar

Recomenda-se redirecionar o arquivo de texto para a entrada do programa via terminal:

**Linux/Mac:**

```bash
./projecao < entrada.txt
```

**Windows:**

```cmd
projecao.exe < entrada.txt
```

### Controles

Uma vez que a janela abrir, você pode interagir com a cena em tempo real:

| Ação | Controle | Descrição |
| :--- | :--- | :--- |
| **Girar Câmera** | `Mouse Esquerdo` + Arrastar | Rotaciona a visualização do Mundo 3D (lado esquerdo). |
| **Zoom** | `Scroll do Mouse` | Aproxima ou afasta a câmera de visualização. |
| **Mover Luz (X)** | `Seta Esq` / `Seta Dir` | Move o Centro de Projeção no eixo X. |
| **Mover Luz (Y)** | `Seta Cima` / `Seta Baixo` | Move o Centro de Projeção no eixo Y. |
| **Mover Luz (Z)** | Teclas `W` / `S` | Move o Centro de Projeção no eixo Z (profundidade). |
| **Sair** | `ESC` ou Fechar Janela | Encerra o programa. |

-----

**Observação:** Ao mover o Centro de Projeção (Luz) com o teclado, observe como a projeção (magenta) muda no lado esquerdo e como o resultado final se altera no lado direito (Viewport). O plano azul se ajusta automaticamente para conter a sombra do objeto.