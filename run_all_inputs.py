import glob
import os
import subprocess
import shutil

# Archivos c++
programa = ["main.cpp", "scanner.cpp", "token.cpp", "parser.cpp", "ast.cpp", "visitor.cpp"]

# Compilar
compile_cmd = ["g++"] + programa
print("Compilando:", " ".join(compile_cmd))
result = subprocess.run(compile_cmd, capture_output=True, text=True)

if result.returncode != 0:
    print("Error en compilación:\n", result.stderr)
    exit(1)

print("Compilación exitosa")

# Ejecutar
input_dir = "inputs"
output_dir = "outputs"
os.makedirs(output_dir, exist_ok=True)

input_files = sorted(glob.glob(os.path.join(input_dir, "*.txt")))
if not input_files:
    print("No se encontraron archivos .txt en el directorio de entrada.")
    exit(1)

for filepath in input_files:
    filename = os.path.basename(filepath)
    basename, _ = os.path.splitext(filename)
    print(f"Ejecutando {filename}")

    run_cmd = ["./a.out", filepath]
    result = subprocess.run(run_cmd, capture_output=True, text=True)

    if result.returncode != 0:
        print(f"Error al compilar {filename}:\n{result.stderr}")
        continue

    generated_file = os.path.join(input_dir, f"{basename}.s")
    if os.path.isfile(generated_file):
        dest_file = os.path.join(output_dir, f"{basename}.s")
        shutil.move(generated_file, dest_file)
        print(f"Guardado {dest_file}")
    else:
        print(f"No se generó el archivo esperado: {generated_file}")
