# CYBERSTORE // E-Commerce de Eletrônicos & Acessórios para PC em C

Um sistema completo de comércio eletrônico com **backend de alta performance escrito em linguagem C nativa** (utilizando Windows Sockets `winsock2.h`) e **frontend web moderno**, responsivo, dark-mode/cyberpunk, rodando diretamente no seu navegador em `http://localhost:8080`.

---

## 🚀 Como Iniciar em 1 Clique

1. **Opção mais rápida:** Dê um duplo-clique no arquivo:
   - `INICIAR_LOJA_ONLINE.bat` (localizado na pasta raiz `AULAS EM C`)
   - ou `loja_eletronicos\iniciar_loja.bat`

2. O terminal do Windows abrirá, inicializará os sockets em C e **abrirá automaticamente a loja no seu navegador padrão**.

---

## 🛠️ Arquitetura do Sistema

```
                    +-------------------------------------+
                    |     Navegador Web (Chrome / Edge)   |
                    |   UI Moderna Tailwind CSS + Icons   |
                    +------------------+------------------+
                                       |
                     Chamadas REST HTTP | (GET, POST, JSON)
                                       v
                    +-------------------------------------+
                    |       Servidor Web em C Nativo      |
                    |     (main.c // Winsock2 TCP 8080)   |
                    +------------------+------------------+
                                       |
                 +---------------------+---------------------+
                 |                     |                     |
                 v                     v                     v
          [ Structs C ]        [ Concorrência ]       [ Persistência ]
          - Products (12)      - Multi-threading      - pedidos_loja.txt
          - CartItems          - Mutex (Critical      - Resumo em disco
          - Orders History       Section)
```

---

## 🧩 Recursos e Funcionalidades

### 🖥️ Frontend (Navegador)
- **Design Moderno Cyber/Dark:** Interface futurista com Tailwind CSS, fontes Outfit & JetBrains Mono, e ícones Lucide.
- **Filtro por Categorias:** Placas de Vídeo, Processadores, Periféricos, Monitores, Memórias & SSD, Acessórios.
- **Busca em Tempo Real:** Pesquisa instantânea por nome, descrição ou categoria.
- **Ordenação Inteligente:** Menor Preço, Maior Preço, Mais Populares e Ordem Alfabética.
- **Modal de Detalhes:** Visualização em alta resolução e ficha técnica detalhada dos componentes.
- **Carrinho Interativo (Drawer):** Controle de quantidades `+` e `-`, remoção de itens, cálculo de subtotal e frete.
- **Cupons de Desconto:**
  - Digite `TECH10` para ganhar 10% de desconto.
  - Digite `CYBER20` para cupom VIP de 20% de desconto.
- **Checkout com Múltiplos Meios de Pagamento:**
  - **PIX:** Geração de QR Code simulado e código Pix Copia e Cola instantâneo.
  - **Cartão de Crédito:** Interface de cartão interativo, parcelamento e validação.
  - **Boleto Bancário:** Emissão simulada com código de barras.
- **Histórico de Pedidos:** Visualização dos pedidos realizados na sessão atual.
- **Telemetria do Servidor C:** Modal de status exibindo contagem de requisições, produtos em memória e faturamento.

### ⚙️ Backend em C (`main.c`)
- **Sockets Puros (`winsock2.h`):** Sem dependência de frameworks externos ou bibliotecas pesadas.
- **Multi-threading:** Cada requisição HTTP é processada em uma thread do Windows (`CreateThread`), garantindo resposta rápida e sem travamentos.
- **Segurança de Concorrência:** `CRITICAL_SECTION` para controle de concorrência ao atualizar estoque e pedidos.
- **Parser e Serializador JSON em C:** Rotinas dedicadas para ler payloads JSON e enviar respostas estruturadas.
- **Baixa Pegada de Memória:** Executável compilado super leve (< 150 KB) com consumo de RAM insignificante (< 10 MB).

---

## 📡 Endpoints da API REST em C

| Método | Rota | Descrição |
|---|---|---|
| `GET` | `/` | Serve a aplicação web (`index.html`) |
| `GET` | `/api/products` | Retorna catálogo de produtos em JSON com estoque atual |
| `GET` | `/api/cart` | Retorna itens do carrinho, quantidades e subtotal |
| `POST` | `/api/cart/add` | Adiciona um produto ao carrinho (valida estoque) |
| `POST` | `/api/cart/update` | Incrementa ou decrementa a quantidade no carrinho |
| `POST` | `/api/cart/remove` | Remove um produto do carrinho |
| `POST` | `/api/checkout` | Conclui a compra, subtrai estoque e registra pedido |
| `GET` | `/api/orders` | Retorna lista de pedidos confirmados na sessão |
| `GET` | `/api/stats` | Retorna métricas do servidor (requisições, receita, itens) |

---

## 🔧 Como Recompilar o Código em C

Caso queira fazer alterações no arquivo `main.c`:

Execute o arquivo:
```cmd
compilar_e_executar.bat
```
Ou compile manualmente com o compilador Microsoft Visual Studio (MSVC):
```cmd
cl /W3 /O2 /Fe:loja.exe main.c ws2_32.lib shell32.lib
```
