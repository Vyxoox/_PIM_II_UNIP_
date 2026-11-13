#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <locale.h>
#include <time.h>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <windows.h>
    #include <shellapi.h>
    #ifdef _MSC_VER
        #pragma comment(lib, "ws2_32.lib")
        #pragma comment(lib, "shell32.lib")
    #endif
    typedef SOCKET socket_t;
    #define FECHAR_SOCKET(s) closesocket(s)
#else
    #include <unistd.h>
    #include <arpa/inet.h>
    #include <sys/socket.h>
    typedef int socket_t;
    #define INVALID_SOCKET (-1)
    #define SOCKET_ERROR (-1)
    #define FECHAR_SOCKET(s) close(s)
#endif

/* ============================================================================
 * CONSTANTES E CONFIGURACOES DO SISTEMA (ALINHADO A ETAPA 3 - PIM.PDF)
 * ============================================================================ */
#define PORTA_SERVIDOR       8080
#define BUFFER_HTTP          131072
#define MAX_PRODUTOS         60
#define MAX_ITENS_CARRINHO   15
#define MAX_USUARIOS         120
#define MAX_PEDIDOS          200
#define MAX_SESSOES          64
#define SEGUNDOS_SESSAO      14400

#define ARQUIVO_USUARIOS     "usuarios_loja.txt"
#define ARQUIVO_PRODUTOS     "produtos_estoque.txt"
#define ARQUIVO_PEDIDOS      "pedidos_loja.txt"
#define ARQUIVO_RECIBOS      "recibos_pedidos.txt"
#define ARQUIVO_HTML         "web/index.html"

/* ============================================================================
 * ESTRUTURAS DE DADOS (MODELAGEM ALINHADA AO DIAGRAMA DE CLASSES DA ETAPA 3)
 * ============================================================================ */
typedef struct {
    int id;
    char nome[100];
    char cpf[20];
    char email[100];
    char telefone[30];
    char endereco[160];
    char papel[16];          /* "CLIENTE" ou "ADMIN" (Usuario -> Cliente / Administrador) */
    char nivel_acesso[32];   /* "PADRAO" ou "GERENTE_TOTAL" */
    int ativo;               /* 1 = Ativo, 0 = Inativado (RF01 / UC01) */
    char senha_hash[96];     /* "<salt_16hex>$<sha256_64hex>" irreversivel (RNF01) */
} Usuario;

typedef struct {
    int id;
    char nome[100];
    char categoria[48];
    double preco;
    double preco_antigo;
    int estoque;
    int estoque_minimo;
    int ativo;               /* 1 = Ativo no catalogo, 0 = Inativo (UC06) */
    char badge[32];
    char foto[64];           /* Rota local /img/<nome>.svg ou URL de imagem */
    char specs[180];
} Produto;

typedef struct {
    int id_produto;
    char nome_produto[100];
    int quantidade;
    double preco_unitario;
    double subtotal;
} ItemCarrinho;

typedef struct {
    char forma_pagamento[24];    /* PIX, CARTAO, BOLETO */
    char status_transacao[24];   /* APROVADO, PENDENTE, RECUSADO */
    char codigo_autorizacao[32]; /* Codigo retornado pelo Gateway de Pagamento */
} Pagamento;

typedef struct {
    int id_pedido;
    int id_usuario;
    char codigo_pedido[28];
    char nome_cliente[100];
    char cpf_cliente[20];
    char email_cliente[100];
    char telefone_cliente[30];
    char endereco_entrega[160];
    char cep_entrega[16];
    ItemCarrinho itens[MAX_ITENS_CARRINHO];
    int total_itens;
    double valor_bruto;
    double desconto;
    double valor_frete;
    double valor_total;
    Pagamento pagamento;
    char status_pedido[32];        /* Pagamento Aprovado, Em Separacao, Enviado, Entregue */
    char codigo_rastreamento[32];  /* Ex: CHW849201BR (RF06 / UC08) */
    char data_hora[28];
} Pedido;

typedef struct {
    char token[65];
    int id_usuario;
    time_t expira_em;
} SessaoAtiva;

/* ============================================================================
 * ESTADO GLOBAL EM MEMORIA (SIMULACAO DA CAMADA MODEL / DATABASE)
 * ============================================================================ */
static Usuario g_usuarios[MAX_USUARIOS];
static int g_total_usuarios = 0;

static Produto g_produtos[MAX_PRODUTOS];
static int g_total_produtos = 0;

static Pedido g_pedidos[MAX_PEDIDOS];
static int g_total_pedidos = 0;

static SessaoAtiva g_sessoes[MAX_SESSOES];
static unsigned long g_contador_seq = 1;
static int g_porta_preferida = PORTA_SERVIDOR;

/* ============================================================================
 * IMPLEMENTACAO SHA-256 PURA EM C (FIPS 180-4) PARA HASH DE SENHAS (RNF01)
 * ============================================================================ */
#define ROTR32(x, n) (((x) >> (n)) | ((x) << (32 - (n))))

static const uint32_t SHA256_K[64] = {
    0x428a2f98u, 0x71374491u, 0xb5c0fbcfu, 0xe9b5dba5u, 0x3956c25bu, 0x59f111f1u, 0x923f82a4u, 0xab1c5ed5u,
    0xd807aa98u, 0x12835b01u, 0x243185beu, 0x550c7dc3u, 0x72be5d74u, 0x80deb1feu, 0x9bdc06a7u, 0xc19bf174u,
    0xe49b69c1u, 0xefbe4786u, 0x0fc19dc6u, 0x240ca1ccu, 0x2de92c6fu, 0x4a7484aau, 0x5cb0a9dcu, 0x76f988dau,
    0x983e5152u, 0xa831c66du, 0xb00327c8u, 0xbf597fc7u, 0xc6e00bf3u, 0xd5a79147u, 0x06ca6351u, 0x14292967u,
    0x27b70a85u, 0x2e1b2138u, 0x4d2c6dfcu, 0x53380d13u, 0x650a7354u, 0x766a0abbu, 0x81c2c92eu, 0x92722c85u,
    0xa2bfe8a1u, 0xa81a664bu, 0xc24b8b70u, 0xc76c51a3u, 0xd192e819u, 0xd6990624u, 0xf40e3585u, 0x106aa070u,
    0x19a4c116u, 0x1e376c08u, 0x2748774cu, 0x34b0bcb5u, 0x391c0cb3u, 0x4ed8aa4au, 0x5b9cca4fu, 0x682e6ff3u,
    0x748f82eeu, 0x78a5636fu, 0x84c87814u, 0x8cc70208u, 0x90befffau, 0xa4506cebu, 0xbef9a3f7u, 0xc67178f2u
};

static void sha256_transform(uint32_t state[8], const uint8_t block[64]) {
    uint32_t w[64];
    for (int i = 0; i < 16; i++) {
        w[i] = ((uint32_t)block[i * 4] << 24) |
               ((uint32_t)block[i * 4 + 1] << 16) |
               ((uint32_t)block[i * 4 + 2] << 8) |
               ((uint32_t)block[i * 4 + 3]);
    }
    for (int i = 16; i < 64; i++) {
        uint32_t s0 = ROTR32(w[i - 15], 7) ^ ROTR32(w[i - 15], 18) ^ (w[i - 15] >> 3);
        uint32_t s1 = ROTR32(w[i - 2], 17) ^ ROTR32(w[i - 2], 19) ^ (w[i - 2] >> 10);
        w[i] = w[i - 16] + s0 + w[i - 7] + s1;
    }

    uint32_t a = state[0], b = state[1], c = state[2], d = state[3];
    uint32_t e = state[4], f = state[5], g = state[6], h = state[7];

    for (int i = 0; i < 64; i++) {
        uint32_t S1 = ROTR32(e, 6) ^ ROTR32(e, 11) ^ ROTR32(e, 25);
        uint32_t ch = (e & f) ^ ((~e) & g);
        uint32_t temp1 = h + S1 + ch + SHA256_K[i] + w[i];
        uint32_t S0 = ROTR32(a, 2) ^ ROTR32(a, 13) ^ ROTR32(a, 22);
        uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
        uint32_t temp2 = S0 + maj;

        h = g; g = f; f = e; e = d + temp1;
        d = c; c = b; b = a; a = temp1 + temp2;
    }

    state[0] += a; state[1] += b; state[2] += c; state[3] += d;
    state[4] += e; state[5] += f; state[6] += g; state[7] += h;
}

/* Calcula o resumo SHA-256 em hexadecimal (64 caracteres + '\0'). */
static void sha256_hex(const char *entrada, char saida_hex[65]) {
    uint32_t state[8] = {
        0x6a09e667u, 0xbb67ae85u, 0x3c6ef372u, 0xa54ff53au,
        0x510e527fu, 0x9b05688cu, 0x1f83d9abu, 0x5be0cd19u
    };
    uint8_t buffer[128];
    size_t len = strlen(entrada);
    size_t offset = 0;

    while (len - offset >= 64) {
        sha256_transform(state, (const uint8_t *)(entrada + offset));
        offset += 64;
    }

    size_t rem = len - offset;
    memset(buffer, 0, sizeof(buffer));
    memcpy(buffer, entrada + offset, rem);
    buffer[rem] = 0x80;

    size_t total_blocks = (rem >= 56) ? 2 : 1;
    uint64_t bit_len = (uint64_t)len * 8ULL;
    size_t len_pos = total_blocks * 64 - 8;
    for (int i = 0; i < 8; i++) {
        buffer[len_pos + i] = (uint8_t)((bit_len >> (56 - i * 8)) & 0xFF);
    }

    sha256_transform(state, buffer);
    if (total_blocks == 2) {
        sha256_transform(state, buffer + 64);
    }

    for (int i = 0; i < 8; i++) {
        snprintf(saida_hex + (i * 8), 9, "%08x", state[i]);
    }
    saida_hex[64] = '\0';
}

/* Gera salt aleatorio hexadecimal de 16 caracteres e grava "<salt>$<sha256>" para RNF01. */
static void gerar_hash_senha(const char *senha_plana, const char *salt_opcional, char destino[96]) {
    char salt[17];
    if (salt_opcional && strlen(salt_opcional) > 0) {
        snprintf(salt, sizeof(salt), "%s", salt_opcional);
    } else {
        uint64_t seed = ((uint64_t)time(NULL) << 20) ^ ((uint64_t)clock() << 8) ^ (g_contador_seq++ * 2654435761ULL);
        #ifdef _WIN32
        seed ^= (uint64_t)GetTickCount64() << 4;
        #endif
        snprintf(salt, sizeof(salt), "%016llx", (unsigned long long)seed);
    }

    char combinado[320];
    char digest[65];
    snprintf(combinado, sizeof(combinado), "CYBERHW:%s:%s", salt, senha_plana);
    sha256_hex(combinado, digest);
    snprintf(destino, 96, "%s$%s", salt, digest);
}

/* Verifica senha em texto contra "<salt>$<sha256>" em tempo constante aproximado. */
static int verificar_hash_senha(const char *senha_plana, const char *hash_armazenado) {
    const char *sep = strchr(hash_armazenado, '$');
    if (!sep) return 0;

    size_t salt_len = (size_t)(sep - hash_armazenado);
    if (salt_len == 0 || salt_len >= 24) return 0;

    char salt[24];
    memcpy(salt, hash_armazenado, salt_len);
    salt[salt_len] = '\0';

    char calculado[96];
    gerar_hash_senha(senha_plana, salt, calculado);

    unsigned char diff = 0;
    size_t len_a = strlen(calculado);
    size_t len_b = strlen(hash_armazenado);
    if (len_a != len_b) return 0;
    for (size_t i = 0; i < len_a; i++) {
        diff |= (unsigned char)(calculado[i] ^ hash_armazenado[i]);
    }
    return diff == 0;
}

/* ============================================================================
 * FUNCOES UTILITARIAS DE STRING, DATA, VALIDACAO E JSON
 * ============================================================================ */
static void copiar_seguro(char *dest, const char *orig, size_t max_tam) {
    if (!dest || max_tam == 0) return;
    if (!orig) {
        dest[0] = '\0';
        return;
    }
    snprintf(dest, max_tam, "%s", orig);
}

/* Remove caracteres ';' e quebras de linha para garantir integridade do formato em disco. */
static void sanitizar_campo(char *texto) {
    if (!texto) return;
    for (size_t i = 0; texto[i] != '\0'; i++) {
        if (texto[i] == ';' || texto[i] == '\r' || texto[i] == '\n' || texto[i] == '|') {
            texto[i] = ' ';
        }
    }
}

static double parse_double_flex(const char *s) {
    if (!s || !*s) return 0.0;
    char buf[64];
    copiar_seguro(buf, s, sizeof(buf));
    for (size_t i = 0; buf[i] != '\0'; i++) {
        if (buf[i] == ',') buf[i] = '.';
    }
    return strtod(buf, NULL);
}

static int comparacao_case_insensitive_prefixo(const char *str, const char *prefixo) {
    while (*prefixo) {
        if (tolower((unsigned char)*str) != tolower((unsigned char)*prefixo)) return 0;
        str++;
        prefixo++;
    }
    return 1;
}

static void obter_data_hora_atual(char *dest, size_t tam) {
    time_t agora = time(NULL);
    struct tm *info = localtime(&agora);
    strftime(dest, tam, "%d/%m/%Y %H:%M:%S", info);
}

static void limpar_buffer_entrada(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
}

static void ler_linha_segura(char *dest, size_t tam) {
    if (fgets(dest, (int)tam, stdin)) {
        dest[strcspn(dest, "\r\n")] = '\0';
    } else if (tam > 0) {
        dest[0] = '\0';
    }
}

static void extrair_apenas_digitos(const char *orig, char *dest, size_t max_tam) {
    size_t j = 0;
    for (size_t i = 0; orig && orig[i] != '\0' && j + 1 < max_tam; i++) {
        if (isdigit((unsigned char)orig[i])) {
            dest[j++] = orig[i];
        }
    }
    dest[j] = '\0';
}

static int validar_cpf_basico(const char *cpf) {
    char digitos[16];
    extrair_apenas_digitos(cpf, digitos, sizeof(digitos));
    return strlen(digitos) == 11;
}

static void json_escapar_string(const char *orig, char *dest, size_t max_tam) {
    size_t j = 0;
    for (size_t i = 0; orig && orig[i] != '\0' && j + 3 < max_tam; i++) {
        unsigned char c = (unsigned char)orig[i];
        if (c == '"' || c == '\\') {
            dest[j++] = '\\';
            dest[j++] = (char)c;
        } else if (c == '\n') {
            dest[j++] = '\\';
            dest[j++] = 'n';
        } else if (c == '\r' || c == '\t') {
            dest[j++] = ' ';
        } else if (c >= 32) {
            dest[j++] = (char)c;
        }
    }
    dest[j] = '\0';
}

static const char* json_encontrar_chave(const char *json, const char *chave) {
    char padrao[96];
    snprintf(padrao, sizeof(padrao), "\"%s\"", chave);
    const char *p = json;
    while ((p = strstr(p, padrao)) != NULL) {
        const char *q = p + strlen(padrao);
        while (*q && isspace((unsigned char)*q)) q++;
        if (*q == ':') {
            q++;
            while (*q && isspace((unsigned char)*q)) q++;
            return q;
        }
        p += strlen(padrao);
    }
    return NULL;
}

static int json_extrair_string(const char *json, const char *chave, char *dest, size_t max_tam) {
    const char *val = json_encontrar_chave(json, chave);
    if (!val || *val != '"') {
        if (max_tam > 0) dest[0] = '\0';
        return 0;
    }
    val++;
    size_t i = 0;
    while (*val && *val != '"' && i + 1 < max_tam) {
        if (*val == '\\' && *(val + 1) != '\0') {
            val++;
        }
        dest[i++] = *val++;
    }
    dest[i] = '\0';
    return 1;
}

static int json_extrair_int(const char *json, const char *chave, int padrao) {
    const char *val = json_encontrar_chave(json, chave);
    if (!val) return padrao;
    if (*val == '"') val++;
    if (!isdigit((unsigned char)*val) && *val != '-') return padrao;
    return atoi(val);
}

static double json_extrair_double(const char *json, const char *chave, double padrao) {
    const char *val = json_encontrar_chave(json, chave);
    if (!val) return padrao;
    if (*val == '"') val++;
    if (!isdigit((unsigned char)*val) && *val != '-') return padrao;
    return strtod(val, NULL);
}

/* ============================================================================
 * BUSCA E PERSISTENCIA: USUARIOS (ARQUIVO_USUARIOS)
 * ============================================================================ */
static Usuario* buscar_usuario_por_id(int id) {
    for (int i = 0; i < g_total_usuarios; i++) {
        if (g_usuarios[i].id == id) return &g_usuarios[i];
    }
    return NULL;
}

static Usuario* buscar_usuario_por_email(const char *email) {
    for (int i = 0; i < g_total_usuarios; i++) {
        if (_stricmp(g_usuarios[i].email, email) == 0) return &g_usuarios[i];
    }
    return NULL;
}

static void salvar_usuarios_arquivo(void) {
    FILE *fp = fopen(ARQUIVO_USUARIOS, "w");
    if (!fp) return;

    fprintf(fp, "# ID;NOME;CPF;EMAIL;TELEFONE;ENDERECO;PAPEL;NIVEL_ACESSO;ATIVO;SENHA_HASH\n");
    for (int i = 0; i < g_total_usuarios; i++) {
        Usuario *u = &g_usuarios[i];
        sanitizar_campo(u->nome);
        sanitizar_campo(u->cpf);
        sanitizar_campo(u->email);
        sanitizar_campo(u->telefone);
        sanitizar_campo(u->endereco);
        fprintf(fp, "%d;%s;%s;%s;%s;%s;%s;%s;%d;%s\n",
                u->id, u->nome, u->cpf, u->email, u->telefone,
                u->endereco, u->papel, u->nivel_acesso, u->ativo, u->senha_hash);
    }
    fclose(fp);
}

static void inicializar_usuarios_padrao(void) {
    g_total_usuarios = 2;

    g_usuarios[0].id = 1;
    copiar_seguro(g_usuarios[0].nome, "Administrador CyberHardware", sizeof(g_usuarios[0].nome));
    copiar_seguro(g_usuarios[0].cpf, "00000000000", sizeof(g_usuarios[0].cpf));
    copiar_seguro(g_usuarios[0].email, "admin@cyberhardware.com.br", sizeof(g_usuarios[0].email));
    copiar_seguro(g_usuarios[0].telefone, "(11) 99999-0001", sizeof(g_usuarios[0].telefone));
    copiar_seguro(g_usuarios[0].endereco, "Av. Paulista, 1000 - Bela Vista, Sao Paulo/SP", sizeof(g_usuarios[0].endereco));
    copiar_seguro(g_usuarios[0].papel, "ADMIN", sizeof(g_usuarios[0].papel));
    copiar_seguro(g_usuarios[0].nivel_acesso, "GERENTE_TOTAL", sizeof(g_usuarios[0].nivel_acesso));
    g_usuarios[0].ativo = 1;
    gerar_hash_senha("Admin@2026", "a1b2c3d4e5f60708", g_usuarios[0].senha_hash);

    g_usuarios[1].id = 2;
    copiar_seguro(g_usuarios[1].nome, "Cliente Demonstracao", sizeof(g_usuarios[1].nome));
    copiar_seguro(g_usuarios[1].cpf, "12345678901", sizeof(g_usuarios[1].cpf));
    copiar_seguro(g_usuarios[1].email, "cliente@cyberhardware.com.br", sizeof(g_usuarios[1].email));
    copiar_seguro(g_usuarios[1].telefone, "(11) 98888-1234", sizeof(g_usuarios[1].telefone));
    copiar_seguro(g_usuarios[1].endereco, "Rua Vergueiro, 3185 - Vila Mariana, Sao Paulo/SP", sizeof(g_usuarios[1].endereco));
    copiar_seguro(g_usuarios[1].papel, "CLIENTE", sizeof(g_usuarios[1].papel));
    copiar_seguro(g_usuarios[1].nivel_acesso, "PADRAO", sizeof(g_usuarios[1].nivel_acesso));
    g_usuarios[1].ativo = 1;
    gerar_hash_senha("Cliente@2026", "1020304050607080", g_usuarios[1].senha_hash);

    salvar_usuarios_arquivo();
}

static void carregar_usuarios_arquivo(void) {
    FILE *fp = fopen(ARQUIVO_USUARIOS, "r");
    if (!fp) {
        inicializar_usuarios_padrao();
        return;
    }

    char linha[1024];
    g_total_usuarios = 0;
    while (fgets(linha, sizeof(linha), fp) && g_total_usuarios < MAX_USUARIOS) {
        if (linha[0] == '#' || linha[0] == '\n' || linha[0] == '\r') continue;
        linha[strcspn(linha, "\r\n")] = '\0';

        char *cols[10] = {0};
        int c = 0;
        char *p = linha;
        cols[c++] = p;
        while (*p && c < 10) {
            if (*p == ';') {
                *p = '\0';
                cols[c++] = p + 1;
            }
            p++;
        }
        if (c < 10) continue;

        Usuario *u = &g_usuarios[g_total_usuarios];
        u->id = atoi(cols[0]);
        copiar_seguro(u->nome, cols[1], sizeof(u->nome));
        copiar_seguro(u->cpf, cols[2], sizeof(u->cpf));
        copiar_seguro(u->email, cols[3], sizeof(u->email));
        copiar_seguro(u->telefone, cols[4], sizeof(u->telefone));
        copiar_seguro(u->endereco, cols[5], sizeof(u->endereco));
        copiar_seguro(u->papel, cols[6], sizeof(u->papel));
        copiar_seguro(u->nivel_acesso, cols[7], sizeof(u->nivel_acesso));
        u->ativo = atoi(cols[8]);
        copiar_seguro(u->senha_hash, cols[9], sizeof(u->senha_hash));
        g_total_usuarios++;
    }
    fclose(fp);

    if (g_total_usuarios == 0) {
        inicializar_usuarios_padrao();
    }
}

/* ============================================================================
 * BUSCA E PERSISTENCIA: CATALOGO DE PRODUTOS (ARQUIVO_PRODUTOS)
 * ============================================================================ */
static Produto* buscar_produto_por_id(int id) {
    for (int i = 0; i < g_total_produtos; i++) {
        if (g_produtos[i].id == id) return &g_produtos[i];
    }
    return NULL;
}

static void salvar_produtos_arquivo(void) {
    FILE *fp = fopen(ARQUIVO_PRODUTOS, "w");
    if (!fp) return;

    fprintf(fp, "# ID;NOME;CATEGORIA;PRECO;PRECO_ANTIGO;ESTOQUE;ESTOQUE_MINIMO;ATIVO;BADGE;FOTO;SPECS\n");
    for (int i = 0; i < g_total_produtos; i++) {
        Produto *p = &g_produtos[i];
        sanitizar_campo(p->nome);
        sanitizar_campo(p->categoria);
        sanitizar_campo(p->badge);
        sanitizar_campo(p->foto);
        sanitizar_campo(p->specs);
        fprintf(fp, "%d;%s;%s;%.2f;%.2f;%d;%d;%d;%s;%s;%s\n",
                p->id, p->nome, p->categoria, p->preco, p->preco_antigo,
                p->estoque, p->estoque_minimo, p->ativo, p->badge, p->foto, p->specs);
    }
    fclose(fp);
}

static void inicializar_produtos_padrao(void) {
    g_total_produtos = 6;

    Produto catalogo_base[6] = {
        {1, "Placa de Video RTX 4080 Super 16GB GDDR6X", "Placas de Video", 7299.90, 8199.90, 4, 2, 1, "Destaque", "/img/gpu.svg", "16GB GDDR6X, 256-bit, Ray Tracing 3a Gen, DLSS 3.5, HDMI 2.1a"},
        {2, "Processador AMD Ryzen 7 9800X3D 5.2GHz", "Processadores", 3299.90, 3699.90, 6, 3, 1, "Novo", "/img/cpu.svg", "8 Nucleos, 16 Threads, Cache 104MB 3D V-Cache, Socket AM5"},
        {3, "Memoria RAM Kingston Fury Beast 32GB DDR5", "Memorias RAM", 899.90, 1099.90, 8, 3, 1, "Oferta", "/img/ram.svg", "Kit 2x16GB, 6000MT/s, CL30, Dissipador Aluminio Low-Profile"},
        {4, "SSD NVMe WD Black SN850X 2TB PCIe 4.0", "Armazenamento", 1149.90, 1349.90, 5, 2, 1, "Top Vendas", "/img/ssd.svg", "M.2 2280, Leitura 7300 MB/s, Gravacao 6600 MB/s, Cache DRAM"},
        {5, "Placa-Mae ASUS ROG Strix X870E-E Gaming", "Placas-Mae", 3499.90, 3899.90, 3, 2, 1, "Premium", "/img/mobo.svg", "Socket AM5, Wi-Fi 7, PCIe 5.0 x16, VRM 18+2+2 Fases, USB4"},
        {6, "Fonte Corsair RM1000x Shift 1000W 80+ Gold", "Fontes", 1299.90, 1499.90, 5, 2, 1, "Modular", "/img/psu.svg", "Full Modular Lateral, ATX 3.0, Cabo PCIe 5.0 12VHPWR Nativo"}
    };

    memcpy(g_produtos, catalogo_base, sizeof(catalogo_base));
    salvar_produtos_arquivo();
}

static void carregar_produtos_arquivo(void) {
    FILE *fp = fopen(ARQUIVO_PRODUTOS, "r");
    if (!fp) {
        inicializar_produtos_padrao();
        return;
    }

    char linha[1024];
    g_total_produtos = 0;
    while (fgets(linha, sizeof(linha), fp) && g_total_produtos < MAX_PRODUTOS) {
        if (linha[0] == '#' || linha[0] == '\n' || linha[0] == '\r') continue;
        linha[strcspn(linha, "\r\n")] = '\0';

        char *cols[11] = {0};
        int c = 0;
        char *p = linha;
        cols[c++] = p;
        while (*p && c < 11) {
            if (*p == ';') {
                *p = '\0';
                cols[c++] = p + 1;
            }
            p++;
        }

        if (c == 11) {
            Produto *pr = &g_produtos[g_total_produtos++];
            pr->id = atoi(cols[0]);
            copiar_seguro(pr->nome, cols[1], sizeof(pr->nome));
            copiar_seguro(pr->categoria, cols[2], sizeof(pr->categoria));
            pr->preco = parse_double_flex(cols[3]);
            pr->preco_antigo = parse_double_flex(cols[4]);
            pr->estoque = atoi(cols[5]);
            pr->estoque_minimo = atoi(cols[6]);
            pr->ativo = atoi(cols[7]);
            copiar_seguro(pr->badge, cols[8], sizeof(pr->badge));
            copiar_seguro(pr->foto, cols[9], sizeof(pr->foto));
            copiar_seguro(pr->specs, cols[10], sizeof(pr->specs));
        } else if (c == 9) {
            /* Compatibilidade retroativa com esquema de 9 colunas */
            Produto *pr = &g_produtos[g_total_produtos++];
            pr->id = atoi(cols[0]);
            copiar_seguro(pr->nome, cols[1], sizeof(pr->nome));
            copiar_seguro(pr->categoria, cols[2], sizeof(pr->categoria));
            pr->preco = parse_double_flex(cols[3]);
            pr->preco_antigo = parse_double_flex(cols[4]);
            pr->estoque = atoi(cols[5]);
            pr->estoque_minimo = atoi(cols[6]);
            pr->ativo = 1;
            copiar_seguro(pr->badge, cols[7], sizeof(pr->badge));
            copiar_seguro(pr->foto, "/img/default.svg", sizeof(pr->foto));
            copiar_seguro(pr->specs, cols[8], sizeof(pr->specs));
        }
    }
    fclose(fp);

    if (g_total_produtos == 0) {
        inicializar_produtos_padrao();
    } else {
        /* Garante gravacao no formato canonico de 11 colunas */
        salvar_produtos_arquivo();
    }
}

/* ============================================================================
 * PERSISTENCIA DE PEDIDOS E RECIBOS (ARQUIVO_PEDIDOS + ARQUIVO_RECIBOS)
 * ============================================================================ */
static void salvar_pedidos_arquivo(void) {
    FILE *fp = fopen(ARQUIVO_PEDIDOS, "w");
    if (!fp) return;

    fprintf(fp, "# P;ID;ID_USER;COD;NOME;CPF;EMAIL;TEL;END;CEP;BRUTO;DESC;FRETE;TOTAL;PG_FORMA;PG_STATUS;PG_AUTH;STATUS;RASTREIO;DATA\n");
    fprintf(fp, "# I;ID_PROD;NOME_PROD;QTD;PRECO_UNIT;SUBTOTAL\n");
    for (int i = 0; i < g_total_pedidos; i++) {
        Pedido *ped = &g_pedidos[i];
        sanitizar_campo(ped->nome_cliente);
        sanitizar_campo(ped->endereco_entrega);
        fprintf(fp, "P;%d;%d;%s;%s;%s;%s;%s;%s;%s;%.2f;%.2f;%.2f;%.2f;%s;%s;%s;%s;%s;%s\n",
                ped->id_pedido, ped->id_usuario, ped->codigo_pedido,
                ped->nome_cliente, ped->cpf_cliente, ped->email_cliente, ped->telefone_cliente,
                ped->endereco_entrega, ped->cep_entrega,
                ped->valor_bruto, ped->desconto, ped->valor_frete, ped->valor_total,
                ped->pagamento.forma_pagamento, ped->pagamento.status_transacao, ped->pagamento.codigo_autorizacao,
                ped->status_pedido, ped->codigo_rastreamento, ped->data_hora);
        for (int j = 0; j < ped->total_itens; j++) {
            ItemCarrinho *it = &ped->itens[j];
            fprintf(fp, "I;%d;%s;%d;%.2f;%.2f\n",
                    it->id_produto, it->nome_produto, it->quantidade, it->preco_unitario, it->subtotal);
        }
    }
    fclose(fp);
}

static void carregar_pedidos_arquivo(void) {
    FILE *fp = fopen(ARQUIVO_PEDIDOS, "r");
    if (!fp) return;

    char linha[2048];
    g_total_pedidos = 0;
    while (fgets(linha, sizeof(linha), fp)) {
        if (linha[0] == '#' || linha[0] == '\n' || linha[0] == '\r') continue;
        linha[strcspn(linha, "\r\n")] = '\0';

        if (strncmp(linha, "P;", 2) == 0 && g_total_pedidos < MAX_PEDIDOS) {
            char *cols[20] = {0};
            int c = 0;
            char *p = linha;
            cols[c++] = p;
            while (*p && c < 20) {
                if (*p == ';') {
                    *p = '\0';
                    cols[c++] = p + 1;
                }
                p++;
            }
            if (c < 20) continue;

            Pedido *ped = &g_pedidos[g_total_pedidos++];
            memset(ped, 0, sizeof(Pedido));
            ped->id_pedido = atoi(cols[1]);
            ped->id_usuario = atoi(cols[2]);
            copiar_seguro(ped->codigo_pedido, cols[3], sizeof(ped->codigo_pedido));
            copiar_seguro(ped->nome_cliente, cols[4], sizeof(ped->nome_cliente));
            copiar_seguro(ped->cpf_cliente, cols[5], sizeof(ped->cpf_cliente));
            copiar_seguro(ped->email_cliente, cols[6], sizeof(ped->email_cliente));
            copiar_seguro(ped->telefone_cliente, cols[7], sizeof(ped->telefone_cliente));
            copiar_seguro(ped->endereco_entrega, cols[8], sizeof(ped->endereco_entrega));
            copiar_seguro(ped->cep_entrega, cols[9], sizeof(ped->cep_entrega));
            ped->valor_bruto = parse_double_flex(cols[10]);
            ped->desconto = parse_double_flex(cols[11]);
            ped->valor_frete = parse_double_flex(cols[12]);
            ped->valor_total = parse_double_flex(cols[13]);
            copiar_seguro(ped->pagamento.forma_pagamento, cols[14], sizeof(ped->pagamento.forma_pagamento));
            copiar_seguro(ped->pagamento.status_transacao, cols[15], sizeof(ped->pagamento.status_transacao));
            copiar_seguro(ped->pagamento.codigo_autorizacao, cols[16], sizeof(ped->pagamento.codigo_autorizacao));
            copiar_seguro(ped->status_pedido, cols[17], sizeof(ped->status_pedido));
            copiar_seguro(ped->codigo_rastreamento, cols[18], sizeof(ped->codigo_rastreamento));
            copiar_seguro(ped->data_hora, cols[19], sizeof(ped->data_hora));
        } else if (strncmp(linha, "I;", 2) == 0 && g_total_pedidos > 0) {
            Pedido *ped = &g_pedidos[g_total_pedidos - 1];
            if (ped->total_itens >= MAX_ITENS_CARRINHO) continue;
            char *cols[6] = {0};
            int c = 0;
            char *p = linha;
            cols[c++] = p;
            while (*p && c < 6) {
                if (*p == ';') {
                    *p = '\0';
                    cols[c++] = p + 1;
                }
                p++;
            }
            if (c < 6) continue;
            ItemCarrinho *it = &ped->itens[ped->total_itens++];
            it->id_produto = atoi(cols[1]);
            copiar_seguro(it->nome_produto, cols[2], sizeof(it->nome_produto));
            it->quantidade = atoi(cols[3]);
            it->preco_unitario = parse_double_flex(cols[4]);
            it->subtotal = parse_double_flex(cols[5]);
        }
    }
    fclose(fp);
}

static void gravar_recibo_legivel(const Pedido *ped) {
    FILE *fp = fopen(ARQUIVO_RECIBOS, "a");
    if (!fp) return;

    fprintf(fp, "\n============================================================\n");
    fprintf(fp, "PEDIDO: %s (ID #%d) | DATA: %s\n", ped->codigo_pedido, ped->id_pedido, ped->data_hora);
    fprintf(fp, "CLIENTE (UID #%d): %s | CPF: %s\n", ped->id_usuario, ped->nome_cliente, ped->cpf_cliente);
    fprintf(fp, "E-MAIL: %s | TEL: %s\n", ped->email_cliente, ped->telefone_cliente);
    fprintf(fp, "ENTREGA: %s (CEP: %s)\n", ped->endereco_entrega, ped->cep_entrega);
    fprintf(fp, "PAGAMENTO: %s | GATEWAY: %s (AUTH %s)\n",
            ped->pagamento.forma_pagamento, ped->pagamento.status_transacao, ped->pagamento.codigo_autorizacao);
    fprintf(fp, "STATUS PEDIDO: %s | RASTREIO: %s\n", ped->status_pedido, ped->codigo_rastreamento);
    fprintf(fp, "------------------------------------------------------------\n");
    for (int i = 0; i < ped->total_itens; i++) {
        const ItemCarrinho *it = &ped->itens[i];
        fprintf(fp, " - %dx %s | Unit: R$ %.2f | Sub: R$ %.2f\n",
                it->quantidade, it->nome_produto, it->preco_unitario, it->subtotal);
    }
    fprintf(fp, "------------------------------------------------------------\n");
    fprintf(fp, "SUBTOTAL: R$ %.2f | DESCONTO: -R$ %.2f | FRETE: R$ %.2f\n",
            ped->valor_bruto, ped->desconto, ped->valor_frete);
    fprintf(fp, "TOTAL PAGO: R$ %.2f\n", ped->valor_total);
    fprintf(fp, "============================================================\n");
    fclose(fp);
}

/* ============================================================================
 * LOGICA DE NEGOCIO (CONTROLLER): SESSAO, FRETE, GATEWAY E CHECKOUT ATOMICO
 * ============================================================================ */
static void criar_sessao_para_usuario(int id_usuario, char token_saida[65]) {
    char semente[128];
    snprintf(semente, sizeof(semente), "TOKEN:%d:%lld:%lu",
             id_usuario, (long long)time(NULL), g_contador_seq++);
    sha256_hex(semente, token_saida);

    int slot = 0;
    time_t agora = time(NULL);
    for (int i = 0; i < MAX_SESSOES; i++) {
        if (g_sessoes[i].id_usuario == 0 || g_sessoes[i].expira_em < agora) {
            slot = i;
            break;
        }
    }
    copiar_seguro(g_sessoes[slot].token, token_saida, sizeof(g_sessoes[slot].token));
    g_sessoes[slot].id_usuario = id_usuario;
    g_sessoes[slot].expira_em = agora + SEGUNDOS_SESSAO;
}

static Usuario* validar_token_sessao(const char *token) {
    if (!token || strlen(token) < 16) return NULL;
    time_t agora = time(NULL);
    for (int i = 0; i < MAX_SESSOES; i++) {
        if (g_sessoes[i].id_usuario > 0 && strcmp(g_sessoes[i].token, token) == 0) {
            if (g_sessoes[i].expira_em < agora) {
                g_sessoes[i].id_usuario = 0;
                return NULL;
            }
            Usuario *u = buscar_usuario_por_id(g_sessoes[i].id_usuario);
            if (!u || !u->ativo) return NULL;
            return u;
        }
    }
    return NULL;
}

static void revogar_sessoes_usuario(int id_usuario) {
    for (int i = 0; i < MAX_SESSOES; i++) {
        if (g_sessoes[i].id_usuario == id_usuario) {
            g_sessoes[i].id_usuario = 0;
            g_sessoes[i].token[0] = '\0';
        }
    }
}

/* Calcula o valor estimado de frete conforme regiao do CEP e quantidade de volumes (RF04). */
static double calcular_frete_pedido(const char *cep, double valor_bruto, int total_unidades, int *prazo_dias) {
    char digitos[16];
    extrair_apenas_digitos(cep, digitos, sizeof(digitos));

    if (strlen(digitos) < 8) {
        if (prazo_dias) *prazo_dias = 5;
        return (total_unidades <= 0) ? 0.0 : 29.90;
    }

    char regiao = digitos[0];
    double base = 29.90;
    double adicional_item = 5.00;
    int dias = 4;

    if (regiao == '0') {
        base = 19.90; adicional_item = 4.00; dias = 2;
    } else if (regiao >= '1' && regiao <= '3') {
        base = 29.90; adicional_item = 5.00; dias = 4;
    } else if (regiao >= '4' && regiao <= '5') {
        base = 44.90; adicional_item = 6.50; dias = 6;
    } else {
        base = 39.90; adicional_item = 6.00; dias = 5;
    }

    if (prazo_dias) *prazo_dias = dias;
    if (total_unidades <= 0) return 0.0;

    /* Frete fixo expresso para pedidos de alto valor (>= R$ 8000) com seguro de carga. */
    if (valor_bruto >= 8000.00) {
        return 0.0;
    }

    int extras = (total_unidades > 1) ? (total_unidades - 1) : 0;
    return base + (extras * adicional_item);
}

/* Simula comunicacao externa TLS com Gateway de Pagamento conforme Diagrama de Sequencia (PIM.pdf p.11). */
static int processar_gateway_pagamento(const char *forma, double valor, int simular_recusa, Pagamento *out_pg) {
    memset(out_pg, 0, sizeof(Pagamento));
    if (_stricmp(forma, "PIX") != 0 && _stricmp(forma, "CARTAO") != 0 && _stricmp(forma, "BOLETO") != 0) {
        copiar_seguro(out_pg->forma_pagamento, "PIX", sizeof(out_pg->forma_pagamento));
    } else {
        copiar_seguro(out_pg->forma_pagamento, forma, sizeof(out_pg->forma_pagamento));
        for (int i = 0; out_pg->forma_pagamento[i]; i++) {
            out_pg->forma_pagamento[i] = (char)toupper((unsigned char)out_pg->forma_pagamento[i]);
        }
    }

    if (simular_recusa || valor <= 0.0) {
        copiar_seguro(out_pg->status_transacao, "RECUSADO", sizeof(out_pg->status_transacao));
        copiar_seguro(out_pg->codigo_autorizacao, "ERR-GATEWAY-402", sizeof(out_pg->codigo_autorizacao));
        return 0;
    }

    copiar_seguro(out_pg->status_transacao, "APROVADO", sizeof(out_pg->status_transacao));
    snprintf(out_pg->codigo_autorizacao, sizeof(out_pg->codigo_autorizacao),
             "GW%06llu", (unsigned long long)((time(NULL) + g_contador_seq++) % 1000000ULL));
    return 1;
}

/* Executa checkout transacional em duas fases (Validacao -> Gateway -> Baixa Atomica + Rastreio). */
static int executar_checkout_atomico(
    Usuario *cliente,
    const char *endereco_entrega,
    const char *cep_entrega,
    const char *forma_pagamento,
    const char *cupom,
    int simular_recusa_gateway,
    ItemCarrinho *itens,
    int qtd_tipos_itens,
    Pedido *out_pedido,
    char *msg_erro,
    size_t tam_erro,
    int *out_alertas_estoque
) {
    if (!cliente || !cliente->ativo) {
        copiar_seguro(msg_erro, "Usuario nao autenticado ou conta inativa (UC02/UC05).", tam_erro);
        return 0;
    }
    if (!validar_cpf_basico(cliente->cpf)) {
        copiar_seguro(msg_erro, "CPF do cliente invalido no cadastro (esperado 11 digitos).", tam_erro);
        return 0;
    }
    if (!endereco_entrega || strlen(endereco_entrega) < 5) {
        copiar_seguro(msg_erro, "Endereco de entrega obrigatorio.", tam_erro);
        return 0;
    }
    if (qtd_tipos_itens <= 0 || qtd_tipos_itens > MAX_ITENS_CARRINHO) {
        copiar_seguro(msg_erro, "Carrinho vazio ou excede o limite maximo de itens.", tam_erro);
        return 0;
    }
    if (g_total_pedidos >= MAX_PEDIDOS) {
        copiar_seguro(msg_erro, "Capacidade maxima de registros de pedidos atingida.", tam_erro);
        return 0;
    }

    /* FASE 1: Validacao integral do estoque (nenhuma baixa ocorre se algum item falhar). */
    double valor_bruto = 0.0;
    int total_unidades = 0;
    for (int i = 0; i < qtd_tipos_itens; i++) {
        Produto *pr = buscar_produto_por_id(itens[i].id_produto);
        if (!pr || !pr->ativo) {
            snprintf(msg_erro, tam_erro, "Produto ID %d indisponivel ou inativo no catalogo.", itens[i].id_produto);
            return 0;
        }
        if (itens[i].quantidade <= 0) {
            snprintf(msg_erro, tam_erro, "Quantidade invalida para '%s'.", pr->nome);
            return 0;
        }

        /* Soma quantidade repetida do mesmo produto dentro do mesmo carrinho */
        int qtd_total_mesmo_prod = 0;
        for (int k = 0; k < qtd_tipos_itens; k++) {
            if (itens[k].id_produto == pr->id) qtd_total_mesmo_prod += itens[k].quantidade;
        }
        if (qtd_total_mesmo_prod > pr->estoque) {
            snprintf(msg_erro, tam_erro, "Estoque insuficiente para '%s' (Disponivel: %d).", pr->nome, pr->estoque);
            return 0;
        }

        copiar_seguro(itens[i].nome_produto, pr->nome, sizeof(itens[i].nome_produto));
        itens[i].preco_unitario = pr->preco;
        itens[i].subtotal = pr->preco * itens[i].quantidade;
        valor_bruto += itens[i].subtotal;
        total_unidades += itens[i].quantidade;
    }

    /* Regras de negocio: Desconto + Frete (Controller) */
    double percentual_desc = 0.0;
    if (cupom && _stricmp(cupom, "TECH10") == 0) percentual_desc = 0.10;
    else if (cupom && _stricmp(cupom, "CYBER20") == 0) percentual_desc = 0.20;

    double desconto = valor_bruto * percentual_desc;
    double valor_frete = calcular_frete_pedido(cep_entrega, valor_bruto, total_unidades, NULL);
    double valor_total = (valor_bruto - desconto) + valor_frete;

    /* FASE 1.5: Autorizacao junto ao Gateway de Pagamento (Sequence Diagram passo 5 e 6) */
    Pagamento pg;
    if (!processar_gateway_pagamento(forma_pagamento, valor_total, simular_recusa_gateway, &pg)) {
        copiar_seguro(msg_erro, "Transacao recusada pelo Gateway de Pagamento. Estoque preservado integralmente.", tam_erro);
        return 0;
    }

    /* FASE 2: Baixa atomica do estoque + emissao de alerta de estoque minimo (RF03 / UC07) */
    int alertas = 0;
    for (int i = 0; i < qtd_tipos_itens; i++) {
        Produto *pr = buscar_produto_por_id(itens[i].id_produto);
        if (pr) {
            pr->estoque -= itens[i].quantidade;
        }
    }
    for (int i = 0; i < g_total_produtos; i++) {
        if (g_produtos[i].ativo && g_produtos[i].estoque <= g_produtos[i].estoque_minimo) {
            alertas++;
        }
    }
    if (out_alertas_estoque) *out_alertas_estoque = alertas;

    /* Registra Pedido no Banco de Dados simulado (Sequence Diagram passo 7 e 8) */
    Pedido novo;
    memset(&novo, 0, sizeof(Pedido));
    novo.id_pedido = g_total_pedidos + 1;
    novo.id_usuario = cliente->id;
    snprintf(novo.codigo_pedido, sizeof(novo.codigo_pedido),
             "CHW-%04d%02llu", novo.id_pedido, (unsigned long long)(time(NULL) % 100ULL));
    copiar_seguro(novo.nome_cliente, cliente->nome, sizeof(novo.nome_cliente));
    copiar_seguro(novo.cpf_cliente, cliente->cpf, sizeof(novo.cpf_cliente));
    copiar_seguro(novo.email_cliente, cliente->email, sizeof(novo.email_cliente));
    copiar_seguro(novo.telefone_cliente, cliente->telefone, sizeof(novo.telefone_cliente));
    copiar_seguro(novo.endereco_entrega, endereco_entrega, sizeof(novo.endereco_entrega));
    copiar_seguro(novo.cep_entrega, (cep_entrega && *cep_entrega) ? cep_entrega : "01310-100", sizeof(novo.cep_entrega));

    novo.total_itens = qtd_tipos_itens;
    for (int i = 0; i < qtd_tipos_itens; i++) {
        novo.itens[i] = itens[i];
    }
    novo.valor_bruto = valor_bruto;
    novo.desconto = desconto;
    novo.valor_frete = valor_frete;
    novo.valor_total = valor_total;
    novo.pagamento = pg;
    copiar_seguro(novo.status_pedido, "Pagamento Aprovado", sizeof(novo.status_pedido));
    snprintf(novo.codigo_rastreamento, sizeof(novo.codigo_rastreamento),
             "CHW%06dBR", 100000 + novo.id_pedido * 7919 % 899999);
    obter_data_hora_atual(novo.data_hora, sizeof(novo.data_hora));

    g_pedidos[g_total_pedidos++] = novo;
    salvar_produtos_arquivo();
    salvar_pedidos_arquivo();
    gravar_recibo_legivel(&novo);

    if (out_pedido) *out_pedido = novo;
    return 1;
}

/* ============================================================================
 * SERVIDOR HTTP REST + ARQUIVOS ESTATICOS (WINSOCK2 / PORTA 8080)
 * ============================================================================ */
static void enviar_resposta_http(socket_t cliente, int status_code, const char *status_text,
                                 const char *content_type, const char *corpo, size_t corpo_len) {
    char cabecalho[1024];
    int header_len = snprintf(
        cabecalho, sizeof(cabecalho),
        "HTTP/1.1 %d %s\r\n"
        "Content-Type: %s; charset=utf-8\r\n"
        "Content-Length: %zu\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Access-Control-Allow-Headers: Content-Type, Authorization\r\n"
        "Access-Control-Allow-Methods: GET, POST, PUT, OPTIONS\r\n"
        "Cache-Control: no-store\r\n"
        "Connection: close\r\n\r\n",
        status_code, status_text, content_type, corpo_len
    );
    send(cliente, cabecalho, header_len, 0);
    if (corpo && corpo_len > 0) {
        size_t enviado = 0;
        while (enviado < corpo_len) {
            int n = send(cliente, corpo + enviado, (int)(corpo_len - enviado), 0);
            if (n <= 0) break;
            enviado += (size_t)n;
        }
    }
}

static void enviar_json_erro(socket_t cliente, int code, const char *status, const char *mensagem) {
    char esc[512];
    char body[700];
    json_escapar_string(mensagem, esc, sizeof(esc));
    int len = snprintf(body, sizeof(body), "{\"ok\":false,\"erro\":\"%s\"}", esc);
    enviar_resposta_http(cliente, code, status, "application/json", body, (size_t)len);
}

static void serializar_usuario_json(const Usuario *u, char *dest, size_t max_tam) {
    char nome_e[180], email_e[160], tel_e[60], end_e[260];
    json_escapar_string(u->nome, nome_e, sizeof(nome_e));
    json_escapar_string(u->email, email_e, sizeof(email_e));
    json_escapar_string(u->telefone, tel_e, sizeof(tel_e));
    json_escapar_string(u->endereco, end_e, sizeof(end_e));

    /* RNF01 / LGPD: nunca expoe senha_hash em respostas HTTP! */
    snprintf(dest, max_tam,
             "{\"id\":%d,\"nome\":\"%s\",\"cpf\":\"%s\",\"email\":\"%s\",\"telefone\":\"%s\","
             "\"endereco\":\"%s\",\"papel\":\"%s\",\"nivel_acesso\":\"%s\",\"ativo\":%s}",
             u->id, nome_e, u->cpf, email_e, tel_e, end_e,
             u->papel, u->nivel_acesso, u->ativo ? "true" : "false");
}

static void extrair_bearer_token(const char *headers, char token_out[65]) {
    token_out[0] = '\0';
    const char *p = headers;
    while (*p) {
        if (comparacao_case_insensitive_prefixo(p, "Authorization:")) {
            p += 14;
            while (*p == ' ' || *p == '\t') p++;
            if (comparacao_case_insensitive_prefixo(p, "Bearer ")) {
                p += 7;
                while (*p == ' ') p++;
                size_t i = 0;
                while (*p && *p != '\r' && *p != '\n' && *p != ' ' && i + 1 < 65) {
                    token_out[i++] = *p++;
                }
                token_out[i] = '\0';
                return;
            }
        }
        const char *eol = strchr(p, '\n');
        if (!eol) break;
        p = eol + 1;
    }
}

static void servir_svg_produto(socket_t cliente, const char *rota) {
    const char *cor_pri = "#00f0ff";
    const char *cor_sec = "#0f172a";
    const char *sigla = "HW";
    const char *sub = "CyberHardware";

    if (strstr(rota, "gpu")) { cor_pri = "#22d3ee"; sigla = "GPU"; sub = "RTX SERIES"; }
    else if (strstr(rota, "cpu")) { cor_pri = "#f97316"; sigla = "CPU"; sub = "RYZEN AM5"; }
    else if (strstr(rota, "ram")) { cor_pri = "#a855f7"; sigla = "DDR5"; sub = "6000 MT/s"; }
    else if (strstr(rota, "ssd")) { cor_pri = "#eab308"; sigla = "NVMe"; sub = "PCIe 4.0"; }
    else if (strstr(rota, "mobo")) { cor_pri = "#ef4444"; sigla = "X870E"; sub = "ROG STRIX"; }
    else if (strstr(rota, "psu")) { cor_pri = "#10b981"; sigla = "1000W"; sub = "80+ GOLD"; }

    char svg[1200];
    int len = snprintf(
        svg, sizeof(svg),
        "<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 320 200' role='img' aria-label='%s'>"
        "<defs><linearGradient id='g' x1='0' y1='0' x2='1' y2='1'>"
        "<stop offset='0%%' stop-color='%s'/><stop offset='100%%' stop-color='#020617'/></linearGradient></defs>"
        "<rect width='320' height='200' rx='14' fill='url(#g)'/>"
        "<rect x='14' y='14' width='292' height='172' rx='10' fill='none' stroke='%s' stroke-opacity='0.35' stroke-width='2'/>"
        "<circle cx='265' cy='48' r='22' fill='%s' fill-opacity='0.14' stroke='%s' stroke-width='1.5'/>"
        "<text x='32' y='105' fill='%s' font-family='Segoe UI,Arial,sans-serif' font-size='44' font-weight='800'>%s</text>"
        "<text x='32' y='142' fill='#cbd5e1' font-family='Consolas,monospace' font-size='15' letter-spacing='2'>%s</text>"
        "</svg>",
        sigla, cor_sec, cor_pri, cor_pri, cor_pri, cor_pri, sigla, sub
    );
    enviar_resposta_http(cliente, 200, "OK", "image/svg+xml", svg, (size_t)len);
}

static void servir_arquivo_html(socket_t cliente) {
    FILE *fp = fopen(ARQUIVO_HTML, "rb");
    if (!fp) {
        const char *erro = "<h1>404 - web/index.html nao encontrado</h1>";
        enviar_resposta_http(cliente, 404, "Not Found", "text/html", erro, strlen(erro));
        return;
    }
    fseek(fp, 0, SEEK_END);
    long tam = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    char *conteudo = (char *)malloc((size_t)tam + 1);
    if (!conteudo) {
        fclose(fp);
        enviar_json_erro(cliente, 500, "Internal Server Error", "Sem memoria para carregar interface.");
        return;
    }
    size_t lido = fread(conteudo, 1, (size_t)tam, fp);
    fclose(fp);
    enviar_resposta_http(cliente, 200, "OK", "text/html", conteudo, lido);
    free(conteudo);
}

/* Parseia array JSON "itens": [{"id": 1, "quantidade": 2}, ...] sem biblioteca externa. */
static int parsear_itens_json(const char *corpo_json, ItemCarrinho *itens, int max_itens) {
    const char *p = json_encontrar_chave(corpo_json, "itens");
    if (!p) return 0;
    const char *abre_col = strchr(p, '[');
    if (!abre_col) return 0;

    int qtd = 0;
    const char *cur = abre_col + 1;
    while (*cur && *cur != ']' && qtd < max_itens) {
        const char *obj_ini = strchr(cur, '{');
        if (!obj_ini) break;
        const char *obj_fim = strchr(obj_ini, '}');
        if (!obj_fim) break;

        size_t sub_len = (size_t)(obj_fim - obj_ini + 1);
        char sub[256];
        if (sub_len >= sizeof(sub)) sub_len = sizeof(sub) - 1;
        memcpy(sub, obj_ini, sub_len);
        sub[sub_len] = '\0';

        int id_p = json_extrair_int(sub, "id", 0);
        int quant = json_extrair_int(sub, "quantidade", 0);
        if (id_p > 0 && quant > 0) {
            memset(&itens[qtd], 0, sizeof(ItemCarrinho));
            itens[qtd].id_produto = id_p;
            itens[qtd].quantidade = quant;
            qtd++;
        }
        cur = obj_fim + 1;
    }
    return qtd;
}

static void tratar_requisicao_http(socket_t cliente) {
    char *buffer = (char *)malloc(BUFFER_HTTP);
    if (!buffer) {
        FECHAR_SOCKET(cliente);
        return;
    }

    int total_lido = 0;
    int header_fim = -1;
    int content_length = 0;

    while (total_lido < BUFFER_HTTP - 1) {
        int n = recv(cliente, buffer + total_lido, BUFFER_HTTP - 1 - total_lido, 0);
        if (n <= 0) break;
        total_lido += n;
        buffer[total_lido] = '\0';

        if (header_fim < 0) {
            char *sep = strstr(buffer, "\r\n\r\n");
            if (sep) {
                header_fim = (int)(sep - buffer) + 4;
                const char *h = buffer;
                while (h < sep) {
                    if (comparacao_case_insensitive_prefixo(h, "Content-Length:")) {
                        content_length = atoi(h + 15);
                        break;
                    }
                    const char *eol = strchr(h, '\n');
                    if (!eol) break;
                    h = eol + 1;
                }
            }
        }
        if (header_fim >= 0 && total_lido >= header_fim + content_length) {
            break;
        }
    }

    if (total_lido <= 0) {
        free(buffer);
        FECHAR_SOCKET(cliente);
        return;
    }

    char metodo[16] = {0};
    char rota[256] = {0};
    sscanf(buffer, "%15s %255s", metodo, rota);

    const char *corpo = (header_fim >= 0) ? (buffer + header_fim) : "";
    char token[65];
    extrair_bearer_token(buffer, token);
    Usuario *auth = validar_token_sessao(token);

    if (strcmp(metodo, "OPTIONS") == 0) {
        enviar_resposta_http(cliente, 204, "No Content", "text/plain", "", 0);
    } else if (strcmp(metodo, "GET") == 0 && (strcmp(rota, "/") == 0 || strcmp(rota, "/index.html") == 0)) {
        servir_arquivo_html(cliente);
    } else if (strcmp(metodo, "GET") == 0 && strncmp(rota, "/img/", 5) == 0) {
        servir_svg_produto(cliente, rota);
    } else if (strcmp(metodo, "GET") == 0 && strcmp(rota, "/api/status") == 0) {
        int ativos = 0, alertas = 0;
        for (int i = 0; i < g_total_produtos; i++) {
            if (g_produtos[i].ativo) {
                ativos++;
                if (g_produtos[i].estoque <= g_produtos[i].estoque_minimo) alertas++;
            }
        }
        char resp[512];
        int len = snprintf(resp, sizeof(resp),
                           "{\"ok\":true,\"status\":\"ONLINE\",\"porta\":%d,\"produtos_ativos\":%d,"
                           "\"produtos_total\":%d,\"alertas_estoque\":%d,\"usuarios\":%d,\"pedidos\":%d}",
                           g_porta_preferida, ativos, g_total_produtos, alertas, g_total_usuarios, g_total_pedidos);
        enviar_resposta_http(cliente, 200, "OK", "application/json", resp, (size_t)len);
    } else if (strcmp(metodo, "POST") == 0 && strcmp(rota, "/api/auth/register") == 0) {
        char nome[100], cpf[20], email[100], tel[30], end[160], senha[80];
        json_extrair_string(corpo, "nome", nome, sizeof(nome));
        json_extrair_string(corpo, "cpf", cpf, sizeof(cpf));
        json_extrair_string(corpo, "email", email, sizeof(email));
        json_extrair_string(corpo, "telefone", tel, sizeof(tel));
        json_extrair_string(corpo, "endereco", end, sizeof(end));
        json_extrair_string(corpo, "senha", senha, sizeof(senha));

        if (strlen(nome) < 3 || !validar_cpf_basico(cpf) || !strchr(email, '@') || strlen(senha) < 6 || strlen(end) < 5) {
            enviar_json_erro(cliente, 400, "Bad Request", "Preencha nome, CPF (11 digitos), e-mail valido, endereco e senha (>= 6 chars).");
        } else if (buscar_usuario_por_email(email) != NULL) {
            enviar_json_erro(cliente, 409, "Conflict", "E-mail ja cadastrado no sistema.");
        } else if (g_total_usuarios >= MAX_USUARIOS) {
            enviar_json_erro(cliente, 507, "Insufficient Storage", "Limite de usuarios atingido.");
        } else {
            Usuario *u = &g_usuarios[g_total_usuarios++];
            memset(u, 0, sizeof(Usuario));
            u->id = g_total_usuarios;
            copiar_seguro(u->nome, nome, sizeof(u->nome));
            extrair_apenas_digitos(cpf, u->cpf, sizeof(u->cpf));
            copiar_seguro(u->email, email, sizeof(u->email));
            copiar_seguro(u->telefone, strlen(tel) ? tel : "(11) 90000-0000", sizeof(u->telefone));
            copiar_seguro(u->endereco, end, sizeof(u->endereco));
            copiar_seguro(u->papel, "CLIENTE", sizeof(u->papel));
            copiar_seguro(u->nivel_acesso, "PADRAO", sizeof(u->nivel_acesso));
            u->ativo = 1;
            gerar_hash_senha(senha, NULL, u->senha_hash);
            salvar_usuarios_arquivo();

            char novo_token[65], user_json[640], resp[900];
            criar_sessao_para_usuario(u->id, novo_token);
            serializar_usuario_json(u, user_json, sizeof(user_json));
            int len = snprintf(resp, sizeof(resp), "{\"ok\":true,\"token\":\"%s\",\"usuario\":%s}", novo_token, user_json);
            enviar_resposta_http(cliente, 201, "Created", "application/json", resp, (size_t)len);
        }
    } else if (strcmp(metodo, "POST") == 0 && strcmp(rota, "/api/auth/login") == 0) {
        char email[100], senha[80];
        json_extrair_string(corpo, "email", email, sizeof(email));
        json_extrair_string(corpo, "senha", senha, sizeof(senha));

        Usuario *u = buscar_usuario_por_email(email);
        if (!u || !verificar_hash_senha(senha, u->senha_hash)) {
            enviar_json_erro(cliente, 401, "Unauthorized", "E-mail ou senha invalidos.");
        } else if (!u->ativo) {
            enviar_json_erro(cliente, 403, "Forbidden", "Conta inativada. Contate o suporte ou cadastre nova conta.");
        } else {
            char novo_token[65], user_json[640], resp[900];
            criar_sessao_para_usuario(u->id, novo_token);
            serializar_usuario_json(u, user_json, sizeof(user_json));
            int len = snprintf(resp, sizeof(resp), "{\"ok\":true,\"token\":\"%s\",\"usuario\":%s}", novo_token, user_json);
            enviar_resposta_http(cliente, 200, "OK", "application/json", resp, (size_t)len);
        }
    } else if (strcmp(metodo, "GET") == 0 && strcmp(rota, "/api/auth/me") == 0) {
        if (!auth) {
            enviar_json_erro(cliente, 401, "Unauthorized", "Sessao ausente ou expirada.");
        } else {
            char user_json[640], resp[768];
            serializar_usuario_json(auth, user_json, sizeof(user_json));
            int len = snprintf(resp, sizeof(resp), "{\"ok\":true,\"usuario\":%s}", user_json);
            enviar_resposta_http(cliente, 200, "OK", "application/json", resp, (size_t)len);
        }
    } else if (strcmp(metodo, "PUT") == 0 && strcmp(rota, "/api/auth/profile") == 0) {
        if (!auth) {
            enviar_json_erro(cliente, 401, "Unauthorized", "Autenticacao obrigatoria para atualizar perfil.");
        } else {
            char nome[100], tel[30], end[160], nova_senha[80];
            if (json_extrair_string(corpo, "nome", nome, sizeof(nome)) && strlen(nome) >= 3)
                copiar_seguro(auth->nome, nome, sizeof(auth->nome));
            if (json_extrair_string(corpo, "telefone", tel, sizeof(tel)) && strlen(tel) >= 8)
                copiar_seguro(auth->telefone, tel, sizeof(auth->telefone));
            if (json_extrair_string(corpo, "endereco", end, sizeof(end)) && strlen(end) >= 5)
                copiar_seguro(auth->endereco, end, sizeof(auth->endereco));
            if (json_extrair_string(corpo, "nova_senha", nova_senha, sizeof(nova_senha)) && strlen(nova_senha) >= 6)
                gerar_hash_senha(nova_senha, NULL, auth->senha_hash);

            salvar_usuarios_arquivo();
            char user_json[640], resp[768];
            serializar_usuario_json(auth, user_json, sizeof(user_json));
            int len = snprintf(resp, sizeof(resp), "{\"ok\":true,\"usuario\":%s}", user_json);
            enviar_resposta_http(cliente, 200, "OK", "application/json", resp, (size_t)len);
        }
    } else if (strcmp(metodo, "POST") == 0 && strcmp(rota, "/api/auth/inactivate") == 0) {
        if (!auth) {
            enviar_json_erro(cliente, 401, "Unauthorized", "Autenticacao obrigatoria.");
        } else {
            auth->ativo = 0;
            revogar_sessoes_usuario(auth->id);
            salvar_usuarios_arquivo();
            const char *ok = "{\"ok\":true,\"mensagem\":\"Conta inativada com sucesso.\"}";
            enviar_resposta_http(cliente, 200, "OK", "application/json", ok, strlen(ok));
        }
    } else if (strcmp(metodo, "GET") == 0 && strncmp(rota, "/api/produtos", 13) == 0) {
        int incluir_inativos = (strstr(rota, "todos=1") != NULL && auth && strcmp(auth->papel, "ADMIN") == 0);
        char *json = (char *)malloc(49152);
        if (!json) {
            enviar_json_erro(cliente, 500, "Internal Server Error", "Memoria insuficiente.");
        } else {
            size_t pos = (size_t)snprintf(json, 49152, "[");
            int primeiro = 1;
            for (int i = 0; i < g_total_produtos && pos + 1024 < 49152; i++) {
                Produto *p = &g_produtos[i];
                if (!incluir_inativos && !p->ativo) continue;
                char n_e[180], c_e[96], b_e[64], f_e[128], s_e[320];
                json_escapar_string(p->nome, n_e, sizeof(n_e));
                json_escapar_string(p->categoria, c_e, sizeof(c_e));
                json_escapar_string(p->badge, b_e, sizeof(b_e));
                json_escapar_string(p->foto, f_e, sizeof(f_e));
                json_escapar_string(p->specs, s_e, sizeof(s_e));

                pos += (size_t)snprintf(
                    json + pos, 49152 - pos,
                    "%s{\"id\":%d,\"nome\":\"%s\",\"categoria\":\"%s\",\"preco\":%.2f,\"preco_antigo\":%.2f,"
                    "\"estoque\":%d,\"estoque_minimo\":%d,\"ativo\":%s,\"alerta_estoque\":%s,"
                    "\"badge\":\"%s\",\"foto\":\"%s\",\"specs\":\"%s\"}",
                    primeiro ? "" : ",",
                    p->id, n_e, c_e, p->preco, p->preco_antigo,
                    p->estoque, p->estoque_minimo,
                    p->ativo ? "true" : "false",
                    (p->estoque <= p->estoque_minimo) ? "true" : "false",
                    b_e, f_e, s_e
                );
                primeiro = 0;
            }
            snprintf(json + pos, 49152 - pos, "]");
            enviar_resposta_http(cliente, 200, "OK", "application/json", json, strlen(json));
            free(json);
        }
    } else if ((strcmp(metodo, "POST") == 0 || strcmp(metodo, "PUT") == 0) && strcmp(rota, "/api/produtos") == 0) {
        if (!auth || strcmp(auth->papel, "ADMIN") != 0) {
            enviar_json_erro(cliente, 403, "Forbidden", "Acesso restrito ao Administrador (UC06/UC07).");
        } else if (strcmp(metodo, "POST") == 0) {
            if (g_total_produtos >= MAX_PRODUTOS) {
                enviar_json_erro(cliente, 507, "Insufficient Storage", "Catalogo cheio.");
            } else {
                char nome[100], cat[48], badge[32], foto[64], specs[180];
                json_extrair_string(corpo, "nome", nome, sizeof(nome));
                json_extrair_string(corpo, "categoria", cat, sizeof(cat));
                json_extrair_string(corpo, "badge", badge, sizeof(badge));
                json_extrair_string(corpo, "foto", foto, sizeof(foto));
                json_extrair_string(corpo, "specs", specs, sizeof(specs));
                double preco = json_extrair_double(corpo, "preco", 0.0);
                double preco_ant = json_extrair_double(corpo, "preco_antigo", preco * 1.12);
                int est = json_extrair_int(corpo, "estoque", 0);
                int est_min = json_extrair_int(corpo, "estoque_minimo", 2);

                if (strlen(nome) < 3 || preco <= 0.0 || est < 0) {
                    enviar_json_erro(cliente, 400, "Bad Request", "Nome, preco (>0) e estoque (>=0) obrigatorios.");
                } else {
                    int max_id = 0;
                    for (int i = 0; i < g_total_produtos; i++) {
                        if (g_produtos[i].id > max_id) max_id = g_produtos[i].id;
                    }
                    Produto *p = &g_produtos[g_total_produtos++];
                    memset(p, 0, sizeof(Produto));
                    p->id = max_id + 1;
                    copiar_seguro(p->nome, nome, sizeof(p->nome));
                    copiar_seguro(p->categoria, strlen(cat) ? cat : "Perifericos", sizeof(p->categoria));
                    p->preco = preco;
                    p->preco_antigo = (preco_ant > preco) ? preco_ant : preco;
                    p->estoque = est;
                    p->estoque_minimo = (est_min >= 0) ? est_min : 2;
                    p->ativo = 1;
                    copiar_seguro(p->badge, strlen(badge) ? badge : "Novo", sizeof(p->badge));
                    copiar_seguro(p->foto, strlen(foto) ? foto : "/img/default.svg", sizeof(p->foto));
                    copiar_seguro(p->specs, strlen(specs) ? specs : "Especificacoes tecnicas CyberHardware", sizeof(p->specs));
                    salvar_produtos_arquivo();

                    char resp[128];
                    int len = snprintf(resp, sizeof(resp), "{\"ok\":true,\"id\":%d,\"mensagem\":\"Produto cadastrado.\"}", p->id);
                    enviar_resposta_http(cliente, 201, "Created", "application/json", resp, (size_t)len);
                }
            }
        } else {
            int id = json_extrair_int(corpo, "id", 0);
            Produto *p = buscar_produto_por_id(id);
            if (!p) {
                enviar_json_erro(cliente, 404, "Not Found", "Produto nao encontrado.");
            } else {
                char buf[180];
                if (json_extrair_string(corpo, "nome", buf, sizeof(buf)) && strlen(buf) >= 3) copiar_seguro(p->nome, buf, sizeof(p->nome));
                if (json_extrair_string(corpo, "categoria", buf, sizeof(buf)) && strlen(buf) >= 2) copiar_seguro(p->categoria, buf, sizeof(p->categoria));
                if (json_extrair_string(corpo, "specs", buf, sizeof(buf)) && strlen(buf) >= 3) copiar_seguro(p->specs, buf, sizeof(p->specs));
                if (json_extrair_string(corpo, "foto", buf, sizeof(buf)) && strlen(buf) >= 3) copiar_seguro(p->foto, buf, sizeof(p->foto));
                double pr = json_extrair_double(corpo, "preco", -1.0);
                if (pr > 0.0) p->preco = pr;
                int est = json_extrair_int(corpo, "estoque", -999);
                if (est >= 0) p->estoque = est;
                int est_min = json_extrair_int(corpo, "estoque_minimo", -999);
                if (est_min >= 0) p->estoque_minimo = est_min;
                int ativo_flag = json_extrair_int(corpo, "ativo", -1);
                if (ativo_flag == 0 || ativo_flag == 1) p->ativo = ativo_flag;

                salvar_produtos_arquivo();
                char resp[256];
                int len = snprintf(resp, sizeof(resp),
                                   "{\"ok\":true,\"id\":%d,\"estoque\":%d,\"alerta_estoque\":%s}",
                                   p->id, p->estoque, (p->estoque <= p->estoque_minimo) ? "true" : "false");
                enviar_resposta_http(cliente, 200, "OK", "application/json", resp, (size_t)len);
            }
        }
    } else if (strcmp(metodo, "POST") == 0 && strcmp(rota, "/api/frete") == 0) {
        char cep[24];
        json_extrair_string(corpo, "cep", cep, sizeof(cep));
        ItemCarrinho itens[MAX_ITENS_CARRINHO];
        int qtd = parsear_itens_json(corpo, itens, MAX_ITENS_CARRINHO);
        double bruto = 0.0;
        int total_un = 0;
        for (int i = 0; i < qtd; i++) {
            Produto *pr = buscar_produto_por_id(itens[i].id_produto);
            if (pr) {
                bruto += pr->preco * itens[i].quantidade;
                total_un += itens[i].quantidade;
            }
        }
        int dias = 4;
        double frete = calcular_frete_pedido(cep, bruto, total_un, &dias);
        char resp[256];
        int len = snprintf(resp, sizeof(resp),
                           "{\"ok\":true,\"valor_frete\":%.2f,\"prazo_dias\":%d,\"frete_gratis\":%s}",
                           frete, dias, (frete == 0.0 && total_un > 0) ? "true" : "false");
        enviar_resposta_http(cliente, 200, "OK", "application/json", resp, (size_t)len);
    } else if (strcmp(metodo, "POST") == 0 && strcmp(rota, "/api/checkout") == 0) {
        if (!auth) {
            enviar_json_erro(cliente, 401, "Unauthorized", "Faca login antes de finalizar o checkout (UC05 inclui UC02).");
        } else {
            char endereco[160], cep[16], forma[24], cupom[24];
            if (!json_extrair_string(corpo, "endereco", endereco, sizeof(endereco)) || strlen(endereco) < 5) {
                copiar_seguro(endereco, auth->endereco, sizeof(endereco));
            }
            if (!json_extrair_string(corpo, "cep", cep, sizeof(cep)) || strlen(cep) < 5) {
                copiar_seguro(cep, "01310-100", sizeof(cep));
            }
            if (!json_extrair_string(corpo, "forma_pagamento", forma, sizeof(forma))) {
                copiar_seguro(forma, "PIX", sizeof(forma));
            }
            json_extrair_string(corpo, "cupom", cupom, sizeof(cupom));
            int simular_recusa = json_extrair_int(corpo, "simular_recusa", 0);

            ItemCarrinho itens[MAX_ITENS_CARRINHO];
            int qtd_itens = parsear_itens_json(corpo, itens, MAX_ITENS_CARRINHO);

            Pedido pedido_gerado;
            char msg_erro[256] = {0};
            int alertas = 0;

            if (!executar_checkout_atomico(auth, endereco, cep, forma, cupom, simular_recusa,
                                           itens, qtd_itens, &pedido_gerado, msg_erro, sizeof(msg_erro), &alertas)) {
                enviar_json_erro(cliente, 400, "Bad Request", msg_erro);
            } else {
                char resp[768];
                int len = snprintf(
                    resp, sizeof(resp),
                    "{\"ok\":true,\"codigo_pedido\":\"%s\",\"codigo_rastreamento\":\"%s\","
                    "\"status_pedido\":\"%s\",\"status_transacao\":\"%s\",\"codigo_autorizacao\":\"%s\","
                    "\"valor_bruto\":%.2f,\"desconto\":%.2f,\"valor_frete\":%.2f,\"valor_total\":%.2f,"
                    "\"alertas_estoque\":%d,\"data_hora\":\"%s\"}",
                    pedido_gerado.codigo_pedido, pedido_gerado.codigo_rastreamento,
                    pedido_gerado.status_pedido, pedido_gerado.pagamento.status_transacao,
                    pedido_gerado.pagamento.codigo_autorizacao,
                    pedido_gerado.valor_bruto, pedido_gerado.desconto,
                    pedido_gerado.valor_frete, pedido_gerado.valor_total,
                    alertas, pedido_gerado.data_hora
                );
                enviar_resposta_http(cliente, 200, "OK", "application/json", resp, (size_t)len);
            }
        }
    } else if (strcmp(metodo, "GET") == 0 && strcmp(rota, "/api/pedidos") == 0) {
        if (!auth) {
            enviar_json_erro(cliente, 401, "Unauthorized", "Faca login para consultar seus pedidos (UC08).");
        } else {
            int eh_admin = (strcmp(auth->papel, "ADMIN") == 0);
            char *json = (char *)malloc(65536);
            if (!json) {
                enviar_json_erro(cliente, 500, "Internal Server Error", "Memoria insuficiente.");
            } else {
                size_t pos = (size_t)snprintf(json, 65536, "[");
                int primeiro_ped = 1;
                for (int i = g_total_pedidos - 1; i >= 0 && pos + 2048 < 65536; i--) {
                    Pedido *ped = &g_pedidos[i];
                    if (!eh_admin && ped->id_usuario != auth->id) continue;

                    char cli_e[160], end_e[240];
                    json_escapar_string(ped->nome_cliente, cli_e, sizeof(cli_e));
                    json_escapar_string(ped->endereco_entrega, end_e, sizeof(end_e));

                    pos += (size_t)snprintf(
                        json + pos, 65536 - pos,
                        "%s{\"id_pedido\":%d,\"codigo_pedido\":\"%s\",\"cliente\":\"%s\",\"endereco\":\"%s\","
                        "\"cep\":\"%s\",\"valor_bruto\":%.2f,\"desconto\":%.2f,\"valor_frete\":%.2f,"
                        "\"valor_total\":%.2f,\"forma_pagamento\":\"%s\",\"status_transacao\":\"%s\","
                        "\"codigo_autorizacao\":\"%s\",\"status_pedido\":\"%s\",\"codigo_rastreamento\":\"%s\","
                        "\"data_hora\":\"%s\",\"itens\":[",
                        primeiro_ped ? "" : ",",
                        ped->id_pedido, ped->codigo_pedido, cli_e, end_e, ped->cep_entrega,
                        ped->valor_bruto, ped->desconto, ped->valor_frete, ped->valor_total,
                        ped->pagamento.forma_pagamento, ped->pagamento.status_transacao,
                        ped->pagamento.codigo_autorizacao, ped->status_pedido,
                        ped->codigo_rastreamento, ped->data_hora
                    );
                    for (int j = 0; j < ped->total_itens && pos + 512 < 65536; j++) {
                        char pn_e[180];
                        json_escapar_string(ped->itens[j].nome_produto, pn_e, sizeof(pn_e));
                        pos += (size_t)snprintf(
                            json + pos, 65536 - pos,
                            "%s{\"id\":%d,\"nome\":\"%s\",\"quantidade\":%d,\"preco_unitario\":%.2f,\"subtotal\":%.2f}",
                            (j == 0) ? "" : ",",
                            ped->itens[j].id_produto, pn_e, ped->itens[j].quantidade,
                            ped->itens[j].preco_unitario, ped->itens[j].subtotal
                        );
                    }
                    pos += (size_t)snprintf(json + pos, 65536 - pos, "]}");
                    primeiro_ped = 0;
                }
                snprintf(json + pos, 65536 - pos, "]");
                enviar_resposta_http(cliente, 200, "OK", "application/json", json, strlen(json));
                free(json);
            }
        }
    } else if (strcmp(metodo, "PUT") == 0 && strcmp(rota, "/api/pedidos/status") == 0) {
        if (!auth || strcmp(auth->papel, "ADMIN") != 0) {
            enviar_json_erro(cliente, 403, "Forbidden", "Apenas Administrador pode atualizar status logistico.");
        } else {
            int id_ped = json_extrair_int(corpo, "id_pedido", 0);
            char novo_status[32];
            json_extrair_string(corpo, "status_pedido", novo_status, sizeof(novo_status));
            Pedido *alvo = NULL;
            for (int i = 0; i < g_total_pedidos; i++) {
                if (g_pedidos[i].id_pedido == id_ped) { alvo = &g_pedidos[i]; break; }
            }
            if (!alvo || strlen(novo_status) < 4) {
                enviar_json_erro(cliente, 404, "Not Found", "Pedido nao encontrado ou status invalido.");
            } else {
                copiar_seguro(alvo->status_pedido, novo_status, sizeof(alvo->status_pedido));
                salvar_pedidos_arquivo();
                char resp[256];
                int len = snprintf(resp, sizeof(resp), "{\"ok\":true,\"id_pedido\":%d,\"status_pedido\":\"%s\"}",
                                   alvo->id_pedido, alvo->status_pedido);
                enviar_resposta_http(cliente, 200, "OK", "application/json", resp, (size_t)len);
            }
        }
    } else {
        enviar_json_erro(cliente, 404, "Not Found", "Endpoint nao encontrado.");
    }

    free(buffer);
    FECHAR_SOCKET(cliente);
}

static void iniciar_servidor_web(int abrir_navegador) {
    #ifdef _WIN32
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("[ERRO] Falha ao inicializar Winsock2.\n");
        return;
    }
    #endif

    socket_t srv = socket(AF_INET, SOCK_STREAM, 0);
    if (srv == INVALID_SOCKET) {
        printf("[ERRO] Nao foi possivel criar socket TCP.\n");
        return;
    }

    int opt = 1;
    setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt));

    struct sockaddr_in addr;
    int porta_ativa = g_porta_preferida;
    int bind_ok = 0;

    for (int tentativa = 0; tentativa < 10; tentativa++) {
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons((uint16_t)(porta_ativa + tentativa));

        if (bind(srv, (struct sockaddr *)&addr, sizeof(addr)) != SOCKET_ERROR) {
            porta_ativa = porta_ativa + tentativa;
            g_porta_preferida = porta_ativa;
            bind_ok = 1;
            break;
        }
        printf("[AVISO] Porta %d ocupada, tentando porta %d...\n",
               porta_ativa + tentativa, porta_ativa + tentativa + 1);
    }

    if (!bind_ok) {
        printf("[ERRO] Nenhuma porta livre encontrada entre %d e %d.\n",
               g_porta_preferida, g_porta_preferida + 9);
        FECHAR_SOCKET(srv);
        return;
    }

    if (listen(srv, 32) == SOCKET_ERROR) {
        printf("[ERRO] Falha ao colocar socket em modo LISTEN.\n");
        FECHAR_SOCKET(srv);
        return;
    }

    printf("\n[OK] Servidor HTTP/REST em C ativo na porta %d!\n", porta_ativa);
    printf("     URL Local: http://localhost:%d/\n", porta_ativa);
    printf("     Pressione Ctrl+C no terminal para encerrar o servidor.\n");
    fflush(stdout);

    #ifdef _WIN32
    if (abrir_navegador) {
        char url[96];
        snprintf(url, sizeof(url), "http://localhost:%d/", porta_ativa);
        ShellExecuteA(NULL, "open", url, NULL, NULL, SW_SHOWNORMAL);
    }
    #endif

    while (1) {
        struct sockaddr_in cli_addr;
        socklen_t cli_len = sizeof(cli_addr);
        socket_t cli = accept(srv, (struct sockaddr *)&cli_addr, &cli_len);
        if (cli != INVALID_SOCKET) {
            tratar_requisicao_http(cli);
        }
    }

    FECHAR_SOCKET(srv);
    #ifdef _WIN32
    WSACleanup();
    #endif
}

/* ============================================================================
 * MENU INTERATIVO DE TERMINAL (CONSOLE CLI)
 * ============================================================================ */
static void terminal_listar_produtos(void) {
    printf("\n=== CATALOGO CYBERHARDWARE (%d PRODUTOS) ===\n", g_total_produtos);
    for (int i = 0; i < g_total_produtos; i++) {
        Produto *p = &g_produtos[i];
        printf("[%02d] %-42s | %-15s | R$ %8.2f | Estoque: %2d (Min %d) | %s%s\n",
               p->id, p->nome, p->categoria, p->preco, p->estoque, p->estoque_minimo,
               p->ativo ? "ATIVO" : "INATIVO",
               (p->estoque <= p->estoque_minimo) ? " [ALERTA ESTOQUE]" : "");
    }
}

static void terminal_alertas_estoque(void) {
    printf("\n=== MONITORAMENTO DE ESTOQUE MINIMO (RF03 / UC07) ===\n");
    int encontrados = 0;
    for (int i = 0; i < g_total_produtos; i++) {
        Produto *p = &g_produtos[i];
        if (p->ativo && p->estoque <= p->estoque_minimo) {
            printf("[ALERTA] ID %d - %s | Atual: %d | Minimo: %d\n",
                   p->id, p->nome, p->estoque, p->estoque_minimo);
            encontrados++;
        }
    }
    if (!encontrados) printf("Nenhum produto abaixo ou igual ao estoque minimo.\n");
}

static void terminal_ajustar_estoque(void) {
    terminal_listar_produtos();
    int id, novo_est;
    printf("\nInforme o ID do produto para reposicao/ajuste (0 cancela): ");
    if (scanf("%d", &id) != 1 || id == 0) { limpar_buffer_entrada(); return; }
    Produto *p = buscar_produto_por_id(id);
    if (!p) { limpar_buffer_entrada(); printf("[!] Produto nao encontrado.\n"); return; }
    printf("Estoque atual de '%s': %d. Novo estoque: ", p->nome, p->estoque);
    if (scanf("%d", &novo_est) == 1 && novo_est >= 0) {
        p->estoque = novo_est;
        salvar_produtos_arquivo();
        printf("[OK] Estoque atualizado e persistido.\n");
    }
    limpar_buffer_entrada();
}

static void terminal_venda_direta(void) {
    char email[100], senha[80], cep[16], forma[24], cupom[24];
    limpar_buffer_entrada();
    printf("\n=== CHECKOUT CONSOLE (UC05 inclui UC02 e Gateway) ===\n");
    printf("E-mail do cliente (ex: cliente@cyberhardware.com.br): ");
    ler_linha_segura(email, sizeof(email));
    printf("Senha: ");
    ler_linha_segura(senha, sizeof(senha));

    Usuario *u = buscar_usuario_por_email(email);
    if (!u || !u->ativo || !verificar_hash_senha(senha, u->senha_hash)) {
        printf("[!] Credenciais invalidas ou conta inativa.\n");
        return;
    }

    terminal_listar_produtos();
    ItemCarrinho itens[MAX_ITENS_CARRINHO];
    int qtd_itens = 0;
    while (qtd_itens < MAX_ITENS_CARRINHO) {
        int id_p, q_p;
        printf("ID do produto (0 para encerrar carrinho): ");
        if (scanf("%d", &id_p) != 1 || id_p == 0) { limpar_buffer_entrada(); break; }
        printf("Quantidade: ");
        if (scanf("%d", &q_p) != 1 || q_p <= 0) { limpar_buffer_entrada(); continue; }
        memset(&itens[qtd_itens], 0, sizeof(ItemCarrinho));
        itens[qtd_itens].id_produto = id_p;
        itens[qtd_itens].quantidade = q_p;
        qtd_itens++;
    }

    printf("CEP de entrega (ex: 01310-100): ");
    ler_linha_segura(cep, sizeof(cep));
    printf("Forma pagamento (PIX / CARTAO / BOLETO): ");
    ler_linha_segura(forma, sizeof(forma));
    printf("Cupom opcional (TECH10 / CYBER20 / Enter): ");
    ler_linha_segura(cupom, sizeof(cupom));

    Pedido ped;
    char err[256];
    int alertas = 0;
    if (executar_checkout_atomico(u, u->endereco, cep, forma, cupom, 0,
                                  itens, qtd_itens, &ped, err, sizeof(err), &alertas)) {
        printf("\n[OK] Pedido %s aprovado! Rastreio: %s | Total: R$ %.2f (Frete: R$ %.2f)\n",
               ped.codigo_pedido, ped.codigo_rastreamento, ped.valor_total, ped.valor_frete);
    } else {
        printf("\n[ERRO] %s\n", err);
    }
}

static void terminal_historico_pedidos(void) {
    printf("\n=== HISTORICO DE PEDIDOS (%d REGISTROS) ===\n", g_total_pedidos);
    for (int i = 0; i < g_total_pedidos; i++) {
        Pedido *p = &g_pedidos[i];
        printf("#%d | %s | %s | Total R$ %.2f (Frete R$ %.2f) | %s | Rastreio: %s | %s\n",
               p->id_pedido, p->codigo_pedido, p->nome_cliente, p->valor_total, p->valor_frete,
               p->pagamento.forma_pagamento, p->codigo_rastreamento, p->status_pedido);
    }
}

int main(int argc, char *argv[]) {
    /* Mantem LC_NUMERIC em "C" para garantir ponto decimal valido em JSON e arquivos TXT */
    setlocale(LC_CTYPE, "Portuguese");
    setlocale(LC_NUMERIC, "C");
    #ifdef _WIN32
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
    #endif

    carregar_usuarios_arquivo();
    carregar_produtos_arquivo();
    carregar_pedidos_arquivo();

    int server_only = 0;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--server-only") == 0) {
            server_only = 1;
        } else if (strcmp(argv[i], "--port") == 0 && i + 1 < argc) {
            int p = atoi(argv[++i]);
            if (p > 0 && p <= 65535) g_porta_preferida = p;
        }
    }

    if (server_only) {
        iniciar_servidor_web(0);
        return 0;
    }

    int opcao = -1;
    do {
        printf("\n============================================================\n");
        printf("   CYBERHARDWARE E-COMMERCE - BACKEND C (ETAPA 5 / PIM II)\n");
        printf("============================================================\n");
        printf(" [1] Iniciar Servidor Web (Porta 8080) e Abrir Loja SPA\n");
        printf(" [2] Listar Catalogo de Produtos e Estoque (RF02)\n");
        printf(" [3] Verificar Alertas de Estoque Minimo (RF03)\n");
        printf(" [4] Ajustar Estoque Manualmente (UC07)\n");
        printf(" [5] Realizar Venda / Checkout Autenticado no Terminal (UC05)\n");
        printf(" [6] Consultar Historico e Rastreamento de Pedidos (RF06)\n");
        printf(" [0] Sair\n");
        printf("------------------------------------------------------------\n");
        printf("Escolha uma opcao: ");
        if (scanf("%d", &opcao) != 1) {
            limpar_buffer_entrada();
            continue;
        }

        switch (opcao) {
            case 1: iniciar_servidor_web(1); break;
            case 2: terminal_listar_produtos(); break;
            case 3: terminal_alertas_estoque(); break;
            case 4: terminal_ajustar_estoque(); break;
            case 5: terminal_venda_direta(); break;
            case 6: terminal_historico_pedidos(); break;
            case 0: printf("Encerrando sistema...\n"); break;
            default: printf("Opcao invalida.\n"); break;
        }
    } while (opcao != 0);

    return 0;
}
