/*
SCRIPT PRINCIPAL DO PROJETO
ESCOPO DO PROJETO:
TODO:

-Medidor de umidade baseado em barra de progresso animada. (% do progresso x 1.8)
-pop up ao entrar na página pedindo uma senha hardcoded no ESP que será dada aos apresentadores, a senha é enviada para o ESP, se for validada, o pop up fecha
e a senha é salva no navegador (possíveis cookies), assim liberando os controles para o apresentador
-Pegar os elementos a serem modificados (Umidade, Regador (on/off), Estufa (On,off) e temperatura) com FETCH, e dar display no HTML.
-Quando a senha registrada é correta os botôes são liberados, e juntamente aos comandos a senha é enviada para validação no ESP

*/




//ELEMENTOS DO LOGIN
const Popup_Senha = document.querySelector(".popup-senha");

const Senha_Digitada = document.querySelector(".input-senha");

const Botao_Enviar = document.querySelector(".botao-enviar-senha");

const Botao_Sou_Visitante = document.querySelector(".botao-sou-visitante");

const Display_Senha_Invalida = document.querySelector(".display-senha-incorreta");

const Botao_Lancamento = document.querySelector(".botao-lancamento");

const Botao_Abortar = document.querySelector(".botao-abortar");


//VARIAVEL PARA REPOSICIONAR A TEMPERATURA
const Temperatura_Row = document.getElementById("para-centro");


//CONFIGURAÇÕES DE LOGIN

function Autorizado() {
    Popup_Senha.style.display = "none";
    Botao_Lancamento.style.display = "inline-block";
    Botao_Abortar.style.display = "inline-block";
}

function SouVisitante() {
    Temperatura_Row.classList.replace("col-4", "col-12");
    Popup_Senha.style.display = "none";
}

function MostrarSenhaInvalida(){
    Display_Senha_Invalida.style.display = "flex";
}



function ValidarAcesso() {
    if (!sessionStorage.getItem("SenhaAutorizada")) {
        Popup_Senha.style.display = "flex";
    }
}

ValidarAcesso();

function ValidarSenha(TentativaSenha) {

    let sucesso = false

    //MANDAR A SENHA PARA O ESP, E ESPERAR PELA RESPOSTA
    fetch("/login", {
        method: "POST",
        headers: {
            "Content-Type": "application/json"
        },
        body: JSON.stringify({ senha: TentativaSenha })
    }).then((Resposta) => Resposta.json())
        .then((json) => {
            if (json.status === "autorizado") {
                sessionStorage.setItem("SenhaAutorizada", json.senhavalidada)
                Autorizado();
                return true
            }

            else {
                MostrarSenhaInvalida();
            }
        });
}

function SenhaSalva(){
    return sessionStorage.getItem("SenhaAutorizada");
}


Botao_Enviar.addEventListener("click", () => {
    const Tentativa_Senha = Senha_Digitada.value;

    //--PLACEHOLDER
    Autorizado();
    // ValidarSenha(Tentativa_Senha);
});

Botao_Sou_Visitante.addEventListener("click", () => {
    SouVisitante();
})


//Definições da Umidade
const Umidade_Progress = document.querySelector(".progress-circle-umidade");

const Umidade_Progress_Text = document.querySelector(".progress-value-umidade");

let Umidade_Valor_Atual = 0;

let Umidade_Animacao;

function UpdateUmidade(NewValue) {

    let Velocidade = 20;

    clearInterval(Umidade_Animacao);
    Umidade_Animacao = setInterval(() => {

        if (Umidade_Valor_Atual < NewValue) {

            Umidade_Valor_Atual++;

        } else if (Umidade_Valor_Atual > NewValue) {

            Umidade_Valor_Atual--;

        } else {

            clearInterval(Umidade_Animacao);

            return;

        }
        let CalculoGraus = Umidade_Valor_Atual * 1.8;
        Umidade_Progress_Text.textContent = `${Umidade_Valor_Atual}%`;

        Umidade_Progress.style.setProperty("--progress", `${CalculoGraus}deg`);

    }, velocidade);

}

 //Definições da Temperatura
 const Temperatura_Progress = document.querySelector(".progress-circle-temperatura");
 const Temperatura_Progress_Text = document.querySelector(".progress-value-temperatura");

 let Temperatura_Atual = 0;

 /**
 * 
 * @param {string} NovoValor
 * 
 */
 function UpdateTemperatura(NovoValor ) {
    Temperatura_Atual = NovoValor.toUpperCase()
    Temperatura_Progress_Text.textContent = `${Temperatura_Atual}°C`
 }


 /*
    TODO:
    -Se der tempo, colocar uma animaçãozinha de mudança de cor Background para vermelho quando temperatura maior igual a 30
 */ 


//Definições da Estufa
const Estufa_Progress = document.querySelector("progress-circle-estufa");
const Estufa_Progress_Text = document.querySelector("progress-value-estufa");

let Estufa_Estado_Atual;

/**
 * 
 * @param {string} NovoStatus
 * 
 */

function UpdateEstufaStatus(NovoStatus) {
    Estufa_Estado_Atual = NovoStatus.toUpperCase();
    Estufa_Progress_Text.textContent = Estufa_Estado_Atual
}

/**
 * Colocar background Verde para estufa aberta e vermelho para estufa fechada
 */



//Definições do Regador
const Regador_Progress = document.querySelector("progress-circle-regador");
const Regador_Progress_Text = document.querySelector("progress-value-regador");

let Regador_Estado_Atual;

function UpdateRegadorStatus(NovoStatus) {
    Regador_Estado_Atual = NovoStatus.toUpperCase();
    Regador_Progress_Text.textContent = Regador_Estado_Atual;
}


/**
 * Colocar background Verde para regador ligado e vermelho para regador fechado
 */


/**
 * 
 * @param {string} botao
 * @param {boolean} valor
 */
//Definições dos botoes
function DisplayBotaoAcao(botao, ligado){
    if (botao.toLowerCase() === "estufa") {
        if (ligado === true) { //Se ligado então está aberto
            Botao_Abortar.textContent = "FECHAR ESTUFA"
        }
        else {
            Botao_Abortar.textContent = "ABRIR ESTUFA"
        }
    }

    if (botao.toLowerCase() === "regador") {
        if (ligado === true) { //Se ligado então está ligado
            Botao_Lancamento.textContent === "DESATIVAR REGADOR"
        }
        else {
            Botao_Lancamento.textContent === "ATIVAR REGADOR"
        }
    }
}




//Definições Status

function GetStatus(){
    //GET DE TODOS OS STATUS
    let Status;
    fetch("/status", 
        {method: "GET"
        }
    ).then((Resposta) => Resposta.json())
    .then((Json) => {
        Status = Json
    })

    return Status;
}

let CurrentStatus;
function UpdateDisplayTela(){
    CurrentStatus = GetStatus();
    UpdateUmidade(CurrentStatus.umidade)
    UpdateTemperatura(CurrentStatus.temperatura)
    UpdateRegadorStatus(CurrentStatus.regador)
    UpdateEstufaStatus(CurrentStatus.estufa)

    if (CurrentStatus.regador === "ligado") {
        DisplayBotaoAcao("regador", true)
    }
    else {
        DisplayBotaoAcao("regador", false)
    }

    if (CurrentStatus.estufa === "aberto"){
        DisplayBotaoAcao("estufa", true)
    }
    else {
        DisplayBotaoAcao("estufa", false)
    }
}