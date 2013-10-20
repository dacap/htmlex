# htmlex - un poderoso preprocesador del lenguaje de marcado de hipertexto (HTML)
> Copyright (C) 2001, 2002, 2003 por David Capello

# Introducci�n

  htmlex es un peque�o pero poderoso preprocesador de ficheros HTML, con el
  cual podr� tener varias ventajas con respecto al formato convencional de
  los archivos .html. Adem�s, como htmlex es completamente compatible con la
  sintaxis HTML estandard, significa que cualquier archivo que ya haya creado
  en HTML puro, lo podr� renombrar tranquilamente a .htex.

  Este software se distribuye bajo la [licencia MIT](license.txt).

## Opciones

  htmlex interpreta las siguientes opciones en la l�nea de comandos:

    -c   compila todos los pr�ximos ficheros (obtiene un archivo de salida
         desde "-o" o genera un nombre con extension .html, mire -E)
    -o   agrega ficheros de salida
    -a   agrega argumentos a los archivos de entrada
    -I   agrega rutas de inclusi�n para b�scar ficheros
    -E   cambia la extensi�n HTML utilizada por la opci�n -c (.html por
         defecto)
    -k   elimina los comentarios (comportamiento antiguo de htmlex)
    -d   calcula las dependencias de los ficheros de entrada (salida
         hacia STDOUT)
    -v   activa el modo verboso (para ver que hace htmlex)
    -V   activa el modo muy verboso (para depurar archivos .htex)
    -h   muestra esta ayuda (en ingl�s) y sale
    --   frena los argumentos -coaI

## Ejecuci�n

  Referencias
  -----------

  STDIN      Entrada estandard: desde el teclado o desde un fichero.

  STDOUT     Salida estandard: hacia la pantalla o hacia un fichero.

  Conducta com�n
  --------------

  ./htmlex
  Procesa STDIN y deja los resultados en STDOUT.

  ./htmlex archivo argumentos...
  Procesa el `archivo' y deja el resultado en STDOUT.

  ./htmlex -c archivos... -a argumentos...
  Procesa los `archivos' y deja los resultados en `archivos.html'.

  ./htmlex -c archivos... -o destinos...
  ./htmlex -o destinos... -c archivos...
  Procesa los `archivos' y deja los resultados en los `destinos'.

  Algunos ejemplos de equivalencias
  ---------------------------------

  ./htmlex fichero.htex arg1 arg2 > fichero.html
  ./htmlex -c fichero.htex -a arg1 arg2
  ./htmlex -c fichero.htex -o fichero.html -a arg1 arg2
  ./htmlex -a arg1 arg2 < fichero.htex > fichero.html
  ./htmlex -a arg1 arg2 -o fichero.html < fichero.htex

  Errores comunes
  ---------------

  ./htmlex -I ejemplos archivo.htex > archivo.html
  Aqu� el programa esperar� la entrada de STDIN, ya que archivo.htex
  cuenta como otra ruta m�s para "-I".
  Posibles soluciones:
    ./htmlex -I ejemplos -- archivo.htex > archivo.html
    ./htmlex -I ejemplos -c archivo.htex
    ./htmlex archivo.htex -I ejemplos > archivo.html

  ./htmlex -c index.htex es Espa�ol
  Aqu� se intentar� compilar los ficheros `index.htex', `es' y
  `Espa�ol', en vez de pasarle los argumentos al fichero `index.htex'.
  Posibles soluciones:
    ./htmlex -c index.htex -a es Espa�ol
    ./htmlex index.htex es Espa�ol > index.html

  Cambios con respecto a versiones anteriores
  -------------------------------------------

  * Ahora puede utilizar "-I" en vez de "-i" (como en gcc).
  * Desde ahora el orden de los argumentos no es necesario, por lo que
    puede hacer cosas como:
    ./htmlex -c orig.htex -o dest.html -I dir

## Caracter�sticas

  Referencias
  -----------

  Esta notaci�n se usar� a lo largo de las definiciones:
    expresi�n             La _expresi�n_ es obligatoria.
    [ expresi�n ]         La _expresi�n_ es opcional, puede o no estar.
    { expresi�n }         La _expresi�n_ puede no estar o puede repetirse
                          una o m�s veces.
    origen -> resultado   Indica que _resultado_ ser� obtenido al procesar
                          mediante el programa el texto de _origen_.

  Etiquetas
  ---------

  Lista de las etiquetas que puede utilizar en los ficheros .htex (por
  orden alfab�tico):

  <!args>
    Devuelve la cantidad de argumentos que se le pasaron al fichero de
    entrada. Es igual a 0 si no se le pas� ning�n par�metro.
    Ejemplo:
      A ingresado <!args> argumento(s).

  <!arg1> ... <!argN>
    Retornan el valor del argumento indicado. Los argumentos se le pasan a
    los ficheros a trav�s de la l�nea de comandos con la opci�n -a o por
    medio de la etiqueta <!include fichero arg1 ... argN>.
    Ejemplo:
       2do argumento = <!arg2>.

  <!basename fichero>
    Devuelve el nombre base del _fichero_, es decir, le saca la extensi�n.
    Ejemplos:
      <!basename src/foo.c>    -> src/foo
      <!basename src-1.0/bar>  -> src-1.0/bar
      <!basename hacks>        -> hacks

  <!chop palabra>
    Le quita el �ltimo car�cter a la _palabra_.
    Ejemplos:
      <!chop llor�n>      -> llor�
      <!chop directorio/> -> directorio

  <!clean palabra>
    Limpia la _palabra_ quit�ndole los espacios (tabuladores y retornos
    de l�neas) que tenga al inicio y al final.
    Ejemplo:
      *<!clean " hola ">* -> *hola*

  <!dep fichero>
    Agrega como dependencia el `fichero'. Como es de esperarce, esta etiqueta
    tiene efecto s�lo en el proceso de generaci�n de dependencias.
    Ejemplo:
      <!exec ./script.sh>
      <!dep script.sh>

  <!dir fichero>
    Devuelve el directorio del _fichero_, es decir, le saca nombre.
    Ejemplos:
      <!dir src/foo.c> -> src/
      <!dir a/b/foo.h> -> a/b/
      <!dir hacks>     -> ./

  <!elif expresi�n>
    Ver <!if>.

  <!else>
    Ver <!if>.

  <!end>
    Ver <!function>.

  <!exec fichero { argumentos }>
    Ejecuta el _fichero_ indicado pas�ndole los _argumentos_. Lo que el
    comando imprima por la salida estandard (STDOUT), ser� insertado tal
    cual en el resultado.
    Ejemplos:
      <!exec date>
      <!exec gcc --version>

  <!exec-proc fichero { argumentos }>
    Igual que <!exec ...> solamente que �ste procesar� los resultados
    como a otro fichero htmlex.
    Ejemplos:
      <!exec-proc cat cabecera.htex>
      <!exec-proc cat "pie de pagina.htex">
      <!exec-proc cat <!find macros.htex>>

  <!fi>
    Ver <!if>.

  <!file-size fichero>
    Devuelve el tama�o del _fichero_ en bytes, kilo-bytes (k) o mega-bytes (M)
    dependiendo de cual sea mejor para la "vista humana".
    Ejemplo:
      imagen <!file-size imagen.jpg>

  <!find fichero>
    Intentar� encontrar el _fichero_ en todas las ubicaciones posibles
    (en el directorio actual y en los directorios indicados con -i), y
    devolver� el nombre del mismo con la ruta de acceso correspondiente,
    en caso de que el _fichero_ no se pueda encontrar, no devuelve nada.
    Ejemplos:
      <!if <!find index.es.html>>
        espa�ol
      <!fi>

  <!function nombre { submacros }>
    Crea una nueva macro funcional al estilo C/C++. El programa al ver
    esta etiqueta, sigue leyendo el archivo de forma "cruda" hasta
    encontrar la etiqueta <!end>. Luego, la macro puede ser llamada
    como cualquier otra etiqueta: <!nombre {args}>.
    Ejemplo:
      <!function mi_macro a b c>a es b c<!end>
      <!mi_macro Esta mi macro>.
    Resulta en:
      Esta es mi macro.

  <!if expresi�n>
    Una de las m�s poderosas caracter�sticas que tiene htmlex con respecto
    al HTML convencional, es la posibilidad de controlar bloques de
    condiciones.

    Por medio de cuatro etiquetas podr� hacer la mayor�a de operaciones de
    condici�n de cualquier otro lenguaje de programaci�n. Aunque hoy en d�a,
    htmlex posee un pobre manejo de operaciones, en el futuro tal vez se
    intente mejorar este aspecto.

    La sintaxis general es la siguiente:

      <!if expresi�n> bloque
      { <!elif expresi�n> bloque }
      [ <!else> bloque ]
      <!fi>

    Los _bloques_ pueden ser cualquier texto (o simplemente nada), hasta
    pueden tener otras etiquetas a su vez, y por consiguiente, puede tener
    m�s etiquetas <!if> anidadas.

    En cuanto a las _expresiones_ para realizar las comparaciones, pueden
    tener estos formatos (en caso contrario siempre dar� falso):

      FORMATO                VERDADERO SI...
      -------                ---------------
      n�mero                 _n�mero_ es distinto a cero
      ! n�mero               _n�mero_ es cero
      palabra                _palabra_ tiene un caracter como m�nimo
      ! palabra              _palabra_ est� vac�a
      n�mero1 < n�mero2      _n�mero1_ es menor a _n�mero2_
      n�mero1 > n�mero2      _n�mero1_ es mayor a _n�mero2_
      n�mero1 <= n�mero2     _n�mero1_ es menor o igual a _n�mero2_
      n�mero1 >= n�mero2     _n�mero1_ es mayor o igual a _n�mero2_
      n�mero1 == n�mero2     _n�mero1_ es igual a _n�mero2_
      n�mero1 != n�mero2     _n�mero1_ es diferente a _n�mero2_
      palabra1 == palabra2   _palabra1_ es id�ntica a _palabra2_
      palabra1 != palabra2   _palabra1_ difiere en un caracter a _palabra2_

    Los _n�meros_ se distinguen de las _palabras_ cuando tiene alguno de
    los siguientes formatos:

      [1-9][0-9]*      un decimal
      0x[0-9a-fA-F]*   un hexadecimal
      0[0-7]           un octal

    Ejemplos:
      <!if <!arg1> == --help>
        �Necesita ayuda?
      <!else>
        Pruebe con --help.
      <!fi>
      <!if <!tolower HOLA> == <!tolower Hola>>
        funciona
      <!fi>
      <!if>Por ac� nunca pasar�<!fi>

  <!include fichero { argumentos }>
    Incluye el _fichero_ para que sea procesado como un archivo .htex.
    Dicho archivo ser� utilizado tambi�n para calcular las dependencias
    (cu�l archivo necesita de qu� otro).
    Ejemplos:
      <!include cabecera.htex>
      <!include pie.htex es AR>

  <!macro nombre [ valor ]>
    Crea y/o modifica una macro con el _nombre_ y el _valor_ especificado.
    Si el _valor_ no se indica, la macro ser� eliminada de la memoria.
    La utilidad principal de las macros, es reemplazar un determinado
    texto (su _nombre_) por otro determinado texto (su _valor_).
    Ejemplo:
      <!macro hola chau>
      hola -> chau
      <!macro hola>
      hola -> hola

  <!macro-reset>
    Borra de la memoria cualquier macro que se haya creado con anterioridad
    a esta etiqueta.
    Ejemplo:
      <!macro a b>a<!macro-reset>a -> ba

  <!notdir fichero>
    Devuelve el nombre del _fichero_ a secas, es decir, le saca el directorio.
    Ejemplos:
      <!notdir src/foo.c> -> foo.c
      <!notdir a/b/foo.h> -> foo.h
      <!notdir hacks>     -> hacks

  <!shift palabra>
    Le quita el primer car�cter a la _palabra_.
    Ejemplos:
      <!shift hola> -> ola
      <!shift .txt> -> txt

  <!suffix fichero>
    Devuelve el sufijo del _fichero_, es decir, su extensi�n.
    Ejemplos:
      <!suffix "src/foo.c">   -> .c
      <!suffix "src-1.0/bar"> ->
      <!suffix "hacks">       ->

  <!tolower palabra [ comienzo [ final ] ]>
    Ver <!toupper>.

  <!toupper palabra [ comienzo [ final ] ]>
    Pasa parcial o completamente la _palabra_ a min�sculas o may�sculas
    dependiendo si especifica o no el _comienzo_ y/o el _final_. Si indica
    solamente el _comienzo_, el mismo deber� ser un �ndice (con base cero,
    es decir, 0 es la primer letra) del caracter que quiere convertir,
    mientras que si tambi�n indica el _final_, est� diciendo que quiere
    convertir todo un rango. Nota: valores negativos para _final_ indican
    valores desde el mismo final de la _palabra_.
    Ejemplos:
      <!tolower HOLA>      -> hola
      <!tolower HOLA 0>    -> hOLA
      <!toupper hola 1 2>  -> hOLa
      <!toupper hola 0 -2> -> HOLa

  <!version>
    Muestra la versi�n del programa con la que se est� procesando el fichero.
    Ejemplo:
      Est� p�gina fue creada con htmlex <!version>.

## Notas Importantes

`<!macro X>` borra la macro X, por lo que si utiliza `<!if X>`
la expresi�n d� verdadero (ya que se utiliza la cadena "X"
y no el valor de X).

`<!macro $X "">` le asigna a la macro X un valor nulo, esto es
necesario para que `<!if X>` d� falso (ya que en este caso, X
s� es reemplazado por su valor).

## Autor

David Capello <davidcapello@gmail.com>

Cualquier sugerencia, agradeciemiento, y, con preferencia, donaci�n ;-),
ser� bien recibida. Puede visitar http://dacap.com.ar/ para mayor
informaci�n.
