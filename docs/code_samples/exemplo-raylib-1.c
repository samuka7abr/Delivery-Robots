#include <raylib.h>
#define CELULAS_X 10
#define CELULAS_Y 10

typedef struct {
    Vector2 posicao;
} Robo;

int main()
{
    int largura = 500;
    int  altura = 500;

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

    for(int y = 0; y < CELULAS_Y; y++) {
        for(int x = 0; x < CELULAS_X; x++) {
            if(map[y][x] > 0) {
                paredes[y][x] = true;
            }
        }
    }
    
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_RIGHT) && robo.posicao.x + 1 < CELULAS_X && 
                paredes[(int)robo.posicao.y][(int)robo.posicao.x + 1] == false) {
            robo.posicao.x+=1;
        }
        if (IsKeyPressed(KEY_LEFT) && robo.posicao.x - 1 > -1 && 
                paredes[(int)robo.posicao.y][(int)robo.posicao.x - 1] == false) {
            robo.posicao.x-=1;
        }
        if(IsKeyPressed(KEY_UP) && robo.posicao.y - 1 > -1 && 
                paredes[(int)robo.posicao.y - 1][(int)robo.posicao.x] == false) {
            robo.posicao.y-=1;
        }
        if(IsKeyPressed(KEY_DOWN) && robo.posicao.y + 1 < CELULAS_Y && 
                paredes[(int)robo.posicao.y + 1][(int)robo.posicao.x] == false) {
            robo.posicao.y+=1;
        }

        // Desenha tudo
        BeginDrawing();

            ClearBackground(RAYWHITE);
                
            // Mapa
            for(int y = 0; y < CELULAS_Y; y++){
                for(int x = 0; x < CELULAS_X; x++){
                    if(map[y][x]==1){
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
