# Megatron 3000

**Megatron 3000** es un sistema de gestión de bases de datos simple que nos permite realizar operaciones básicas sobre datos almacenados en archivos,utilizando una sintaxis similar a la de SQL, resaltar que este proyecto fue trabajado en C++, .

## Tareas

### 1. Lectura de Datos (Leonardo Gaona)
- **Lectura del esquema**: Interpreta el esquema de las tablas a partir de `scheme.txt` para entender las columnas y tipos de datos.
- **Carga de datos**: Lee los datos de las tablas almacenadas en archivos `.txt`.

### 2. Selección de Tablas (Kevin Rodriguez)
- **Selección completa (*)**: Para  llevar a cabo operaciones como `SELECT *`, obteniendo todas las columnas de una tabla, utilizamos la función **select()** que nos permite imprimir todas las columnas de una tabla en específico, algo a resaltar es que a partir de esto punto para llevara a cabo las consultas se utilizan '**expresiones regulares** para establecer rápidamente un patrón que sigue la query del usuario.
- **Selección específica por columnas**: Esto se desarrollo posteriormente sobre la misma funcion **select()** y la actualizamos para que permita seleccionar columnas particulares con consultas como `SELECT columna1, columna2 FROM tabla`.

### 3. Filtrado con `WHERE` (Kevin Rodriguez)
- **Cláusula WHERE**: Para llevar a cabo el filtrado de datos específicos implementé la función `cumpleCondicion()` que recibe la condición que se pide cumplir, esta función se apoya de la funcion `evaluarCondicion()` donde se revisa la condición es decir se lleva a cabo las operaciones =,<,>,<=,>=,!= actualmente, de esta manera se simula o se lleva a cabo el`WHERE` .
- **Funciones clave**: `cumpleCondicion()` para evaluar condiciones y `Select()` para ejecutar consultas con filtrado.

### 4. Unión de Tablas (Fabricio Villantoy)
- **Soporte para JOIN**: Permite unir dos o más tablas basadas en columnas compartidas, similar a un `INNER JOIN` en SQL.

### 5. Selección después de un JOIN (Fabricio Villantoy)
- **Filtrado post-unión**: Realiza selecciones con condiciones después de unir tablas.

### 6. Actualización de Valores (Leonardo Gaona)
- **Actualización de datos**: Modifica valores en las tablas existentes a través de consultas `UPDATE`.

### 7. Guardar resultados (Fabricio Villantoy)
- **Persistencia de resultados**: Guarda el resultado de una consulta `SELECT` o `JOIN` en un archivo de salida.

## Compilación y Ejecución

El proyecto fue trabajo en visual studio, pero cuenta con un Makefile para compilarlo.


## Cosas a corregir o ideas para el futuro