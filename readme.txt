htmlex - a powerful hypertext markup language preprocessor (HTML)
Copyright (C) 2001 by David A. Capello

LICENSE
=======

  htmlex is free software; you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation; either version 2 of the License, or (at your option) any later
  version.

  This program is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
  details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc., 59 Temple
  Place, Suite 330, Boston, MA  02111-1307  USA

INTRODUCTION
============

  htmlex is a small but powerful HTML preprocesor, which you can have some
  advantages relative to the conventional format of the .html files. Besides,
  as htmlex is fully compatible with the standard HTML syntax, you can use
  any HTML file that you have, and rename it to .htex.

OPTIONS
=======

  htmlex interpreta las siguientes opciones en la l�nea de comandos:

    -c   compiles all next files (use the active output file names
         or generate they as `file.htex' to `file.html')
    -o   adds output files (use it before the `-c')
    -a   adds arguments for the input files (use it before the `-c')
    -i   adds all next arguments to inclusion paths (for search files)
    -d   calculates dependencies of the input files (output to STDOUT)
    -h   shows this help and exit
    -v   shows the htmlex version and exit
    --   stops the -c, -o and -i arguments

EXECUTION
=========

  References
  ----------

  STDIN      Standard input: from the keyboard or from file.

  STDOUT     Standard output: to the screen or to file.

  Common behavior
  ---------------

  ./htmlex
  Process STDIN and leave the results in STDOUT.

  ./htmlex file arguments...
  Process the `file' and leave the results in STDOUT.

  ./htmlex -c files...
  Process the `files' and leave the results in `files.html'.

  ./htmlex -o destinations... -c files...
  Process the `files' and leave the results in the `destinations'.

  Some examples of equivalence
  ----------------------------

  ./htmlex file.htex arg1 arg2 > file.html
  ./htmlex -a arg1 arg2 -c file.htex
  ./htmlex -a arg1 arg2 -o file.html -c file.htex
  ./htmlex -a arg1 arg2 < file.htex > file.html
  ./htmlex -a arg1 arg2 -o file.html < file.htex

  Common errors
  -------------

  ./htmlex -c src.htex -o dest.html
  Here the result remain in src.html, and not in dest.html.
  Solution:
    ./htmlex -o dest.html -c src.htex

  ./htmlex -i examples src.htex > src.html
  Here the program wait the input from STDIN, because src.htex
  count as other path for -i.
  Solution:
    ./htmlex -i examples -- src.htex > src.html
    ./htmlex -i examples -c src.htex

  ./htmlex src.htex -i dir
  Aqu� src.htex se buscar� solamente en el directorio actual,
  ya que dir/ se agrega una vez que orig.htex ya es procesado.
  Solution:
    ./htmlex -i dir -- src.htex
    ./htmlex dir/src.htex

  ./htmlex -c index.htex es Espa�ol
  Aqu� se intentar� compilar los ficheros `index.htex', `es' y
  `Espa�ol', en vez de pasarle los argumentos al fichero `index.htex'.
  Solution:
    ./htmlex index.htex es Espa�ol > index.html
    ./htmlex -a es Espa�ol -- index.htex > index.html
    ./htmlex -a es Espa�ol -c index.htex

FEATURES
========

  References
  ----------

  Esta notaci�n se usar� a lo largo de las definiciones:
    expresi�n             La _expresi�n_ es obligatoria.
    [ expresi�n ]         La _expresi�n_ es opcional, puede o no estar.
    { expresi�n }         La _expresi�n_ puede no estar o puede repetirse
                          una o m�s veces.
    origen -> resultado   Indica que _resultado_ ser� obtenido al procesar
                          mediante el programa el texto de _origen_.

  Tags
  ----

  Lista de las etiquetas extras que puede utilizar en los ficheros
  (por orden alfab�tico):

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
      <!chop hola>        -> hol
      <!chop directorio/> -> directorio

  <!clean palabra>
    Limpia la _palabra_ quit�ndole los espacios (tabuladores y retornos
    de l�neas) que tenga al inicio y al final.
    Ejemplo:
      *<!clean " hola ">* -> *hola*

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

  <!exec fichero { argumentos }>
    Ejecuta el _fichero_ indicado pas�ndole los _argumentos_. Lo que el
    comando imprima por la salida estandard (STDOUT), ser� insertado tal
    cual en el resultado. Algo a tomar en cuenta, es que todos los
    argumentos (menos el fichero claro est�), ser�n encerrados entre
    comillas dobles antes de pasarle el comando a la shell.
    Ejemplos:
      <!exec date>
      <!exec gcc --version>

  <!exec-proc fichero { argumentos }>
    Igual que <!exec ...> solamente que �ste procesar� los resultados
    como a otro fichero htmlex.
    Ejemplos:
      <!exec-proc cat cabecera.htex>
      <!exec-proc cat "pie de pagina.htex">
      <!exec-proc cat <!include-path macros.htex>>

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
      </!if>

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
      n�mero1 < n�mero2      _numero1_ es menor a _n�mero2_
      n�mero1 > n�mero2      _numero1_ es mayor a _n�mero2_
      n�mero1 <= n�mero2     _numero1_ es menor o igual a _n�mero2_
      n�mero1 >= n�mero2     _numero1_ es mayor o igual a _n�mero2_
      n�mero1 == n�mero2     _numero1_ es igual a _n�mero2_
      n�mero1 != n�mero2     _numero1_ es diferente a _n�mero2_
      palabra1 == palabra2   _palabra1_ es id�ntica a _palabra2_
      palabra1 != palabra2   _palabra1_ difiere en un caracter a _palabra2_

    Los n�meros se distinguen de las palabras cuando tiene alguno de
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

  <!include fichero { argumentos }>
    Incluye el _fichero_ para que sea procesado como un archivo .htex.
    Dicho archivo ser� utilizado tambi�n para calcular las dependencias
    (cual archivo necesita de que otro). Los argumentos son opcionales,
    y si el no _fichero_ no se encuentra, el programa sale con error.
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
    respectivamente dependiendo si especifica o no el _comienzo_ y/o
    el _final_. Si indica solamente el _comienzo_, el mismo deber� ser
    un �ndice (con base cero, es decir, 0 es la primer letra) del caracter
    que quiere convertir, mientras que si tambi�n indica el _final_, est�
    diciendo que quiere convertir todo un rango. Nota: valores negativos
    para _final_ indican valores desde el mismo final de la _palabra_.
    Ejemplos:
      <!tolower HOLA>      -> hola
      <!tolower HOLA 0>    -> hOLA
      <!toupper hola 1 2>  -> hOLa
      <!toupper hola 0 -2> -> HOLa

  <!version>
    Muestra la versi�n del programa con la que se est� procesando el fichero.
    Ejemplo:
      Est� p�gina fue creada con htmlex <!version>.

BUGS
====

  Report `bugs' to <dacap@users.sourceforge.net>.
  Nowadays I don't have knowledge about some error.

WORK TO DO
==========

  - Tranlate the file `cambios.txt' to `changes.txt'.

UPDATES
=======

  To obtain the last information about htmlex, you can go to:

    http://htmlex.sourceforge.net

AUTHOR
======

  David A. Capello <dacap@users.sourceforge.net>.

  Any suggestion, thankfulness, and, with preference, donation ;-),
  will be welcome. You can visit http://www.davidcapello.com.ar for
  more information.
  
