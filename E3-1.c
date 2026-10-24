#include <stdio.h>
#include <stdlib.h>

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

int main(){

    //printf("%d\n", len("ABCDEF"));

    //printf("%s\n", strDuplicate("Hola Mundo"));

    //printf("%d\n", strCompare("Minecraft", "Fornite"));

    //printf("%s\n", strConcatenate("Chi", "qui tapia"));

    return 0;
};

