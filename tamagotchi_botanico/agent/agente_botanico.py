import os
import requests
import csv
from datetime import datetime
from dotenv import load_dotenv
from langchain_core.tools import tool
from langchain_openai import ChatOpenAI
from langchain.agents import create_agent
from langchain_core.messages import HumanMessage

# Cargar variables de entorno
load_dotenv()

@tool
def consultar_sensor_planta() -> str:
    """
    Útil cuando necesitas saber el estado actual de humedad de la planta.
    Se conecta al microcontrolador ESP32 mediante la red local.
    """
    try:
        respuesta = requests.get("http://tamagotchi.local/estado", timeout=5)
        return respuesta.text
    except requests.exceptions.RequestException as e:
        return f"Error físico: No se pudo contactar al sensor: {e}"
    
@tool
def registrar_en_archivo_csv(humedad: int, diagnostico: str) -> str:
    """
    Útil para guardar el historial de salud de la planta.
    Debe llamarse despúes de obtener la humedad y el diagnóstico del LLM.
    """
    archivo = "historial_planta.csv"
    cabecera = ["Fecha", "Humedad (%)", "Diagnóstico"]
    
    existe = os.path.exists(archivo)
    
    try:
        with open(archivo, mode = "a", newline = "", encoding = "utf-8") as f:
            writer = csv.writer(f)
            if not existe:
                writer.writerow(cabecera)
            # Guardando los datos
            fecha_iso = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
            writer.writerow([fecha_iso, humedad, diagnostico])
        return f"Éxito: Registro guardado en {archivo} el {fecha_iso}"
    except Exception as e:
        return f"Error al guardar el registro: {e}"

    
herramientas = [consultar_sensor_planta, registrar_en_archivo_csv]

llm = ChatOpenAI(model="gpt-4o-mini", temperature=0)

instrucciones_sistema = """
Eres un Ingeniero Agrónomo y Botánico experto encargado de monitorear un tamagotchi botánico.
Tu flujo de trabajo OBLIGATORIO es:
1. Consultar el sensor de la planta.
2. Analizar los datos de humedad del sensor (dada en porcentaje) y proporcionar un diagnóstico preciso; da sugerencias teniendo en cuenta que la planta es una vid (vitis vinifera). 
3. UTILIZAR la herramienta "registrar_en_archivo_csv" para guardar el porcentaje y tu diagnóstico final.
4. Responder al usuario confirmando que los datos han sido registrados.
"""

agente = create_agent(llm, herramientas, system_prompt=instrucciones_sistema)


def evaluar_planta():
    print("Iniciando escaneo agéntico...\n")
    
    inputs = {"messages": [HumanMessage(content="¿Cuál es el estado actual de mi planta? Toma una decisión.")]}
    
    for s in agente.stream(inputs, stream_mode="values"):
        mensaje = s["messages"][-1]
        
        if mensaje.type == "ai" and mensaje.tool_calls:
            print(f"[Agente decidiendo] -> Invocando herramienta: {mensaje.tool_calls[0]["name"]}")
        elif mensaje.type == "tool":
            print(f"[Hardware] -> Dator recibidos del ESP32: {mensaje.content}")
        elif mensaje.type == "ai" and not mensaje.tool_calls:
            print(f"\n[Diagnóstico Final]:\n{mensaje.content}")
            
if __name__ == "__main__":
    evaluar_planta()