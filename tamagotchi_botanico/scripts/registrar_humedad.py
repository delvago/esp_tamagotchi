import os
import csv
import requests
from datetime import datetime

ESP32_URL = "http://tamagotchi.local/estado"
DATA_DIR = "data"
HISTORIAL_FILE = "data/humedad_hora.csv"


def main():
    os.makedirs(DATA_DIR, exist_ok=True)

    try:
        response = requests.get(ESP32_URL, timeout=5)
        response.raise_for_status()
        humedad = response.json().get("humedad_porcentaje", 0)
    except Exception as e:
        print(f"Error: {e}")
        return

    fecha = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    existe = os.path.exists(HISTORIAL_FILE)

    with open(HISTORIAL_FILE, mode="a", newline="", encoding="utf-8") as f:
        writer = csv.writer(f)
        if not existe:
            writer.writerow(["Fecha", "Humedad (%)"])
        writer.writerow([fecha, humedad])

    print(f"Registrado: {fecha} -> {humedad}%")


if __name__ == "__main__":
    main()