import os
import sys
import csv
import requests
from datetime import datetime
from typing import Optional
from fastapi import FastAPI, HTTPException
from pydantic import BaseModel
from dotenv import load_dotenv
from langchain_core.messages import HumanMessage
from langchain_openai import ChatOpenAI


sys.path.insert(0, os.path.join(os.path.dirname(__file__), ".."))
os.chdir(os.path.join(os.path.dirname(__file__), ".."))

load_dotenv(dotenv_path="../agent/.env")

app = FastAPI(title="Tamagotchi Botánico API", version="1.0.0")

DATA_DIR = os.path.join(os.path.dirname(__file__), "..", "data")
HISTORIAL_FILE = os.path.join(DATA_DIR, "historial_planta.csv")
ESP32_URL = "http://tamagotchi.local/estado"


def get_actual_humedad() -> dict:
    try:
        response = requests.get(ESP32_URL, timeout=5)
        response.raise_for_status()
        return response.json()
    except requests.exceptions.RequestException as e:
        raise HTTPException(status_code=502, detail=f"Error al conectar al ESP32: {e}")


def get_historial() -> list:
    if not os.path.exists(HISTORIAL_FILE):
        return []
    
    historial = []
    with open(HISTORIAL_FILE, mode="r", encoding="utf-8") as f:
        reader = csv.DictReader(f)
        for row in reader:
            historial.append({
                "fecha": row.get("Fecha", ""),
                "humedad": int(row.get("Humedad (%)", 0)),
                "diagnostico": row.get("Diagnóstico", "")
            })
    return historial


def evaluar_planta() -> dict:
    llm = ChatOpenAI(model="gpt-4o-mini", temperature=0)
    
    humidity_data = get_actual_humedad()
    humedad = humidity_data.get("humedad_porcentaje", 0)
    
    system_prompt = """
Eres un Ingeniero Agrónomo y Botánico experto encargado de monitorear un tamagotchi botánico.
Analiza la humedad de la planta (dada en porcentaje) y proporciona un diagnóstico preciso.
La planta es una vid (vitis vinifera).
Respondé en español con recomendaciones claras."""
    
    user_prompt = f"La humedad actual del sensor es {humedad}%. ¿Cuál es el estado de la planta? ¿Qué recomendaciones das?"
    
    response = llm.invoke([{"role": "system", "content": system_prompt}, {"role": "user", "content": user_prompt}])
    
    diagnostico = response.content if hasattr(response, "content") else str(response)
    
    _save_to_historial(humedad, diagnostico)
    
    return {
        "humedad": humedad,
        "diagnostico": diagnostico,
        "timestamp": datetime.now().isoformat()
    }


def _save_to_historial(humedad: int, diagnostico: str):
    os.makedirs(DATA_DIR, exist_ok=True)
    
    existe = os.path.exists(HISTORIAL_FILE)
    cabecera = ["Fecha", "Humedad (%)", "Diagnóstico"]
    
    with open(HISTORIAL_FILE, mode="a", newline="", encoding="utf-8") as f:
        writer = csv.writer(f)
        if not existe:
            writer.writerow(cabecera)
        fecha_iso = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        writer.writerow([fecha_iso, humedad, diagnostico])


@app.get("/api/estado")
def api_estado():
    """Consulta el estado actual de humedad del sensor."""
    return get_actual_humedad()


@app.get("/api/historial")
def api_historial():
    """Retorna el historial de lecturas."""
    return {"historial": get_historial()}


@app.post("/api/evaluar")
def api_evaluar():
    """Evalúa la planta usando el agente IA."""
    return evaluar_planta()


if __name__ == "__main__":
    import uvicorn
    uvicorn.run(app, host="0.0.0.0", port=8000)