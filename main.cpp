#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <pthread.h>

//Pra usar a função rand(), temos que adicionar a biblioteca time.h 
//e para saber o valor de RAND_MAX, temos que usar a função stdlib.h.

// constantes para serem usadas na execução do programa
#define tamvet 50  // quantidade de elementos a ser gerado
#define nbuckets 5  // quantidade de buckets para separar o vetor original
#define nthreads 4  // quantidade de Threads para executar o programa

//cria uma estrutura contendo 2 elementos do tipo inteiro e um vetor de int
//a estrutura representará um novo tipo de dados, chamado "bucket"
//logo, será possível criar variáveis do tipo "bucket"

typedef struct {
    int id;
    int tam;
    int elementosVetor[tamvet];
} bucket;

// Variaveis globias usadas na execução do programa
int MINIMO = -1; // contem o menor valor da faixa do bucket
int alternaBucket = 0; // usado pela função "thread_bucket", para alternar entre os buckets
int compBucket = 0; // usado pela função "thread_bucket", para verificar o numero de buckets
double faixaNumeroBuckets = (tamvet / nbuckets);
pthread_mutex_t mutex;
bucket *vetorBucket;

//método que ordena um dado vetor desordenado

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

    // aloca dinamicamente um vetor de tamanho "nbuckets"
    vetorBucket = (bucket *) malloc(sizeof (bucket) * nbuckets);
    int j = 0; // usado como iterador para os elementos de cada bucket
    //e ao final de cada loop terá o tamanho do vetor
    int valorMaior = 0; // variavel auxiliar que conterá o maior valor da faixa de cada bucket
    int valorMenor = 0; // variavel auxiliar que conterá o menor valor da faixa de cada bucket

    // verifica se a divisão de elementos pela quantidade de buckets é inteira
    if ((tamvet % nbuckets) != 0) {

        // buckets que terão +1 em relação numero médio
        int quaBucketsPrimarios = (tamvet % nbuckets); // pega o resto da divisão
        int faixaBucketsPrimarios = (tamvet / nbuckets) + 1; // faixa de valores que o bucket vai conter

        // buckets que terão -1 em relação aos BucktesPrimarios
        int quaBucketsSecundarios = nbuckets - (tamvet % nbuckets); 
        int faixaBucketsSecundarios = (tamvet / nbuckets); // faixa de valores que o bucket vai conter

        // para preencher os bucktes+1
        for (int i = 0; i < quaBucketsPrimarios; ++i) {
            //usa "i" como identificador de cada bucket
            vetorBucket[i].id = i;
            // Como MINIMO é -1 o primeiro vetor recebe valor minimo 0
            valorMenor = MINIMO + 1;
            // MINIMO recebe agora ele mesmo mais a faixa de números. Ex.: faixa = 10 então agora MINIMO = 9
            MINIMO = MINIMO + faixaBucketsPrimarios;
            // Como MINIMO = 9 valor maior do primeiro vetor recebe 9
            valorMaior = MINIMO;
            // Para atribuir os valores a cada posição do bucket
            for (int k = 0; k < tamvet; ++k) {
                if ((pVetorOriginal[k] >= valorMenor) && (pVetorOriginal[k] <= valorMaior)) {
                    vetorBucket[i].elementosVetor[j] = pVetorOriginal[k];
                    j++;
                }
            }
            // Zera j para poder começar a atribuir na posição inicial dos buckets seguintes.
            vetorBucket[i].tam = j; // j contem a quantidade de valores no bucket atual, ou seja, o tamanho
            j = 0;
        }

        // para preencher os bucktes secundários
        for (int i = 0; i < quaBucketsSecundarios; ++i) {
            vetorBucket[quaBucketsPrimarios].id = quaBucketsPrimarios;
            // Como MINIMO é -1 o primeiro vetor recebe valor minimo 0
            valorMenor = MINIMO + 1;
            // MINIMO recebe agora ele mesmo mais a faixa de números. Ex.: faixa = 10 então agora MINIMO = 9
            MINIMO = MINIMO + faixaBucketsSecundarios;
            // Como MINIMO = 9 valor maior do primeiro vetor recebe 9
            valorMaior = MINIMO;
            // Para atribuir os valores a cada posição do bucket
            for (int k = 0; k < tamvet; ++k) {
                if ((pVetorOriginal[k] >= valorMenor) && (pVetorOriginal[k] <= valorMaior)) {
                    vetorBucket[quaBucketsPrimarios].elementosVetor[j] = pVetorOriginal[k];
                    j++;
                }
            }
            // Zera j para poder começar a atribuir na posição inicial dos buckets seguintes.
            vetorBucket[quaBucketsPrimarios].tam = j; // j contem a quantidade de valores no bucket atual, ou seja, o tamanho
            quaBucketsPrimarios++;
            j = 0;
        }

        // caso for divisão inteira
    } else {
        // Para alternar de buckets
        for (int i = 0; i < nbuckets; ++i) {
            vetorBucket[i].id = i;
            // Como MINIMO é -1 o primeiro vetor recebe valor minimo 0
            valorMenor = MINIMO + 1;
            // MINIMO recebe agora ele mesmo mais a faixa de números. Ex.: faixa = 10 então agora MINIMO = 9
            MINIMO = MINIMO + faixaNumeroBuckets;
            // Como MINIMO = 9 valor maior do primeiro vetor recebe 9
            valorMaior = MINIMO;
            // Para atribuir os valores a cada posição do bucket
            for (int k = 0; k < tamvet; ++k) {
                if ((pVetorOriginal[k] >= valorMenor) && (pVetorOriginal[k] <= valorMaior)) {
                    vetorBucket[i].elementosVetor[j] = pVetorOriginal[k];
                    j++;
                }
            }
            // Zera j para poder começar a atribuir na posição inicial dos buckets seguintes.
            vetorBucket[i].tam = j; // j contem a quantidade de valores no bucket atual, ou seja, o tamanho
            j = 0;
        }
    }
}

void *thread_bucket() {
    // enquanto todos os buckets não forem ordenados
    while (compBucket < nbuckets) {
        pthread_mutex_lock(&mutex); // tratando condição de corrida
        // condição usada pois a thread pode ter verificado a condição do while, visto que era válida, então entrado na fila do lock,
        // só que nesse tempo outra thread que já estava em execução pode ter alterado o valor de compBucket e então a condição não é mais válida
        // por isso temos o if, um jeito de verificar novamente a condição quando chega a vez de determinada thread executar
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

int main() {
    // imprime erro caso alguma das seguintes condições seja violada
    if (nthreads < 1 && nbuckets > tamvet) {
        printf("ERRO! Verifique o número de threads informado, bem como o tamanho do vetor e o número de buckets.");
    } else if (nthreads < 1) {
        printf("ERRO! Verifique o número de threads informado.");
    } else if (nbuckets > tamvet) {
        printf("ERRO! Verifique o número de buckets informado e o tamanho do vetor.");
    } else { // caso não tiver erro
        
        int vetorOriginal[tamvet]; // define um vetor com o tamanho estipulado pelo usuario
        int *pVetorOriginal = vetorOriginal; // guarda a primeira posição de memoria do vetor em um ponteiro
        
        int i; // contador usado nos for
        
        // Usando a função srand(), vamos alimentar a função rand() com uma semente, com um número, que é o tempo atual.
        // Assim, os números gerados vão (podem) ser diferentes.
        srand((unsigned) time(NULL));

        // cria vetor de numeros aleatorio
        for (i = 0; i < tamvet; i++) {
            // fazendo módulo com tamvet, garante-se que só será gerado números entre 0 e tamvet-1.
            pVetorOriginal[i] = (rand() % (tamvet));
            printf("%d ", pVetorOriginal[i]);
        }

        printf("\n");
        cria_bucktes(pVetorOriginal); // separa o vetor original em buckets
        printf("\n");

        pthread_mutex_init(&mutex, NULL); // inicializa mutex para tratar condição de corrida
        pthread_t thread;

        short unsigned int j = 0;
        // criando as threads
        // e já manda cada uma organizar os buckets
        for (j = 0; j < nthreads; ++j) {
            pthread_create(&thread, NULL, thread_bucket, NULL);
        }
        // fazendo as threads esperarem até que todas "voltem" da execução
        for (j = 0; j < nthreads; ++j) {
            pthread_join(thread, NULL);
        }

        printf("\n");
        i = 0;
        // imprime o vetor ordenado
        while (i < tamvet) {
            for (j = 0; j < nbuckets; j++) {
                for (int k = 0; k < vetorBucket[j].tam; k++) {
                    vetorOriginal[i] = vetorBucket[j].elementosVetor[k];
                    printf("%d ", vetorOriginal[i]);
                    i++;
                }
            }
        }
        // destroi o mutex
        pthread_mutex_destroy(&mutex);
    }
}