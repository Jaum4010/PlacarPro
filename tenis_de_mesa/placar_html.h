#ifndef PLACAR_HTML_H
#define PLACAR_HTML_H

const char paginaHTML[] PROGMEM = R"rawhtml(
<!DOCTYPE html><html><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width,initial-scale=1.0'><link rel='manifest' href='/manifest.json'><meta name='mobile-web-app-capable' content='yes'><meta name='apple-mobile-web-app-capable' content='yes'><meta name='theme-color' content='#1e1e24'>
<style>
body{font-family:sans-serif;text-align:center;background:#1e1e24;color:#fff;margin:0;padding:15px;user-select:none;-webkit-user-select:none;overflow-anchor:none;-webkit-overflow-anchor:none}html{overflow-anchor:none;-webkit-overflow-anchor:none}
h1{color:#4CAF50}.box{background:#2a2a35;padding:20px;border-radius:15px;max-width:400px;margin:20px auto;box-shadow:0 4px 15px #000}
input{width:100%;padding:12px;margin-bottom:15px;border-radius:8px;border:1px solid #555;background:#1e1e24;color:#fff;box-sizing:border-box;text-transform:uppercase}
button{padding:12px;border:none;border-radius:8px;font-weight:bold;cursor:pointer;width:100%;margin:5px 0}
.btn-g{background:#4CAF50;color:#fff;font-size:18px}.btn-y{background:#ffcc00;color:#1e1e24}
.flex{display:flex;justify-content:space-around;max-width:500px;margin:0 auto}
.player-group{display:flex;flex:1;align-items:center}
.player-group .jog{flex:1}
.jog{background:#2a2a35;padding:15px;border-radius:15px;width:44%;box-shadow:0 4px 15px #000;transition:0.3s}
.sacando{border:2px solid #ffcc00}.pts{font-size:65px;font-weight:bold;margin:2px 0;line-height:1}
.sets-count{font-size:14px;color:#ffcc00;margin-top:2px;font-weight:bold}
.btn-m{background:#2e7d32;color:#fff}.btn-less{background:#555;color:#fff;width:80%}
.status{position:fixed;top:6%;left:50%;transform:translate(-50%,-50%);-webkit-transform:translate(-50%,-50%);background:rgba(30,90,40,0.95);border:2px solid #4CAF50;padding:14px 22px;border-radius:12px;font-weight:bold;display:none;z-index:60;text-align:center;font-size:3.5vh;max-width:90vw;box-sizing:border-box;box-shadow:0 4px 18px rgba(0,0,0,0.5)}
#fullscreen-btn{position:fixed;top:5px;right:5px;background:rgba(255,255,255,0.1);color:#fff;border:1px solid #555;border-radius:5px;padding:4px 8px;font-size:11px;cursor:pointer;z-index:99;width:auto}
#somBtn{position:fixed;bottom:10px;left:10px;background:rgba(255,255,255,0.1);color:#888;border:1px solid #555;border-radius:8px;padding:8px 12px;font-size:14px;cursor:pointer;z-index:99;width:auto}
#somBtn.ativo{color:#4CAF50;border-color:#4CAF50}
#trocarLadoBtn{position:fixed;bottom:10px;right:10px;background:#1565c0;color:#fff;border:none;border-radius:8px;padding:10px 16px;font-size:13px;font-weight:bold;cursor:pointer;z-index:99;width:auto;box-shadow:0 2px 8px rgba(0,0,0,0.4)}
#autoNextMsg{position:fixed;top:18%;left:50%;transform:translate(-50%,-50%);-webkit-transform:translate(-50%,-50%);display:none;background:rgba(60,45,0,0.95);color:#ffcc00;border:2px solid #ffcc00;padding:14px 22px;border-radius:12px;font-weight:bold;font-size:3.5vh;text-align:center;z-index:65;max-width:90vw;box-sizing:border-box;box-shadow:0 4px 18px rgba(0,0,0,0.5)}
._awake-indicator{position:fixed;top:-100px;left:0;width:2px;height:2px;opacity:0;pointer-events:none;animation:_w 0.05s infinite;will-change:transform}@keyframes _w{from{transform:translateX(0)}to{transform:translateX(10px)}}
.set-box{background:#ffcc00;color:#000;border-radius:6px;font-weight:bold;display:flex;align-items:center;justify-content:center;width:6%;font-size:2.5vw}
@media(orientation:landscape){
  .jogando h1{display:none}.jogando #fullscreen-btn{display:none}
  .jogando #lA,.jogando #lB{font-size:22px;font-weight:bold}
  .jogando .pts{font-size:40vh;line-height:1;margin:0}
  .jogando .btn-m,.jogando .btn-less{width:38%;padding:4px;font-size:11px;margin:2px auto;min-height:28px}
  .jogando .player-group .jog{padding:27px 4px}
  .jogando .flex{max-width:94vw}
  .jogando .set-box{height:39vh;font-size:16vh;width:11%}
  .jogando .flex{gap:6px}
  .jogando #skA,.jogando #skB{font-size:11px;height:12px!important}
}
#trocarLadoBtn,#virarTelaBtn{display:none;position:fixed;bottom:10px;color:#fff;border:none;border-radius:8px;padding:10px 16px;font-size:13px;font-weight:bold;cursor:pointer;z-index:99;width:auto;box-shadow:0 2px 8px rgba(0,0,0,0.4)}
.jogando #trocarLadoBtn{display:flex}
.jogando #virarTelaBtn.multi{display:flex}
#trocarLadoBtn{right:10px;background:#1565c0}
#virarTelaBtn{left:96px;background:#5c4a1a}
.jogando.virado .player-group{flex-direction:row-reverse}.jogando.virado .player-group:first-child{order:2}.jogando.virado .player-group:last-child{order:1}
.btn-hist{background:#3a3a45;color:#bbb;font-size:12px;width:auto;padding:6px 12px;margin-top:15px}
.hist-box{background:#15151a;padding:10px;border-radius:8px;margin-top:10px;display:none;text-align:left;font-size:13px;color:#ccc}
.vs{animation:vsPulse 1.2s ease-in-out infinite;border:1px solid #ffcc00}
.vs .vs-row{display:flex;justify-content:space-between;align-items:center;gap:8px}
.vs .vs-n{background:#2e7d32;padding:16px 8px;border-radius:12px;flex:1;font-size:26px;font-weight:bold;word-break:break-word}
.vs .vs-x{color:#ffcc00;font-size:22px;font-weight:bold;flex:0 0 auto}
@keyframes vsPulse{0%,100%{opacity:1}50%{opacity:0.35}}
@media(orientation:landscape){
  #t2,#t0{max-width:none;margin:0;padding:6vh 4vw;min-height:100vh;border-radius:0;box-sizing:border-box;text-align:center;background:#1e1e24}
  #t2 h2,#t0 h2{font-size:6vh;margin:2vh 0 4vh}
  #t2 button{display:inline-block;width:38vw;min-height:26vh;font-size:9vw;font-weight:bold;margin:2vh 2vw;vertical-align:top}
  .vs .vs-row{width:92vw;margin:0 auto}
  .vs .vs-n{font-size:8vh;padding:8vh 2vw}
  .vs .vs-x{font-size:9vh}
  .vs button{font-size:6vh;padding:4vh;width:76vw;margin-top:5vh}
}
#rotatePrompt{position:fixed;inset:0;background:rgba(0,0,0,0.88);z-index:300;display:none;align-items:center;justify-content:center;flex-direction:column;text-align:center;padding:25px;box-sizing:border-box}
#rotatePrompt .rp-ico{font-size:56px;animation:rpSpin 1.8s ease-in-out infinite}
#rotatePrompt .rp-txt{font-size:18px;color:#ffcc00;font-weight:bold;margin-top:15px;max-width:290px;line-height:1.5}
@keyframes rpSpin{0%,100%{transform:rotate(0deg)}55%{transform:rotate(90deg)}}
#fsHint{position:fixed;inset:0;background:rgba(0,0,0,0.92);z-index:320;display:none;align-items:center;justify-content:center;text-align:center;padding:25px;box-sizing:border-box;cursor:pointer}
#fsHint .fs-box{background:#2a2a35;border-radius:15px;padding:25px;max-width:340px;box-shadow:0 4px 20px #000}
#fsTap{position:fixed;inset:0;background:rgba(0,0,0,0.93);z-index:310;display:none;align-items:center;justify-content:center;flex-direction:column;text-align:center;padding:25px;box-sizing:border-box;cursor:pointer}
</style></head><body>
<div id='topAnchor' style='position:absolute;top:0;left:0;width:1px;height:1px'></div>
<button id='fullscreen-btn' onclick='toggleTelaCheia()'>⛶ TELA CHEIA</button>
<button id='somBtn' onclick='toggleSom()'>🔇 SOM</button>
<a id='cfgBtn' href='/config_camp' style='position:fixed;top:6px;left:10px;background:rgba(255,255,255,0.12);color:#ddd;border:1px solid #555;border-radius:8px;padding:8px 12px;font-size:20px;z-index:99;width:auto;text-decoration:none;line-height:1' title='Configurações'>⚙</a>
<h1>PLACAR DIGITAL</h1>
<div id='batBadge' style='position:fixed;top:5px;right:8px;background:rgba(0,0,0,0.5);color:#4ade80;border-radius:6px;padding:3px 8px;font-size:12px;z-index:99;display:none;transform:translateZ(0);-webkit-transform:translateZ(0);will-change:transform'></div>
<div id='wifiBadge' style='position:fixed;top:5px;left:50%;transform:translate(-50%,0);-webkit-transform:translate(-50%,0);background:rgba(0,0,0,0.5);color:#888;border-radius:6px;padding:3px 8px;font-size:12px;z-index:99;display:none;white-space:nowrap;will-change:transform' title='Conexão com o roteador'></div>
<div id='confModal' style='position:fixed;inset:0;background:rgba(0,0,0,0.75);display:none;z-index:200;align-items:center;justify-content:center;flex-direction:column'>
  <div style='background:#2a2a35;padding:25px;border-radius:15px;max-width:340px;width:90%;text-align:center;box-shadow:0 4px 20px #000'>
    <h3 style='margin:0 0 12px;color:#4CAF50'>CONFIRMAR RESULTADO</h3>
    <div id='confMsg' style='font-size:20px;font-weight:bold;line-height:1.5'></div>
    <div style='margin-top:20px;display:flex;flex-direction:column;gap:10px'>
      <button onclick='confirmarResultado(true)' style='background:#4CAF50;color:#fff;width:100%'>✓ ENVIAR RESULTADO</button>
      <button onclick='confirmarResultado(false)' style='background:#7f0000;color:#fff;width:100%'>✗ CORRIGIR PONTOS</button>
    </div>
  </div>
</div>
<div id='conectado' style='background:#2a7d2e;padding:6px;border-radius:8px;font-size:13px;max-width:400px;margin:0 auto 15px;display:none'>✓ Conectado! Abra <b>192.168.4.1</b> no navegador</div>
<div id='aviso' style='background:#7f0000;color:#fff;padding:10px;border-radius:8px;font-size:14px;font-weight:bold;max-width:400px;margin:0 auto 15px;display:none'></div>
<button id='btnVoltarPartida' onclick='voltarPartida()' style='display:none;background:#1565c0;color:#fff;border:none;border-radius:8px;padding:12px 20px;font-size:16px;font-weight:bold;cursor:pointer;margin:0 auto 15px;width:auto;box-shadow:0 2px 8px rgba(0,0,0,0.4)'>VOLTAR A PARTIDA</button>
<div id='instrucao' style='background:#1a3a5c;padding:8px;border-radius:8px;font-size:13px;max-width:400px;margin:0 auto 15px;color:#aaddff'>📱 Conecte-se à WiFi <b>Placar_Tenis_Mesa</b> e abra <b>tenis_de_mesa.com</b> ou <b>192.168.4.1</b> no navegador</div>
<div id='t1' class='box'><input type='text' id='nA' placeholder='Jogador A' maxlength='15' oninput='this.value=this.value.toUpperCase()'><input type='text' id='nB' placeholder='Jogador B' maxlength='15' oninput='this.value=this.value.toUpperCase()'><button class='btn-g' onclick='go()'>PRÓXIMO</button><button class='btn-hist' onclick='toggleHistIni()'>Exibir Resultados das Partidas</button><div id='histIni' class='hist-box'></div></div>
<div id='t0' class='box vs' style='display:none'><h2 style='margin:0 0 6px'>PRÓXIMA PARTIDA</h2><div class='vs-row'><div class='vs-n' id='tVsA'></div><div class='vs-x'>VS</div><div class='vs-n' id='tVsB'></div></div><div style='margin-top:18px'><button class='btn-g' onclick='iniciarPartida()' style='font-size:22px;padding:16px'>▶ INICIAR PARTIDA</button></div></div>
<div id='t2' class='box' style='display:none'><h2>QUEM COMEÇA O SAQUE?</h2><button id='sA' class='btn-y' onclick='setS(1)'></button><button id='sB' class='btn-y' onclick='setS(2)'></button></div>
<div id='t3' style='display:none'><div id='msg' class='status'></div><div id='autoNextMsg'></div><div class='flex'>
<div class='player-group'><div class='jog' id='cA'><div id='lA'></div><div class='pts' id='pA'>0</div><div style='color:#ffcc00;height:15px' id='skA'></div><button class='btn-m' onclick="cmd('A1')">+1</button><button class='btn-less' onclick="cmd('A0')">-1</button></div><div class='set-box' id='setA'>0</div></div>
<div class='player-group'><div class='set-box' id='setB'>0</div><div class='jog' id='cB'><div id='lB'></div><div class='pts' id='pB'>0</div><div style='color:#ffcc00;height:15px' id='skB'></div><button class='btn-m' onclick="cmd('B1')">+1</button><button class='btn-less' onclick="cmd('B0')">-1</button></div></div>
</div><div style='max-width:300px;margin:20px auto'><button id='btnAction' style='background:#7f0000;color:#fff' onclick='actionBtn()'>REINICIAR PONTOS</button><button style='background:#555;color:#fff' onclick='voltarConfig()'>NOVO JOGO</button><button class='btn-hist' onclick='toggleHist()'>Exibir Resultados dos Sets</button><div id='hist' class='hist-box'></div></div></div>
<button id='trocarLadoBtn' onclick='trocarLado()'>TROCAR LADO</button>
<button id='virarTelaBtn' onclick='virarTela()'>↻ VIRAR TELA</button>
<div id='rotatePrompt'><div class='rp-ico'>📱</div><div class='rp-txt'>Gire o celular de lado para o modo paisagem</div></div>
<div id='fsHint' onclick='document.getElementById("fsHint").style.display="none"'><div class='fs-box'><div style='font-size:40px'>📲</div><div style='font-size:15px;color:#fff;line-height:1.7;margin-top:10px'>No <b>iPhone</b> a tela cheia é feita pelo próprio celular:<br><br>1️⃣ Toque em <b>Compartilhar</b> ⬆️<br>2️⃣ <b>Adicionar à Tela de Início</b><br>3️⃣ Abra pelo ícone (fica sem a barra de endereço)</div><button style='background:#4CAF50;color:#fff;width:auto;padding:10px 30px'>ENTENDI</button></div></div>
<div id='fsTap' onclick='ativarVisual()'><div style='font-size:44px'>📲↻</div><div style='font-size:17px;color:#fff;font-weight:bold;line-height:1.6;margin-top:12px;background:rgba(255,255,255,0.08);padding:16px 22px;border-radius:14px'>Toque para ativar<br>TELA CHEIA + PAISAGEM</div><div style='font-size:12px;color:#9aa5b1;margin-top:14px;max-width:300px;line-height:1.5'>💡 Para abrir sempre em paisagem (tripé), instale como app:<br>menu ⋮ do navegador → “Adicionar à tela inicial”</div></div>
<div id='tCamp' style='position:fixed;inset:0;background:radial-gradient(circle at 50% 28%,#3a2a00,#0d0d10);z-index:150;display:none;align-items:center;justify-content:center;flex-direction:column;text-align:center'>
  <div style='font-size:16vh;line-height:1' ontimeout=''>🏆</div>
  <div style='color:#ffcc00;font-size:7vh;font-weight:900;letter-spacing:2px;margin-top:2vh'>CAMPEÃO</div>
  <div id='campNome' style='color:#fff;font-size:12vh;font-weight:900;text-shadow:0 0 30px #ffcc00;margin-top:2vh;word-break:break-word'></div>
  <div id='campVice' style='display:none;color:#93c5fd;font-size:4.5vh;font-weight:700;margin-top:2vh;letter-spacing:1px'></div>
  <button onclick='sairCampeao()' style='position:fixed;top:12px;right:16px;background:rgba(255,255,255,0.12);color:#ddd;border:1px solid #555;border-radius:50%;width:13vh;height:13vh;font-size:6vh;line-height:1;cursor:pointer;z-index:151'>✕</button>
</div>
<script>
if(history&&history.scrollRestoration){history.scrollRestoration='manual'}
var isSetF=false, fin=false, autoNextTimer=null, autoFinalTimer=null, wakeLock=null, wakeAudioCtx=null, wakeInterval=null, wakePing=null, wakeVideo=null, wakeOsc=null, wakeGain=null, flipped=false, somAtivo=false, antigoPA=-1, antigoPB=-1, ultimoH='', emJogando=false;
var VERSAO_PAGINA=30, checadoVersao=false;

var rolarAte=0;
function rolarScroll(){function s(){try{var x=document.getElementById('topAnchor');if(x&&x.scrollIntoView)x.scrollIntoView()}catch(e){}window.scrollTo(0,0);try{window.scrollTo({top:0,left:0,behavior:'auto'})}catch(e){}if(document.body)document.body.scrollTop=0;if(document.documentElement)document.documentElement.scrollTop=0}s();s();setTimeout(s,40);setTimeout(s,120)}
function rolarTopo(){var a=document.activeElement;if(a&&typeof a.blur==='function'){try{a.blur()}catch(e){}}rolarAte=Date.now()+1800;rolarScroll()}
function reafirmarTopo(){if(rolarAte>Date.now()){rolarScroll()}}

// Renderiza os dados recebidos na tela do celular
function desenharTela(d) {
  if(!checadoVersao&&typeof d.ver==='number'&&d.ver!==VERSAO_PAGINA){checadoVersao=true;location.reload();return}
  checadoVersao=true;
  ultimoH=d.h;
  var cmp=id('tCamp');
  if(d.camp && !d.ini){cmp.style.display='flex';
    var partes=d.camp.split('  ·  ');
    id('campNome').innerText=partes[0];
    var sub=id('campVice');
    if(partes.length>1){sub.style.display='block';sub.innerHTML='2º LUGAR · '+partes[1]}else{sub.style.display='none'}
    ativarTelaAtiva();
    if(document.documentElement.requestFullscreen){document.documentElement.requestFullscreen().catch(function(){})}
    if(screen.orientation&&screen.orientation.lock){screen.orientation.lock('landscape').catch(function(){})}
    return}
  cmp.style.display='none';
  var sv=id('campVice');if(sv)sv.style.display='none';
  var bb=id('batBadge');
  if(typeof d.b==='number'&&d.b>=0){bb.style.display='block';bb.textContent='🔋 '+d.b+'%';bb.style.color=d.b<20?'#ff4444':(d.b<50?'#ffcc00':'#4ade80')}else{bb.style.display='none'}
  var wb=id('wifiBadge');
  if(d.sta){wb.style.display='block';wb.textContent='📶 '+d.staIP;wb.style.color='#4ade80';wb.style.background='rgba(0,60,0,0.6)'}else{wb.style.display='block';wb.textContent='📡 roteador não conectado';wb.style.color='#ff4444';wb.style.background='rgba(90,0,0,0.6)'}
  if(id('histIni').style.display=='block'){id('histIni').innerHTML=d.h}
  if(d.aviso){id('aviso').style.display='block';id('aviso').innerText=d.aviso}else{id('aviso').style.display='none'}
  id('btnVoltarPartida').style.display=d.pend?'block':'none';
  id('t0').style.display='none';
  if(!d.ini){emJogando=false;cancelarAutoFinal();cancelarAutoNext();show('t1','t2','t3');document.body.classList.remove('jogando');id('instrucao').style.display='block';antigoPA=-1;antigoPB=-1;rolarTopo()}else if(d.alg&&!d.srt){emJogando=false;cancelarAutoFinal();cancelarAutoNext();show('t0','t1','t2');id('t3').style.display='none';document.body.classList.remove('jogando');id('instrucao').style.display='none';id('tVsA').innerText=d.nA;id('tVsB').innerText=d.nB;antigoPA=-1;antigoPB=-1;tocarVsSom();rolarTopo()}else if(!d.srt){emJogando=false;cancelarAutoFinal();cancelarAutoNext();show('t2','t1','t3');document.body.classList.remove('jogando');id('instrucao').style.display='none';id('sA').innerText=d.nA;id('sB').innerText=d.nB;antigoPA=-1;antigoPB=-1;rolarTopo()}else{show('t3','t1','t2');document.body.classList.add('jogando');id('instrucao').style.display='none';
    id('lA').innerText=d.nA;id('lB').innerText=d.nB;id('pA').innerText=d.pA;id('pB').innerText=d.pB;
    id('setA').innerText=d.sA;id('setB').innerText=d.sB;
    if(document.documentElement.requestFullscreen){document.documentElement.requestFullscreen().catch(function(){})}
    if(screen.orientation&&screen.orientation.lock){screen.orientation.lock('landscape').catch(function(){})}
    if(!emJogando||(+d.pA===0&&+d.pB===0)){emJogando=true;ativarTelaAtiva();rolarTopo()}
    if(antigoPA>=0&&antigoPB>=0){
      if(d.pA>antigoPA||d.pB>antigoPB){tocarSom(880,0.15)}
      else if(d.pA<antigoPA||d.pB<antigoPB){tocarSom(660,0.25)}
    }
    if(d.sf&&!isSetF){setTimeout(function(){tocarSom(880,0.08);setTimeout(function(){tocarSom(1100,0.1)},120);setTimeout(function(){tocarSom(1320,0.15)},260)},50)}
    if(!d.sf&&isSetF){rolarTopo()}
    isSetF=d.sf;
    fin=d.fin;
    antigoPA=d.pA;antigoPB=d.pB;
  if(d.sf&&d.fin){cancelarAutoNext();iniciarAutoFinal()}
  else if(d.sf&&!d.fin){cancelarAutoFinal();if(!autoNextTimer)iniciarAutoNext()}
  else{cancelarAutoFinal();cancelarAutoNext()}
  id('cA').className=d.sq==1?'jog sacando':'jog';id('cB').className=d.sq==2?'jog sacando':'jog';
  id('skA').innerText=d.sq==1?'SAQUE':'';id('skB').innerText=d.sq==2?'SAQUE':'';
  id('msg').style.display=d.sf?'block':'none';id('msg').innerText=d.msg;
  id('btnAction').innerText=d.fin?'FINALIZAR PARTIDA':(d.sf?'INICIAR PRÓXIMO SET':'REINICIAR PONTOS');
  id('hist').innerHTML=d.h;
  var vt=id('virarTelaBtn');
  if(d.ncl>=2){vt.classList.add('multi')}
  else{if(flipped){flipped=false;document.body.classList.remove('virado');vt.innerText='↻ VIRAR TELA'}vt.classList.remove('multi')}}
  if(d.cmp){
    ativarTelaAtiva();
    if(document.documentElement.requestFullscreen){document.documentElement.requestFullscreen().catch(function(){})}
    if(screen.orientation&&screen.orientation.lock){screen.orientation.lock('landscape').catch(function(){})}
    var pp=document.getElementById('rotatePrompt');if(pp)pp.style.display='none';
  }
  atualizarRotacao();
  checarVisual();
  reafirmarTopo();
}

// Redireciona URLs de portal cativo para a raiz
var p=window.location.pathname;
if(p!='/'&&p!='/dados'){history.replaceState(null,'','/')}
// Mostra confirmação de conexão
setTimeout(function(){var e=id('conectado');if(e)e.style.display='block';setTimeout(function(){if(e)e.style.display='none'},6000)},500)

// Inicializa a pagina puxando o estado atual
fetch('/dados').then(r=>r.json()).then(desenharTela);
// Mantém o dispositivo marcado como ativo no ESP32 (detecção de presença)
setInterval(function(){fetch('/ping',{cache:'no-store'}).catch(function(){})},2500);

// ESCUTA EVENTOS DA PORTA 82: Resposta instantânea dos botões físicos
var source = new EventSource('http://' + window.location.hostname + ':82/events');
source.onmessage = function(event) {
  desenharTela(JSON.parse(event.data));
  reafirmarTopo();
};
source.onerror = function() {
  // Se o SSE falhar, cai no fallback de polling a cada 500ms
  if (source.readyState === EventSource.CLOSED) {
    setInterval(function() {
      fetch('/dados').then(r=>r.json()).then(desenharTela).catch(function(){});
    }, 500);
  }
};

function iniciarAutoNext() {
  if(autoNextTimer||!isSetF)return;
  var s=12,e=id('autoNextMsg');e.style.display='block';e.innerText='PRÓXIMO SET EM '+s+'s';
  autoNextTimer=setInterval(function(){
    s--;if(s<=0){clearInterval(autoNextTimer);autoNextTimer=null;e.style.display='none';fetch('/next_set').then(function(r){return r.json()}).then(desenharTela)}
    else{e.innerText='PRÓXIMO SET EM '+s+'s'}
  },1000);
}
function cancelarAutoNext(){
  if(autoNextTimer){clearInterval(autoNextTimer);autoNextTimer=null;var e=id('autoNextMsg');if(e)e.style.display='none'}
}
function iniciarAutoFinal() {
  if(autoFinalTimer||!fin)return;
  var s=12,e=id('autoNextMsg');e.style.display='block';e.innerText='ENVIAR RESULTADO EM '+s+'s';
  autoFinalTimer=setInterval(function(){
    s--;if(s<=0){clearInterval(autoFinalTimer);autoFinalTimer=null;e.style.display='none';voltarConfig()}
    else{e.innerText='ENVIAR RESULTADO EM '+s+'s'}
  },1000);
}
function cancelarAutoFinal(){
  if(autoFinalTimer){clearInterval(autoFinalTimer);autoFinalTimer=null;var e=id('autoNextMsg');if(e)e.style.display='none'}
}
function ativarTelaAtiva(){
  if(!document.getElementById('_awake')){
    var d=document.createElement('div');d.id='_awake';d.className='_awake-indicator';
    document.body.appendChild(d);
  }
  function pedir(){
    if(navigator.wakeLock){
      navigator.wakeLock.request('screen').then(function(w){
        wakeLock=w;
        w.addEventListener('release',function(){wakeLock=null;setTimeout(pedir,50)});
      }).catch(function(){});
    }
    if(navigator.requestWakeLock){
      try{var w=navigator.requestWakeLock('screen');if(w)setTimeout(function(){pedir()},2000)}catch(e){}
    }
  }
  pedir();
  if(wakeInterval) clearInterval(wakeInterval);
  wakeInterval=setInterval(pedir,1500);
  document.addEventListener('visibilitychange',function vis(){
    if(document.visibilityState==='visible'){pedir();setTimeout(pedir,100)}
  });
  if(!wakePing){
    wakePing=setInterval(function(){
      void(document.body.offsetHeight);
      var e=document.getElementById('_awake');if(e)e.style.transform='translateX('+(Math.random()*2)+'px)';
    },200);
  }
  if(!wakeAudioCtx){
    try{
      wakeAudioCtx=new(window.AudioContext||window.webkitAudioContext)();
      wakeGain=wakeAudioCtx.createGain();wakeGain.gain.value=0.003;
      wakeOsc=wakeAudioCtx.createOscillator();wakeOsc.frequency.value=25;wakeOsc.type='sine';
      wakeOsc.connect(wakeGain);wakeGain.connect(wakeAudioCtx.destination);wakeOsc.start();
    }catch(e){}
  }else if(wakeAudioCtx.state==='suspended'){
    wakeAudioCtx.resume().catch(function(){});
    if(wakeOsc&&wakeOsc.frequency){try{wakeOsc.start()}catch(e){}}
  }
  if(!wakeVideo){
    try{
      var cc=document.createElement('canvas');cc.width=100;cc.height=100;
      var gc=cc.getContext('2d');gc.fillStyle='#000';gc.fillRect(0,0,100,100);
      wakeVideo=document.createElement('video');wakeVideo.playsInline=true;
      wakeVideo.style.position='fixed';wakeVideo.style.bottom='0';wakeVideo.style.right='0';
      wakeVideo.style.width='2px';wakeVideo.style.height='2px';wakeVideo.style.opacity='0.99';wakeVideo.style.zIndex='-1';
      document.body.appendChild(wakeVideo);
      var st=cc.captureStream(10);
      if(wakeAudioCtx&&wakeAudioCtx.createMediaStreamDestination){
        var d=wakeAudioCtx.createMediaStreamDestination();
        if(wakeOsc){wakeOsc.connect(d)}
        st.addTrack(d.stream.getAudioTracks()[0]);
      }
      wakeVideo.srcObject=st;wakeVideo.loop=true;wakeVideo.muted=true;wakeVideo.play().catch(function(){});
    }catch(e){}
  }
}
function desativarTelaAtiva(){
  var e=document.getElementById('_awake');if(e)e.remove();
  if(wakeLock){wakeLock.release().catch(function(){});wakeLock=null}
  if(wakeInterval){clearInterval(wakeInterval);wakeInterval=null}
  if(wakePing){clearInterval(wakePing);wakePing=null}
  if(wakeOsc){try{wakeOsc.stop()}catch(ee){};wakeOsc=null}
  if(wakeGain){wakeGain.disconnect();wakeGain=null}
  if(wakeAudioCtx){wakeAudioCtx.close().catch(function(){});wakeAudioCtx=null}
  if(wakeVideo){wakeVideo.pause();wakeVideo.src='';wakeVideo.load();wakeVideo.remove();wakeVideo=null}
}
function toggleTelaCheia() {
  var sup=document.documentElement.requestFullscreen||document.documentElement.webkitRequestFullscreen;
  if(!sup){document.getElementById('fsHint').style.display='flex';return}
  var ativo=document.fullscreenElement||document.webkitFullscreenElement;
  if(!ativo){
    var f=document.documentElement.requestFullscreen||document.documentElement.webkitRequestFullscreen;
    f.call(document.documentElement).catch(function(){});
  }else{
    if(document.exitFullscreen)document.exitFullscreen().catch(function(){});
    else if(document.webkitExitFullscreen)document.webkitExitFullscreen();
  }
}
function ativarVisual(){
  var f=document.documentElement.requestFullscreen||document.documentElement.webkitRequestFullscreen;
  var d=document.documentElement;
  var terminar=function(){
    document.getElementById('fsTap').style.display='none';
    setTimeout(function(){
      if(screen.orientation&&screen.orientation.lock){screen.orientation.lock('landscape').catch(function(){})}
    },300);
  };
  if(f){
    var p=f.call(d);
    if(p&&p.then)p.then(terminar).catch(function(){terminar()});
    else terminar();
  }else{
    terminar();
  }
}
function atualizarRotacao(){
  var p=document.getElementById('rotatePrompt');
  if(!p)return;
  var suportaLock=!!(screen.orientation&&screen.orientation.lock);
  var jg=(document.body.classList.contains('jogando')||document.getElementById('tCamp').style.display==='flex')&&true;
  var retrato=window.matchMedia&&window.matchMedia('(orientation:portrait)').matches;
  p.style.display=(jg&&retrato&&!suportaLock)?'flex':'none';
}
function checarVisual(){
  var jg=(document.body.classList.contains('jogando')||document.getElementById('tCamp').style.display==='flex')&&true;
  var ativo=document.fullscreenElement||document.webkitFullscreenElement;
  var retrato=window.matchMedia&&window.matchMedia('(orientation:portrait)').matches;
  var appAberto=(window.matchMedia&&(window.matchMedia('(display-mode: fullscreen)').matches||window.matchMedia('(display-mode: standalone)').matches));
  var tap=document.getElementById('fsTap');
  if(!tap)return;
  // Em modo app (instalado na tela inicial) já está em tela cheia: nunca pede toque.
  if(appAberto){if(tap.__mostrado){tap.__mostrado=false;tap.style.display='none'}return}
  // Aba normal: só pede toque quando o celular está em pé (retrato) e sem tela cheia.
  // Se o celular já está deitado (paisagem) no tripé, não mostra nada — já está correto.
  if(jg&&retrato&&!ativo&&!document.getElementById('fsHint').style.display==='flex'){
    if(!tap.__mostrado){tap.__mostrado=true;tap.style.display='flex'}
  }else if(tap.__mostrado){tap.__mostrado=false;tap.style.display='none'}
}
document.addEventListener('fullscreenchange',function(){setTimeout(checarVisual,300)});
document.addEventListener('webkitfullscreenchange',function(){setTimeout(checarVisual,300)});
window.addEventListener('orientationchange',function(){setTimeout(atualizarRotacao,300)});
window.addEventListener('resize',function(){setTimeout(atualizarRotacao,300)});
// 2a tela em paisagem: o celular 2 nao faz o gesto de iniciar partida, e o
// navegador so permite requestFullscreen()/orientation.lock() dentro de um
// gesto do usuario. Capturamos o PRIMEIRO touch/click em qualquer lugar da
// tela e, se o jogo ja comecou (jogando), ja forca tela cheia + paisagem
// antes do botao agir. Cooldown curto evita repetir a cada clique.
var ultimoTapPaisagem=0;
function forcarPaisagem(){
  var jg=document.body.classList.contains('jogando')||document.getElementById('tCamp').style.display==='flex';
  if(!jg)return;
  var agora=Date.now();
  if(agora-ultimoTapPaisagem<4000)return;
  var ativo=document.fullscreenElement||document.webkitFullscreenElement;
  if(!ativo){
    var f=document.documentElement.requestFullscreen||document.documentElement.webkitRequestFullscreen;
    if(f){f.call(document.documentElement).catch(function(){})}
  }
  if(screen.orientation&&screen.orientation.lock){screen.orientation.lock('landscape').catch(function(){})}
  var tap=document.getElementById('fsTap');
  if(tap){tap.style.display='none';tap.__mostrado=false}
  ultimoTapPaisagem=agora;
}
document.addEventListener('touchstart',forcarPaisagem,true);
document.addEventListener('click',forcarPaisagem,true);
function tocarVsSom(){
  if(!somAtivo)return;
  tocarSom(880,0.15);
  setTimeout(function(){if(somAtivo)tocarSom(1100,0.15)},200);
  setTimeout(function(){if(somAtivo)tocarSom(880,0.15)},400);
}
function iniciarPartida(){
  rolarTopo();
  if(wakeAudioCtx&&wakeAudioCtx.state==='suspended'){wakeAudioCtx.resume().catch(function(){})}
  fetch('/iniciar_partida').then(function(r){return r.json()}).then(desenharTela);
}
function voltarConfig() {
  cancelarAutoNext();
  cancelarAutoFinal();
  rolarTopo();
  fetch('/controle?cmd=config_tela').then(function(r){return r.json()}).then(function(d){
    if(d.confirm){
      var oCampeonato=d.cmp||d.camp;
      if(oCampeonato){
        if(document.documentElement.requestFullscreen){document.documentElement.requestFullscreen().catch(function(){})}
        if(screen.orientation&&screen.orientation.lock){screen.orientation.lock('landscape').catch(function(){})}
      }
      var vencedor=d.sA>d.sB?d.nA:(d.sB>d.sA?d.nB:'EMPATE');
      id('confMsg').innerHTML='<b>'+d.nA+'</b> '+d.sA+' x '+d.sB+' <b>'+d.nB+'</b>'
        +'<div style='+'"font-size:14px;color:#4ade80;margin-top:8px;font-weight:bold"'+'>'+(d.sA===d.sB?'EMPATE':vencedor+' VENCEU')+'</div>'
        +'<div style='+'"font-size:12px;color:#aaa;margin-top:6px;font-weight:normal"'+'>Enviar este resultado para o campeonato?</div>';
      id('confModal').style.display='flex';
      return;
    }
    if(!d.cmp&&!d.camp){
      if(document.exitFullscreen){document.exitFullscreen().catch(function(){})}
      if(screen.orientation&&screen.orientation.unlock){screen.orientation.unlock()}
    }
    id('nA').value='';id('nB').value='';show('t1','t2','t3');id('instrucao').style.display='block';
    desenharTela(d);
    if(d.ini&&d.srt){ativarTelaAtiva()}
  }).catch(function(){});
}
function confirmarResultado(ok){
  id('confModal').style.display='none';
  rolarTopo();
  fetch('/controle?cmd='+(ok?'confirmar_resultado':'cancelar_resultado')).then(function(r){return r.json()}).then(function(d){
    desenharTela(d);
    if(d.ini&&d.srt){ativarTelaAtiva()}else{id('nA').value='';id('nB').value='';show('t1','t2','t3');id('instrucao').style.display='block'}
  }).catch(function(){});
}
function sairCampeao(){
  if(document.exitFullscreen){document.exitFullscreen().catch(function(){})}
  if(screen.orientation&&screen.orientation.unlock){screen.orientation.unlock()}
  fetch('/api/limpar_campeao',{method:'POST'}).then(function(r){return r.json()}).then(function(){
    return fetch('/dados',{method:'POST'});
  }).then(function(r){return r.json()}).then(desenharTela).catch(function(){});
}
function id(i){return document.getElementById(i)}function show(a,b,c){id(a).style.display='block';id(b).style.display='none';id(c).style.display='none'}
function go(){rolarTopo();var a=id('nA').value.trim()||'Jogador A',b=id('nB').value.trim()||'Jogador B';id('sA').innerText=a;id('sB').innerText=b;show('t2','t1','t3');fetch('/config?nA='+encodeURIComponent(a)+'&nB='+encodeURIComponent(b)).then(function(r){return r.json()}).then(desenharTela);}
function setS(i){
  cancelarAutoNext();rolarTopo();show('t3','t1','t2');
  if(wakeAudioCtx&&wakeAudioCtx.state==='suspended'){wakeAudioCtx.resume().catch(function(){})}
  ativarTelaAtiva();
  if(document.documentElement.requestFullscreen){document.documentElement.requestFullscreen().catch(function(){})}
  if(screen.orientation&&screen.orientation.lock){screen.orientation.lock('landscape').catch(function(){})}
  fetch('/definirSaque?id='+i).then(function(r){return r.json()}).then(desenharTela)
}
function reativarTela(){
  if(navigator.wakeLock&&(!wakeLock||document.visibilityState!='visible')){navigator.wakeLock.request('screen').then(function(w){wakeLock=w}).catch(function(){})}
  if(wakeAudioCtx&&wakeAudioCtx.state==='suspended'){wakeAudioCtx.resume().catch(function(){})}
  if(wakeVideo&&wakeVideo.paused){wakeVideo.play().catch(function(){})}
}
function cmd(c){
  cancelarAutoNext();reativarTela();
  if(c=='A1'||c=='B1'){tocarSom(880,0.15)}
  else if(c=='A0'||c=='B0'){tocarSom(330,0.2)}
  fetch('/controle?cmd='+c).then(function(r){return r.json()}).then(desenharTela)
}
function voltarPartida(){
  cancelarAutoNext();rolarTopo();
  if(wakeAudioCtx&&wakeAudioCtx.state==='suspended'){wakeAudioCtx.resume().catch(function(){})}
  ativarTelaAtiva();
  if(document.documentElement.requestFullscreen){document.documentElement.requestFullscreen().catch(function(){})}
  if(screen.orientation&&screen.orientation.lock){screen.orientation.lock('landscape').catch(function(){})}
  fetch('/controle?cmd=retomar_partida').then(function(r){return r.json()}).then(desenharTela)
}
function actionBtn(){cancelarAutoNext();reativarTela();rolarTopo();
  if(fin){voltarConfig();return}
  if(isSetF){fetch('/next_set').then(function(r){return r.json()}).then(desenharTela)}
  else{cmd('reset')}
}
function trocarLado(){cancelarAutoNext();reativarTela();fetch('/trocarLado').then(function(r){return r.json()}).then(desenharTela)}
function virarTela(){flipped=!flipped;document.body.classList.toggle('virado',flipped);id('virarTelaBtn').innerText=flipped?'↺ NORMAL':'↻ VIRAR TELA'}
function toggleHist(){var s=id('hist').style;s.display=s.display=='block'?'none':'block'}
function toggleHistIni(){var s=id('histIni').style;s.display=s.display=='block'?'none':'block';if(s.display=='block'){id('histIni').innerHTML=ultimoH}}
function tocarSom(freq,dur){
  if(!somAtivo)return;
  try{
    var ctx=wakeAudioCtx;
    if(!ctx||ctx.state==='closed')return;
    var g=ctx.createGain();g.gain.value=0.2;g.connect(ctx.destination);
    var o=ctx.createOscillator();o.frequency.value=freq;o.type='sine';
    o.connect(g);o.start(ctx.currentTime);o.stop(ctx.currentTime+dur);
  }catch(e){}
}
function toggleSom(){
  somAtivo=!somAtivo;
  var b=id('somBtn');
  if(somAtivo){
    b.innerText='🔊 SOM';b.classList.add('ativo');
    tocarSom(660,0.12);
    id('btnAction').style.border='2px solid #4CAF50';
    setTimeout(function(){id('btnAction').style.border=''},200);
  }else{
    b.innerText='🔇 SOM';b.classList.remove('ativo');
  }
}
</script></body></html>
)rawhtml";

// Página do portal cativo: o Android/Motorola frequentemente abre o captivo num
// navegador leve (não-Chrome) sem tela cheia/rotação. Aqui oferecemos abrir no Chrome.
const char paginaCaptivaHTML[] PROGMEM = R"rawhtml(
<!DOCTYPE html><html lang="pt"><head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1, maximum-scale=1">
<title>PLACAR</title>
<style>
html,body{margin:0;padding:0;background:#0b0f1a;color:#fff;font-family:Arial,Helvetica,sans-serif;height:100%}
body{display:flex;flex-direction:column;align-items:center;justify-content:center;text-align:center;padding:24px;box-sizing:border-box}
h1{font-size:34px;margin:0 0 8px}
p{color:#9aa5b1;font-size:15px;margin:8px 0 26px;line-height:1.5}
    a.btn{display:block;width:100%;max-width:340px;padding:18px;margin:10px auto;border-radius:14px;text-decoration:none;font-size:18px;font-weight:800;text-align:center;box-sizing:border-box}
.chrome{background:#1a73e8;color:#fff}
.hint{font-size:13px;color:#7d8894;margin-top:18px}
</style>
</head><body>
<h1>TÊNIS DE MESA</h1>
<p>Para o placar funcionar em tela cheia e paisagem automática, abra no Google Chrome.</p>
<a class="btn chrome" id="btnChrome" href="intent://192.168.4.1/#Intent;scheme=http;package=com.android.chrome;S.browser_fallback_url=http%3A%2F%2F192.168.4.1%2F;end">ABRIR NO CHROME</a>
<p class="hint">Se não abrir, verifique se o Google Chrome está instalado ou abra http://192.168.4.1 manualmente no Chrome.</p>
<script>
if(/iPhone|iPad|iPod/i.test(navigator.userAgent)){
  document.getElementById('btnChrome').href='googlechrome://192.168.4.1';
}
</script>
</body></html>
)rawhtml";

#endif
