# Shell

## English
This project involves the creation of a shell using C.

### Features
- Execute built-in commands: `cd`, `help`, `exit`, `pwd`, `echo`, `clear`, `alias`
- Read and execute commands from the user input, with quoting (`"..."`) and backslash-escaping
- Load configuration from a JSON file (`PATH`, `EDITOR`, `PROMPT`)
- Support for aliases, expanded at execution time (e.g. `ll` -> `ls -l`)
- Minimal colored prompt showing the current directory (`~` shortened) plus the configured `PROMPT`
- Self-contained: vendors its own minimal JSON parser, no external `-lcjson` dependency needed

### Files
- `main.c`: Entry point of the application
- `shell.c`: Contains the main loop of the shell
- `command.c`: Implements command execution and built-in commands
- `config.c`: Handles loading configuration from a JSON file
- `cleanup.c`: Performs cleanup tasks before exiting
- `cJSON.h` / `cJSON.c`: Vendored minimal JSON parser used by `config.c`
- `config.json`: Example configuration file
- `LICENSE`: MIT License
- `README.md`: This file

### How to Build
To build the project, run the following command:
```sh
gcc -o shell main.c shell.c command.c config.c cleanup.c cJSON.c
```

### How to Run
To run the shell, use the following command:
```sh
./shell [config.json]
```


## Español
Este proyecto implica la creación de una shell usando C.

### Características
- Ejecutar comandos integrados: cd, help, exit, pwd, echo, clear, alias
- Leer y ejecutar comandos desde la entrada del usuario, con comillas (`"..."`) y escape con backslash
- Cargar configuración desde un archivo JSON (`PATH`, `EDITOR`, `PROMPT`)
- Soporte para alias, expandidos al ejecutar (p. ej. `ll` -> `ls -l`)
- Prompt minimalista con color que muestra el directorio actual (acortado con `~`) y el `PROMPT` configurado
- Autocontenido: incluye su propio parser JSON mínimo, sin dependencia externa de `-lcjson`

### Archivos
- `main.c`: Punto de entrada de la aplicación
- `shell.c`: Contiene el bucle principal de la shell
- `command.c`: Implementa la ejecución de comandos y comandos integrados
- `config.c`: Maneja la carga de configuración desde un archivo JSON
- `cleanup.c`: Realiza tareas de limpieza antes de salir
- `cJSON.h` / `cJSON.c`: Parser JSON mínimo incluido, usado por `config.c`
- `config.json`: Ejemplo de archivo de configuración
- `LICENSE`: Licencia MIT
- `README.md`: Este archivo

### Cómo Compilar
Para compilar el proyecto, ejecuta el siguiente comando:
```sh
gcc -o shell main.c shell.c command.c config.c cleanup.c cJSON.c
```

### Cómo Ejecutar
Para ejecutar la shell, usa el siguiente comando:
```sh
./shell [config.json]
```
