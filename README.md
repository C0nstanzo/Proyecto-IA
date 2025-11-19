# Proyecto IA - EPTP

### Estructura del repositorio

- `Entrega 1/` : Material de la primera entrega (LaTeX, referencias).
- `Presentación 1/` : Recursos para la presentación 1.
- `Proyecto/` : Código fuente y datos del proyecto.  
  - `main.cpp` : Programa principal en C++.
  - `Makefile` : Reglas de compilación (ver dentro de `Proyecto/`).
  - `Instancias/` : Carpetas con archivos de entrada.  
    - `nodos/` : Instancias relacionadas con los nodos.
    - `usuarios/` : Instancias relacionadas con usuarios.

### Compilación

Para la compilación se recomienda utilizar el archivo `Makefile` con el comando make
Luego ejecutar el comando ./solver <archivo_nodos> <archivo_usuarios> 
  - Ejemplo de ejecución: ./solver instancias/nodos/10_instancia_0.txt instancias/usuarios/10_instancia_0_5us.txt

### Salida. 
Se genera un archivo llamado salida_<nombre_instancia_nodos>.txt

