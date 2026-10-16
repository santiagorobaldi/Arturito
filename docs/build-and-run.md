# Preparar la computadora y cargar un programa en Arturito

Esta guía comienza con una computadora Windows sin herramientas del proyecto instaladas. Su objetivo es que puedas elegir una de las plataformas existentes, entender qué necesitás descargar y llegar a compilar y cargar un programa.

La organización interna del firmware se explica en los otros documentos de `docs/`. Acá seguimos el recorrido desde los archivos del proyecto hasta la placa.

## 1. Por dónde empezar

Primero elegí la placa que vas a usar:

- **ESP32:** seguí la preparación común y después el camino ESP32.
- **EDU-CIAA NXP:** seguí la preparación común y después el camino EDU-CIAA.

No necesitás instalar las herramientas de ambas plataformas. Cada procesador necesita herramientas que puedan generar un programa para él.

Los caminos de esta guía corresponden a las plataformas que el proyecto ya contempla. Una placa nueva necesita su propia configuración; no alcanza con elegirla de una lista.

## 2. Qué vamos a hacer

El recorrido es el mismo para ambas placas:

```text
Obtener los archivos del proyecto
    ↓
Preparar las herramientas de la computadora
    ↓
Comprobar que esas herramientas están disponibles
    ↓
Elegir el programa y la placa en la configuración del proyecto
    ↓
Compilar el programa
    ↓
Conectar la placa y cargar el resultado
```

**Compilar** significa transformar los archivos C/C++ y las bibliotecas utilizadas en un programa para el procesador de la placa. La construcción también une las partes compiladas; ese paso se llama **enlace**. El resultado que ejecutará la placa se llama **firmware**.

**Cargar** significa escribir ese firmware en la memoria de la placa. Compilar y cargar son pasos distintos: podés compilar sin tener la placa conectada.

## 3. Qué archivos necesitás obtener

Obtené una copia completa de Arturito, por ejemplo mediante un ZIP proporcionado por el equipo. Extraela en una ubicación conocida. En esta guía usaremos como ejemplo:

```text
C:\Arturito
```

Si la guardaste en otro lugar, reemplazá esa ruta en los comandos.

### 3.1. Mapa de carpetas del repositorio

El **repositorio** es la carpeta completa de Arturito. El **workspace de VS Code** es una selección de carpetas de ese repositorio que el editor muestra juntas. Abrir el workspace no instala dependencias ni elige qué programa se compila.

Este mapa muestra las partes importantes para orientarte. Describe la organización actual y el tipo de contenido que corresponde a cada ubicación; no propone todavía mover ni renombrar archivos.

```text
Arturito/
├── README.md                  Presentación y enlaces a la documentación
├── Arturito.code-workspace    Carpetas que se muestran juntas en VS Code
├── .gitignore                 Reglas para excluir archivos del seguimiento de Git
├── shared/                    Código del firmware compartido entre plataformas
│   ├── config/                Parámetros comunes del programa
│   ├── layer1_hal/            Cabeceras de las interfaces de acceso al hardware
│   ├── layer2_drivers/        Código compartido para utilizar dispositivos
│   └── layer3_app/            Código compartido de aplicación
├── ESP32/                     Construcción y código específico de ESP32
│   ├── platformio.ini         Selección de placa, fuentes y dependencias
│   ├── src/                   Fuentes específicas de esta plataforma
│   │   ├── main.cpp           Entrada al programa
│   │   ├── layer1_hal/        Implementaciones de acceso al hardware ESP32
│   │   └── tests/             Programas alternativos de diagnóstico
│   └── .pio/                  Resultados y dependencias generados por PlatformIO
├── EDU-CIAA/                  Construcción y soporte para EDU-CIAA
│   ├── Makefile               Instrucciones para construir y cargar
│   ├── board.mk               Selección de placa
│   ├── program.mk             Selección de programa
│   ├── rover/                 Programa principal para esta plataforma
│   │   ├── config.mk          Fuentes, opciones y dependencias del programa
│   │   ├── inc/               Cabeceras y configuración propias del programa
│   │   ├── src/               Entrada y código específico de EDU-CIAA
│   │   │   └── layer1_hal/    Implementaciones de acceso al hardware EDU-CIAA
│   │   └── out/               Resultados generados por la construcción
│   ├── libs/                  Bibliotecas y soporte externo incluidos
│   ├── scripts/               Archivos auxiliares de construcción y programación
│   └── test/                  Programas independientes de diagnóstico
├── Processing/                Programas de visualización que corren en la PC
├── KiCad/                     Proyecto electrónico, esquemático y PCB
├── LatexSanti/                Informes y documentos académicos en LaTeX
└── docs/                      Documentación técnica organizada por tema
    ├── decisions/             Registros de decisiones y sus motivos
    └── images/                Imágenes utilizadas por la documentación
```

`.pio/` y `out/` pueden no existir al descargar el proyecto: las herramientas las crean al construirlo. No son carpetas donde escribir la aplicación.

### 3.2. Qué contenido corresponde a cada grupo

| Ubicación | Qué debería contener | Qué tener presente al trabajar |
|---|---|---|
| `shared/` | Código del proyecto que se busca reutilizar entre plataformas, sus interfaces y parámetros comunes | Evitar incorporar aquí dependencias directas de una sola placa. La separación de responsabilidades se explica en [Arquitectura de software](software-architecture.md) |
| `ESP32/src/` | Entrada al programa, adaptaciones ESP32 y diagnósticos de esa plataforma | El código compartido se utiliza desde shared; no necesita copiarse aquí |
| `EDU-CIAA/rover/` | Entrada, adaptaciones y configuración del programa para EDU-CIAA | `inc/` contiene actualmente FreeRTOSConfig.h; `src/` contiene main.c y las implementaciones HAL |
| `EDU-CIAA/libs/` | Bibliotecas y soporte externo, con sus archivos de integración | Que estén dentro del repositorio no las convierte en código propio del robot. Conservar licencias y distinguir cambios locales al modificarlas |
| `EDU-CIAA/scripts/` | Configuraciones de programación y utilidades auxiliares de la infraestructura CIAA | Algunas son heredadas; su presencia no significa que todas se utilicen en la construcción habitual |
| `ESP32/src/tests/` y `EDU-CIAA/test/` | Programas para diagnosticar componentes por separado | Son programas alternativos. Cargarlos reemplaza la aplicación anterior; el nombre tests no implica pruebas automáticas |
| `Processing/` | Código que recibe y representa información en la computadora | Se ejecuta por separado; no se incorpora al firmware |
| `KiCad/` | Archivos del diseño electrónico y bibliotecas de ese diseño | No interviene en la compilación del firmware |
| `LatexSanti/` | Fuentes de informes académicos y documentación complementaria | No interviene en la compilación del firmware |
| `docs/` | Guías, descripción del sistema, arquitectura, hardware, requisitos y estado | Enlazar al documento que desarrolla cada tema en lugar de repetirlo |
| `.pio/` y `out/` | Dependencias administradas y productos de construcción | Cambiar la fuente o configuración original y reconstruir; no editar el resultado para cambiar el programa |
| `.vscode/`, cuando exista | Configuración local o compartida del editor | No sustituye a platformio.ini ni a los archivos de construcción de Make |

Dentro de `EDU-CIAA/libs/` hay seis grupos principales:

| Carpeta | Qué aporta |
|---|---|
| `lpc_open/` | Soporte del microcontrolador LPC43xx y la placa, incluyendo preparación del arranque |
| `sapi/` | Funciones de CIAA para utilizar periféricos y dispositivos |
| `freertos/` | Biblioteca que permite ejecutar y coordinar tareas |
| `cmsis_core/` | Definiciones y soporte del núcleo del procesador ARM |
| `cmsis_dsp/` | Funciones de procesamiento digital de señales; su presencia no implica que la aplicación las utilice |
| `sys_newlib/` | Adaptación de servicios de la biblioteca estándar de C para el entorno embebido |

En ESP32, buena parte del soporte externo se obtiene mediante PlatformIO. Sus paquetes generales están fuera del repositorio, normalmente en `.platformio` dentro del perfil del usuario; las bibliotecas descargadas para el proyecto se encuentran bajo `ESP32/.pio/`. Más adelante se explica qué descarga PlatformIO y por qué.

### 3.3. Qué muestra el workspace de VS Code

El archivo `Arturito.code-workspace` abre actualmente estas cinco carpetas:

| Nombre visible en VS Code | Carpeta real |
|---|---|
| ESP32 - Firmware activo | `ESP32/` |
| EDU-CIAA - Port futuro | `EDU-CIAA/` |
| Shared - Firmware portable | `shared/` |
| Processing - Visualización V0 | `Processing/` |
| Documentación | `docs/` |

Los nombres visibles son etiquetas del editor: no seleccionan herramientas ni demuestran el estado de una plataforma. Por ejemplo, “Port futuro” no significa que la carpeta EDU-CIAA carezca de código.

`KiCad/`, `LatexSanti/` y los archivos de la raíz no aparecen como entradas independientes en ese workspace. Para ver todo el repositorio, abrir la carpeta Arturito completa en VS Code.

Las rutas del workspace están escritas en minúsculas, mientras algunas carpetas reales usan mayúsculas. Esto normalmente funciona en Windows, pero debe alinearse si se prepara el proyecto para un sistema que distinga mayúsculas y minúsculas.

### 3.4. Relación con las herramientas descargadas

La carpeta `Software-CIAA` que puede acompañar a Arturito es un paquete de herramientas para la computadora, no una parte de la aplicación. Puede estar fuera del repositorio. Para utilizarla se prepara la variable PATH, como se explica en el camino EDU-CIAA.

Si la carpeta de trabajo también contiene otra copia de firmware CIAA, esa copia no sustituye automáticamente a `Arturito/EDU-CIAA`. Los comandos de esta guía construyen los archivos que están dentro de Arturito.

### 3.5. Antes de reorganizar archivos

Conservá `shared`, `ESP32` y `EDU-CIAA` como carpetas hermanas mientras uses la configuración actual: hay rutas relativas entre ellas. Una ruta relativa describe cómo llegar a un archivo desde otra carpeta, en lugar de indicar una ubicación completa como C:\Arturito.

Para una refactorización posterior, comprobar cada movimiento contra los lugares que lo utilizan:

- Si se mueve o agrega código ESP32, revisar la selección de fuentes y cabeceras en `platformio.ini`.
- Si se mueve o agrega código EDU-CIAA, revisar el config.mk del programa y las reglas correspondientes de Make.
- Si se mueve código compartido, revisar las referencias desde ambas plataformas y los includes del código.
- Si se cambia la carpeta de un programa EDU-CIAA, revisar su selección en `program.mk`.
- Si se cambian carpetas visibles, actualizar `Arturito.code-workspace` y los enlaces de documentación.

La configuración de construcción debe continuar seleccionando el código correcto después del cambio. Actualizar solamente el explorador de VS Code no actualiza esas referencias.

Hay tres clases de contenido que conviene distinguir:

| Contenido | Qué hacés con él |
|---|---|
| Código de aplicación del proyecto | Lo obtenés con Arturito y lo compilás |
| Bibliotecas externas utilizadas por ese código | Algunas ya vienen dentro del proyecto; otras las descargará la herramienta de construcción |
| Herramientas externas | Las preparás en la computadora para compilar y cargar |

Una biblioteca externa es código que el proyecto utiliza. Una herramienta externa es un programa que ejecutás en la computadora para trabajar con ese código. Tener una biblioteca descargada no significa que todos los programas la utilicen: eso lo decide la configuración de construcción.

Git permite descargar y llevar el historial del código, pero no es necesario instalarlo si recibiste el proyecto completo por ZIP. El equipo todavía debe incorporar a esta guía el enlace concreto desde el cual obtener Arturito.

## 4. Cómo leer los comandos de esta guía

Los bloques con comandos se ejecutan en una **terminal**: una ventana donde escribís instrucciones para la computadora. Para EDU-CIAA usaremos PowerShell, disponible en Windows. Para ESP32 usaremos la terminal preparada por PlatformIO.

`Set-Location` cambia la carpeta desde la cual estás trabajando. Por ejemplo:

```powershell
Set-Location 'C:\Arturito'
```

Ejecutá un comando por vez y revisá su resultado antes de seguir. Si falla, el mensaje permite identificar qué paso no se completó.

# Camino A: ESP32

## A1. Qué necesitás y quién lo prepara

Vamos a usar **PlatformIO**, una herramienta que organiza la compilación y carga de proyectos para microcontroladores. También descarga las herramientas y bibliotecas que el proyecto declara necesitar.

La usaremos dentro de **VS Code**, un editor de archivos. VS Code te permite trabajar con el código; PlatformIO realiza la construcción y carga mediante las herramientas correspondientes.

| Elemento | Para qué se usa | Qué tenés que hacer |
|---|---|---|
| VS Code | Abrir y editar el proyecto | Instalarlo |
| Extensión PlatformIO IDE | Agregar a VS Code la interfaz de PlatformIO | Instalarla desde VS Code |
| PlatformIO Core | Ejecutar las operaciones de PlatformIO | La extensión lo prepara |
| Soporte espressif32 | Indicar a PlatformIO cómo trabajar con ESP32 | PlatformIO lo descarga según el proyecto |
| Compilador para ESP32 | Convertir C/C++ en código para ese procesador | PlatformIO lo descarga |
| Framework Arduino para ESP32 | Aportar código de soporte y funciones que usa este proyecto | PlatformIO lo descarga |
| Bibliotecas externas declaradas | Aportar funciones adicionales utilizadas por el programa | PlatformIO las descarga cuando corresponde |
| esptool | Escribir el programa compilado en ESP32 | PlatformIO lo obtiene y ejecuta durante la carga |

Un **framework** es un conjunto de código de soporte y bibliotecas sobre el cual se construye la aplicación. En este proyecto ESP32 se utiliza Arduino. No necesitás instalar el editor Arduino IDE para seguir este camino.

## A2. Instalar VS Code y PlatformIO

1. Descargar e instalar [VS Code para Windows](https://code.visualstudio.com/download).
2. Abrir VS Code.
3. Abrir la sección Extensions o Extensiones.
4. Buscar **PlatformIO IDE**, identificador `platformio.platformio-ide`, e instalarla.
5. Esperar a que termine de preparar sus componentes. Reiniciar VS Code si lo solicita.
6. En VS Code, abrir la carpeta `C:\Arturito\ESP32` mediante Abrir carpeta.

PlatformIO Core viene incluido en la preparación de PlatformIO IDE. La [documentación oficial de instalación](https://docs.platformio.org/en/latest/core/installation/) describe esta relación.

## A3. Entender la configuración antes de compilar

Dentro de `ESP32` está `platformio.ini`. Este archivo le indica a PlatformIO qué debe construir y con qué dependencias. No es un archivo de configuración personal del editor.

| Dato del archivo | Qué indica |
|---|---|
| `platform` | El soporte de la familia de microcontroladores; aquí espressif32 |
| `board` | El modelo de placa usado para configurar la construcción; aquí esp32dev |
| `framework` | El código de soporte elegido; aquí Arduino |
| `build_src_filter` | Qué archivos de código se incluyen en el programa |
| `build_flags` | Opciones adicionales, como rutas de cabeceras y definiciones que seleccionan código |
| `lib_deps` | Bibliotecas externas que PlatformIO debe obtener |

Un **entorno de PlatformIO** es una configuración de construcción identificada por un nombre. No es una variable de entorno de Windows. El proyecto puede tener varios para construir programas diferentes. Actualmente el programa principal está en el entorno `esp32dev`.

Antes de cargar, confirmar que la placa física coincide con la configuración. El README menciona “NodeMCU V3”, pero ese nombre no alcanza para confirmar que sea el ESP32 configurado. Este dato debe resolverlo el equipo.

Para un programa nuevo, se necesita una configuración que seleccione sus fuentes y dependencias. En los comandos se elige el nombre de esa configuración; el comando por sí solo no decide qué código es tu aplicación.

## A4. Comprobar PlatformIO

Abrir la terminal de PlatformIO desde su interfaz en VS Code o mediante el comando **PlatformIO: New Terminal**. Ejecutar:

```powershell
pio --version
```

`pio` es el nombre del comando de PlatformIO. Si muestra una versión, está disponible en esa terminal. Si Windows dice que no reconoce el comando, comprobar que estés usando la terminal de PlatformIO y que haya terminado su instalación.

## A5. Compilar

En esa terminal:

```powershell
Set-Location 'C:\Arturito\ESP32'
pio run -e esp32dev
```

`run` inicia la construcción. `-e esp32dev` elige el entorno llamado esp32dev.

La primera vez puede demorar porque descarga los paquetes que faltan. Necesitás Internet. No tenés que descargar manualmente cada compilador o biblioteca de la tabla A1.

Los paquetes generales se guardan normalmente bajo `.platformio` en tu perfil de usuario. Los archivos generados para este proyecto se guardan dentro de `ESP32/.pio/`. Son resultados y paquetes administrados por la herramienta; no necesitás editarlos para modificar la aplicación.

Al finalizar, comprobar que PlatformIO informe éxito. Si informa un error, resolverlo antes de intentar la carga.

## A6. Conectar e identificar la placa

Conectar la placa mediante un cable USB de datos. Ejecutar:

```powershell
pio device list
```

Este comando muestra los dispositivos disponibles. Identificar el puerto asignado a la placa, por ejemplo `COM7`. Podés comparar la lista antes y después de conectarla.

Si Windows no reconoce el dispositivo, puede faltar un **controlador USB**: software de Windows que permite acceder al componente USB de la placa. El controlador depende del dispositivo real; identificarlo en el Administrador de dispositivos e instalar el del fabricante correspondiente. Falta confirmar ese componente para poder dar una descarga única en esta guía.

## A7. Cargar el programa

Reemplazar `COM7` por el puerto identificado:

```powershell
pio run -e esp32dev -t upload --upload-port COM7
```

`-t upload` pide cargar el resultado. `--upload-port` indica qué puerto utilizar. PlatformIO organiza la construcción necesaria y ejecuta la herramienta de carga.

Comprobar que termine correctamente. Cada carga reemplaza el programa anterior de la placa.

## A8. Observar la ejecución, si el programa utiliza serie

Para leer los mensajes que envía el programa:

```powershell
pio device monitor --port COM7 -b 115200
```

El **monitor serie** es un programa de la computadora que muestra los datos enviados por la placa. El valor indicado debe coincidir con la configuración del programa cargado; 115200 es el utilizado por el firmware actual.

Salir con Ctrl+C. Cerrar el monitor antes de volver a cargar o de abrir otro programa que use el mismo puerto.

# Camino B: EDU-CIAA NXP

## B1. Qué necesitás y quién lo prepara

Este proyecto utiliza **Make** para organizar la construcción de EDU-CIAA. Make lee instrucciones del proyecto y ejecuta el compilador y las demás herramientas.

Para obtenerlas usaremos el paquete **Software-CIAA para Windows**, que reúne varios programas ya preparados. El paquete contiene más herramientas de las necesarias; no hace falta aprender ni utilizar todas.

| Elemento | Para qué se usa | De dónde viene |
|---|---|---|
| Make | Ejecutar las instrucciones de construcción | Paquete Software-CIAA |
| ARM GCC y G++ | Compilar C y C++ para el procesador de EDU-CIAA | Paquete Software-CIAA |
| Herramientas de enlace y conversión | Unir el código compilado y generar los archivos de firmware | Incluidas con el conjunto de herramientas ARM |
| Utilidades como sh, mkdir y rm | Ejecutar operaciones de las instrucciones de construcción, como crear carpetas | Carpeta tools/system del paquete |
| OpenOCD | Acceder a la placa por su interfaz de programación y escribir el firmware | Paquete Software-CIAA |
| Bibliotecas utilizadas por el firmware | Aportar soporte de placa y funciones utilizadas por la aplicación | Ya incluidas en EDU-CIAA/libs |
| Controladores USB | Permitir que Windows acceda a programación y puerto serie | Preparación indicada por el paquete |

El conjunto de compilador y herramientas asociadas también se llama **toolchain**. El nombre `arm-none-eabi-gcc` identifica el compilador ARM utilizado aquí. No es el mismo compilador que utiliza ESP32.

## B2. Obtener el paquete de herramientas

El paquete del relevamiento es **CIAA_Software_1.1-Win**. Su README indica obtenerlo de la [release r1.1.0 de Software-CIAA](https://github.com/epernia/software/releases/tag/r1.1.0).

1. Abrir esa página.
2. Descargar el archivo del paquete para Windows. Los archivos llamados Source code contienen fuentes del repositorio y no reemplazan el paquete de herramientas.
3. Extraerlo en una ruta sin espacios.
4. Identificar la carpeta que contiene `CIAA-Launcher.exe` y `tools`.

En esta guía usaremos:

```text
C:\CIAA\CIAA_Software_1.1-Win
```

Si la carpeta extraída tiene otro nombre o ubicación, usar su ruta real en el próximo paso.

Las bibliotecas como sAPI, LPCOpen y FreeRTOS ya vienen en `EDU-CIAA/libs`. No requieren instalar un programa en Windows: son código que las instrucciones de construcción incorporan cuando corresponde.

## B3. Hacer que la terminal encuentre las herramientas

Cuando escribís `make`, Windows necesita saber dónde está ese ejecutable. Para eso utiliza **PATH**, una variable de entorno que contiene carpetas donde buscar programas.

Abrir PowerShell y ejecutar:

```powershell
$ciaaRoot = 'C:\CIAA\CIAA_Software_1.1-Win'
$ciaaTools = Join-Path $ciaaRoot 'tools'
$env:Path = "$ciaaTools\arm-none-eabi-gcc\bin;$ciaaTools\openocd\bin;$ciaaTools\system;$env:Path"
```

La primera línea indica dónde extrajiste el paquete. La segunda obtiene la ruta de sus herramientas. La tercera agrega sus carpetas al PATH de esta terminal, conservando las rutas que ya tenía.

Este cambio afecta esta sesión y los programas que inicie. Al cerrar la terminal, se pierde; al abrir otra, repetirlo. No modifica el PATH permanente de Windows.

El lanzador CIAA también prepara el entorno de los programas que abre. Aquí usamos la preparación explícita para poder seguir el recorrido desde PowerShell.

## B4. Comprobar las herramientas

En la misma terminal:

```powershell
make --version
arm-none-eabi-gcc --version
openocd --version
```

Cada comando debe mostrar información de su herramienta. Esto comprueba que se puede ejecutar; no prueba todavía que el proyecto compile ni que la placa esté accesible.

Si un comando no se reconoce, revisar la ruta del paquete y repetir B3. Si más adelante fallan operaciones como crear carpetas, comprobar que `tools/system` esté completo y incluido en PATH.

## B5. Entender la configuración antes de compilar

Make toma las instrucciones del archivo `Makefile` y de otros archivos con extensión `.mk`. Para el uso normal no hace falta comprender todo su contenido, pero sí saber qué selecciona cada uno:

| Archivo | Para qué se usa |
|---|---|
| `EDU-CIAA/Makefile` | Organizar la construcción y definir acciones como compilar y cargar |
| `EDU-CIAA/board.mk` | Elegir la placa; actualmente edu_ciaa_nxp |
| `EDU-CIAA/program.mk` | Elegir la carpeta/programa; actualmente rover |
| `EDU-CIAA/rover/config.mk` | Indicar fuentes, opciones y bibliotecas de ese programa |
| `EDU-CIAA/libs/*/module.mk` | Indicar cómo incorporar cada biblioteca |

El programa que se construye lo elige esta configuración. Si se crea otro programa, también necesita una configuración que describa sus archivos y dependencias.

Las bibliotecas están en el repositorio, pero no todas sus versiones se utilizan: las reglas eligen cuál incorporar. Por ejemplo, la configuración actual selecciona sAPI v0.5.2.

## B6. Compilar

Desde la terminal preparada:

```powershell
Set-Location 'C:\Arturito'
make -C EDU-CIAA all
```

`-C EDU-CIAA` indica a Make que use esa carpeta. `all` es el nombre de la acción de construcción definida por el proyecto.

Make ejecuta las herramientas y compila el programa seleccionado. Actualmente los resultados se guardan en `EDU-CIAA/rover/out/`.

Entre ellos hay un archivo `.elf`, que contiene el programa enlazado y datos útiles para depuración, y archivos `.bin`/`.hex`, que representan el programa para su carga. El proyecto se encarga de generarlos; no necesitás fabricarlos a mano.

Comprobar que termine sin errores antes de cargar.

## B7. Preparar el acceso USB

EDU-CIAA utiliza una interfaz USB con funciones de programación y comunicación serie. Para este proyecto, OpenOCD accede a la programación mediante FTDI, el componente que proporciona esa conexión.

El README de Software-CIAA describe la preparación con capturas. Seguirlo desde el paquete descargado:

1. Conectar la EDU-CIAA por su conexión de programación con cable USB de datos.
2. Ejecutar `CIAA-Launcher.exe`.
3. Abrir desde el lanzador el instalador **FTDI VCP** e instalarlo siguiendo sus pasos. Proporciona el acceso como puerto serie.
4. Abrir **Zadig** desde el lanzador. Zadig permite cambiar el controlador USB de una interfaz para que la herramienta de programación acceda a ella.
5. Activar `Options → List All Devices`.
6. Identificar **Dual RS232HS (Interface 0)** de la placa conectada y configurar el controlador indicado por las capturas del README del paquete.
7. Conservar la otra interfaz para el puerto serie.

Falta registrar y probar en esta guía el controlador exacto elegido y su versión en Windows. La identificación de la interfaz es importante: programación y serie cumplen funciones distintas.

## B8. Cargar el programa

Volver a la terminal preparada, desde la raíz de Arturito:

```powershell
make -C EDU-CIAA download
```

`download` es la acción de carga definida en el Makefile. Ejecuta OpenOCD con la configuración de conexión y microcontrolador incluida en el proyecto.

Esa configuración está en `EDU-CIAA/scripts/openocd/`. Le indica a OpenOCD cómo acceder a esta placa. Normalmente se conserva al cambiar el programa de aplicación; una placa diferente puede necesitar otra configuración.

Comprobar que el proceso de programación termine correctamente. La carga reemplaza el programa anterior.

## B9. Observar la ejecución, si el programa utiliza serie

El paquete incluye terminales serie, como CoolTerm. Son programas que muestran los mensajes enviados por la placa.

1. Identificar en el Administrador de dispositivos el puerto COM asignado a la comunicación serie.
2. Abrir la terminal serie.
3. Seleccionar ese puerto y la velocidad configurada por el programa. El firmware actual utiliza 115200.
4. Conectar la terminal. Reiniciar la placa si necesitás observar mensajes de arranque.

Cerrar la conexión antes de abrir otro programa que use ese puerto.

## 5. Qué cambia cuando querés cargar otro programa

Las herramientas se preparan una vez. Después podés construir otros programas para la misma plataforma utilizando esas herramientas.

Antes de compilar, la configuración debe indicar:

- Qué archivos contienen la aplicación.
- Qué bibliotecas utiliza.
- Qué placa y código de soporte necesita.
- Qué opciones adicionales necesita la construcción.

En ESP32 esto se expresa principalmente en `platformio.ini`; elegís la configuración con `pio run -e nombre_del_entorno`. Ese nombre debe existir en el archivo.

En EDU-CIAA se selecciona el programa mediante `program.mk` o los parámetros que admite Make; su `config.mk` describe su construcción. El comando de carga utiliza el programa seleccionado.

Modificar un `.c` ya incluido en el programa suele requerir solamente volver a compilar y cargar. Agregar una biblioteca o archivos nuevos puede requerir actualizar la configuración. Cambiar el microcontrolador requiere comprobar también compilador, soporte de placa y herramienta de carga.

Los programas de diagnóstico que quedaron en el proyecto son alternativas para investigar sensores. No son pasos obligatorios para preparar el entorno ni cargar la aplicación principal.

## 6. Si un paso falla

| Problema | Primer lugar que revisar |
|---|---|
| No se reconoce pio | Instalación y terminal de PlatformIO |
| No se reconoce make, GCC u OpenOCD | Ruta del paquete CIAA y PATH de esa terminal |
| No puede encontrar una cabecera o biblioteca | Dependencias y rutas de la configuración de construcción |
| Una función no aparece durante el enlace | Que su implementación esté incluida entre las fuentes o bibliotecas |
| No aparece la placa | Cable de datos, conector y controlador USB |
| No puede abrir el puerto | Puerto seleccionado y otros programas que lo estén utilizando |
| Compila, pero no carga | Conexión y configuración de la herramienta de carga |
| Carga, pero no funciona como se esperaba | Configuración y requisitos del programa, además del hardware conectado |

## Estado de la guía

Los pasos se basan en el relevamiento y los archivos del proyecto examinados el 8/10/2026. No se ejecutaron compilaciones ni cargas en esta revisión.

Para cerrar el recorrido desde cero todavía hay que incorporar el enlace de descarga de Arturito, confirmar el modelo físico ESP32 y sus controladores USB, registrar el controlador utilizado para programación EDU-CIAA y probar ambos recorridos en una instalación nueva.

El cableado y la alimentación necesarios para ejecutar cada aplicación deben estar en `hardware.md`. Las versiones de dependencias deben registrarse y fijarse después de validar una combinación: actualmente algunas dependencias ESP32 se resuelven sin una versión única fijada.

## Nota de dependencias de la actualización MPU6050

Desde el 12/10/2026, los entornos esp32dev e imu_only declaran Adafruit MPU6050 2.2.9. PlatformIO también resuelve Adafruit BusIO y Adafruit Unified Sensor. Esa biblioteca proporciona inicialización y adquisición de aceleración y giro en unidades físicas; el adaptador del proyecto está en src/layer1_hal/hal_mpu6050_esp32.cpp. La calibración residual está en el driver compartido y la estimación angular en shared/layer3_app/odometry.c.

Las implementaciones HAL MPU6050 sustituyen a los archivos hal_imu anteriores. El diagnóstico imu_only ahora también compila odometry.c; tof_only conserva sus fuentes y dependencias.

Los cuatro adaptadores del rover EDU-CIAA están implementados en C y seleccionados mediante SRC en rover/config.mk. Este programa no necesita la ruta adicional de cabeceras C++ utilizada en una comprobación anterior. ESP32 conserva sus adaptadores .cpp para utilizar sus bibliotecas C++.

## Actualizacion VL53L0X (14/10/2026)

La interfaz vigente es `shared/layer1_hal/hal_vl53l0x.h` y los adaptadores se llaman `hal_vl53l0x_educiaa.c` y `hal_vl53l0x_esp32.cpp`. EDU-CIAA incorpora `libs/vl53l0x` mediante `USE_VL53L0X=y`; ESP32 conserva Adafruit.

El diagnostico antes llamado `tof_only` pasa a `vl53l0x_only`: compilarlo con `pio run -e vl53l0x_only`. Las menciones anteriores del nombre corresponden al estado previo. Para compilar EDU-CIAA, ejecutar `make all` desde `EDU-CIAA`, con el toolchain configurado. Ninguno de esos comandos carga firmware en la placa.

Desde el 15/10/2026, runtime utiliza `driver_vl53l0x.h`, que delega en
`hal_vl53l0x.h`. La biblioteca ST conserva `libs/vl53l0x/Api/core` y
`libs/vl53l0x/Api/platform`. Las comprobaciones no requieren guardar logs en las
carpetas del firmware.
