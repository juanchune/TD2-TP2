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
//Cambiamos los colores para que se vea mas bonito.

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
        i++;   //Suma 1 en cada iteración.
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

    int lonS1 = len(s1) + 1; //Calculamos la longitud del str1.
    int lonS2 = len(s2) + 1; //Calculamos la longitud del str2.


    int i = 0;
    //el while funciona de la siguiente manera: si un caracter es menor alfabeticamente,
    //automaticamente esa palabra es menor, result vale 1 y el while no se ejecuta.
    //mismo caso al reves. Y en caso de que dos strings sean iguales pero una de ellas tenga
    //un caracter agregado, como casa vs casaa, la menor de ellas es la mas corta.


    while ((i<lonS1 || i<lonS2) && result != 1 && result != -1){
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
    int lonS1 = len(src1);   //Llamamos a len para sacar la longitud del string.
    int lonS2 = len(src2);  //Llamamos a len para sacar la longitud del string.

    char* strConcatenado = (char*)malloc(sizeof(char)* (lonS1 + lonS2 + 1)); //Pedimos memoria

    for (int i=0; i <lonS1; i++){  //Recorremos todo el primer string y lo copiamos a nuestro nuevo espacio de memoria.
        strConcatenado[i] = src1[i];
    }

    for (int i = 0; i < lonS2; i++){  //Recorremos todo nuestor segundo string y lo copiamos a nuestro espacio de memoria.
        strConcatenado[lonS1 + i] = src2[i];  
    }

    strConcatenado[lonS1 + lonS2] = 0;  //Agregamos el 0 al finl porque es un string.

    free(src1);  //Liberamos
    free(src2);  //Liberamos
    return strConcatenado;
}


// ========= EJERCICIO 2: FUNCIONES A IMPLEMENTAR (EL TP) =========


void gameBoardDelete(GameBoard* board) {
    // TODO: Debe tolerar board == NULL.
    if(board == 0){
        return; 
    }

    // TODO: Recorrer y liberar todos los RowNode con free().
    if ((*board).first_row != 0) {
        RowNode* actual = (*board).first_row; //Asignamos actual como el primer nodo que viene del board.
        RowNode* siguiente = (*actual).next;  //Asignamos siguiente como el next del board->frist_row.

        while (siguiente != 0) {
            free(actual);                // liberamos el nodo actual.
            actual = siguiente;          // avanzamos al siguiente.
            siguiente = (*actual).next;  // actualizamos siguiente para que siga apuntando al siguiente del actual.
        }
        free(actual);                    // liberamos el ultimo nodo.
    }

    // TODO: Recorrer y liberar todos los ParticlePiece.
    PieceQueueNode* actualPiece = (*board).queue_head;  //Nos creamos un puntero que apunte a la cabeza de la cola almacenada en board.
    if(actualPiece != 0){  //Recorremos mientras nuestra pieza actual no apunte a 0, si apunta a 0 significa que ya llego al final (ya se paso de la cola).
        PieceQueueNode* siguientePiece = (*actualPiece).next;  //Asignamos siguiente como el siguiente de nuestra pieza actual.

        while((*actualPiece).next != 0){ //Recorremos mientras el siguiente de nuestra pieza actual NO sea 0. Si el siguien es 0 significa que ya llegamos al final de la cola.
            free(actualPiece); //liberamos el nodo actual.
            actualPiece = siguientePiece; //reasignamos el puntero actualpiece al siguiente nodo.
            siguientePiece = (*actualPiece).next;  //Actualizamos siguiente.
        }
        free((*board).queue_tail); //liberamos el ultimo nodo de la cola.
    }

    // TODO: Recorrer y liberar todos los ParticleNode.
    ParticleNode* actualParticle = (*board).first_particle; //Creamos un puntero a nuestra pieza actual.
    if(actualParticle != 0){ //Recorremos mientras nuestra pieza actual sea diferente a 0
        ParticleNode* siguienteParticle = (*actualParticle).next; //Creamos un puntero que señale nuestra siguiente piza de actual.

        while((*actualParticle).next != 0){ //Recorremos mientras la siguiente pieza de actual sea diferente a 0.
            free(actualParticle); //liberamos el nodo actual.
            actualParticle = siguienteParticle; //reasignamos el puntero actualparticle al siguiente nodo.
            siguienteParticle = (*actualParticle).next; //Actualizamos al siguiente de la particula actual.
        }
        free(actualParticle); //liberamos el ultimo nodo de la lista.
    }

    // TODO: Finalmente, liberar la estructura GameBoard.
    free(board); //Libera tablero.

    //printf("Función gameBoardDelete no implementada.\n");
}

void inicializarCero (RowNode* fila, int valor){   //Función auxiliar para llenar las celdas de la linea.
    for (int i = 0; i < GRID_COLS; i++){ //Recorremos todas las celdas
        (*fila).cells[i] = valor; //Asignamos el valor a cada celda
    }
    return;
} 

GameBoard* gameBoardNew() {

    // TODO: Reservar memoria para el GameBoard.
    GameBoard* game_board = (GameBoard*)malloc(sizeof(GameBoard));  //Reserva memoria para un objeto de tipo GameBoard
    if (game_board == 0){ //Por si no se genera correctamente el malloc.
        return NULL;
    }
   
    (*game_board).score = 0;   //Inicializa el puntaje del objeto en 0.
    (*game_board).tick_timer = 0; //Inicializa el tick_timer del objeto en 0.

     // TODO: Inicializar queue_head, queue_tail y first_particle en NULL.
    (*game_board).queue_head = NULL;
    (*game_board).queue_tail = NULL;
    (*game_board).first_particle = NULL;
    (*game_board).current_piece.active = 0;
   
    // TODO: Usar un bucle para hacer malloc de las 20 filas (RowNode) y conectarlas.
    RowNode* cabeza = (RowNode*)malloc(sizeof(RowNode)); //Creamos un puntero para ir apuntando a la cabeza.
    if(cabeza == 0){ //Por si falla el malloc.
        free(game_board); //Libera tablero.
        return 0;
    }

    // TODO: El puntero first_row debe apuntar al primero de esos nodos.
    (*game_board).first_row = cabeza;
    (*cabeza).next = 0;
    RowNode* actual = cabeza;
    inicializarCero(actual, 0);

    //Crear el enlazado de malloc.
    for (int i = 0;i < (GRID_ROWS-1); i++){
        RowNode* nuevo = (RowNode*)malloc(sizeof(RowNode)); //Guardamos un espacio de memoria para el numero nodo.
        if(nuevo == 0){  //Si el malloc fallo
            gameBoardDelete(game_board); //Libera tablero.
            return 0; //Devolvemos NULL.
        }

        inicializarCero(nuevo, 0);
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
    (*nuevo).life_timer = 30 + (rand() % 30); //copiado del juego base.

    // TODO: Agregarlo al principio de la lista enlazada simple de partículas.
    (*nuevo).next = (*board).first_particle; //Lo agregamos a la cola
    (*board).first_particle = nuevo; //Asignamos el nuevo como la cabeza de la cola.
    
     
}

int gameBoardCheckAndClearLines(GameBoard* board) {
    
    RowNode* linea = (*board).first_row; //creamos un puntero al primer nodo de la lista.
    RowNode* anterior = NULL; //asignamos por defecto el puntero anterior a 0.
    RowNode* siguiente = (*linea).next; //asignamos el siguiente.

    float x;
    float y;
    int color;

    int celda = 0;
    int borradas = 0;
   // TODO: Recorrer la lista de RowNode.
   for (int i = 0; (i < GRID_ROWS) && (linea != NULL); i++){ // Recorro todas las filas.
    celda = 0;                          // Seteamos la cantidad de celdas en 0 para que arrenque desde la primera.
    while ((celda < GRID_COLS) && ((*linea).cells[celda] != 0)){ // Recorremos mientras celda < columnas (10) y mientras alguna de las celdas no sean 0
        // Si solo una de las celdas llega a ser 0 significa que la linea no esta completa.
        celda += 1; // Sumamos para seguir en la siguiente celda.
    }

    // TODO: Si un RowNode tiene todas sus celdas > 0, desenlazarlo y hacer free().
    if (celda == GRID_COLS){  //SI las 10 celdas eran != 0 entonces celda = 10 por lo tanto esta lleno.
        
         // TODO: Crear un nuevo RowNode vacío (lleno de ceros).
        RowNode* nuevo = (RowNode*)malloc(sizeof(RowNode));
        if(nuevo == NULL){  //Por si falla el malloc.
            gameBoardDelete(board); //Eliminamos todo lo que creamos hasta ahora.
            return 0;
        }
        inicializarCero(nuevo, 0); // Si el malloc funciono inicializamos todo en 0.
        borradas += 1;
        // TODO: Por cada celda destruida, invocar a gameBoardAddParticle().
         for (int j = 0; j < GRID_COLS; j++){
            x = OFFSET_X + j * CELL_SIZE + CELL_SIZE/2;
            y = OFFSET_Y + i * CELL_SIZE + CELL_SIZE/2;
            color = (*linea).cells[j];
            gameBoardAddParticle (board, x,  y, color);
        }


        // TODO: Insertar el nuevo RowNode en la CABEZA del tablero (board->first_row).
        //Todos los casos los hicimos pensando en los casos bordes cuando se quiere borrar el nodo seleccionado.
        if (anterior == NULL){ //Si anterior es NULL entonces nos encontramos en la cabeza, el primero elemento.
            (*board).first_row = siguiente; //Como lo vamos a borrar nuestra cabeza ahora va a ser el siguiente.
        }else{ //Si no es el primero elemento 
            (*anterior).next = siguiente; //Lo el siguiente de anterior tiene que apuntar a siguiente, porque el nodo actual se va a borrar.
                                        // ANTERIOR --> LINEA --> SIGUIENTE    ------ ANTERIOR --> SIGUIENTE [LINEA SALE DE LA lista]
        }
        free(linea); //Liberamos la linea acutal.

        // Insertamos de nuevo en la cabeza.
        (*nuevo).next = (*board).first_row; //el siguiente del nuevo va apuntar al primero de la lista
        (*board).first_row = nuevo; //el nuevo pasa a ser el primero de la lista

         if (anterior == NULL){  //SI anterior es NULL es porque estamos en la cabeza de lista
            anterior = nuevo; //Re apuntamos el anteior al que acabamos de crear.
        }

    
         
    }else{
        //Si no tiene todas las celdas llenas no borramos y avanzamos a la siguiente linea
        anterior = linea;
    }
    linea = siguiente;
    if (linea != NULL){ //Si linea es NULL entonces llegamos al final de la lista, entonces no hacemos nada, en caso contrario hacemos:
         siguiente = (*linea).next; //Siguiente va a ser el siguiente de nuestra linea acutal.
    }
    
   }

      
    //printf("Función gameBoardCheckAndClearLines no implementada.\n");
    return borradas;
}


void gameBoardEnqueuePiece(GameBoard* board, int type) {

    // TODO: Crear un PieceQueueNode con el 'type'.
    PieceQueueNode* piece = (PieceQueueNode*)malloc(sizeof(PieceQueueNode)); //Creamos un espacio de memoria para apuntar las piezaas
    (*piece).type = type; 
    (*piece).next = NULL; //Asignamos el siguiente como NULL ya que va ser la primera pieza

    if((*board).queue_head == NULL){ //Si la cabeza es NULL

        (*board).queue_head = piece; //Agregamos a la cola nuestra primera pieza
       
    }else{
        // TODO: Insertarlo al final de la cola (queue_tail) y actualizar punteros.

        //PieceQueueNode* actual = (*board).queue_head;
        //PieceQueueNode* siguiente = (*actual).next;
        PieceQueueNode* cola = (*board).queue_tail; //Si la cola ya tenia algun elemento entonces lo insrtamos al final
        (*cola).next = piece;
    
    }
   
    (*board).queue_tail = piece; //Agregamos al final de la cola
    

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
    *out_type = (*primero).type;
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
            printf(" _._     _,-'\"\"`-._\n"
        "(,-.`._,'(       |\\`-/|\n"
        "    `-.-' \\ )-`( , o o)\n"
        "          `-    \\`_`\"'-\n"
        "GAME OVER.\n"
        "Puntaje final: %d\n", board->score);
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

    //Todos estos string son para hacer testeos
    char a[] = {0};
    char b[] = {'x',0};
    char c[] = {'H','0','l','a',' ','M','u','n','d','O','!','?',0};
    char d[] = {'H', 'o', 'l', 'a', ' ', 0};
    char e[] = {'m', 'u', 'n', 'd', 'o', 0};
    char f[] = {'c', 'a', 's', 'a', 0};
    char g[] = {'c', 'a', 's', 'a', 's', 0};
    char h[] = {'a', 'b', 'c', 0};
    char k[] = {'H', 'o', 'l', 'a', ' ', 'm', 'u', 'n', 'd', 'o', 0};

    char* s1;
    char* s2;


    s1 = strDuplicate(a); //Duplicamos ""
    if (strCompare(s1, a) != 0){
        //Comparamos con el a original
        printf("error en strDuplicate (a)\n");
    }
    free(s1);

    s1 = strDuplicate(b); //Duplicamos "x"
    if (strCompare(s1, b) != 0){
        //Coparamos con el b original
        printf("error en strDuplicate (b)\n");
    }
    free(s1);

    s1 = strDuplicate(c); //Duplicamos "H0la MundO!?"
    if (strCompare(s1, c) != 0){
        //Coparamos con el c original
        printf("error en strDuplicate (c)\n");
    }
    free(s1);

    //  - strCompare: "" vs "", "a" vs "a", y una desigualdad en ambos sentidos (s1/s2 y s2/s1).
    
    //Duplicamos el mismo string en punteros distintos
    s1 = strDuplicate(a); 
    s2 = strDuplicate(a);
    if (strCompare(s1, s2) != 0){
        
        printf("error en strCompare de dos iguales (a-a)\n");
    }
    free(s1);
    free(s2);

    //Duplicamos el mismo string en punteros distintos
    s1 = strDuplicate(b);
    s2 = strDuplicate(b);
    if (strCompare(s1, s2) != 0){
        printf("error en strCompare de dos iguales (b-b)\n");
    }
    free(s1);
    free(s2);

    //Duplicamos diferentes string en punteros distintos donde f (el primero) es mas chiquito
    s1 = strDuplicate(f);
    s2 = strDuplicate(g);
    if (strCompare(s1, s2) != 1){
        printf("error en strCompare de dos diferentes (f-g)\n");
    }
    free(s1);
    free(s2);

    //Duplicamos diferentes string en punteros distintos donde f (el segundo) es mas chiquito
    s1 = strDuplicate(g);
    s2 = strDuplicate(f);
    if (strCompare(s1, s2) != -1){
        printf("error en strCompare de dos diferentes (g-f)\n");
    }
    free(s1);
    free(s2);

    //  - strConcatenate: "" + "abc" y "hola " + "mundo". Los argumentos deben crearse con
    //    strDuplicate, porque strConcatenate los libera (no pasar literales).
    //int tamano = 5;


    //Creamos un puntero y concatenamos "" con "abc" --> tendria que devolver "abc"
    char* resultado = strConcatenate(strDuplicate(a), strDuplicate(h));
    if (strCompare(resultado, h) != 0){
        printf("error en strConcatenate (a-h), devolvio \"%s\"\n", resultado);
    }
    free(resultado);

    //Creamos un puntero y concatenamos "Hola " con "mundo" --> tendria que devolver "Hola mundo"
    char* resultado2 = strConcatenate(strDuplicate(d), strDuplicate(e));
    if (strCompare(resultado2, k) != 0){
        printf("error en strConcatenate (d-e), devolvio \"%s\"\n", resultado2);
    }
    free(resultado2);

    //  - Cola: 5 push y 6 pop; el 6to debe devolver 0 y dejar queue_head y queue_tail en NULL.
    //    Verificar tambien que un push posterior funcione.

  
    GameBoard* tablero = gameBoardNew(); //Creamos el tablero
    if (tablero == NULL){ //Por si falla
        printf("error en gameBoardNew devolvio NULL\n");
        return ;
    }

    for(int i = 0; i < 5; i++){   //hago 5 push con 5 piezas de 5 tipos diferentes
        gameBoardEnqueuePiece(tablero, i);
    }

  
    int tipo = 0;
    int retorno = 1;


    for(int i=0; i<6; i++){
        retorno = gameBoardDequeuePiece(tablero, &tipo); //saco las piezas
    }

    //chequeo si el ultimo pop devuelve 0 y que la cola y la cabeza sean NULL (al mismo tiempo)
    if (retorno != 0 || (*tablero).queue_tail != NULL || (*tablero).queue_head != NULL){
        printf("error en la cola, el 6to pop no dejo la cola vacia\n");
        gameBoardDelete(tablero);
        return ;
    }

    //verifico si despues de un push nuevo, la cola tiene algo o no
    gameBoardEnqueuePiece(tablero, 1);
    if ((*tablero).queue_tail == NULL){
        printf("error en la cola, el push posterior no funciono\n");
        gameBoardDelete(tablero);
        return ;
    }
    

    //  - Tablero: llenar la fila de indice 19 (la de abajo), llamar a
    //    gameBoardCheckAndClearLines y comprobar que devuelve 1,
    //    que first_row es la fila nueva vacia y que la lista sigue teniendo 20 filas.
    RowNode* fila = (*tablero).first_row;

    //voy hasta la 19° fila (desde 0)
    for(int i=0; i<19; i++){
        fila = (*fila).next;
    }

    //lleno las celdas de numeros 2 (arbitrario, puede ser cualquiera distinto de 0).
    for(int i=0; i<10; i++){
        (*fila).cells[i] = 2;
    }

    //chequeo que la funcion de borrar filas devuelve 1
    int res = gameBoardCheckAndClearLines(tablero);
    if(res != 1){
        printf("error en gameBoardCheckAndClearLine,  esperaba 1 y devolvio %d\n", res);
        gameBoardDelete(tablero);
        return ;
    }

    //chequeo que la nueva primera fila esta vacia, evaluando celda por celda
    for(int j = 0; j < GRID_COLS; j++){
        if ((*(*tablero).first_row).cells[j] != 0) {
            printf("error, la primera fila no quedo vacia\n");
            gameBoardDelete(tablero);
            return ;
        }
    }
    

    //verifico que el tablero tiene 20 filass
    int cantidad = 0;
    fila = (*tablero).first_row;
    while(fila != NULL){
        cantidad++;
        fila = (*fila).next;
    }
    if (cantidad != GRID_ROWS){
        printf("error, despues de borrar 1 fila, el tablero tiene %d filas\n", cantidad);
        gameBoardDelete(tablero);
        return ;
    }


    //Llenar las filas 0 y 1 y llamar a gameBoardCheckAndClearLines, comprobando que
    //devuelve 2 y que la lista sigue teniendo 20 filas.
    fila = (*tablero).first_row;
    for(int f2 = 0; f2 < 2; f2++){
        for(int j = 0; j < GRID_COLS; j++){
            (*fila).cells[j] = 2;
        }
        fila = (*fila).next;
    }

    //chequeo que la funcion de borrar filas devuelve 2
     res = gameBoardCheckAndClearLines(tablero);
    if(res != 2){
        printf("error en gameBoardCheckAndClearLines, esperaba 2 y devolvio %d\n", res);
        gameBoardDelete(tablero);
        return ;
    }

    //verifico que el tablero tiene 20 filas
     cantidad = 0;
    fila = (*tablero).first_row;
    while(fila != NULL){
        cantidad++;
        fila = (*fila).next;
    }
    if (cantidad != GRID_ROWS){
        printf("erro, despues de borrar 2 filas, el tablero tiene %d filas\n", cantidad);
        gameBoardDelete(tablero);
        return ;
    }

    gameBoardDelete(tablero);

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
