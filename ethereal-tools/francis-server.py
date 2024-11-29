import socket
import requests
import json
import struct

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
        print(f"API response: {response.json()}")
        text =  response.json().get("candidates", [])[0].get("content", {}).get("parts", [])[0].get("text", "") 
        # Convert tildes ands special characters (without extended ASCII)
        text = text.replace("á", "a").replace("é", "e").replace("í", "i").replace("ó", "o").replace("ú", "u")
        text = text.replace("Á", "A").replace("É", "E").replace("Í", "I").replace("Ó", "O").replace("Ú", "U")
        text = text.replace("ñ", "n").replace("Ñ", "N").replace("ü", "u").replace("Ü", "U")
        text = ''.join([c for c in text if ord(c) < 128])
        return text
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
                                length = len(data)
                                # header = 0x00 01 00 00 00 05 7E 40 (but remember to convert to 64-bit little-endian)
                                header = struct.pack("<Q", 0x00010000057E40) + length.to_bytes(4, byteorder='big')
                                client_socket.sendall(header)
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
                                length = len(data)
                                header = b'\x00\x00\x01\x00\x00\x00\x00\x00' + length.to_bytes(4, byteorder='big')
                                client_socket.sendall(header)
                                client_socket.sendall(data)
                                print("Enviado pietra.bin")
                        except FileNotFoundError:
                            print("No se encontró el archivo pietra.bin")
                            client_socket.sendall(b"Error: pietra.bin no encontrado\n")
                        continue

                    # Manejar comandos con prefijo "chatgpt"
                    if line.startswith("chatgpt "):
                        # 16 bits del codigo de respuesta, 8 bits del tipo del contenido, 8 bits del tipo de respuesta y 32 bits del tamaño del contenido
                        user_prompt = line[len("chatgpt "):].strip()
                        print(f"Enviando a la API: {user_prompt}")
                        api_response = send_to_google_api(user_prompt)
                        # length = len(api_response)
                        # header = b'\x00\x00\x00\x00' + bytes([0, 0, 0, 0]) + length.to_bytes(4, byteorder='big')
                        # client_socket.sendall(header)
                        client_socket.sendall(f"{api_response}\n".encode('utf-8'))
                        # Send \0
                        client_socket.sendall(b'\x00')
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
