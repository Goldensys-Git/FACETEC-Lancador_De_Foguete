/*
SCRIPT PRINCIPAL DO PROJETO
ESCOPO DO PROJETO:
TODO:

-Medidor de pressão baseado em barra de progresso animada. (% do progresso x 1.8)
-pop up ao entrar na página pedindo uma senha hardcoded no ESP que será dada aos apresentadores, a senha é enviada para o ESP, se for validada, o pop up fecha
e a senha é salva no navegador (possíveis cookies), assim liberando os controles para o apresentador
-Pegar os elementos a serem modificados (Pressão, STATUS, ) com FETCH, e dar display no HTML.
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




//CONFIGURAÇÕES DE LOGIN

function Autorizado() {
    Popup_Senha.style.display = "none";
    Botao_Lancamento.style.display = "inline-block";
    Botao_Abortar.style.display = "inline-block";
}

function SouVisitante() {
    Popup_Senha.style.display = "none";
}

function MostrarSenhaInvalida() {
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
        }).catch((error) => {
            DisplayCustomMessage("Sem Conexão com o Servidor.")
        });
}

function SenhaSalva() {
    return sessionStorage.getItem("SenhaAutorizada");
}


Botao_Enviar.addEventListener("click", () => {
    const Tentativa_Senha = Senha_Digitada.value;
    //--PLACEHOLDER
    ValidarSenha(Tentativa_Senha);
});

Botao_Sou_Visitante.addEventListener("click", () => {
    SouVisitante();
})


//Definições da pressao
const Pressao_Progress = document.querySelector(".progress-circle-pressao");

const Pressao_Progress_Text = document.querySelector(".progress-value-pressao");

let Pressao_Valor_Atual = 0;

let Pressao_Animacao;

function UpdatePressao(NewValue) {

    let Velocidade = 20;

    clearInterval(Pressao_Animacao);
    Pressao_Animacao = setInterval(() => {

        if (Pressao_Valor_Atual < NewValue) {

            Pressao_Valor_Atual++;

        } else if (Pressao_Valor_Atual > NewValue) {

            Pressao_Valor_Atual--;

        } else {

            clearInterval(Pressao_Animacao);

            return;

        }
        let CalculoGraus = Pressao_Valor_Atual * 1.8;
        Pressao_Progress_Text.textContent = `${Pressao_Valor_Atual}%`;

        Pressao_Progress.style.setProperty("--progress", `${CalculoGraus}deg`);

    }, Velocidade);

}


//Definições de Status

const Status_Progress = document.querySelector(".progress-circle-status")
const Status_Progress_Text = document.querySelector(".progress-value-status")

let Status_Estado_Atual;

/**
 * 
 * @param {string} NovoStatus
 * 
 */

function UpdateCurrentStatus(NovoStatus) {
    Status_Estado_Atual = NovoStatus.toUpperCase();
    Status_Progress_Text.textContent = Status_Estado_Atual
}

function AumentaTextoStatus() {
    Status_Progress_Text.style.setProperty("font-size", "larger")
}

function DiminuiTextoStatus() {
    Status_Progress_Text.style.setProperty("font-size", "large")
}







//Definições De CountDown
const Countdown_Progress = document.querySelector(".progress-circle-countdown")
const Countdown_Progress_Text = document.querySelector(".progress-value-countdown")

/**
 * @param {string} NovoStatus
 */
function UpdateCountDownValue(NovoStatus) {
    Countdown_Progress_Text.textContent = NovoStatus
}

function AumentaTextoCountdown() {
    Countdown_Progress_Text.style.setProperty("font-size", "larger");
}

function DiminuiTextoCountdown() {
    Countdown_Progress_Text.style.setProperty("font-size", "large")
}




//Definições de avisos customizados
const Aviso_Texto = document.querySelector(".custom-mensagem")
const Popup_Texto = document.querySelector(".popup-custom-message")
let TimeOutAtual;
/**
 * 
 * @param {string} Texto 
 * @param {number} Duracao 
 */
function DisplayCustomMessage(Texto, Duracao) {
    Aviso_Texto.textContent = `AVISO: ${Texto}`
    Aviso_Texto.style.setProperty("display", "block");
    Popup_Texto.textContent = `AVISO: ${Texto}`
    Popup_Texto.style.setProperty("display", "block")

    if (TimeOutAtual) {
        clearTimeout(TimeOutAtual);
    }

    if (Duracao && Number(Duracao)) {
        TimeOutAtual = setTimeout(() => {
            Aviso_Texto.style.setProperty("display", "none")
            Popup_Texto.style.setProperty("display", "none")

        }, Number(Duracao))
    }

}


/**
 * 
 * @param {string} botao
 * @param {boolean} valor
 */


//FETCH STATUS DO ESP
async function GetStatusFromESP() {
    //GET DE TODOS OS STATUS
    try {

        const Resposta = await fetch("/status",
            {
                method: "GET"
            }
        )

        const Status = await Resposta.json()
        return Status

    } catch (error) {
        DisplayCustomMessage("Sem Conexão com o Servidor.")
        return;
    }


    // let Status;
    // fetch("/status", 
    //     {method: "GET"
    //     }
    // ).then((Resposta) => Resposta.json())
    // .then((Json) => {
    //     Status = Json
    // })

    // return Status;
}

let CurrentStatus;
async function UpdateDisplayTela() {
    CurrentStatus = await GetStatusFromESP();
    if (!CurrentStatus) {
        return
    }
    UpdatePressao(CurrentStatus.pressao)
    UpdateCurrentStatus(CurrentStatus.status)
    UpdateCountDownValue(CurrentStatus.countdown)

    if (CurrentStatus.message) {
        if (CurrentStatus.messageduration) {
            DisplayCustomMessage(CurrentStatus.message, CurrentStatus.messageduration);
        }
        else {
            DisplayCustomMessage(CurrentStatus.message);
        }
    }

    if (CurrentStatus.status.length >= 5) {
        DiminuiTextoStatus()
    }
    else {
        AumentaTextoStatus()
    }

    if (CurrentStatus.countdown >= 5) {
        DiminuiTextoCountdown()
    }
    else {
        AumentaTextoCountdown()
    }
}

setInterval(UpdateDisplayTela, 1000);

//Definições para iniciar lançamento

//MANDAR POST NA ROTA START, E COM A SENHA DENTRO DO BODY
/**        headers: {
            "Content-Type": "application/json"
        }, */

//Pretendo não bloquear a requisição no front-end, mas fazer a validação apenas no servidor

async function IniciarLancamento() {
    try {
        const Reposta = await fetch("/start", {
            method: "POST",
            headers: {
                "Content-Type": "application/json"
            },
            body: JSON.stringify({
                comando: "lançamento",
                senha: SenhaSalva()
            })
        })

        if (!Reposta.ok) {
            DisplayCustomMessage("ERRO DE AUTENTICAÇÃO");
            return
        }

        const json = await Reposta.json()

        if (json) {
            DisplayCustomMessage("AUTORIZADO LANÇAMENTO DO FOGUETE")
        }

    } catch (error) {
        DisplayCustomMessage("Sem Conexão com o Servidor.")
    }
}

async function AbortarLancamento() {

    try {
        const Resposta = await fetch("/abort", {
            method: "POST",
            headers: {
                "Content-Type": "application/json"
            },
            body: JSON.stringify({
                comando: "abortar",
                senha: SenhaSalva()
            })
        })


        if (!Resposta.ok) {
            DisplayCustomMessage("ERRO DE AUTENTICAÇÃO");
            return
        }

        const json = await Resposta.json()

        if (json) {
            DisplayCustomMessage("ABORTAGEM EM PROCESSO")
        }
    } catch (error) {
        DisplayCustomMessage("Sem Conexão com o Servidor.")
    }
}

Botao_Lancamento.addEventListener("click", IniciarLancamento);
Botao_Abortar.addEventListener("click", AbortarLancamento);

