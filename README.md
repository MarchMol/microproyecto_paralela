## Requerimientos
gcc, OpenMP y python

## Setup
#### Compliacion
Debido al uso de openmo, la compilacion de los codigos en c cambia, ahora con el uso de la bandera -fopenmp
```
gcc -fopenmp ./scripts/<name>.c -o <executable>
```
### Ejecucion
Los programas tanto secuencial como paralelizado, debido a que implementan el mismo algoritmo, poseen los mismos argumentos de entrada
```
./ecosistema <input.txt> <max_ticks> <verbose>
```
- Input: para el uso del programa se requiere de una condicion inicial presente en un archivo .txt (preferiblemente en la carpeta data) y debe poseer el siguiente formato:
  - '_' -> Espacio libre
  - 'p' o 'P' -> Planta
  - 'h' o 'H' -> Herviboro
  - 'c' o 'C' -> Carnivoro

- Max Ticks: representan la cantidad maxima de simulaciones
- Verboes: similar a su uso en modelos de redes neuronales, significa que tantos mensajes imprimira el modelo
  - 0 -> No imprimira nada
  - 1 -> Imprimira solo el tiempo de ejecucion
  - 2 -> imprimira los estados del ecosistema en cada tick, el tiempo de ejecucion, y la completacion de funciones auxiliares como el reconocimiento del tamaño de la entrada.

### Testing
Para correr los 2 archivos de testing se requiere de la siguiente sintaxis

- Generador.py:
```
python3 generador.py <ancho> <alto> <path>
```
En donde el ancho y alto representan las dimensiones del ecosistema, y path la direccion y nombre con el cual almacear el contenido generado

- Tester.py
```
python3 tester.py
```
Este correra sin argumentos o parametros y solo imprimira los tests y el tiempo obtenido en cada uno
