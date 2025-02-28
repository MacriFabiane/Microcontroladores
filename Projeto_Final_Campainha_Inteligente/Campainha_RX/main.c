#include <msp430.h>
#include "msprf24.h"
#include "nrf_userconfig.h"

uint8_t rf_speed_power;  // Declaração da variável
volatile unsigned int user; //armazena informações sobre o estado do rádio nRF24L01+ e sobre a última tentativa de transmissão.

void Configurar_uCOn(void);
void Ini_Portas(void);

int main(void){
    Configurar_uCOn();
    Ini_Portas();

    uint8_t addr[5];//enderesso de 5 bytes para a comunicaççao (garante maior confiabilidade na transmissão
    uint8_t buf[5]; // via passar as infos de byte de ini, end interruptor, nivel bat, byte fim
    user = 0xFE;


    rf_crc = RF24_EN_CRC | RF24_CRCO;  // Habilita CRC de 16 bits
    rf_addr_width = 5;                 // Endereço de 5 bytes
    rf_speed_power = RF24_SPEED_1MBPS | RF24_POWER_0DBM;  // 1Mbps e potência 0dBm
    rf_channel = 120;                   // Canal RF 120

    msprf24_init(); //Inicializa o módulo NRF24L01.
    msprf24_set_pipe_packetsize(0, 5); //Define 4 bytes como o tamanho do pacote
    msprf24_open_pipe(0, 1); //Abre o pipe 0 para comunicação bidirecional com AutoACK (respostas automáticas do receptor).

    addr[0] = 0xDE; addr[1] = 0xAD; addr[2] = 0xBE; addr[3] = 0xEF; addr[4] = 0x00; //O receptor só aceitará pacotes enviados para este endereço de 5 bytes.devem ser o mesmo do transmissor
    w_rx_addr(0, addr);
    w_tx_addr(addr);//n

    msprf24_standby();//n
    if (!(RF24_QUEUE_RXEMPTY & msprf24_queue_state())) {
        flush_rx();  // Limpa o buffer de recepção, se houver dados antigos
    }
    msprf24_activate_rx();  // Entra no modo de recepção
    LPM4;  // Coloca o MSP430 em baixo consumo até receber uma interrupção

    while(1){
        if (rf_irq & RF24_IRQ_FLAGGED) { //Quando o NRF24L01 recebe um pacote, ele gera uma interrupção IRQ
            rf_irq &= ~RF24_IRQ_FLAGGED;   // Limpa a flag de interrupção
            msprf24_get_irq_reason();      // Obtém o motivo da interrupção
        }

        if (rf_irq & RF24_IRQ_RX || msprf24_rx_pending()) { //Se os dados chegaram corretamente:
            r_rx_payload(5, buf);      // Lê os 4 bytes recebidos para o buffer buf
            msprf24_irq_clear(RF24_IRQ_RX); // Limpa a interrupção de RX
            user = buf[0];  // Guarda o primeiro byte recebido

            if(buf[1] == 0x01 && buf[0] == 0xAA  && buf[4] == 0x55){ //verifica se o sinal veio da campainha certa, e se os bytes de inicio e fim são correspondentes
                if(buf[3] == 1){
                    P1OUT |= BIT0; //TEM ALGUEM NA PORTA
                }
                else{
                    P1OUT &= ~BIT0;
                }

                if(buf[2] < 4){ // se a bateria estiver menor que 20%
                    P1OUT |= BIT4; //bateria fraca
                }
                else{
                    P1OUT &= ~BIT4;
                }
            }

        } else {
            user = 0xFF;
        }
        LPM4;
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

    P1DIR |= BIT0 + BIT4; //led para verificar a transmissão e bateria
    P1OUT &= ~(BIT0 + BIT4);

    P2DIR = 0xFF;
    P2OUT = 0;
}
