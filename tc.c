// No codeblocks inclua no menu em: Project -> Build Options... -> Linker settings -> Other link options -l wsock32
//#define WIN 
// Se nao for no windows comente essa linha e compile no terminal: gcc -o tc tc.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#ifdef WIN
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#endif

#define TAM_MENSAGEM 6     /* mensagem de maior tamanho */
#define PORTA_SERVIDOR_TCP 9999
#define MAXPENDING 5

#define NITEM 30
#define ARQE "e.txt"
#define TEMPO_SLEEP 10


void m_usleep(unsigned long pause)
{
#ifdef MWIN
   Sleep(pause);
#else
   usleep(pause*1000);
#endif
   return;
}

int gerar_entrada()
{
    FILE *arq;
    int i;

    if ((arq = fopen(ARQE,"wt"))==NULL)
    {
        printf("\n>> ERRO: criando o arquivo de entrada (e.txt)\n");
        return(1);
    }

    for (i = 1 ; i <= NITEM; ++i)
        fprintf(arq,"%05d\n",i);

    fflush(arq);
    fclose(arq);

    return(0);
}

int criar_socket(int porta)
{
    int sock;
    struct sockaddr_in endereco; /* Endereço Local */

    /* Criacao socket datagrama/UDP para recepção e envio de pacotes */
    if ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\nErro na criacao socket!\n");fflush(stdout);
        return(-1);
    }

    if (porta > 0)
    {
        /* Construção da estrutura de endereço local */
        memset(&endereco, 0, sizeof(endereco));       /* Zerar a estrutura */
        endereco.sin_family      = AF_INET;           /* Família de endereçamento da Internet */
        endereco.sin_addr.s_addr = htonl(INADDR_ANY); /* Qualquer interface de entrada */
        endereco.sin_port        = htons(porta);      /* Porta local */

        /* Instanciar o endereco local */
        if (bind(sock, (struct sockaddr *) &endereco, sizeof(endereco)) < 0)
        {
           printf("\nErro no bind()!\n");fflush(stdout);
           return(-1);
        }

        /* Indica que o socket escutara as conexões */
        if (listen(sock, MAXPENDING) < 0)
        {
           printf("\nErro no listen()!\n");fflush(stdout);
           return(-1);
        }

    }

    return(sock);
}

int conectar_com_servidor(int sock,char *IP,int porta)
{
    struct sockaddr_in endereco; /* Endereço Local */

    /* Construção da estrutura de endereço do servidor */
    memset(&endereco, 0, sizeof(endereco));   /* Zerar a estrutura */
    endereco.sin_family      = AF_INET;       /* Família de endereçamento da Internet */
    endereco.sin_addr.s_addr = inet_addr(IP); /* Endereço IP do Servidor */
    endereco.sin_port        = htons(porta);  /* Porta do Servidor */

    /* Estabelecimento da conexão com o servidor de echo */
    if (connect(sock, (struct sockaddr *) &endereco, sizeof(endereco)) < 0)
    {
        printf("\nErro no connect()!\n");fflush(stdout);
        return(-1);
    }
    return(0);
}

int enviar_mensagem(char *mensagem,int sock)
{
    /* Envia o conteúdo da mensagem para o cliente */
    if (send(sock, mensagem, strlen(mensagem), 0) != strlen(mensagem))
    {
        printf("\nErro no envio da mensagem\n");fflush(stdout);
        return(-1);
    }

    printf("\nTCP Cliente: Enviei (%s)\n",mensagem);fflush(stdout);

    return(0);
}

int receber_mensagem(char *mensagem,int sock)
{
    /* Limpar o buffer da mensagem */
    memset((void *) mensagem,0,TAM_MENSAGEM);

    /* Espera pela recepcao alguma mensagem do cliente conectado*/
    if (recv(sock, mensagem, TAM_MENSAGEM, 0) < 0)
    {
        printf("\nErro na recepcao mensagem\n");fflush(stdout);
        return(-1);
    }

    printf("\nTCP Cliente: Recebi (%s)\n",mensagem);fflush(stdout);

    return(0);
}


int main(int argc, char *argv[])
{
    int  sock;                   // Socket 
    int  resultado;              // Resultado das funcoes
    char mensagem[TAM_MENSAGEM]; // Buffer para a recepcao string de echo
    int  valor,sete=0, par=0, impar=0, flag=0;
    char IP[] = "0.0.0.0";       // EnderecIP do servidor

    FILE *arq;

#ifdef WIN
    WORD wPackedValues;
    WSADATA  SocketInfo;
    int      nLastError,
	         nVersionMinor = 1,
	         nVersionMajor = 1;
    wPackedValues = (WORD)(((WORD)nVersionMinor)<< 8)|(WORD)nVersionMajor;
    nLastError = WSAStartup(wPackedValues, &SocketInfo);
#endif


//  limpa o buffer de mensagem
    memset((void *) mensagem,0,TAM_MENSAGEM);

//  geracao do arquivo de entrada
    gerar_entrada();

//  abre o arquivo
    arq = fopen(ARQE,"r");

// loop leitura do arquivo e.txt
    while (fscanf(arq,"%s",mensagem) == 1)
    {
         
         sete=par=impar=flag=0;

         valor = atoi(mensagem);

//ANALISA VALOR LIDO  DE ENTRADA E DEFINE O TIPO
         sete   = !(valor%7);
         par    = !(valor%2)*3;
         impar  = (!par)*5;
         
         flag = sete + par + impar;  
         
         switch (flag)
         {
            case 4: 
         	 mensagem[5]='A';         
                 break;
            case 3: 
         	 mensagem[5]='B';         
                 break;
            case 6: ;
         	 mensagem[5]='C';         
                 break;
            case 5: 
         	 mensagem[5]='D';         
                 break;
            default: 
         	 mensagem[5]='U';         

         }

/*         if ((valor%7) == 0)
	    mensagem[5] = 'S';
         else if ((valor%2) == 0)
  	      mensagem[5] = 'P';
         else
  	      mensagem[5] = 'I';*/

         sock = criar_socket(0);

         if (sock < 0)
         {
            printf("\nErro na criacao do  socket!\n");
            return(1);
         }

         resultado = conectar_com_servidor(sock,IP,PORTA_SERVIDOR_TCP);
         if (resultado < 0)
         {
           printf("\nErro na conexao com o servidor\n");
           return(1);
         }

         resultado = enviar_mensagem(mensagem,sock);
//       printf("%s\n", mensagem);

         if (resultado < 0)
            printf("\nErro no envio da mensagem\n");

//       Recebendo como resposta a mesma string vinda do servidor
         resultado = receber_mensagem(mensagem,sock);
         if (resultado < 0)
            printf("\nErro no recebimento da mensagem\n");

         close(sock);
         m_usleep(TEMPO_SLEEP);
    } 

    return(0);
}
