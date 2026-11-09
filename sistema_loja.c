#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <locale.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <shellapi.h>
#include <conio.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "shell32.lib")

#define PORTA_SERVIDOR 8080
#define BUFFER_HTTP 65536
#define MAX_PRODUTOS 20
#define MAX_ITENS_CARRINHO 10
#define ARQUIVO_PEDIDOS "pedidos_loja.txt"
#define ARQUIVO_PRODUTOS "produtos_estoque.txt"
#define ARQUIVO_HTML "web/index.html"

// ============================================================================
// 1. DEFINIÇÃO DAS ESTRUTURAS DE DADOS (STRUCTS)
// ============================================================================

typedef struct {
    int id;                     // Código identificador único
    char nome[80];              // Descrição comercial do produto
    char categoria[40];         // Ex: Placas de Video, Processadores, Perifericos
    double preco;               // Valor de venda em Reais (R$)
    double preco_antigo;        // Preço promocional (de/por)
    int estoque;                // Quantidade real disponível em estoque
    int estoque_minimo;         // Ponto de reposição
    char specs[80];             // Especificações resumidas
    char badge[30];             // Selo comercial
} Produto;

typedef struct {
    int id_produto;
    int quantidade;
    double preco_unitario;
    double subtotal;
} ItemCarrinho;

typedef struct {
    char nome[80];
    char cpf[16];
    char email[80];
    char endereco[120];
} Cliente;

typedef struct {
    int id_pedido;
    char codigo_pedido[32];
    Cliente cliente;
    ItemCarrinho itens[MAX_ITENS_CARRINHO];
    int total_itens;
    double valor_bruto;
    double desconto;
    double valor_total;
    char forma_pagamento[30];
    char data_hora[30];
} Pedido;

// Variáveis Globais de Telemetria
static int g_requisicoes_atendidas = 0;

// ============================================================================
// 2. FUNÇÕES AUXILIARES DE SISTEMA
// ============================================================================

void limpar_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void pausar_tela() {
    printf("\nPressione [ENTER] para voltar ao menu principal...");
    limpar_buffer();
}

void obter_data_hora_atual(char *buffer, size_t tamanho) {
    time_t tempo_bruto;
    struct tm *info_tempo;
    time(&tempo_bruto);
    info_tempo = localtime(&tempo_bruto);
    strftime(buffer, tamanho, "%d/%m/%Y %H:%M:%S", info_tempo);
}

// ============================================================================
// 3. FUNÇÃO: INICIALIZAR CATÁLOGO DE PRODUTOS
// ============================================================================
int inicializar_catalogo_padrao(Produto catalogo[]) {
    int count = 0;

    // 1. Placa de Vídeo
    catalogo[count].id = 1;
    strcpy(catalogo[count].nome, "Placa de Video RTX 4080 Super 16GB");
    strcpy(catalogo[count].categoria, "Placas de Video");
    catalogo[count].preco = 7299.90;
    catalogo[count].preco_antigo = 8199.00;
    catalogo[count].estoque = 5;
    catalogo[count].estoque_minimo = 2;
    strcpy(catalogo[count].specs, "16GB GDDR6X | 256-bit | Tri-Fan");
    strcpy(catalogo[count].badge, "Destaque");
    count++;

    // 2. Processador
    catalogo[count].id = 2;
    strcpy(catalogo[count].nome, "Processador AMD Ryzen 7 7800X3D");
    strcpy(catalogo[count].categoria, "Processadores");
    catalogo[count].preco = 2899.00;
    catalogo[count].preco_antigo = 3299.00;
    catalogo[count].estoque = 8;
    catalogo[count].estoque_minimo = 3;
    strcpy(catalogo[count].specs, "8 Cores / 16 Threads | 104MB Cache");
    strcpy(catalogo[count].badge, "Mais Vendido");
    count++;

    // 3. Teclado
    catalogo[count].id = 3;
    strcpy(catalogo[count].nome, "Teclado Mecanico RGB Switch Red");
    strcpy(catalogo[count].categoria, "Perifericos");
    catalogo[count].preco = 349.90;
    catalogo[count].preco_antigo = 429.00;
    catalogo[count].estoque = 15;
    catalogo[count].estoque_minimo = 5;
    strcpy(catalogo[count].specs, "Anti-Ghosting | Switch Red Linear");
    strcpy(catalogo[count].badge, "Oferta");
    count++;

    // 4. Mouse
    catalogo[count].id = 4;
    strcpy(catalogo[count].nome, "Mouse Gamer Sem Fio 26000 DPI");
    strcpy(catalogo[count].categoria, "Perifericos");
    catalogo[count].preco = 299.90;
    catalogo[count].preco_antigo = 389.00;
    catalogo[count].estoque = 12;
    catalogo[count].estoque_minimo = 4;
    strcpy(catalogo[count].specs, "Sensor Optico 26K | Peso 58g");
    strcpy(catalogo[count].badge, "Novo");
    count++;

    // 5. Monitor
    catalogo[count].id = 5;
    strcpy(catalogo[count].nome, "Monitor Gamer 27\" IPS 165Hz 1ms");
    strcpy(catalogo[count].categoria, "Monitores");
    catalogo[count].preco = 1499.00;
    catalogo[count].preco_antigo = 1799.00;
    catalogo[count].estoque = 6;
    catalogo[count].estoque_minimo = 2;
    strcpy(catalogo[count].specs, "27 pol | IPS | 165Hz | HDR");
    strcpy(catalogo[count].badge, "Popular");
    count++;

    // 6. Armazenamento
    catalogo[count].id = 6;
    strcpy(catalogo[count].nome, "SSD NVMe M.2 2TB Gen4 7000MB/s");
    strcpy(catalogo[count].categoria, "Armazenamento");
    catalogo[count].preco = 899.90;
    catalogo[count].preco_antigo = 1099.00;
    catalogo[count].estoque = 10;
    catalogo[count].estoque_minimo = 3;
    strcpy(catalogo[count].specs, "PCIe 4.0 | Leitura 7000MB/s");
    strcpy(catalogo[count].badge, "Alta Velocidade");
    count++;

    return count;
}

// ============================================================================
// 4. PERSISTÊNCIA EM ARQUIVOS (FILE*)
// ============================================================================

int salvar_produtos_arquivo(Produto catalogo[], int total) {
    FILE *arquivo = fopen(ARQUIVO_PRODUTOS, "w");
    if (arquivo == NULL) {
        printf("[!] Erro crítico: Não foi possível abrir %s para gravação.\n", ARQUIVO_PRODUTOS);
        return 0;
    }

    fprintf(arquivo, "# ID;NOME;CATEGORIA;PRECO;PRECO_ANTIGO;ESTOQUE;ESTOQUE_MINIMO;SPECS;BADGE\n");
    for (int i = 0; i < total; i++) {
        fprintf(arquivo, "%d;%s;%s;%.2f;%.2f;%d;%d;%s;%s\n",
                catalogo[i].id,
                catalogo[i].nome,
                catalogo[i].categoria,
                catalogo[i].preco,
                catalogo[i].preco_antigo,
                catalogo[i].estoque,
                catalogo[i].estoque_minimo,
                catalogo[i].specs,
                catalogo[i].badge);
    }

    fclose(arquivo);
    return 1;
}

int carregar_produtos_arquivo(Produto catalogo[]) {
    FILE *arquivo = fopen(ARQUIVO_PRODUTOS, "r");
    if (arquivo == NULL) {
        int total = inicializar_catalogo_padrao(catalogo);
        salvar_produtos_arquivo(catalogo, total);
        return total;
    }

    char linha[512];
    int count = 0;

    // Pula o cabeçalho
    fgets(linha, sizeof(linha), arquivo);

    while (fgets(linha, sizeof(linha), arquivo) != NULL && count < MAX_PRODUTOS) {
        linha[strcspn(linha, "\r\n")] = '\0';
        if (strlen(linha) == 0) continue;

        char *token = strtok(linha, ";");
        if (!token) continue;
        catalogo[count].id = atoi(token);

        token = strtok(NULL, ";");
        if (token) strcpy(catalogo[count].nome, token);

        token = strtok(NULL, ";");
        if (token) strcpy(catalogo[count].categoria, token);

        token = strtok(NULL, ";");
        if (token) catalogo[count].preco = atof(token);

        token = strtok(NULL, ";");
        if (token) catalogo[count].preco_antigo = atof(token);
        else catalogo[count].preco_antigo = catalogo[count].preco * 1.15;

        token = strtok(NULL, ";");
        if (token) catalogo[count].estoque = atoi(token);

        token = strtok(NULL, ";");
        if (token) catalogo[count].estoque_minimo = atoi(token);

        token = strtok(NULL, ";");
        if (token) strcpy(catalogo[count].specs, token);
        else strcpy(catalogo[count].specs, "Hardware de Alta Performance");

        token = strtok(NULL, ";");
        if (token) strcpy(catalogo[count].badge, token);
        else strcpy(catalogo[count].badge, "Normal");

        count++;
    }

    fclose(arquivo);
    return count;
}

int gravar_pedido_arquivo(Pedido *p, Produto catalogo[], int total_produtos) {
    FILE *arquivo = fopen(ARQUIVO_PEDIDOS, "a+");
    if (arquivo == NULL) return 0;

    fprintf(arquivo, "======================================================================\n");
    fprintf(arquivo, "COMPROVANTE DE PEDIDO: %s\n", p->codigo_pedido);
    fprintf(arquivo, "DATA / HORA          : %s\n", p->data_hora);
    fprintf(arquivo, "CLIENTE              : %s (CPF: %s)\n", p->cliente.nome, p->cliente.cpf);
    if (strlen(p->cliente.endereco) > 0) {
        fprintf(arquivo, "ENDEREÇO DE ENTREGA  : %s\n", p->cliente.endereco);
    }
    fprintf(arquivo, "PAGAMENTO            : %s\n", p->forma_pagamento);
    fprintf(arquivo, "----------------------------------------------------------------------\n");
    fprintf(arquivo, "ITENS DO PEDIDO:\n");

    for (int i = 0; i < p->total_itens; i++) {
        int idx = -1;
        for (int j = 0; j < total_produtos; j++) {
            if (catalogo[j].id == p->itens[i].id_produto) {
                idx = j;
                break;
            }
        }
        char nome_prod[80] = "Item";
        if (idx != -1) strcpy(nome_prod, catalogo[idx].nome);

        fprintf(arquivo, "  - %dx %-38s (Un: R$ %8.2f | Subtotal: R$ %8.2f)\n",
                p->itens[i].quantidade,
                nome_prod,
                p->itens[i].preco_unitario,
                p->itens[i].subtotal);
    }

    fprintf(arquivo, "----------------------------------------------------------------------\n");
    fprintf(arquivo, "SUBTOTAL BRUTO       : R$ %.2f\n", p->valor_bruto);
    fprintf(arquivo, "DESCONTO APLICADO    : R$ %.2f\n", p->desconto);
    fprintf(arquivo, "TOTAL FINAL FATURADO : R$ %.2f\n", p->valor_total);
    fprintf(arquivo, "STATUS               : APROVADO / ESTOQUE ATUALIZADO\n");
    fprintf(arquivo, "======================================================================\n\n");

    fclose(arquivo);
    return 1;
}

void listar_pedidos_arquivo() {
    FILE *arquivo = fopen(ARQUIVO_PEDIDOS, "r");
    if (arquivo == NULL) {
        printf("\n======================================================================\n");
        printf("                     HISTÓRICO DE PEDIDOS EM DISCO                    \n");
        printf("======================================================================\n");
        printf("[-] Nenhum pedido registrado ainda no arquivo '%s'.\n", ARQUIVO_PEDIDOS);
        return;
    }

    printf("\n======================================================================\n");
    printf("         HISTÓRICO DE PEDIDOS GRAVADOS EM DISCO (%s)\n", ARQUIVO_PEDIDOS);
    printf("======================================================================\n");

    char linha[256];
    int leu_algo = 0;
    while (fgets(linha, sizeof(linha), arquivo) != NULL) {
        printf("%s", linha);
        leu_algo = 1;
    }

    if (!leu_algo) {
        printf("O arquivo de pedidos existe, mas está vazio no momento.\n");
    }

    fclose(arquivo);
}

// ============================================================================
// 5. APRESENTAÇÃO E CONSULTA EM TERMINAL
// ============================================================================

void listar_catalogo(Produto catalogo[], int total) {
    printf("\n===============================================================================================\n");
    printf("                       CATÁLOGO DE PRODUTOS E NÍVEIS DE ESTOQUE                                \n");
    printf("===============================================================================================\n");
    printf("| %-4s | %-36s | %-16s | %-12s | %-10s |\n", "ID", "DESCRIÇÃO DO PRODUTO", "CATEGORIA", "PREÇO (R$)", "ESTOQUE");
    printf("-----------------------------------------------------------------------------------------------\n");

    for (int i = 0; i < total; i++) {
        char status_estoque[20] = "";
        if (catalogo[i].estoque <= catalogo[i].estoque_minimo) {
            strcpy(status_estoque, " (! BAIXO)");
        }

        printf("| %-4d | %-36s | %-16s | R$ %9.2f | %4d un%-9s |\n",
               catalogo[i].id,
               catalogo[i].nome,
               catalogo[i].categoria,
               catalogo[i].preco,
               catalogo[i].estoque,
               status_estoque);
    }
    printf("===============================================================================================\n");
}

int buscar_produto_por_id(Produto catalogo[], int total, int id_procurado) {
    for (int i = 0; i < total; i++) {
        if (catalogo[i].id == id_procurado) {
            return i;
        }
    }
    return -1;
}

void menu_consultar_produto(Produto catalogo[], int total) {
    int id_digitado;
    printf("\n--- CONSULTA DE PRODUTO POR CÓDIGO ---\n");
    printf("Informe o ID do produto desejado: ");
    if (scanf("%d", &id_digitado) != 1) {
        printf("[!] Erro: Digite um número válido.\n");
        limpar_buffer();
        return;
    }

    int indice = buscar_produto_por_id(catalogo, total, id_digitado);

    if (indice != -1) {
        printf("\n================ DETALHES DO PRODUTO ================\n");
        printf(" Código (ID)       : %d\n", catalogo[indice].id);
        printf(" Nome Comercial    : %s\n", catalogo[indice].nome);
        printf(" Categoria         : %s\n", catalogo[indice].categoria);
        printf(" Preço Unitário    : R$ %.2f\n", catalogo[indice].preco);
        printf(" Estoque Físico    : %d unidades\n", catalogo[indice].estoque);
        printf(" Estoque Mínimo    : %d unidades (limite de alerta)\n", catalogo[indice].estoque_minimo);
        if (catalogo[indice].estoque <= catalogo[indice].estoque_minimo) {
            printf(" Situação          : [ALERTA] Estoque crítico! Necessita reposição.\n");
        } else {
            printf(" Situação          : [OK] Saldo de estoque regular.\n");
        }
        printf("======================================================\n");
    } else {
        printf("\n[-] Produto com ID %d não foi encontrado no catálogo!\n", id_digitado);
    }
}

void relatorio_gerencial(Produto catalogo[], int total_produtos, int contador_pedidos, double faturamento_total) {
    printf("\n======================================================================\n");
    printf("              CYBERHARDWARE - RELATÓRIO GERENCIAL DE VENDAS           \n");
    printf("======================================================================\n");
    printf(" Pedidos Concluídos nesta Sessão : %d pedido(s)\n", contador_pedidos);
    printf(" Faturamento Bruto Acumulado     : R$ %.2f\n", faturamento_total);
    printf(" Requisições Web Atendidas       : %d chamadas HTTP\n", g_requisicoes_atendidas);
    printf("----------------------------------------------------------------------\n");
    printf(" AUDITORIA DE REPOSIÇÃO (PRODUTOS COM ESTOQUE CRÍTICO):\n");

    int itens_criticos = 0;
    for (int i = 0; i < total_produtos; i++) {
        if (catalogo[i].estoque <= catalogo[i].estoque_minimo) {
            printf(" [!] ALERTA: [%d] %-36s -> Saldo: %d un (Mínimo: %d un)\n",
                   catalogo[i].id,
                   catalogo[i].nome,
                   catalogo[i].estoque,
                   catalogo[i].estoque_minimo);
            itens_criticos++;
        }
    }

    if (itens_criticos == 0) {
        printf(" [OK] Nenhum produto com estoque crítico no momento! Todos acima do mínimo.\n");
    } else {
        printf(" Total de itens que necessitam de ordem de compra urgente: %d\n", itens_criticos);
    }
    printf("======================================================================\n");
}

// ============================================================================
// 6. REGRAS DE VENDAS E BAIXA ATÔMICA DE ESTOQUE
// ============================================================================

double aplicar_cupom_desconto(const char *cupom, double valor_bruto) {
    if (strcmp(cupom, "TECH10") == 0) {
        return valor_bruto * 0.10;
    } else if (strcmp(cupom, "CYBER20") == 0) {
        return valor_bruto * 0.20;
    }
    return 0.0;
}

void menu_realizar_venda(Produto catalogo[], int total_produtos, int *contador_pedidos, double *faturamento_total) {
    ItemCarrinho carrinho[MAX_ITENS_CARRINHO];
    int total_itens_carrinho = 0;
    char continuar = 'S';

    printf("\n======================================================================\n");
    printf("         CYBERHARDWARE - NOVA VENDA / CHECKOUT DE PEDIDO              \n");
    printf("======================================================================\n");

    listar_catalogo(catalogo, total_produtos);

    do {
        int id_escolhido, quantidade;
        printf("\n[Item %d do Carrinho]\n", total_itens_carrinho + 1);
        printf("Informe o ID do produto a adicionar: ");
        if (scanf("%d", &id_escolhido) != 1) {
            printf("[!] Entrada inválida.\n");
            limpar_buffer();
            continue;
        }

        int indice = buscar_produto_por_id(catalogo, total_produtos, id_escolhido);
        if (indice == -1) {
            printf("[-] Produto com ID %d inexistente! Tente novamente.\n", id_escolhido);
            continue;
        }

        printf("Produto: %s (Saldo atual: %d un | R$ %.2f)\n", 
               catalogo[indice].nome, catalogo[indice].estoque, catalogo[indice].preco);

        printf("Quantidade desejada: ");
        if (scanf("%d", &quantidade) != 1 || quantidade <= 0) {
            printf("[!] Quantidade inválida! Deve ser maior que 0.\n");
            limpar_buffer();
            continue;
        }

        if (quantidade > catalogo[indice].estoque) {
            printf("[!] ERRO: Estoque insuficiente! Solicitado: %d, Disponível: %d.\n", 
                   quantidade, catalogo[indice].estoque);
            continue;
        }

        carrinho[total_itens_carrinho].id_produto = id_escolhido;
        carrinho[total_itens_carrinho].quantidade = quantidade;
        carrinho[total_itens_carrinho].preco_unitario = catalogo[indice].preco;
        carrinho[total_itens_carrinho].subtotal = quantidade * catalogo[indice].preco;
        total_itens_carrinho++;

        printf("[+] %d un de '%s' adicionadas com sucesso!\n", quantidade, catalogo[indice].nome);

        if (total_itens_carrinho >= MAX_ITENS_CARRINHO) {
            printf("[!] Limite máximo de itens por venda atingido (%d itens).\n", MAX_ITENS_CARRINHO);
            break;
        }

        printf("\nDeseja adicionar outro produto ao carrinho? (S/N): ");
        limpar_buffer();
        scanf("%c", &continuar);

    } while ((continuar == 'S' || continuar == 's') && total_itens_carrinho < MAX_ITENS_CARRINHO);

    if (total_itens_carrinho == 0) {
        printf("\nNenhum item foi adicionado. Venda cancelada.\n");
        return;
    }

    double valor_bruto = 0.0;
    for (int i = 0; i < total_itens_carrinho; i++) {
        valor_bruto += carrinho[i].subtotal;
    }

    printf("\n---------------- RESUMO DO CARRINHO ----------------\n");
    for (int i = 0; i < total_itens_carrinho; i++) {
        int idx = buscar_produto_por_id(catalogo, total_produtos, carrinho[i].id_produto);
        printf(" %dx %-35s - R$ %9.2f\n", 
               carrinho[i].quantidade, catalogo[idx].nome, carrinho[i].subtotal);
    }
    printf(" Subtotal Bruto: R$ %.2f\n", valor_bruto);

    char cupom[20] = "";
    printf("\nPossui cupom de desconto? (Ex: TECH10, CYBER20 ou Enter para pular): ");
    limpar_buffer();
    if (fgets(cupom, sizeof(cupom), stdin) != NULL) {
        cupom[strcspn(cupom, "\r\n")] = '\0';
    }

    double desconto = aplicar_cupom_desconto(cupom, valor_bruto);
    if (desconto > 0.0) {
        printf("[OK] Cupom '%s' aplicado com sucesso! Desconto de R$ %.2f\n", cupom, desconto);
    } else if (strlen(cupom) > 0) {
        printf("[!] Cupom '%s' inválido. Nenhum desconto aplicado.\n", cupom);
    }

    double valor_liquido = valor_bruto - desconto;
    printf(" TOTAL A PAGAR: R$ %.2f\n", valor_liquido);

    Pedido novo_pedido;
    memset(&novo_pedido, 0, sizeof(Pedido));
    printf("\n------------- DADOS DO COMPRADOR -------------\n");
    printf("Nome Completo: ");
    if (fgets(novo_pedido.cliente.nome, sizeof(novo_pedido.cliente.nome), stdin) != NULL) {
        novo_pedido.cliente.nome[strcspn(novo_pedido.cliente.nome, "\r\n")] = '\0';
    }
    printf("CPF: ");
    if (fgets(novo_pedido.cliente.cpf, sizeof(novo_pedido.cliente.cpf), stdin) != NULL) {
        novo_pedido.cliente.cpf[strcspn(novo_pedido.cliente.cpf, "\r\n")] = '\0';
    }

    int opcao_pagamento = 1;
    strcpy(novo_pedido.forma_pagamento, "PIX");
    printf("\nForma de Pagamento: [1] PIX | [2] Cartão de Crédito | [3] Boleto: ");
    if (scanf("%d", &opcao_pagamento) == 1) {
        if (opcao_pagamento == 2) strcpy(novo_pedido.forma_pagamento, "Cartão de Crédito");
        else if (opcao_pagamento == 3) strcpy(novo_pedido.forma_pagamento, "Boleto Bancário");
        else strcpy(novo_pedido.forma_pagamento, "PIX");
    }

    // Baixa Atômica: Fase 1 (Validação preventiva)
    int estoque_valido = 1;
    for (int i = 0; i < total_itens_carrinho; i++) {
        int idx = buscar_produto_por_id(catalogo, total_produtos, carrinho[i].id_produto);
        if (catalogo[idx].estoque < carrinho[i].quantidade) {
            printf("\n[!] FALHA NA TRANSAÇÃO: O produto '%s' não tem mais saldo suficiente!\n", catalogo[idx].nome);
            estoque_valido = 0;
            break;
        }
    }

    if (!estoque_valido) {
        printf("[-] Venda abortada por segurança. Nenhum estoque foi corrompido.\n");
        return;
    }

    // Baixa Atômica: Fase 2 (Baixa real)
    for (int i = 0; i < total_itens_carrinho; i++) {
        int idx = buscar_produto_por_id(catalogo, total_produtos, carrinho[i].id_produto);
        catalogo[idx].estoque -= carrinho[i].quantidade;
    }

    salvar_produtos_arquivo(catalogo, total_produtos);

    (*contador_pedidos)++;
    *faturamento_total += valor_liquido;

    novo_pedido.id_pedido = *contador_pedidos;
    sprintf(novo_pedido.codigo_pedido, "PED-%05d", *contador_pedidos);
    obter_data_hora_atual(novo_pedido.data_hora, sizeof(novo_pedido.data_hora));
    novo_pedido.total_itens = total_itens_carrinho;
    for (int i = 0; i < total_itens_carrinho; i++) {
        novo_pedido.itens[i] = carrinho[i];
    }
    novo_pedido.valor_bruto = valor_bruto;
    novo_pedido.desconto = desconto;
    novo_pedido.valor_total = valor_liquido;

    gravar_pedido_arquivo(&novo_pedido, catalogo, total_produtos);

    printf("\n======================================================================\n");
    printf("                   COMPROVANTE DE PEDIDO / VENDA CONCLUÍDA            \n");
    printf("======================================================================\n");
    printf(" Código do Pedido  : %s\n", novo_pedido.codigo_pedido);
    printf(" Data / Hora       : %s\n", novo_pedido.data_hora);
    printf(" Cliente           : %s (CPF: %s)\n", novo_pedido.cliente.nome, novo_pedido.cliente.cpf);
    printf(" Pagamento         : %s\n", novo_pedido.forma_pagamento);
    printf(" Itens Comprados   : %d produto(s)\n", total_itens_carrinho);
    printf(" Subtotal Bruto    : R$ %.2f\n", valor_bruto);
    printf(" Desconto          : R$ %.2f\n", desconto);
    printf(" TOTAL FATURADO    : R$ %.2f\n", valor_liquido);
    printf(" Situação          : [APROVADO] Gravado com sucesso em %s!\n", ARQUIVO_PEDIDOS);
    printf("======================================================================\n");
}

// ============================================================================
// 7. MÓDULO WEB SOCKETS: SERVIDOR HTTP NATIVO WINSOCK2
// ============================================================================

// Serializa o catálogo de produtos em formato JSON
void gerar_json_produtos(Produto catalogo[], int total, char *buffer_saida, size_t max_len) {
    strcpy(buffer_saida, "[");
    for (int i = 0; i < total; i++) {
        char item[512];
        sprintf(item,
                "{\"id\":%d,\"nome\":\"%s\",\"categoria\":\"%s\",\"preco\":%.2f,\"precoAntigo\":%.2f,\"estoque\":%d,\"specs\":\"%s\",\"badge\":\"%s\"}%s",
                catalogo[i].id,
                catalogo[i].nome,
                catalogo[i].categoria,
                catalogo[i].preco,
                catalogo[i].preco_antigo,
                catalogo[i].estoque,
                catalogo[i].specs,
                catalogo[i].badge,
                (i < total - 1) ? "," : "");
        strcat(buffer_saida, item);
    }
    strcat(buffer_saida, "]");
}

// Processa o checkout recebido via POST /api/checkout em JSON
int processar_checkout_web_json(const char *json_body, Produto catalogo[], int total_produtos,
                               int *contador_pedidos, double *faturamento_total, char *resposta_json) {
    Pedido p;
    memset(&p, 0, sizeof(Pedido));

    // Extrai dados básicos do cliente do JSON
    const char *pos_nome = strstr(json_body, "\"nome\":\"");
    if (pos_nome) {
        sscanf(pos_nome + 8, "%79[^\"]", p.cliente.nome);
    } else {
        strcpy(p.cliente.nome, "Cliente Web");
    }

    const char *pos_cpf = strstr(json_body, "\"cpf\":\"");
    if (pos_cpf) {
        sscanf(pos_cpf + 7, "%15[^\"]", p.cliente.cpf);
    } else {
        strcpy(p.cliente.cpf, "000.000.000-00");
    }

    const char *pos_end = strstr(json_body, "\"endereco\":\"");
    if (pos_end) {
        sscanf(pos_end + 12, "%119[^\"]", p.cliente.endereco);
    }

    const char *pos_pg = strstr(json_body, "\"formaPagamento\":\"");
    if (pos_pg) {
        sscanf(pos_pg + 18, "%29[^\"]", p.forma_pagamento);
    } else {
        strcpy(p.forma_pagamento, "PIX");
    }

    char cupom[32] = "";
    const char *pos_cupom = strstr(json_body, "\"cupom\":\"");
    if (pos_cupom) {
        sscanf(pos_cupom + 9, "%31[^\"]", cupom);
    }

    // Parser simples dos itens do carrinho: "id":X,"quantidade":Y
    const char *ptr = json_body;
    while ((ptr = strstr(ptr, "\"id\":")) != NULL && p.total_itens < MAX_ITENS_CARRINHO) {
        int id_prod = 0, qtd = 0;
        if (sscanf(ptr + 5, "%d", &id_prod) == 1) {
            const char *ptr_qtd = strstr(ptr, "\"quantidade\":");
            if (ptr_qtd && sscanf(ptr_qtd + 13, "%d", &qtd) == 1) {
                int idx = buscar_produto_por_id(catalogo, total_produtos, id_prod);
                if (idx != -1 && qtd > 0) {
                    p.itens[p.total_itens].id_produto = id_prod;
                    p.itens[p.total_itens].quantidade = qtd;
                    p.itens[p.total_itens].preco_unitario = catalogo[idx].preco;
                    p.itens[p.total_itens].subtotal = qtd * catalogo[idx].preco;
                    p.valor_bruto += p.itens[p.total_itens].subtotal;
                    p.total_itens++;
                }
            }
        }
        ptr += 5; // Avança para o próximo item
    }

    if (p.total_itens == 0) {
        sprintf(resposta_json, "{\"erro\":\"Carrinho vazio ou itens inválidos\"}");
        return 0;
    }

    // Baixa Atômica - Fase 1: Validação preventiva
    for (int i = 0; i < p.total_itens; i++) {
        int idx = buscar_produto_por_id(catalogo, total_produtos, p.itens[i].id_produto);
        if (catalogo[idx].estoque < p.itens[i].quantidade) {
            sprintf(resposta_json, "{\"erro\":\"Estoque insuficiente para o produto '%s'\"}", catalogo[idx].nome);
            return 0;
        }
    }

    // Baixa Atômica - Fase 2: Decremento definitivo
    for (int i = 0; i < p.total_itens; i++) {
        int idx = buscar_produto_por_id(catalogo, total_produtos, p.itens[i].id_produto);
        catalogo[idx].estoque -= p.itens[i].quantidade;
    }

    // Salva estoque atualizado no arquivo
    salvar_produtos_arquivo(catalogo, total_produtos);

    p.desconto = aplicar_cupom_desconto(cupom, p.valor_bruto);
    p.valor_total = p.valor_bruto - p.desconto;

    (*contador_pedidos)++;
    *faturamento_total += p.valor_total;
    p.id_pedido = *contador_pedidos;
    sprintf(p.codigo_pedido, "PED-%05d", *contador_pedidos);
    obter_data_hora_atual(p.data_hora, sizeof(p.data_hora));

    gravar_pedido_arquivo(&p, catalogo, total_produtos);

    sprintf(resposta_json,
            "{\"status\":\"ok\",\"codigo\":\"%s\",\"dataHora\":\"%s\",\"total\":\"R$ %.2f\",\"pagamento\":\"%s\"}",
            p.codigo_pedido, p.data_hora, p.valor_total, p.forma_pagamento);

    return 1;
}

// Inicia o Servidor HTTP na porta 8080 com Winsock2
void iniciar_servidor_web(Produto catalogo[], int total_produtos, int *contador_pedidos, double *faturamento_total) {
    WSADATA wsa;
    SOCKET server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    int client_addr_len = sizeof(client_addr);

    printf("\n======================================================================\n");
    printf("         CYBERHARDWARE - SERVIDOR WEB NATIVO EM C (WINSOCK2)          \n");
    printf("======================================================================\n");
    printf("[+] Inicializando biblioteca Windows Sockets (ws2_32.lib)...\n");

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("[-] Falha ao inicializar o Winsock. Erro: %d\n", WSAGetLastError());
        pausar_tela();
        return;
    }

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == INVALID_SOCKET) {
        printf("[-] Falha ao criar o socket TCP. Erro: %d\n", WSAGetLastError());
        WSACleanup();
        pausar_tela();
        return;
    }

    // Configura reuso de endereço
    BOOL opt = TRUE;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORTA_SERVIDOR);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        printf("[-] Falha no bind na porta %d. Verifique se a porta já está em uso.\n", PORTA_SERVIDOR);
        closesocket(server_fd);
        WSACleanup();
        pausar_tela();
        return;
    }

    if (listen(server_fd, 10) == SOCKET_ERROR) {
        printf("[-] Falha no listen do socket.\n");
        closesocket(server_fd);
        WSACleanup();
        pausar_tela();
        return;
    }

    // Configura o socket para não travar indefinidamente (timeout de 500ms)
    DWORD timeout = 500;
    setsockopt(server_fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));

    printf("[+] Servidor HTTP escutando com sucesso na porta %d!\n", PORTA_SERVIDOR);
    printf("[+] Endereço local da CyberStore: http://localhost:%d\n", PORTA_SERVIDOR);
    printf("----------------------------------------------------------------------\n");
    printf(" [!] Pressione a tecla [Q] ou [ESC] no terminal para encerrar o servidor.\n");
    printf("----------------------------------------------------------------------\n\n");

    // Abre o navegador padrão automaticamente
    ShellExecuteA(NULL, "open", "http://localhost:8080", NULL, NULL, SW_SHOWNORMAL);

    char *buffer_req = (char *)malloc(BUFFER_HTTP);
    char *buffer_resp = (char *)malloc(BUFFER_HTTP * 2);

    while (1) {
        // Verifica se o usuário pressionou uma tecla para sair
        if (_kbhit()) {
            int tecla = _getch();
            if (tecla == 'q' || tecla == 'Q' || tecla == 27) { // 27 = ESC
                printf("\n[INFO] Comando de encerramento detectado. Desligando o servidor web...\n");
                break;
            }
        }

        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_fd == INVALID_SOCKET) {
            // Timeout normal para checar _kbhit()
            continue;
        }

        int bytes_recebidos = recv(client_fd, buffer_req, BUFFER_HTTP - 1, 0);
        if (bytes_recebidos > 0) {
            buffer_req[bytes_recebidos] = '\0';
            g_requisicoes_atendidas++;

            char metodo[16], rota[128];
            sscanf(buffer_req, "%15s %127s", metodo, rota);

            printf("[HTTP %s] Rota requisitada: %s\n", metodo, rota);

            // ROTA 1: Entrega da Página Inicial (GET / ou GET /index.html)
            if (strcmp(rota, "/") == 0 || strcmp(rota, "/index.html") == 0) {
                FILE *f_html = fopen(ARQUIVO_HTML, "rb");
                if (f_html) {
                    fseek(f_html, 0, SEEK_END);
                    long fsize = ftell(f_html);
                    fseek(f_html, 0, SEEK_SET);

                    char *html_content = (char *)malloc(fsize + 1);
                    fread(html_content, 1, fsize, f_html);
                    html_content[fsize] = '\0';
                    fclose(f_html);

                    sprintf(buffer_resp,
                            "HTTP/1.1 200 OK\r\n"
                            "Content-Type: text/html; charset=utf-8\r\n"
                            "Content-Length: %ld\r\n"
                            "Connection: close\r\n\r\n",
                            fsize);

                    send(client_fd, buffer_resp, strlen(buffer_resp), 0);
                    send(client_fd, html_content, fsize, 0);
                    free(html_content);
                } else {
                    const char *erro404 = "HTTP/1.1 404 Not Found\r\nContent-Length: 26\r\n\r\nArquivo HTML nao encontrado";
                    send(client_fd, erro404, strlen(erro404), 0);
                }
            }
            // ROTA 2: API de Produtos (GET /api/produtos)
            else if (strcmp(rota, "/api/produtos") == 0) {
                char *json_produtos = (char *)malloc(8192);
                gerar_json_produtos(catalogo, total_produtos, json_produtos, 8192);

                sprintf(buffer_resp,
                        "HTTP/1.1 200 OK\r\n"
                        "Content-Type: application/json; charset=utf-8\r\n"
                        "Access-Control-Allow-Origin: *\r\n"
                        "Content-Length: %zu\r\n"
                        "Connection: close\r\n\r\n%s",
                        strlen(json_produtos), json_produtos);

                send(client_fd, buffer_resp, strlen(buffer_resp), 0);
                free(json_produtos);
            }
            // ROTA 3: API de Checkout e Baixa Atômica (POST /api/checkout)
            else if (strcmp(rota, "/api/checkout") == 0 && strcmp(metodo, "POST") == 0) {
                const char *corpo_json = strstr(buffer_req, "\r\n\r\n");
                if (corpo_json) corpo_json += 4;
                else corpo_json = "";

                char resposta_checkout[512] = "";
                int ok = processar_checkout_web_json(corpo_json, catalogo, total_produtos,
                                                    contador_pedidos, faturamento_total, resposta_checkout);

                sprintf(buffer_resp,
                        "HTTP/1.1 %s\r\n"
                        "Content-Type: application/json; charset=utf-8\r\n"
                        "Access-Control-Allow-Origin: *\r\n"
                        "Content-Length: %zu\r\n"
                        "Connection: close\r\n\r\n%s",
                        ok ? "200 OK" : "400 Bad Request",
                        strlen(resposta_checkout), resposta_checkout);

                send(client_fd, buffer_resp, strlen(buffer_resp), 0);

                if (ok) {
                    printf("  -> [CHECKOUT APROVADO] Pedido web gravado com baixa atômica de estoque!\n");
                } else {
                    printf("  -> [CHECKOUT RECUSADO] %s\n", resposta_checkout);
                }
            }
            // ROTA 4: Telemetria do Servidor (GET /api/status)
            else if (strcmp(rota, "/api/status") == 0) {
                char json_status[256];
                sprintf(json_status,
                        "{\"porta\":%d,\"requisicoes\":%d,\"pedidos\":%d,\"faturamento\":%.2f}",
                        PORTA_SERVIDOR, g_requisicoes_atendidas, *contador_pedidos, *faturamento_total);

                sprintf(buffer_resp,
                        "HTTP/1.1 200 OK\r\n"
                        "Content-Type: application/json; charset=utf-8\r\n"
                        "Access-Control-Allow-Origin: *\r\n"
                        "Content-Length: %zu\r\n"
                        "Connection: close\r\n\r\n%s",
                        strlen(json_status), json_status);

                send(client_fd, buffer_resp, strlen(buffer_resp), 0);
            }
            // Rota Não Encontrada
            else {
                const char *resp404 = "HTTP/1.1 404 Not Found\r\nContent-Length: 13\r\n\r\nRota 404";
                send(client_fd, resp404, strlen(resp404), 0);
            }
        }

        closesocket(client_fd);
    }

    free(buffer_req);
    free(buffer_resp);

    closesocket(server_fd);
    WSACleanup();

    printf("[+] Servidor Winsock2 finalizado com sucesso. Retornando ao menu...\n");
    pausar_tela();
}

// ============================================================================
// 8. PROGRAMA PRINCIPAL
// ============================================================================
int main() {
    setlocale(LC_ALL, "Portuguese");

    Produto catalogo[MAX_PRODUTOS];
    int total_produtos = 0;
    int contador_pedidos = 0;
    double faturamento_total = 0.0;
    int opcao = -1;

    // Inicialização do Catálogo (lê produtos_estoque.txt se existir)
    total_produtos = carregar_produtos_arquivo(catalogo);

    do {
        printf("\n");
        printf("======================================================================\n");
        printf("         CYBERHARDWARE E-COMMERCE - SISTEMA DE GESTÃO EM C (PIM II)   \n");
        printf("======================================================================\n");
        printf(" [1] Listar Catálogo Completo de Produtos e Níveis de Estoque\n");
        printf(" [2] Consultar Produto por Código (ID)\n");
        printf(" [3] Realizar Nova Venda / Checkout no Terminal\n");
        printf(" [4] Consultar Histórico de Pedidos Gravados em Disco (FILE*)\n");
        printf(" [5] Relatório Gerencial (Faturamento e Auditoria de Estoque Baixo)\n");
        printf(" [6] Iniciar Servidor Web / Conexão Sockets Winsock2 (Porta %d)\n", PORTA_SERVIDOR);
        printf(" [0] Salvar Dados e Encerrar o Sistema\n");
        printf("----------------------------------------------------------------------\n");
        printf(" Digite a opção desejada: ");

        if (scanf("%d", &opcao) != 1) {
            printf("\n[!] Entrada inválida! Digite um número correspondente ao menu.\n");
            limpar_buffer();
            continue;
        }

        switch (opcao) {
            case 1:
                listar_catalogo(catalogo, total_produtos);
                pausar_tela();
                break;

            case 2:
                menu_consultar_produto(catalogo, total_produtos);
                pausar_tela();
                break;

            case 3:
                menu_realizar_venda(catalogo, total_produtos, &contador_pedidos, &faturamento_total);
                pausar_tela();
                break;

            case 4:
                listar_pedidos_arquivo();
                pausar_tela();
                break;

            case 5:
                relatorio_gerencial(catalogo, total_produtos, contador_pedidos, faturamento_total);
                pausar_tela();
                break;

            case 6:
                iniciar_servidor_web(catalogo, total_produtos, &contador_pedidos, &faturamento_total);
                break;

            case 0:
                salvar_produtos_arquivo(catalogo, total_produtos);
                printf("\nDados gravados com sucesso! Encerrando a CyberHardware... Até logo!\n");
                break;

            default:
                printf("\n[!] Opção desconhecida (%d). Digite uma opção entre 0 e 6.\n", opcao);
                pausar_tela();
                break;
        }

    } while (opcao != 0);

    return 0;
}
