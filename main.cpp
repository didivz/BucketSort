/* 
 * File:   main.cpp
 * Authors: Diogenes Vanzella / Luiz Felipe
 *
 * Created on 12 de Abril de 2015, 16:46
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <pthread.h>

//Pra usar a função rand(), temos que adicionar a biblioteca time.h 
//e para saber o valor de RAND_MAX, temos que usar a função stdlib.h.

// constantes para ser usado na execução do programa
#define tamvet 50  // quantidade de elementos a ser gerado
#define nbuckets 5  // quantidade de buckets para separar o vetor original
#define nthreads 4  // quantidade de Threads para executar o programa

// Variaveis globias usadas na execução do programa
int MINIMO = -1;  // contem o menor valor da faixa do bucket
int alternaBucket = 0;  // usado pela função "thread_bucket", para alternar entre os buckets
int compBucket = 0;  // usado pela função "thread_bucket", para verificar o numero de buckets
int valorMaior = 0;  // 
int valorMenor = 0;
double faixaNumeroBuckets = (tamvet / nbuckets);
pthread_mutex_t mutex;
bucket *vetorBucket;

//cria uma estrutura contendo 2 elementos do tipo inteiro e um vetor de int
//a estrutura representará um novo tipo de dados, chamado "bucket"
//logo, será possível criar variáveis do tipo "bucket"

typedef struct {
    int id;
    int tam;
    int elementosVetor[tamvet];
} bucket;

//método que cria um vetor desordenado de ordem tam;

void bubble_sort(int *v, int tam) {
    int i, j, temp, trocou;
    for (j = 0; j < tam - 1; j++) {
        trocou = 0;
        for (i = 0; i < tam - 1; i++) {
            if (v[i + 1] < v[i]) {
                temp = v[i];
                v[i] = v[i + 1];
                v[i + 1] = temp;
                trocou = 1;
            }
        }
        if (!trocou) break;
    }
}

// recebe um ponteiro que aponta para o primeiro endereço de memória do vetor
// asssim podemos manipula-lo como se fosse um vetor normal

void cria_bucktes(int *pVetorOriginal) {

    vetorBucket = (bucket *) malloc(sizeof (bucket) * nbuckets);
    int j = 0;
    if ((tamvet % nbuckets) != 0) {

        int quaBucketsPrimarios = (tamvet % nbuckets);
        int faixaBucketsPrimarios = (tamvet / nbuckets) + 1; // equivalente faixaNumeroBuckets

        int quaBucketsSecundarios = nbuckets - (tamvet % nbuckets);
        int faixaBucketsSecundarios = (tamvet / nbuckets);

        for (int i = 0; i < quaBucketsPrimarios; ++i) {
            vetorBucket[i].id = i;
            // usar lock e unlock
            // Como MINIMO é -1 o primeiro vetor recebe valor minimo 0
            valorMenor = MINIMO + 1;
            // MINIMO recebe agora ele mesmo mais a faixa de números. Ex.: faixa = 10 então agora MINIMO = 9
            MINIMO = MINIMO + faixaBucketsPrimarios;
            // Como MINIMO = 9 valor maior do primeiro vetor recebe 9
            valorMaior = MINIMO;
            //printf("\nBucket %d\n", vetorBucket[i].id);
            // Para atribuir os valores a cada posição do bucket
            for (int k = 0; k < tamvet; ++k) {
                if ((pVetorOriginal[k] >= valorMenor) && (pVetorOriginal[k] <= valorMaior)) {
                    vetorBucket[i].elementosVetor[j] = pVetorOriginal[k];
                    //printf("%d ", vetorBucket[i].elementosVetor[j]);
                    j++;
                }
            }
            vetorBucket[i].tam = j;
            j = 0;
        }
        int z = quaBucketsPrimarios;
        for (int i = 0; i < quaBucketsSecundarios; ++i) {
            vetorBucket[quaBucketsPrimarios].id = quaBucketsPrimarios;
            // usar lock e unlock
            // Como MINIMO é -1 o primeiro vetor recebe valor minimo 0
            valorMenor = MINIMO + 1;
            // MINIMO recebe agora ele mesmo mais a faixa de números. Ex.: faixa = 10 então agora MINIMO = 9
            MINIMO = MINIMO + faixaBucketsSecundarios;
            // Como MINIMO = 9 valor maior do primeiro vetor recebe 9
            valorMaior = MINIMO;
            //printf("\nBucket %d\n", vetorBucket[i].id);
            // Para atribuir os valores a cada posição do bucket
            for (int k = 0; k < tamvet; ++k) {
                if ((pVetorOriginal[k] >= valorMenor) && (pVetorOriginal[k] <= valorMaior)) {
                    vetorBucket[quaBucketsPrimarios].elementosVetor[j] = pVetorOriginal[k];
                    //printf("%d ", vetorBucket[i].elementosVetor[j]);
                    j++;
                }
            }
            // Zera j para poder começar a atribuir na posição inicial dos buckets seguintes.
            vetorBucket[quaBucketsPrimarios].tam = j;
            quaBucketsPrimarios++;
            j = 0;
        }

    } else {
        // Para alternar de buckets
        for (int i = 0; i < nbuckets; ++i) {
            vetorBucket[i].id = i;
            // usar lock e unlock
            // Como MINIMO é -1 o primeiro vetor recebe valor minimo 0
            valorMenor = MINIMO + 1;
            // MINIMO recebe agora ele mesmo mais a faixa de números. Ex.: faixa = 10 então agora MINIMO = 9
            MINIMO = MINIMO + faixaNumeroBuckets;
            // Como MINIMO = 9 valor maior do primeiro vetor recebe 9
            valorMaior = MINIMO;
            //printf("\nBucket %d\n", vetorBucket[i].id);
            // Para atribuir os valores a cada posição do bucket
            for (int k = 0; k < tamvet; ++k) {
                if ((pVetorOriginal[k] >= valorMenor) && (pVetorOriginal[k] <= valorMaior)) {
                    vetorBucket[i].elementosVetor[j] = pVetorOriginal[k];
                    //printf("%d ", vetorBucket[i].elementosVetor[j]);
                    j++;
                }
            }
            vetorBucket[i].tam = j;
            j = 0;
            //printf("\n");	
        }
    }
    //return vetorBucket;
}

void *thread_bucket() {
    while (compBucket < nbuckets) {
        pthread_mutex_lock(&mutex);
        if (compBucket < nbuckets) {
            bubble_sort(vetorBucket[alternaBucket].elementosVetor, vetorBucket[alternaBucket].tam);
            pthread_t db = pthread_self();
            printf("Thread %d processando bucket %d \n", ((unsigned int) db - 1), vetorBucket[alternaBucket].id);
            alternaBucket++;
            compBucket++;
            pthread_mutex_unlock(&mutex);
        } else {
            pthread_mutex_unlock(&mutex);
            pthread_exit(NULL);
        }
    }
}

int main(int argc, char **argv) {
    int i;

    //recupera o numero de threads passado na linha de comando: ex. ./programa 5
    //os parâmetros passados na linha de comando são strings, logo é necessário converter
    //para o tipo de dados desejado. Nesse caso o valor é inteiro, logo utiliza-se atoi()
    int n_threads = atoi(argv[1]);

    //cria o vetor do tipo pthread_t para armazenar o retorno de pthread_create() de cada thread
    //o número de elementos do vetor será igual ao número de threads passado na linha de comando (n_threads)
    pthread_t *threads = (pthread_t *) malloc(sizeof (pthread_t) * n_threads);

    //
    bucket *id = (bucket *) malloc(sizeof (bucket) * nbuckets);

    //aloca dinamicamente um vetor que armazenará os parametros passados para cada thread
    //o vetor terá tamanho n_threads
    //cada parâmetro será do tipo "parametros_thread_t", ou seja, conterá 2 elementos inteiros que serão passados a cada thread
    parametros_thread_t *params = (parametros_thread_t *) malloc(sizeof (parametros_thread_t) * n_threads);

    //preenche os parâmetros a serem passados a cada thread e cria as threads
    for (i = 0; i < n_threads; i++) {
        params[i].parametro1 = i;
        params[i].parametro2 = i + 1;
        pthread_create(&threads[i], NULL, thread, (void *) &params[i]);
    }

    //aguarda a finalização de todas as threads
    for (i = 0; i < n_threads; i++)
        pthread_join(threads[i], NULL);

    //libera o espaço alocado para os vetores de threads e parâmetros
    free(threads);
    free(params);

    return 0;
}
