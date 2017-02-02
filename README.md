Keil uVision project.
Intel 8051

projeto para um sistema automatico de controle de cameras vigilancia.

Especificações:

1 - Cada câmera está associada a um sensor e sempre que este detectar um movimento a filmagem nesta câmera deverá estar entre as que serão gravadas (câmera ativada). Caso passe mais de 5seg sem que o sensor perceba algum movimento, esta câmera não será mais gravada (câmera desativada);

2 – Caso não tenha nenhuma câmera ativada, o vídeo deve ficar em pause;

 3 – Caso mais de uma câmera esteja ativada, o multiplexador (mux) deve ser usado para selecionar o sinal das câmeras de forma que cada câmera ativa seja gravada por 3seg. Após este período, a imagem da próxima câmera ativa é gravada por 3seg, de forma que todas fiquem sendo gravadas enquanto estiverem ativas. Logicamente se apenas uma câmera estiver ativa apenas o sinal dela deverá ser selecionado no mux.

4 – O controle mostrado na figura deverá ser implementado usando um 8051. Os sinais de controle (linhas vermelhas) devem ser mapeados para os bits das portas paralelas de modo conveniente ao desenvolvedor; 

5 – Todas as vezes que um sensor detectar movimento ou uma câmera se tornar ativa, esta informação deverá ser enviada pela interface serial. Estas informações deverão ser recebidas por um programa em um PC e apresentadas ao usuário visualmente.