# Informe del Grupo N°30

-----  

### Laboratorio 2 - Sistema de archivos Big Brother   

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


### ¿Cómo compilar?

-        $ git clone https://marmiiT@bitbucket.org/sistop-famaf/so22lab4g30.git
-        $ cd so22lab4g30
-        $ make 
-        $ mkdir mnt 
-        $ ./fat-fuse resources/fatfs.img ./mnt
            - Para montar la imagen
-        $ ./fat-fuse -d resources/fatfs.img ./mnt
            - Para ver en tiempo real las fat-fuse operations
-        $ ./fat-fuse -f resources/fatfs.img ./mnt
           -Para debug mode

-----

### *Coding style*

Para mantener un estilo de código uniforme utilizamos la herramienta *clang-format* a través de su extensión en el editor VSCode.
https://clang.llvm.org/docs/ClangFormat.html

-----

### Preguntas y Respuestas

##### Cuando se ejecuta el main con la opción -d, ¿qué se está mostrando en la pantalla? :
- Por pantalla se nos muestran todas las *fuse operations* que se están realizando en tiempo real mientras se hace uso del sistema de archivos.

##### ¿Hay alguna manera de saber el nombre del archivo guardado en el cluster 157? :
- Es posible obtener el nombre de un archivo dado el número de cluster. Si repetimos el proceso que hicimos en big_brother.c, casteamos como fat_file y lo leemos con full_pread podemos acceder al nombre. Pero para ello debemos estar seguro que dicho cluster sea un fat_file porque de no ser así tendríamos un segmentation fault.


##### ¿Dónde se guardan las entradas de directorio? ¿Cuántos archivos puede tener adentro un directorio en FAT32?:
- Las entradas se guardan dependiendo del cluster order, ya que éste determina el tamaño del directorio. Eso nos indica el número de dir_entries que puede tener, es decir la cantidad de archivos.


##### Cuando se ejecuta el comando como ls -l, el sistema operativo, ¿Llama a algún programa de usuario?: 
- El OS no llama a ningún programa de usuario y tampoco hace ninguna llamada del sistema. Esto se debe a que toda la información ya está guardada en el árbol de fat_fuse, por lo que no necesita leerlo de otro lado. 
    
##### ¿A alguna llamada al sistema? ¿Cómo se conecta esto con FUSE? ¿Qué funciones de su código se ejecutan finalmente?:

- Fat_fuse, a nivel interno, abre el directorio en el que se encuentra para ver sus atributos (OPENDIR , GETATTR, READDIR, LOOKUP ) y obtener de esta manera a sus hijos. Luego itera hasta que no le quede ningún hijo, mientras hace uso de lookup y getattr.


##### ¿Por qué tienen que escribir las entradas de directorio manualmente pero no tienen que guardar la tabla FAT cada vez que la modifican? :
- Es necesario que se modifiquen las entradas de directorio ya que dentro de éstas están marcados los archivos que se deben ignorar por estar eliminados. Además nos permite ver como esta formado el árbol de archivos. Por otro lado la FAT es una lista enlazada que permite agilizar los accesos a discos y ésta se encuentra guardada en RAM.

##### Para los sistemas de archivos FAT32, la tabla FAT, ¿Siempre tiene el mismo tamaño? En caso de que sea así, ¿Qué tamaño tiene? :  
- Si, el número de clusters es constante (4.294.967.264), que es aproximadamente 2^32. Esto se debe a como esta impementada la estructura de la fat table. Podemos ver que la cantidad de cluster esta representada con un u32 (unsigned int de 32 bits), por lo tanto la capacidad máxima es de 2^32. Por otro lado el tamaño de un solo cluster es de 2^9 == 512 bytes. 
    Entonces el tamaño de la tabla FAT32 es de 2^32 * 2^9 = 2^41 = 2TB 
    
-----

## Conclusion

- En este lab aprendimos como funciona el sistema de archivos Fat-Fuse y lo facil que es ocultarle informacion al usuario desde la propia implementacion. 