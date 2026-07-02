# Trabalho Prático (T) — Programação Concorrente

**Instituição:** IDP — Instituto Brasileiro de Ensino, Desenvolvimento e Pesquisa (IDP Asa Norte)
**Curso:** Ciência da Computação e Engenharia de Software

| Código | Disciplina | Professor | Período |
|---|---|---|---|
| CIC_5MA e EGS_5MA | Programação Concorrente | Jeremias Moreira Gomes | 2026/1 |

---

## 1. Objetivo

Capacitar o aluno a estudar e compreender conceitos de programação concorrente. O aluno deverá desenvolver um sistema cujo gerenciamento e execução de tarefas envolvam recursos utilizados por múltiplas *threads*.

Ao final, o aluno terá adquirido:
1. Compreensão prática dos conceitos-chave de programação concorrente;
2. Aprimoramento de escrita técnica e comunicação (a documentação exige apresentação clara e estruturada);
3. Confiança e capacidade para explorar esses conceitos na solução de problemas computacionais.

---

## 2. Enunciado

A **Indústria de Deslocamento de Pacotes (IDP)** é uma empresa especializada em logística automatizada e movimentação inteligente de cargas em centros de distribuição. Ela desenvolve sistemas para transportar grandes quantidades de pacotes entre setores de uma fábrica, usando esteiras automatizadas e agentes autônomos de coleta e entrega.

Robôs autônomos coletam pacotes em áreas de armazenamento, transportam até pontos de processamento e realizam entregas em setores de expedição. O desafio central: **coordenar múltiplos robôs que compartilham os mesmos recursos físicos**, evitando colisões, bloqueios e perda de produtividade.

Funcionamento geral:
- Pacotes são gerados continuamente em pontos de coleta na área de armazenamento.
- Robôs coletores localizam os pacotes, transportam até a entrada da esteira e os inserem.
- A esteira tem capacidade limitada e desloca os pacotes automaticamente em direção ao setor de expedição, uma posição por unidade de tempo.
- Robôs entregadores retiram os pacotes da saída da esteira e os encaminham para pontos de despacho.

O squad (grupo) deve desenvolver uma **simulação do centro de distribuição da IDP**, configurável para diferentes cenários (quantidade de robôs, capacidade da esteira, volume de pacotes).

---

## 3. Centro de Distribuição Automatizado

O ambiente é dividido em três regiões:

1. **Área de coleta** — contém estações geradoras de pacotes (pontos `P`). Essas estações produzem pacotes continuamente; os pacotes aguardam coleta até um robô coletor estar disponível.
2. **Esteira transportadora** — conecta a área de coleta à área de expedição. Composta por posições discretas, cada uma armazenando no máximo **1 pacote**. A cada unidade de tempo os pacotes avançam uma posição em direção a `out`, desde que a posição seguinte esteja livre.
3. **Área de expedição** — contém pontos de despacho (`D`). Robôs entregadores retiram pacotes da saída da esteira (`out`) e levam até um ponto de despacho disponível.

### Regras de movimentação
- Cada robô (coletor ou entregador) transporta **apenas um pacote por vez**.
- Robô coletor: desloca-se até a estação → retira o pacote → leva até a entrada da esteira (`in`). Se `in` estiver ocupada, aguarda.
- Robô entregador: desloca-se até `out` → retira o pacote → leva até um ponto `D`.
- **Regra física central: duas entidades não podem ocupar simultaneamente a mesma célula do mapa.** Isso vale para robôs, posições da esteira, e acesso às áreas de coleta/despacho — tudo deve ser coordenado para garantir consistência do estado global.

### Condição de término
A simulação termina quando uma quantidade pré-definida de pacotes tiver sido entregue aos pontos de despacho, ou quando uma condição de encerramento definida pelo grupo for atingida.

---

## 4. Implementação

- **Linguagem:** C
- Organização dos arquivos livre, mas deve incluir instruções de compilação e um `Makefile`.
- Testes e validação em ambiente Linux (Ubuntu 24.04, similar ao dos laboratórios).

### 4.1. Elementos da simulação (threads / recursos compartilhados)

A implementação deve conter, no mínimo, estes elementos funcionando como threads ou recursos compartilhados:

- **Gerador de Pacotes**
- **Interface**
- **Robôs Coletores**
- **Esteira Transportadora**
- **Robôs Entregadores**

#### 4.1.1. Gerador de Pacotes
Gera novos pacotes durante a execução, nas estações de coleta (posições `P`). A geração pode ser em intervalos regulares ou aleatórios (a critério do grupo). Cada pacote gerado deve eventualmente ser coletado, transportado pela esteira e entregue.

> As estações de coleta são recursos compartilhados: múltiplos robôs podem tentar coletar da mesma estação simultaneamente.

#### 4.2. Interface
Deve permitir acompanhamento da execução em tempo real, representando visualmente:

- Área de coleta
- Área de expedição
- Posição dos robôs coletores
- Posição dos robôs entregadores
- Estado atual da esteira
- Quantidade de pacotes aguardando coleta
- Quantidade de pacotes atualmente na esteira
- Quantidade de pacotes entregues
- Tempo de execução da simulação

> Como diversas entidades alteram o estado simultaneamente, a interface deve acessar os dados compartilhados de forma sincronizada.

**Biblioteca padrão:** `ncurses` (guia completo no Apêndice A).
**Alternativa (bônus):** `raylib` — interface gráfica em vez de terminal (ver seção 5.3 e Apêndice B).

#### 4.2.1. Robôs Coletores
Retiram pacotes das estações de coleta e transportam até a entrada da esteira (`in`). Um pacote por vez. Se `in` estiver ocupada, aguardam. Movimentação respeita a restrição de não ocupar a mesma célula que outra entidade.

#### 4.2.2. Esteira Transportadora
Buffer limitado, posições discretas, no máximo 1 pacote por posição. A cada unidade de tempo os pacotes avançam uma posição em direção a `out`, se a posição seguinte estiver livre; senão permanecem parados. Recurso compartilhado, precisa de sincronização adequada.

#### 4.2.3. Robôs Entregadores
Retiram pacotes de `out` e levam até um ponto de despacho `D`. Um pacote por vez. Após a entrega, atualizam o contador global de pacotes despachados e ficam disponíveis para nova tarefa. Mesma restrição de não sobreposição de células.

---

## 5. Metodologia / Avaliação

Dividido em: (i) Implementação da Simulação; (ii) Relatório Técnico (Documentação).

### 5.1. Implementação da Simulação — **80 pts**

A simulação deve permitir execução em diferentes cenários, via uma das duas abordagens:

- **Configuração Estática** — cenários pré-definidos na implementação. **Atinge até 90% da pontuação desta etapa**, desde que ofereça diferentes cenários previamente configurados e atenda aos demais requisitos. Recomendado: no mínimo 3 cenários distintos, selecionáveis por parâmetro de execução ou menu.
- **Configuração Dinâmica** — parâmetros definidos pelo usuário em tempo de execução.

Parâmetros configuráveis (exemplos):
- Dimensões do mapa
- Quantidade de robôs coletores
- Quantidade de robôs entregadores
- Quantidade de estações geradoras de pacotes
- Quantidade de pontos de despacho
- Comprimento da esteira transportadora
- Quantidade total de pacotes da simulação
- Quantidade de obstáculos no ambiente

> **Recomendação forte do professor:** implementar primeiro uma versão estática e simples da simulação, para garantir que o funcionamento básico esteja correto, antes de adicionar complexidade que possa comprometer a corretude (leia-se: perder pontos por besteira).

### 5.2. Documentação — **20 pts**

Deve descrever principalmente:
- Organização das threads
- Recursos compartilhados existentes
- Seções críticas identificadas
- Mecanismos de sincronização utilizados
- Decisões de projeto adotadas

Deve incluir também um **guia de utilização** (instruções de compilação e execução em Linux/Ubuntu 24.04).

**Formato:** concisa e objetiva, sem perder clareza. **Máximo de 4 páginas**, seguindo o template de Publicação de Artigos da SBC (Sociedade Brasileira de Computação).

### 5.3. Interface Gráfica com Raylib — **Bônus de até 10 pts**

Grupos que substituírem **integralmente** a interface ncurses por uma interface gráfica em raylib podem receber até 10 pontos de bonificação, condicionados ao correto funcionamento da simulação concorrente.

---

## 6. Detalhes do Trabalho

### 6.1. Restrições
- Até **3 integrantes** por grupo.
- Integrantes deveriam ser informados ao professor até **22/06/2026 (segunda-feira)**. Sem comunicação = trabalho considerado individual. Grupo formado sem comunicação, ou alteração de composição sem aviso no prazo = **penalidade de 20% na nota final**.

### 6.2. Datas importantes
- Submissão liberada a partir de **17/06/2026 (quarta-feira), 12:00h**
- Prazo final: **04/07/2026 (sábado), 23:55h**

### 6.3. Onde entregar
Ambiente Virtual (https://ambientevirtual.idp.edu.br/), disciplina Programação Concorrente, atividade "Trabalho Prático (T)".

### 6.4. O que entregar
Documentação + código-fonte produzido.

### 6.5. Como entregar
Arquivo único `.zip` contendo documentação e diretório com os códigos (evitar `.git/` e outros arquivos desnecessários).

**Nome do arquivo:** `matricula1-matricula2-matricula3-hashmd5.zip`
(matrículas separadas por hífen + hash md5 do próprio arquivo zip + extensão `.zip`)

Exemplo: `20001101-20010203-202813378-3b1b448e9a49cd6a91a1ad044e67fff2.zip`

Processo:
```bash
zip -r aaa.zip trabalho-01-apc-jeremias.c
md5sum aaa.zip
# copiar o hash retornado
mv aaa.zip <matriculas>-<hash>.zip
```

### 6.6. Quem entrega
Se o Ambiente Virtual suportar trabalho em grupo, um membro submete e o arquivo fica disponível pra todos. Caso contrário, **apenas um aluno deve gerar a versão final do zip**, para o hash não divergir entre entregas.

### 6.7. Observações importantes
- Não deixar para fazer de última hora.
- Não serão aceitos trabalhos com atraso.
- Cópias recebem zero (plágio é crime).

### 6.8. Dicas de implementação

Desenvolvimento incremental recomendado — implementar e validar separadamente, nesta ordem:

1. Representação do mapa e movimentação dos robôs
2. Geração de pacotes nas estações de coleta
3. Funcionamento da esteira transportadora
4. Entrega de pacotes nos pontos de despacho
5. Interface de acompanhamento da simulação
6. Introdução das threads e mecanismos de sincronização

Recursos compartilhados a mapear com cuidado:
- Células do mapa
- Estações geradoras de pacotes
- Posições da esteira transportadora
- Pontos de despacho
- Contadores e estatísticas da simulação

Iniciar testes com cenários simples e estáticos antes de evoluir para cenários complexos/dinâmicos. Grupos que optarem por raylib devem validar a simulação funcionando antes de investir na interface gráfica.

---

## 7. Bibliografia

1. TANENBAUM, Andrew S. *Sistemas Operacionais Modernos*. 3ª edição, 2010.
2. ncurses: A Free Software emulation of curses in System V Release 6.6, and more. https://invisible-island.net/ncurses/announce.html
3. Raylib: A Simple and Easy-to-use Library to Enjoy Game Programming. https://www.raylib.com/

---

## Apêndice A — ncurses: Guia Simplificado

Interface gráfica em modo texto no terminal.

**Compilação:**
```bash
gcc -o programa codigo.c -lncurses
```

### Funções e constantes principais

| Função/Constante | Descrição |
|---|---|
| `LINES` | número de linhas da tela |
| `COLS` | número de colunas da tela |
| `initscr()` | inicializa o terminal em modo curses |
| `endwin()` | finaliza o modo curses |
| `clear()` | limpa a tela |
| `echo()` / `noecho()` | habilita/desabilita exibição do que é digitado |
| `keypad(stdscr, TRUE/FALSE)` | habilita/desabilita teclas especiais |
| `raw()` / `noraw()` | desativa/ativa buffer de quebra de linha |
| `start_color()` | inicializa uso de cores |
| `init_pair(numero, cor_texto, cor_fundo)` | inicializa um par de cores |
| `attron(atributos)` / `attroff(atributos)` | liga/desliga atributo |
| `printw("texto")` | exibe texto (no lugar de `printf`) |
| `move(linha, coluna)` | move o cursor |
| `mvprintw(linha, coluna, "texto")` | move + printw |
| `getch()` | lê um caractere digitado |
| `getstr(string)` | lê uma string digitada |
| `addch(caractere)` | exibe um caractere na posição atual |

**Cores disponíveis:** `COLOR_BLACK`, `COLOR_RED`, `COLOR_GREEN`, `COLOR_YELLOW`, `COLOR_BLUE`, `COLOR_MAGENTA`, `COLOR_CYAN`, `COLOR_WHITE`

**Atributos:** `A_NORMAL`, `A_STANDOUT`, `A_BOLD`, `A_DIM`, `A_UNDERLINE`, `A_REVERSE`, `A_BLINK`, `A_ALTCHARSET`

**Material de apoio:** https://terminalroot.com.br/ncurses/

### Exemplo simples

```c
// gcc -o programa exemplo-ncurses.c -lncurses
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ncurses.h>

void imprime_conteudo(int tipo)
{
    attron(COLOR_PAIR(7));
    move(3, 0);
    printw("Par de cores %02d", tipo);
    move(4, 0);
    attron(COLOR_PAIR(tipo));
    for (int i = '0'; i <= 'z'; i++) {
        printw("%c", i);
    }
    attron(A_REVERSE);
    move(5, 0);
    attron(A_ALTCHARSET);
    for (int i = '0'; i <= 'z'; i++) {
        printw("%c", i);
    }
    attroff(A_ALTCHARSET);
    attroff(A_REVERSE);
    refresh();
}

int main()
{
    int tecla;
    initscr();       // Inicializa a tela (posição atual é (0,0))
    start_color();
    raw();            // Não precisa esperar uma quebra de linha
    noecho();         // O que for digitado não aparece na tela
    keypad(stdscr, TRUE); // Teclas especiais como F1, F2, etc.

    // Cria as cores que serão utilizadas (mas ainda não usa)
    init_pair(7, COLOR_BLACK, COLOR_WHITE); // padrão
    init_pair(1, COLOR_BLUE, COLOR_WHITE);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(5, COLOR_RED, COLOR_BLACK);

    printw("Iniciando...");
    refresh(); // Realiza todos os comandos pendentes atualizando na tela
    sleep(1);

    attron(COLOR_PAIR(5));
    mvprintw(LINES - 1, 0, "(%d - %d) - Pressione 'q' para sair...", LINES, COLS);
    attroff(COLOR_PAIR(5));

    move(0, 0);
    printw(" - Digite 1, 2, q ou F4: ");
    do {
        tecla = getch();
        switch (tecla) {
            case '1':
                imprime_conteudo(1);
                break;
            case '2':
                imprime_conteudo(2);
                break;
            case KEY_F(4):
                attron(COLOR_PAIR(7));
                move(8, 0);
                printw("Tecla F4 foi pressionada.. saindo");
                refresh();
                sleep(1);
            case 'q':
            default:
                break;
        }
    } while (tecla != 'q' && tecla != KEY_F(4));

    // Desabilitando tudo que foi habilitado antes de sair, para não deixar o
    // terminal em estado diferente do anterior à execução
    keypad(stdscr, FALSE);
    noraw();
    echo();

    endwin();

    printf("Termino da execucao\n");

    return 0;
}
```

---

## Apêndice B — Raylib: Guia Simplificado

Biblioteca multiplataforma para aplicações gráficas/jogos (janela gráfica em vez de terminal). **Uso opcional**, alternativa à interface ncurses (ver bônus, seção 5.3).

### Instalação (Ubuntu 24.04 / WSL)

```bash
sudo apt install build-essential git
sudo apt install libasound2-dev libx11-dev libxrandr-dev libxi-dev \
libgl1-mesa-dev libglu1-mesa-dev libxcursor-dev libxinerama-dev \
libwayland-dev libxkbcommon-dev

git clone --depth 1 https://github.com/raysan5/raylib.git raylib
cd raylib/src/
make PLATFORM=PLATFORM_DESKTOP
```

### Compilação

```bash
gcc -o prog codigo.c -lraylib -lm -lX11
```

### Estrutura básica de um programa raylib
1. Inicialização da janela
2. Configuração da taxa de atualização
3. Laço principal da aplicação
4. Atualização da lógica do programa
5. Desenho dos elementos da interface
6. Encerramento da janela

### Funções principais

| Função | Descrição |
|---|---|
| `InitWindow(largura, altura, titulo)` | cria a janela |
| `SetTargetFPS(valor)` | define a taxa de atualização |
| `WindowShouldClose()` | verifica se o usuário pediu pra encerrar |
| `BeginDrawing()` / `EndDrawing()` | inicia/finaliza etapa de desenho |
| `ClearBackground(cor)` | limpa a tela com uma cor |
| `DrawRectangle(x, y, largura, altura, cor)` | desenha retângulo preenchido |
| `DrawText(texto, x, y, tamanho, cor)` | desenha texto |
| `CloseWindow()` | encerra a aplicação |
| `IsKeyPressed(KEY_RIGHT)` | verifica tecla pressionada (exemplo: seta direita) |

**Cores pré-definidas:** `BLACK`, `WHITE`, `RED`, `GREEN`, `BLUE`, `YELLOW`, `ORANGE`, `PURPLE`, `GRAY`, `RAYWHITE`

### Representação de mapas
Estratégia comum: matriz onde cada posição é uma célula.
- `0` = célula livre
- `1` = parede

O robô é uma struct com posição no mapa; o desenho converte coordenadas da matriz para coordenadas de janela.

**Material de apoio:** https://www.raylib.com/cheatsheet/cheatsheet.html

### Exemplo simples

```c
#include <raylib.h>
#define CELULAS_X 10
#define CELULAS_Y 10

typedef struct {
    Vector2 posicao;
} Robo;

int main()
{
    int largura = 500;
    int altura = 500;

    InitWindow(largura, altura, "Exemplo de mapa");

    Robo robo = {0};
    robo.posicao = (Vector2){1, 1};

    float largura_da_celula = (float)largura / (float)CELULAS_X;
    float altura_da_celula = (float)altura / (float)CELULAS_Y;

    bool paredes[CELULAS_Y][CELULAS_X] = {false};

    // Cuidado ao mexer nas dimensões do mapa, porque este foi criado (por
    // preguiça já que é só um exemplo) pensando em algo (10, 10)
    int map[CELULAS_Y][CELULAS_X] = {
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 1, 1, 1, 1, 1, 1, 0, 1},
        {1, 0, 0, 0, 1, 1, 0, 0, 0, 1},
        {1, 0, 0, 0, 1, 1, 0, 0, 0, 1},
        {1, 0, 0, 0, 1, 1, 0, 0, 0, 1},
        {1, 0, 0, 0, 1, 1, 0, 0, 0, 1},
        {1, 0, 1, 1, 1, 1, 1, 1, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
    };

    for (int y = 0; y < CELULAS_Y; y++) {
        for (int x = 0; x < CELULAS_X; x++) {
            if (map[y][x] > 0) {
                paredes[y][x] = true;
            }
        }
    }

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_RIGHT) && robo.posicao.x + 1 < CELULAS_X &&
                paredes[(int)robo.posicao.y][(int)robo.posicao.x + 1] == false) {
            robo.posicao.x += 1;
        }
        if (IsKeyPressed(KEY_LEFT) && robo.posicao.x - 1 > -1 &&
                paredes[(int)robo.posicao.y][(int)robo.posicao.x - 1] == false) {
            robo.posicao.x -= 1;
        }
        if (IsKeyPressed(KEY_UP) && robo.posicao.y - 1 > -1 &&
                paredes[(int)robo.posicao.y - 1][(int)robo.posicao.x] == false) {
            robo.posicao.y -= 1;
        }
        if (IsKeyPressed(KEY_DOWN) && robo.posicao.y + 1 < CELULAS_Y &&
                paredes[(int)robo.posicao.y + 1][(int)robo.posicao.x] == false) {
            robo.posicao.y += 1;
        }

        // Desenha tudo
        BeginDrawing();

        ClearBackground(RAYWHITE);

        // Mapa
        for (int y = 0; y < CELULAS_Y; y++) {
            for (int x = 0; x < CELULAS_X; x++) {
                if (map[y][x] == 1) {
                    DrawRectangle(
                        x * largura_da_celula,
                        y * altura_da_celula,
                        largura_da_celula,
                        altura_da_celula,
                        BLACK
                    );
                }
            }
        }
        // Robô
        DrawRectangle(
            robo.posicao.x * largura_da_celula,
            robo.posicao.y * altura_da_celula,
            largura_da_celula,
            altura_da_celula,
            RED
        );

        DrawText("Use as setas do teclado", 0, 0, 20, WHITE);
        EndDrawing();
    }

    CloseWindow();

    return 0;
}
```
