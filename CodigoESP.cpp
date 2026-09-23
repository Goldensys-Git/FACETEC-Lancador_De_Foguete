#include <ESP32Servo.h>
#include <ArduinoJson.h>
#include <WebServer.h>
#include <WiFi.h>



/*

  PROTÓTIPO DE FUNÇÕES USADAS

*/

const char* stateName(byte state);
void startCountdown();
void resetSystem();

// ==========================================================
// PINOS
// ==========================================================


//GPIO 6, 7, 8, 9, 10 e 11

const byte LED_1 = 13;
const byte LED_2 = 27;

const byte BUTTON_START = 26;
const byte BUTTON_EMERGENCY = 25;

// SERVOS
const byte SERVO_READY_PIN = 4;
const byte SERVO_ABORT_PIN = 5;

// SENSOR DE PRESSÃO / POTENCIÔMETRO
const byte PRESSURE_SENSOR = 32;


// ==========================================================
// SERVO
// ==========================================================

// Posições usadas apenas como indicação visual de estado
const int SERVO_INITIAL_POSITION = 0;
const int SERVO_INDICATOR_POSITION = 90;

Servo servoReady;
Servo servoAbort;


// ==========================================================
// PRESSÃO
// ==========================================================

// 0    -> pressão mínima
// 1023 -> pressão máxima

const int PRESSURE_LIMIT = 800;


// ==========================================================
// ESTADOS
// ==========================================================

const byte STATE_SAFE = 0;
const byte STATE_COUNTDOWN = 1;
const byte STATE_READY = 2;
const byte STATE_ABORTED = 3;

byte currentState = STATE_SAFE;


// ==========================================================
// CONTAGEM
// ==========================================================

const byte COUNTDOWN_INITIAL = 10;

const unsigned long ONE_SECOND = 1000UL;
const unsigned long DEBOUNCE_TIME = 50UL;

byte countdown = COUNTDOWN_INITIAL;

unsigned long countdownTimer = 0;


// ==========================================================
// EMERGENCY
// ==========================================================

bool emergencyLatched = false;


// ==========================================================
// BOTÃO START
// ==========================================================

bool startRawState = HIGH;
bool startStableState = HIGH;

unsigned long startChangeTime = 0;


// ==========================================================
// BOTÃO EMERGENCY
// ==========================================================

bool emergencyRawState = HIGH;
bool emergencyStableState = HIGH;

unsigned long emergencyChangeTime = 0;






/*
    GLOBAIS PARA REQUISIÇÕES DO FRONT-END
*/

WebServer server(80);

const char *ssid = "Servidor";
const char *password = "";

const char *SenhaAcesso = "123Mudar";
String CustomMessageAtual = "";
int CustomMessageDurationAtual = 0;


const char html_page[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="pt-br">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Menu Principal</title>
    <style>
        /* Grids e Resets base */
        * { margin: 0; padding: 0; box-sizing: border-box; }
        .container { width: 800px; margin: 0 auto; }
        .row { display: grid; grid-template-columns: repeat(12, 1fr); }
        .col-4 { grid-column-end: span 4; }
        .col-6 { grid-column-end: span 6; }
        .col-12 { grid-column-end: span 12; }
        /* Fundo Animado */
        @keyframes MoveFundo { 0% { background-position: 0% 50%; } 50% { background-position: 100% 50%; } 100% { background-position: 0% 50%; } }
        body { background: linear-gradient(45deg, #9182e4, #765ece, rgb(45, 43, 196)); min-height: 100vh; animation: MoveFundo 15s ease infinite; background-size: 400% 400%; }
        /* Estilização do Pop-up */
        .popup-senha { position: fixed; width: 100%; height: 100%; z-index: 1; background-color: rgba(0, 0, 0, 0.85); display: flex; align-items: center; justify-content: center; }
        .input-senha { width: 300px; height: 35px; font-size: 16px; border: 2.5px solid rgb(20, 80, 5); border-radius: 2px; outline: none; }
        .input-senha:focus { border-color: #0066cc; }
        .botao-enviar-senha, .botao-sou-visitante { height: 60px; width: 140px; font-weight: 600; }
        .botao-enviar-senha { background-color: rgb(0, 255, 0); }
        .botao-sou-visitante { background-color: rgb(0, 255, 242); }
        .display-senha-incorreta { color: red; font-weight: 600; display: none; }
        /* Alinhamentos */
        .progress-container, .centertext { display: flex; align-items: center; justify-content: center; }
        .topo-text { color: white; font-weight: bold; }
        /* Medidores Circulares */
        .progress-circle-pressao, .progress-circle-countdown, .progress-circle-status { width: 200px; height: 100px; border-radius: 120px 120px 0 0; position: relative; --progress: 10deg; }
        .progress-circle-status::before, .progress-circle-pressao::before, .progress-circle-countdown::before { content: ""; position: absolute; left: 50%; transform: translateX(-50%); bottom: 0; border-radius: 100px 100px 0 0; width: 160px; height: 80px; background-color: #0a1410; }
        .progress-circle-pressao { background: conic-gradient(from 270deg at 50% 100%, rgb(251, 255, 0) var(--progress), rgb(94, 94, 94) var(--progress)); }
        .progress-circle-countdown { background-color: red; }
        .progress-circle-status { background-color: green; }
        .progress-value-countdown, .progress-value-status, .progress-value-pressao { position: absolute; top: 50%; left: 50%; transform: translateX(-50%); color: #fffefe; bottom: 10px; }
        /* Botões do Controle (Sua correção de Ocultação Aplicada) */
        .botao-lancamento, .botao-abortar, .botao-reset { display: none; width: 250px; border-radius: 25px; cursor: pointer; }
        .descritive-value { position: absolute; left: 50%; top: 100%; transform: translateX(-50%); color: white; }
        
        .botao-lancamento { background-color: rgb(64, 226, 49); border: none; color: white; text-align: center; font-size: 16px; font-weight: bold; padding: 15px 32px; }
        .botao-abortar, .botao-reset { background-color: rgb(248, 8, 8); border: none; color: white; text-align: center; font-size: 16px; font-weight: bold; padding: 15px 32px; }
        .botao-sou-visitante:active, .botao-enviar-senha:active, .botao-lancamento:active, .botao-abortar:active, .botao-reset:active { transform: scale(0.90); }
        .custom-mensagem, .popup-custom-message { color: red; background-color: black; font-weight: 200; display: none; }
    </style>
</head>
<body>
    <div class="popup-senha">
        <div class="container">
            <div class="row">
                <div class="col-12 centertext"><input type="password" placeholder="Digite a senha: ..." class="input-senha"></div>
            </div>
            <div class="row">
                <div class="col-12 centertext"><h4 class="display-senha-incorreta">ERRO: Senha inválida, tente novamente</h4></div>
            </div>
            <div class="row">
                <div class="col-12 centertext" style="gap: 10px;">
                    <button class="botao-enviar-senha">Enviar</button>
                    <button class="botao-sou-visitante">Sou Visitante</button>
                </div>
            </div>
            <div class="row" style="margin-top: 15px;">
                <div class="col-12 centertext"><h3 class="popup-custom-message"></h3></div>
            </div>
        </div>
    </div>
    <div class="container">
        <div class="row"><div class="col-12 centertext topo-text"><h1>MENU DE CONTROLE</h1></div></div>
    </div>
    <div class="container centertext" style="margin-top: 25px; gap: 65px;">
        <div class="progress-container">
            <div class="progress-circle-pressao">
                <h1><span class="progress-value-pressao">0%</span></h1>
                <h2><span class="descritive-value">PRESSÃO</span></h2>
            </div>
        </div>
        <div class="progress-container">
            <div class="progress-circle-countdown">
                <h3><span class="progress-value-countdown">AGUARDANDO</span></h3>
                <h2><span class="descritive-value">COUNTDOWN</span></h2>
            </div>
        </div>
        <div class="progress-container">
            <div class="progress-circle-status">
                <h3><span class="progress-value-status">SEGURO</span></h3>
                <h2><span class="descritive-value">STATUS</span></h2>
            </div>
        </div>
    </div>
    <div class="container" style="margin-top: 40px;">
        <div class="row centertext" style="gap: 25px;">
            <div class="centertext col-4"><button class="botao-lancamento">INICIAR LANÇAMENTO</button></div>
            <div class="centertext col-4"><button class="botao-abortar">ABORTAR</button></div>
            <div class="centertext col-4"><button class="botao-reset">REINICIAR SISTEMA</button></div>
        </div>
        <div class="row" style="margin-top: 25px;"> 
            <div class="col-12 centertext"><h3><span class="custom-mensagem"></span></h3></div>
        </div>
    </div>
    <script>
        const Popup_Senha = document.querySelector(".popup-senha");
        const Senha_Digitada = document.querySelector(".input-senha");
        const Botao_Enviar = document.querySelector(".botao-enviar-senha");
        const Botao_Sou_Visitante = document.querySelector(".botao-sou-visitante");
        const Display_Senha_Invalida = document.querySelector(".display-senha-incorreta");
        const Botao_Lancamento = document.querySelector(".botao-lancamento");
        const Botao_Abortar = document.querySelector(".botao-abortar");
        const Botao_Reset = document.querySelector(".botao-reset");
        // Sua correção Aplicada!
        function Autorizado() {
            Popup_Senha.style.display = "none";
            Botao_Lancamento.style.display = "inline-block";
            Botao_Abortar.style.display = "inline-block";
            Botao_Reset.style.display = "inline-block"; 
        }
        function SouVisitante() {
            Popup_Senha.style.display = "none";
        }
        function MostrarSenhaInvalida() {
            Display_Senha_Invalida.style.display = "flex";
        }
        // Correção de Lógica: Sobrevivendo ao Refresh (F5)
        function ValidarAcesso() {
            if (!sessionStorage.getItem("SenhaAutorizada")) {
                Popup_Senha.style.display = "flex";
            } else {
                Autorizado();
            }
        }
        ValidarAcesso();
        function ValidarSenha(TentativaSenha) {
            fetch("/login", {
                method: "POST",
                headers: { "Content-Type": "application/json" },
                body: JSON.stringify({ senha: TentativaSenha })
            }).then((Resposta) => Resposta.json())
              .then((json) => {
                  if (json.status === "autorizado") {
                      sessionStorage.setItem("SenhaAutorizada", json.senhavalidada);
                      Autorizado();
                  } else {
                      MostrarSenhaInvalida();
                  }
              }).catch((error) => {
                  DisplayCustomMessage("Sem Conexão com o Servidor.");
              });
        }
        function SenhaSalva() { return sessionStorage.getItem("SenhaAutorizada"); }
        Botao_Enviar.addEventListener("click", () => { ValidarSenha(Senha_Digitada.value); });
        Botao_Sou_Visitante.addEventListener("click", () => { SouVisitante(); });
        const Pressao_Progress = document.querySelector(".progress-circle-pressao");
        const Pressao_Progress_Text = document.querySelector(".progress-value-pressao");
        let Pressao_Valor_Atual = 0;
        let Pressao_Animacao;
        function UpdatePressao(NewValue) {
            let Velocidade = 20;
            clearInterval(Pressao_Animacao);
            Pressao_Animacao = setInterval(() => {
                if (Pressao_Valor_Atual < NewValue) { Pressao_Valor_Atual++; } 
                else if (Pressao_Valor_Atual > NewValue) { Pressao_Valor_Atual--; } 
                else { clearInterval(Pressao_Animacao); return; }
                
                let CalculoGraus = Pressao_Valor_Atual * 1.8;
                Pressao_Progress_Text.textContent = `${Pressao_Valor_Atual}%`;
                Pressao_Progress.style.setProperty("--progress", `${CalculoGraus}deg`);
            }, Velocidade);
        }
        const Status_Progress_Text = document.querySelector(".progress-value-status");
        function UpdateCurrentStatus(NovoStatus) { Status_Progress_Text.textContent = NovoStatus.toUpperCase(); }
        function AumentaTextoStatus() { Status_Progress_Text.style.setProperty("font-size", "larger"); }
        function DiminuiTextoStatus() { Status_Progress_Text.style.setProperty("font-size", "large"); }
        const Countdown_Progress_Text = document.querySelector(".progress-value-countdown");
        function UpdateCountDownValue(NovoStatus) { Countdown_Progress_Text.textContent = NovoStatus; }
        function AumentaTextoCountdown() { Countdown_Progress_Text.style.setProperty("font-size", "larger"); }
        function DiminuiTextoCountdown() { Countdown_Progress_Text.style.setProperty("font-size", "large"); }
        const Aviso_Texto = document.querySelector(".custom-mensagem");
        const Popup_Texto = document.querySelector(".popup-custom-message");
        let TimeOutAtual;
        function DisplayCustomMessage(Texto, Duracao) {
            Aviso_Texto.textContent = `AVISO: ${Texto}`;
            Aviso_Texto.style.display = "block";
            Popup_Texto.textContent = `AVISO: ${Texto}`;
            Popup_Texto.style.display = "block";
            if (TimeOutAtual) { clearTimeout(TimeOutAtual); }
            if (Duracao && Number(Duracao)) {
                TimeOutAtual = setTimeout(() => {
                    Aviso_Texto.style.display = "none";
                    Popup_Texto.style.display = "none";
                }, Number(Duracao));
            }
        }
        async function GetStatusFromESP() {
            try {
                const Resposta = await fetch("/status", { method: "GET" });
                const Status = await Resposta.json();
                return Status;
            } catch (error) {
                DisplayCustomMessage("Sem Conexão com o Servidor.");
                return null;
            }
        }
        async function UpdateDisplayTela() {
            let CurrentStatus = await GetStatusFromESP();
            if (!CurrentStatus) { return; }
            
            UpdatePressao(CurrentStatus.pressao);
            UpdateCurrentStatus(CurrentStatus.status);
            UpdateCountDownValue(CurrentStatus.countdown);
            if (CurrentStatus.message) {
                if (CurrentStatus.messageduration) {
                    DisplayCustomMessage(CurrentStatus.message, CurrentStatus.messageduration);
                } else {
                    DisplayCustomMessage(CurrentStatus.message);
                }
            }
            if (CurrentStatus.status.length >= 5) { DiminuiTextoStatus(); } else { AumentaTextoStatus(); }
            if (CurrentStatus.countdown >= 5) { DiminuiTextoCountdown(); } else { AumentaTextoCountdown(); }
        }
        setInterval(UpdateDisplayTela, 1000);
        async function IniciarLancamento() {
            try {
                const Reposta = await fetch("/start", {
                    method: "POST",
                    headers: { "Content-Type": "application/json" },
                    body: JSON.stringify({ comando: "lançamento", senha: SenhaSalva() })
                });
                if (!Reposta.ok) { return DisplayCustomMessage("ERRO DE AUTENTICAÇÃO"); }
                
                const json = await Reposta.json();
                if (json) { DisplayCustomMessage("AUTORIZADO LANÇAMENTO DO FOGUETE"); }
            } catch (error) { DisplayCustomMessage("Sem Conexão com o Servidor."); }
        }
        async function AbortarLancamento() {
            try {
                const Resposta = await fetch("/abort", {
                    method: "POST",
                    headers: { "Content-Type": "application/json" },
                    body: JSON.stringify({ comando: "abortar", senha: SenhaSalva() })
                });
                if (!Resposta.ok) { return DisplayCustomMessage("ERRO DE AUTENTICAÇÃO"); }
                
                const json = await Resposta.json();
                if (json) { DisplayCustomMessage("ABORTAGEM EM PROCESSO"); }
            } catch (error) { DisplayCustomMessage("Sem Conexão com o Servidor."); }
        }
        async function ResetSistema() {
            try {
                // Correção de Lógica: Faltava definir a Intenção da requisição como POST!
                const Resposta = await fetch("/reset", {
                    method: "POST",
                    headers: { "Content-Type": "application/json" },
                    body: JSON.stringify({ comando: "reset", senha: SenhaSalva() })
                });
                if (!Resposta.ok) { return DisplayCustomMessage("ERRO DE AUTENTICAÇÃO"); }
                
                const json = await Resposta.json();
                if (json) { DisplayCustomMessage("SISTEMA REINICIADO"); }
            } catch (error) { DisplayCustomMessage("Sem Conexão com o Servidor."); }
        }
        Botao_Lancamento.addEventListener("click", IniciarLancamento);
        Botao_Abortar.addEventListener("click", AbortarLancamento);
        Botao_Reset.addEventListener("click", ResetSistema);
    </script>
</body>
</html>
)rawliteral";



/*Root Principal*/
void HandleRoot() {
  server.send(200, "text/html", html_page);
}


std::pair<String, int> GetMessage() {
  String message = CustomMessageAtual;
  int Duration = CustomMessageDurationAtual;

  CustomMessageAtual = "";
  CustomMessageDurationAtual = 0;

  return {message, Duration};
}

void SetNewMessage(String message, int duration = 0) {
  CustomMessageAtual = message;
  if (duration > 0) {
    CustomMessageDurationAtual = duration;
  }
  else {
    CustomMessageDurationAtual = 0;
  }
}


/*
Configurações de Login
*/
bool ValidaSenha(String Senha) {
    if (Senha == SenhaAcesso) {
        return true;    
        }
    else {
        return false;
    }
}

/*ROTA: /login*/
void EfetuaLogin() {
    JsonDocument doc;
    JsonDocument returndoc;

    if (server.hasArg("plain")) {
      String ConteudoBruto = server.arg("plain");
      deserializeJson(doc, ConteudoBruto);

      if (doc["senha"] == SenhaAcesso) {
        returndoc["status"] = "autorizado";
        returndoc["senhavalidada"] = SenhaAcesso;

        String pacoteRetorno;
        serializeJson(returndoc, pacoteRetorno);
        server.send(200, "application/json", pacoteRetorno);
      }

      else {
        returndoc["status"] = "negado";

        String pacoteRetorno;
        serializeJson(returndoc, pacoteRetorno);
        server.send(200, "application/json", pacoteRetorno);
      }
    }
    else {
      returndoc["status"] = "formato invalido" ;
      String pacoteRetorno;
      serializeJson(returndoc, pacoteRetorno);
      server.send(401, "application/json", pacoteRetorno);
    }
}



/*
  ROTA: /status
*/

void EnviaStatus() {
    JsonDocument doc;
    JsonDocument returndoc;


      String ConteudoBruto = server.arg("plain");
      deserializeJson(doc, ConteudoBruto);

      /*
        Calculo de % da pressao
      */

      int pressure = analogRead(PRESSURE_SENSOR);
      // Mapeia o valor atual (que vai de min a max) para a escala de 0 a 100
      int Porcentagem = map(pressure, 0, PRESSURE_LIMIT,0,100);

      auto [message, duration] = GetMessage();

      returndoc["pressao"] = Porcentagem;
      returndoc["status"] = stateName(currentState);
      returndoc["countdown"] = countdown;
      returndoc["message"] = message;
      
      if (duration > 0) {
        returndoc["messageduration"] = duration;
      }

      String PacoteRetorno;
      serializeJson(returndoc, PacoteRetorno);
      server.send(200, "application/json", PacoteRetorno);

    

}

/*ROTA:
  /start  
*/

void RecebimentoStart() {
  JsonDocument doc;
  JsonDocument returndoc;

  if (server.hasArg("plain")) {
    String ConteudoBruto = server.arg("plain");
    deserializeJson(doc, ConteudoBruto);

    if (ValidaSenha(doc["senha"])) {
      
      returndoc["status"] = "autorizado";

      startCountdown();
      String PacoteRetorno;
      serializeJson(returndoc, PacoteRetorno);
      server.send(200, "application/json", PacoteRetorno);
    }

    else {

      returndoc["status"] = "negado";

      String PacoteRetorno;
      serializeJson(returndoc, PacoteRetorno);
      server.send(401, "application/json", PacoteRetorno);
    }
  }
  else {
      returndoc["status"] = "negado";
      String PacoteRetorno;
      serializeJson(returndoc, PacoteRetorno);
      server.send(401, "application/json", PacoteRetorno);
  }

}

/* ROTA:
  /abortar
*/

void RecebimentoAbort() {
  JsonDocument doc;
  JsonDocument returndoc;

  if (server.hasArg("plain")) {
    String ConteudoBruto = server.arg("plain");
    deserializeJson(doc, ConteudoBruto);

    if (ValidaSenha(doc["senha"])) {
      returndoc["status"] = "autorizado";
      emergencyStop();
      String PacoteRetorno;
      serializeJson(returndoc, PacoteRetorno);
      server.send(200, "application/json", PacoteRetorno);
    }

    else {
      returndoc["status"] = "negado";
      String PacoteRetorno;
      serializeJson(returndoc, PacoteRetorno);
      server.send(401, "application/json", PacoteRetorno);
    }

  }

  else {
      returndoc["status"] = "negado";
      String PacoteRetorno;
      serializeJson(returndoc, PacoteRetorno);
      server.send(401, "application/json", PacoteRetorno);
  }

}


/* ROTA:
 /reset
*/

void RecebimentoReset() {
    JsonDocument doc;
  JsonDocument returndoc;

   if (server.hasArg("plain")) {
    String ConteudoBruto = server.arg("plain");
    deserializeJson(doc, ConteudoBruto);

    if (ValidaSenha(doc["senha"])) {
      returndoc["status"] = "autorizado";
      resetSystem();
      String PacoteRetorno;
      serializeJson(returndoc, PacoteRetorno);
      server.send(200, "application/json", PacoteRetorno);
    }

    else {
      returndoc["status"] = "negado";
      String PacoteRetorno;
      serializeJson(returndoc, PacoteRetorno);
      server.send(401, "application/json", PacoteRetorno);
    }

  }

  else {
      returndoc["status"] = "negado";
      String PacoteRetorno;
      serializeJson(returndoc, PacoteRetorno);
      server.send(401, "application/json", PacoteRetorno);
  }



}


// ==========================================================
// SERVOS
// ==========================================================

void servosInitialPosition()
{
  servoReady.write(SERVO_INITIAL_POSITION);
  servoAbort.write(SERVO_INITIAL_POSITION);
}


// ----------------------------------------------------------
// INDICAÇÃO DE READY
// ----------------------------------------------------------

void servoReadyIndicator()
{
  // Movimento rápido apenas para indicação visual
  for (int position = SERVO_INITIAL_POSITION;
       position <= SERVO_INDICATOR_POSITION;
       position++)
  {
    servoReady.write(position);
    delay(5);
  }
}


// ----------------------------------------------------------
// INDICAÇÃO DE ABORT
// ----------------------------------------------------------

void servoAbortIndicator()
{
  // Movimento deliberadamente mais lento
  for (int position = SERVO_INITIAL_POSITION;
       position <= SERVO_INDICATOR_POSITION;
       position++)
  {
    servoAbort.write(position);
    delay(20);
  }
}


// ==========================================================
// LEDs
// ==========================================================

void ledsOff()
{
  digitalWrite(LED_1, LOW);
  digitalWrite(LED_2, LOW);
}


void led1On()
{
  digitalWrite(LED_1, HIGH);
  digitalWrite(LED_2, LOW);
}


void led2On()
{
  digitalWrite(LED_1, LOW);
  digitalWrite(LED_2, HIGH);
}


// ==========================================================
// NOME DO ESTADO
// ==========================================================

const char* stateName(byte state)
{
  if (state == STATE_SAFE)
    return "SAFE";

  if (state == STATE_COUNTDOWN)
    return "COUNTDOWN";

  if (state == STATE_READY)
    return "READY";

  if (state == STATE_ABORTED)
    return "ABORTED";

  return "UNKNOWN";
}


// ==========================================================
// MUDANÇA DE ESTADO
// ==========================================================

void changeState(byte newState)
{
  currentState = newState;

  // ========================================================
  // SAFE
  // ========================================================

  if (currentState == STATE_SAFE)
  {
    countdown = COUNTDOWN_INITIAL;
    countdownTimer = 0;

    ledsOff();

    servosInitialPosition();

    Serial.println();
    Serial.println(F("=============================="));
    Serial.println(F("SYSTEM STATE: SAFE"));
    Serial.println(F("SERVOS: INITIAL POSITION"));
    Serial.println(F("Waiting for START."));
    Serial.println(F("=============================="));
  }


  // ========================================================
  // COUNTDOWN
  // ========================================================

  else if (currentState == STATE_COUNTDOWN)
  {
    countdown = COUNTDOWN_INITIAL;
    countdownTimer = millis();

    servosInitialPosition();

    led1On();

    Serial.println();
    Serial.println(F("=============================="));
    Serial.println(F("COUNTDOWN STARTED"));
    Serial.println(F("COUNT: 10"));
    Serial.println(F("SERVOS: INITIAL POSITION"));
    Serial.println(F("=============================="));
  }


  // ========================================================
  // READY
  // ========================================================

  else if (currentState == STATE_READY)
  {
    ledsOff();

    Serial.println();
    Serial.println(F("=============================="));
    Serial.println(F("COUNT: 0"));
    Serial.println(F("COUNTDOWN COMPLETE"));
    Serial.println(F("SYSTEM STATE: READY"));
    Serial.println(F("SERVO 4: INDICATOR ACTIVE"));
    Serial.println(F("=============================="));

    servoReadyIndicator();

    Serial.println();
    Serial.println(F("Ao infinito e alem!"));
    Serial.println();
  }


  // ========================================================
  // ABORTED
  // ========================================================

  else if (currentState == STATE_ABORTED)
  {
    ledsOff();

    Serial.println();
    Serial.println(F("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"));
    Serial.println(F("       EMERGENCY ABORT"));
    Serial.println(F("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"));
    Serial.println(F("COUNTDOWN CANCELLED."));
    Serial.println(F("SERVO 3: ABORT INDICATOR ACTIVE"));
    Serial.println(F("SYSTEM LOCKED."));
    Serial.println(F("Use RESET."));
    Serial.println(F("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"));

    servoAbortIndicator();
  }
}


// ==========================================================
// BOTÃO START
// ==========================================================

bool startPressed()
{
  bool reading = digitalRead(BUTTON_START);

  if (reading != startRawState)
  {
    startRawState = reading;
    startChangeTime = millis();
  }

  if ((unsigned long)(millis() - startChangeTime) >= DEBOUNCE_TIME)
  {
    if (startStableState != startRawState)
    {
      startStableState = startRawState;

      if (startStableState == LOW)
      {
        return true;
      }
    }
  }

  return false;
}


// ==========================================================
// BOTÃO EMERGENCY
// ==========================================================

bool emergencyPressed()
{
  bool reading = digitalRead(BUTTON_EMERGENCY);

  if (reading != emergencyRawState)
  {
    emergencyRawState = reading;
    emergencyChangeTime = millis();
  }

  if ((unsigned long)(millis() - emergencyChangeTime) >= DEBOUNCE_TIME)
  {
    if (emergencyStableState != emergencyRawState)
    {
      emergencyStableState = emergencyRawState;

      if (emergencyStableState == LOW)
      {
        return true;
      }
    }
  }

  return false;
}


// ==========================================================
// EMERGENCY STOP
// ==========================================================

void emergencyStop()
{
  if (emergencyLatched)
  {
    return;
  }

  emergencyLatched = true;

  Serial.println();
  Serial.println(F("!!! EMERGENCY ACTIVATED !!!"));

  changeState(STATE_ABORTED);
}


// ==========================================================
// ABORT POR PRESSÃO
// ==========================================================

void pressureAbort()
{
  if (emergencyLatched)
  {
    return;
  }

  Serial.println();
  Serial.println(F("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"));
  Serial.println(F("     PRESSURE LIMIT EXCEEDED"));
  Serial.println(F("        SYSTEM ABORTED"));
  Serial.println(F("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"));

  emergencyLatched = true;

  changeState(STATE_ABORTED);
}


// ==========================================================
// START
// ==========================================================

void startCountdown()
{
  if (emergencyLatched)
  {
    Serial.println(F("START DENIED: EMERGENCY LATCHED."));
    SetNewMessage("Não é possível iniciar em estado de emergência.");
    return;
  }

  if (currentState != STATE_SAFE)
  {
    Serial.print(F("START DENIED. STATE: "));
    Serial.println(stateName(currentState));
    SetNewMessage("Não é possível iniciar no estado no status atual.");
    return;
  }

  Serial.println(F("START ACCEPTED."));

  changeState(STATE_COUNTDOWN);
}


// ==========================================================
// RESET
// ==========================================================

void resetSystem()
{
  // Não permite RESET enquanto o botão de emergência estiver
  // pressionado.

  if (digitalRead(BUTTON_EMERGENCY) == LOW)
  {
    Serial.println(F("RESET DENIED."));
    Serial.println(F("Release EMERGENCY first."));
    return;
  }


  // Não permite RESET se a pressão continuar acima do limite.

  int pressure = analogRead(PRESSURE_SENSOR);

  if (pressure >= PRESSURE_LIMIT)
  {
    Serial.println(F("RESET DENIED."));
    Serial.println(F("PRESSURE STILL ABOVE LIMIT."));
     SetNewMessage("ERRO: Pressão acima do limite aceito.");
    return;
  }


  Serial.println();
  Serial.println(F("SYSTEM RESET."));
  SetNewMessage("REINICIANDO SISTEMA");

  emergencyLatched = false;


  // Reinicializa botão START

  startRawState = digitalRead(BUTTON_START);
  startStableState = startRawState;
  startChangeTime = millis();


  // Reinicializa botão EMERGENCY

  emergencyRawState = digitalRead(BUTTON_EMERGENCY);
  emergencyStableState = emergencyRawState;
  emergencyChangeTime = millis();


  // Retorna servos à posição inicial

  servosInitialPosition();


  // Retorna ao estado SAFE

  changeState(STATE_SAFE);
}


// ==========================================================
// MONITORAMENTO DA PRESSÃO
// ==========================================================

void checkPressure()
{
  if (currentState != STATE_COUNTDOWN)
  {
    return;
  }

  int pressureValue = analogRead(PRESSURE_SENSOR);

  if (pressureValue >= PRESSURE_LIMIT)
  {
    pressureAbort();
  }
}


// ==========================================================
// COUNTDOWN
// ==========================================================

void updateCountdown()
{
  if (currentState != STATE_COUNTDOWN)
  {
    return;
  }

  unsigned long now = millis();

  if ((unsigned long)(now - countdownTimer) >= ONE_SECOND)
  {
    countdownTimer += ONE_SECOND;


    if (countdown > 0)
    {
      countdown--;
    }


    Serial.print(F("COUNT: "));
    Serial.println(countdown);


    // Alternar LEDs

    if (countdown > 0)
    {
      if ((countdown % 2) == 0)
      {
        led1On();
      }
      else
      {
        led2On();
      }
    }


    // Chegou a zero

    if (countdown == 0)
    {
      ledsOff();

      changeState(STATE_READY);
    }
  }
}


// ==========================================================
// STATUS
// ==========================================================

void showStatus()
{
  int pressure = analogRead(PRESSURE_SENSOR);

  Serial.println();
  Serial.println(F("========== STATUS =========="));


  Serial.print(F("STATE: "));
  Serial.println(stateName(currentState));


  Serial.print(F("COUNTDOWN: "));
  Serial.println(countdown);


  Serial.print(F("PRESSURE SENSOR: "));
  Serial.println(pressure);


  Serial.print(F("PRESSURE LIMIT: "));
  Serial.println(PRESSURE_LIMIT);


  Serial.print(F("EMERGENCY LATCHED: "));

  if (emergencyLatched)
    Serial.println(F("YES"));
  else
    Serial.println(F("NO"));


  Serial.print(F("SERVO READY PIN: "));
  Serial.println(SERVO_READY_PIN);


  Serial.print(F("SERVO ABORT PIN: "));
  Serial.println(SERVO_ABORT_PIN);


  Serial.println(F("============================"));
}


// ==========================================================
// SERIAL
// ==========================================================

const byte SERIAL_BUFFER_SIZE = 32;

char serialBuffer[SERIAL_BUFFER_SIZE];
byte serialPosition = 0;


// ==========================================================
// PROCESSAR COMANDO
// ==========================================================

void processCommand(char* command)
{
  if (strcmp(command, "START") == 0)
  {
    startCountdown();
  }


  else if (strcmp(command, "RESET") == 0)
  {
    resetSystem();
  }


  else if (strcmp(command, "EMERGENCY") == 0)
  {
    emergencyStop();
  }


  else if (strcmp(command, "ABORT") == 0)
  {
    emergencyStop();
  }


  else if (strcmp(command, "STATUS") == 0)
  {
    showStatus();
  }


  else if (strcmp(command, "PING") == 0)
  {
    Serial.println(F("PONG"));
  }


  else if (strcmp(command, "HELP") == 0)
  {
    Serial.println();
    Serial.println(F("========== COMMANDS =========="));
    Serial.println(F("START"));
    Serial.println(F("RESET"));
    Serial.println(F("EMERGENCY"));
    Serial.println(F("ABORT"));
    Serial.println(F("STATUS"));
    Serial.println(F("PING"));
    Serial.println(F("HELP"));
    Serial.println(F("=============================="));
  }


  else if (command[0] != '\0')
  {
    Serial.print(F("UNKNOWN COMMAND: "));
    Serial.println(command);
  }
}


// ==========================================================
// LEITURA SERIAL
// ==========================================================

void readSerial()
{
  while (Serial.available() > 0)
  {
    char c = Serial.read();


    if (c == '\n' || c == '\r')
    {
      if (serialPosition > 0)
      {
        serialBuffer[serialPosition] = '\0';


        // Converte minúsculas para maiúsculas

        for (byte i = 0; i < serialPosition; i++)
        {
          if (serialBuffer[i] >= 'a' &&
              serialBuffer[i] <= 'z')
          {
            serialBuffer[i] =
              serialBuffer[i] - 'a' + 'A';
          }
        }


        processCommand(serialBuffer);

        serialPosition = 0;
      }
    }


    else
    {
      if (serialPosition < SERIAL_BUFFER_SIZE - 1)
      {
        serialBuffer[serialPosition] = c;
        serialPosition++;
      }


      else
      {
        serialPosition = 0;

        Serial.println(F("ERROR: COMMAND TOO LONG."));
      }
    }
  }
}


// ==========================================================
// SETUP
// ==========================================================



void setup()
{
  Serial.begin(115200);


  /*SERVIDOR*/
  WiFi.softAP(ssid, password);
  server.on("/", HTTP_GET, HandleRoot);
  server.on("/login", HTTP_POST, EfetuaLogin);
  server.on("/status", HTTP_GET, EnviaStatus);
  server.on("/start", HTTP_POST, RecebimentoStart);
  server.on("/abort", HTTP_POST, RecebimentoAbort);
  server.on("/reset", HTTP_POST, RecebimentoReset);

 
  server.begin();
  Serial.println(WiFi.softAPIP());

  // ========================================================
  // LEDs
  // ========================================================

  pinMode(LED_1, OUTPUT);
  pinMode(LED_2, OUTPUT);


  // ========================================================
  // BOTÕES
  // ========================================================

  pinMode(BUTTON_START, INPUT_PULLUP);
  pinMode(BUTTON_EMERGENCY, INPUT_PULLUP);


  // ========================================================
  // SENSOR
  // ========================================================

  pinMode(PRESSURE_SENSOR, INPUT);


  // ========================================================
  // SERVOS
  // ========================================================

  servoReady.attach(SERVO_READY_PIN);
  servoAbort.attach(SERVO_ABORT_PIN);

  servosInitialPosition();


  // ========================================================
  // LEDs
  // ========================================================

  ledsOff();


  // ========================================================
  // BOTÃO START
  // ========================================================

  startRawState = digitalRead(BUTTON_START);
  startStableState = startRawState;
  startChangeTime = millis();


  // ========================================================
  // BOTÃO EMERGENCY
  // ========================================================

  emergencyRawState = digitalRead(BUTTON_EMERGENCY);
  emergencyStableState = emergencyRawState;
  emergencyChangeTime = millis();


  // ========================================================
  // ESTADO INICIAL
  // ========================================================

  emergencyLatched = false;


  // ========================================================
  // SERIAL
  // ========================================================

  Serial.println();
  Serial.println(F("================================"));
  Serial.println(F("     LAUNCH SAFETY CONTROLLER"));
  Serial.println(F("            ARDUINO UNO"));
  Serial.println(F("              V8.0"));
  Serial.println(F("================================"));



  changeState(STATE_SAFE);
}


// ==========================================================
// LOOP PRINCIPAL
// ==========================================================

void loop()
{
  // ========================================================
  // EMERGENCY
  // ========================================================

  server.handleClient();

  if (emergencyPressed())
  {
    emergencyStop();
  }


  // Se houve emergência, permanece bloqueado,
  // mas continua aceitando comandos Serial.

  if (currentState == STATE_ABORTED)
  {
    readSerial();
    return;
  }


  // ========================================================
  // START
  // ========================================================

  if (startPressed())
  {
    startCountdown();
  }


  // ========================================================
  // SERIAL
  // ========================================================

  readSerial();


  // ========================================================
  // PRESSÃO
  // ========================================================

  checkPressure();


  // Se a pressão causou abort,
  // não continua o countdown.

  if (currentState == STATE_ABORTED)
  {
    return;
  }


  // ========================================================
  // COUNTDOWN
  // ========================================================

  updateCountdown();
}