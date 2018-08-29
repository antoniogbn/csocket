// No codeblocks inclua no menu em: Project -> Build Options... -> Linker settings -> Other link options -l wsock32
//#define WIN // Se não for no windows comente essa linha e compile no terminal: gcc -o ts ts.c
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
#define MAXPENDING 100    /* Número máximo de requisições para conexão pendentes */

#define ARQP "par.txt"
#define ARQI "impar.txt"
#define ARQS "7.txt"



int criar_socket(int porta)
{
    int sock;
    struct sockaddr_in endereco; /* Endereço Local */

    /* Criação do socket TCP para recepção e envio de pacotes */
    if ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\nErro na criação do socket!\n");fflush(stdout);
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

int aceitar_conexao(int sock)
{
    int                socket_cliente;
    struct sockaddr_in endereco; /* Endereço Local */
    int                tamanho_endereco;

    /* Define o tamanho do endereço de recepção e envio */
    tamanho_endereco = sizeof(endereco);

    /* Aguarda pela conexão de um cliente */
    if ((socket_cliente = accept(sock, (struct sockaddr *) &endereco, &tamanho_endereco)) < 0)
    {
        printf("\nErro no accept()!\n");fflush(stdout);
        return(0);
    }
    return(socket_cliente);
}

int receber_mensagem(char *mensagem,int sock)
{
    /* Limpar o buffer da mensagem */
    memset((void *) mensagem,0,TAM_MENSAGEM);

    /* Espera pela recepção de alguma mensagem do cliente conectado*/
    if (recv(sock, mensagem, TAM_MENSAGEM, 0) < 0)
    {
        printf("\nErro na recepção da mensagem\n");fflush(stdout);
        return(-1);
    }

    printf("\nTCP Servidor: Recebi (%s)\n",mensagem);fflush(stdout);

    return(0);
}

int enviar_mensagem(char *mensagem,int sock)
{
    /* Devolve o conteúdo da mensagem para o cliente */
    if (send(sock, mensagem, strlen(mensagem), 0) != strlen(mensagem))
    {
        printf("\nErro no envio da mensagem\n");fflush(stdout);
        return(-1);
    }

    printf("\nTCP Servidor: Enviei (%s)\n",mensagem);fflush(stdout);

    return(0);
}

int main()
{

    int                sock;                   /* Socket */
    int                socket_cliente;         /* Socket de conexão com o cliente */
    int                resultado;              /* Resultado das funções */
    char               mensagem[TAM_MENSAGEM]; /* Buffer para a recepção da string de echo */

    FILE *arqI, *arqP, *arqS;


#ifdef WIN
    WORD wPackedValues;
    WSADATA  SocketInfo;
    int      nLastError,
	         nVersionMinor = 1,
	         nVersionMajor = 1;
    wPackedValues = (WORD)(((WORD)nVersionMinor)<< 8)|(WORD)nVersionMajor;
    nLastError = WSAStartup(wPackedValues, &SocketInfo);
#endif


    if ((arqP = fopen(ARQP,"wt"))==NULL)
    {
        printf("\n>> ERRO: criando o arquivo de saida (par.txt)\n");
        return(1);
    }
    if ((arqI = fopen(ARQI,"wt"))==NULL)
    {
        printf("\n>> ERRO: criando o arquivo de saida (impar.txt)\n");
        return(1);
    }
    if ((arqS = fopen(ARQS,"wt"))==NULL)
    {
        printf("\n>> ERRO: criando o arquivo de saida (7.txt)\n");
        return(1);
    }

    sock = criar_socket(PORTA_SERVIDOR_TCP);
    if (sock < 0)
    {
        printf("\nErro na criação do socket!\n");
        return(1);
    }

    for (;;) /* Loop eterno */
    {
        /* Aguarda por uma conexão e a aceita criando o socket de contato com o cliente */
        socket_cliente = aceitar_conexao(sock);
        if (socket_cliente == 0)
        {
            printf("\nErro na conexao do socket!\n");
            return(1);
        }

        /* Recebe a mensagem do cliente */
        resultado = receber_mensagem(mensagem,socket_cliente);
        if (resultado < 0)
        {
            printf("\nErro no recebimento da mensagem\n");
            return(1);
        }

        if (mensagem[5]=='A')
        {
           fprintf(arqS,"%.5s\n",mensagem);
           fprintf(arqP,"%.5s\n",mensagem);
        }
	else if (mensagem[5]=='B')
	{
           fprintf(arqP,"%.5s\n",mensagem);
	}
	else if (mensagem[5]=='C')
	{
           fprintf(arqS,"%.5s\n",mensagem);
           fprintf(arqI,"%.5s\n",mensagem);
        }
	else if (mensagem[5]=='D')
           fprintf(arqI,"%.5s\n",mensagem);
	else 
	{   
	   printf("\nErro : Mensagem Indefinida >> %s",mensagem);	
           mensagem[5]='U';
	}

	if (mensagem[5] != 'U') mensagem[5]='O';
	   
        /* Devolve o conteúdo da mensagem para o cliente */
        resultado = enviar_mensagem(mensagem,socket_cliente);
        if (resultado < 0)
        {
            printf("\nErro no envio da mensagem\n");
            return(1);
        }
        fflush(arqP);
        fflush(arqI);
        fflush(arqS);

        close(socket_cliente);    /* Fecha o socket do cliente */
    }

    fclose(arqP);
    fclose(arqI);
    fclose(arqS);

    /*não passa por aqui */
}
