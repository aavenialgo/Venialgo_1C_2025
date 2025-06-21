import asyncio
from bleak import BleakClient
import matplotlib.pyplot as plt
import re
from collections import deque

## @file visualizacion_bluetooth.py
#  @brief Visualizacion en tiempo real de datos recibidos por Bluetooth usando Bleak y Matplotlib.
#  @author Andres Venialgo
#  @date 1C-2025

address = "40:4C:CA:44:57:0A"  # Direccion MAC del dispositivo Bluetooth
char_uuid = "0000ffe1-0000-1000-8000-00805f9b34fb"  # UUID del servicio de datos

data_buffer = deque(maxlen=100)  # Guarda los ultimos 100 datos recibidos

## @brief Extrae valores numericos flotantes precedidos por '*G' de un texto.
#  @param texto Cadena de texto recibida desde el dispositivo Bluetooth.
#  @return Lista de valores flotantes extraidos.
def extraer_valores(texto):
    # Busca todos los numeros despues de '*G' nota: asume que los datos estan en el formato '*G123.45'
    return [float(x) for x in re.findall(r'\*G(-?\d+\.\d+)', texto)]

## @brief Callback para manejar los datos recibidos por Bluetooth.
#  @param sender Identificador del emisor.
#  @param data Datos recibidos en formato bytes.
def handle_data(sender, data):
    texto = data.decode(errors='ignore')
    valores = extraer_valores(texto)
    data_buffer.extend(valores)

## @brief Corrutina que actualiza en tiempo real la grafica con los datos recibidos.
async def plot_data():
    plt.ion()
    fig, ax = plt.subplots()
    line, = ax.plot([], [])
    ax.set_ylim(-200, 500)  
    ax.set_xlim(0, 100)
    while True:
        if data_buffer:
            ydata = list(data_buffer)
            xdata = list(range(len(ydata)))
            line.set_xdata(xdata)
            line.set_ydata(ydata)
            ax.relim()
            ax.autoscale_view()
            plt.draw()
            plt.pause(0.001)
        await asyncio.sleep(0.001)

## @brief Funcion principal que gestiona la conexion Bluetooth y la visualizacion.
async def main():
    async with BleakClient(address) as client:
        await client.start_notify(char_uuid, handle_data)
        plot_task = asyncio.create_task(plot_data())
        await asyncio.sleep(100)
        await client.stop_notify(char_uuid)
        plot_task.cancel()

# Ejecuta la funcion principal
asyncio.run(main())