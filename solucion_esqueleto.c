// Uso:  ./tetris           corre los tests y despues el juego
//       ./tetris --test    corre solo los tests (valgrind --leak-check=full ./tetris --test)

#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// ========= CONSTANTES DEL JUEGO =========
#define SCREEN_WIDTH 500
#define SCREEN_HEIGHT 800
#define GRID_ROWS 20
#define GRID_COLS 10
#define CELL_SIZE 35
#define OFFSET_X 75
#define OFFSET_Y 50
#define TICK_RATE 30

const int TETROMINOS[7][4][4][4] = {
    { {{0,0,0,0},{1,1,1,1},{0,0,0,0},{0,0,0,0}}, {{0,0,1,0},{0,0,1,0},{0,0,1,0},{0,0,1,0}}, {{0,0,0,0},{0,0,0,0},{1,1,1,1},{0,0,0,0}}, {{0,1,0,0},{0,1,0,0},{0,1,0,0},{0,1,0,0}} },
    { {{2,0,0,0},{2,2,2,0},{0,0,0,0},{0,0,0,0}}, {{0,2,2,0},{0,2,0,0},{0,2,0,0},{0,0,0,0}}, {{0,0,0,0},{2,2,2,0},{0,0,2,0},{0,0,0,0}}, {{0,2,0,0},{0,2,0,0},{2,2,0,0},{0,0,0,0}} },
    { {{0,0,3,0},{3,3,3,0},{0,0,0,0},{0,0,0,0}}, {{0,3,0,0},{0,3,0,0},{0,3,3,0},{0,0,0,0}}, {{0,0,0,0},{3,3,3,0},{3,0,0,0},{0,0,0,0}}, {{3,3,0,0},{0,3,0,0},{0,3,0,0},{0,0,0,0}} },
    { {{0,4,4,0},{0,4,4,0},{0,0,0,0},{0,0,0,0}}, {{0,4,4,0},{0,4,4,0},{0,0,0,0},{0,0,0,0}}, {{0,4,4,0},{0,4,4,0},{0,0,0,0},{0,0,0,0}}, {{0,4,4,0},{0,4,4,0},{0,0,0,0},{0,0,0,0}} },
    { {{0,5,5,0},{5,5,0,0},{0,0,0,0},{0,0,0,0}}, {{0,5,0,0},{0,5,5,0},{0,0,5,0},{0,0,0,0}}, {{0,0,0,0},{0,5,5,0},{5,5,0,0},{0,0,0,0}}, {{5,0,0,0},{5,5,0,0},{0,5,0,0},{0,0,0,0}} },
    { {{0,6,0,0},{6,6,6,0},{0,0,0,0},{0,0,0,0}}, {{0,6,0,0},{0,6,6,0},{0,6,0,0},{0,0,0,0}}, {{0,0,0,0},{6,6,6,0},{0,6,0,0},{0,0,0,0}}, {{0,6,0,0},{6,6,0,0},{0,6,0,0},{0,0,0,0}} },
    { {{7,7,0,0},{0,7,7,0},{0,0,0,0},{0,0,0,0}}, {{0,0,7,0},{0,7,7,0},{0,7,0,0},{0,0,0,0}}, {{0,0,0,0},{7,7,0,0},{0,7,7,0},{0,0,0,0}}, {{0,7,0,0},{7,7,0,0},{7,0,0,0},{0,0,0,0}} }
};

SDL_Color colors[8] = {
    {30, 30, 30, 255},{255, 229, 236, 255},{255, 194, 209, 255},{255, 179, 198, 255},
    {255, 143, 171, 255},{251, 111, 146, 255},{255, 77, 109, 255},{201, 24, 74, 255}     
};

// ========= ESTRUCTURAS DE DATOS =========

// 1. Nodo para la fila del tablero (Lista Enlazada)
typedef struct RowNode {
    int cells[GRID_COLS]; 
    struct RowNode* next; 
} RowNode;

// 2. Nodo para la cola de próximas piezas (Queue)
typedef struct PieceQueueNode {
    int type; 
    struct PieceQueueNode* next;
} PieceQueueNode;

// 3. Nodo para el sistema de partículas 
typedef struct ParticleNode {
    float x, y;
    float vel_x, vel_y;
    int color;
    int life_timer;
    struct ParticleNode* next;
} ParticleNode;

typedef struct ActivePiece {
    int col, row;
    int type;      
    int rotation;  
    int active;
} ActivePiece;

typedef struct GameBoard {
    RowNode* first_row;             
    PieceQueueNode* queue_head;     
    PieceQueueNode* queue_tail;     
    ParticleNode* first_particle;   
    ActivePiece current_piece;
    int score;
    int tick_timer;
} GameBoard;

// ========= VARIABLES GLOBALES =========
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
GameBoard* game_board = NULL;

// ========= EJERCICIO 1: STRINGS (NO USAR string.h) =========

int len(char* src){

    /*Devuelve la cantidad de elementos de un str sin contar el 0 del final.*/
    int i = 0;
    while (src[i]!= 0){ //Se repite mientras el arreglo no llegue al 0.
        i++;
    };
    return i;
};

char* strDuplicate(char* src){

    /*Duplica un string en memoria. Debe contar la canti-
    dad de caracteres de src, solicitar mediante malloc la memoria necesaria (incluyendo el
    terminador ’\0’), y copiar todos los caracteres. Retorna el puntero al nuevo string.*/

    int longitud = len(src);  //Llamamos a len para que nos de la longitud del str.

    char* copia = (char*)malloc(sizeof(char) * (longitud + 1));  //Creamos un puntero para reotrnar la copia del str.

    //si malloc falla y copia apunta "a la nada"
    if (copia == 0){
        return NULL;
    }

    for (int i=0; i < longitud; i++){
        copia[i] = src[i];  //Copiamos 1 a 1.
    }

    copia[longitud] = 0; //Agregamos un 0 al final ya que es un str.
    return copia;
}


int strCompare(char* s1, char* s2){

    /*Compara dos strings lexicográficamente. De-
    vuelve 0 si son iguales, 1 si el primer string es menor al segundo, -1 en otro caso. Atención:
    es la convención inversa a la de strcmp. Por ejemplo, strCompare("a", "b") devuelve
    1, strCompare("b", "a") devuelve -1, y strCompare("casa", "casas") devuelve 1 (un
    prefijo es menor).*/

    //Para este ejercicio tomamos en cuenta que los signos son reperesentados como ascii.

    int result = 0; //Inicializamos la respuesta como si fueran iguales ambos str.

    int lonS1 = len(s1); //Calculamos la longitud del str1.
    int lonS2 = len(s2); //Calculamos la longitud del str2.


    int i = 0;
    //el while funciona de la siguiente manera: si un caracter es menor alfabeticamente,
    //automaticamente esa palabra es menor, result vale 1 y el while no se ejecuta.
    //mismo caso al reves. Y en caso de que dos strings sean iguales pero una de ellas tenga
    //un caracter agregado, como casa vs casaa, la menor de ellas es la mas corta.


    while (i<lonS1 && i<lonS2 && result != 1 && result != -1){
        if(s1[i] < s2[i]){
            result = 1;
        }else if (s1[i] > s2[i])
        {
            result = -1;
        } 

        i += 1;

    }

    return result;
}

char* strConcatenate(char* src1, char* src2) {
    int lonS1 = len(src1);
    int lonS2 = len(src2);

    char* strConcatenado = (char*)malloc(sizeof(char)* (lonS1 + lonS2 + 1));

    for (int i=0; i <lonS1; i++){
        strConcatenado[i] = src1[i];
    }

    for (int i = 0; i < lonS2; i++){
        strConcatenado[lonS1 + i] = src2[i];
    }

    strConcatenado[lonS1 + lonS2] = 0;

    free(src1);
    free(src2);
    return strConcatenado;
}


// ========= EJERCICIO 2: FUNCIONES A IMPLEMENTAR (EL TP) =========


void gameBoardDelete(GameBoard* board) {
    // TODO: Debe tolerar board == NULL.
    if(board == 0){
        return; 
    }
    // TODO: Recorrer y liberar todos los RowNode con free().
    RowNode* actual = (*board).first_row;
    RowNode* siguiente = (*actual).next;
    // RowNode* aux = actual; ????
    // RowNode* aux = actual; ????
    // RowNode* aux = actual; ????
    // RowNode* aux = actual; ????

    for(int i=0; i< GRID_ROWS;i++){
        free(actual); //liberamos el nodo actual.
        actual = siguiente; //reasignamos el puntero actual al siguiente nodo.
        siguiente = (*actual).next;
    }


    // TODO: Recorrer y liberar todos los ParticlePiece.
    PieceQueueNode* actualPiece = (*board).queue_head;
    if(actualPiece != 0){
        PieceQueueNode* siguientePiece = (*actualPiece).next;

        while((*actualPiece).next != 0){
            free(actualPiece); //liberamos el nodo actual.
            actualPiece = siguientePiece; //reasignamos el puntero actualpiece al siguiente nodo.
            siguientePiece = (*actualPiece).next;
        }
        free((*board).queue_tail); //liberamos el ultimo nodo de la cola.
    }

    // TODO: Recorrer y liberar todos los ParticleNode.
    ParticleNode* actualParticle = (*board).first_particle;
    if(actualParticle != 0){
        ParticleNode* siguienteParticle = (*actualParticle).next;

        while((*actualParticle).next != 0){
            free(actualParticle); //liberamos el nodo actual.
            actualParticle = siguienteParticle; //reasignamos el puntero actualparticle al siguiente nodo.
            siguienteParticle = (*actualParticle).next;
        }
        free(actualParticle); //liberamos el ultimo nodo de la lista.
    }

    // TODO: Finalmente, liberar la estructura GameBoard.
    free(board);

    //printf("Función gameBoardDelete no implementada.\n");
}


GameBoard* gameBoardNew() {

    // TODO: Reservar memoria para el GameBoard.
    GameBoard* game_board = (GameBoard*)malloc(sizeof(GameBoard));  //Reserva memoria para un objeto de tipo GameBoard
    (*game_board).score = 0;   //Inicializa el puntaje del objeto en 0.
    (*game_board).tick_timer = 0; //Inicializa el tick_timer del objeto en 0.

     // TODO: Inicializar queue_head, queue_tail y first_particle en NULL.
    (*game_board).queue_head = NULL;
    (*game_board).queue_tail = NULL;
    (*game_board).first_particle = NULL;
   
    // TODO: Usar un bucle para hacer malloc de las 20 filas (RowNode) y conectarlas.
    RowNode* cabeza = (RowNode*)malloc(sizeof(RowNode));
    if(cabeza == 0){
        //gameBoardDelete(game_board);
        return 0;
    }

    // TODO: El puntero first_row debe apuntar al primero de esos nodos.
    (*game_board).first_row = cabeza;
    RowNode* actual = cabeza;

    //Crear el enlazado de malloc.
    for (int i = 0;i < GRID_ROWS; i++){
        RowNode* nuevo = (RowNode*)malloc(sizeof(RowNode)); //Guardamos un espacio de memoria para el numero nodo.
        if(nuevo == 0){  //Si el malloc fallo
            gameBoardDelete(game_board);
            return 0; //Devolvemos NULL.
        }
        (*actual).next = nuevo; //Guardamos en nuestro nodo actual el nuevo (el siguiente).
        actual = nuevo;  //Redireccionamos el actual a nuevo para avanzar y trabajar con el siguiete.
        (*nuevo).next = 0;  //Lo dejamos en 0 por si es la ultima vuelta y tiene que apuntar a NULL. 
                            //(en caos dde que no sea la ultima se va a modifcar solo x le estructura del codigo).
    }
   
    return game_board;
}




void gameBoardAddParticle(GameBoard* board, float x, float y, int color) {

    // TODO: Crear un ParticleNode con memoria dinámica.

    
    ParticleNode* nuevo = (ParticleNode*)malloc(sizeof(ParticleNode));

    if (nuevo == NULL) { 
        return;
    }
    
    // TODO: Inicializar x, y, velocidades aleatorias y un life_timer
    (*nuevo).x = x;
    (*nuevo).y = y;    
    (*nuevo).vel_x = ((rand() % 100) / 10.0f) - 5.0f;   //copiado del juego base
    (*nuevo).vel_y = ((rand() % 50) / -10.0f) - 2.0f;  //copiado del juego base 
    (*nuevo).color = color;
    (*nuevo).life_timer = 30 + (rand() % 30);

    // TODO: Agregarlo al principio de la lista enlazada simple de partículas.
    (*nuevo).next = (*board).first_particle;
    (*board).first_particle = nuevo;
    
     
}

int gameBoardCheckAndClearLines(GameBoard* board) {
    
    RowNode* primero = (*board).first_row; //creamos un puntero al primer nodo de la lista.

    //CAMBIO
    RowNode* anterior = primero; //asignamos por defecto el puntero anterior al primer nodo.

    RowNode* linea = primero; //creamos una copia del puntero primero para recorrer la lista.
    RowNode* siguiente = (*linea).next;

    int filasElim = 0; //contador de filas eliminadas para saber cuantas filas nuevas agregar
   
    int celdasLlenas = 0; //contador de celdas llenas para saber si la fila esta llena o no.

    float x;
    float y;
    int color;

    // TODO: Recorrer la lista de RowNode.
    for (int i=0; i < GRID_ROWS ; i++){

        celdasLlenas = 0; //reiniciamos el contador de celdas llenas para cada fila.

        for (int j=0; j < GRID_COLS; j++) {
            if ((*linea).cells[j] != 0){ //si la celda actual contiene algo, incrementamos el contador.
                celdasLlenas += 1;
            }
        }
        // TODO: Si un RowNode tiene todas sus celdas > 0, desenlazarlo y hacer free().
        //anterior = linea;
        

        if (celdasLlenas == GRID_COLS){ //si las 10 celdas de la fila estan llenas, eliminamos la fila.

            filasElim += 1;

            if (i == 0){ //caso borde: primera fila, anterior se mantiene en null.
                
                (*board).first_row = siguiente;
                anterior = primero; //RECORDATORIO: me parece redundante.
                
            }else{
                (*anterior).next = siguiente; //si no es la primera fila, *anterior.next apunta al siguiente nodo
            }

            // TODO: Por cada celda destruida, invocar a gameBoardAddParticle().
            /*int color; //guardo el color de la celda actual
            for(int j=0; j<GRID_COLS; j++){
                color = (*linea).cells[j];
                gameBoardAddParticle(board, i, j, color);
            }*/

            free(linea); //liberamos el nodo al que apunta linea y reasignamos linea
            linea = siguiente;

        
            // TODO: Crear un nuevo RowNode vacío (lleno de ceros).
            RowNode* filaNueva = (RowNode*)malloc(sizeof(RowNode));
            for(int j=0; j < GRID_ROWS; j++){
                (*filaNueva).cells[j] = 0;
            }

            // TODO: Insertar el nuevo RowNode en la CABEZA del tablero (board->first_row).
            (*board).first_row = filaNueva;
            (*filaNueva).next = primero;
            primero = (*board).first_row;

    
            
            // TODO: Por cada celda destruida, invocar a gameBoardAddParticle().
            for (int j = 0; j < GRID_COLS; j++) {
                 x = OFFSET_X + i * CELL_SIZE + CELL_SIZE / 2; //CELL_SIZE es una constante definida al principio de tu solucion_esqueleto.c, en la linea 14
                 y = OFFSET_Y + j * CELL_SIZE + CELL_SIZE / 2;  //Estas formulas la copiamos del juego_base.c.
                 color = linea->cells[j];
                gameBoardAddParticle(board, x, y, color);
            }

            // Desenlazar la fila llena y liberarla
            if (anterior == NULL) {
                board->first_row = siguiente;
            } else {
                anterior->next = siguiente;
            }
            free(linea);



            
        } else {
            anterior = linea;
            linea = siguiente;
        }

        if (linea != 0){
            
            siguiente = (*linea).next;
        }
        
    }

    
    //printf("Función gameBoardCheckAndClearLines no implementada.\n");
    return 0;
}


void gameBoardEnqueuePiece(GameBoard* board, int type) {

    // TODO: Crear un PieceQueueNode con el 'type'.
    PieceQueueNode* piece = (PieceQueueNode*)malloc(sizeof(PieceQueueNode));
    (*piece).type = type;
    (*piece).next = NULL;

    if((*board).queue_head == NULL){

        (*board).queue_head = piece;
       
    }else{
        // TODO: Insertarlo al final de la cola (queue_tail) y actualizar punteros.

        //PieceQueueNode* actual = (*board).queue_head;
        //PieceQueueNode* siguiente = (*actual).next;
        PieceQueueNode* cola = (*board).queue_tail;
        (*cola).next = piece;
    
    }
    /*PREGUNTAR SOBRE SI LIBERAR COLA*/
    /*PREGUNTAR SOBRE SI LIBERAR COLA*/
    /*PREGUNTAR SOBRE SI LIBERAR COLA*/
    /*PREGUNTAR SOBRE SI LIBERAR COLA*/
    (*board).queue_tail = piece;
    

    return;
    //printf("Función gameBoardEnqueuePiece no implementada.\n");
}



int gameBoardDequeuePiece(GameBoard* board, int* out_type) {
    // TODO: Si la cola está vacía, retornar 0.
    if ((*board).queue_head == NULL){
        return 0;
    }

    // TODO: Extraer el primer nodo (queue_head).
    // TODO: Guardar el type en la variable out_type, liberar el nodo y actualizar el head.
    PieceQueueNode* primero = (*board).queue_head;
    PieceQueueNode* siguiente = (*primero).next;
    out_type = &(*primero).type;
    free(primero);

    if(siguiente == NULL){

        (*board).queue_head = NULL;
        (*board).queue_tail = NULL;

    }else{

        (*board).queue_head = siguiente;
    }

    return 1;



    
    //printf("Función gameBoardDequeuePiece no implementada.\n");
    //return 0;
}

// ========= LOGICA DE DIBUJADO Y COLISIONES (NO MODIFICAR) =========

RowNode* getRowAt(GameBoard* board, int index) {
    RowNode* curr = board->first_row;
    for(int i = 0; i < index && curr != NULL; i++) curr = curr->next;
    return curr;
}

int canMove(GameBoard* board, int target_col, int target_row, int target_rot) {
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            if (TETROMINOS[board->current_piece.type][target_rot][r][c] != 0) {
                int board_col = target_col + c;
                int board_row = target_row + r;
                if (board_col < 0 || board_col >= GRID_COLS || board_row >= GRID_ROWS) return 0;
                if (board_row >= 0) {
                    RowNode* row_node = getRowAt(board, board_row);
                    if (row_node && row_node->cells[board_col] != 0) return 0;
                }
            }
        }
    }
    return 1;
}

void lockPiece(GameBoard* board) {
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            int val = TETROMINOS[board->current_piece.type][board->current_piece.rotation][r][c];
            if (val != 0) {
                RowNode* row_node = getRowAt(board, board->current_piece.row + r);
                if (row_node) row_node->cells[board->current_piece.col + c] = val;
            }
        }
    }
    board->current_piece.active = 0;
    board->score += gameBoardCheckAndClearLines(board) * 100;
}

// Devuelve 0 si la pieza nueva no entra (GAME OVER), 1 en otro caso.
int spawnNextPiece(GameBoard* board) {
    int type;
    if (gameBoardDequeuePiece(board, &type)) {
        board->current_piece.type = type;
        board->current_piece.rotation = 0;
        board->current_piece.col = GRID_COLS / 2 - 2;
        board->current_piece.row = 0;
        board->current_piece.active = 1;
        gameBoardEnqueuePiece(board, rand() % 7);
        if (!canMove(board, board->current_piece.col, board->current_piece.row, board->current_piece.rotation)) {
            printf("GAME OVER. Puntaje final: %d\n", board->score);
            board->current_piece.active = 0;
            return 0;   // main sale del loop y libera tablero y SDL
        }
    }
    return 1;
}

// Devuelve 0 cuando termina la partida.
int gameBoardUpdate(GameBoard* board) {
    if (!board) return 0;
    if (!board->current_piece.active && !spawnNextPiece(board)) return 0;

    board->tick_timer++;
    if (board->tick_timer >= TICK_RATE) {
        board->tick_timer = 0;
        if (canMove(board, board->current_piece.col, board->current_piece.row + 1, board->current_piece.rotation)) {
            board->current_piece.row++;
        } else {
            lockPiece(board);
        }
    }

    ParticleNode* curr_part = board->first_particle;
    ParticleNode* prev_part = NULL;
    while (curr_part) {
        curr_part->x += curr_part->vel_x;
        curr_part->y += curr_part->vel_y;
        curr_part->vel_y += 0.5f;
        curr_part->life_timer--;

        if (curr_part->life_timer <= 0) {
            ParticleNode* to_delete = curr_part;
            if (prev_part == NULL) {
                board->first_particle = curr_part->next;
                curr_part = board->first_particle;
            } else {
                prev_part->next = curr_part->next;
                curr_part = prev_part->next;
            }
            free(to_delete);
        } else {
            prev_part = curr_part;
            curr_part = curr_part->next;
        }
    }
    return 1;
}

void gameBoardDraw(GameBoard* board) {
    if (!board) return;
    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_Rect board_rect = {OFFSET_X - 2, OFFSET_Y - 2, GRID_COLS * CELL_SIZE + 4, GRID_ROWS * CELL_SIZE + 4};
    SDL_RenderDrawRect(renderer, &board_rect);

    RowNode* current_row = board->first_row;
    int r = 0;
    while (current_row) {
        for (int c = 0; c < GRID_COLS; c++) {
            if (current_row->cells[c] > 0) {
                SDL_Color col = colors[current_row->cells[c]];
                SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, 255);
                SDL_Rect cell_rect = {OFFSET_X + c * CELL_SIZE, OFFSET_Y + r * CELL_SIZE, CELL_SIZE - 1, CELL_SIZE - 1};
                SDL_RenderFillRect(renderer, &cell_rect);
            }
        }
        current_row = current_row->next;
        r++;
    }

    if (board->current_piece.active) {
        for(int pr = 0; pr < 4; pr++) {
            for(int pc = 0; pc < 4; pc++) {
                int cell_val = TETROMINOS[board->current_piece.type][board->current_piece.rotation][pr][pc];
                if (cell_val != 0) {
                    SDL_Color col = colors[cell_val];
                    SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, 255);
                    SDL_Rect p_rect = {OFFSET_X + (board->current_piece.col + pc) * CELL_SIZE, 
                                       OFFSET_Y + (board->current_piece.row + pr) * CELL_SIZE, 
                                       CELL_SIZE - 1, CELL_SIZE - 1};
                    SDL_RenderFillRect(renderer, &p_rect);
                }
            }
        }
    }

    ParticleNode* curr_part = board->first_particle;
    while (curr_part) {
        SDL_Color col = colors[curr_part->color];
        int alpha = curr_part->life_timer * 8;   // se satura: SDL recibe un Uint8
        if (alpha > 255) alpha = 255;
        SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, alpha);
        SDL_Rect part_rect = {(int)curr_part->x, (int)curr_part->y, 6, 6};
        SDL_RenderFillRect(renderer, &part_rect);
        curr_part = curr_part->next;
    }
    SDL_RenderPresent(renderer);
}

int inicializar() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) return 0;
    window = SDL_CreateWindow("Tetris Base - TP", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL) return 0;
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) return 0;
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    return 1;
}

// ========= TESTS (COMPLETAR) =========

void correrTests() {
    // TODO: Implementar los casos minimos del enunciado. Por ejemplo:
    //  - strDuplicate: "", "x", un string con distintos caracteres.
    char a[] = {0};
    char b[] = {'x',0};
    char c[] = {'H','0','l','a',' ','M','u','n','d','O','!','?',0};
    char d[] = {'H', 'o', 'l', 'a', ' ', 0};
    char e[] = {'m', 'u', 'n', 'd', 'o', 0};
    char f[] = {'c', 'a', 's', 'a', 0};
    char g[] = {'c', 'a', 's', 'a', 's', 0};
    char h[] = {'a', 'b', 'c', 0};
    char k[] = {'H', 'o', 'l', 'a', ' ', 'm', 'u', 'n', 'd', 'o', 0};


    if (strCompare(strDuplicate(a), a) != 0){
        printf("error en strDuplicate");
    }

    if (strCompare(strDuplicate(b), b) != 0){
        printf("error en strDuplicate");
    }

    if (strCompare(strDuplicate(c), c) != 0){
        printf("error en strDuplicate");
    }

    //  - strCompare: "" vs "", "a" vs "a", y una desigualdad en ambos sentidos (s1/s2 y s2/s1).
    
    if (strCompare(strDuplicate(a), strDuplicate(a)) != 0){
        printf("error en strCompare");
    }

    if (strCompare(strDuplicate(b), strDuplicate(b)) != 0){
        printf("error en strCompare");
    }
    if (strCompare(strDuplicate(f), strDuplicate(g)) != 1){
        printf("error en strCompare");
    }
    if (strCompare(strDuplicate(g), strDuplicate(f)) != -1){
        printf("error en strCompare");
    }

    //  - strConcatenate: "" + "abc" y "hola " + "mundo". Los argumentos deben crearse con
    //    strDuplicate, porque strConcatenate los libera (no pasar literales).
    //int tamano = 5;



    if (strConcatenate(strDuplicate(a), strDuplicate(h)) != h){
        printf("error en strConcatenate");
    }
    if (strConcatenate(strDuplicate(d), strDuplicate(e)) != k){
        printf("error en strConcatenate");
    }

    //  - Cola: 5 push y 6 pop; el 6to debe devolver 0 y dejar queue_head y queue_tail en NULL.
    //    Verificar tambien que un push posterior funcione.

    //hago 5 push con 5 piezas de 5 tipos diferentes
    GameBoard* tablero = (GameBoard*)malloc(sizeof(GameBoard));
    for(int i = 0; i < 5; i++){
        gameBoardEnqueuePiece(tablero, i);
    }

    //hago los 6 push
    //DUDA: QUE CORNO ES OUT_TYPE Y QUE LE TENGO QUE PASAR
    //DUDA: QUE CORNO ES OUT_TYPE Y QUE LE TENGO QUE PASAR
    //DUDA: QUE CORNO ES OUT_TYPE Y QUE LE TENGO QUE PASAR
    PieceQueueNode* pieza = (*tablero).queue_head;
    int tipo = (*pieza).type;
    int* ptr_tipo = &tipo;

    int retorno;
    for(int i=0; i<6; i++){
        retorno = gameBoardDequeuePiece(tablero, ptr_tipo);
    }

    //chequeo si el ultimo pop devuelve 0 y que la cola y la cabeza sean NULL (al mismo tiempo)
    if (retorno != 0 || (*tablero).queue_tail != NULL || (*tablero).queue_head != NULL){
        return ;
    }

    //verifico si despues de un push nuevo, la cola tiene algo o no
    gameBoardEnqueuePiece(tablero, 1);
    if ((*tablero).queue_tail == NULL){
        return ;
    }

    

    //  - Tablero: llenar la fila de indice 19 (la de abajo), llamar a
    //    gameBoardCheckAndClearLines y comprobar que devuelve 1,
    //    que first_row es la fila nueva vacia y que la lista sigue teniendo 20 filas.
    RowNode* fila = (*tablero).first_row;
    //voy hasta la 19° fila (desde 0)
    for(int i=0; i<20; i++){
        fila = (*fila).next;
    }

    //lleno las celdas de numeros 2 (arbitrario, puede ser cualquiera distinto de 0).
    for(int i=0; i<10; i++){
        (*fila).cells[i] = 2;
    }

    //chequeo que la funcion de borrar filas devuelve 1
    int res;
    res = gameBoardCheckAndClearLines(tablero);
    if(res != 1) return ;

    //chequeo que la nueva primera fila esta vacia, evaluando celda por celda
    int i = 0;
    while(i<10){
        if ((*(*tablero).first_row).cells[i] != 0) {
            return ;
        } 
        i++;
    }

    //verifico que el tablero tiene 20 filass
    i = 0;
    fila = (*tablero).first_row;
    while(i<20){
        fila = (*fila).next;
        //si la fila actual apunta a 0 antes de llegar a la 19° fila (desde 0), no hay 20.
        if (fila == NULL) return ;
        i++;
    }


    //Llenar las filas 0 y 1 y llamar a gameBoardCheckAndClearLines, comprobando que
    //devuelve 2 y que la lista sigue teniendo 20 filas.
    fila = (*tablero).first_row;
    i = 0;
    int cant_fila = 0;
    
    while(i<GRID_COLS && cant_fila < 2){
        (*fila).cells[i] = 2;
        i++;
        if(i == 9){
            fila = ((*fila).next);
            i = 0;
            cant_fila++;
        }
    }

    //chequeo que la funcion de borrar filas devuelve 2
    res = gameBoardCheckAndClearLines(tablero);
    if(res != 2){
        return ;
    }

    //verifico que el tablero tiene 20 filas
    i = 0;
    fila = (*tablero).first_row;
    while(i<GRID_ROWS){
        fila = (*fila).next;
        //si la fila actual apunta a 0 antes de llegar a la 19° fila (desde 0), no hay 20.
        if (fila == NULL) return ;
        i++;
    }


    // Liberar toda la memoria pedida en los tests (strings y tableros).
    //printf("Tests no implementados.\n");
}

// Compara contra "--test" sin depender de string.h ni del Ejercicio 1.
int esFlagTest(char* arg) {
    char* flag = "--test";
    int i = 0;
    while (arg[i] != '\0' && arg[i] == flag[i]) i++;
    return arg[i] == '\0' && flag[i] == '\0';
}

int main(int argc, char* args[]) {
    srand(time(NULL));

    // Los tests corren antes de SDL. Con --test termina aca, asi valgrind solo
    // reporta la memoria del TP (SDL tiene bloques propios que no se liberan).
    correrTests();
    if (argc > 1 && esFlagTest(args[1])) return 0;

    if (!inicializar()) return 1;

    game_board = gameBoardNew();
    if (game_board == NULL) {
        printf("ADVERTENCIA: Implementa gameBoardNew() para iniciar el juego.\n");
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 0;
    }

    for(int i=0; i<3; i++) gameBoardEnqueuePiece(game_board, rand() % 7);

    SDL_Event e;
    int game_over = 0;

    while (!game_over) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) game_over = 1;
            if (e.type == SDL_KEYDOWN && game_board->current_piece.active) {
                if (e.key.keysym.sym == SDLK_LEFT && canMove(game_board, game_board->current_piece.col - 1, game_board->current_piece.row, game_board->current_piece.rotation)) game_board->current_piece.col--;
                if (e.key.keysym.sym == SDLK_RIGHT && canMove(game_board, game_board->current_piece.col + 1, game_board->current_piece.row, game_board->current_piece.rotation)) game_board->current_piece.col++;
                if (e.key.keysym.sym == SDLK_DOWN && canMove(game_board, game_board->current_piece.col, game_board->current_piece.row + 1, game_board->current_piece.rotation)) game_board->current_piece.row++;
                if (e.key.keysym.sym == SDLK_UP) {
                    int next_rot = (game_board->current_piece.rotation + 1) % 4;
                    if (canMove(game_board, game_board->current_piece.col, game_board->current_piece.row, next_rot)) game_board->current_piece.rotation = next_rot;
                }
                if (e.key.keysym.sym == SDLK_SPACE) {
                     while(canMove(game_board, game_board->current_piece.col, game_board->current_piece.row + 1, game_board->current_piece.rotation)) game_board->current_piece.row++;
                     lockPiece(game_board);
                }
            }
        }
        if (!gameBoardUpdate(game_board)) game_over = 1;
        gameBoardDraw(game_board);
        SDL_Delay(16);
    }
    gameBoardDelete(game_board);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
