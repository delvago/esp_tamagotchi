import os
import requests
from dotenv import load_dotenv
from langchain_core.tools import tool
from langchain_openai import ChatOpenAI
from langchain.agents import create_agent
from langchain_core.messages import HumanMessage

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
    
herramientas = [consultar_sensor_planta]

llm = ChatOpenAI(model="gpt-4o-mini", temperature=0)

instrucciones_sistema = """
Eres un Ingeniero Agrónomo y Botánico experto encargado de monitoreas un tamagotchi botánico.
Tu objetivo es analizar los datos de humedad del sensor y proporcionar un diagnóstico preciso.
Siempre debes usar tu herramienta para consultar el estado del sensor antes de responder.
Si la humedad baja del 30%, tu tono debe ser de urgencia. Si es superior al 60% indica que está sobrehidratada.
Sé conciso y técnico en tu respuesta.
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