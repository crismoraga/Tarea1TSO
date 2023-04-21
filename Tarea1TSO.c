#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/wait.h>

//IMPORTANTE!! PARA COMPILAR SE DEBE HACER DE LA SIGUIENTE MANERA:
// gcc Tarea1TSO -std=gnu11

//LUEGO, AL CORRER EL PROGRAMA SE DEBE HACER:
//./a.out N 
// donde N es el numero que recibe el programa

//Definimos las funciones para calcular pi, de Gregory-Leibniz y la de Nilakantha, que seran llamadas mas adelante

//Gregory-Leibniz
double gregory_leibniz(int N){
    double x = 0.0;
    int a = 1;
    for (int i = 0; i < N; i++) {
        x += a * 4.0 / (2*i + 1);
        a *= -1;
    }
    return x;
}

//Nilakantha
double nilakantha(int N){
    double y = 3.0;
    int b = 1;
    for (int i = 1; i <= N; i++) {
        int denominador = 2*i*(2*i + 1)*(2*i + 2);
        y += b * 4.0 / denominador;
        b *= -1;
    }
    return y;
}

//Importante, estas funciones fueron sacadas y adaptadas de internet, no fue de nuestra autoria completamente, solo su implementacion

typedef struct{ //Definimos una estructura donde guardaremos los valores de N, de pi del primer hijo y de pi del segundo hijo
	
	int n;
	double pi_hijo1;
	double pi_hijo2;
	
}memoria_compartida;

//Teniendo las funciones hechas, creamos el main donde se crearán los procesos padre e hijos

int main(int argc, char** argv) { //Definimos que main reciba argumentos
	
	//Capturamos el valor ingresado
	int N = atoi(argv[1]);
	
	if(N<0){ //Corroboramos que sea un valor válido
		printf("Número debe ser mayor que cero \n"); //De no serlo, cierra el programa para evitar errores
		return 1;
	}
	
	//Creamos la memoria compartida
	memoria_compartida *mem= mmap(NULL, sizeof(memoria_compartida), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    mem->n = N; //Puntero al valor N almacenado en memoria compartida

	// Creamos el primer hijo que calculara la serie de Gregory-Leibniz
    pid_t hijo1 = fork();
    
    if (hijo1 == 0) { //Sabemos que es el hijo por su id

        int N1 = mem->n; //Sacamos N de la memoria compartida
        double pi1 = gregory_leibniz(N1); //Creamos la variable pi1 para almacenar el valor obtenido en la funcion
        printf("Serie de Gregory-Leibniz: %.15f\n", pi1);
        mem->pi_hijo1 = pi1; //Guardamos el valor obtenido en pi1 en la memoria
        exit(0);
    }

    // Creamos el segundo hijo para calcular la serie de Nilakantha
    pid_t hijo2 = fork();
    if (hijo2 == 0) { //con esto sabemos que es el hijo
    
        int N2 = mem->n; // Obtenemos N desde la memoria compartida creada
        double pi2 = nilakantha(N2); //Llamamos la función con el parámetro obtenido y lo guardamos en pi2
        printf("Serie de Nilakantha: %.15f\n", pi2);
        mem->pi_hijo2 = pi2; //Guardamos el valor del segundo hijo en la memoria
        exit(0);
    }

    // Esperamos a que ambos hijos terminen, para evitar zombies o huerfanos
    waitpid(hijo1, NULL, 0);
    waitpid(hijo2, NULL, 0);
    
    //Definimos los valores de pi de cada serie obtenidos, sacandolas de la memoria compartida
    double pi_gregory = mem->pi_hijo1; 
    double pi_nilakantha = mem->pi_hijo2;

	//Mostramos la diferencia entre ambos resultados
	
	double pi_final; //Variable que guardará la diferencia entre ambos
	pi_final = pi_gregory - pi_nilakantha; //Casi siempre será negativa ya que Nilakantha tiende a ser mayor que Gregory-Leibniz
	
	printf("La diferencia entre ambos es de: %.15f\n", pi_final);
		
	//Finalmente liberamos la memoria compartida creada 
    munmap(mem, sizeof(memoria_compartida));
	
	return 0;
}

