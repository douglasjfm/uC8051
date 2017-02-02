/* PROJETO FINAL - SISTEMA SELETOR DE CAMERAS DE MONITORAMENTO*/
/*
	por: Douglas Medeiros, djfm
*/
/*
	DISPOSITIVO NO uVision: 80C51FA - INTEL
*/
/*DEFINICOES:
	OS SINAIS DOS SENSORES (sensor0,sensor1,sensor2,sensor3) SAO LIDOS PELA PORTA P0 8051.
	O SINAL DE Pause É ENVIADO PELA PORTA P1 BIT 0 ;
	OS BITS SELETORES (mux0 e mux1) DO MUX SAO ENVIADOS PELA PORTA P2: BIT 0 E BIT 1.
	INTUITIVAMENTE:
		mux0	mux1	camera
		0		0		0
		0		1		1
		1		0		2
		1		1		3
*/
/*	TIMER UTILIZADO: Timer 0 */

/*BASICAMENE:
    QUANDO UMA OU MAIS CAMERAS SAO ATIVADAS ESTAS SAO MARCADAS,DESMARCA-SE-AS
    QUANDO ATINGEM O TEMPO DE 5 SEGUNDOS DESATIVADAS.
*/

/*
	CAMERA FILMAVEL: SAO AS CAMERAS MARCADAS PARA SEREM GRAVADAS NO VIDEO;
	É DIFERENTE DE CAMERA ATIVA - CAMERA CUJO SENSOR ESTA ACIONADO.
		HA CAMERAS FILMAVEIS QUE PODEM NAO ESTAR ATIVAS.!
*/

#include <REG51F.h>

#define FrClk 12000000
#define FreqTimer0_emHz 100
#define VALOR_TH0 ((65536 -(FrClk / (12 * FreqTimer0_emHz  ))) >>8)
#define VALOR_TL0 ((65536 -(FrClk / (12 * FreqTimer0_emHz  ))) & 0xFF)

sbit sensor0 = P0^0;
sbit sensor1 = P0^1;
sbit sensor2 = P0^2;
sbit sensor3 = P0^3;

sbit pause = P1^0;

sbit mux0 = P2^0;
sbit mux1 = P2^1;


int cem5[4];//contadores de 5 segs para cada camera; (conta 1/100 segs 500 vezes)
int cem3=0;//contador de 3 segs	para altarnar a saida do mux de video
short int camera_ativa[4],filmavel[4];//flag de camera ativa e camera a ser filmada, para cada uma delas.
short int sel_msg,i_msg;
char msg[2][22] = {" cameras desativadas"," camera _ filmando  "};//mensagens
bit alternar;//FLAG
short camera_atual=0;

////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////
//           INICIALIZACAO DA SERAL
void inicializa_serial(void)
{
    SCON = 0x50;				 // modo 1 com stop_bit zero e recepcao habilitada.
    if ((PCON>>7)!=0)PCON += 128; //ajusta PCON.7 pra zero.
    ES=1;						 //habilita interrupcao da serial
}
////////////////////////////////////////////////////////////////
//           INICIALIZACAO DO TIMER_1
void inicializa_timer1(void)//para sincronizar a serial
{
    TR1=0;
    TMOD = (TMOD & 0x0f) | 0x20;
    TH1 = 0xE6;
    TL1 = 0xFF;	 // AJUSTA BOUDRATE PARA ~1200 BPS (1201 NA PRATICA!)
    ET1 = 0;
}
////////////////////////////////////////////////////////////////
//INICIALIZACAO DO TIMER_0
void inicializa_timer0(void)

{
    TR0 = 0; // Desliga Timer0
    TMOD = (TMOD & 0xF0) | 0x01; // Timer 0 programado como timer de 16 bits
    TH0 = VALOR_TH0; // Programa contagem do Timer0
    TL0 = VALOR_TL0;
    ET0 = 1; // Habilita interrupcao do timer 0
    TR0 = 1; // Habilita contagem do timer 0
}
/////////////////////////////////////////////////////////////////
//INTERRUPCAO DO TIMER_0
void timer0_int (void) interrupt 1 using 2
{
    char i;
    TR0 = 0; // Desliga Timer0
    TH0 += VALOR_TH0; // Programa contagem do Timer0
    TL0 += VALOR_TL0;
    TR0 = 1; // Habilita contagem do timer 0

    cem3++;
    cem5[0]++;//INCREMENTA O CONTADOR DE 5SEGUNDOS DE CADA CAMERA.
    cem5[1]++;
    cem5[2]++;
    cem5[3]++;
    for (i=0;i<4;i++)//SE I-ESIMA CAMERA DESATIVADA E cem5[I]==500 ->MARCA-SE CAMERA COMO NAO-FILMAVEL.
        if (cem5[i]==500 && !camera_ativa[i]) filmavel[i] = 0;//DESMARCA-SE A CAMERA;

    if (cem3==300 && alternar)//DECORRIDOS TRES SEGUNDOS E SINALIZADA A ALTERNANCIA
    {
        camera_atual = (camera_atual+1) & 0x03;//passa para a proxima camera
        i=camera_atual;
        while (!filmavel[i]) camera_atual = i = (i+1)&0x03;// evita as camaras que nao estao marcadas
		cem3 = 0;  //reinicia o contador.
    }

}

//////////////////////////////////////////////////
//FUNCAO QUE CALCULA O NUMERO DE CAMERAS A FILMAR.
char cameras_a_filmar (void)
{
    char i,count=0;
    i = 0;
    for(i = 0; i < 4; i++)
		if (filmavel[i]==1)
			count++;
    return count;
}

void serial_int(void) interrupt 4 using 2
{
    if (TI && msg[sel_msg][i_msg]!='\0')//SE NAO FOR FIM DA STRING:
    {
        SBUF = msg[sel_msg][i_msg];
        i_msg++;
        TI=0;
    }
    else if (TI)
    {
        TI = 0;//FIM DA TRANSMISSAO.
        TR1=0; //INTERROMPE TODA A SERIAL.
    }
}

void send_message(void)
{
    i_msg = 0;//INICIALIZA INDICE DA STRING PARA TRANSMISSAO
    TR1=1;	  //DISPARA O TIMER 1
    TI = 1;	  //INICIA A TRANSMISSAO
}
//FUNCAO QUE MANDA MENSAGEM
void message(bit tipo,short int cam)
//message(tipo, camera) -- SE (tipo == ZERO) -> IMPRIME A MSG DE CAMERAS DESATIVADAS;
{
    if (!tipo)
    {
        sel_msg = 0;
        msg[sel_msg][20] = '\n';//POE QUEBRA DE LINHA NO FIM DA STRING;
        msg[sel_msg][21] = '\0';

		msg[1][8] = '_';//REINICIALIZA A MSG DE CAMERA FILMANDO. CASO A MESMA CAMERA VOLTE
    }					// A FILMAR ELA NAO IMPRIMIRA POR CAUSA DO IF (LN: 165);
    else
    {
        sel_msg = 1;
        msg[sel_msg][20] = '\n';//POE QUEBRA DE LINHA NO FIM DA STRING;
        msg[sel_msg][21] = '\0';
        if (msg[sel_msg][8] == (cam+48) ) return;//SE NAO HOUVE MUDANCA DE CAMERA NAO ENVIA A MESMA MSG.
        msg[sel_msg][8] = cam + 48;
    }
    send_message();
}

void main (void)
{
    char estado/*ESTADO DA MAQUINA*/,estado_antigo = 1111/*SEM ESTADO ANTIGO*/, n/*VARIAVEL QUALQUE PARA CONTAGENS*/;
    EA = 1;
    pause = 1;
    camera_ativa[0] = camera_ativa[1] = camera_ativa[2] = 0;
    camera_ativa[3] = 0;//TODAS AS CAMERAS DESATIVADAS
    filmavel[0] = filmavel[1] = filmavel[2] = 0;
    filmavel[3] = 0;// TODAS AS CAMERAS NAO FILMAVEIS
    inicializa_serial();
    inicializa_timer1();
    inicializa_timer0();
    while (1)
    {
	//////////CAMERA 0:///////////////////////////////////////////////////////////
        if (sensor0)//EFETUA-SE A LEITURA DO SENSOR DE PRESENCA;
        {
            if (!camera_ativa[0])
            {
                camera_ativa[0] = 1;//SE A CAMERA ESTAVA DESATIVADA, ATIVA-SE.
                filmavel[0] = 1;		// CAMERA FILMAVEL.
            }
        }
        else
        {
            if (camera_ativa[0])//CASO SENSOR INATIVO:
            {
                cem5[0] = 0;//INICIA-SE A CONTAGEM PARA REMOCAO DA FLAG QUE MARCA A CAMERA PRA FILMAR
                camera_ativa[0] = 0;//DESATIVA-SE A CAMERA
            }
        }

        ////////////O COMPORTAMENTO DAS DEMAIS É IDENTICO À PRIMEIRA/////////////////

		//////////CAMERA 1:///////////////////////////////////////////////////////////
        if (sensor1)
        {
            if (!camera_ativa[1] )
            {
                camera_ativa[1] = 1;
                filmavel[1] = 1;
            }
        }
        else
        {
            if (camera_ativa[1])
            {
                cem5[1] = 0;
                camera_ativa[1] = 0;
            }
        }

        ////////////////////////////////////////////////////////
		//////////CAMERA 2:///////////////////////////////////////////////////////////
        if (sensor2)
        {
            if (!camera_ativa[2] )
            {
                camera_ativa[2] = 1;
                filmavel[2] = 1;
            }
        }
        else
        {
            if (camera_ativa[2])
            {
                cem5[2] = 0;
                camera_ativa[2] = 0;
            }
        }

        ///////////////////////////////////////////////////////
		//////////CAMERA 3:///////////////////////////////////////////////////////////
        if (sensor3)
        {
            if (!camera_ativa[3])
            {
                camera_ativa[3] = 1;
                filmavel[3] = 1;
            }
        }
        else
        {
            if (camera_ativa[3])
            {
                cem5[3] = 0;
                camera_ativa[3] = 0;
            }
        }

        /////////////////////////////////////////////////////////////////
        n = cameras_a_filmar();// PEGA A QUANTIDADE CAMERAS PARA FILMAR;
        

        if (n == 1) estado = 1;//UMA UNICA CAMERA FILMAVEL
        else if (n > 1)estado = 2;// VARIAS CAMERAS alternar DE 3 SECS
        else estado = 0;// NENHUMA CAMERA ATIVA(filmavel na verdade).

        switch ( estado )
        {
        case 0 :
            if (estado_antigo!=0)
            {
                alternar = 0; // NA HA ALTERNANCIA (NA SELECAO DA SAIDA DO MUX);
                message(0,0);//message(char tipo,short camera) -- SE (tipo == ZERO) -> IMPRIME A MSG DE CAMERAS DESATIVADAS;
                pause = 1;	// PAUSA-SE O GRAVADOR DE VIDEO
            }
            estado_antigo = 0;
            break;
        case 1 :

            if (estado_antigo!=1)
            {//ESTE BLOCO SO EXECUTA QUANDO ENTRA NESTE ESTADO VINDO DE OUTRO
				alternar = 0;// NA HA ALTERNANCIA (NA SELECAO DA SAIDA DO MUX);
                n = camera_atual = 0;		  //BUSCA-SE NO VETOR filmavel[] A CAMERa__
                while (!filmavel[n]) camera_atual = ++n;//__qUE ESTA PRA FILMAR.
				message(1,camera_atual);// ENVIA A MENSAGEM DA CAMERA FILMADA
            }// NO FIM DO while ACIMA camera_atual INDICA A CAMERA A SER FILMADA.
			pause = 0;
            estado_antigo = 1;
            break;
        case 2:

            if (estado_antigo!=2)
            {//OBS.: QUE ESTA PARTE SO PRECISA EXECUTAR A 1A VEZ QUE A MAQ. VEM DE OUTRO ESTADO
                n = camera_atual = 0;
                while (!filmavel[n]) camera_atual = ++n;
				// NO FIM DO while ACIMA camera_atual INDICA A CAMERA A SER FILMADA.
				cem3=0;//INICIALIZA CONTAGEM DE ALTERNACIA (3 seg)
				alternar = 1;//MARCA-SE A FLAG QUE INDICA ALTERNANCIA
            }
            message(1,camera_atual);//IMPRIME A MENSAGEM DA CAMERA ATUAL

            pause = 0;
            estado_antigo = 2;
            break;
        }

        //ATUALIZA O MUX DE VIDEO.
        if ((camera_atual & 0x01)==0) mux0=0;
        else mux0=1;
        if ((camera_atual & 0x02)==0) mux1=0;
        else mux1=1;

    }
}
