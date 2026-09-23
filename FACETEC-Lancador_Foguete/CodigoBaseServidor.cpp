#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>

WebServer server(80);

//Site necessário em uma váriavel
//SERÁ ADAPTADO COM O LAYOUT FUTURAMENTE


//IDEIA:
/*UTILIZAR VARIAVEÍS GLOBAIS COMO BASE DOS COMANDOS Servidor.send(), sendo essas variáveis modificadas constantemente durante o código */

const char* html_page = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <title>Teste de Lógica ESP32</title>
  <style>
    body { font-family: Arial, sans-serif; text-align: center; margin-top: 50px; background-color: #f4f4f4;}
    input[type=text] { padding: 10px; width: 250px; border-radius: 5px; border: 1px solid #ccc;}
    input[type=submit] { padding: 10px 20px; cursor: pointer; background-color: #28a745; color: white; border: none; border-radius: 5px;}
  </style>
</head>
<body>
  <h2>Envie um texto para o Serial do ESP32</h2>
  <!-- O formulário envia os dados via método POST para o caminho "/enviar" -->
  <form action="/enviar" method="POST">
    <input type="text" name="mensagem" placeholder="Digite algo aqui..." required>
    <br><br>
    <input type="submit" value="Enviar para o ESP32">
  </form>
</body>
</html>
)rawliteral";

//Validação baseado em busca no pacote por conta do Json.stringfy()
bool validaSenha(String pacote) {

}

void handleRoot() {
  server.send(200, "text/html", html_page);
}

void setup() {

}

void loop() {
  server.handleClient();
}
