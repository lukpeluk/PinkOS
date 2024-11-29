import socket
import requests
import json

# Prompt constante o inicial
CONSTANT_PROMPT = "No uses emojis, por favor responde únicamente con texto en ASCII reducido. Sé conciso."

# Configuración de la API de Google
API_URL = "https://generativelanguage.googleapis.com/v1beta/models/gemini-1.5-flash-latest:generateContent"
API_KEY = "AIzaSyCcOusago52BmRAI1WGcyhXAnlJHlnR16Q"  # Reemplaza con tu API Key de Google

def send_to_google_api(user_prompt):
    """Envía un prompt a la API de Google y retorna la respuesta."""
    try:
        headers = {"Content-Type": "application/json"}
        data = {
            "contents": [
                {
                    "parts": [
                        {"text": f"{CONSTANT_PROMPT} {user_prompt}"}
                    ]
                }
            ]
        }
        response = requests.post(f"{API_URL}?key={API_KEY}", headers=headers, data=json.dumps(data))
        response.raise_for_status()  # Lanza un error si la respuesta no es 2xx
        return response.json().get("contents", [{}])[0].get("parts", [{}])[0].get("text", "Sin respuesta de la API.")
    except Exception as e:
        return f"Error al comunicarse con la API: {str(e)}"

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

                    # Manejar comandos especiales
                    if line == "francis":
                        print("Recibido francis")
                        # Mandar francis.bin
                        try:
                            with open("./ethereal-tools/francis.bin", "rb") as f:
                                data = f.read()
                                client_socket.sendall(data)
                                print("Enviado francis.bin")
                        except FileNotFoundError:
                            print("No se encontró el archivo francis.bin")
                            client_socket.sendall(b"Error: francis.bin no encontrado\n")
                        continue

                    if line == "pietra":
                        print("Recibido PIETRA")
                        # Mandar pietra.bin
                        try:
                            with open("./ethereal-tools/pietra.bin", "rb") as f:
                                data = f.read()
                                client_socket.sendall(data)
                                print("Enviado pietra.bin")
                        except FileNotFoundError:
                            print("No se encontró el archivo pietra.bin")
                            client_socket.sendall(b"Error: pietra.bin no encontrado\n")
                        continue

                    # Manejar comandos con prefijo "chatgpt"
                    if line.startswith("chatgpt "):
                        user_prompt = line[len("chatgpt "):].strip()
                        print(f"Enviando a la API: {user_prompt}")
                        api_response = send_to_google_api(user_prompt)
                        client_socket.sendall(f"{api_response}\n".encode('utf-8'))
                        print(f"Respuesta de la API enviada: {api_response}")
                        continue

                    # Responder al servidor con un mensaje por defecto
                    respuesta = "PUTO EL QUE LEE\n"
                    client_socket.sendall(respuesta.encode('utf-8'))
                    print(f"Enviado: {respuesta.strip()}")

    except ConnectionRefusedError:
        print("No se pudo conectar al servidor. Asegúrate de que esté ejecutándose.")
    except Exception as e:
        print(f"Error: {e}")

if __name__ == "__main__":
    main()
