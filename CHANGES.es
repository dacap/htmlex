2001-08-11  David A. Capello

	* Se elimin¢ la posibilidad del fichero ejecutable en la
	  primer l¡nea (ya no m s "#!").

2001-07-30  David A. Capello

	* Arreglado un error con los <if>, ahora s¡ funcionan :-)
	* Nuevo ejemplo `expr.htex'.
	* Ya se soportan <if> anidados.

2001-07-29  David A. Capello

	* Dos primeros ejemplos (en examples/): `args.htex' y `if.htex'.
	* Hoy mismo, se elimin¢ los dos modificadores `+' y `-' y se
	  optimiz¢ el procesamiento de las etiquetas. Por ejemplo ahora
	  solamente en algunas propiedades de determinadas etiquetas
	  se toman como expresiones y se reemplazan macros.
	* Se agreg¢ el nuevo modificador `+' en los nombres de propiedades
	  (para interpretar valores como expresiones).
	* Se optimiz¢ el proceso de etiquetas.

2001-07-28  David A. Capello

	* Agregado `expr.c' y las etiquetas <if>, <elif>, <else> e </if>.
	* Mejorado el aspecto de los documentos (layouts).

2001-07-27  David A. Capello

	* Entre hoy y ayer: eliminado por completo los BUFFERs, ahora
	  todas las operaciones se hacen mediante STREAMs.
	* Nuevo `memory.[ch]': para manejar la memoria, adem s utilizo
	  un contador de llamadas a `alloc' y `free' para saber si hay
	  algo que no se libera al finalizar el programa (con esto pude
	  detectar un peque¤o error en la funci¢n `create_stream').

2001-07-26  David A. Capello

	* Agregado `macro_allowed': ahora, en cada propiedad de una etiqueta
	  se puede especificar si se quiere o no procesar macros en su valor.
	* Corregido `process_char': antes, si una etiqueta HTML normal
	  ten¡a dentro una etiqueta HTMLex esta £ltima no era procesada
	  (con esto tambi‚n se elimin¢ el argumento `tags_allowed' de
	  la funci¢n en cuesti¢n).

2001-07-25  David A. Capello

	* Agregado el soporte para generar dependencias.
	* Se arregl¢ un error que ocurr¡a al llegar el final del fichero:
	  la £ltima l¡nea no se imprim¡a.
	* Soporte para utilizar macros y etiquetas en los valores de
	  las propiedades.

2001-07-24  David A. Capello

	* Nuevo stream.c: ahora con los STREAM puedo manejar un fichero
	  o un bloque de memoria de la misma forma (putc, getc, puts, gets,
	  seek, tell, eof). Esto podr¡a llegar a significar el final para
	  los BUFFERs?

2001-07-20  David A. Capello

	* Agregado --compress: para "comprimir" un poco la distribuci¢n
	  de los caracteres `\n' (no permite dos seguidos).
	* Arreglada la detecci¢n de comentarios HTML: ahora se saltea
	  completamente todo el comentario (antes se iva todo al demonio).
	* Nueva documentaci¢n (doc/): license.es.htex y bnf.es.htex son los
	  dos m s importantes (luego index*.htex).
	* M s comentarios modificados.
	* Se arregl¢ `process_char': en la *detecci¢n* de etiquetas y el
	  salto de los valores de las propiedades entre comillas ="...",
	  ya se agreg¢ la posibilidad de seguir lleyendo l¡neas hasta
	  encontrar el *REAL* cierre de la comilla.

2001-07-19  David A. Capello

	* Modifiqu‚ varios comentarios que quedaron viejos (hasta
	  comentarios que hice ESTE MISMO d¡a).
	* Se separaron m s funciones de htmlex.c: en este caso, las funciones
	  para etiquetas TAG y TAG_PROP pasaron al archivo tag.[ch].
	* Definitivamente arreglado todos los errores de recursividad,
	  agregado buffer.c y optimizadas varias partes de c¢digo ilegibles.
	* Se agreg¢ la lectura de l¡neas m£ltiples: ahora ya se pueden
	  crear macros que no necesariamente comiencen y terminen en la misma
	  l¡nea.
	* Nueva lectura de cualquier l¡nea: ahora ya se soportan l¡neas de un
	  tama¤o mayor a 256 caracteres, en realidad y te¢ricamente, se puede
	  leer *CUALQUIER* l¡nea.
	* Nuevo FAQ.es: por ahora un par de preguntas y respuesta, luego
	  lo reviso.

2001-07-18  David A. Capello

	* Separ‚ un poco el trabajo de `htmlex.c' en: `macro.[ch]',
	  `stringex.[ch]' y `types.h',
	* Nueva posibilidad de imprimir la cabecera HTTP (alg£n d¡a se
	  llegar  a la altura de PHP? :-)
	* Arreglado y optimizado el manejo de las MACROS: mejores y m s
	  funciones para las mismas.
	* Se agreg¢ un archivo `bnf.txt' con la explicaci¢n BNF de HTMLex.
	* Corregidos algunos errores con las macros y la detecci¢n de
	  propiedades en las etiquetas.
	* Arregl‚ un grave error en `get_macro()'.
	* Se elimin¢ el error de recursividad infinita con las macros
	  (simple, doble y m£ltiple): por medio de la variable `used' en
	  cada una.
	* Los nombres de las macros ya son reemplazados por sus
	  correspondientes valores en el proceso l¡nea por l¡nea principal,
	  todav¡a me falta en los argumentos de las etiquetas.

2001-07-17  David A. Capello

	* Etiquetas <macro [name [value]|reset]> y <include>
	* Tambi‚n ya se soportan las etiquetas <argc> y <argv*>
	* El programa ya lee ficheros y hasta puede incluir otros, acepta
	  los par metros -I y -v. Por ahora trabaja l¡nea por l¡nea.
	* Versi¢n inicial: htmlex 0.1



