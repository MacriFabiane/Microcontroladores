#include <msp430.h> 

void ini_uCon(void);
void ini_P1_P2(void);
void ini_t_deb(void);

/**
 * main.c
 */
void main(void)
{
    ini_uCon();
    ini_P1_P2();
    ini_t_deb();

    do{

    }while(1);
}


void ini_uCon(void){
    WDTCTL = WDTPW | WDTHOLD; // Para o contador do watchdog timer
    // Configuracoes do BCS
    // MCLK = DCOCLK ~ 16 MHz
    // ACLK = LFXT1CLK = 32768 Hz
    // SMCLK = DCOCLK / 8 ~ 2 MHz

    DCOCTL = CALDCO_16MHZ;
    BCSCTL1 = CALBC1_16MHZ;
    // Freq. Calibrada de 16 MHz
    BCSCTL2 = DIVS0 + DIVS1;  // Fator divisao = 8 para SMCLK
    BCSCTL3 = XCAP0 + XCAP1;  // Capacitor do cristal ~12.5 pF

    while(BCSCTL3 & LFXT1OF); // LFXT1 vai ser na mesma frequencia do cristal

    __enable_interrupt();  // seta o bit GIE - permite geracao de interrupcoes
}

void ini_P1_P2(void){
    /*
     *  PORTA 1
     *    - P1.4 - CH_A - entrada digital pull up
     *    - P1.5 - CH_B - entrada digital pull up
     *    - P1.6 - LED VERDE - sa�da digital
     *    - P1.0 - LED VERMELHO - sa�da digital
     *
     */

    P1DIR = ~(BIT4 + BIT5);
    P1REN = BIT4 + BIT5;
    P1OUT = BIT4 + BIT5;
    P1IES = BIT4;
    P1IFG = 0;
    P1IE =  BIT4;

    /*
     *   PORTA 2
     *
     *   - Todas as portas em sa�da em n�vel baixo
     */

    P2DIR = 0xFF;
    P2OUT = 0;
}

void ini_t_deb(void){
    /*  t_deb = 2ms
     *
     * CONTADOR
     *    - Clock = SMCLK ~ 2MHz
     *       - Fdiv.:
     *    - Modo Contagem = PARADO -> UP
     *    - INT. cont.: Desabilitada
     *
     * MODULO 0
     *    - Funcao nativa: comparacao
     *    - TA0CCR0 = (2M * 0,002) - 1 = 3999;
     *    - INT.: Habilitada
     *
     */

    TA0CTL = TASSEL1;
    TA0CCTL0 = CCIE;
    TA0CCR0 = 3999;
}



#pragma vector=PORT1_VECTOR
__interrupt void RTI_Porta1(void){

    P1IFG &= ~BIT4;       // Desabilitar flag de int. (opcional)

    P1IE &= ~BIT4;   // Desabilitar interrupcao

    TA0CTL |= MC0;   // Contagem UP
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void RTI_Timer0(void){

    TA0CTL &= ~MC0;     // Para a contagem

    if((~P1IN) & BIT4){
        if((~P1IN) & BIT5){
            P1OUT ^= BIT0; // Acende LED Vermelho

        }
        else{
            P1OUT ^= BIT6; // Acende LED Verde

        }
    }


    P1IFG &= ~BIT4;  // limpa a flag de int. (obrigatorio)

    P1IE |= BIT4;  // Habilita interrupcao
}



/*if((~P1IN) & BIT4){
        if(~P1IN & BIT5){

           // if((P1IN) & BIT0){
                P1OUT &= ~BIT6; // Apaga LED Verde
            //}
                P1OUT |= BIT0; // Acende LED Vermelho
        }
        else{
           // if(P1IN & BIT6){
                P1OUT &= ~BIT0; // Apaga LED Vermelho
           // }
            P1OUT |= BIT6; // Acende LED Verde
        }
    }





    if(~P1IN & BIT4){
       if(P1IN & BIT5){
           P1OUT ^= BIT0; // Alterar Led Vermelho
       }
       else if(~P1IN & BIT5) {
           P1OUT ^= BIT6; // Alterar Led Verde
       }
      }

    */
