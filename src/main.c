#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

// Veio com uma estruturinha? veio, mas eu entendi tudinho

int main() {
	// Desabilita o buffer para garantir que as mensagem exiba imediatamente
	// sem precisar de preencher o buffer
	setbuf(stdout, NULL);
 	setbuf(stderr, NULL);

	// server_fd é o identificador do socket do servidor
	int server_fd;
	// client_addr_len é o tamanho da estrutura client_addr
	// int tem tamanho fixo, e socklen_t vai usar o tamanho adequado
	// Usado para armazenar informação como IP e porta para identificar o cliente
	struct sockaddr_in client_addr;

	// Garante que accept tem o espaço adequado
	socklen_t client_addr_len = sizeof(client_addr);
	
	// AF_INET diz que é IPV4
	// SOCK_STREAM vai dizer que é um socket de fluxo que usa TCP
	// 0 é uma conversão que indica que o sistema operacional deve escolher o padrão para SOCK STREAM
	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	
	// Houve falha a criar o socket (falta de permissão, falta de memória, ou qualquer erro que não consiga preencher o socket)
	if (server_fd == -1) {
		printf("Criar o socket Falhou: %s...\n", strerror(errno));
		return 1;
	}
	
	// Configura o Socket para permitir a reutilização, quando o socket é fechado
	int reuse = 1;
	// Configura opção de socket para fazer reuse
	// SOL_SOCKET indica que é generico, não especifica se é TCP / UDP
	// SO_REUSEADDR opção que vai ser configurada
	// sizeof é pra saber se houve erro ao aplicar
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
		printf("SO_REUSEADDR Falhou: %s \n", strerror(errno));
		return 1;
	}
	
	// Armazenar o endereço do servidor
	struct sockaddr_in serv_addr = { .sin_family = AF_INET , // Familia de endereço IPv4
									 // htons (host to network short)
									 // - converte a porta para ordem de bytes de rede
									 // para ser 16 bits short do host, pro format big endian
									.sin_port = htons(4221), 
									 // Configura servidor para aceitar conexões de qualquer endereço IP disponivel na maquina
									 // htonl (host to network long) converte inteiro de 32 bits para o 
									 // formato de rede big endian
									 .sin_addr = { htonl(INADDR_ANY) },
									};
	
	// Verifica se o bind falhou
	// Associa o socket para um endereço IP e porta especifica
	// se falhar, ele vai dar menos -1
	if (bind(server_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) != 0) {
		printf("Bind Falhou: %s \n", strerror(errno));
		return 1;
	}
	
	// numero máximo de conexão pendente em fila
	int connection_backlog = 5;

	// Coloca o socket em modo de escuta, para aceitar conexão
	if (listen(server_fd, connection_backlog) != 0) {
		printf("Listen Falhou: %s \n", strerror(errno));
		return 1;
	}
	
	printf("Aguardando clientes...\n");
	
	// Accept cria um novo socket para cada requisição
	// e preenche com o detalhe do cliente
	int new_socket = accept(server_fd, (struct sockaddr *) &client_addr, &client_addr_len);
	
	printf("Client connected\n");

	// Agora eu tenho que retornar codigo 200 pro socket
	// baseado do `man send`, send(int socket, const void *buffer, size_t length, int flags);
	char *response = "HTTP/1.1 200 OK\r\n\r\n";
	send(new_socket, response, strlen(response), 0);	

	// fecha a conexão
	close(server_fd);

	return 0;
}
