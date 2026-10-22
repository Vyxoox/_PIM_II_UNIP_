#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <shellapi.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "shell32.lib")

#define PORT 8080
#define BUFFER_SIZE 65536
#define MAX_PRODUCTS 32
#define MAX_CART_ITEMS 64
#define MAX_ORDERS 128

// Estrutura de Produto
typedef struct {
    int id;
    char name[128];
    char category[64];
    double price;
    double oldPrice;
    int stock;
    char imageUrl[256];
    char description[256];
    char specs[256];
    char badge[32];
    double rating;
    int reviews;
} Product;

// Estrutura de Item no Carrinho
typedef struct {
    int productId;
    int quantity;
} CartItem;

// Estrutura de Pedido
typedef struct {
    int orderId;
    char orderCode[32];
    char customerName[128];
    char customerEmail[128];
    char customerCpf[32];
    char customerAddress[256];
    char paymentMethod[32];
    double totalAmount;
    double discount;
    int itemsCount;
    char timestamp[64];
} Order;

// Variáveis Globais de Estado do E-commerce
static Product g_products[MAX_PRODUCTS];
static int g_productsCount = 0;

static CartItem g_cart[MAX_CART_ITEMS];
static int g_cartCount = 0;

static Order g_orders[MAX_ORDERS];
static int g_ordersCount = 0;

static LONG g_totalRequests = 0;
static CRITICAL_SECTION g_storeMutex;

// Helper para obter data/hora formatada
void get_current_timestamp(char *buffer, size_t max_len) {
    time_t rawtime;
    struct tm *timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(buffer, max_len, "%d/%m/%Y %H:%M:%S", timeinfo);
}

// Inicialização do Catálogo de Produtos de Informática & Eletrônicos
void init_store_catalog() {
    g_productsCount = 0;

    // 1. Placa de Vídeo RTX 4080
    g_products[g_productsCount++] = (Product){
        .id = 1,
        .name = "Placa de Video RTX 4080 Super 16GB GDDR6X",
        .category = "Placas de Video",
        .price = 7299.90,
        .oldPrice = 8199.00,
        .stock = 7,
        .imageUrl = "https://images.unsplash.com/photo-1587202372775-e229f172b9d7?auto=format&fit=crop&w=700&q=80",
        .description = "Performance extrema com arquitetura Ada Lovelace, DLSS 3.5 e Ray Tracing em 4K real.",
        .specs = "16GB GDDR6X | 256-bit | 10240 CUDA Cores | Tri-Fan RGB",
        .badge = "Destaque",
        .rating = 4.9,
        .reviews = 128
    };

    // 2. Processador AMD Ryzen 7 7800X3D
    g_products[g_productsCount++] = (Product){
        .id = 2,
        .name = "Processador AMD Ryzen 7 7800X3D AM5",
        .category = "Processadores",
        .price = 2899.00,
        .oldPrice = 3399.00,
        .stock = 12,
        .imageUrl = "https://images.unsplash.com/photo-1591799264318-7e6ef8ddb7ea?auto=format&fit=crop&w=700&q=80",
        .description = "O campeao definitivo para games com tecnologia 3D V-Cache de altissima velocidade.",
        .specs = "8 Cores / 16 Threads | Cache 104MB | Boost ate 5.0GHz | Soquete AM5",
        .badge = "Top Vendas",
        .rating = 5.0,
        .reviews = 310
    };

    // 3. Teclado Mecanico Gamer Custom
    g_products[g_productsCount++] = (Product){
        .id = 3,
        .name = "Teclado Mecanico Custom RGB Hot-Swap Switch Brown",
        .category = "Perifericos",
        .price = 459.90,
        .oldPrice = 599.90,
        .stock = 25,
        .imageUrl = "https://images.unsplash.com/photo-1587829741301-dc798b83add3?auto=format&fit=crop&w=700&q=80",
        .description = "Construcao premium em aluminio CNC, teclas PBT Double-Shot e iluminacao customizavel.",
        .specs = "Switch Mecanico Lubrificado | Conexao USB-C / Wireless | Teclas PBT",
        .badge = "Oferta",
        .rating = 4.8,
        .reviews = 89
    };

    // 4. Mouse Gamer Sem Fio Ultraleve
    g_products[g_productsCount++] = (Product){
        .id = 4,
        .name = "Mouse Gamer Wireless Ultraleve 49g 26.000 DPI",
        .category = "Perifericos",
        .price = 379.00,
        .oldPrice = 450.00,
        .stock = 18,
        .imageUrl = "https://images.unsplash.com/photo-1615663245857-ac93bb7c39e7?auto=format&fit=crop&w=700&q=80",
        .description = "Sensor optico flagship PAW3395 com zero delay e bateria de ate 100 horas continuas.",
        .specs = "Sensor PAW3395 | Polling Rate 4000Hz | Peso 49g | Bateria 100h",
        .badge = "Popular",
        .rating = 4.9,
        .reviews = 215
    };

    // 5. Monitor Gamer OLED 27" 240Hz
    g_products[g_productsCount++] = (Product){
        .id = 5,
        .name = "Monitor Gamer 27 Pol OLED QHD 240Hz 0.03ms",
        .category = "Monitores",
        .price = 4999.00,
        .oldPrice = 5799.00,
        .stock = 5,
        .imageUrl = "https://images.unsplash.com/photo-1527443224154-c4a3942d3acf?auto=format&fit=crop&w=700&q=80",
        .description = "Cores puras com painel OLED verdadeiro, resposta instantanea de 0.03ms e fluidez sem igual.",
        .specs = "Resolucao 2560x1440 | OLED 240Hz | HDR True Black 400 | G-Sync Ready",
        .badge = "Premium",
        .rating = 5.0,
        .reviews = 76
    };

    // 6. Memoria RAM DDR5 32GB RGB
    g_products[g_productsCount++] = (Product){
        .id = 6,
        .name = "Kit Memoria RAM DDR5 32GB (2x16GB) 6000MHz CL30 RGB",
        .category = "Memorias & SSD",
        .price = 849.90,
        .oldPrice = 999.00,
        .stock = 20,
        .imageUrl = "https://images.unsplash.com/photo-1562976540-1502c2145186?auto=format&fit=crop&w=700&q=80",
        .description = "Baixa latencia CL30 com perfil AMD EXPO e Intel XMP 3.0 para estavel overclock.",
        .specs = "2x16GB | 6000 MT/s | Timings CL30-38-38-96 | RGB Sync",
        .badge = "",
        .rating = 4.8,
        .reviews = 94
    };

    // 7. SSD NVMe 2TB PCIe 4.0
    g_products[g_productsCount++] = (Product){
        .id = 7,
        .name = "SSD M.2 NVMe 2TB PCIe 4.0 7.450 MB/s",
        .category = "Memorias & SSD",
        .price = 920.00,
        .oldPrice = 1150.00,
        .stock = 15,
        .imageUrl = "https://images.unsplash.com/photo-1597872200969-2b65d56bd16b?auto=format&fit=crop&w=700&q=80",
        .description = "Velocidade maxima para instalacao de jogos e edicao de video 4K sem engasgos.",
        .specs = "Leitura: 7450MB/s | Gravacao: 6900MB/s | Dissipador de Calor Incluso",
        .badge = "Top Vendas",
        .rating = 4.9,
        .reviews = 180
    };

    // 8. Headset Wireless 7.1 Surround
    g_products[g_productsCount++] = (Product){
        .id = 8,
        .name = "Headset Gamer Sem Fio 7.1 Espacial com Microfone",
        .category = "Perifericos",
        .price = 629.90,
        .oldPrice = 799.00,
        .stock = 14,
        .imageUrl = "https://images.unsplash.com/photo-1546435770-a3e426bf472b?auto=format&fit=crop&w=700&q=80",
        .description = "Drivers de 50mm em Neodimio com espuma memoria ultra confortavel para longas sessoes.",
        .specs = "Conexao 2.4GHz + BT | Microfone com Cancelamento de Ruido | Bateria 50h",
        .badge = "",
        .rating = 4.7,
        .reviews = 112
    };

    // 9. Gabinete Aquario Full Tower
    g_products[g_productsCount++] = (Product){
        .id = 9,
        .name = "Gabinete Gamer Aquario Vidro Temperado Dual Chamber",
        .category = "Acessorios",
        .price = 549.00,
        .oldPrice = 699.00,
        .stock = 8,
        .imageUrl = "https://images.unsplash.com/photo-1587202372634-32705e3bf49c?auto=format&fit=crop&w=700&q=80",
        .description = "Design panoramico sem coluna frontal com suporte a radiadores de ate 360mm.",
        .specs = "Suporta Placas E-ATX, ATX | 10 Slots para Fans 120mm | Porta USB-C 3.2",
        .badge = "Novo",
        .rating = 4.8,
        .reviews = 64
    };

    // 10. Water Cooler 360mm Display LCD
    g_products[g_productsCount++] = (Product){
        .id = 10,
        .name = "Water Cooler 360mm ARGB com Display LCD Customizavel",
        .category = "Acessorios",
        .price = 1199.00,
        .oldPrice = 1499.00,
        .stock = 10,
        .imageUrl = "https://images.unsplash.com/photo-1555680202-c86f0e12f086?auto=format&fit=crop&w=700&q=80",
        .description = "Tela IPS 2.4 Pol na bomba para monitorar temperatura ou exibir GIFs em tempo real.",
        .specs = "Radiador 360mm Aluminio | 3x Fans ARGB Silenciosas | Compativel AM5 e LGA1700",
        .badge = "Lancamento",
        .rating = 4.9,
        .reviews = 53
    };

    // 11. Cadeira Gamer Ergonomica
    g_products[g_productsCount++] = (Product){
        .id = 11,
        .name = "Cadeira Ergonomica Mesh Respiravel com Apoio Lombar 3D",
        .category = "Acessorios",
        .price = 1290.00,
        .oldPrice = 1600.00,
        .stock = 6,
        .imageUrl = "https://images.unsplash.com/photo-1598550476439-6847785fcea6?auto=format&fit=crop&w=700&q=80",
        .description = "Ergonomia avancada com tecido em malha mesh respiravel para conforto total.",
        .specs = "Apoio de Braco 3D | Pistao Classe 4 | Reclinavel ate 135 graus | Ate 150kg",
        .badge = "",
        .rating = 4.8,
        .reviews = 142
    };

    // 12. Microfone Condensador Streaming USB
    g_products[g_productsCount++] = (Product){
        .id = 12,
        .name = "Microfone Condensador Profissional USB com Braco Articulado",
        .category = "Perifericos",
        .price = 329.90,
        .oldPrice = 420.00,
        .stock = 16,
        .imageUrl = "https://images.unsplash.com/photo-1590658268037-6bf12165a8df?auto=format&fit=crop&w=700&q=80",
        .description = "Capsula cardioide de alta definicao com botao de mute touch e iluminacao RGB.",
        .specs = "Taxa 192kHz/24-bit | Padrao Cardioide | Pop Filter e Shock Mount Inclusos",
        .badge = "Custo-Beneficio",
        .rating = 4.7,
        .reviews = 98
    };
}

// Helpers simples de JSON em C
int json_get_int(const char *json, const char *key, int default_val) {
    if (!json) return default_val;
    const char *pos = strstr(json, key);
    if (!pos) return default_val;
    pos += strlen(key);
    while (*pos && *pos != ':') pos++;
    if (!*pos) return default_val;
    pos++;
    while (*pos == ' ' || *pos == '\t' || *pos == '\"') pos++;
    return atoi(pos);
}

void json_get_str(const char *json, const char *key, char *out, size_t max_out) {
    out[0] = '\0';
    if (!json) return;
    const char *pos = strstr(json, key);
    if (!pos) return;
    pos += strlen(key);
    while (*pos && *pos != ':') pos++;
    if (!*pos) return;
    pos++;
    while (*pos == ' ' || *pos == '\t') pos++;
    if (*pos == '\"') {
        pos++;
        size_t idx = 0;
        while (*pos != '\"' && *pos != '\0' && idx + 1 < max_out) {
            if (*pos == '\\' && *(pos + 1) != '\0') pos++;
            out[idx++] = *pos++;
        }
        out[idx] = '\0';
    }
}

// Envio de Resposta HTTP
void send_http_response(SOCKET client, int status_code, const char *status_text, const char *content_type, const char *body, int body_len) {
    char header[1024];
    int header_len = sprintf(header,
        "HTTP/1.1 %d %s\r\n"
        "Content-Type: %s; charset=utf-8\r\n"
        "Content-Length: %d\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
        "Access-Control-Allow-Headers: Content-Type\r\n"
        "Connection: close\r\n"
        "\r\n",
        status_code, status_text, content_type, body_len);

    send(client, header, header_len, 0);
    if (body && body_len > 0) {
        send(client, body, body_len, 0);
    }
}

// Handlers de API
void handle_get_products(SOCKET client) {
    EnterCriticalSection(&g_storeMutex);
    char *buffer = (char *)malloc(131072);
    if (!buffer) {
        LeaveCriticalSection(&g_storeMutex);
        send_http_response(client, 500, "Internal Server Error", "application/json", "{\"error\":\"Memory\"}", 18);
        return;
    }

    int offset = sprintf(buffer, "[");
    for (int i = 0; i < g_productsCount; ++i) {
        Product *p = &g_products[i];
        offset += sprintf(buffer + offset,
            "%s{\"id\":%d,\"name\":\"%s\",\"category\":\"%s\",\"price\":%.2f,\"oldPrice\":%.2f,\"stock\":%d,\"imageUrl\":\"%s\",\"description\":\"%s\",\"specs\":\"%s\",\"badge\":\"%s\",\"rating\":%.1f,\"reviews\":%d}",
            (i > 0 ? "," : ""),
            p->id, p->name, p->category, p->price, p->oldPrice, p->stock, p->imageUrl, p->description, p->specs, p->badge, p->rating, p->reviews
        );
    }
    offset += sprintf(buffer + offset, "]");
    LeaveCriticalSection(&g_storeMutex);

    send_http_response(client, 200, "OK", "application/json", buffer, offset);
    free(buffer);
}

void handle_get_cart(SOCKET client) {
    EnterCriticalSection(&g_storeMutex);
    char *buffer = (char *)malloc(65536);
    if (!buffer) {
        LeaveCriticalSection(&g_storeMutex);
        return;
    }

    double subtotal = 0.0;
    int totalItems = 0;

    int offset = sprintf(buffer, "{\"items\":[");
    int itemWritten = 0;

    for (int i = 0; i < g_cartCount; ++i) {
        int pid = g_cart[i].productId;
        int qty = g_cart[i].quantity;
        // Encontra produto correspondente
        Product *p = NULL;
        for (int j = 0; j < g_productsCount; ++j) {
            if (g_products[j].id == pid) {
                p = &g_products[j];
                break;
            }
        }
        if (p) {
            subtotal += p->price * qty;
            totalItems += qty;
            offset += sprintf(buffer + offset,
                "%s{\"productId\":%d,\"name\":\"%s\",\"price\":%.2f,\"quantity\":%d,\"imageUrl\":\"%s\",\"stock\":%d}",
                (itemWritten > 0 ? "," : ""),
                p->id, p->name, p->price, qty, p->imageUrl, p->stock
            );
            itemWritten++;
        }
    }

    offset += sprintf(buffer + offset,
        "],\"subtotal\":%.2f,\"count\":%d,\"shipping\":0.0}",
        subtotal, totalItems);

    LeaveCriticalSection(&g_storeMutex);

    send_http_response(client, 200, "OK", "application/json", buffer, offset);
    free(buffer);
}

void handle_cart_add(SOCKET client, const char *body) {
    int productId = json_get_int(body, "productId", 0);
    int quantity = json_get_int(body, "quantity", 1);
    if (quantity <= 0) quantity = 1;

    EnterCriticalSection(&g_storeMutex);

    // Encontra produto
    Product *p = NULL;
    for (int i = 0; i < g_productsCount; ++i) {
        if (g_products[i].id == productId) {
            p = &g_products[i];
            break;
        }
    }

    if (!p) {
        LeaveCriticalSection(&g_storeMutex);
        char res[] = "{\"success\":false,\"message\":\"Produto nao encontrado\"}";
        send_http_response(client, 404, "Not Found", "application/json", res, (int)strlen(res));
        return;
    }

    if (p->stock < quantity) {
        LeaveCriticalSection(&g_storeMutex);
        char res[] = "{\"success\":false,\"message\":\"Estoque insuficiente\"}";
        send_http_response(client, 400, "Bad Request", "application/json", res, (int)strlen(res));
        return;
    }

    // Verifica se ja esta no carrinho
    int found = 0;
    for (int i = 0; i < g_cartCount; ++i) {
        if (g_cart[i].productId == productId) {
            if (p->stock >= g_cart[i].quantity + quantity) {
                g_cart[i].quantity += quantity;
            } else {
                LeaveCriticalSection(&g_storeMutex);
                char res[] = "{\"success\":false,\"message\":\"Limite de estoque atingido\"}";
                send_http_response(client, 400, "Bad Request", "application/json", res, (int)strlen(res));
                return;
            }
            found = 1;
            break;
        }
    }

    if (!found && g_cartCount < MAX_CART_ITEMS) {
        g_cart[g_cartCount].productId = productId;
        g_cart[g_cartCount].quantity = quantity;
        g_cartCount++;
    }

    LeaveCriticalSection(&g_storeMutex);

    char res[] = "{\"success\":true,\"message\":\"Adicionado com sucesso\"}";
    send_http_response(client, 200, "OK", "application/json", res, (int)strlen(res));
}

void handle_cart_update(SOCKET client, const char *body) {
    int productId = json_get_int(body, "productId", 0);
    int delta = json_get_int(body, "delta", 0);

    EnterCriticalSection(&g_storeMutex);

    // Encontra produto
    Product *p = NULL;
    for (int i = 0; i < g_productsCount; ++i) {
        if (g_products[i].id == productId) {
            p = &g_products[i];
            break;
        }
    }

    for (int i = 0; i < g_cartCount; ++i) {
        if (g_cart[i].productId == productId) {
            int newQty = g_cart[i].quantity + delta;
            if (newQty <= 0) {
                // Remove item
                for (int j = i; j < g_cartCount - 1; ++j) {
                    g_cart[j] = g_cart[j + 1];
                }
                g_cartCount--;
            } else if (p && newQty <= p->stock) {
                g_cart[i].quantity = newQty;
            }
            break;
        }
    }

    LeaveCriticalSection(&g_storeMutex);

    char res[] = "{\"success\":true}";
    send_http_response(client, 200, "OK", "application/json", res, (int)strlen(res));
}

void handle_cart_remove(SOCKET client, const char *body) {
    int productId = json_get_int(body, "productId", 0);

    EnterCriticalSection(&g_storeMutex);
    for (int i = 0; i < g_cartCount; ++i) {
        if (g_cart[i].productId == productId) {
            for (int j = i; j < g_cartCount - 1; ++j) {
                g_cart[j] = g_cart[j + 1];
            }
            g_cartCount--;
            break;
        }
    }
    LeaveCriticalSection(&g_storeMutex);

    char res[] = "{\"success\":true}";
    send_http_response(client, 200, "OK", "application/json", res, (int)strlen(res));
}

void handle_checkout(SOCKET client, const char *body) {
    char name[128], email[128], cpf[32], addr[256], method[32];
    json_get_str(body, "customerName", name, sizeof(name));
    json_get_str(body, "customerEmail", email, sizeof(email));
    json_get_str(body, "customerCpf", cpf, sizeof(cpf));
    json_get_str(body, "customerAddress", addr, sizeof(addr));
    json_get_str(body, "paymentMethod", method, sizeof(method));
    int discountPct = json_get_int(body, "discountPercent", 0);

    EnterCriticalSection(&g_storeMutex);

    if (g_cartCount == 0) {
        LeaveCriticalSection(&g_storeMutex);
        char res[] = "{\"success\":false,\"message\":\"Carrinho vazio\"}";
        send_http_response(client, 400, "Bad Request", "application/json", res, (int)strlen(res));
        return;
    }

    double subtotal = 0.0;
    int itemsCount = 0;

    // Reduz estoque em C
    for (int i = 0; i < g_cartCount; ++i) {
        int pid = g_cart[i].productId;
        int qty = g_cart[i].quantity;
        for (int j = 0; j < g_productsCount; ++j) {
            if (g_products[j].id == pid) {
                if (g_products[j].stock >= qty) {
                    g_products[j].stock -= qty;
                } else {
                    g_products[j].stock = 0;
                }
                subtotal += g_products[j].price * qty;
                itemsCount += qty;
                break;
            }
        }
    }

    double discount = (subtotal * discountPct) / 100.0;
    double total = subtotal - discount;
    if (total < 0) total = 0;

    // Registra pedido
    char orderCode[32];
    sprintf(orderCode, "#CS-%04d", rand() % 9000 + 1000);

    char timestamp[64];
    get_current_timestamp(timestamp, sizeof(timestamp));

    if (g_ordersCount < MAX_ORDERS) {
        Order *ord = &g_orders[g_ordersCount++];
        ord->orderId = g_ordersCount;
        strncpy(ord->orderCode, orderCode, sizeof(ord->orderCode));
        strncpy(ord->customerName, name[0] ? name : "Cliente Convidado", sizeof(ord->customerName));
        strncpy(ord->customerEmail, email, sizeof(ord->customerEmail));
        strncpy(ord->customerCpf, cpf, sizeof(ord->customerCpf));
        strncpy(ord->customerAddress, addr, sizeof(ord->customerAddress));
        strncpy(ord->paymentMethod, method[0] ? method : "PIX", sizeof(ord->paymentMethod));
        ord->totalAmount = total;
        ord->discount = discount;
        ord->itemsCount = itemsCount;
        strncpy(ord->timestamp, timestamp, sizeof(ord->timestamp));

        // Salva resumo em arquivo local
        FILE *f = fopen("pedidos_loja.txt", "a");
        if (f) {
            fprintf(f, "[%s] Pedido %s | Cliente: %s | Total: R$ %.2f | Itens: %d | Pag: %s\n",
                    timestamp, orderCode, ord->customerName, total, itemsCount, ord->paymentMethod);
            fclose(f);
        }
    }

    // Esvazia carrinho
    g_cartCount = 0;

    LeaveCriticalSection(&g_storeMutex);

    char res[512];
    int len = sprintf(res, "{\"success\":true,\"orderCode\":\"%s\",\"total\":%.2f,\"timestamp\":\"%s\"}",
                      orderCode, total, timestamp);
    send_http_response(client, 200, "OK", "application/json", res, len);
}

void handle_get_orders(SOCKET client) {
    EnterCriticalSection(&g_storeMutex);
    char *buffer = (char *)malloc(65536);
    if (!buffer) {
        LeaveCriticalSection(&g_storeMutex);
        return;
    }

    int offset = sprintf(buffer, "[");
    for (int i = g_ordersCount - 1; i >= 0; --i) {
        Order *ord = &g_orders[i];
        offset += sprintf(buffer + offset,
            "%s{\"orderId\":%d,\"orderCode\":\"%s\",\"customerName\":\"%s\",\"paymentMethod\":\"%s\",\"totalAmount\":%.2f,\"itemsCount\":%d,\"timestamp\":\"%s\"}",
            (i < g_ordersCount - 1 ? "," : ""),
            ord->orderId, ord->orderCode, ord->customerName, ord->paymentMethod, ord->totalAmount, ord->itemsCount, ord->timestamp
        );
    }
    offset += sprintf(buffer + offset, "]");
    LeaveCriticalSection(&g_storeMutex);

    send_http_response(client, 200, "OK", "application/json", buffer, offset);
    free(buffer);
}

void handle_get_stats(SOCKET client) {
    EnterCriticalSection(&g_storeMutex);
    double totalRevenue = 0.0;
    for (int i = 0; i < g_ordersCount; ++i) {
        totalRevenue += g_orders[i].totalAmount;
    }
    int prodCount = g_productsCount;
    int ordCount = g_ordersCount;
    LeaveCriticalSection(&g_storeMutex);

    char buffer[512];
    int len = sprintf(buffer,
        "{\"requestsCount\":%ld,\"productsCount\":%d,\"ordersCount\":%d,\"totalRevenue\":%.2f,\"port\":%d}",
        g_totalRequests, prodCount, ordCount, totalRevenue, PORT);

    send_http_response(client, 200, "OK", "application/json", buffer, len);
}

// Servir o arquivo HTML
void handle_serve_file(SOCKET client, const char *filepath) {
    char exePath[MAX_PATH] = {0};
    char targetPath[MAX_PATH] = {0};
    FILE *f = NULL;

    // 1. Tenta caminho relativo ao diretorio do executavel
    GetModuleFileNameA(NULL, exePath, sizeof(exePath));
    char *lastSlash = strrchr(exePath, '\\');
    if (lastSlash) {
        *(lastSlash + 1) = '\0';
        sprintf(targetPath, "%s%s", exePath, filepath);
        f = fopen(targetPath, "rb");
        if (!f) {
            sprintf(targetPath, "%sweb\\%s", exePath, filepath);
            f = fopen(targetPath, "rb");
        }
    }

    // 2. Tenta caminhos relativos ao diretorio de trabalho atual
    if (!f) f = fopen(filepath, "rb");
    if (!f) {
        sprintf(targetPath, "web/%s", filepath);
        f = fopen(targetPath, "rb");
    }
    if (!f) {
        sprintf(targetPath, "loja_eletronicos/web/%s", filepath);
        f = fopen(targetPath, "rb");
    }

    if (!f) {
        const char *not_found = "<h1>404 - Arquivo nao encontrado</h1>";
        send_http_response(client, 404, "Not Found", "text/html", not_found, (int)strlen(not_found));
        return;
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *data = (char *)malloc(size + 1);
    if (!data) {
        fclose(f);
        return;
    }

    fread(data, 1, size, f);
    fclose(f);
    data[size] = '\0';

    send_http_response(client, 200, "OK", "text/html", data, size);
    free(data);
}

// Thread de Conexao
DWORD WINAPI client_thread(LPVOID arg) {
    SOCKET client = (SOCKET)arg;
    char buffer[BUFFER_SIZE];
    int total_bytes = recv(client, buffer, sizeof(buffer) - 1, 0);

    if (total_bytes > 0) {
        buffer[total_bytes] = '\0';
        InterlockedIncrement(&g_totalRequests);

        char method[16] = {0};
        char path[256] = {0};
        sscanf(buffer, "%15s %255s", method, path);

        // Acha o corpo da requisicao (apos \r\n\r\n)
        char *header_end = strstr(buffer, "\r\n\r\n");
        char *body = "";
        if (header_end) {
            body = header_end + 4;
            int current_body_len = total_bytes - (int)(body - buffer);
            
            // Checa Content-Length para garantir recebimento completo
            char *cl_pos = strstr(buffer, "Content-Length:");
            if (!cl_pos) cl_pos = strstr(buffer, "content-length:");
            if (cl_pos) {
                int content_len = atoi(cl_pos + 15);
                while (current_body_len < content_len && total_bytes < (int)sizeof(buffer) - 1) {
                    int n = recv(client, buffer + total_bytes, (int)sizeof(buffer) - 1 - total_bytes, 0);
                    if (n <= 0) break;
                    total_bytes += n;
                    buffer[total_bytes] = '\0';
                    current_body_len += n;
                }
            }
        }

        printf("[HTTP %ld] %s %s\n", g_totalRequests, method, path);

        if (strcmp(method, "OPTIONS") == 0) {
            send_http_response(client, 204, "No Content", "text/plain", "", 0);
        } else if (strcmp(method, "GET") == 0 || strcmp(method, "HEAD") == 0) {
            if (strcmp(path, "/") == 0 || strcmp(path, "/index.html") == 0) {
                handle_serve_file(client, "web/index.html");
            } else if (strcmp(path, "/api/products") == 0) {
                handle_get_products(client);
            } else if (strcmp(path, "/api/cart") == 0) {
                handle_get_cart(client);
            } else if (strcmp(path, "/api/orders") == 0) {
                handle_get_orders(client);
            } else if (strcmp(path, "/api/stats") == 0) {
                handle_get_stats(client);
            } else {
                handle_serve_file(client, path + 1);
            }
        } else if (strcmp(method, "POST") == 0) {
            if (strcmp(path, "/api/cart/add") == 0) {
                handle_cart_add(client, body);
            } else if (strcmp(path, "/api/cart/update") == 0) {
                handle_cart_update(client, body);
            } else if (strcmp(path, "/api/cart/remove") == 0) {
                handle_cart_remove(client, body);
            } else if (strcmp(path, "/api/checkout") == 0) {
                handle_checkout(client, body);
            } else {
                char res[] = "{\"error\":\"Rota POST desconhecida\"}";
                send_http_response(client, 404, "Not Found", "application/json", res, (int)strlen(res));
            }
        }
    }

    closesocket(client);
    return 0;
}

int main() {
    // Configura titulo do console
    SetConsoleTitleA("CYBERSTORE // Servidor HTTP em C (Winsock2)");
    srand((unsigned int)time(NULL));
    InitializeCriticalSection(&g_storeMutex);

    printf("========================================================\n");
    printf("   CYBERSTORE E-COMMERCE - SERVIDOR WEB NATIVO EM C   \n");
    printf("   Eletronicos & Hardware Gamer para PC                \n");
    printf("========================================================\n\n");

    // Inicializa catálogo
    init_store_catalog();
    printf("[+] Catalogo carregado: %d produtos em memoria.\n", g_productsCount);

    // Inicializa Winsock
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("[-] Falha ao inicializar Winsock. Erro: %d\n", WSAGetLastError());
        return 1;
    }
    printf("[+] Winsock 2.2 inicializado com sucesso.\n");

    // Cria Socket TCP
    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) {
        printf("[-] Falha ao criar Socket. Erro: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // Configura endereco e porta
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Habilita reuso de porta
    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt));

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        printf("[-] Erro no bind na porta %d. Erro: %d\n", PORT, WSAGetLastError());
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    if (listen(server_socket, SOMAXCONN) == SOCKET_ERROR) {
        printf("[-] Erro no listen. Erro: %d\n", WSAGetLastError());
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    printf("[+] Servidor escutando na porta %d.\n", PORT);
    printf("[+] Acessivel em: http://localhost:%d\n\n", PORT);
    printf("--> Abrindo seu navegador automaticamente...\n");
    printf("--> Pressione Ctrl + C no terminal para encerrar o servidor.\n\n");

    // Abre o navegador automaticamente
    ShellExecuteA(NULL, "open", "http://localhost:8080", NULL, NULL, SW_SHOWNORMAL);

    // Loop de aceitacao de conexoes HTTP
    while (1) {
        struct sockaddr_in client_addr;
        int client_addr_len = sizeof(client_addr);
        SOCKET client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);

        if (client_socket != INVALID_SOCKET) {
            HANDLE thread = CreateThread(NULL, 0, client_thread, (LPVOID)client_socket, 0, NULL);
            if (thread) {
                CloseHandle(thread);
            } else {
                closesocket(client_socket);
            }
        }
    }

    // Cleanup
    closesocket(server_socket);
    WSACleanup();
    DeleteCriticalSection(&g_storeMutex);
    return 0;
}
