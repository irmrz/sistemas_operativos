# Informe del Grupo N°30

### Laboratorio 1 - Mybash: Programando nuestro propio shell de Linux  

### Integrantes:  

- Bruno Volpini  
  - email:  
- Ignacio Ramirez  
  - email: ignaciotramirez@mi.unc.edu.ar
- Tomas Marmay  
  - email:  
- Kevin Prieto  
  - email: kevin-prieto@mi.unc.edu.ar  
    
-----

## Introducción.

El objetivo de este laboratorio es implementar un `shell` al estilo de *bash* al que llamaremos *mybash*. El programa debe poder:  

- Ejecutar comandos en modo *foreground* y *background*.  
- Redirigir la entrada y salida estándar.  
- Hacer pipe entre comandos.  

Para esto se divide la tarea en 5 módulos:  

- *mybash*: módulo principal.  
- *command*: módulo con las definiciones de los *TADs* para representar comandos.  
- *parsing*: módulo de procesamiento de la entrada del usuario usando un - parser.  
- *execute*: módulo ejecutor de comandos, administra las llamadas al sistema operativo.  
- *builtin*: módulo que implementa los comandos internos del intérprete de comandos.  

### ¿Cómo compilar?

        $ git clone https://irmrz@bitbucket.org/sistop-famaf/so22lab1g30.git
        $ cd so22lab1g30
        $ make
        $ ./mybash

### Algunos comandos probados:  

- Dos comandos simples conectados por un *pipe*  

        ls | wc -l
        wget -O- https://www.gutenberg.org/files/11/11-0.txt | sed s/Alice/nombre/g > out

- Comando simple con un *input* y *output*  

        wc -l < informe.md > informe_contador.txt

- Los comandos internos que implementamos:  

        exit
        help
        cd

- Comandos con dos *pipes*

        ls | grep execute | wc > out.txt
        ip addr | grep ether | head -1

### Desarrollo  

En este laboratorio el progreso en los diferentes módulos siguió la siguiente formula:  
1. Consultar los archivos `.h` y la consigna del *Lab 1*.  
2. Implementar funciones, estructuras, etc. para poder realizar lo pedido.  
3. Chequear que el código haga lo pedido y compile.
4. Repetir **2.** y **3.** hasta llegar al final de cosas a implementar.  
5. Chequear que el código pase los tests dados, y si no los pasa volver a **2.** hasta pasar todos los tests.

-----

## Módulo *command*:  

En este módulo trabajamos con 2 *TADs*, uno para manejar comandos simples (*scommand*) y otro para manejar las tuberías (*pipeline*) que conectan a estos comandos simples, ambos estan implementados con la ayuda de la librería `glib`, en particular con el TAD `GQueue`. Sobre estos *TADs* implementamos una serie de funciones para poder manipular estos *TADs*, como por ejemplo para inicializar una lista, consultar el largo de la lista, encolar un elemento, consultar qué elemento está adelante, etc.

### Comentarios:

En este primer módulo utilizamos la librería `glib` para implementar las colas de comandos y de *pipes*, pero si tuviéramos que rehacer este *lab* hubiéramos elegido un diferente *TAD*, ya que tuvimos varios problemas con `glib`.  
Sin embargo, aparte de estos problemas, este *lab* fue relativamente sencillo y lo completamos sin mayores complicaciones.

-----

## Módulo *parsing*:

El módulo *parsing* tiene como objetivo tomar comandos, sus argumentos, redirectores, los *pipes* y el operador de *background* del `stdin` y transformar esto a nuestros *TADs scommand* y *pipeline* para prepararnos para la ejecución de estos comandos, además de ser robusto para poder tomar entradas erróneas y basura sin comprometer a *mybash*.   

Un problema que encontramos fue al momento de parsear un comando que tuviese los símbolos que nos indican el comienzo de un *string*. Estamos hablando de (`""`) y (`' '`). Para que se entienda más específicamente, un ejemplo claro sería el siguiente:  

        $grep 'model name' /proc/cpuinfo

En este caso, `'model'` y `'name'` son tomados como dos argumentos por separado, por lo que `grep` toma a `'name'` como el archivo a analizar. Una solución rápida sería ejecutar:  

        $grep -F model name /proc/cpuinfo

Si bien este comando nos devuelve lo que buscábamos, falta una solución general que no dependa en su totalidad de las funcionalidades que tenga cada comando.  

### Test  

Cambio en el test:  

        test_parsing.c:126 - Cambio "output == NULL" por "pipeline_length (output) == 1".

-----

## Módulo *builtin*:  

Este módulo implementa maneras de detectar y ejecutar "comandos internos", ya que estos comandos internos actúan de manera diferente a otros comandos. Los comandos internos que fueron implementados son:  `cd`, `help` y `exit`.

-----

## Módulo *execute*:

Como penúltimo módulo y el que más trabajo nos llevó para implementar, este módulo ejecuta comandos, utilizando `fork()` y `execvp()`, enviando la información de un comando a otro con `pipe()`, ejecutándolos con `execvp()`, o en el caso de comandos internos, con los comandos implementados en el módulo *builtin*, y manejando correctamente las redirecciones de entrada y salida (`<` y `>`).
El comportamiento de los redirectores es (con las pruebas hechas hasta el momento) el mismo que *bash*.


### Errores

El error:  

	test_execute.c:200:E:Functionality:test_external_arguments:0: (after this point) Received signal 6 (Aborted)

Si bien encontramos que el problema es a la hora de ejecutar `exevcp()` *(linea 217)*, no podemos encontrar donde estará andando mal, si lo que intenta hacer ese test es lo siguiente:

Setear un pipe de la siguiente forma  

	command arg1 -arg2

y luego contar 1 `fork()`, 1 `execvep()`, 0 `exit()` y que parsee bien los argumentos para el `execvp()`. 
Lo que indicaría que *bash* no puede correr comandos del siguiente estilo:  

	sort -k 4 test.in

pero si probamos correr ese comando en *mybash* y luego el mismo comando en la *terminal* ambos tienen el mismo comportamiento 
Tanto este error como los siguientes:

	-test_execute.c:200:E:Functionality:test_external_arguments:0: (after this point) Received signal 6 (Aborted)  
        -test_execute.c:386:F:Functionality:test_pipe2_child2:0: Assertion 'mock_counter_wait+mock_counter_waitpid == 0' failed  
        -(null):-1:S:Functionality:test_redir_out_child:0: (after this point) Received signal 6 (Aborted)  
       	-(null):-1:S:Functionality:test_redir_in_child:0: (after this point) Received signal 6 (Aborted)  
        -(null):-1:S:Functionality:test_redir_inout_child:0: (after this point) Received signal 6 (Aborted)  

Creemos que se generan por tener una función `static` donde se generan esos testeos, ya que probando esos mismos ejemplos (del mismo tipo) pero en *mybash* todos funcionan correctamente 

-----

## Módulo *mybash*:

Finalmente, tenemos el módulo *mybash* que combina todos los módulos anteriores para poder leer, parsear y ejecutar comandos.

-----
