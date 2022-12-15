# Informa del grupo N°30

### Laboratorio 3 - Planificador de Procesos

### Integrantes:  

- Bruno Volpini
    - email: bruno.volpini@mi.unc.edu.ar

- Ignacio Ramirez
    - email: ignaciotramirez@mi.unc.edu.ar

- Tomas Marmay
    - email: tomas.marmay@mi.unc.edu.ar

- Kevin Prieto
    - email: kevin-prieto@mi.unc.edu.ar


-----

## Introduccion 

En este laboratorio vamos a modificar el planificador base de xv6 (*Round Robin*) para que utilize la politica de planificacion llamada Multi-Level Feedback Queue (MLFQ), ademas vamos a realizar mediciones utilizando *iobench* y *cpubench* con diferentes *Quantums* y politicas de planificacion.

### ¿Cómo compilar?

        $ git clone https://<user>@bitbucket.org/sistop-famaf/so22lab3g30.git
        $ cd so22lab3g30
        $ make qemu CPUS=1

### *Coding style*

  - para mantener el *coding style* lo mas parecido a *xv6* usamos la herramienta *Indent*:

        indent -bad -bli0 -i4 -l99 -ncs -npcs -npsl -lc99 -ts4 -ut kernel/proc.c



### Primera parte

- 1) xv6 usa una politica de planificacion de round robin, lo que significa que le da un tiempo especifico (quantum) a cada proceso para usar la cpu.
- 2) - a) El quantum en xv6 esta definido como 1000000 ciclos de reloj que es equivalente a 0,1 segundo en qemu.
    - b) En xv6 el cambio de contexto esta determinado por la funcion swtch por lo que el cambio de contexto va a demorar lo que tarde dicha funcion.
    - c) No, el cambio de contexto no consume tiempo del quantum, de echo, cuando se acaba el quantum se hace el cambio de contexto (si el panificador lo decide).
    - d) De que se le asigne menos tiempo no, pero un proceso puede perfectamente no utilizar todo el tiempo del quantum, basta que el proceso llame a la funcion sched. Que dicha funcion se llama cuando ocurre una de las siguientes situaciones: yeild,exit y sleep  


-----


### Segunda parte

En esta seccion vamos a correr los benchmarks *iobench* y *cpubench* utilizando *qemu* con un solo CPU (con `make CPUS=1 qemu`)

-   Quantum Default

    - En el **Caso 0** y el **Caso 1**, corremos *iobench* y *cpubench* solos, y nos van a dar un caso base para comparar los siguientes *benchmarks*.  
    - En el **Caso 2**, cuando corremos 1 *iobench* y 1 *cpubench*, vemos que el *cpubench* corre basicamente de manera identica al **Caso 1**, pero en cambio, el *iobench* se ve muy afectado, con metricas acercandose a 0. Esto se debe a que el cpubench va a usar todo su *Quantum* dado, mientras que el *iobench* va a ceder su *Quantum* rapidamente cuando necesite hacer operaciones I/O y va a tener que esperar para retomar control hasta que *cpubench* termine todo su tiempo alocado antes de poder hacer otra corta operacion y ceder su *Quantum* nuevamente, drasticamente reduciendo sus metricas.  
    - En el **Caso 3** ambos *iobench* van a correr de manera similar al **Caso 0**, ya que cuando *iobench* luego de hacer una pequeña operacion cede su *Quantum* al hace alguna operacion I/O, el otro *iobench* va a hacer algo similar, correr una pequeña operacion y ceder su *Quantum* al hacer I/O, entonces ambos procesos van a intercalarse, cuando un proceso se esta ejecutando, el otro va a estar esperando a la respuesta de una operacion I/O, entonces las metricas van a ser muy similares al caso donde se corre un solo *iobench*.
    - En el **Caso 4** a diferencia del **Caso 3**, ambos *cpubench* van a usar todo su *Quantum* realizando operaciones, entonces las metricas van a ser aproximadamente ~ 1/2 del **Caso 1**, en donde un solo *cpubench* va a poder utilizar el *cpu* para si solo.

    - Los graficos de las pruebas se pueden ver en Benchmarks_EJ2.tar.gz


-----


### Tercera parte

-   1) Basta con definir la constante NPRIO en el archivo param.h que es donde estn definidas las constantes de xv6. Para añadir los campos solicitados hay que editar en proc.h la estructura de porc y luego editar una funcione(allocproc) para inicializar estos nuevos campos. Luego cremos 3 syscalls: para subir y bajar la prioridad de un proceso y una tercera para subir la cantidad de veces que el scheduler elige a un proceso.
    Ademas tuvimos que entender un poco cuando el proceso termina antes de un quantum para premiarlo con prioridad (sleep) y cuando termina porque se ocupo todo el quantum para bajarle la prioridad (yield).   
-   2) Añadimos los dos campos requeridos en el printf de procdump().


-----
### Cuarta parte
-   2) Como podemos ver en los graficos no hay una mejora notable como se esperara de MLFQ. Esto se debe a la implementacion que tuvimos que hacer (*) que en casos es mejor Round Robin y en casos peor, por lo que en promedio son parecidas.
       - En el **Caso 0** podemos ver que la cantidad de IOP's aumento comparado con xv6 sin MLFQ, con un maximo de 6464 IOP's.
       - En el **Caso 1** podemos ver que la cantidad de KFLOP's aumento, con un maximo de 86846 KFLOP's
       - En el **Caso 2** los KFLOP's se mantienen igual que el **Caso 1** pero los IOP's se reducieron a la mitad comparado a xv6 sin MLFQ
       - En el **Caso 3** el segundo *iobench* no corre en ningun momento, solo quedando las metricas de 1 solo IObench, mas adelante notamos que cuando reducimos los quantums a 100 veces mas chicos, ambos procesos corrieron
       - En el **Caso 4** los dos procesos utilizan todo su quantum, entonces cada proceso utiliza la mitad de KFLOP's comparado con el **Caso 1**
       - Los graficos de las pruebas se pueden ver en Graficos_MLFQ.tar.gz
-   3) En este nuevo planificador no produce starvation porque en el scheduler cada un cierta cantidad de ticks, pone a los procesos que estan en runneble en la mejor prioridad. Lo bueno de la forma en la que esta implementada esta parte es que solo lo hace en los procesos que estan en las colas y ademas no necesita un loop para subir todos los procesos de prioridad, sino que se aprovecha del loop en el que esta, por lo que esta accion nos sale "gratis".
-----

### Errores Implemantacion (*):  
- Nuestra idea para implemanar MLFQ era a medida que se genear procesos en la tabla de procesos (proc[NPROC]) los ponia en una cola que representaba la mejor prioridad. A medida que estos procesos corrian podian mantenerse o bajar la prioridad, lo que los bajaria a otra cola. En total habia 3 colas que representaba 3 niveles de prioridades distintos. Luego el scheduler se fijaria si en la cola mas importante hay un proceso y si habia lo ponia a correr. Pero en el caso en el que la cola mas importante no tenia procesos para corrrer saltaba a la segunda mas importante y chequeaba lo mismo. Y asi para las 3 colas.
- Dejamos el codigo de esta implemantacion igual salvo el scheduler que esta comentado. No pudimos hacerlo andar pero estamos muy cerca, creemos que todos los pasos estan correctos y debe haber en algun lugar un error que genera un panic. El problema fue que se nos acerco el dia de la entrega y teniamos que entregar algo que ande. Entoces tuvimo que escribir la imlementacion actual que funciona pero no es la mejor implementacion.



