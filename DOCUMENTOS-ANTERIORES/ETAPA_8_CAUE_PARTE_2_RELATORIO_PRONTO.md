# UNIVERSIDADE PAULISTA (UNIP)
## CST em Análise e Desenvolvimento de Sistemas — PIM II

**Relatório Técnico — Etapa 8: Consolidação, Integração Multidisciplinar e Fechamento do Relatório**  
**Organização Foco:** CyberHardware Comércio e Soluções em Tecnologia Ltda. (*CyberStore*)  
**Disciplina:** Projeto Integrado Multidisciplinar II (PIM II)  
**Aluno Responsável (Etapa 8):** Cauê Carmacio  
**Campus:** Mirassol / SP — 2026  

---

## 1. Introdução, Contextualização e Integração Sistêmica Multidisciplinar

A **Etapa 8** consolida o ciclo de engenharia do **Projeto Integrado Multidisciplinar II (PIM II)** aplicado à **CyberHardware Comércio e Soluções em Tecnologia Ltda.** (CNPJ fictício `45.892.104/0001-78`, sediada na Av. Paulista, nº 1842, CEP `01310-200`, São Paulo/SP, caracterizada na Etapa 1). O objetivo desta etapa final é demonstrar como a especificação de Engenharia de Software definida na **Etapa 3** (`RF01`–`RF06`, `RNF01`–`RNF06`, `UC01`–`UC08`, **5 Sprints** com Product Backlog `US01`–`US08`, Arquitetura em 3 Camadas / MVC, Diagrama de Classes e Diagrama de Sequência com Gateway de Pagamento) articula organicamente a modelagem de banco de dados (**Etapa 4**), o motor transacional em C e frontend SPA (**Etapa 5**), a topologia de rede e nuvem (**Etapa 6**) e a conformidade legal em direitos humanos, acessibilidade e LGPD (**Etapa 7**).

### 1.1. Fluxo Completo de uma Requisição no Sistema (Arquitetura 3-Tier / MVC)

1. **Camada de Apresentação (*View / Frontend* — Etapas 3, 5 e 7):** O cliente ou administrador interage com a Single Page Application (`web/index.html`) construída segundo os padrões **WCAG 2.1/2.2** e **ABNT NBR 17225:2025** (`RNF03`), contando com contraste adequado, navegação via teclado, rótulos semânticos e textos alternativos `alt` nos ativos vetoriais (`/img/*.svg`).
2. **Infraestrutura de Rede, Segurança Perimetral e Criptografia em Trânsito (Etapa 6):** A requisição trafega sob criptografia **HTTPS/TLS na porta 443** (`RNF02`), atravessa o Firewall de borda (*Default Deny*) e o Reverse Proxy/Load Balancer na **VLAN 10 (DMZ)**, alcançando a **VLAN 20 (Aplicação/Administração)** em arquitetura de nuvem AWS/Azure projetada para **99,5% de disponibilidade** (`RNF05`).
3. **Camada de Lógica de Negócio (*Controller / Backend em C* — Etapas 2, 3 e 5):** O servidor HTTP nativo em C (`sistema_loja.c`, compilado com MinGW GCC `-Wall -Wextra -O2 -static` sobre `winsock2.h` na porta `8080`) processa as rotas REST estruturadas em **6 `structs` principais** (`Usuario`, `Produto`, `ItemCarrinho`, `Pagamento`, `Pedido` e `SessaoAtiva`). O motor executa:
   - Criptografia irreversível de senhas via **SHA-256 com *salt* aleatório** (`<salt_16hex>$<sha256_64hex>` em `sha256_hex()`, `gerar_hash_senha()` e `verificar_hash_senha()`), sem expor `senha_hash` nas respostas JSON (`RF01`, `RNF01`);
   - Estimativa dinâmica de frete por CEP (`calcular_frete_pedido()` em `POST /api/frete` — `RF04`);
   - Comunicação com o **Gateway de Pagamento** (`processar_gateway_pagamento()` para `PIX`, `CARTAO` e `BOLETO` — `RF05`, `UC05`), abortando a compra (`HTTP 400`) e preservando 100% do estoque caso o pagamento seja recusado;
   - **Baixa atômica de estoque** (`executar_checkout_atomico()` — `RF03`, `UC07`), geração de código de rastreamento (`CHW...BR`) e emissão de alerta quando o saldo atinge o `estoque_minimo` (Ponto de Pedido formulado na Etapa 2).
4. **Camada de Persistência de Dados (*Model / Database* — Etapas 4 e 7):** Os registros seguem o modelo relacional normalizado até a **3ª Forma Normal (3FN)** em MySQL 8.0 (**10 tabelas**: `usuario`, `cliente`, `administrador`, `categoria`, `produto`, `carrinho`, `item_carrinho`, `pedido`, `item_pedido` e `pagamento`) isolado na **VLAN 30 (Dados)**, com transações **ACID**, cache complementar NoSQL (**Redis**) para sessões/carrinhos e **backups diários automatizados** (`mysqldump --single-transaction` + snapshots AES-256 — `RNF06`).

### 1.2. Matriz de Rastreabilidade e Coerência Multidisciplinar

| Requisito / Caso de Uso (Etapa 3) | Banco de Dados 3FN — 10 Tabelas (Etapa 4) | Motor em C — 6 `structs` e APIs (Etapa 5) | Redes, LGPD e Latência Verificada (Etapas 6 e 7) |
| :--- | :--- | :--- | :--- |
| **RF01 / RNF01 / UC01 / UC02** (Cadastro, login, perfil, inativação e hash irreversível) | `usuario`, `cliente`, `administrador` (`senha_hash` VARCHAR(96), `papel`, `ativo`) | `struct Usuario`, `struct SessaoAtiva`, `sha256_hex()`, `/api/auth/*` | Minimização LGPD, bloqueio `403` pós-inativação, sem vazamento de `senha_hash` |
| **RF02 / RNF03 / UC03 / UC06** (Catálogo, pesquisa, specs, fotos e gestão Admin) | `categoria`, `produto` (`preco`, `especificacoes`, `foto`, `badge`, `ativo`) | `struct Produto`, `GET/POST/PUT /api/produtos`, `/img/*.svg` | Acessibilidade WCAG (`alt` em SVGs), guarda de perfil Admin (`403` para clientes) |
| **RF03 / UC07** (Baixa automática de estoque e alerta de limite mínimo) | `produto` (`quantidade_estoque CHECK >= 0`, `estoque_minimo`) | `executar_checkout_atomico()`, `terminal_alertas_estoque()` | Prevenção de ruptura invisível (Etapa 2) e sincronização atômica |
| **RF04 / UC04** (Carrinho de compras e estimativa de frete) | `carrinho`, `item_carrinho` (`cep_destino`, `valor_frete`, `valor_total`) | `struct ItemCarrinho`, `calcular_frete_pedido()`, `POST /api/frete` | Cache de sessão/carrinho (Redis na VLAN 20) e cálculo por faixa de CEP |
| **RF05 / UC05** (Checkout e autorização junto ao Gateway de Pagamento) | `pedido`, `item_pedido`, `pagamento` (`forma_pagamento`, `status_transacao`, `codigo_autorizacao`) | `struct Pagamento`, `struct Pedido`, `processar_gateway_pagamento()`, `POST /api/checkout` | HTTPS/TLS (porta 443), reversão imediata se Gateway recusar (`HTTP 400`) |
| **RF06 / RNF04 / RNF05 / RNF06 / UC08** (Histórico, rastreio, SLA e backup) | `pedido` (`status_pedido`, `codigo_rastreamento`) + `mysqldump` diário | `GET /api/pedidos`, `PUT /api/pedidos/status` (`CHW...BR`) | Latência medida de **0,42 a 58,81 ms** (< 2 s do `RNF04`), SLA 99,5% AWS |

---

## 2. Planejamento Ágil Scrum: 5 Sprints e Product Backlog (US01–US08)

Em conformidade com a **Etapa 3**, o desenvolvimento foi organizado pelo framework **Scrum** com um Product Backlog de **8 Histórias de Usuário (`US01` a `US08`)** distribuídas em **5 Sprints de 2 semanas cada**:

| Sprint (2 semanas) | Histórias / Requisitos / UCs | Funcionalidades Entregues | Objetivo e Validação Técnica |
| :--- | :--- | :--- | :--- |
| **Sprint 1** | `US01`, `US02`, `US03` (`RF01`, `RF02`, `RNF01` / `UC01`, `UC02`, `UC03`) | Cadastro, login com hash SHA-256 + salt e consulta inicial de produtos | Desenvolver a estrutura base da loja, autenticação segura e listagem de catálogo |
| **Sprint 2** | `US03`, `US04` (`RF02`, `RF04`, `RNF03` / `UC03`, `UC04`) | Pesquisa/filtros por categoria, carrinho dinâmico e cálculo de frete (`POST /api/frete`) | Permitir que o cliente pesquise hardwares, monte o carrinho e estime o frete por CEP |
| **Sprint 3** | `US05`, `US06` (`RF03`, `RF05` / `UC05`) | Finalização da compra, integração com Gateway de Pagamento e baixa atômica | Implementar o fluxo transacional de checkout (`PIX`, `CARTAO`, `BOLETO`) e reserva de saldo |
| **Sprint 4** | `US06`, `US07` (`RF03`, `RF06` / `UC06`, `UC07`, `UC08`) | Painel administrativo de produtos/estoque, alerta mínimo e rastreamento de pedidos | Desenvolver governança administrativa e acompanhamento logístico (`CHW...BR`) |
| **Sprint 5** | `US08` (`RNF02`, `RNF04`, `RNF05`, `RNF06`) | Bateria de testes de integração, topologia de 3 VLANs em nuvem e rotinas de backup | Garantir estabilidade, segurança HTTPS/TLS e latência medida entre **0,42 e 58,81 ms** |

---

## 3. Resolução dos Gargalos (Cenário Antes vs. Depois) e Ganhos para o Negócio

| Dimensão Avaliada | Cenário Anterior (Diagnóstico da Etapa 1) | Cenário Implantado (Pós-Alinhamento Etapas 3 a 7) |
| :--- | :--- | :--- |
| **Controle de Estoque** | Planilhas manuais desconectadas; alta incidência de *ruptura invisível* e cancelamentos pós-pagamento. | **Baixa atômica em C** vinculada à aprovação do Gateway (`UC05`/`UC07`), bloqueio de venda sem saldo e alerta automático de `estoque_minimo`. |
| **Desempenho e Latência** | Interpretadores pesados com travamentos em horários de pico e respostas acima de **6 segundos**. | **Servidor HTTP nativo em C (`-O2 -static`)** com latências reais medidas entre **0,42 ms e 58,81 ms** nos 16 testes de API (meta `RNF04` <= 2.000 ms). |
| **Modelagem de Dados** | Armazenamento sem normalização, sujeito a redundâncias e perda de histórico de preços. | **10 tabelas em 3FN (MySQL 8.0)** espelhadas em **6 `structs` C**, preço congelado em `item_pedido`, cache NoSQL (Redis) e backup diário (`RNF06`). |
| **Frete, Pagamento e Rastreio** | Processo manual sem estimativa de frete integrada nem rastreabilidade de entrega. | Cálculo de frete por CEP (`RF04`), autorização via **Gateway de Pagamento** (`UC05`) e histórico com código de rastreamento `CHW...BR` (`RF06`/`UC08`). |
| **Segurança e LGPD** | Senhas e dados expostos sem criptografia forte ou isolamento de rede. | Hash irreversível **SHA-256 + salt** (`RNF01`), inativação lógica (`RF01`), HTTPS/TLS (`RNF02`) e segmentação em **3 VLANs** com Firewall *Default Deny*. |
| **Acessibilidade Digital** | Interface sem suporte a teclado, baixo contraste e ausência de atributos semânticos (violação à LBI). | Conformidade com **WCAG 2.1/2.2 e ABNT NBR 17225:2025** (`RNF03`), textos `alt` nos SVGs, foco visível e navegação integral por teclado. |

### 3.1. Mensuração dos Ganhos sob Quatro Dimensões
* **Ganhos Operacionais:** sincronização imediata entre vitrine web, motor em C e camada de persistência, com disparo automático de alerta de reposição quando o estoque atinge o limite mínimo (`estoque_minimo`).
* **Ganhos Financeiros:** eliminação de multas, estornos e perdas de vendas por lentidão; quando o Gateway recusa uma transação (`HTTP 400`), nenhuma unidade física é descontada indevidamente do inventário.
* **Ganhos Técnicos:** execução nativa enxuta com latências aferidas de **0,42 ms** (`GET /api/status`) a **58,81 ms** (operações completas de autenticação/checkout), atendendo com ampla margem ao requisito `RNF04` (< 2 segundos) e operando sob arquitetura em 3 camadas isolada em 3 VLANs.
* **Ganhos Legais e Sociais:** aderência à LGPD (Lei nº 13.709/2018), ao Marco Civil da Internet, ao CDC, à Lei Brasileira de Inclusão (Lei nº 13.146/2015) e à Política Nacional de Resíduos Sólidos (Lei nº 12.305/2010 — logística reversa de e-lixo).

---

## 4. Quadro de Integrantes e Atribuições no Projeto (8 Etapas)

| Etapa | Integrante Responsável | Disciplina / Área do Conhecimento | Atribuição e Contribuição Específica no Trabalho |
| :--- | :--- | :--- | :--- |
| **Etapa 1** | Todos os Integrantes | Caracterização Organizacional | Diagnóstico institucional da CyberHardware Ltda. (CNPJ `45.892.104/0001-78`, CEP `01310-200`), portfólio e mapeamento dos 4 gargalos (ruptura invisível, latência > 6 s, LGPD e acessibilidade). |
| **Etapa 2** | Morgan | Pesquisa Científica e Inovação | Fundamentação teórica e estatística (ABIACOM, SciELO, Sofist/CNN) sobre ruptura de estoque e latência; formulação matemática do Ponto de Pedido (Estoque de Segurança e Estoque Mínimo). |
| **Etapa 3** | Kenai | Engenharia de Software Ágil | Levantamento de `RF01`–`RF06` e `RNF01`–`RNF06`; Product Backlog (`US01`–`US08`) em **5 Sprints** de 2 semanas; **8 Casos de Uso UML (`UC01`–`UC08`)**; Arquitetura 3-Tier/MVC, Diagrama de Classes e Diagrama de Sequência com Gateway de Pagamento. |
| **Etapa 4** | Saullo | Modelagem de Banco de Dados | Modelagem conceitual (DER), lógica e física (DDL/DML MySQL 8.0) em **3FN com 10 tabelas** (`usuario`, `cliente`, `administrador`, `categoria`, `produto`, `carrinho`, `item_carrinho`, `pedido`, `item_pedido`, `pagamento`), ACID, camada NoSQL (Redis) e backup diário (`RNF06`). |
| **Etapa 5** | Vittor Salvador | Programação Estruturada em C | Implementação do backend HTTP/REST em C (`sistema_loja.c`, `winsock2.h`, porta 8080) com **6 `structs`**, hash **SHA-256 + salt**, cálculo de frete, Gateway de Pagamento, baixa atômica, rastreamento, SPA (`web/index.html`) e bateria de 16 testes (**0,42–58,81 ms**). |
| **Etapa 6** | Vitor Godoy | Redes e Sistemas Distribuídos | Projeto de infraestrutura TCP/IP segmentada em **3 VLANs** (VLAN 10 DMZ, VLAN 20 Aplicação/Admin, VLAN 30 Banco de Dados), matriz de Firewall Stateful, HTTPS/TLS (porta 443) e alta disponibilidade de 99,5% em nuvem AWS/Azure. |
| **Etapa 7** | Gabriel | Direitos Humanos, LGPD e Segurança | Auditoria de acessibilidade digital (**ABNT NBR 17225:2025 / WCAG 2.1 e 2.2**), governança de privacidade segundo a **LGPD (Lei nº 13.709/2018)**, direito do consumidor (CDC) e logística reversa de lixo eletrônico (**Lei nº 12.305/2010**). |
| **Etapa 8** | Cauê Carmacio | Integração Multidisciplinar e Conclusão | Consolidação sistêmica das Etapas 1 a 7, matriz de rastreabilidade (`RF`/`RNF`/`UC`/Sprints), análise comparativa Antes vs. Depois, aferição de ganhos, limitações do MVP, trabalhos futuros e conclusão geral do PIM II. |

---

## 5. Limitações do Protótipo, Trabalhos Futuros e Considerações Finais

### 5.1. Limitações do Protótipo Atual
* **Persistência Local em Arquivos Estruturados:** No executável stand-alone em C (`sistema_loja.exe`), as 10 tabelas relacionais são simuladas localmente nos arquivos `usuarios_loja.txt`, `produtos_estoque.txt`, `pedidos_loja.txt` e `recibos_pedidos.txt`, garantindo portabilidade imediata para avaliação acadêmica sem exigir instância MySQL preinstalada na máquina avaliadora.
* **Simulação Local de Serviços Externos:** A autorização do Gateway de Pagamento (`processar_gateway_pagamento()`) e a tarifação logística por CEP (`calcular_frete_pedido()`) operam de forma determinística no próprio binário em C.

### 5.2. Trabalhos Futuros (Próximos Ciclos Evolutivos)
* **Integração Nativa C com MySQL (`libmysqlclient`) e Redis (`hiredis`):** Substituir a camada de arquivos locais por chamadas diretas via conector C ao cluster MySQL 8.0 na VLAN 30 e ao cache Redis na VLAN 20.
* **Webhooks Bancários Reais e Faturamento Fiscal (NF-e):** Integrar a rotina de checkout às APIs oficiais do Banco Central (PIX) e operadoras de cartão, automatizando a emissão de Nota Fiscal Eletrônica junto à SEFAZ.
* **Aplicativo Mobile Nativo:** Expandir a experiência responsiva para Android e iOS seguindo as diretrizes de acessibilidade móvel da **ABNT NBR 17060:2022**.

### 5.3. Considerações Finais
Retomando o problema de pesquisa estabelecido no início do projeto — *"Como desenvolver uma solução computacional integrada de e-commerce que concilie alto desempenho transacional no controle de estoque com a conformidade integral aos preceitos da LGPD e diretrizes de acessibilidade web?"* —, conclui-se que todos os objetivos gerais e específicos foram plenamente alcançados. A integração entre Engenharia de Software Ágil (5 Sprints, `RF01`–`RF06`, `RNF01`–`RNF06`, `UC01`–`UC08`), Banco de Dados Relacional em 3FN (10 tabelas), Programação Estruturada em C (6 `structs`, SHA-256, cálculo de frete, Gateway de Pagamento e latências de `0,42–58,81 ms`), Redes Seguras (3 VLANs, HTTPS/TLS e nuvem) e Direitos Humanos (WCAG e LGPD) resultou em uma plataforma coesa, auditável e preparada para o crescimento sustentável da **CyberHardware Ltda.**

---

## Referências

* ASSOCIAÇÃO BRASILEIRA DE NORMAS TÉCNICAS. **NBR 14724**: Informação e documentação — Trabalhos acadêmicos — Apresentação. Rio de Janeiro, 2011.
* ASSOCIAÇÃO BRASILEIRA DE NORMAS TÉCNICAS. **NBR 17225**: Acessibilidade em conteúdo e aplicações web — Requisitos. Rio de Janeiro, 2025.
* BRASIL. **Lei nº 13.146, de 6 de julho de 2015**. Institui a Lei Brasileira de Inclusão da Pessoa com Deficiência. Brasília, DF, 2015.
* BRASIL. **Lei nº 13.709, de 14 de agosto de 2018**. Lei Geral de Proteção de Dados Pessoais (LGPD). Brasília, DF, 2018.
* ELMASRI, Ramez; NAVATHE, Shamkant B. **Sistemas de banco de dados**. 7. ed. São Paulo: Pearson, 2019.
* PRESSMAN, Roger S.; MAXIM, Bruce R. **Engenharia de software**: uma abordagem profissional. 9. ed. Porto Alegre: AMGH, 2021.
* TANENBAUM, Andrew S.; WETHERALL, David J. **Redes de computadores**. 6. ed. São Paulo: Pearson, 2021.
