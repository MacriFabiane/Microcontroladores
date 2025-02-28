#include <msp430.h>
#include "msprf24.h"
#include "nrf_userconfig.h"
#include "stdint.h"

#define POT BIT1 //P1.1
#define BTN BIT3 //P1.3

unsigned int ADC10_vetor[31];
unsigned int BTN_PRESSIONADO = 0; // 0 -> btn não pressionado    1 -> btn pressionado
float nivel_bateria;
float soma = 0.0;
float media = 0.0;
unsigned int i = 0;
volatile unsigned int user; //armazena informações sobre o estado do rádio nRF24L01+ e sobre a última tentativa de transmissão.


void Ini_Portas(void);
void Configurar_uCOn(void);
void Config_timer(void);
void Config_ADC(void);


int main(void)
{
    Configurar_uCOn();
    Ini_Portas();
    Config_timer();
    Config_ADC();


    uint8_t addr[5];//enderesso de 5 bytes para a comunicaççao (garante maior confiabilidade na transmissão
    uint8_t buf[5]; // via passar as infos de byte de ini, end interruptor, nivel bat, byte fim
    user = 0xFE;

    rf_crc = RF24_EN_CRC | RF24_CRCO;  // Habilita CRC de 16 bits
    rf_addr_width = 5;                 // Endereço de 5 bytes
    rf_speed_power = RF24_SPEED_1MBPS | RF24_POWER_0DBM;  // 1Mbps e potência 0dBm
    rf_channel = 120;                   // Canal RF 120

    msprf24_init(); //Inicializa o módulo NRF24L01.
    msprf24_set_pipe_packetsize(0, 5); //Define 4 bytes como o tamanho do pacote
    msprf24_open_pipe(0, 1); //Abre o pipe 0 para comunicação bidirecional com AutoACK (respostas automáticas do receptor)

    msprf24_standby();
    user = msprf24_current_state();
    addr[0] = 0xDE; addr[1] = 0xAD; addr[2] = 0xBE; addr[3] = 0xEF; addr[4] = 0x00;
    w_tx_addr(addr); //transmissor  e receptor devem ter o mesmo enderço pra comunicação funcionar
    w_rx_addr(0, addr);


    while(1){
        __delay_cycles(800000); // Pequena espera


        if((~P1IN) & BTN){
            BTN_PRESSIONADO = 1;
           buf[0] = 0xAA;   // Byte de início
           buf[1] = 0x01;   // Endereço do interruptor
           buf[2] = (unsigned int)(nivel_bateria*100);   // Nível de bateria
           buf[3] = BTN_PRESSIONADO;
           buf[4] = 0x55;   // Byte de fim


           w_tx_payload(5, buf);  // Carrega os dados para transmissão
           msprf24_activate_tx();  // Inicia a transmissão
           LPM4;  // Coloca o MSP430 em modo de baixo consumo (espera interrupção)

           msprf24_get_irq_reason();  // Pega causa da interrupção

           if (rf_irq & RF24_IRQ_FLAGGED) {
               rf_irq &= ~RF24_IRQ_FLAGGED;  // Limpa a flag

               if (rf_irq & RF24_IRQ_TX) { //transmissão foi bem sucedida
                  BTN_PRESSIONADO = 2; //transmitiu com sucesso!

               }
               if (rf_irq & RF24_IRQ_TXFAILED) { //transmissao falhou
                   P1OUT &= ~BIT0; // Apaga LED vermelho

               }

               msprf24_irq_clear(rf_irq);  // Limpa as interrupções do NRF24
               user = msprf24_get_last_retransmits();  // Obtém número de retransmissões
           }

        }
        if(BTN_PRESSIONADO == 2){
          buf[0] = 0xAA;   // Byte de início
          buf[1] = 0x01;   // Endereço do interruptor
          buf[2] = (unsigned int)(nivel_bateria*100);  // Nível de bateria
          buf[3] = BTN_PRESSIONADO;
          buf[4] = 0x55;   // Byte de fim

          w_tx_payload(5, buf);  // Carrega os dados para transmissão
          msprf24_activate_tx();  // Inicia a transmissão
          LPM4;  // Coloca o MSP430 em modo de baixo consumo (espera interrupção)

          msprf24_get_irq_reason();  // Pega causa da interrupção

          if (rf_irq & RF24_IRQ_FLAGGED) {
              rf_irq &= ~RF24_IRQ_FLAGGED;  // Limpa a flag

              if (rf_irq & RF24_IRQ_TX) { //transmissão foi bem sucedida
                  BTN_PRESSIONADO = 0;
              }
              if (rf_irq & RF24_IRQ_TXFAILED) { //transmissao falhou
                 P1OUT &= ~BIT0; // Apaga LED vermelho

             }

             msprf24_irq_clear(rf_irq);  // Limpa as interrupções do NRF24
             user = msprf24_get_last_retransmits();  // Obtém número de retransmissões
          }
        }

    }
}


void Configurar_uCOn(void){
    WDTCTL = WDTPW | WDTHOLD; // Stop watchdog timer

    /* CONFIG. DO BCS
     *
     * DCOCLK ~ 16 MHz (dados de calibracao)
     * VLOCLK = Nao Utilizado
     * LFXT1CLK = 32768 Hz
     *
     * MCLK = DCOCLK ~ 16 MHz
     * SMCLK = DCOCLK/4 ~ 2 MHz
     * ACLK = LFXT1CLK = 32768 Hz
     */

    DCOCTL = CALDCO_16MHZ;
    BCSCTL1 = CALBC1_16MHZ;
    BCSCTL3 = XCAP0 + XCAP1;
    BCSCTL2 = DIVS0 + DIVS1;

    while(BCSCTL3 & LFXT1OF);

     __enable_interrupt();

}

void Ini_Portas(void) {

    P1DIR = ~(BTN + POT); // seta o btn como entrada e o potenciometro
    P1REN |= BTN;
    P1OUT = BTN + POT;

    P1IES |= BTN;
    P1IFG = 0;
    P1IE |= BTN;

    P2DIR = 0xFF;
    P2OUT = 0;

}

void Config_timer(void){
     /*
      * Configuracoes iniciais do Timer0 para o Debouncer de S2
      *
      * CONTADOR
      * - Fonte de Clock: SMCLK ~ 2 MHz
      * - Fdiv clock: 1
      * - Modo cont.: Inicialmente >>>> PARADO!
      * - Int. cont.: desabilitada
      *
      * MODULO 0
      * - Modo: Comparacao (default)
      * - Interrupcao: Habilitada
      * - TA0CCR0 = (SMCLK * 0.025) - 1 = 49999
      *
      */

    TA0CTL = TASSEL1;
    TA0CCTL0 = CCIE;
    TA0CCTL1 = OUTMOD0 + OUTMOD1 + OUTMOD2 + OUT;
    TA0CCR0 = 49999;
    TA0CCR1 = 24999;

}

void Config_ADC(void){

    ADC10CTL0 = ADC10SHT0 + ADC10ON + MSC + ADC10IE;
    ADC10CTL1 = SHS0 + ADC10SSEL0 + ADC10SSEL1 + CONSEQ1 + INCH0;
    ADC10DTC1 = 31;
    ADC10AE0 = POT;
    ADC10SA = (unsigned int)&ADC10_vetor[0];
    ADC10CTL0 |= ENC;

}

#pragma vector=PORT1_VECTOR
__interrupt void RTI_DEB(void){

    P1IFG &= ~BTN;
    P1IE &= ~BTN;
    TA0CTL |= MC0;

}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void RTI_TIMER0(void){

    TA0CTL &= ~MC0;

    P1IFG &= ~BTN;

    P1IE |= BTN;

}

#pragma vector=ADC10_VECTOR
__interrupt void RTI_ADC10(void){

    ADC10CTL0 &= ~ENC;

    soma = 0;
    media = 0;

    for(i=0; i<31; i++){
        soma = soma + ADC10_vetor[i];
    }

    media = soma/31;

    nivel_bateria = (3.3*(float)media)/1023.0;


    ADC10SA = (unsigned int)&ADC10_vetor[0]; //reativar DTC para fazer as outras conversãoes
    ADC10CTL0 |= ENC;


}
