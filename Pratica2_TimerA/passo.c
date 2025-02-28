#include <msp430.h> 

void ini_uCon(void);
void ini_P1_P2(void);
void ini_Timer0_deb(void);
void ini_timer1(void);

/**
 * main.c
 */
void main(void)
{
    ini_uCon();
    ini_P1_P2();
    ini_Timer0_deb();
    ini_timer1();

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
     *   - P2.0 - IN1
     *   - P2.3 - IN2
     *   - P2.4 - IN3
     *   - P2.5 - IN4
     *   - Todas as outras portas em saída em nível baixo
     */

    P2DIR = 0xFF;
    P2OUT = 0;
}

void ini_Timer0_deb(void){
    /*  t_deb = 2ms
     *
     * CONTADOR
     *    - Clock = SMCLK ~ 2MHz
     *       - Fdiv.: 1
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

void ini_timer1(void){
    /*
     * t = 100ms
     *
     *  CONTADOR
     *    - Clock = SMCLK ~ 2MHz
     *       - Fdiv.: 4
     *    - Modo Contagem = PARADO -> UP
     *    - INT. cont.: Desabilitada
     *
     * MODULO 0
     *    - Funcao nativa: comparacao
     *    - TA1CCR0 = (2M * 0,1)/4 - 1 = 49999;
     *    - INT.: desabilitada
     *
     * */

    TA1CTL = TASSEL1 + MC0 + ID1;
    TA1CCTL0 = CCIE;
    TA1CCR0 = 49999;

}

#pragma vector=PORT1_VECTOR
__interrupt void RTI_Porta1(void){

    P1IFG &= ~BIT4;       // Desabilitar flag de int. (opcional)

    P1IE &= ~BIT4;   // Desabilitar interrupcao

    TA0CTL |= MC0;   // Contagem UP
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void RTI_Timer0_A0(void){

    TA0CTL &= ~MC0;     // Para a contagem

    if((~P1IN) & BIT4){
        if((~P1IN) & BIT5){ //Anti-horario
            if(TA1CCR0 <= 46399){ //43749
                TA1CCR0 += 3600; //4861
            }
            else{
               TA1CCR0 = 49999;
            }
        }
        else{  // Horario

             if(TA1CCR0 >= 7200){ //9722
                 TA1CCR0 -= 3600; // 4861
             }
            else{
                TA1CCR0 = 3600;
           }

        }
    }


    P1IFG &= ~BIT4;  // limpa a flag de int. (obrigatorio)

    P1IE |= BIT4;  // Habilita interrupcao
}

#pragma vector=TIMER1_A0_VECTOR
__interrupt void RTI_Timer1_A0(void){
    switch(P2OUT){
        case BIT0:
            P2OUT |= BIT3;
            P2OUT &= ~BIT0;
            break;
        case BIT3:
            P2OUT |= BIT4;
            P2OUT &= ~BIT3;
            break;
        case BIT4:
            P2OUT |= BIT5;
            P2OUT &= ~BIT4;
            break;
        case BIT5:
            P2OUT &= ~BIT5;
            P2OUT |= BIT0;
            break;
        default:
            P2OUT |= BIT0;
    }
}
