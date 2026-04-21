# Tamagotchi Botánico 🌿

Sistema de monitoreo de humedad de vid (vitis vinifera) utilizando ESP32-S3 con sensor resistivo, API REST, y agente IA con LangChain.

## Descripción

Este proyecto implementa un "tamagotchi botánico" que monitorea automáticamente la humedad del suelo de una planta de vid. El ESP32-S3 realiza lecturas periódicas del sensor y las expone via HTTP. Un agente IA basado en LangChain analiza los datos y proporciona diagnósticos agronómicos. Un frontend FastAPI permite consultar el estado y el historial desde cualquier navegador.

## Arquitectura

```
┌─────────────┐     WiFi      ┌──────────────┐     HTTP      ┌────────────┐
│  ESP32-S3  │ ────────────> │   FastAPI    │ <───────────> │   Browser  │
│  (Sensor)  │               │  (Frontend) │               │  (/docs)   │
└─────────────┘               └──────┬───────┘               └────────────┘
                                     │
                                     v
                              ┌────────────┐
                              │  LangChain │
                              │ GPT-4o-mini│
                              └────────────┘
```

## Hardware

| Componente | Pin | Notas |
|------------|-----|-------|
| Sensor humedad suelo | GPIO10 (ADC Channel 9) | Resistivo |
| Alimentación | 3.3V / GND | - |

**Configuración ADC:**
- Resolución: 12 bits
- Atenuación: 0-3.3V (ADC_ATTEN_DB_12)
- Lecturas por ciclo: 10 muestras promediadas
- Intervalo: 60 segundos

**Fórmula de humedad:**
```
humedad (%) = 100 - ((rawADC - 1300) / (3350 - 1300) * 100)
```
Nota: Escala invertida — húmedo = alto %, seco = bajo %.

## Instalación

### 1. Flashear el ESP32

```bash
cd tamagotchi_botanico
idf.py set-target esp32s3  # Solo si no está configurado
idf.py build
idf.py -p /dev/ttyACM0 flash monitor
```

### 2. Instalar dependencias Python

```bash
cd tamagotchi_botanico
pip install -r requirements.txt
```

### 3. Configurar variables de entorno

Crear archivo `.env` en `tamagotchi_botanico/`:

```env
OPENAI_API_KEY=sk-tu-clave-aqui
```

## Uso

### Frontend FastAPI

```bash
cd tamagotchi_botanico
python frontend/main.py
```

Abrir en el navegador: **http://localhost:8000/docs**

### Agente IA (línea de comandos)

```bash
cd tamagotchi_botanico
python agent/agente_botanico.py
```

## API Endpoints

### GET /api/estado

Retorna la humedad actual leída del sensor ESP32.

```bash
curl http://localhost:8000/api/estado
```

**Respuesta:**
```json
{"humedad_porcentaje": 65}
```

### GET /api/historial

Retorna el historial de evaluaciones guardadas.

```bash
curl http://localhost:8000/api/historial
```

**Respuesta:**
```json
{
  "historial": [
    {
      "fecha": "2026-04-21 10:30:00",
      "humedad": 65,
      "diagnostico": "La planta se encuentra en óptimas condiciones..."
    }
  ]
}
```

### POST /api/evaluar

Dispara el agente IA para obtener un diagnóstico completo basado en la humedad actual.

```bash
curl -X POST http://localhost:8000/api/evaluar
```

**Respuesta:**
```json
{
  "humedad": 65,
  "diagnostico": "La planta se encuentra en óptimas condiciones de humedad...",
  "timestamp": "2026-04-21T10:30:00"
}
```

## Estructura del Proyecto

```
esp_tamagotchi/
├── tamagotchi_botanico/
│   ├── main/
│   │   └── main.c          # Firmware ESP32
│   ├── agent/
│   │   └── agente_botanico.py  # Agente LangChain CLI
│   ├── frontend/
│   │   └── main.py      # FastAPI frontend
│   ├── data/
│   │   └── historial_planta.csv  # Historial de evaluaciones
│   ├── requirements.txt
│   └── CMakeLists.txt
├── AGENTS.md
└── README.md
```

## Requisitos

- **Hardware:** ESP32-S3 con ADC
- **Firmware:** ESP-IDF 6.0.0
- **Python:** 3.10+
- **API Key:** OpenAI (GPT-4o-mini)

## Troubleshooting

| Problema | Solución |
|---------|---------|
| ESP32 no conecta a WiFi | Verificar credenciales en `main.c:23-24` |
| Error 502 en FastAPI | Verificar que ESP32 esté en la misma red |
| Error "OpenAI" | Verificar `OPENAI_API_KEY` en `.env` |
| `/api/historial` vacío | Ejecutar `/api/evaluar` al menos una vez |

## Licencia

MIT