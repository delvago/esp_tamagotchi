# esp_tamagotchi

ESP32-S3 botanical sensor with WiFi HTTP API + Python AI agent (grapevine monitoring).

## Build & Flash

```bash
cd tamagotchi_botanico
idf.py build
idf.py -p /dev/ttyACM0 flash monitor
```

## Key Details

- **Target**: ESP32-S3, ADC channel 9 (GPIO10)
- **Reading interval**: 60s (line 227: `vTaskDelay(pdMS_TO_TICKS(60000))`)
- **Humidity formula**: `100 - ((raw - 1300) / (3350 - 1300) * 100)` — inverted scale (wet = high %)
- **API**: `GET http://tamagotchi.local/estado` → `{"humedad_porcentaje": N}`
- **mDNS hostname**: `tamagotchi`

## Python Agent

```bash
cd tamagotchi_botanico
pip install -r requirements.txt
# Create .env: OPENAI_API_KEY=sk-...
python agent/agente_botanico.py
```

## Gotchas

- WiFi credentials hardcoded in `main.c:23-24`
- Agent expects grapevine (vitis vinifera) in system prompt
- CSV output: `data/historial_planta.csv` (moved from CWD)
- Build artifacts in `tamagotchi_botanico/build/`

## FastAPI Frontend

```bash
cd tamagotchi_botanico
pip install -r requirements.txt
python frontend/main.py
# Swagger UI: http://localhost:8000/docs
```

| Endpoint | Method | Description |
|----------|--------|------------|
| `/api/estado` | GET | Current humidity from ESP32 |
| `/api/historial` | GET | Historical readings |
| `/api/evaluar` | POST | Trigger IA diagnosis |
