#include <msp430.h> 


/**
 * main.c
 */
void ini_uCon(void);
void ini_P1_P2(void);
void ini_timer0_debouncer(void);
void ini_timer1_PWM(void);

int main(void)
{
    ini_uCon();
    ini_P1_P2();
    ini_timer0_debouncer();
    ini_timer1_PWM();

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
      *   - Todas as portas em sa�da em n�vel baixo
      *   - p2.1 saída do servo motor
      */

     P2SEL = BIT1;
     P2DIR = 0xFF;
     P2OUT = 0;//tava em vdd
}


void ini_timer0_debouncer(void){
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

void ini_timer1_PWM(void){
    /*
     * freq - 50hz -> passando para período = 20ms
     * razão ciclica - 5% a 10%
     * largura de pulso - 1ms a 2ms
     *
     *
     *  TA1CCR0 = ((2M*0,02)) -1  = 39999
     *
     * */

    TA1CTL = TASSEL1 + MC0;
    TA1CCTL1 = OUTMOD0 + OUTMOD1 + OUTMOD2;
    TA1CCR0 = 39999;
    TA1CCR1 = 1999; // 5%
    //TA1CCR2 = 3999; // 10 %
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
            if((~P1IN) & BIT5){//ANTI HORÁRIO
                if(TA1CCR1 >= 1999){
                    TA1CCR1 -= 100;
                 }
                else{
                    TA1CCR1 = 1999;
                }

            }
            else{//HORÁRIO
                if(TA1CCR1 <= 3999){
                    TA1CCR1 += 100;
                }
                else{
                    TA1CCR1 = 3999;
                }

            }
        }



    P1IFG &= ~BIT4;  // limpa a flag de int. (obrigatorio)

    P1IE |= BIT4;  // Habilita interrupcao
}
