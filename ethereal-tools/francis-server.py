import socket
import requests
import json
import struct
import os
import time
import sys
import threading
from datetime import datetime

# Prompt constante o inicial
CONSTANT_PROMPT = "No uses emojis, por favor responde únicamente con texto en ASCII reducido. Sé conciso."

# Configuración de la API de Google
API_URL = "https://generativelanguage.googleapis.com/v1beta/models/gemini-1.5-flash-latest:generateContent"
API_KEY = "AIzaSyCcOusago52BmRAI1WGcyhXAnlJHlnR16Q"  # Reemplaza con tu API Key de Google

class MemoryDebugger:
    def __init__(self):
        # Lista de sectores registrados (dirección_inicio, dirección_fin, tag)
        self.memory_sectors = []
    
    def register_memory_sector(self, start_addr, end_addr, tag):
        """Registra un nuevo sector de memoria"""
        # Validar que el rango sea correcto
        if start_addr > end_addr:
            return f"ERROR: Direccion inicio {hex(start_addr)} > direccion fin {hex(end_addr)} - rango invalido"
        
        self.memory_sectors.append((start_addr, end_addr, tag))
        return f"Sector registrado: {hex(start_addr)}-{hex(end_addr)} [{tag}]"
    
    def log_memory_address(self, address, name):
        """Busca en qué sectores se encuentra una dirección"""
        matching_sectors = []
        
        for start, end, tag in self.memory_sectors:
            if start <= address <= end:
                matching_sectors.append(tag)
        
        if matching_sectors:
            sectors_str = ", ".join(matching_sectors)
            return f"{name} esta en la direccion {hex(address)} en el sector [{sectors_str}]"
        else:
            # Mostrar información adicional para debugging
            debug_info = f"{name} esta en la direccion {hex(address)} en sector [DESCONOCIDO]"
            if self.memory_sectors:
                debug_info += f"\nSectores disponibles:"
                for start, end, tag in self.memory_sectors:
                    debug_info += f"\n  {hex(start)}-{hex(end)} [{tag}]"
            else:
                debug_info += "\nNo hay sectores registrados"
            return debug_info
    
    def free_memory_sector(self, start_addr):
        """Libera un sector que coincida exactamente con la dirección de inicio"""
        for i, (start, end, tag) in enumerate(self.memory_sectors):
            if start == start_addr:
                removed_sector = self.memory_sectors.pop(i)
                return f"Sector liberado: {hex(start)}-{hex(end)} [{tag}]"
        
        return f"No se encontro sector con direccion de inicio {hex(start_addr)}"
    
    def list_sectors(self):
        """Lista todos los sectores registrados (para debugging)"""
        if not self.memory_sectors:
            return "No hay sectores registrados"
        
        result = "Sectores registrados:\n"
        for start, end, tag in self.memory_sectors:
            result += f"  {hex(start)}-{hex(end)} [{tag}]\n"
        return result.rstrip()

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

def show_connecting_animation():
    """Muestra una animación simple mientras se intenta conectar"""
    frames = ['⠋', '⠙', '⠹', '⠸', '⠼', '⠴', '⠦', '⠧', '⠇', '⠏']
    colors = {
        'RESET': '\033[0m',
        'BOLD': '\033[1m',
        'YELLOW': '\033[93m',
        'GRAY': '\033[90m'
    }
    
    timestamp = datetime.now().strftime("%H:%M:%S")
    for i in range(10):  # 10 frames * 0.1s = 1 segundo
        message = (f'{colors["GRAY"]}[{timestamp}]{colors["RESET"]} '
                  f'{colors["YELLOW"]}{colors["BOLD"]}[RECONECTANDO]{colors["RESET"]} '
                  f'{frames[i % len(frames)]} Intentando conectar...')
        sys.stdout.write(f'\r{message}')
        sys.stdout.flush()
        time.sleep(0.1)
    sys.stdout.write('\r' + ' ' * 80 + '\r')  # Limpiar la línea
    sys.stdout.flush()

class TickCounter:
    def __init__(self):
        self.tick_count = 0
        self.last_reset_time = time.time()
        self.is_monitoring = False
        self.display_thread = None
        self.stop_display = False
        
    def add_tick(self):
        """Añade un tick al contador"""
        current_time = time.time()
        if not self.is_monitoring:
            self.is_monitoring = True
            self.last_reset_time = current_time
            self.tick_count = 0
            self.start_display()
            
        self.tick_count += 1
        
    def start_display(self):
        """Inicia el hilo de visualización"""
        if self.display_thread is None or not self.display_thread.is_alive():
            self.stop_display = False
            self.display_thread = threading.Thread(target=self._display_loop, daemon=True)
            self.display_thread.start()
            
    def _display_loop(self):
        """Loop de visualización que se ejecuta cada segundo"""
        while not self.stop_display:
            current_time = time.time()
            elapsed = current_time - self.last_reset_time
            
            if elapsed >= 5.0:  # Cada 5 segundos
                ticks_per_second = self.tick_count / elapsed
                
                # Limpiar la línea actual
                sys.stdout.write('\r' + ' ' * 100 + '\r')
                
                # Obtener timestamp actual
                timestamp = datetime.now().strftime("%H:%M:%S")
                
                # Mostrar resultado final con colores
                colors = {
                    'RESET': '\033[0m',
                    'BOLD': '\033[1m',
                    'GREEN': '\033[92m',
                    'CYAN': '\033[96m',
                    'YELLOW': '\033[93m',
                    'BLUE': '\033[94m',
                    'GRAY': '\033[90m'
                }
                
                final_message = (f"{colors['GRAY']}[{timestamp}]{colors['RESET']} "
                               f"{colors['GREEN']}{colors['BOLD']}[TICK FINAL]{colors['RESET']} "
                               f"{colors['CYAN']}{self.tick_count}{colors['RESET']} ticks en "
                               f"{colors['YELLOW']}{elapsed:.2f}s{colors['RESET']} = "
                               f"{colors['BLUE']}{colors['BOLD']}{ticks_per_second:.1f} ticks/s{colors['RESET']}")
                
                print(final_message)
                
                # Resetear contador
                self.tick_count = 0
                self.last_reset_time = current_time
            else:
                # Mostrar progreso con colores
                ticks_per_second = self.tick_count / elapsed if elapsed > 0 else 0
                
                colors = {
                    'RESET': '\033[0m',
                    'BOLD': '\033[1m',
                    'MAGENTA': '\033[95m',
                    'CYAN': '\033[96m',
                    'YELLOW': '\033[93m',
                    'WHITE': '\033[97m'
                }
                
                progress_message = (f'\r{colors["MAGENTA"]}{colors["BOLD"]}[TICK MONITOR]{colors["RESET"]} '
                                  f'{colors["CYAN"]}{self.tick_count}{colors["RESET"]} ticks en '
                                  f'{colors["YELLOW"]}{elapsed:.1f}s{colors["RESET"]} = '
                                  f'{colors["WHITE"]}{colors["BOLD"]}{ticks_per_second:.1f} ticks/s{colors["RESET"]} '
                                  f'{colors["MAGENTA"]}(actualizando...){colors["RESET"]}')
                
                sys.stdout.write(progress_message)
                sys.stdout.flush()
                
            time.sleep(0.5)  # Actualizar cada 0.5 segundos
            
    def stop_monitoring(self):
        """Detiene el monitoreo"""
        if self.is_monitoring:
            # Mostrar mensaje final antes de parar
            current_time = time.time()
            elapsed = current_time - self.last_reset_time
            
            # Limpiar la línea actual del monitor
            sys.stdout.write('\r' + ' ' * 100 + '\r')
            
            if elapsed > 0 and self.tick_count > 0:
                ticks_per_second = self.tick_count / elapsed
                
                # Obtener timestamp actual
                timestamp = datetime.now().strftime("%H:%M:%S")
                
                colors = {
                    'RESET': '\033[0m',
                    'BOLD': '\033[1m',
                    'RED': '\033[91m',
                    'CYAN': '\033[96m',
                    'YELLOW': '\033[93m',
                    'WHITE': '\033[97m',
                    'GRAY': '\033[90m'
                }
                
                final_message = (f"{colors['GRAY']}[{timestamp}]{colors['RESET']} "
                               f"{colors['RED']}{colors['BOLD']}[TICK MONITOR DETENIDO]{colors['RESET']} "
                               f"Último registro: {colors['CYAN']}{self.tick_count}{colors['RESET']} ticks en "
                               f"{colors['YELLOW']}{elapsed:.2f}s{colors['RESET']} = "
                               f"{colors['WHITE']}{colors['BOLD']}{ticks_per_second:.1f} ticks/s{colors['RESET']}")
                
                print(final_message)
        
        # Detener completamente el monitoreo
        self.stop_display = True
        self.is_monitoring = False
        self.tick_count = 0  # Resetear para que no muestre ceros
        
        if self.display_thread and self.display_thread.is_alive():
            self.display_thread.join(timeout=1.0)

def format_log_message(log_message):
    """Formatea el mensaje de log con colores de fondo según el tipo"""
    # Códigos de colores ANSI para fondos y formato
    colors = {
        'RESET': '\033[0m',
        'BOLD': '\033[1m',          # Texto en negrita
        'BG_RED': '\033[101m',      # Error - fondo rojo
        'BG_YELLOW': '\033[103m',   # Warning - fondo amarillo
        'BG_GREEN': '\033[102m',    # Success - fondo verde
        'BG_BLUE': '\033[104m',     # Info - fondo azul
        'BG_CYAN': '\033[106m',     # Default - fondo cyan
    }
    
    if log_message.startswith("E:"):
        return f"{colors['BG_RED']}{colors['BOLD']}[ERROR]{colors['RESET']} {log_message[2:].strip()}"
    elif log_message.startswith("W:"):
        return f"{colors['BG_YELLOW']}{colors['BOLD']}[WARNING]{colors['RESET']} {log_message[2:].strip()}"
    elif log_message.startswith("S:"):
        return f"{colors['BG_GREEN']}{colors['BOLD']}[SUCCESS]{colors['RESET']} {log_message[2:].strip()}"
    elif log_message.startswith("I:"):
        return f"{colors['BG_BLUE']}{colors['BOLD']}[INFO]{colors['RESET']} {log_message[2:].strip()}"
    else:
        return f"{colors['BG_CYAN']}{colors['BOLD']}[PINKOS DEBUG]{colors['RESET']} {log_message}"

def main():
    # Configuración del cliente TCP
    host = 'localhost'
    port = 4444
    reconnect_delay = 0.1  # segundos entre intentos de reconexión
    first_attempt = True
    tick_counter = TickCounter()
    memory_debugger = MemoryDebugger()

    while True:  # Bucle principal para reconexión
        try:
            # Crear el socket
            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as client_socket:
                # Conectarse al servidor
                if first_attempt:
                    print(f"Conectando a {host}:{port}...")
                    first_attempt = False
                else:
                    show_connecting_animation()
                
                client_socket.connect((host, port))
                
                # Mostrar mensaje de conexión establecida con timestamp y colores
                timestamp = datetime.now().strftime("%H:%M:%S")
                colors = {
                    'RESET': '\033[0m',
                    'BOLD': '\033[1m',
                    'GREEN': '\033[92m',
                    'GRAY': '\033[90m'
                }
                
                connection_message = (f"{colors['GRAY']}[{timestamp}]{colors['RESET']} "
                                    f"{colors['GREEN']}{colors['BOLD']}[CONEXIÓN ESTABLECIDA]{colors['RESET']} "
                                    f"Escuchando en {host}:{port}...")
                print(connection_message)

                buffer = ""
                while True:
                    try:
                        # Recibir datos
                        data = client_socket.recv(1024).decode('utf-8')
                        if not data:
                            # Servidor cerró la conexión
                            timestamp = datetime.now().strftime("%H:%M:%S")
                            colors = {
                                'RESET': '\033[0m',
                                'BOLD': '\033[1m',
                                'RED': '\033[91m',
                                'GRAY': '\033[90m'
                            }
                            
                            disconnect_message = (f"{colors['GRAY']}[{timestamp}]{colors['RESET']} "
                                                f"{colors['RED']}{colors['BOLD']}[SERVIDOR DESCONECTADO]{colors['RESET']} "
                                                f"El servidor cerró la conexión.")
                            print(f"\n{disconnect_message}")
                            tick_counter.stop_monitoring()
                            break
                        
                        buffer += data
                        while '\n' in buffer:
                            # Procesar línea completa
                            line, buffer = buffer.split('\n', 1)
                            # print(f"Recibido: {line}")

                            # Manejar logs del sistema operativo
                            if line.startswith("LOG: "):
                                log_message = line[5:]  # Remover el prefijo "LOG: "
                                
                                # Verificar si es un TICK
                                if log_message.strip() == "TICK":
                                    tick_counter.add_tick()
                                    continue  # No mostrar los TICKs individuales
                                
                                formatted_log = format_log_message(log_message)
                                # Limpiar la línea del contador antes de mostrar otros logs
                                if tick_counter.is_monitoring:
                                    sys.stdout.write('\r' + ' ' * 80 + '\r')
                                print(formatted_log)
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

                            # Manejar comandos de debugging de memoria
                            if line.startswith("MEMREG "):
                                parts = line[7:].strip().split()
                                if len(parts) >= 3:
                                    try:
                                        start_addr = int(parts[0], 16)  # Convertir de hex
                                        end_addr = int(parts[1], 16)    # Convertir de hex
                                        tag = " ".join(parts[2:])       # El resto es el tag
                                        
                                        result = memory_debugger.register_memory_sector(start_addr, end_addr, tag)
                                        print(f"[MEMORY DEBUG] {result}")
                                    except ValueError:
                                        error_msg = "Error: Direcciones invalidas en MEMREG"
                                        print(f"[MEMORY DEBUG] {error_msg}")
                                else:
                                    error_msg = "Error: MEMREG requiere <inicio_hex> <fin_hex> <tag>"
                                    print(f"[MEMORY DEBUG] {error_msg}")
                                continue

                            if line.startswith("MEMLOG "):
                                parts = line[7:].strip().split(None, 1)  # Split en máximo 2 partes
                                if len(parts) >= 2:
                                    try:
                                        address = int(parts[0], 16)  # Convertir de hex
                                        name = parts[1]              # El resto es el nombre
                                        
                                        result = memory_debugger.log_memory_address(address, name)
                                        print(f"[MEMORY DEBUG] {result}")
                                    except ValueError:
                                        error_msg = "Error: Direccion invalida en MEMLOG"
                                        print(f"[MEMORY DEBUG] {error_msg}")
                                else:
                                    error_msg = "Error: MEMLOG requiere <direccion_hex> <nombre>"
                                    print(f"[MEMORY DEBUG] {error_msg}")
                                continue

                            if line.startswith("MEMFREE "):
                                addr_str = line[8:].strip()
                                try:
                                    start_addr = int(addr_str, 16)  # Convertir de hex
                                    
                                    result = memory_debugger.free_memory_sector(start_addr)
                                    print(f"[MEMORY DEBUG] {result}")
                                except ValueError:
                                    error_msg = "Error: Direccion invalida en MEMFREE"
                                    print(f"[MEMORY DEBUG] {error_msg}")
                                continue

                            # Comando extra para listar sectores (para debugging del servidor)
                            if line == "MEMLIST":
                                result = memory_debugger.list_sectors()
                                print(f"[MEMORY DEBUG] {result}")
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
                        # Conexión perdida de forma abrupta
                        timestamp = datetime.now().strftime("%H:%M:%S")
                        colors = {
                            'RESET': '\033[0m',
                            'BOLD': '\033[1m',
                            'RED': '\033[91m',
                            'GRAY': '\033[90m'
                        }
                        
                        disconnect_message = (f"{colors['GRAY']}[{timestamp}]{colors['RESET']} "
                                            f"{colors['RED']}{colors['BOLD']}[CONEXIÓN PERDIDA]{colors['RESET']} "
                                            f"{str(e)}")
                        print(f"\n{disconnect_message}")
                        tick_counter.stop_monitoring()
                        break  # Salir del bucle interno para reconectar
                    
                    except socket.error as e:
                        # Error de socket
                        timestamp = datetime.now().strftime("%H:%M:%S")
                        colors = {
                            'RESET': '\033[0m',
                            'BOLD': '\033[1m',
                            'RED': '\033[91m',
                            'GRAY': '\033[90m'
                        }
                        
                        disconnect_message = (f"{colors['GRAY']}[{timestamp}]{colors['RESET']} "
                                            f"{colors['RED']}{colors['BOLD']}[ERROR DE SOCKET]{colors['RESET']} "
                                            f"{str(e)}")
                        print(f"\n{disconnect_message}")
                        tick_counter.stop_monitoring()
                        break  # Salir del bucle interno para reconectar

        except ConnectionRefusedError:
            # No hacer nada - solo mostrar la animación de reconexión
            if tick_counter.is_monitoring:
                tick_counter.stop_monitoring()
        except socket.gaierror as e:
            timestamp = datetime.now().strftime("%H:%M:%S")
            colors = {
                'RESET': '\033[0m',
                'BOLD': '\033[1m',
                'RED': '\033[91m',
                'GRAY': '\033[90m'
            }
            
            error_message = (f"{colors['GRAY']}[{timestamp}]{colors['RESET']} "
                           f"{colors['RED']}{colors['BOLD']}[ERROR DNS]{colors['RESET']} "
                           f"Error de resolución de nombre: {e}")
            print(f"\n{error_message}")
            tick_counter.stop_monitoring()
        except Exception as e:
            timestamp = datetime.now().strftime("%H:%M:%S")
            colors = {
                'RESET': '\033[0m',
                'BOLD': '\033[1m',
                'RED': '\033[91m',
                'GRAY': '\033[90m'
            }
            
            error_message = (f"{colors['GRAY']}[{timestamp}]{colors['RESET']} "
                           f"{colors['RED']}{colors['BOLD']}[ERROR INESPERADO]{colors['RESET']} "
                           f"{str(e)}")
            print(f"\n{error_message}")
            tick_counter.stop_monitoring()
        
        # Esperar antes de reintentar (sin mostrar mensaje)
        time.sleep(reconnect_delay)

if __name__ == "__main__":
    main()