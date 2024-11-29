import socket

def main():
    # Configuración del cliente TCP
    host = 'localhost'
    port = 4444

    try:
        # Crear el socket
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as client_socket:
            # Conectarse al servidor
            print(f"Conectando a {host}:{port}...")
            client_socket.connect((host, port))
            print("Conexión establecida. Escuchando...")

            buffer = ""
            while True:
                # Recibir datos
                data = client_socket.recv(1024).decode('utf-8')
                if not data:
                    print("El servidor cerró la conexión.")
                    break
                
                buffer += data
                while '\n' in buffer:
                    # Procesar línea completa
                    line, buffer = buffer.split('\n', 1)
                    print(f"Recibido: {line}")

                    if line == "francis":
                        print("Recibido francis")
                        # Mandar francis.bin
                        with open("./ethereal-tools/francis.bin", "rb") as f:
                            data = f.read()
                            client_socket.sendall(data)
                            print("Enviado francis.bin")
                        continue

                    # Responder al servidor
                    # respuesta = f"Respuesta a: {line}\n"
                    respuesta = "PUTO EL QUE LEE\n"
                    client_socket.sendall(respuesta.encode('utf-8'))
                    print(f"Enviado: {respuesta.strip()}")

    except ConnectionRefusedError:
        print("No se pudo conectar al servidor. Asegúrate de que esté ejecutándose.")
    except Exception as e:
        print(f"Error: {e}")

if __name__ == "__main__":
    main()
