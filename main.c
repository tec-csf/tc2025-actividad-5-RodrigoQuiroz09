/**
 * Autor - Rodrigo Quiroz Reyes
 * Fecha - 17/09/2020
 * Actividad individual 5: IPC 
 * Ejercicio - Generar un anillo de procesos (Token Ring) y que se pasen una variable testigo entre los hijos mediante un pipe
*/

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

typedef struct {
  int pipe[2];
}Tuberia;//Estructura Tuberia para usarlo como 2d array de pipes

void procesos_fork(Tuberia*,int);//Método para realizar todo el método de la creación de forks
char leer(Tuberia*);//Método para leer desde un pipe
void escribir(char,Tuberia*);//Método para escribir desde un pipe
void habilitar_pipe(Tuberia *,int); //Metodo para habilitar todas los pipes para un uso libre en los ciclos

int main(int argc, char * const * argv) 
{

    char* cvalue; // Extracción de el valor del inpu
    int c; //Varible para obtener el argumento de la command line
    Tuberia *procesos; //Arreglo de pipes

    
    while ((c = getopt (argc, argv, "n:")) != -1)
        switch (c)
    {
        case 'n':
            cvalue = optarg;
            break;
        case '?':
            if (optopt == 'n')
                fprintf (stderr, "Opción -%c requiere un argumento de tipo entero.\n", optopt);
            else if (isprint (optopt))
                fprintf (stderr, "Opción desconocida '-%c'. Debe de usar el parámetro -n\n", optopt);
            else
                fprintf (stderr,
                         "Opción desconocida '\\x%x'.\n",
                         optopt);
            return 1;
        default:
            abort ();
    }

    int no_hijos=atoi(cvalue); //Conversión del valor a int

    if(no_hijos<1)
    {
        printf("No se puede realizar un circulo con 0 procesos\n");
        printf("Favor de intentar la inserción de no. de procesos con \"./a.out -n x\", siendo x mayor a 1\n");
    }else
    {
        //Malloc de la estructura
        procesos=(Tuberia*)malloc(no_hijos*sizeof(Tuberia));
        
        procesos_fork(procesos,no_hijos);

        free(procesos);
    }


    
    return 0;
    
}


/**
 * Metodo para habilitar todos los pares de ints a pipes para su uso en el loop del Token Rng
 * 
 * @param int Número de hijos dados por el usuario
 * @param Tuberia* Apuntador a un par de pipes
 */
void habilitar_pipe(Tuberia * hijos, int no_hijos)
{
    Tuberia *aux=hijos;
     for(;aux<(hijos+no_hijos);++aux)
        {  
            pipe(aux->pipe);
        }
}

/**
 * Metodo para generar el loop infinito y que se impriman de manera circular los pid pasando el char
 * testigo entre el array de pipes
 * 
 * @param int Número de hijos dados por el usuario
 * @param Tuberia* Apuntador a un par de pipes
 */
void procesos_fork(Tuberia * hijos, int no_hijos){
    
    pid_t pid; //Proceso inicial
    Tuberia *aux=hijos;//Apuntador auxiliar que nos ayuda en el loop
    int i=0;//Counter para dar formato a algunos printf
    char testigoInit='T'; //Testigo inicial
    int status;//Estado de un procesos hijo

    habilitar_pipe(hijos,no_hijos); //Creo todos los pipes

    pid=fork();

    //Genero un proceso 0 que no se encuentra dentro del Token Ring para meter el testigo sin alterar el formato de impresión
    if(pid==0) 
    {   
        printf("Generando el Token Ring\n");
        escribir(testigoInit,(aux+no_hijos-1));
        exit(0);
        
    }
    else
    {
        if(waitpid(pid, &status,0)!=-1)
        {
            if(WIFEXITED(status))
            {
                sleep(1);
                }
            }
        }  

    printf("\n");

    //Genero todos los procesos con while 1 para mantenerlos con vida y que impriman con el mismo formato
    for(;aux<(hijos+no_hijos);aux++,i++)
    {    
            
        pid=fork();//Creación del proceso hijo      
            
        if(pid==-1)//Condición en caso de error
        {
            printf("Hubo un error a la hora de crear al hijo\n");
            printf("Se han creado %d hijos\n", i+1);
            aux=(hijos+no_hijos);//Se aumenta hasta el final del ciclo
        }

        if(pid==0 && i==0)//En caso de que sea el primer proceso se lee desde el último pipe
        {
            while(1)
            {
                char testigo=leer((aux+no_hijos-1)); //Leo el testigo enviado por el proceso 0
                printf("<-- Soy el proceso con PID = %d y recibí el testigo = %c, el cual tendré por 5 segundos\n",getpid(),testigo);
                sleep(2);
                printf("<—- Soy el proceso con PID %d y acabo de enviar el testigo %c\n",getpid(),testigo);
                escribir(testigo,aux); //Escribo para el primer pipe 
                printf("\n");  
            }
            exit(0);    
        }
        else if(pid==0)//Condición de todos los demás hijos
        {
            while(1)
            {   
                char testigo=leer(aux-1); //Leo desde el ciclo correspondiente 
                printf("<-- Soy el proceso con PID = %d y recibí el testigo = %c, el cual tendré por 5 segundos\n",getpid(),testigo);
                sleep(2);
                printf("<—- Soy el proceso con PID %d y acabo de enviar el testigo %c\n",getpid(),testigo);
                escribir(testigo,aux); // Escribo para el siguiente pid
                printf("\n");                                        
                    
            }                                               
            exit(1);
        }

    }
}

/**
 * Metodo para leer el pipe entre los procesos y regresar el promedio
 * 
 * @param Tuberia* Apuntador a un par de pipes
 */
char leer(Tuberia* hijos)
{
    char i;
    close(hijos->pipe[1]);//Se cierra el otro extremo del pipe
    read(hijos->pipe[0], &i, sizeof(char)); //Lectura del pipe
    return i;
}

/**
 * Metodo para escribir el promedio mediante el pipe entre los procesos
 * 
 * @param char El testigo para escribirlo en el pipe
 * @param Tuberia* Apuntador a un par de pipes
 */
void escribir( char testigo,Tuberia* hijos)
{
     
    close(hijos->pipe[0]);//Se cierra el otro extremo del pipe
    write(hijos->pipe[1], &testigo, sizeof(testigo)); //Escritura del pipe

    return;
}