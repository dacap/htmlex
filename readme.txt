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

  htmlex interpreta las siguientes opciones en la línea de comandos:

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
  Here src.htex will be search just in the active directory,
  because dir/ is added once that orig.htex already is processed.
  Solution:
    ./htmlex -i dir -- src.htex
    ./htmlex dir/src.htex

  ./htmlex -c index.htex en English
  Here will be try of compile the files `index.htex', `en' and
  `English', instead of pass the arguments to `index.htex'.
  Solution:
    ./htmlex index.htex en English > index.html
    ./htmlex -a en English -- index.htex > index.html
    ./htmlex -a en English -c index.htex

FEATURES
========

  References
  ----------

  This notation will be used along of the definitions:
    expression            The _expression_ is obligatory.
    [ expression ]        The _expression_ is optional, could or not to be.
    { expression }        The _expression_ could not be or it could be repeat
                          one or more times.
    source -> result      Indicates that _results_ will be obtained
                          when the program will process the _source_ text.

  Tags
  ----

  Lists of extra tags which you can use in the files (by alphabetical
  sorting):

  <!args>
    Returns the quantity of arguments which was passed to the input file.
    Example:
      You input <!args> argument(s).

  <!arg1> ... <!argN>
    Retornan el valor del argumento indicado. Los argumentos se le pasan a
    los ficheros a través de la línea de comandos con la opción -a o por
    medio de la etiqueta <!include fichero arg1 ... argN>.
    Example:
       2nd argument = <!arg2>.

  <!basename file>
    Returns the base name of the _file_, in other words, removes its
    extension.
    Examples:
      <!basename src/foo.c>    -> src/foo
      <!basename src-1.0/bar>  -> src-1.0/bar
      <!basename hacks>        -> hacks

  <!chop word>
    Removes the last character of the _word_.
    Examples:
      <!chop hello>       -> hell
      <!chop directorio/> -> directorio

  <!clean word>
    Clean the _word_ removing the blank spaces (tabulators and line breaks)
    which it has in the beginning and end.
    Example:
      *<!clean " hola ">* -> *hola*

  <!dir file>
    Returns the directory of the _file_, in other words, deletes the name.
    Examples:
      <!dir src/foo.c> -> src/
      <!dir a/b/foo.h> -> a/b/
      <!dir hacks>     -> ./

  <!elif expression>
    See <!if>.

  <!else>
    See <!if>.

  <!exec file { arguments }>
    Execute the indicated _file_ passing the _arguments_ to it. That the
    command prints for the standard output (STDOUT), will be inserted
    such in the result.
    Examples:
      <!exec date>
      <!exec gcc --version>

  <!exec-proc file { arguments }>
    Equal to <!exec ...> just this will process the results as other
    htmlex file.
    Examples:
      <!exec-proc cat header.htex>
      <!exec-proc cat "a footer.htex">
      <!exec-proc cat <!find macros.htex>>

  <!fi>
    See <!if>.

  <!file-size file>
    Returns the size of the _file_ in bytes, kilo-bytes (k) or mega-bytes (M)
    depending which is the better for the "human sight".
    Example:
      image <!file-size image.jpg>

  <!find file>
    Intentará encontrar el _fichero_ en todas las ubicaciones posibles
    (en el directorio actual y en los directorios indicados con -i), y
    devolverá el nombre del mismo con la ruta de acceso correspondiente,
    en caso de que el _fichero_ no se pueda encontrar, no devuelve nada.
    Examples:
      <!if <!find index.en.html>>
        english
      </!if>

  <!if expression>
    Una de las más poderosas características que tiene htmlex con respecto
    al HTML convencional, es la posibilidad de controlar bloques de
    condiciones.

    Por medio de cuatro etiquetas podrá hacer la mayoría de operaciones de
    condición de cualquier otro lenguaje de programación. Aunque hoy en día,
    htmlex posee un pobre manejo de operaciones, en el futuro tal vez se
    intente mejorar este aspecto.

    La sintaxis general es la siguiente:

      <!if expresión> bloque
      { <!elif expresión> bloque }
      [ <!else> bloque ]
      <!fi>

    Los _bloques_ pueden ser cualquier texto (o simplemente nada), hasta
    pueden tener otras etiquetas a su vez, y por consiguiente, puede tener
    más etiquetas <!if> anidadas.

    En cuanto a las _expresiones_ para realizar las comparaciones, pueden
    tener estos formatos (en caso contrario siempre dará falso):

      FORMATO                VERDADERO SI...
      -------                ---------------
      número                 _número_ es distinto a cero
      ! número               _número_ es cero
      palabra                _palabra_ tiene un caracter como mínimo
      ! palabra              _palabra_ está vacía
      número1 < número2      _numero1_ es menor a _número2_
      número1 > número2      _numero1_ es mayor a _número2_
      número1 <= número2     _numero1_ es menor o igual a _número2_
      número1 >= número2     _numero1_ es mayor o igual a _número2_
      número1 == número2     _numero1_ es igual a _número2_
      número1 != número2     _numero1_ es diferente a _número2_
      palabra1 == palabra2   _palabra1_ es idéntica a _palabra2_
      palabra1 != palabra2   _palabra1_ difiere en un caracter a _palabra2_

    Los números se distinguen de las palabras cuando tiene alguno de
    los siguientes formatos:

      [1-9][0-9]*      un decimal
      0x[0-9a-fA-F]*   un hexadecimal
      0[0-7]           un octal

    Examples:
      <!if <!arg1> == --help>
        ¿Necesita ayuda?
      <!else>
        Pruebe con --help.
      <!fi>
      <!if <!tolower HOLA> == <!tolower Hola>>
        funciona
      <!fi>
      <!if>Por acá nunca pasará<!fi>

  <!include file { arguments }>
    Incluye el _fichero_ para que sea procesado como un archivo .htex.
    Dicho archivo será utilizado también para calcular las dependencias
    (cual archivo necesita de que otro). Los argumentos son opcionales,
    y si el no _fichero_ no se encuentra, el programa sale con error.
    Examples:
      <!include header.htex>
      <!include footer.htex en UK>

  <!macro name [ value ]>
    Creates and/or modifies an macro with the specify _name_ and _value_.
    If the _value_ is not indicated, the macro will be eliminated from the
    memory. The main utility of the macros, is replace a determined text
    (its _name_) for other determined text (its _value_).
    Example:
      <!macro hi bye>
      hi -> bye
      <!macro hi>
      hi -> hi

  <!macro-reset>
    Removes from the memory any macro which was created previously to this
    tag.
    Example:
      <!macro a b>a<!macro-reset>a -> ba

  <!notdir file>
    Returns just the name of the _file_, in other words, removes its
    directory.
    Examples:
      <!notdir src/foo.c> -> foo.c
      <!notdir a/b/foo.h> -> foo.h
      <!notdir hacks>     -> hacks

  <!shift word>
    Removes the first character of the _word_.
    Examples:
      <!shift your> -> our
      <!shift .txt> -> txt

  <!suffix file>
    Returns the suffix of the _file_, in other words, its extension.
    Examples:
      <!suffix "src/foo.c">   -> .c
      <!suffix "src-1.0/bar"> ->
      <!suffix "hacks">       ->

  <!tolower word [ beginning [ final ] ]>
    See <!toupper>.

  <!toupper word [ beginning [ final ] ]>
    Converts partial or completelly the _word_ to lower or upper case
    depending if you specify or not the _beginning_ and/or the _final_.
    If you indicate just the _beginning_, the same should be an index
    (with zero base, in other words, 0 is the first letter) of that
    character which you want convert, while if you also indicate the
    _final_, you are saying which you want convert an entire range.
    Note: negative values for the _final_ indicates values from the
    end of the _word_.
    Examples:
      <!tolower HELLO>      -> hello
      <!tolower HELLO 0>    -> hELLO
      <!toupper hello 1 2>  -> hELlo
      <!toupper hello 0 -2> -> HELLo

  <!version>
    Shows the program version which is processing the file.
    Example:
      This page was created with htmlex <!version>.

BUGS
====

  Report `bugs' to <dacap@users.sourceforge.net>.
  Nowadays I don't have knowledge about some error.

WORK TO DO
==========

  - Translate the file `cambios.txt' to `changes.txt'.
  - Improve this `readme.txt' file, the english isn't understandable
    and some parts are in spanish yet.

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

