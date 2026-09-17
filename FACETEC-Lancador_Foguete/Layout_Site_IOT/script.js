//SCRIPT PARA TESTES


const BarraProgresso = document.querySelector(".progress-circle-umidade");
const TextoProgresso = document.querySelector(".progress-value-umidade"); 

const Botao_Regador = document.querySelector(".botao-regador");


let contadorAtual = 0;
let valorFinal = 70;
let velocidade = 20;


function consoleOutput() {
    console.log("Botão do regador Pressionado!")
}

Botao_Regador.addEventListener("click", consoleOutput);

//Função de teste

// let loopAnimacao = setInterval(() => {
//     contadorAtual++;

//     TextoProgresso.textContent = `${contadorAtual}%`;

//     let Graus = contadorAtual * 1.8;

//     BarraProgresso.style.setProperty("--progress", `${Graus}deg`)

//     if (contadorAtual === valorFinal){
//         clearInterval(loopAnimacao)
//     }


// }, velocidade)