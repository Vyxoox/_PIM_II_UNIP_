# ETAPA 3 – ENGENHARIA DE SOFTWARE ÁGIL
## PARTE 1: DIRETRIZES, ESPECIFICAÇÕES E GUIA DE DESENVOLVIMENTO
**Projeto Integrado Multidisciplinar II (PIM II)**  
**Curso:** Curso Superior de Tecnologia em Análise e Desenvolvimento de Sistemas (UNIP)  
**Polo:** Mirassol – SP | Ano: 2026  
**Empresa Fictícia:** CyberHardware Comércio e Soluções em Tecnologia Ltda. (*CyberStore*)  
**Responsável da Etapa:** Kenai  
**Integração:** Saullo (Banco de Dados), Vittor Salvador (Backend em C), Vitor Godoy (Redes) e Gabriel (LGPD e Inclusão)

---

## 1. Visão Geral da Etapa e Papel no PIM II

Fala, Kenai! Este documento traz todas as **diretrizes, decisões arquiteturais e requisitos oficiais** que alinhamos em equipe para a construção da plataforma de e-commerce da **CyberHardware**.

A sua etapa (**Engenharia de Software Ágil**) é o pilar estruturante de todo o projeto acadêmico: ela define **o que o sistema deve fazer** (Requisitos Funcionais), **como ele deve se comportar** (Requisitos Não Funcionais), **como o time se organizou para entregar valor** (Scrum, Kanban, MoSCoW, Sprints) e **como os módulos interagem** (Modelagem UML: Diagrama de Casos de Uso e Diagrama de Sequência).

Você tem total liberdade para redigir o texto com suas próprias palavras e estilo. Use este material como bússola para garantir que o seu capítulo converse em 100% de harmonia com o Banco de Dados do Saullo, o Servidor em C do Vittor Salvador, a Infraestrutura de Redes do Vitor Godoy e a Conformidade LGPD do Gabriel.

> 💡 **Nota:** Além deste guia (Parte 1), você também tem em mãos a **Parte 2 (Gabarito Pronto)**. Ela contém a documentação técnica integral e os diagramas já redigidos para consulta, destravamento ou utilização direta como base.

---

## 2. Metodologia de Desenvolvimento de Software

No seu texto acadêmico, fundamente o processo de desenvolvimento em um **modelo ágil híbrido** altamente consagrado na indústria moderna:

1. **Framework Ágil Base:** **Scrum** em conjunto com práticas de fluxo visual do **Kanban**.
2. **Ciclos de Entrega (Sprints):** Sprints curtas e iterativas de **1 semana** (Time-boxed), permitindo rápida validação e integração contínua entre as frentes de banco, código nativo e redes.
3. **Cerimônias Essenciais Adotadas:**
   * *Sprint Planning:* Planejamento do backlog da iteração e definição da meta da Sprint.
   * *Daily Scrum:* Alinhamento diário rápido de impedimentos e progresso entre os integrantes.
   * *Sprint Review & Retrospective:* Demonstração dos incrementos funcionais (software em C rodando e interface web) e identificação de melhorias de processo.
4. **Técnica de Priorização:** **MoSCoW** (*Must Have*, *Should Have*, *Could Have*, *Won't Have*), garantindo foco estrito no núcleo transacional antes de recursos cosméticos.
5. **Estimativa de Esforço:** **Planning Poker** com sequência simplificada de Fibonacci (1, 2, 3, 5, 8 story points) para dimensionar a complexidade das Histórias de Usuário.

---

## 3. Especificação dos Requisitos do Sistema

Para manter a coerência com as outras etapas já finalizadas, o seu documento deve listar e descrever formalmente os **6 Requisitos Funcionais (RF)** e os **6 Requisitos Não Funcionais (RNF)** detalhados a seguir:

### 3.1. Requisitos Funcionais (RF)
* **RF01 – Gestão e Cadastro de Clientes (Must Have):** O sistema deve capturar dados essenciais do comprador (Nome completo, CPF, e-mail, telefone e endereço de entrega) durante o checkout, viabilizando o faturamento e a entrega dos periféricos.
* **RF02 – Catálogo Dinâmico de Produtos por Categoria (Must Have):** O sistema deve exibir os produtos organizados por departamentos de hardware (*Placas de Vídeo*, *Processadores*, *Periféricos*, *Monitores*, *Armazenamento*), contendo título, preço original, preço à vista com desconto, ficha técnica e indicador de estoque em tempo real.
* **RF03 – Controle Atômico de Estoque (Must Have):** O sistema deve verificar a disponibilidade dos componentes e executar a baixa imediata e atômica das quantidades em estoque assim que o pagamento for confirmado, impedindo venda de itens indisponíveis (*overselling*).
* **RF04 – Gestão do Carrinho de Compras e Cupons (Must Have):** O sistema deve permitir inserção, remoção, alteração de quantidades de itens na sacola virtual e aplicação de cupons promocionais de desconto (*TECH10*, *CYBER20*).
* **RF05 – Checkout e Formas de Pagamento (Must Have):** O sistema deve concluir o pedido suportando múltiplas modalidades: PIX (código copia-e-cola / QR Code simulado), Cartão de Crédito (com parcelamento) e Boleto Bancário.
* **RF06 – Histórico e Telemetria de Pedidos (Should Have):** O sistema deve gravar de forma persistente cada venda aprovada, gerando código único identificador e permitindo consulta a vendas passadas e estatísticas da aplicação.

### 3.2. Requisitos Não Funcionais (RNF)
* **RNF01 – Segurança da Informação e Conformidade LGPD:** O sistema deve assegurar a privacidade dos dados pessoais coletados conforme a Lei nº 13.709/2018, aplicando mascaramento e tráfego seguro.
* **RNF02 – Alto Desempenho e Baixa Latência:** O backend deve responder às requisições da interface em tempo inferior a 100 milissegundos sob condições operacionais típicas, viabilizado pela compilação nativa em linguagem C e Winsock2.
* **RNF03 – Acessibilidade Digital (WCAG 2.1 AA / ABNT NBR 17225:2025):** A interface web deve dispor de navegação por teclado, rótulos ARIA, contraste cromático superior a 4.5:1 e tipografia legível para garantir usabilidade universal.
* **RNF04 – Concorrência e Integridade Transacional:** O servidor deve empregar mecanismos de exclusão mútua (`CRITICAL_SECTION` / Mutex) para evitar condições de corrida (*race conditions*) no decremento de estoque durante acessos simultâneos.
* **RNF05 – Arquitetura Cliente-Servidor e Portabilidade:** O sistema deve adotar arquitetura desacoplada via protocolo HTTP/1.1 e payloads estruturados em JSON, compatível com os navegadores web modernos (Edge, Chrome, Firefox).
* **RNF06 – Persistência Estruturada de Dados:** O sistema deve garantir a persistência dos dados em disco (estruturas formatadas em arquivo texto para o protótipo em C e modelo relacional normalizado na 3ª Forma Normal no SGBD).

---

## 4. Product Backlog Ágil e Planejamento de Sprints

Organize os requisitos no formato clássico de **User Stories** (*Como... Quero... Para...*), especificando a pontuação (Story Points) e os **Critérios de Aceite** (*Definition of Done - DoD*).

### 4.1. Estrutura Recomendada das Histórias de Usuário
1. **US01 (Catálogo - 5 pts - Must Have):** Navegação e busca dinâmica de produtos por categoria de hardware.
2. **US02 (Carrinho - 5 pts - Must Have):** Gestão interativa de sacola com slide-over e recálculo dinâmico.
3. **US03 (Estoque Concorrente - 8 pts - Must Have):** Decremento atômico de estoque com proteção via Mutex no backend em C.
4. **US04 (Checkout Multi-pagamento - 8 pts - Must Have):** Coleta de dados, escolha de PIX/Cartão/Boleto e emissão do comprovante.
5. **US05 (Cupons Promocionais - 3 pts - Should Have):** Aplicação de regras de desconto percentual (`TECH10` e `CYBER20`).
6. **US06 (Histórico e Persistência - 5 pts - Should Have):** Gravação do pedido em disco (`pedidos_loja.txt`) e consulta de histórico.
7. **US07 (Painel de Telemetria - 5 pts - Could Have):** Dashboard operacional com métricas de uptime, volume faturado e requisições.

*Carga total:* **39 Story Points**.

### 4.2. Sugestão de Divisão das Sprints
* **Sprint 1 (18 pts) – MVP de Catálogo e Estoque:** US01 + US02 + US03.
* **Sprint 2 (16 pts) – Transações e Checkout:** US04 + US05 + US06.
* **Sprint 3 (5 pts) – Telemetria, Acessibilidade e Estabilização:** US07 + testes de usabilidade e auditoria.

---

## 5. Modelagem Visual com UML (Unified Modeling Language)

A banca avaliadora da UNIP exige representação visual padronizada da arquitetura funcional. Na sua seção, apresente e descreva:

### 5.1. Atores do Sistema
* **Cliente:** Usuário externo que acessa a CyberStore, consulta catálogo, monta o carrinho, aplica cupons e efetua a compra.
* **Administrador:** Responsável pela supervisão técnica e operacional, acompanhando telemetria do servidor, volume de pedidos e integridade do estoque.

### 5.2. Especificação Textual dos 4 Casos de Uso Críticos
Para cada caso de uso, forneça a ficha descritiva com:
* **Identificador e Nome:** UC01 (Consultar Catálogo), UC02 (Gerenciar Carrinho), UC03 (Efetuar Checkout), UC04 (Monitorar Telemetria e Estoque).
* **Ator Primário, Pré-condições e Pós-condições.**
* **Fluxo Principal de Eventos (Passo a passo numerado).**
* **Fluxos Alternativos** (ex.: aplicação com sucesso de cupom de desconto).
* **Fluxos de Exceção** (ex.: tentativa de comprar produto com estoque zerado ou concorrência simultânea).

### 5.3. Diagramas UML Obrigatórios
1. **Diagrama de Casos de Uso (UML Use Case):**
   * Mostrar os atores *Cliente* e *Administrador*.
   * Representar as relações de associação.
   * Utilizar os estereótipos `<<include>>` (ex.: *Efetuar Checkout* inclui *Baixar Estoque* e *Gravar Pedido*) e `<<extend>>` (ex.: *Gerenciar Carrinho* pode ser estendido por *Aplicar Cupom*).
2. **Diagrama de Sequência (UML Sequence):**
   * Mostrar o ciclo de vida da requisição no fluxo de checkout: `Cliente (Navegador)` $\rightarrow$ `Interface Web SPA` $\rightarrow$ `Servidor HTTP C (Winsock2)` $\rightarrow$ `Controle de Mutex` $\rightarrow$ `Memória de Estoque` $\rightarrow$ `Arquivo em Disco` $\rightarrow$ `Resposta HTTP 200/400`.

---

## 6. Rastreabilidade Multidisciplinar (O Segredo da Nota Máxima)

A UNIP valoriza intensamente a integração real entre as matérias. Ao redigir, faça ganchos explícitos demonstrando como a sua etapa se conecta às demais:

1. **Com a Etapa 4 (Banco de Dados - Saullo):**  
   Os requisitos RF01, RF02 e RF05 fornecem os atributos e relacionamentos que o Saullo modelou nas tabelas `CLIENTE`, `PRODUTO`, `CATEGORIA`, `PEDIDO` e `ITEM_PEDIDO` (normalizadas até a 3ª Forma Normal).
2. **Com a Etapa 5 (Programação Estruturada em C - Vittor Salvador):**  
   Os requisitos RF03, RNF02 e RNF04 são o alicerce do código em C: as structs `Product` e `Order`, o socket TCP na porta 8080 e a seção crítica de concorrência (`EnterCriticalSection` / `LeaveCriticalSection`).
3. **Com a Etapa 6 (Redes de Computadores - Vitor Godoy):**  
   O requisito RNF05 dita o modelo arquitetural cliente-servidor, a pilha TCP/IP, o protocolo HTTP/1.1 e os requisitos de firewall na porta 8080.
4. **Com a Etapa 7 (LGPD e Inclusão - Gabriel):**  
   Os requisitos RNF01 e RNF03 garantem que a especificação de software já nasce com conformidade à LGPD (*Privacy by Design*) e com diretrizes universais de acessibilidade (WCAG 2.1 AA / NBR 17225:2025).

---

## 7. Roteiro Prático de Redação

Para montar a sua entrega com tranquilidade:
1. Comece com uma introdução contextualizando a importância da Engenharia de Software no e-commerce da CyberHardware.
2. Explique a adoção do Scrum/Kanban e justifique as escolhas (Sprints semanais, MoSCoW, Fibonacci).
3. Apresente as tabelas de Requisitos Funcionais e Requisitos Não Funcionais.
4. Insira o Backlog de User Stories e o planejamento das 3 Sprints.
5. Detalhe os 4 Casos de Uso textuais e adicione os diagramas UML (Casos de Uso e Sequência).
6. Finalize com a matriz de rastreabilidade e as referências acadêmicas (Pressman, Sommerville, Schwaber).

Se pintar qualquer dúvida durante a elaboração, consulte a **Parte 2 (Gabarito Pronto)** que já está com todos os diagramas e tabelas prontos para servir de referência!
