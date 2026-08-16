#include "placar_html.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <Preferences.h>
#include <DNSServer.h>
#include <esp_mac.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <WiFiClientSecure.h>
#include <HTTPUpdate.h>
#include <esp_ota_ops.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Configuração persistente (NVS) — configurada UMA vez por centro pela tela
// de configuração (abre sozinha na primeira vez). NÃO precisa recompilar.
String mesaSsid = "Placar_Tenis_Mesa";
String mesaSenha = "12345678Super";
int mesaCanal = 0;   // 0 = automático (escolhe 1/6/11 livre no boot)
String rotSsid = "";
String rotSenha = "";
String srvIP = "192.168.0.10";
String nomeMesa = "";   // nome personalizado (ex.: "Mesa 01"); vazio = Placar-<MAC>
String ultimoStaIP = "";
unsigned long ultimaQuedaSta = 0;
unsigned long ultimoLogSta = 0;
const unsigned long intervaloLogSta = 20000;
String configSenha = "1234";
bool configIgnorada = false;

WiFiUDP udp;
unsigned long ultimoProcurarServidor = 0;
const unsigned long intervaloProcurarServidor = 15000;

Preferences prefs;

// Detecção automática do servidor de campeonato (notebook):
// em modo standalone fica em 5dBm (economia); quando o servidor é detectado, sobe para 11dBm.
bool campeonatoDetectado = false;
int formatoGrupos = 0;   // 0 = livre; senão melhor de N (consultado do servidor)
int formatoMata = 0;
int formatoFinal = 0;
int formatoAtual = 0;    // formato efetivo da partida chamada (0 = livre)
String modoCampeonato = "absoluto";
unsigned long ultimoCheckCampeonato = 0;
const unsigned long intervaloCheckCampeonato = 6000;

// Atualização OTA via GitHub (repo público Jaum4010/PlacarPro)
const String GITHUB_REPO = "Jaum4010/PlacarPro";   // usuário/repositório
const String FIRMWARE_VER = "1.1.7";               // versão deste firmware (tags do repo: v1.0.0, v1.0.1, ...)
const unsigned long INTERVALO_OTA = 24UL * 60UL * 60UL * 1000UL;  // procura nova versão a cada 24h
HTTPUpdate httpUpdatePro;
WiFiClientSecure otaClient;  // para HTTPS (GitHub obriga TLS)
unsigned long ultimoCheckOTA = 0;  // 0 => ainda nao checou
const unsigned long ATRASO_INICIAL_OTA = 5UL * 60UL * 1000UL;  // primeiro check 5min apos o boot

// OTA em segundo plano (task no core 0): o loop continua servindo a mesa enquanto baixa.
volatile bool otaEmAndamento = false;
String otaStatus = "idle";   // idle | verificando | baixando | atualizado | erro: <motivo>
volatile int otaProgresso = -1;  // -1 = sem progresso; 0..100 durante o download
const unsigned long RETRY_OTA = 10UL * 60UL * 1000UL;  // se falhar sem rede, tenta de novo a cada 10min (nao espera 24h)
bool otaCheckFeito = false;   // true quando uma checagem realmente alcancou o GitHub
bool otaAlcancouGithub = false;

const int pinoBotaoA = 18; 
const int pinoBotaoB = 19; 
const int pinoBateria = 32;     // ADC1: leitura da bateria (divisor 100k/100k)
const int pinoLedBateria = 25;  // LED vermelho de carga baixa (apaga quando cheia)

float tensaoBateria = 0.0;
int percentualBateria = -1;
unsigned long ultimoLeituraBateria = 0;
const unsigned long intervaloLeituraBateria = 5000;
unsigned long ultimoEnvioBateria = 0;
const unsigned long intervaloEnvioBateria = 30000;
volatile bool envioBateriaEmAndamento = false;
unsigned long ultimoEnvioBateriaBg = 0;

int pontosA = 0, pontosB = 0, quemSaca = 1, sacadorInicial = 1; 
int setsA = 0, setsB = 0, totalSetsJogados = 0;
bool jogoFinalizado = false, jogoIniciado = false, sorteioRealizado = false, setFechado = false;
bool botoesFisicosInvertidos = false;   // config persistente (NVS): botões instalados com os lados trocados na mesa
bool botoesInvertidos = botoesFisicosInvertidos;          // reflete botoesFisicosInvertidos; usado no loop físico (nunca é alternado por TROCAR LADO)
bool aguardandoInicio = false;   // tela "VS" (partida chamada, ainda não iniciou o saque) 
String nomeJogadorA = "Jogador A", nomeJogadorB = "Jogador B", msgStatus = "";
String campeaoAtual = "";
String avisoAtual = "";
const int VERSAO_PAGINA = 28;   // incrementar a cada mudanca no JS servido (placar_html.h)
String historicoArquivo = "";
String historicoJogoAtual = "";
String setsDetalhados = "";

bool partidaPendenteErro = false;
String pendNomeA = "", pendNomeB = "", pendHistorico = "", pendSetsDetalhados = "";
int pendSetsA = 0, pendSetsB = 0, pendTotalSets = 0, pendSaque = 1, pendSacadorInicial = 1;

// Confirmação do resultado antes de enviar ao campeonato
bool aguardandoConfirmacao = false;
String confNA = "", confNB = "", confDet = "";
int confSA = 0, confSB = 0;
String preHist = "", preDet = "", preAviso = "";
int preSA = 0, preSB = 0, prePA = 0, prePB = 0, preTotal = 0, preSaque = 1, preSacIni = 1;
bool preSF = false;

unsigned long tempoApertadoA = 0, tempoApertadoB = 0;
bool processadoLongPressA = false, processadoLongPressB = false;
const int tempoLongPress = 1000; 

DNSServer dnsServer;
WebServer server(80);
WiFiServer sseServer(82);
#define MAX_SSE 6
#define MAX_DISP 8
WiFiClient sseClients[MAX_SSE];
struct Dispositivo { IPAddress ip; unsigned long ultimoPing; };
Dispositivo disp[MAX_DISP];
bool flagNotificar = false;
unsigned long ultimoHeartbeat = 0;
unsigned long ultimoCheckDisp = 0;
unsigned long ultimoClienteAtivo = 0;
const int intervaloHeartbeat = 5000;
const int tempoResetSemClientes = 30000;
const unsigned long tempoVidaPing = 10000;

void registrarPingIP(IPAddress ip) {
  unsigned long agora = millis();
  for (int i = 0; i < MAX_DISP; i++) {
    if (disp[i].ultimoPing && disp[i].ip == ip) { disp[i].ultimoPing = agora; return; }
  }
  for (int i = 0; i < MAX_DISP; i++) {
    if (!disp[i].ultimoPing) { disp[i].ip = ip; disp[i].ultimoPing = agora; return; }
  }
}

int contarClientes() {
  int n = 0;
  unsigned long agora = millis();
  for (int i = 0; i < MAX_DISP; i++) {
    if (disp[i].ultimoPing && agora - disp[i].ultimoPing < tempoVidaPing) n++;
  }
  return n;
}

bool clienteAtivo(int idx) {
  if (!sseClients[idx] || !sseClients[idx].connected()) return false;
  IPAddress ip = sseClients[idx].remoteIP();
  unsigned long agora = millis();
  for (int j = 0; j < MAX_DISP; j++) {
    if (disp[j].ultimoPing && disp[j].ip == ip && agora - disp[j].ultimoPing < tempoVidaPing) return true;
  }
  return false;
}

bool partidaFinalizada();

void enviarDadosJSON(WebServer &srv) {
  String j = "{\"ini\":" + String(jogoIniciado ? "true" : "false") + ",";
  j += "\"srt\":" + String(sorteioRealizado ? "true" : "false") + ",";
  j += "\"alg\":" + String(aguardandoInicio ? "true" : "false") + ",";
  j += "\"nA\":\"" + nomeJogadorA + "\",";
  j += "\"nB\":\"" + nomeJogadorB + "\",";
  j += "\"pA\":" + String(pontosA) + ",";
  j += "\"pB\":" + String(pontosB) + ",";
  j += "\"sA\":" + String(setsA) + ",";
  j += "\"sB\":" + String(setsB) + ",";
  j += "\"sq\":" + String(quemSaca) + ",";
  j += "\"sf\":" + String(setFechado ? "true" : "false") + ",";
  j += "\"msg\":\"" + msgStatus + "\",";
  j += "\"aviso\":\"" + avisoAtual + "\",\"pend\":" + String(partidaPendenteErro ? "true" : "false") + ",";
  j += "\"h\":\"" + historicoCompleto() + "\",";
  j += "\"ncl\":" + String(contarClientes()) + ",";
  j += "\"b\":" + String(percentualBateria) + ",";
  j += "\"btv\":" + String(tensaoBateria, 2) + ",";
  j += "\"sta\":" + String(WiFi.status() == WL_CONNECTED ? "true" : "false") + ",";
  j += "\"staIP\":\"" + (WiFi.status() == WL_CONNECTED ? WiFi.localIP().toString() : "") + "\"";
  j += ",\"camp\":\"" + escapeJson(campeaoAtual) + "\"";
  j += ",\"cmp\":" + String(campeonatoDetectado ? "true" : "false") + "";
  j += ",\"fin\":" + String(partidaFinalizada() ? "true" : "false");
  j += ",\"ver\":" + String(VERSAO_PAGINA);
  j += ",\"ota\":\"" + otaStatus + "\",\"otap\":" + String(otaProgresso);
  j += "}";
  srv.send(200, "application/json", j);
}

void registrarSetHistorico() {
  if (historicoJogoAtual != "") historicoJogoAtual += "<br>";
  if (setsDetalhados != "") setsDetalhados += ", ";
  setsDetalhados += String(pontosA) + "x" + String(pontosB);
  if (pontosA > pontosB) {
    historicoJogoAtual += "Set " + String(totalSetsJogados + 1) + ": <b style='color:#ffcc00'>" + nomeJogadorA + "</b> " + String(pontosA) + " x " + String(pontosB) + " " + nomeJogadorB;
  } else {
    historicoJogoAtual += "Set " + String(totalSetsJogados + 1) + ": " + nomeJogadorA + " " + String(pontosA) + " x " + String(pontosB) + " <b style='color:#ffcc00'>" + nomeJogadorB + "</b>";
  }
}

void finalizarJogoAtual() {
  if (setFechado) {
    registrarSetHistorico();
    if (pontosA > pontosB) setsA++; else setsB++;
  }
  if (historicoJogoAtual == "") return;
  int totalSets = setsA + setsB;
  bool invalida = false;
  String motivo = "";
  if (setsA == setsB) {
    invalida = true;
    motivo = "empate";
  } else if (campeonatoDetectado) {
    int fmt = (formatoAtual > 0) ? formatoAtual : ((modoCampeonato == "chave") ? formatoMata : formatoGrupos);
    if (fmt > 0 && max(setsA, setsB) < (fmt + 1) / 2) {
      invalida = true;
      motivo = "vencedor sem " + String((fmt + 1) / 2) + " sets";
    }
  }
  if (totalSets > 0 && invalida) {
    pendNomeA = nomeJogadorA; pendNomeB = nomeJogadorB;
    pendSetsA = setsA; pendSetsB = setsB; pendTotalSets = totalSetsJogados;
    pendSaque = quemSaca; pendSacadorInicial = sacadorInicial;
    pendHistorico = historicoJogoAtual;
    pendSetsDetalhados = setsDetalhados;
    partidaPendenteErro = true;
    avisoAtual = "ATENÇÃO: partida " + String(setsA) + " x " + String(setsB) + " é inválida (" + motivo + "). Toque em VOLTAR À PARTIDA para corrigir.";
    return;
  }
  partidaPendenteErro = false;
  String bloco = "";
  if (setsA > setsB) {
    bloco += "<b style='color:#ffcc00'>" + nomeJogadorA + "</b> VENCEU A PARTIDA " + String(setsA) + " x " + String(setsB);
  } else if (setsB > setsA) {
    bloco += "<b style='color:#ffcc00'>" + nomeJogadorB + "</b> VENCEU A PARTIDA " + String(setsB) + " x " + String(setsA);
  } else {
    bloco += "PARTIDA EMPATADA " + String(setsA) + " x " + String(setsB);
  }
  bloco += "<br>" + historicoJogoAtual;
  if (historicoArquivo != "") bloco += "<br><br>";
  historicoArquivo = bloco + historicoArquivo;
  historicoJogoAtual = "";
}

String historicoCompleto() {
  if (historicoJogoAtual == "") return historicoArquivo;
  if (historicoArquivo == "") return historicoJogoAtual;
  return historicoJogoAtual + "<br><br>" + historicoArquivo;
}

// Fila de resultados finalizados (o notebook puxa, em vez de depender do POST)
#define MAX_FILA_RESULT 10
struct ResultadoFila { unsigned long seq; String jogA, jogB, sets; int sA, sB; };
ResultadoFila filaResultados[MAX_FILA_RESULT];
int filaHead = 0, filaTail = 0;
unsigned long seqResultado = 0;
unsigned long seqEntregue = 0;

void enfileirarResultado() {
  if (setsA + setsB == 0) return;
  if (filaTail - filaHead >= MAX_FILA_RESULT) { filaHead = filaTail - MAX_FILA_RESULT; }
  int i = filaTail % MAX_FILA_RESULT;
  seqResultado++;
  filaResultados[i].seq = seqResultado;
  filaResultados[i].jogA = nomeJogadorA;
  filaResultados[i].jogB = nomeJogadorB;
  filaResultados[i].sA = setsA;
  filaResultados[i].sB = setsB;
  filaResultados[i].sets = setsDetalhados;
  filaTail++;
  Serial.print("Resultado na fila p/ o campeonato (#"); Serial.print(seqResultado);
  Serial.print("): "); Serial.print(nomeJogadorA); Serial.print(" x "); Serial.println(nomeJogadorB);
}

// Fecha a partida confirmada: envia ao campeonato e zera a mesa.
void confirmarPartida() {
  if (aguardandoConfirmacao) {
    nomeJogadorA = confNA; nomeJogadorB = confNB;
    setsDetalhados = confDet;
    setsA = confSA; setsB = confSB;
  } else if (setFechado) {
    registrarSetHistorico();
    totalSetsJogados++;
    if (pontosA > pontosB) setsA++; else setsB++;
  }
  if (setsA + setsB > 0 && !partidaPendenteErro) {
    aguardandoConfirmacao = false;
    enviarResultadoCampeonato();
    partidaPendenteErro = false;
  }
  pontosA = 0; pontosB = 0; jogoIniciado = false; sorteioRealizado = false; setFechado = false;
  aguardandoInicio = false;
  setsA = 0; setsB = 0; totalSetsJogados = 0; setsDetalhados = "";
  historicoJogoAtual = ""; avisoAtual = ""; msgStatus = "";
  quemSaca = 1; sacadorInicial = 1;
  botoesInvertidos = botoesFisicosInvertidos;
}

String escapeJson(String s) {
  s.replace("\\", "\\\\"); s.replace("\"", "\\\""); s.replace("\n", " ");
  return s;
}

void enviarResultadoCampeonato() {
  // O notebook agora puxa os resultados (GET /api/coleta) e confirma (POST /api/coletado).
  enfileirarResultado();
}

void lerBateria() {
  long soma = 0;
  for (int i = 0; i < 24; i++) { soma += analogRead(pinoBateria); delay(3); }
  long leitura = soma / 24;
  long mV = leitura * 3300L / 4095L;            // tensão no pino ADC (0-3.3V)
  tensaoBateria = mV * 2.0 / 1000.0;           // divisor 100k/100k = tensão real x2
  if (tensaoBateria < 0.70) {                    // sem bateria (só USB): oculta o indicador
    percentualBateria = -1;
  } else if (tensaoBateria >= 4.20) { percentualBateria = 100; }
  else if (tensaoBateria <= 3.20) { percentualBateria = 0; }
  else { percentualBateria = (int)((tensaoBateria - 3.20) * 100.0); }
  if (percentualBateria != -1) {
    if (percentualBateria < 0) percentualBateria = 0;
    if (percentualBateria > 100) percentualBateria = 100;
  }
  digitalWrite(pinoLedBateria, (tensaoBateria >= 0.70 && percentualBateria < 20) ? HIGH : LOW);
}

void enviarBateria() {
  if (WiFi.status() != WL_CONNECTED) return;
  lerBateria();
  int codigo = -1;
  for (int tentativa = 0; tentativa < 2; tentativa++) {
    HTTPClient http;
    http.setTimeout(1500);
    http.begin("http://" + srvIP + ":5000/api/bateria");
    http.addHeader("Content-Type", "application/json");
    String payload = "{\"mesa\":\"" + ssidDaMesa() + "\",\"b\":" + String(percentualBateria)
                   + ",\"v\":\"" + String(tensaoBateria, 2) + "\",\"ip\":\"" + WiFi.localIP().toString() + "\"}";
    codigo = http.POST(payload);
    http.end();
    if (codigo >= 200 && codigo < 300) break;
    delay(120);  // nova tentativa se a API falhar momentaneamente
  }
  Serial.print("Bateria enviada ao campeonato: ");
  Serial.print(percentualBateria);
  Serial.print("%  HTTP ");
  Serial.println(codigo);
}

void tarefaEnvioBateria(void* param) {
  enviarBateria();
  envioBateriaEmAndamento = false;
  vTaskDelete(NULL);
}

String extrairCampoJSON(const String& js, const char* campo) {
  String chave = String("\"") + campo + "\":";
  int i = js.indexOf(chave);
  if (i < 0) return "";
  i += chave.length();
  while (i < (int)js.length() && (js[i] == ' ' || js[i] == '\t' || js[i] == '\r' || js[i] == '\n')) i++;
  if (i >= (int)js.length() || js[i] != '"') return "";
  i++;
  int f = js.indexOf('"', i);
  if (f < 0) return "";
  return js.substring(i, f);
}

void verificarCampeonato() {
  // Fluxo invertido: o notebook conduz via puxada/empurrada.
  // A chamada da partida e o cancelamento chegam por POST /api/ordem;
  // os resultados sao puxados por GET /api/coleta. Nao puxamos mais
  // /api/estado nem /api/proxima_partida (direcao que o roteador bloqueia).
}

// ---- Atualização OTA (GitHub) ----
bool mesaLivreOta() {
  // Nao atualizar no meio de uma partida ou com resultado pendente p/ enviar.
  if (jogoIniciado || aguardandoInicio || aguardandoConfirmacao || partidaPendenteErro) return false;
  if (filaHead != filaTail) return false;  // resultado na fila aguardando o notebook puxar
  return true;
}

// Compara "1.0.0" vs "1.2.0". Retorna <0 se a<b, 0 se igual, >0 se a>b.
int compararVersoes(const String& a, const String& b) {
  int pa = 0, pb = 0;
  while (true) {
    int ia = a.indexOf('.', pa);
    int ib = b.indexOf('.', pb);
    String sa = (ia < 0) ? a.substring(pa) : a.substring(pa, ia);
    String sb = (ib < 0) ? b.substring(pb) : b.substring(pb, ib);
    int na = atoi(sa.c_str());
    int nb = atoi(sb.c_str());
    if (na != nb) return (na < nb) ? -1 : 1;
    if (ia < 0 && ib < 0) return 0;
    if (ia < 0) return -1;
    if (ib < 0) return 1;
    pa = ia + 1; pb = ib + 1;
  }
}

bool verificarVersaoGithub(String& tagNova) {
  if (WiFi.status() != WL_CONNECTED) return false;
  HTTPClient http;
  otaClient.setInsecure();  // não valida o certificado (aceitável p/ esta aplicação)
  String url = String("https://api.github.com/repos/") + GITHUB_REPO + "/releases/latest";
  if (!http.begin(otaClient, url)) return false;
  http.setTimeout(5000);
  http.addHeader("User-Agent", "PlacarTenisMesa");  // API do GitHub exige User-Agent
  int codigo = http.GET();
  String corpo = http.getString();
  http.end();
  otaAlcancouGithub = true;   // chegou a responder (mesmo que HTTP != 200): rede OK
  if (codigo != 200) {
    Serial.print("[OTA] github HTTP "); Serial.println(codigo);
    return false;
  }
  tagNova = extrairCampoJSON(corpo, "tag_name");
  if (tagNova.length() == 0) return false;
  String v = tagNova;
  if (v.startsWith("v")) v = v.substring(1);
  Serial.print("[OTA] versao disponivel: "); Serial.println(tagNova);
  return compararVersoes(v, FIRMWARE_VER) > 0;
}

bool baixarEAtualizar(const String& tag) {
  if (ESP.getFreeHeap() < 60 * 1024) {
    Serial.print("[OTA] heap insuficiente p/ baixar: "); Serial.println((int)ESP.getFreeHeap());
    otaStatus = "erro: heap baixo";
    return false;
  }
  httpUpdatePro.onProgress([](int cur, int total) {
    if (total > 0) { otaProgresso = cur * 100 / total; }
    delay(1);  // cede a CPU p/ as tarefas de WiFi/TCP não morrerem de fome
  });
  String url = String("https://github.com/") + GITHUB_REPO + "/releases/download/" + tag + "/firmware.bin";
  Serial.print("[OTA] baixando "); Serial.println(url);
  otaClient.setInsecure();
  httpUpdatePro.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  httpUpdatePro.rebootOnUpdate(false);  // decidimos o reinício aqui
  httpUpdatePro.setLedPin(-1);
  t_httpUpdate_return ret = httpUpdatePro.update(otaClient, url);
  if (ret == HTTP_UPDATE_OK) {
    Serial.println("[OTA] gravado com sucesso. Reiniciando...");
    otaStatus = "atualizado";
    otaProgresso = 100;
    ESP.restart();
    return true;
  }
  Serial.print("[OTA] falhou: "); Serial.println((int)ret);
  otaStatus = "erro: codigo " + String((int)ret);
  otaProgresso = -1;
  return false;
}

void tarefaOta(void* param) {
  String tag;
  otaStatus = "verificando";
  if (!verificarVersaoGithub(tag)) {
    otaStatus = otaAlcancouGithub ? "atualizado" : "erro: sem rede";
  } else {
    baixarEAtualizar(tag);
  }
  otaCheckFeito = otaAlcancouGithub;
  otaEmAndamento = false;
  vTaskDelete(NULL);
}

void iniciarOtaBackground() {
  // Segurança: nunca sobrepor duas tarefas de OTA e não baixar no meio de uma partida.
  if (otaEmAndamento) return;
  if (!mesaLivreOta()) return;
  if (WiFi.status() != WL_CONNECTED) { otaStatus = "erro: sem rede"; return; }
  otaEmAndamento = true;
  otaProgresso = -1;
  xTaskCreatePinnedToCore(tarefaOta, "otaTask", 8192, NULL, 1, NULL, 0);
}

void enviarBroadcastDescoberta() {
  int s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (s < 0) return;
  int en = 1;
  setsockopt(s, SOL_SOCKET, SO_BROADCAST, &en, sizeof(en));
  struct sockaddr_in dest;
  memset(&dest, 0, sizeof(dest));
  dest.sin_family = AF_INET;
  dest.sin_port = htons(7777);
  dest.sin_addr.s_addr = htonl(INADDR_BROADCAST);
  sendto(s, "CAMP_DESCOBERTA", 15, 0, (struct sockaddr *)&dest, sizeof(dest));
  close(s);
}

void procurarServidor() {
  if (WiFi.status() != WL_CONNECTED) return;
  String ipAtual = WiFi.localIP().toString();
  if (ipAtual != ultimoStaIP) {
    ultimoStaIP = ipAtual;
    salvarConfig();
  }
  if (millis() - ultimoProcurarServidor > intervaloProcurarServidor) {
    ultimoProcurarServidor = millis();
    enviarBroadcastDescoberta();
  }
  int n = udp.parsePacket();
  if (n >= 9) {
    char buf[16];
    int r = udp.read(buf, sizeof(buf) - 1);
    if (r > 0) {
      buf[r] = 0;
      if (strncmp(buf, "CAMP_AQUI", 9) == 0) {
        String novoIP = udp.remoteIP().toString();
        if (novoIP != srvIP) {
          srvIP = novoIP;
          salvarConfig();
          Serial.print("Servidor encontrado automaticamente: ");
          Serial.println(novoIP);
        }
      }
    }
    udp.flush();
  }
}

bool configurado() {
  return rotSsid.length() > 0 || configIgnorada;
}

bool partidaFinalizada() {
  int sa = setsA, sb = setsB;
  if (setFechado) { if (pontosA > pontosB) sa++; else sb++; }
  if (sa == sb) return false;
  if (!campeonatoDetectado) return false; // avulso: sem limite de sets, continua até clicar NOVO JOGO
  int fmt = (formatoAtual > 0) ? formatoAtual : ((modoCampeonato == "chave") ? formatoMata : formatoGrupos);
  if (fmt <= 0) return true;
  return max(sa, sb) >= (fmt + 1) / 2;
}

String ssidDaMesa() {
  if (nomeMesa.length() > 0) return nomeMesa;
  uint8_t mac[6] = {0};
  esp_read_mac(mac, ESP_MAC_WIFI_STA);
  char s[9];
  snprintf(s, sizeof(s), "%02X%02X", mac[4], mac[5]);
  return String("Placar-") + s;
}

String nomeMdns() {
  uint8_t mac[6];
  WiFi.macAddress(mac);
  char s[9];
  snprintf(s, sizeof(s), "%02X%02X", mac[4], mac[5]);
  String h = String("placar-") + s;
  h.toLowerCase();
  return h;
}

String macDaMesa() {
  uint8_t mac[6];
  WiFi.macAddress(mac);
  char s[18];
  snprintf(s, sizeof(s), "%02X:%02X:%02X:%02X:%02X:%02X",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  return String(s);
}

int escolherCanalAutomatico() {
  int canais[3] = {1, 6, 11};
  int n = -1;
  for (int t = 0; t < 6 && n < 0; t++) { n = WiFi.scanNetworks(); if (n < 0) delay(1000); }
  int melhor = 1, melhorCarga = -1;
  for (int c = 0; c < 3; c++) {
    int carga = 0;
    for (int i = 0; i < n; i++) {
      if (WiFi.channel(i) == canais[c]) {
        int r = WiFi.RSSI(i);
        if (r > -100) carga += (r + 100);
      }
    }
    if (melhorCarga < 0 || carga < melhorCarga) { melhorCarga = carga; melhor = canais[c]; }
  }
  return melhor;
}

void carregarConfig() {
  prefs.begin("placar", false);
  mesaSenha = prefs.getString("mesa_senha", "12345678Super");
  mesaCanal = prefs.getInt("mesa_canal", 0);
  rotSsid = prefs.getString("rot_ssid", "");
  rotSenha = prefs.getString("rot_senha", "");
  srvIP = prefs.getString("srv_ip", "192.168.0.10");
  nomeMesa = prefs.getString("mesa_nome", "");
  ultimoStaIP = prefs.getString("last_sta_ip", "");
  configSenha = prefs.getString("cfg_senha", "1234");
  configIgnorada = prefs.getBool("ignorada", false);
  botoesFisicosInvertidos = prefs.getBool("bot_inv", false);
  prefs.end();
  botoesInvertidos = botoesFisicosInvertidos;
  computarTokenConfig();
}

void salvarConfig() {
  prefs.begin("placar", false);
  prefs.putString("mesa_senha", mesaSenha);
  prefs.putInt("mesa_canal", mesaCanal);
  prefs.putString("rot_ssid", rotSsid);
  prefs.putString("rot_senha", rotSenha);
  prefs.putString("srv_ip", srvIP);
  prefs.putString("mesa_nome", nomeMesa);
  prefs.putString("last_sta_ip", ultimoStaIP);
  prefs.putString("cfg_senha", configSenha);
  prefs.putBool("ignorada", configIgnorada);
  prefs.putBool("bot_inv", botoesFisicosInvertidos);
  prefs.end();
}

String tokenConfig = "";

void computarTokenConfig() {
  uint32_t h1 = 2166136261u;
  for (unsigned int i = 0; i < configSenha.length(); i++) { h1 ^= (uint8_t)configSenha[i]; h1 *= 16777619u; }
  uint32_t h2 = 2166136261u;
  for (unsigned int i = 0; i < configSenha.length(); i++) { h2 ^= (uint8_t)(configSenha[i] ^ 0x5A); h2 *= 16777619u; }
  char buf[17];
  snprintf(buf, sizeof(buf), "%08X%08X", (unsigned)h1, (unsigned)h2);
  tokenConfig = String(buf);
}

bool autenticadoConfig() {
  String t = server.arg("tok");
  return t.length() > 0 && t == tokenConfig;
}

String paginaLoginHTML(const String& erro) {
  String h = "<!DOCTYPE html><html><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width,initial-scale=1.0'>";
  h += "<style>body{font-family:sans-serif;text-align:center;background:#1e1e24;color:#fff;margin:0;padding:15px}h1{color:#4CAF50;font-size:18px}";
  h += ".box{background:#2a2a35;padding:20px;border-radius:15px;max-width:420px;margin:15px auto;box-shadow:0 4px 15px #000}";
  h += "input{width:100%;padding:10px;margin:5px 0 12px;border-radius:8px;border:1px solid #555;background:#1e1e24;color:#fff;box-sizing:border-box}";
  h += "button{padding:12px;border:none;border-radius:8px;font-weight:bold;cursor:pointer;width:100%;background:#4CAF50;color:#fff;font-size:15px}";
  h += "label{font-size:12px;color:#aaa}.erro{color:#f88;font-size:13px;margin:6px 0}";
  h += "</style></head><body><h1>ACESSO RESTRITO</h1><div class='box'>";
  if (erro != "") h += "<div class='erro'>" + erro + "</div>";
  h += "<form method='POST' action='/login_camp'>";
  h += "<label>Código de acesso</label><input type='password' name='senha' required>";
  h += "<button>ENTRAR</button>";
  h += "</form></div></body></html>";
  return h;
}

String paginaConfigHTML(const String& tok) {
  String h = "<!DOCTYPE html><html><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width,initial-scale=1.0'>";
  h += "<style>body{font-family:sans-serif;text-align:center;background:#1e1e24;color:#fff;margin:0;padding:15px}h1{color:#4CAF50;font-size:18px}";
  h += ".box{background:#2a2a35;padding:20px;border-radius:15px;max-width:420px;margin:15px auto;box-shadow:0 4px 15px #000;text-align:left}";
  h += "input,select{width:100%;padding:10px;margin:5px 0 12px;border-radius:8px;border:1px solid #555;background:#1e1e24;color:#fff;box-sizing:border-box}";
  h += "button{padding:12px;border:none;border-radius:8px;font-weight:bold;cursor:pointer;width:100%;background:#4CAF50;color:#fff;font-size:15px}";
  h += "label{font-size:12px;color:#aaa}";
  h += "</style></head><body><h1>CONFIGURAÇÃO DO CAMPEONATO</h1><div class='box'>";
  h += "<form method='POST' action='/salvar_camp'>";
  h += "<input type='hidden' name='tok' value='" + tok + "'>";
  h += "<label>Nome da mesa / etiqueta (ex.: Mesa 01) — usado no painel e como WiFi. Se trocar, atualize o adesivo e reprograme a NFC</label><input name='mesa_nome' maxlength='16' value='" + ssidDaMesa() + "'>";
  h += "<label>WiFi da mesa (nome atual acima; vazio volta a ser Placar-<MAC> automático)</label><input value='" + mesaSsid + "' readonly>";
  h += "<label>MAC da placa (use para gravar a NFC / etiqueta)</label><input value='" + macDaMesa() + "' readonly>";
  h += "<label>Senha da mesa (mínimo 8 caracteres — se trocar, reprograme a NFC com os novos dados)</label><input name='mesa_senha' value='" + mesaSenha + "' required>";
  h += "<label>Canal da mesa (0 = automático; ou 1, 6 ou 11 — diferente em mesas próximas)</label><input type='number' name='mesa_canal' min='0' max='13' value='" + String(mesaCanal) + "'>";
  h += "<label>WiFi do roteador do centro (para enviar resultados ao notebook)</label>";
  h += "<div style='display:flex;gap:8px'><input id='rot_ssid' name='rot_ssid' value='" + rotSsid + "' placeholder='digite o nome da rede' style='flex:1'>";
  h += "<button type='button' id='btnScan' style='width:auto;background:#2196F3;padding:10px;white-space:nowrap' onclick='procurarRedes()'>PROCURAR</button></div>";
  h += "<div id='redesBox' style='display:none'><select id='selRedes' onchange='document.getElementById(\"rot_ssid\").value=this.value'></select></div>";
  h += "<script>function procurarRedes(){var b=document.getElementById('btnScan');b.textContent='PROCURANDO...';b.disabled=true;var x=new XMLHttpRequest();x.open('GET','/scan_redes');x.onload=function(){var s=document.getElementById('selRedes');s.innerHTML='';var r=JSON.parse(x.responseText);if(!r.length){s.innerHTML='<option>Nenhuma rede encontrada</option>';}for(var i=0;i<r.length;i++){var o=document.createElement('option');o.value=r[i].ssid;o.textContent=r[i].ssid+(r[i].aberta?' (aberta)':'');s.appendChild(o);}document.getElementById('redesBox').style.display='block';b.textContent='PROCURAR';b.disabled=false;};x.onerror=function(){b.textContent='ERRO — tente de novo';b.disabled=false;};x.send();}</script>";
  h += "<label>Senha do roteador do centro</label><input name='rot_senha' value='" + rotSenha + "'>";
  h += "<label>IP do notebook no roteador (onde roda o campeonato)</label><input name='srv_ip' value='" + srvIP + "'>";
  h += "<label>Código de acesso da configuração (deixe vazio para manter o atual)</label><input name='cfg_senha' value='' placeholder='código atual: " + configSenha + "'>";
  h += "<label>Botões físicos do placar: marque se o botão que fica no lado do jogador A está marcando os pontos do jogador B (lados trocados na mesa)</label>";
  h += "<div style='background:#1e1e24;border-radius:8px;padding:12px;margin:6px 0 12px;display:flex;align-items:center;gap:8px'><input type='checkbox' id='bot_inv' name='bot_inv' " + String(botoesFisicosInvertidos ? "checked" : "") + " style='width:auto;margin:0'><label for='bot_inv' style='font-size:13px;color:#fff;cursor:pointer'>INVERTER O LADO DOS BOTÕES FÍSICOS</label></div>";
  h += "<button>SALVAR E REINICIAR</button>";
  h += "</form>";
  h += "<form method='POST' action='/ignorar_camp' style='margin-top:10px'>";
  h += "<input type='hidden' name='tok' value='" + tok + "'>";
  h += "<button style='background:#555'>IGNORAR POR ENQUANTO (usar só o placar)</button>";
  h += "</form>";
  h += "</div></body></html>";
  return h;
}

String paginaReiniciando(const String& titulo) {
  String urlLocal = String("http://") + nomeMdns() + ".local/";
  String h = "<!DOCTYPE html><html><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width,initial-scale=1.0'>";
  h += "<style>body{font-family:sans-serif;text-align:center;background:#1e1e24;color:#fff;margin:0;padding:15px}h1{color:#4CAF50;font-size:18px}";
  h += ".box{background:#2a2a35;padding:20px;border-radius:15px;max-width:420px;margin:15px auto;box-shadow:0 4px 15px #000}";
  h += "p{font-size:14px;color:#ccc}button{background:#4CAF50;color:#fff;border:0;border-radius:12px;padding:16px 22px;font-size:17px;font-weight:bold;cursor:pointer;margin-top:6px;width:100%;}button:active{background:#388E3C}";
  h += ".links{font-size:12px;color:#888;margin-top:14px;word-break:break-all}";
  h += "</style></head><body><h1>" + titulo + "</h1><div class='box'>";
  h += "<p>A placa está reiniciando e conectando na rede <b>" + rotSsid + "</b>...</p>";
  h += "<p id='msg'>Aguardando a placa voltar...</p>";
  h += "<button id='btn' onclick='goSmart()'>Abrir o Placar agora</button>";
  h += "<div class='l'>Atafixe: <a id='lk1' href='#'>placa</a> &middot; <a id='lk2' href='#'>rede do roteador</a></div>";
  h += "</div>";
  h += "<script>var t=0;var srv='http://" + srvIP + ":5000';var mesa='"+ ssidDaMesa() + "';var alvos=['http://192.168.4.1/'";
  if (ultimoStaIP.length() > 0) h += ",'http://" + ultimoStaIP + "/'";
  h += ",'" + urlLocal + "'];";
  h += "function go(i){if(!alvos[i])return;window.location.replace(alvos[i]);}";
  h += "function addAlvo(u){if(!u)return;for(var i=0;i<alvos.length;i++){if(alvos[i]===u)return;}alvos.unshift(u);}";
  h += "document.getElementById('lk1').href=alvos[0];document.getElementById('lk1').innerHTML=alvos[0].replace('http://','');";
  h += "document.getElementById('lk2').href=alvos[1];document.getElementById('lk2').innerHTML=alvos[1].replace('http://','');";
  h += "var x=new XMLHttpRequest();x.timeout=4000;x.open('GET',srv+'/api/mes_ip?mesa='+encodeURIComponent(mesa));";
  h += "var resIP=null;";
  h += "x.onload=function(){try{var d=JSON.parse(x.responseText);if(d.ip){resIP='http://'+d.ip+'/';addAlvo(resIP);}}catch(e){}};";
  h += "x.onerror=function(){};x.send();";
  h += "function goSmart(){window.location.replace(resIP||'http://192.168.4.1/');}</script>";
  h += "</body></html>";
  return h;
}

void trocarLado() {
  String tn = nomeJogadorA; nomeJogadorA = nomeJogadorB; nomeJogadorB = tn;
  int tp = pontosA; pontosA = pontosB; pontosB = tp;
  int ts = setsA; setsA = setsB; setsB = ts;
  // Mantem o historico de sets alinhado a A/B: inverte cada "NxM" -> "MxN".
  if (setsDetalhados != "") {
    String novo = "";
    int ini = 0;
    while (true) {
      int fim = setsDetalhados.indexOf(',', ini);
      if (fim < 0) fim = setsDetalhados.length();
      String parte = setsDetalhados.substring(ini, fim);
      parte.trim();
      int x = parte.indexOf('x');
      if (x > 0) {
        String pa = parte.substring(0, x);
        String pb = parte.substring(x + 1);
        pa.trim(); pb.trim();
        if (novo != "") novo += ", ";
        novo += pb + "x" + pa;
      }
      if (fim >= (int)setsDetalhados.length()) break;
      ini = fim + 1;
    }
    setsDetalhados = novo;
  }
  quemSaca = (quemSaca == 1) ? 2 : 1;
  sacadorInicial = (sacadorInicial == 1) ? 2 : 1;
  // Nao invertemos botoesInvertidos aqui: nomes/pontos/sets ja trocam de coluna
  // acima, entao o botao fisico A continua marcando a coluna A (quem esta nela).
  // Inverter junto (dupla inversao) faria o ponto ir para o outro jogador.
}

void notificarCliqueFisico() {
  String j = "{\"ini\":" + String(jogoIniciado ? "true" : "false") + ",";
  j += "\"srt\":" + String(sorteioRealizado ? "true" : "false") + ",";
  j += "\"alg\":" + String(aguardandoInicio ? "true" : "false") + ",";
  j += "\"nA\":\"" + nomeJogadorA + "\",\"nB\":\"" + nomeJogadorB + "\",";
  j += "\"pA\":" + String(pontosA) + ",\"pB\":" + String(pontosB) + ",";
  j += "\"sA\":" + String(setsA) + ",\"sB\":" + String(setsB) + ",";
  j += "\"sq\":" + String(quemSaca) + ",\"sf\":" + String(setFechado ? "true" : "false") + ",";
  j += "\"msg\":\"" + msgStatus + "\",\"aviso\":\"" + avisoAtual + "\",\"pend\":" + String(partidaPendenteErro ? "true" : "false") + ",\"h\":\"" + historicoCompleto() + "\",\"ncl\":" + String(contarClientes()) + ",\"b\":" + String(percentualBateria) + ",\"btv\":" + String(tensaoBateria, 2) + ",\"sta\":" + String(WiFi.status() == WL_CONNECTED ? "true" : "false") + ",\"staIP\":\"" + (WiFi.status() == WL_CONNECTED ? WiFi.localIP().toString() : "") + "\"";
  j += ",\"camp\":\"" + escapeJson(campeaoAtual) + "\"";
  j += ",\"fin\":" + String(partidaFinalizada() ? "true" : "false");
  j += ",\"ver\":" + String(VERSAO_PAGINA) + ",\"ota\":\"" + otaStatus + "\",\"otap\":" + String(otaProgresso) + "}";
  for (int i = 0; i < MAX_SSE; i++) {
    if (clienteAtivo(i)) {
      sseClients[i].print("data: ");
      sseClients[i].println(j);
      sseClients[i].println();
    }
  }
}

void atualizarRegrasEsaque() {
  int total = pontosA + pontosB;
  setFechado = false;
  msgStatus = "";
  if (pontosA >= 10 && pontosB >= 10) {
    if (sacadorInicial == 1) { quemSaca = (total % 2 == 0) ? 1 : 2; } 
    else { quemSaca = (total % 2 == 0) ? 2 : 1; }
    if (pontosA >= pontosB + 2) { setFechado = true; msgStatus = "SET FECHADO: " + nomeJogadorA + " VENCEU O SET!"; }
    else if (pontosB >= pontosA + 2) { setFechado = true; msgStatus = "SET FECHADO: " + nomeJogadorB + " VENCEU O SET!"; }
  } else {
    int b2 = total / 2;
    if (sacadorInicial == 1) { quemSaca = (b2 % 2 == 0) ? 1 : 2; } 
    else { quemSaca = (b2 % 2 == 0) ? 2 : 1; }
    if (pontosA >= 11) { setFechado = true; msgStatus = "SET FECHADO: " + nomeJogadorA + " VENCEU O SET!"; }
    else if (pontosB >= 11) { setFechado = true; msgStatus = "SET FECHADO: " + nomeJogadorB + " VENCEU O SET!"; }
  }
}

void logWifiStatus(const char* rotulo) {
  unsigned char s = (unsigned char)WiFi.status();
  Serial.print("["); Serial.print(rotulo); Serial.print("] status="); Serial.print(s);
  if (s == WL_CONNECTED) {
    Serial.print(" conectado ip="); Serial.print(WiFi.localIP());
  } else {
    Serial.print(" NAO_CONNECTED sinal="); Serial.print((int)WiFi.RSSI());
  }
Serial.print(" srvIP="); Serial.println(srvIP);
}

void manterStaConectado() {
  if (rotSsid.length() == 0) return;
  if (WiFi.status() == WL_CONNECTED) { ultimaQuedaSta = 0; return; }
  if (millis() - ultimaQuedaSta < 5000) return;
  ultimaQuedaSta = millis();
  logWifiStatus("reconectar");
  Serial.print("[reconectar] tentando "); Serial.println(rotSsid);
  WiFi.disconnect();
  WiFi.begin(rotSsid.c_str(), rotSenha.c_str());
}

void setup() {
  Serial.begin(115200);
  // Se esta versão foi instalada via OTA e o boot completo der certo, cancela o
  // contador de rollback do bootloader: daqui pra frente ela é a versão estável.
  // Se esta versão travar no boot antes daqui, o Bootloader volta p/ a anterior.
  if (esp_ota_mark_app_valid_cancel_rollback() == ESP_OK) {
    Serial.println("[boot] firmware atual validado (rollback seguro ativo)");
  } else {
    Serial.println("[boot] aviso: rollback nao suportado pelo build (sem marca de validacao)");
  }
  setCpuFrequencyMhz(80);
  btStop();
  pinMode(pinoBotaoA, INPUT_PULLUP);
  pinMode(pinoBotaoB, INPUT_PULLUP);
  pinMode(pinoLedBateria, OUTPUT);
  digitalWrite(pinoLedBateria, LOW);
  analogReadResolution(12);
  analogSetPinAttenuation(pinoBateria, ADC_11db);
  lerBateria();
  carregarConfig();
  
  WiFi.setTxPower(WIFI_POWER_11dBm);  // potência fixa alta p/ manter o STA (conexão ao roteador) estável
  IPAddress apIP(192, 168, 4, 1);
  WiFi.mode(WIFI_AP_STA);
  mesaSsid = ssidDaMesa();  // SSID automático único por placa (nunca muda → NFC sempre funciona)
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  if (mesaCanal <= 0) {
    mesaCanal = escolherCanalAutomatico();
    Serial.print("Canal automatico da mesa: "); Serial.println(mesaCanal);
  }
  WiFi.softAP(mesaSsid.c_str(), mesaSenha.c_str(), mesaCanal);
  if (rotSsid.length() > 0) {
    WiFi.setAutoReconnect(true);
    WiFi.begin(rotSsid.c_str(), rotSenha.c_str());
  }
  if (MDNS.begin(nomeMdns().c_str())) {
    MDNS.addService("http", "tcp", 80);
    Serial.print("mDNS ativo: http://"); Serial.print(nomeMdns()); Serial.println(".local");
  }
  udp.begin(7777);
  dnsServer.start(53, "*", apIP);
  
  auto servirPagina = [](const String& html) {
    server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    server.send(200, "text/html", html);
  };
  auto servirHTML = [&]() {
    if (configurado() || WiFi.status() == WL_CONNECTED) servirPagina(paginaHTML);
    else if (autenticadoConfig()) servirPagina(paginaConfigHTML(server.arg("tok")));
    else servirPagina(paginaLoginHTML(""));
  };
  server.on("/", servirHTML);
  auto servirCaptiva = [&]() {
    if (configurado() || WiFi.status() == WL_CONNECTED) servirPagina(paginaCaptivaHTML);
    else if (autenticadoConfig()) servirPagina(paginaConfigHTML(server.arg("tok")));
    else servirPagina(paginaLoginHTML(""));
  };
  server.on("/generate_204", servirCaptiva);
  server.on("/hotspot-detect.html", servirCaptiva);
  server.on("/success.txt", servirCaptiva);
  server.on("/ncsi.txt", servirCaptiva);
  server.on("/manifest.json", []() {
    server.sendHeader("Cache-Control", "no-cache");
    server.send(200, "application/manifest+json",
      "{\"name\":\"PLACAR TÉNIS DE MESA\",\"short_name\":\"PLACAR\",\"start_url\":\"/\",\"display\":\"fullscreen\",\"orientation\":\"landscape\",\"background_color\":\"#1e1e24\",\"theme_color\":\"#1e1e24\"}");
  });
  server.on("/connecttest.txt", servirCaptiva);
  server.on("/login_camp", HTTP_POST, [&]() {
    String s = server.hasArg("senha") ? server.arg("senha") : "";
    if (s == configSenha) {
      computarTokenConfig();
      server.send(200, "text/html", paginaConfigHTML(tokenConfig));
    } else {
      server.send(200, "text/html", paginaLoginHTML("Código incorreto."));
    }
  });
  server.on("/config_camp", HTTP_GET, [&]() {
    if (autenticadoConfig()) servirPagina(paginaConfigHTML(server.arg("tok")));
    else servirPagina(paginaLoginHTML(""));
  });
  server.on("/ignorar_camp", HTTP_POST, [&]() {
    if (!autenticadoConfig()) { server.send(200, "text/html", paginaLoginHTML("Código incorreto.")); return; }
    configIgnorada = true;
    salvarConfig();
    Serial.println("[ignorar] placar direto (sem reiniciar)");
    notificarCliqueFisico();
    servirPagina(paginaHTML);
  });
  server.on("/salvar_camp", HTTP_POST, [&]() {
    if (!autenticadoConfig()) { server.send(200, "text/html", paginaLoginHTML("Código incorreto.")); return; }
    if (server.hasArg("mesa_senha")) mesaSenha = server.arg("mesa_senha");
    if (server.hasArg("mesa_canal")) mesaCanal = server.arg("mesa_canal").toInt();
    if (server.hasArg("rot_ssid")) rotSsid = server.arg("rot_ssid");
    if (server.hasArg("rot_senha")) rotSenha = server.arg("rot_senha");
    if (server.hasArg("srv_ip")) {
      String v = server.arg("srv_ip");
      v.trim();
      if (v.length() > 0) srvIP = v;
    }
    if (server.hasArg("mesa_nome")) {
      String v = server.arg("mesa_nome");
      v.trim();
      String limpo = "";
      for (unsigned int i = 0; i < v.length(); i++) {
        char ch = v[i];
        bool ok = (ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')
                  || ch == ' ' || ch == '-' || ch == '.' || ch == '_' || ch > 127;
        if (ok) limpo += ch;
      }
      if (limpo.length() > 16) limpo = limpo.substring(0, 16);
      nomeMesa = limpo;
    }
    if (server.hasArg("cfg_senha") && server.arg("cfg_senha").length() > 0) { configSenha = server.arg("cfg_senha"); computarTokenConfig(); }
    botoesFisicosInvertidos = server.hasArg("bot_inv");
    configIgnorada = false;
    salvarConfig();
    Serial.println("[salvar] rot=" + rotSsid + " srvIP=" + srvIP + " ultimoStaIP=" + ultimoStaIP);
    server.send(200, "text/html", paginaReiniciando("CONFIGURAÇÃO SALVA"));
    Serial.println("[salvar] pagina reiniciando enviada");
    delay(2000);
    ESP.restart();
  });
  server.on("/dados", []() { registrarPingIP(server.client().remoteIP()); enviarDadosJSON(server); });
  server.on("/api/coleta", []() {
    ultimoClienteAtivo = millis();   // o notebook (puxada) conta como cliente ativo: evita o auto-reset de 30s
    lerBateria();
    String j = "{\"mesa\":\"" + ssidDaMesa() + "\",\"ip\":\"" + WiFi.localIP().toString()
             + "\",\"b\":\"" + String(percentualBateria) + "\",\"tensao\":\"" + String(tensaoBateria, 2)
             + "\",\"ssid\":\"" + escapeJson(rotSsid)
             + "\",\"livre\":" + String(!(jogoIniciado || aguardandoInicio || aguardandoConfirmacao) ? "true" : "false")
             + ",\"jogA\":\"" + escapeJson(nomeJogadorA) + "\",\"jogB\":\"" + escapeJson(nomeJogadorB)
             + "\",\"camp\":\"" + escapeJson(campeaoAtual) + "\",\"seqN\":" + String(seqResultado) + ",\"seqEntregue\":" + String(seqEntregue)
             + ",\"resultados\":[";
    bool primeira = true;
    for (int k = filaHead; k < filaTail; k++) {
      ResultadoFila& r = filaResultados[k % MAX_FILA_RESULT];
      if (r.seq <= seqEntregue) continue;
      if (!primeira) j += ",";
      primeira = false;
      j += "{\"seq\":" + String(r.seq) + ",\"A\":\"" + escapeJson(r.jogA) + "\",\"B\":\"" + escapeJson(r.jogB)
         + "\",\"sA\":" + String(r.sA) + ",\"sB\":" + String(r.sB)
         + ",\"sets\":\"" + escapeJson(r.sets) + "\"}";
    }
    j += "]}";
    server.send(200, "application/json", j);
  });

  server.on("/api/ordem", HTTP_POST, [&]() {
    ultimoClienteAtivo = millis();
    String acao = server.arg("acao");
    if (acao == "cancelar") {
      campeaoAtual = "";
      pontosA = 0; pontosB = 0; setsA = 0; setsB = 0; totalSetsJogados = 0; setsDetalhados = "";
      historicoJogoAtual = ""; avisoAtual = ""; msgStatus = "";
      pendNomeA = pendNomeB = pendHistorico = pendSetsDetalhados = "";
      pendSetsA = pendSetsB = pendTotalSets = 0; pendSaque = 1; pendSacadorInicial = 1;
      partidaPendenteErro = false; aguardandoConfirmacao = false; aguardandoInicio = false;
      jogoIniciado = false; sorteioRealizado = false; setFechado = false;
      quemSaca = 1; sacadorInicial = 1;
      botoesInvertidos = botoesFisicosInvertidos;
      notificarCliqueFisico();
      server.send(200, "application/json", "{\"ok\":true}");
      return;
    }
    if (acao == "campeao") {
      campeaoAtual = server.arg("nome");
      pontosA = 0; pontosB = 0; setsA = 0; setsB = 0; totalSetsJogados = 0; setsDetalhados = "";
      historicoJogoAtual = ""; avisoAtual = ""; msgStatus = "";
      partidaPendenteErro = false; setFechado = false;
      jogoIniciado = false; sorteioRealizado = false; aguardandoInicio = false; aguardandoConfirmacao = false;
      quemSaca = 1; sacadorInicial = 1;
      botoesInvertidos = botoesFisicosInvertidos;
      notificarCliqueFisico();
      Serial.print("Campeao definido: "); Serial.println(campeaoAtual);
      server.send(200, "application/json", "{\"ok\":true}");
      return;
    }
    if (jogoIniciado && (aguardandoInicio || sorteioRealizado || (pontosA + pontosB + setsA + setsB > 0))) {
      server.send(200, "application/json", "{\"ok\":false,\"erro\":\"ocupada\"}");
      return;
    }
    String na = server.arg("A");
    String nb = server.arg("B");
    if (na.length() == 0 || nb.length() == 0) { server.send(400, "application/json", "{\"ok\":false}"); return; }
    campeaoAtual = "";
    nomeJogadorA = na; nomeJogadorB = nb;
    pontosA = 0; pontosB = 0; setsA = 0; setsB = 0; totalSetsJogados = 0;
    setsDetalhados = ""; historicoJogoAtual = ""; avisoAtual = ""; msgStatus = "";
    partidaPendenteErro = false; setFechado = false;
    botoesInvertidos = botoesFisicosInvertidos;
    jogoIniciado = true; sorteioRealizado = false; aguardandoInicio = true;
    if (server.hasArg("modo")) modoCampeonato = server.arg("modo");
    if (server.hasArg("formatoMata")) formatoMata = server.arg("formatoMata").toInt();
    if (server.hasArg("formatoGrupos")) formatoGrupos = server.arg("formatoGrupos").toInt();
    if (server.hasArg("formatoFinal")) formatoFinal = server.arg("formatoFinal").toInt();
    if (server.hasArg("formato") && server.arg("formato") != "") formatoAtual = server.arg("formato").toInt();
    else if (server.arg("acao") == "chamar") formatoAtual = 0;
    campeonatoDetectado = true;
    atualizarRegrasEsaque();
    notificarCliqueFisico();
    Serial.print("Partida recebida do notebook: "); Serial.print(na); Serial.print(" x "); Serial.println(nb);
    server.send(200, "application/json", "{\"ok\":true}");
    return;
  });

  server.on("/api/limpar_campeao", HTTP_POST, [&]() {
    campeaoAtual = "";
    pontosA = 0; pontosB = 0; setsA = 0; setsB = 0; totalSetsJogados = 0; setsDetalhados = "";
    historicoJogoAtual = ""; avisoAtual = ""; msgStatus = "";
    partidaPendenteErro = false; setFechado = false;
    jogoIniciado = false; sorteioRealizado = false; aguardandoInicio = false; aguardandoConfirmacao = false;
    quemSaca = 1; sacadorInicial = 1;
    notificarCliqueFisico();
    server.send(200, "application/json", "{\"ok\":true}");
  });

  server.on("/api/nome", HTTP_POST, [&]() {
    if (!server.hasArg("nome")) { server.send(400, "application/json", "{\"ok\":false,\"erro\":\"faltou_nome\"}"); return; }
    String v = server.arg("nome");
    v.trim();
    String limpo = "";
    for (unsigned int i = 0; i < v.length(); i++) {
      char ch = v[i];
      bool ok = (ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')
                || ch == ' ' || ch == '-' || ch == '.' || ch == '_' || ch > 127;
      if (ok) limpo += ch;
    }
    if (limpo.length() > 16) limpo = limpo.substring(0, 16);
    nomeMesa = limpo;
    salvarConfig();
    String j = "{\"ok\":true,\"mesa\":\"" + ssidDaMesa() + "\"}";
    server.send(200, "application/json", j);
    Serial.print("[nome] mesa renomeada para: "); Serial.println(ssidDaMesa());
    delay(1000);
    ESP.restart();
  });

  server.on("/api/coletado", HTTP_POST, [&]() {
    unsigned long ate = server.arg("ate").toInt();
    if (ate > seqEntregue) seqEntregue = ate;
    while (filaHead < filaTail && filaResultados[filaHead % MAX_FILA_RESULT].seq <= seqEntregue) filaHead++;
    server.send(200, "application/json", "{\"ok\":true}");
  });
  server.on("/scan_redes", []() {
    WiFi.scanDelete();
    WiFi.disconnect();          // interrompe a reconexão STA atual (que disputa o rádio e faz o scan falhar)
    delay(300);
    WiFi.mode(WIFI_AP_STA);     // garante STA(radar) + AP ativos, sem derrubar o AP
    WiFi.enableSTA(true);
    delay(200);
    int n = -1;
    for (int t = 0; t < 8 && n < 0; t++) { n = WiFi.scanNetworks(); if (n < 0) delay(800); }
    String saida = "[";
    for (int i = 0; i < n; i++) {
      if (i > 0) saida += ",";
      String ssid = WiFi.SSID(i);
      ssid.replace("\\", "\\\\"); ssid.replace("\"", "\\\"");
      saida += "{\"ssid\":\"" + ssid + "\",\"aberta\":" + String(WiFi.encryptionType(i) == WIFI_AUTH_OPEN ? "true" : "false") + "}";
    }
    WiFi.scanDelete();
    saida += "]";
    server.send(200, "application/json", saida);
  });
  server.on("/ping", []() { registrarPingIP(server.client().remoteIP()); Serial.print("[ping] de "); Serial.println(server.client().remoteIP()); server.send(200, "text/plain", "ok"); });
  server.on("/log", []() { Serial.print("[log] "); Serial.println(server.arg("m")); server.send(200, "text/plain", "ok"); });
  
  server.on("/config", []() {
    if (campeonatoDetectado) {
      if (partidaPendenteErro) { enviarDadosJSON(server); return; }
      avisoAtual = "Campeonato em andamento: aguarde a chamada para o proximo jogo.";
      enviarDadosJSON(server);
      notificarCliqueFisico();
      return;
    }
    nomeJogadorA = server.hasArg("nA") ? server.arg("nA") : "Jogador A";
    nomeJogadorB = server.hasArg("nB") ? server.arg("nB") : "Jogador B";
    jogoIniciado = true; sorteioRealizado = false; setFechado = false;
    setsA = 0; setsB = 0; totalSetsJogados = 0; historicoJogoAtual = ""; avisoAtual = "";
    setsDetalhados = "";
    partidaPendenteErro = false;
    enviarDadosJSON(server);
  });
  
  server.on("/definirSaque", []() {
    sacadorInicial = server.arg("id").toInt(); quemSaca = sacadorInicial; sorteioRealizado = true;
    atualizarRegrasEsaque();
    enviarDadosJSON(server);
  });

  server.on("/iniciar_partida", []() {
    if (aguardandoInicio) { aguardandoInicio = false; }  // sai da tela "VS" -> mostra a escolha de saque
    enviarDadosJSON(server);
    notificarCliqueFisico();
  });
  
  server.on("/controle", []() {
    String c = server.arg("cmd");
    if (aguardandoConfirmacao && c != "confirmar_resultado" && c != "cancelar_resultado") {
      enviarDadosJSON(server);
      notificarCliqueFisico();
      return;
    }
    if (c == "confirmar_resultado") {
      if (aguardandoConfirmacao) {
        nomeJogadorA = confNA; nomeJogadorB = confNB;
        setsDetalhados = confDet;
        setsA = confSA; setsB = confSB;
      }
      if (setsA + setsB > 0) {
        aguardandoConfirmacao = false;
        enviarResultadoCampeonato();      // enfileira o resultado p/ o notebook puxar
        partidaPendenteErro = false;
        pontosA = 0; pontosB = 0; jogoIniciado = false; sorteioRealizado = false; setFechado = false;
        aguardandoInicio = false;
        setsA = 0; setsB = 0; totalSetsJogados = 0; setsDetalhados = "";
        historicoJogoAtual = ""; avisoAtual = ""; msgStatus = "";
        quemSaca = 1; sacadorInicial = 1;
        botoesInvertidos = botoesFisicosInvertidos;
      }
      enviarDadosJSON(server);
      notificarCliqueFisico();
      return;
    }
    if (c == "cancelar_resultado") {
      if (aguardandoConfirmacao) {
        aguardandoConfirmacao = false;
        pontosA = prePA; pontosB = prePB; setsA = preSA; setsB = preSB;
        setFechado = preSF; totalSetsJogados = preTotal;
        historicoJogoAtual = preHist; setsDetalhados = preDet; avisoAtual = preAviso;
        quemSaca = preSaque; sacadorInicial = preSacIni;
      }
      enviarDadosJSON(server);
      notificarCliqueFisico();
      return;
    }
    if (c == "reset") { pontosA = 0; pontosB = 0; quemSaca = sacadorInicial; setFechado = false; atualizarRegrasEsaque(); }
    else if (c == "config_tela") {
      if (jogoIniciado && aguardandoInicio && (pontosA + pontosB + setsA + setsB == 0)) {
        avisoAtual = "";
        enviarDadosJSON(server);
        notificarCliqueFisico();
        return;
      }
      if (campeonatoDetectado && jogoIniciado && (pontosA + pontosB + setsA + setsB > 0) && !partidaFinalizada()) {
        avisoAtual = "FINALIZE A PARTIDA EM ANDAMENTO antes de começar outra.";
      } else {
        preSA = setsA; preSB = setsB; prePA = pontosA; prePB = pontosB;
        preSF = setFechado; preTotal = totalSetsJogados;
        preHist = historicoJogoAtual; preDet = setsDetalhados;
        preSaque = quemSaca; preSacIni = sacadorInicial; preAviso = avisoAtual;
        finalizarJogoAtual();
        if (!partidaPendenteErro && campeonatoDetectado && jogoIniciado && (setsA + setsB > 0)) {
          confNA = nomeJogadorA; confNB = nomeJogadorB;
          confSA = setsA; confSB = setsB; confDet = setsDetalhados;
          aguardandoConfirmacao = true;
          String j = "{\"confirm\":true,\"nA\":\"" + confNA + "\",\"nB\":\"" + confNB
                   + "\",\"sA\":" + String(confSA) + ",\"sB\":" + String(confSB)
                   + ",\"ini\":" + String(jogoIniciado ? "true" : "false")
                   + ",\"srt\":" + String(sorteioRealizado ? "true" : "false") + "}";
          server.send(200, "application/json", j);
          notificarCliqueFisico();
          return;
        } else {
          if (!partidaPendenteErro && (setsA + setsB > 0)) enviarResultadoCampeonato();
          pontosA = 0; pontosB = 0; jogoIniciado = false; sorteioRealizado = false; setFechado = false;
          aguardandoInicio = false;
          setsA = 0; setsB = 0; totalSetsJogados = 0; setsDetalhados = "";
          historicoJogoAtual = ""; avisoAtual = ""; msgStatus = "";
          quemSaca = 1; sacadorInicial = 1;
          botoesInvertidos = botoesFisicosInvertidos;
        }
      }
    }
    else if (c == "retomar_partida" && partidaPendenteErro) {
      nomeJogadorA = pendNomeA; nomeJogadorB = pendNomeB;
      setsA = pendSetsA; setsB = pendSetsB; totalSetsJogados = pendTotalSets;
      quemSaca = pendSaque; sacadorInicial = pendSacadorInicial;
      historicoJogoAtual = pendHistorico;
      setsDetalhados = pendSetsDetalhados;
      pontosA = 0; pontosB = 0; setFechado = false; sorteioRealizado = true; jogoIniciado = true;
      partidaPendenteErro = false; avisoAtual = "";
      atualizarRegrasEsaque();
    }
    else if (c == "next_set" && setFechado) {
      if (partidaFinalizada()) {
        confirmarPartida();
      } else {
        registrarSetHistorico();
        totalSetsJogados++;
        if (pontosA > pontosB) setsA++; else setsB++;
        pontosA = 0; pontosB = 0;
        sacadorInicial = (sacadorInicial == 1) ? 2 : 1; quemSaca = sacadorInicial; setFechado = false;
        atualizarRegrasEsaque();
      }
    } else {
      if (c == "A1" && !setFechado) pontosA++;
      if (c == "B1" && !setFechado) pontosB++;
      if (c == "A0" && pontosA > 0) pontosA--;
      if (c == "B0" && pontosB > 0) pontosB--;
      atualizarRegrasEsaque();
    }
    enviarDadosJSON(server);
    notificarCliqueFisico(); // Avisa o canal de streaming sobre o clique digital
  });
  
  server.on("/next_set", []() {
    if (aguardandoConfirmacao) { confirmarPartida(); enviarDadosJSON(server); notificarCliqueFisico(); return; }
    if (setFechado && partidaFinalizada()) {
      confirmarPartida();
    } else if (setFechado) {
      registrarSetHistorico();
      totalSetsJogados++;
      if (pontosA > pontosB) setsA++; else setsB++;
      pontosA = 0; pontosB = 0;
      sacadorInicial = (sacadorInicial == 1) ? 2 : 1; quemSaca = sacadorInicial; setFechado = false;
      atualizarRegrasEsaque();
      trocarLado();
    }
    enviarDadosJSON(server);
    notificarCliqueFisico();
  });

  server.on("/confirmar_resultado", []() {
    confirmarPartida();
    enviarDadosJSON(server);
    notificarCliqueFisico();
  });

  server.on("/cancelar_resultado", []() {
    if (aguardandoConfirmacao) {
      aguardandoConfirmacao = false;
      pontosA = prePA; pontosB = prePB; setsA = preSA; setsB = preSB;
      setFechado = preSF; totalSetsJogados = preTotal;
      historicoJogoAtual = preHist; setsDetalhados = preDet; avisoAtual = preAviso;
      quemSaca = preSaque; sacadorInicial = preSacIni;
    }
    enviarDadosJSON(server);
    notificarCliqueFisico();
  });

  server.on("/trocarLado", []() {
    trocarLado();
    enviarDadosJSON(server);
    notificarCliqueFisico();
  });

  server.onNotFound([]() {
    server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    if (configurado()) server.send(200, "text/html", paginaHTML);
    else if (autenticadoConfig()) server.send(200, "text/html", paginaConfigHTML(server.arg("tok")));
    else server.send(200, "text/html", paginaLoginHTML(""));
  });
  server.begin();
  sseServer.begin();
  WiFi.setSleep(false);  // desliga o modem-sleep (evita quedas do STA)
  Serial.println("PLACAR TÊNIS DE MESA PRONTO");
  Serial.print("SSID: "); Serial.println(mesaSsid);
  Serial.print("Canal: "); Serial.println(mesaCanal);
  Serial.print("IP: "); Serial.println(WiFi.softAPIP());
  if (configurado()) {
    Serial.print("Roteador (STA): "); Serial.println(rotSsid);
  } else {
    Serial.println("Ainda não configurado. Abra 192.168.4.1 para configurar o campeonato.");
  }
}

void loop() {
  verificarCampeonato();
  if (millis() >= ATRASO_INICIAL_OTA
      && !otaEmAndamento
      && (ultimoCheckOTA == 0
          || millis() - ultimoCheckOTA >= (otaCheckFeito ? INTERVALO_OTA : RETRY_OTA))) {
    ultimoCheckOTA = millis();
    iniciarOtaBackground();
  }
  if (millis() - ultimoEnvioBateriaBg > intervaloEnvioBateria && !envioBateriaEmAndamento) {
    ultimoEnvioBateriaBg = millis();
    envioBateriaEmAndamento = true;
    xTaskCreatePinnedToCore(tarefaEnvioBateria, "batTask", 8192, NULL, 1, NULL, 0);
  }
  procurarServidor();
  manterStaConectado();
  if (millis() - ultimoLeituraBateria > intervaloLeituraBateria) {
    ultimoLeituraBateria = millis();
    lerBateria();
  }
  dnsServer.processNextRequest();
  server.handleClient();
  delay(5);
  if (millis() - ultimoLogSta > intervaloLogSta) {
    ultimoLogSta = millis();
    logWifiStatus("watch");
  }

  for (int i = 0; i < MAX_SSE; i++) {
    if (sseClients[i] && !sseClients[i].connected()) { sseClients[i].stop(); flagNotificar = true; }
  }
  WiFiClient novo = sseServer.accept();
  if (novo) {
    novo.setNoDelay(true);   // evita que eventos SSE fiquem presos no buffer TCP (atraso na 2a tela)
    unsigned long t = millis();
    while (novo.connected() && !novo.available() && millis() - t < 200) { delay(1); }
    String req = "";
    while (novo.available()) {
      char c = novo.read(); req += c;
      if (req.endsWith("\r\n\r\n")) { break; }
    }
    novo.println("HTTP/1.1 200 OK");
    novo.println("Content-Type: text/event-stream");
    novo.println("Cache-Control: no-cache");
    novo.println("Connection: keep-alive");
    novo.println("Access-Control-Allow-Origin: *");
    novo.println();
    for (int i = 0; i < MAX_SSE; i++) {
      if (!sseClients[i] || !sseClients[i].connected()) { sseClients[i] = novo; flagNotificar = true; break; }
    }
    registrarPingIP(novo.remoteIP());
  }

  if (millis() - ultimoCheckDisp > 2000) {
    ultimoCheckDisp = millis();
    unsigned long agora = millis();
    bool mudou = false;
    for (int i = 0; i < MAX_DISP; i++) {
      if (disp[i].ultimoPing && agora - disp[i].ultimoPing > tempoVidaPing) { disp[i].ultimoPing = 0; mudou = true; }
    }
    for (int i = 0; i < MAX_SSE; i++) {
      if (sseClients[i] && sseClients[i].connected()) {
        bool ativo = false;
        for (int j = 0; j < MAX_DISP; j++) {
          if (disp[j].ultimoPing && disp[j].ip == sseClients[i].remoteIP()) { ativo = true; break; }
        }
        if (!ativo) { sseClients[i].stop(); mudou = true; }
      }
    }
    if (mudou) flagNotificar = true;
  }

  if (millis() - ultimoHeartbeat > intervaloHeartbeat) {
    ultimoHeartbeat = millis();
    notificarCliqueFisico();
  }

  if (contarClientes() > 0) {
    ultimoClienteAtivo = millis();
  } else if (jogoIniciado && millis() - ultimoClienteAtivo > tempoResetSemClientes) {
    finalizarJogoAtual();
    if (!partidaPendenteErro) enviarResultadoCampeonato();
    pontosA = 0; pontosB = 0; jogoIniciado = false; sorteioRealizado = false; setFechado = false;
    aguardandoInicio = false;
    setsA = 0; setsB = 0; totalSetsJogados = 0; msgStatus = ""; setsDetalhados = "";
    if (!partidaPendenteErro) avisoAtual = "";
    quemSaca = 1; sacadorInicial = 1;
    ultimoClienteAtivo = millis();
  }

  if (flagNotificar) { flagNotificar = false; notificarCliqueFisico(); }
  
  if (jogoIniciado && sorteioRealizado && !aguardandoConfirmacao) {
    int ladoPinoA = botoesInvertidos ? 2 : 1;
    int ladoPinoB = botoesInvertidos ? 1 : 2;
    if (digitalRead(pinoBotaoA) == LOW) {
      unsigned long t = millis();
      if (tempoApertadoA == 0) { tempoApertadoA = t; processadoLongPressA = false; }
      if ((t - tempoApertadoA > tempoLongPress) && !processadoLongPressA) {
        if (setFechado) setFechado = false;
        if (ladoPinoA == 1) { if (pontosA > 0) pontosA--; } else { if (pontosB > 0) pontosB--; }
        atualizarRegrasEsaque(); processadoLongPressA = true; notificarCliqueFisico();
      }
    } else {
      if (tempoApertadoA > 0) {
        if (!processadoLongPressA && (millis() - tempoApertadoA > 50)) {
          if (!setFechado) { if (ladoPinoA == 1) pontosA++; else pontosB++; }
          atualizarRegrasEsaque(); notificarCliqueFisico();
        }
        tempoApertadoA = 0;
      }
    }
    if (digitalRead(pinoBotaoB) == LOW) {
      unsigned long t = millis();
      if (tempoApertadoB == 0) { tempoApertadoB = t; processadoLongPressB = false; }
      if ((t - tempoApertadoB > tempoLongPress) && !processadoLongPressB) {
        if (setFechado) setFechado = false;
        if (ladoPinoB == 1) { if (pontosA > 0) pontosA--; } else { if (pontosB > 0) pontosB--; }
        atualizarRegrasEsaque(); processadoLongPressB = true; notificarCliqueFisico();
      }
    } else {
      if (tempoApertadoB > 0) {
        if (!processadoLongPressB && (millis() - tempoApertadoB > 50)) {
          if (!setFechado) { if (ladoPinoB == 1) pontosA++; else pontosB++; }
          atualizarRegrasEsaque(); notificarCliqueFisico();
        }
        tempoApertadoB = 0;
      }
    }
  }
}
