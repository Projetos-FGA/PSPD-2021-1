// =================================================================================
//                              Trabalho 1 - PSPD
// 	Professor  : Bruno Ribas
// 	Alunos     : Flavio Vieira / 
// 	Matricula  : 15/0125682    /
//
// =================================================================================
//  Resultados
//  file-011-big
//      
//   threads = 1-> 22,34s user 0,20s system 99% cpu 22,564 total
//   threads = 2-> 32,23s user 0,27s system 191% cpu 16,991 total
//   threads = 3-> 29,47s user 0,35s system 223% cpu 13,353 total  
//   threads = 4-> 29,85s user 0,32s system 265% cpu 11,343 total 
//   threads = 8-> 32,30s user 0,41s system 275% cpu 11,891 total  
//      
//      
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
    int num_th, sum;
    int i, j, n, fsize, num_reg;
    int sfe[3];
    int sfe_offset[3];
    int num_votos, votos_val, votos_pres;
    float percent;
    FILE *fp;
    Candidato **reg_votos_global;
    Candidato reg_votos_global_res[MAX];
    double start, wtime;
    
    wtime = start = omp_get_wtime();

    if(argc < 2)
    {
        fprintf(stderr, "[Erro]: Parametro <caminho/nome_do_arquivo> nao informado!\n");
        exit(EXIT_FAILURE);
    }

    if( argc == 3)
        num_th = strtol(argv[2], NULL, 10);
    else
        num_th = 1;                            //sem argv[2] -> sequencial

// =================================================================================

    
    int linha_res = num_th + 1;

    reg_votos_global = (Candidato**) calloc((linha_res), sizeof(Candidato*));
    
    for(i = 0; i < linha_res; i++)
        reg_votos_global[i] = (Candidato*) calloc(MAX, sizeof(Candidato));        

    memset(reg_votos_global_res, 0, sizeof(Candidato) * MAX);

    fp = fopen(argv[1], "r");

    sfe_offset[0] = s_offset;
    sfe_offset[1] = f_offset;
    sfe_offset[2] = e_offset;
    
    num_votos = votos_val = votos_pres = num_reg = 0;
    
    for(i = 0; i < 3; i++)
        fscanf(fp, "%d", &sfe[i]);

    fseek(fp, 0, SEEK_END);
    fsize = ftell(fp) - 1;
  
    rewind(fp);
//==============================================================================
    wtime = omp_get_wtime() - wtime;
    printf("antes do bloco paralelo = %f\n", wtime);
    wtime = omp_get_wtime();
    
    # pragma omp parallel num_threads(num_th) 
    // reduction(+: num_votos) reduction(+: votos_pres)
    {
        int id_th = omp_get_thread_num();
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
                // reg_votos_global[id_th][voto].id = voto;
                reg_votos_global[id_th][voto].cont_votos++;

                if( voto < 100)
                    #pragma omp atomic
                        votos_pres++;
                
            }
            #pragma omp atomic
                num_votos++;
            inicio = ftell(fp);
        }
        fclose(fp);

    }    // Fim pragma Parallel
//==============================================================================
    
    wtime = omp_get_wtime() - wtime;
    printf("bloco paralelo = %f\n", wtime);
    wtime = omp_get_wtime(); 
    
    //consolidar votos das threads
// #pragma omp parallel for private(sum)  private(j)
    for( i = 0; i < MAX; i++)
    {
        sum = 0;
        for( j = 0; j < num_th; j++)
            if (reg_votos_global[j][i].cont_votos != 0 )
                sum += reg_votos_global[j][i].cont_votos;
        
        if(sum){
            reg_votos_global_res[i].id = i;
            reg_votos_global_res[i].cont_votos = sum;
            votos_val += sum;
        }
    }

    for(i = 0; i < num_th; i++)
        free(reg_votos_global[i]);
    
    free(reg_votos_global);
//==============================================================================
    wtime = omp_get_wtime() - wtime;
    printf("time consolidar matriz = %f\n", wtime);
    wtime = omp_get_wtime();

    // printf("==================================================================\n");

          // ponteiro posicao inicio    tamanho
    qsort(reg_votos_global_res           ,  p_offset            , sizeof(Candidato), comparaVotos);
    qsort(reg_votos_global_res + p_offset, (s_offset - p_offset), sizeof(Candidato), comparaVotos);
    qsort(reg_votos_global_res + s_offset, (f_offset - s_offset), sizeof(Candidato), comparaVotos);
    qsort(reg_votos_global_res + f_offset, (e_offset - f_offset), sizeof(Candidato), comparaVotos);
    
    wtime = omp_get_wtime() - wtime;
    printf("time sort = %f\n", wtime);
    wtime = omp_get_wtime();

//------------------------------------------------------------------------------
    
    percent = (reg_votos_global_res[99].cont_votos * 100) / votos_pres;

    printf("%d %d\n", votos_val, num_votos - votos_val);

    if(percent > 51)
        printf("%d\n", reg_votos_global_res[99].id);
    else 
        printf("Segundo turno\n");
    
    for(j = 0; j < 3; j++)
    {
        printf("%d", reg_votos_global_res[sfe_offset[j] - 1].id);
        for(i = 2; i <= sfe[j]; i++)
            printf(" %d", reg_votos_global_res[sfe_offset[j] -i].id);
        
        printf("\n");
    }

    wtime = omp_get_wtime() - wtime;
    printf("\n\n display dados = %f\n", wtime);
    return (EXIT_SUCCESS);
}
// =========================== Sub Programas =================================== 

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
