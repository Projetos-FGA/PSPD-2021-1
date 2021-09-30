// =================================================================================
//                              Trabalho 1 - PSPD
// 	Professor  : Bruno Ribas
// 	Alunos     : Flavio Vieira / 
// 	Matricula  : 15/0125682    /
//
// =================================================================================
//  Resultados
//  file-011-big
//      1 thread  - 21,61s user 0,26s system  99% cpu 21,878 total
//      2 thread  - 31,09s user 0,22s system 199% cpu 15,724 total
//      4 thread  - 32,19s user 0,24s system 314% cpu 10,298 total
//      8 thread  - 32,22s user 0,25s system 353% cpu  9,186 total
//      16 thread - 32,15s user 0,29s system 335% cpu  9,668 total
// =================================================================================
# include <stdio.h>
# include <stdlib.h>
# include <omp.h>
# include <string.h>

#define MAX 100000
#define p_offset 100              //[0     - 99   ]   -> 100 
#define s_offset 1000             //[100   - 999  ]   -> 900
#define f_offset 10000            //[1000  - 9999 ]   -> 9k
#define e_offset 100000           //[10000 - 99999]   -> 90k    
// =================================================================================
typedef struct
{
    int id;
    int cont_votos;
}Candidato;

int comparaVotos (const void *x, const void *y);
void resultado(void);

// ================================  Main ========================================== 
int main( int argc, char *argv[] )
{

    int max_threads;
    double start, wtime;
    wtime = start = omp_get_wtime();

    if(argc < 2)
    {
        fprintf(stderr, "[Erro]: Parametro <caminho/nome_do_arquivo> nao informado!\n");
        exit(EXIT_FAILURE);
    }

    if( argc == 3)
        max_threads = strtol(argv[2], NULL, 10);
    else
        max_threads = 1;                            //sem argv[2] -> sequencial

// =================================================================================
    int i, j, n, fsize, num_reg;
    int sfe[3];
    int sfe_offset[3];
    int num_votos, votos_val, votos_pres;
    float percent;
    FILE *fp;
    Candidato reg_votos_global[MAX];

    memset(reg_votos_global, 0, sizeof(Candidato) * MAX);
    fp = fopen(argv[1], "r");

    sfe_offset[0] = s_offset;
    sfe_offset[1] = f_offset;
    sfe_offset[2] = e_offset;
    
    for(i = 0; i < 3; i++)
        fscanf(fp, "%d", &sfe[i]);

    fseek(fp, 0, SEEK_END);
    fsize = ftell(fp) - 1;
  
    rewind(fp);

    wtime = omp_get_wtime() - wtime;
    printf("leitura incial = %f\n", wtime);
    wtime = omp_get_wtime() ;

    num_votos = 0;
    // while ((fscanf(fp, "%d", &n))!= EOF )
    // {
    //     // printf("linha[%3d] = %5d -> ftell: %d\n", num_votos + 1, n, ftell(fp));
    //     num_votos++;
    // }
    // printf("\n--------------------------------\n");
    // printf("count = %d\n", num_votos);
    // printf("\n--------------------------------\n");
    // num_votos = 0;
    
//==============================================================================
    wtime = omp_get_wtime() - wtime;
    printf("time contagem de linhas= %f\n", wtime);
    wtime = omp_get_wtime() ;
    
    # pragma omp parallel num_threads(max_threads) \
    reduction(+: num_votos) reduction(+: votos_pres)  //reduction(+:reg_votos_global[100])
    {
        int id_th = omp_get_thread_num();
        int num_th = omp_get_num_threads();

        Candidato reg_votos_local[MAX];
        FILE *fp = fopen(argv[1], "r");

        int i, voto;
        int bloco, inicio, fim, resto;

        // definir blocos/chunks de leitura
        bloco = fsize / num_th;
        inicio = bloco * id_th;
        fim = inicio + bloco;

        fseek(fp, inicio, SEEK_SET);
        while(fgetc(fp) != '\n');

        while(inicio < fim)
        {
            fscanf(fp, "%d", &voto);
            if(voto > 0)
            {
                // reg_votos_local[voto].cont_votos++;
                // reg_votos_local[voto].id = voto;

            #pragma omp atomic
                reg_votos_global[voto].cont_votos++;

                reg_votos_global[voto].id = voto;
                
                if( voto < 100)
                    votos_pres++;

                votos_val++;
            }
            num_votos++;
            inicio = ftell(fp);
        }
        fclose(fp);

    }



    wtime = omp_get_wtime() - wtime;
    printf("time bloco paralelo = %f\n", wtime);
    wtime = omp_get_wtime() ;

          // ponteiro posicao inicio    tamanho
    qsort(reg_votos_global           ,  p_offset            , sizeof(Candidato), comparaVotos);
    qsort(reg_votos_global + p_offset, (s_offset - p_offset), sizeof(Candidato), comparaVotos);
    qsort(reg_votos_global + s_offset, (f_offset - s_offset), sizeof(Candidato), comparaVotos);
    qsort(reg_votos_global + f_offset, (e_offset - f_offset), sizeof(Candidato), comparaVotos);
    
    wtime = omp_get_wtime() - wtime;
    printf("time sort = %f\n", wtime);
    wtime = omp_get_wtime();
//------------------------------------------------------------------------------
    
    percent = (reg_votos_global[99].cont_votos * 100) / votos_pres;

    printf("%d %d\n", votos_val, num_votos - votos_val);

    if(percent > 51)
        printf("%d\n", reg_votos_global[99].id);
    else 
        printf("Segundo turno\n");
    
    for(j = 0; j < 3; j++)
    {
        printf("%d", reg_votos_global[sfe_offset[j] - 1].id);
        for(i = 2; i <= sfe[j]; i++)
            printf(" %d", reg_votos_global[sfe_offset[j] -i].id);
        
        printf("\n");
    }

        
    wtime = omp_get_wtime() - wtime;
    start = omp_get_wtime() - start;
    printf("\n\n Completo = %f\n", start);
    return (EXIT_SUCCESS);
}
// ===========================  Sub Programas ================================== 

int comparaVotos (const void *x, const void *y) 
{
    int a, b, c, d;
    a = ((Candidato*)x)->cont_votos;
    b = ((Candidato*)y)->cont_votos;
    c = ((Candidato*)x)->id;
    d = ((Candidato*)y)->id;

    if (a != b)
        return (a - b);
    else 
        return (c - d);
}
//===============================================================================


//===============================================================================
