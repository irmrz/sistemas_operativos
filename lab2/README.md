# Informe del Grupo N°30

-----  

### Laboratorio 2 - Semáforos en XV6  

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

## Introducción.

En este lab implementamos semáforos nombrados en XV6, estos semáforos nos permiten controlar el acceso a recursos compartidos en entornos de multiprocesamiento. Con este fin utilizamos las siguientes *syscalls*:  

-  `acquire()` y `release()`:  
   La primera *syscall*, `acquire()`, bloquea la *spinlock* que elegimos, para que el *thread* que toma esta *spinlock* sea el único en cierta parte de nuestro código, y cuando terminamos de ejecutar el código que queremos, vamos a usar `release()` para liberar la *spinlock* tomada.  

-  `sleep()`:  
   Esta *syscall*, cuando es llamada por un proceso, pone al proceso a "dormir" terminando su *timeslice* y cambiando su estado a `SLEEPING` para que no sea elegido para ejecutar por el *scheduler*.
-  `wakeup()`:  
   Despierta a un proceso que está "durmiendo", cambiando su estado a `RUNNABLE`, para que el *scheduler* lo pueda ejecutar.
-  `argint()`:  
   Toma el ***n*** *argumento* pasado a la función, y lo devuelve como puntero.  

-----

### ¿Cómo compilar?

        $ git clone https://<user>@bitbucket.org/sistop-famaf/so22lab2g30.git
        $ cd so22lab2g30
        $ make qemu

-----

- ### *Coding style*

  - para mantener el *coding style* lo mas parecido a *xv6* usamos la herramienta *Indent*:

        indent -bad -bli0 -i4 -l99 -ncs -npcs -npsl -lc99 -ts4 -ut syssemaphore.c

-----

## Decisiones de implementación    

- ### `sem_open()`  

  - En el caso en donde el usuario use un semáforo ***x***, y llame a la función `sem_open(x,n)` nosotros decidimos que no se pise el semáforo anterior y devuelva un error indicando que este semaforo esta en uso.

- ### `sem_close()`  

  - En el caso en donde el usuario use un semáforo ***x***, y llame a la función `sem_close(x)` nosotros decidimos darle el completo control al usuario. Es decir que pude cerrar un semaforo independientemente de el valor en el que este. Pero decidimos "informar" al usuario cual fue el valor del semaforo antes de llamar a sem_close.

- ### semáforos

    - La cantidad de semáforos es correspondiente a un semáforo por cada *syscall* (declaradas en syscall.h) para poder permitir en un caso extremo poder trabajar con todas las *syscalls* al mismo tiempo sin tener problemas de concurrencia.

