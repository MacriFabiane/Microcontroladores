#include <msp430.h>

void ini_P1_P2(void);
volatile unsigned int estado;  // Variável que controla o semáforo
int interrup = 0; //variavel q controla se houve ou n uma interrupção pra levar o semáfaro pro estado certo

/**
 * main.c
 */
int main(void)
{
    ini_P1_P2(); // Inicializa as portas
    estado = 0;

    __enable_interrupt();

    while(1) {

        if (estado == 0) { // verde 1 ligado e vermelho 2 ligado
            P1OUT |= BIT4;  // Liga verde 1
            P2OUT |= BIT0;  // Liga vermelho 2
            __delay_cycles(8000000); // Delay para simular tempo do verde

            if(interrup == 0){//se n teve interrup. segue o fluxo
                estado = 1;      // Vai para o próximo estado

            }
            else{
                P2OUT &= ~BIT0;//apaga vermelho 2
                estado = 2; // Avança para o estado vermelho 1 e verde 2
                interrup = 0; // Reseta a interrupção
            }
        }
        else if (estado == 1) { // amarelo 1 e vermelho 2 ligados
            P1OUT &= ~BIT4;  // Desliga verde 1
            P1OUT |= BIT5;   // Liga amarelo 1
            __delay_cycles(2000000); // Tempo do amarelo

            P1OUT &= ~BIT5;  // Desliga amarelo 1
            P2OUT &= ~BIT0;  // Desliga vermelho 2
            estado = 2;      // Passa para o próximo estado
        }
        else if (estado == 2) { // verde 2 e vermelho 1 ligados
            P2OUT |= BIT2;   // Liga verde 2
            P1OUT |= BIT6;   // Liga vermelho 1
            __delay_cycles(8000000); // Tempo do verde 2

            if(interrup == 0){ //se n teve interrup. segue o fluxo
                estado = 3;      // Vai para o próximo estado
            }
            else{
                P1OUT &= ~BIT6; //desliga vermelho 1
                estado = 0; // Avança para o estado vermelho 2 e verde 1
                interrup = 0; // Reseta a interrupção
            }
        }
        else if(estado == 3){
            P2OUT &= ~BIT2; //desliga verde2
            P2OUT |= BIT1; //liga amarelo 2
            __delay_cycles(2000000);

            P2OUT &= ~BIT1; //desliga amarelo 2
            P1OUT &= ~BIT6; //desliga vermelho 1

            estado = 0; //retorna para o inicio
        }
    }
}

#pragma vector=PORT1_VECTOR
__interrupt void pedestre(void) {

    if ((estado == 0) && (P1OUT & BIT4)) {  // Semáforo 1 está no verde
        P1OUT &= ~BIT4;  // Desliga o verde 1
        P1OUT |= BIT5;   // Liga o amarelo 1
        __delay_cycles(2000000); // Tempo do amarelo

        P1OUT &= ~BIT5;  // Desliga o amarelo 1
        P1OUT |= BIT6;   // Liga o vermelho 1
        P2OUT |= BIT0;   // Liga o vermelho 2
        __delay_cycles(10000000); // Tempo para o pedestre atravessar

        interrup = 1;//pra sinalizar q ocorreu uma interrupção

    }
    else if ((estado == 2) && (P2OUT & BIT2)) { // Se estiver no verde 2
        P2OUT &= ~BIT2;  // Desliga o verde 2
        P2OUT |= BIT1;   // Liga o amarelo 2
        __delay_cycles(2000000); // Tempo do amarelo

        P2OUT &= ~BIT1;  // Desliga amarelo 2
        P2OUT |= BIT0;   // Liga o vermelho 2
        P1OUT |= BIT6;   // Deixa o vermelho 1 ligado
        __delay_cycles(10000000); // Tempo para o pedestre atravessar

        interrup = 1;//pra sinalizar q ocorreu uma interrupção

    }

    P1IFG &= ~(BIT3);  // Limpa a flag de interrupção
}

void ini_P1_P2(void) {
    WDTCTL = WDTPW | WDTHOLD;   // Para o watchdog timer

    // Porta 1
    P1DIR = 0xF7; // Configura todos os pinos 1 como saída, exceto P1.3 (entrada)
    P1OUT = 0x08; // Habilita saída baixa nos pinos P1.4, P1.5, P1.6 e o pull-up de P1.3
    P1REN = BIT3; // Habilita resistor de pull-up no P1.3
    P1IE = BIT3;  // Habilita interrupção no P1.3
    P1IES |= BIT3; // Configura para borda de descida
    P1IFG = 0;    // Inicializa a flag como 0

    // Porta 2
    P2DIR = 0xFF; // Configura todos os pinos como saída
    P2OUT = 0x00; // Inicializa as saídas em nível baixo para P2.0, P2.1 e P2.2
}
