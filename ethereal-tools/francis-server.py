import socket
import requests
import json
import struct
import os
import time

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

def send_response_with_header(client_socket, header_code, content_type, response_type, data):
    """Envía respuesta con header correcto en little-endian"""
    total_size = len(data)
    
    # Crear header de 14 bytes en little-endian
    # 2 bytes header_code + 2 bytes content_type + 2 bytes response_type + 8 bytes total_size
    header = struct.pack('<HHHQ', header_code, content_type, response_type, total_size)
    
    # Enviar header + datos
    client_socket.sendall(header + data)
    print(f"Enviado header: code={header_code}, content_type={content_type}, response_type={response_type}, size={total_size}")

def main():
    # Configuración del cliente TCP
    host = 'localhost'
    port = 4444
    reconnect_delay = 0.25  # segundos entre intentos de reconexión

    while True:  # Bucle principal para reconexión
        try:
            # Crear el socket
            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as client_socket:
                # Conectarse al servidor
                print(f"Conectando a {host}:{port}...")
                client_socket.connect((host, port))
                print("Conexión establecida. Escuchando...")

                buffer = ""
                while True:
                    try:
                        # Recibir datos
                        data = client_socket.recv(1024).decode('utf-8')
                        if not data:
                            print("El servidor cerró la conexión.")
                            break
                        
                        buffer += data
                        while '\n' in buffer:
                            # Procesar línea completa
                            line, buffer = buffer.split('\n', 1)
                            # print(f"Recibido: {line}")

                            # Manejar logs del sistema operativo
                            if line.startswith("LOG: "):
                                log_message = line[5:]  # Remover el prefijo "LOG: "
                                print(f"🔍 [PINKOS DEBUG] {log_message}")
                                continue

                            # Manejar comandos especiales
                            if line == "francis":
                                print("Recibido francis")
                                # Mandar francis.bin
                                try:
                                    with open("./francis.bin", "rb") as f:
                                        data = f.read()
                                        # SUCCESS(2), BITMAP(1), FIXED_SIZE(0)
                                        send_response_with_header(client_socket, 2, 1, 0, data)
                                        print("Enviado francis.bin")
                                except FileNotFoundError:
                                    print("No se encontró el archivo francis.bin")
                                    error_msg = b"Error: francis.bin no encontrado"
                                    # ERROR(3), PLAIN_TEXT(0), ASCII_STREAM(1)
                                    send_response_with_header(client_socket, 3, 0, 1, error_msg)
                                continue

                            if line == "pietra":
                                print("Recibido pietra")
                                # Mandar pietra.bin
                                try:
                                    with open("./pietra.bin", "rb") as f:
                                        data = f.read()
                                        # SUCCESS(2), BITMAP(1), FIXED_SIZE(0)
                                        send_response_with_header(client_socket, 2, 1, 0, data)
                                        print("Enviado pietra.bin")
                                except FileNotFoundError:
                                    print("No se encontró el archivo pietra.bin")
                                    error_msg = b"Error: pietra.bin no encontrado"
                                    # ERROR(3), PLAIN_TEXT(0), ASCII_STREAM(1)
                                    send_response_with_header(client_socket, 3, 0, 1, error_msg)
                                continue

                            # Manejar comandos con prefijo "chatgpt"
                            if line.startswith("chatgpt "):
                                user_prompt = line[len("chatgpt "):].strip()
                                print(f"Enviando a la API: {user_prompt}")
                                api_response = send_to_google_api(user_prompt)
                                # Convertir a bytes y agregar null terminator para ASCII_STREAM
                                response_data = api_response.encode('utf-8') + b'\x00'
                                # SUCCESS(2), PLAIN_TEXT(0), ASCII_STREAM(1)
                                send_response_with_header(client_socket, 2, 0, 1, response_data)
                                print(f"Respuesta de la API enviada: {api_response}")
                                continue

                            if line.startswith("apt install "):
                                # Instalar un paquete
                                package = line[len("apt install "):].strip()
                                print(f"Instalando paquete: {package}")
                                try:
                                    # get from /programs the .bin
                                    with open(f"./programs/{package}.bin", "rb") as f:
                                        data = f.read()
                                        # SUCCESS(2), BITMAP(1), FIXED_SIZE(0)
                                        send_response_with_header(client_socket, 2, 1, 0, data)
                                        print(f"Enviado {package}.bin")
                                        continue
                                except FileNotFoundError:
                                    print(f"No se encontró el archivo {package}.bin")
                                    respuesta = f"Error: No se encontró el archivo {package}.bin"
                                except Exception as e:
                                    respuesta = f"Error al instalar el paquete {package}: {e}"

                                # Enviar error con null terminator
                                error_data = respuesta.encode('utf-8') + b'\x00'
                                # ERROR(3), PLAIN_TEXT(0), ASCII_STREAM(1)
                                send_response_with_header(client_socket, 3, 0, 1, error_data)
                                print(f"Enviado: {respuesta}")
                                continue

                            # # Responder al servidor con un mensaje por defecto
                            # respuesta = input("mensaje: ")
                            # response_data = respuesta.encode('utf-8') + b'\x00'
                            # # SUCCESS(2), PLAIN_TEXT(0), ASCII_STREAM(1)
                            # send_response_with_header(client_socket, 2, 0, 1, response_data)
                            # print(f"Enviado: {respuesta}")
                    
                    except (ConnectionResetError, ConnectionAbortedError, BrokenPipeError) as e:
                        print(f"Conexión perdida: {e}")
                        break  # Salir del bucle interno para reconectar
                    
                    except socket.error as e:
                        print(f"Error de socket: {e}")
                        break  # Salir del bucle interno para reconectar

        except ConnectionRefusedError:
            print(f"No se pudo conectar al servidor en {host}:{port}. Reintentando en {reconnect_delay} segundos...")
        except socket.gaierror as e:
            print(f"Error de resolución de nombre: {e}. Reintentando en {reconnect_delay} segundos...")
        except Exception as e:
            print(f"Error inesperado: {e}. Reintentando en {reconnect_delay} segundos...")
        
        # Esperar antes de reintentar
        time.sleep(reconnect_delay)
        print("Reintentando conexión...")

if __name__ == "__main__":
    main()