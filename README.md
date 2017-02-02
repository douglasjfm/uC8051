Keil uVision project.
Intel 8051

projeto para um sistema automatico de controle de cameras vigilancia.

Especifica��es:

1 - Cada c�mera est� associada a um sensor e sempre que este detectar um movimento a filmagem nesta c�mera dever� estar entre as que ser�o gravadas (c�mera ativada). Caso passe mais de 5seg sem que o sensor perceba algum movimento, esta c�mera n�o ser� mais gravada (c�mera desativada);

2 � Caso n�o tenha nenhuma c�mera ativada, o v�deo deve ficar em pause;

 3 � Caso mais de uma c�mera esteja ativada, o multiplexador (mux) deve ser usado para selecionar o sinal das c�meras de forma que cada c�mera ativa seja gravada por 3seg. Ap�s este per�odo, a imagem da pr�xima c�mera ativa � gravada por 3seg, de forma que todas fiquem sendo gravadas enquanto estiverem ativas. Logicamente se apenas uma c�mera estiver ativa apenas o sinal dela dever� ser selecionado no mux.

4 � O controle mostrado na figura dever� ser implementado usando um 8051. Os sinais de controle (linhas vermelhas) devem ser mapeados para os bits das portas paralelas de modo conveniente ao desenvolvedor; 

5 � Todas as vezes que um sensor detectar movimento ou uma c�mera se tornar ativa, esta informa��o dever� ser enviada pela interface serial. Estas informa��es dever�o ser recebidas por um programa em um PC e apresentadas ao usu�rio visualmente.