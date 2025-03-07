## Projeto Final Campainha Inteligente

O Objetivo deste projeto foi comprovar todo o conhecimento obtido durante a disciplina de Sistemas Microcontrolados, utilizando o **MSP430G2553**

![2024_02_Proposta_SM26CP_Campainha](https://github.com/user-attachments/assets/a962804f-555d-41e3-94dc-d737eb5abb36)

O projeto foi dividido em duas partes, sendo uma delas o Transmissor e a outra o Receptor. Tanto no Transmissor quanto no Receptor foram utilizados um MSP430G2553 e um módulo NRF24L01 em cada. No Transmissor também havia um potenciômetro que simulava a bateria para poder verificar se estava baixa ou alta. A interface para transferência de dados foi a SPI. Utilizamos as bibliotecas nRF24L01.h, msprf24.h e msp430_spi.h para fazer as transferências de dados.

### [Projeto Campainha - TX](https://github.com/MacriFabiane/Microcontroladores/blob/main/Projeto_Final_Campainha_Inteligente/Campainha_TX/main.c)

### [Projeto Campainha - RX](https://github.com/MacriFabiane/Microcontroladores/blob/main/Projeto_Final_Campainha_Inteligente/Campainha_RX/main.c)

