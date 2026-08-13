# PlacarPro

Placar eletrônico de tênis de mesa com atualização remota (OTA via GitHub).

## Estrutura

- `tenis_de_mesa/` — firmware ESP32 (Arduino IDE 1.8.19, board `ESP32 Dev Module`)
  - `tenis_de_mesa.ino` — código principal
  - `placar_html.h` — página web embarcada
- `.github/workflows/firmware.yml` — GitHub Actions: compila e anexa `firmware.bin` ao Release de cada tag `v*`

## Como publicar uma nova versão do firmware

1. Altere a versão no `.ino`:
   ```cpp
   const String FIRMWARE_VER = "1.0.1";
   ```
2. Commit e crie uma tag:
   ```bash
   git add -A
   git commit -m "v1.0.1: descrição"
   git push
   git tag v1.0.1
   git push origin v1.0.1
   ```
3. A GitHub Actions compila e cria o Release automaticamente (aba **Actions**).
4. Cada mesa ESP32 conectada à internet procura por versão nova 1x por dia e instala sozinha.

> ⚠️ A tag do Release DEVE ser `v` + o mesmo número de `FIRMWARE_VER`. O ESP só baixa versões **maiores** que a instalada (compara `x.y.z`).

## Requisitos

- ESP32 com flash de 4MB (padrão)
- Tabela de partição **Default 4MB** (com 2 slots OTA) — padrão do Arduino IDE
- Placa conectada ao roteador do centro (STA) com acesso à internet
- Repositório público (o ESP lê `api.github.com/repos/.../releases/latest` sem login)

## Atenção

- A primeira gravação de cada placa é feita **via USB** (Arduino IDE > Upload). Depois disso, as atualizações são **O-TA**.
- O servidor do campeonato (modo pago) **não** está neste repositório: ele fica fora do escopo público.