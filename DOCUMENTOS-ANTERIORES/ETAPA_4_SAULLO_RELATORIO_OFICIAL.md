# UNIVERSIDADE PAULISTA (UNIP)
## CST em Análise e Desenvolvimento de Sistemas — PIM II

**Relatório Técnico — Etapa 4: Modelagem e Banco de Dados Relacional**  
**Projeto:** Sistema de E-Commerce CyberHardware  
**Disciplina:** Projeto Integrado Multidisciplinar II (PIM II)  
**Aluno:** Saullo  
**Campus:** Mirassol / SP — 2026  

---

## 1. Introdução e Alinhamento com a Etapa 3

Este relatório apresenta a modelagem conceitual, lógica (normalizada até a 3FN) e física (MySQL 8.0) do banco de dados do sistema **CyberHardware E-Commerce**, tomando como referência integral a especificação definida na Etapa 3 (`PIM.pdf`). Toda estrutura foi desenhada para suportar os Requisitos Funcionais (`RF01` a `RF06`), os Requisitos Não Funcionais (`RNF01` a `RNF06`), os Casos de Uso (`UC01` a `UC08`) e as entidades do Diagrama de Classes (`Usuario`, `Cliente`, `Administrador`, `Produto`, `Carrinho`, `ItemCarrinho`, `Pagamento` e `Pedido`).

| Requisito / Caso de Uso (Etapa 3) | Cobertura na Modelagem de Banco de Dados (Etapa 4) | Correspondência no Protótipo C (Etapa 5) |
| :--- | :--- | :--- |
| **RF01 / UC01 / UC02 / RNF01** (Usuários, perfis, inativação e hash irreversível) | Superclasse `USUARIO` (`senha_hash` SHA-256 + salt, `papel`, `ativo`) especializada em `CLIENTE` (`cpf`, `endereco_entrega`) e `ADMINISTRADOR` (`nivel_acesso`). | `struct Usuario` + `usuarios_loja.txt` + `sha256_hex()` |
| **RF02 / UC03 / UC06** (Catálogo, specs técnicas, foto e inativação) | Tabelas `CATEGORIA` e `PRODUTO` (`preco`, `preco_antigo`, `especificacoes`, `foto`, `badge`, `ativo`). | `struct Produto` + `produtos_estoque.txt` |
| **RF03 / UC07** (Controle de estoque, baixa automática e alerta mínimo) | Colunas `quantidade_estoque` e `estoque_minimo` com restrição `CHECK (>= 0)` e consulta/visão de alerta mínimo. | `terminal_alertas_estoque()` + `executar_checkout_atomico()` |
| **RF04 / UC04** (Carrinho e estimativa de frete) | Tabelas `CARRINHO` (`cep_destino`, `valor_frete`, `valor_total`) e `ITEM_CARRINHO`. | `calcular_frete_pedido()` + `POST /api/frete` |
| **RF05 / UC05** (Checkout e Gateway de Pagamento) | Tabelas `PEDIDO`, `ITEM_PEDIDO` e `PAGAMENTO` (`forma_pagamento`, `status_transacao`, `codigo_autorizacao`) em transação ACID. | `struct Pagamento`, `struct Pedido` + `processar_gateway_pagamento()` |
| **RF06 / UC08 / RNF06** (Histórico, rastreamento e backups diários) | Colunas `status_pedido`, `codigo_rastreamento` em `PEDIDO`, FKs restritas e rotina diária `mysqldump`. | `pedidos_loja.txt` + `GET /api/pedidos` + `PUT /api/pedidos/status` |

---

## 2. Modelagem Conceitual (Entidades e Atributos)

1. **`USUARIO` (Superclasse — `RF01`, `RNF01`):** centraliza identidade e credenciais de acesso (`id_usuario`, `nome`, `email`, `senha_hash`, `telefone`, `papel`, `ativo`). A coluna `senha_hash` armazena exclusivamente resumos criptográficos irreversíveis com *salt* aleatório (`<salt_16hex>$<sha256_64hex>`), em conformidade com o `RNF01` e a LGPD (Lei nº 13.709/2018).
2. **`CLIENTE` (Especialização `1:1` de `USUARIO` — `UC01`, `UC05`, `UC08`):** estende `USUARIO` com `cpf` (único, 11 dígitos), `endereco_entrega` e `cep`. A exclusão lógica (inativação de conta prevista no `RF01`) ocorre via atributo `ativo = FALSE` em `USUARIO`, preservando o histórico fiscal dos pedidos realizados.
3. **`ADMINISTRADOR` (Especialização `1:1` de `USUARIO` — `UC06`, `UC07`):** estende `USUARIO` com `nivel_acesso` (`GERENTE_TOTAL`), autorizando cadastro/edição de produtos e atualização de status logístico.
4. **`CATEGORIA` (`RF02`):** classifica o portfólio (`Placas de Video`, `Processadores`, `Memorias RAM`, `Armazenamento`, `Placas-Mae`, `Fontes`, `Perifericos`).
5. **`PRODUTO` (`RF02`, `RF03`, `UC06`):** armazena `nome`, `preco`, `preco_antigo`, `quantidade_estoque`, `estoque_minimo`, `especificacoes`, `foto`, `badge`, `ativo` e `id_categoria` (FK).
6. **`CARRINHO` e `ITEM_CARRINHO` (`RF04`, `UC04`):** mantém os itens selecionados pelo cliente antes do checkout, armazenando `cep_destino`, `valor_frete` estimado e `valor_total`.
7. **`PEDIDO` e `ITEM_PEDIDO` (`RF05`, `RF06`, `UC05`, `UC08`):** formaliza a compra concluída com `codigo_pedido`, `data_pedido`, `status_pedido` (`Pagamento Aprovado`, `Em Separacao`, `Enviado`, `Entregue`), `codigo_rastreamento`, `valor_bruto`, `desconto`, `valor_frete` e `valor_total`, resolvendo o relacionamento `N:N` com `PRODUTO` por meio da entidade associativa `ITEM_PEDIDO`.
8. **`PAGAMENTO` (`Diagrama de Classes` e `Diagrama de Sequência` — `UC05`):** registra a transação autorizada pelo Gateway de Pagamento externo (`forma_pagamento` em `PIX`, `CARTAO` ou `BOLETO`, `status_transacao`, `codigo_autorizacao` e `data_processamento`).

---

## 3. Diagrama Entidade-Relacionamento (DER)

![Diagrama Entidade-Relacionamento CyberHardware](diagramas/der_cyberhardware.png)

---

## 4. Modelo Lógico e Normalização (1FN, 2FN e 3FN)

* **Primeira Forma Normal (1FN):** todos os atributos são atômicos e monovalorados. Itens de pedido/carrinho foram extraídos para tabelas próprias (`ITEM_PEDIDO` e `ITEM_CARRINHO`), evitando colunas multivaloradas ou grupos repetitivos.
* **Segunda Forma Normal (2FN):** todas as tabelas utilizam chave primária simples (`id_*`), e atributos como `quantidade`, `preco_unitario` e `subtotal` em `ITEM_PEDIDO` dependem integralmente da relação entre `PEDIDO` e `PRODUTO`.
* **Terceira Forma Normal (3FN):** não há dependências transitivas entre atributos não-chave. Dados de `USUARIO`/`CLIENTE`, `CATEGORIA` e `PAGAMENTO` residem em tabelas segregadas, e o `preco_unitario` é copiado em `ITEM_PEDIDO` no instante da compra para preservar o histórico financeiro mesmo após reajustes no catálogo.

---

## 5. Modelo Físico (DDL MySQL 8.0)

```sql
CREATE DATABASE IF NOT EXISTS cyberhardware_db
  DEFAULT CHARACTER SET utf8mb4
  DEFAULT COLLATE utf8mb4_unicode_ci;
USE cyberhardware_db;

CREATE TABLE usuario (
    id_usuario INT AUTO_INCREMENT PRIMARY KEY,
    nome VARCHAR(100) NOT NULL,
    email VARCHAR(100) NOT NULL UNIQUE,
    senha_hash VARCHAR(96) NOT NULL,
    telefone VARCHAR(30) NOT NULL,
    papel ENUM('CLIENTE', 'ADMIN') NOT NULL DEFAULT 'CLIENTE',
    ativo BOOLEAN NOT NULL DEFAULT TRUE,
    criado_em TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP
) ENGINE=InnoDB;

CREATE TABLE cliente (
    id_usuario INT PRIMARY KEY,
    cpf CHAR(11) NOT NULL UNIQUE,
    endereco_entrega VARCHAR(180) NOT NULL,
    cep CHAR(8) NOT NULL DEFAULT '01310100',
    CONSTRAINT fk_cliente_usuario FOREIGN KEY (id_usuario)
        REFERENCES usuario(id_usuario) ON DELETE RESTRICT ON UPDATE CASCADE
) ENGINE=InnoDB;

CREATE TABLE administrador (
    id_usuario INT PRIMARY KEY,
    nivel_acesso VARCHAR(32) NOT NULL DEFAULT 'GERENTE_TOTAL',
    CONSTRAINT fk_admin_usuario FOREIGN KEY (id_usuario)
        REFERENCES usuario(id_usuario) ON DELETE RESTRICT ON UPDATE CASCADE
) ENGINE=InnoDB;

CREATE TABLE categoria (
    id_categoria INT AUTO_INCREMENT PRIMARY KEY,
    nome_categoria VARCHAR(60) NOT NULL UNIQUE,
    descricao VARCHAR(150) NULL
) ENGINE=InnoDB;

CREATE TABLE produto (
    id_produto INT AUTO_INCREMENT PRIMARY KEY,
    id_categoria INT NOT NULL,
    nome VARCHAR(120) NOT NULL,
    preco DECIMAL(10,2) NOT NULL CHECK (preco > 0),
    preco_antigo DECIMAL(10,2) NOT NULL DEFAULT 0.00,
    quantidade_estoque INT NOT NULL CHECK (quantidade_estoque >= 0),
    estoque_minimo INT NOT NULL DEFAULT 2 CHECK (estoque_minimo >= 0),
    especificacoes TEXT NOT NULL,
    foto VARCHAR(255) NOT NULL DEFAULT '/img/default.svg',
    badge VARCHAR(32) NULL,
    ativo BOOLEAN NOT NULL DEFAULT TRUE,
    CONSTRAINT fk_produto_categoria FOREIGN KEY (id_categoria)
        REFERENCES categoria(id_categoria) ON DELETE RESTRICT ON UPDATE CASCADE
) ENGINE=InnoDB;

CREATE TABLE carrinho (
    id_carrinho INT AUTO_INCREMENT PRIMARY KEY,
    id_cliente INT NOT NULL UNIQUE,
    cep_destino CHAR(8) NULL,
    valor_frete DECIMAL(10,2) NOT NULL DEFAULT 0.00,
    valor_total DECIMAL(10,2) NOT NULL DEFAULT 0.00,
    CONSTRAINT fk_carrinho_cliente FOREIGN KEY (id_cliente)
        REFERENCES cliente(id_usuario) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB;

CREATE TABLE item_carrinho (
    id_item_carrinho INT AUTO_INCREMENT PRIMARY KEY,
    id_carrinho INT NOT NULL,
    id_produto INT NOT NULL,
    quantidade INT NOT NULL CHECK (quantidade > 0),
    preco_unitario DECIMAL(10,2) NOT NULL,
    CONSTRAINT fk_ic_carrinho FOREIGN KEY (id_carrinho) REFERENCES carrinho(id_carrinho) ON DELETE CASCADE,
    CONSTRAINT fk_ic_produto FOREIGN KEY (id_produto) REFERENCES produto(id_produto) ON DELETE RESTRICT
) ENGINE=InnoDB;

CREATE TABLE pedido (
    id_pedido INT AUTO_INCREMENT PRIMARY KEY,
    id_cliente INT NOT NULL,
    codigo_pedido VARCHAR(28) NOT NULL UNIQUE,
    data_pedido DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
    status_pedido ENUM('Pagamento Aprovado','Em Separacao','Enviado','Entregue') NOT NULL DEFAULT 'Pagamento Aprovado',
    codigo_rastreamento VARCHAR(32) NOT NULL UNIQUE,
    endereco_entrega VARCHAR(180) NOT NULL,
    cep_entrega CHAR(8) NOT NULL,
    valor_bruto DECIMAL(10,2) NOT NULL,
    desconto DECIMAL(10,2) NOT NULL DEFAULT 0.00,
    valor_frete DECIMAL(10,2) NOT NULL DEFAULT 0.00,
    valor_total DECIMAL(10,2) NOT NULL,
    CONSTRAINT fk_pedido_cliente FOREIGN KEY (id_cliente)
        REFERENCES cliente(id_usuario) ON DELETE RESTRICT ON UPDATE CASCADE
) ENGINE=InnoDB;

CREATE TABLE item_pedido (
    id_item_pedido INT AUTO_INCREMENT PRIMARY KEY,
    id_pedido INT NOT NULL,
    id_produto INT NOT NULL,
    quantidade INT NOT NULL CHECK (quantidade > 0),
    preco_unitario DECIMAL(10,2) NOT NULL,
    subtotal DECIMAL(10,2) NOT NULL,
    CONSTRAINT fk_ip_pedido FOREIGN KEY (id_pedido) REFERENCES pedido(id_pedido) ON DELETE CASCADE,
    CONSTRAINT fk_ip_produto FOREIGN KEY (id_produto) REFERENCES produto(id_produto) ON DELETE RESTRICT
) ENGINE=InnoDB;

CREATE TABLE pagamento (
    id_pagamento INT AUTO_INCREMENT PRIMARY KEY,
    id_pedido INT NOT NULL UNIQUE,
    forma_pagamento ENUM('PIX', 'CARTAO', 'BOLETO') NOT NULL,
    status_transacao ENUM('APROVADO', 'PENDENTE', 'RECUSADO') NOT NULL,
    codigo_autorizacao VARCHAR(32) NOT NULL,
    data_processamento DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
    CONSTRAINT fk_pagamento_pedido FOREIGN KEY (id_pedido)
        REFERENCES pedido(id_pedido) ON DELETE RESTRICT ON UPDATE CASCADE
) ENGINE=InnoDB;
```

---

## 6. Scripts DML Unificados com o Backend C e Consultas Operacionais

```sql
INSERT INTO categoria (id_categoria, nome_categoria, descricao) VALUES
(1, 'Placas de Video', 'GPUs de alta performance com Ray Tracing'),
(2, 'Processadores', 'CPUs desktop multi-core'),
(3, 'Memorias RAM', 'Modulos DDR5 alta frequencia'),
(4, 'Armazenamento', 'SSDs NVMe PCIe Gen4'),
(5, 'Placas-Mae', 'Motherboards socket AM5 e VRM reforcado'),
(6, 'Fontes', 'Fontes ATX 3.0 certificacao 80 Plus Gold');

INSERT INTO produto (id_produto, id_categoria, nome, preco, preco_antigo, quantidade_estoque, estoque_minimo, ativo, badge, foto, especificacoes) VALUES
(1, 1, 'Placa de Video RTX 4080 Super 16GB GDDR6X', 7299.90, 8199.90, 4, 2, TRUE, 'Destaque', '/img/gpu.svg', '16GB GDDR6X, 256-bit, Ray Tracing 3a Gen, DLSS 3.5, HDMI 2.1a'),
(2, 2, 'Processador AMD Ryzen 7 9800X3D 5.2GHz', 3299.90, 3699.90, 6, 3, TRUE, 'Novo', '/img/cpu.svg', '8 Nucleos, 16 Threads, Cache 104MB 3D V-Cache, Socket AM5'),
(3, 3, 'Memoria RAM Kingston Fury Beast 32GB DDR5', 899.90, 1099.90, 8, 3, TRUE, 'Oferta', '/img/ram.svg', 'Kit 2x16GB, 6000MT/s, CL30, Dissipador Aluminio Low-Profile'),
(4, 4, 'SSD NVMe WD Black SN850X 2TB PCIe 4.0', 1149.90, 1349.90, 5, 2, TRUE, 'Top Vendas', '/img/ssd.svg', 'M.2 2280, Leitura 7300 MB/s, Gravacao 6600 MB/s, Cache DRAM'),
(5, 5, 'Placa-Mae ASUS ROG Strix X870E-E Gaming', 3499.90, 3899.90, 3, 2, TRUE, 'Premium', '/img/mobo.svg', 'Socket AM5, Wi-Fi 7, PCIe 5.0 x16, VRM 18+2+2 Fases, USB4'),
(6, 6, 'Fonte Corsair RM1000x Shift 1000W 80+ Gold', 1299.90, 1499.90, 5, 2, TRUE, 'Modular', '/img/psu.svg', 'Full Modular Lateral, ATX 3.0, Cabo PCIe 5.0 12VHPWR Nativo');

-- Alerta de Estoque Minimo (RF03 / UC07)
SELECT id_produto, nome, quantidade_estoque, estoque_minimo
FROM produto WHERE ativo = TRUE AND quantidade_estoque <= estoque_minimo;

-- Historico e Rastreamento do Cliente (RF06 / UC08)
SELECT p.codigo_pedido, p.data_pedido, p.status_pedido, p.codigo_rastreamento,
       pg.forma_pagamento, pg.status_transacao, p.valor_frete, p.valor_total
FROM pedido p JOIN pagamento pg ON pg.id_pedido = p.id_pedido
WHERE p.id_cliente = 2 ORDER BY p.data_pedido DESC;
```

---

## 7. Persistência em Arquivos, Backup Automatizado (`RNF06`) e Arquitetura Híbrida SQL + NoSQL

Enquanto a camada corporativa em produção na VLAN 30 utiliza o **SGBD Relacional (MySQL 8.0 / InnoDB)** com integridade referencial rigorosa e **backups diários automatizados** (`mysqldump --single-transaction` agendado às 03h00 UTC para Storage replicado com criptografia AES-256 e retenção de 30 dias, cumprindo o `RNF06`), o protótipo executável stand-alone em C (`sistema_loja.c`) reflete exatamente esse esquema lógico por meio dos arquivos estruturados `usuarios_loja.txt`, `produtos_estoque.txt` e `pedidos_loja.txt`.

Para sustentar o requisito não funcional de desempenho `RNF04` (respostas <= 2 segundos em pesquisas no catálogo e checkout), recomenda-se uso complementar de **NoSQL Chave-Valor (Redis)** na VLAN 20 exclusivamente para armazenamento efêmero de sessões (*Bearer Tokens* com TTL), cache de consulta de catálogo e bloqueio temporário de carrinhos, preservando a consistência **ACID** do SGBD relacional para transações financeiras, baixa atômica de estoque e registros de pagamento.

---

## 8. Referências

ASSOCIAÇÃO BRASILEIRA DE NORMAS TÉCNICAS. **NBR 14724**: Informação e documentação — Trabalhos acadêmicos — Apresentação. Rio de Janeiro, 2011.

BRASIL. **Lei nº 13.709, de 14 de agosto de 2018**. Lei Geral de Proteção de Dados Pessoais (LGPD). Brasília, DF: Presidência da República, 2018.

ELMASRI, Ramez; NAVATHE, Shamkant B. **Sistemas de banco de dados**. 7. ed. São Paulo: Pearson, 2019.

SILBERSCHATZ, Abraham; KORTH, Henry F.; SUDARSHAN, S. **Sistema de banco de dados**. 7. ed. Rio de Janeiro: LTC, 2020.
