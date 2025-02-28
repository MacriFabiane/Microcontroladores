#include <msp430.h> 


void ini_uCOn(void);
void ini_P1_P2(void);
void ini_ADC10(void);
void ini_TA0_ADC10(void);


unsigned int ADC10_vetor[64];
unsigned int soma = 0, media = 0;
unsigned char i = 0;


void main(void) {

    ini_P1_P2();
    ini_uCOn();
    ini_ADC10();
    ini_TA0_ADC10();

    do{

    }while(1);
}

// RTI do Mod. 0 do Timer 0

#pragma vector=ADC10_VECTOR
__interrupt void RTI_ADC10(void){
    ADC10CTL0 &= ~ENC;
    soma = 0;

    for(i = 0; i < 64; i++) {
        if (i>10 && i<64){
            soma += ADC10_vetor[i];
        }
    }

    media = soma >> 6; // M�dia de 64 amostras (divis�o por 64) ou media = soma/64;

    if (media > 512) // Limite para ligar/desligar LED (ajust�vel conforme necessidade)
       P1OUT &= ~BIT0;  // Liga LED vermelho
    else
       P1OUT |= BIT0; // Desliga LED vermelho


    ADC10SA = &ADC10_vetor[0]; //reativar DTC para fazer as outras convers�es
    ADC10CTL0 |= ENC;
}


void ini_TA0_ADC10(void){

    /*
     * f = 10 Hz
     * fa = 100ms
     * RC = 50%
     *
     * CONTADOR
     *      - Modo: UP
     *      - Clock: SMCLK = 2 MHz
     *         -fdiv = 4
     *
     * MODULO 0
     *      - Funcao nativa: comparacao
     *      - TA0CCR0 = 2M*0,1/4 = 50000-1
     *      - Int.: Habilitada
     *
     * MODULO 1
     *    - Funcao nativa: comparacao
     *    - TA1CCR1 = 24999
     *    - INT.: desabilitada
     *    - Modo de sa�da: 7 - Reset/ set
     */


    TA0CTL = TASSEL1 + ID1 + MC0;
    TA0CCTL1 = OUTMOD0 + OUTMOD1 + OUTMOD2 + OUT;
    TA0CCR0 = 49999;
    TA0CCR1 = 24999;  //PWM

}


void ini_ADC10(void){

     /* ADC modo: canal �nico repetido
      * Gatilho por Hardware via Mod. 1 do Timer0
      * Clock: ADC10OSC -> Hz
      *
      * Canal de entrada: A5     Canal de sa�da: A4
      * Impedancia: Pot. de 10 kohms
      *
      */


     ADC10CTL0 = SREF0 + ADC10SHT1 + ADC10SHT0 + MSC + REF2_5V + REFON + REFOUT + ADC10ON + ADC10IE; // precisa do REFOUT para a sa�da anal�gica
     ADC10CTL1 = INCH0 + INCH2 + SHS0 + CONSEQ1;
     ADC10DTC1 = 64; // Coleta 64 amostras
     ADC10AE0 = BIT5 + BIT4; // Ativa A5 como entrada anal�gica E A4 como sa�da anal�gica
     ADC10SA = &ADC10_vetor[0]; // Define o endere�o do buffer para o DTC
     ADC10CTL0 |= ENC;

}




void ini_uCOn(void){
    WDTCTL = WDTPW | WDTHOLD; // Stop watchdog timer

    /* CONFIG. DO BCS
     *
     * DCOCLK ~ 16 MHz (dados de calibracao)
     * VLOCLK = Nao Utilizado
     * LFXT1CLK = 32768 Hz
     *
     * MCLK = DCOCLK ~ 16 MHz
     * SMCLK = DCOCLK/8 ~ 2 MHz
     * ACLK = LFXT1CLK = 32768 Hz
     */

    DCOCTL = CALDCO_16MHZ;
    BCSCTL1 = CALBC1_16MHZ;
    BCSCTL3 = XCAP0 + XCAP1;
    BCSCTL2 = DIVS0 + DIVS1;

    while(BCSCTL3 & LFXT1OF);

     __enable_interrupt();

}


void ini_P1_P2(void){

    P1DIR = BIT0; // Configura P1.0 (LED vermelho) como sa�da
    P1OUT = 0;
    P2DIR = 0xFF;
    P2OUT = 0;

}

