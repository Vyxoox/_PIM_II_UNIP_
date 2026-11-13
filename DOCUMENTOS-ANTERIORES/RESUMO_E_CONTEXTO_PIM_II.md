# RESUMO DE CONTEXTO E PROJETO PIM II (UNIP 2026/2)
> **Arquivo de Continuidade:** Utilize este documento para retomar o projeto no Antigravity do computador de casa.

---

## 📌 1. Informações Gerais do Projeto
* **Curso:** CST em Análise e Desenvolvimento de Sistemas (UNIP 2026/2).
* **Projeto:** Projeto Integrado Multidisciplinar II (PIM II).
* **Prazo de Envio:** `26/10/2026` a `14/11/2026`.
* **Apresentações:** `24/11/2026` a `27/11/2026`.
* **Organização Fictícia Criada:** *CyberHardware Comércio e Soluções em Tecnologia Ltda.* (Nome Fantasia: *CyberStore* / *CyberHardware E-commerce*).
* **Ramo:** Comércio eletrônico especializado em hardware de alta performance e periféricos gamer/pro.

---

## 👥 2. Divisão de Tarefas da Equipe

1. **Etapa 1 (Todos juntos):** Caracterização da Organização *(100% Concluída e fundamentada)*.
2. **Etapa 2 (Morgan):** Pesquisa Científica e Inovação *(Roteiro com 4 eixos, objetivos e referências pronto)*.
3. **Etapa 3 (Kenai):** Engenharia de Software Ágil *(Requisitos RF01-RF06, RNF01-RNF06, Product Backlog MoSCoW e Casos de Uso prontos)*.
4. **Etapa 4 (Saullo):** Modelagem de Banco de Dados *(5 tabelas normalizadas: Categoria, Produto, Cliente, Pedido, Item_Pedido)*.
5. **Etapa 5 (Vittor Salvador):** Programação Estruturada em C *(Sistema web em C nativo com Winsock2 compilado e funcional em `loja_eletronicos/`)*.
6. **Etapa 6 (Vitor Godoy):** Redes de Computadores e Nuvem *(Topologia de rede, arquitetura cliente-servidor HTTP e segurança)*.
7. **Etapa 7 (Gabriel):** Direitos Humanos, Inclusão e Segurança *(Conformidade LGPD e Acessibilidade WCAG 2.1)*.
8. **Etapa 8 (Cauê):** Integração da Solução, Benefícios e Conclusão Geral.

---

## 🏢 3. Etapa 1 – Caracterização da Organização (Texto Oficial para o Relatório)

* **Razão Social:** CyberHardware Comércio e Soluções em Tecnologia Ltda.
* **Nome Fantasia:** CyberHardware E-commerce (CyberStore)
* **CNPJ (fictício):** 45.892.104/0001-78
* **Inscrição Estadual:** 112.345.678.910
* **Sede:** Av. Paulista, 1842, 10º andar, Conjunto 102 – Bela Vista, São Paulo – SP, CEP: 01310-200.
* **Porte:** Empresa de Pequeno Porte (EPP).
* **Missão:** Fornecer componentes de hardware e periféricos de ponta com excelência no atendimento, garantindo segurança transacional, agilidade logística e preços competitivos para gamers, estudantes e profissionais de tecnologia.
* **Visão:** Consolidar-se como uma das principais referências nacionais em e-commerce de eletrônicos e soluções de hardware de alta performance até 2028.
* **Valores:** Transparência e Ética, Segurança e Privacidade (LGPD), Inovação Contínua e Inclusão Digital.
* **Produtos:** Processadores, Placas de Vídeo, SSDs, RAMs, Teclados Mecânicos, Mouses, Headsets, Monitores e Water Coolers.
* **Gargalos Operacionais Identificados:** Controle manual de estoque que gerava ruptura/vendas duplicadas, lentidão no fechamento de pedidos, vulnerabilidade no armazenamento de dados de clientes e falta de acessibilidade para pessoas com deficiência.
* **Justificativa da Solução:** Criação de um sistema com backend nativo em C de alta velocidade e baixo consumo de memória, controle atômico de estoque, banco de dados normalizado, criptografia para adequação à LGPD e interface web acessível.

---

## 🔍 4. Etapa 2 – Guia para a Morgan (Pesquisa Científica)

* **Pergunta de Pesquisa:** *"Como desenvolver uma solução computacional integrada de e-commerce que concilie alto desempenho transacional no controle de estoque com a conformidade integral aos preceitos da LGPD e diretrizes de acessibilidade web?"*
* **Objetivo Geral:** Projetar e implementar uma solução computacional para automação e gestão de vendas de uma loja virtual de periféricos e hardware, assegurando processamento eficiente, escalabilidade e segurança de dados.
* **Metodologia:** Natureza Aplicada; Objetivos Exploratórios e Descritivos; Procedimentos Bibliográficos e Estudo de Caso/Experimental com desenvolvimento de software.
* **Eixos de Pesquisa:**
  1. Comércio eletrônico de informática no Brasil e problemas de ruptura de estoque.
  2. Vantagens de desempenho da Linguagem C para sistemas de alto tráfego e baixo overhead.
  3. Trabalhos correlatos e estudos de caso do mercado (Kabum, Pichau, Terabyte, ERPs).
  4. Segurança de dados (LGPD - Lei 13.709/2018) e Acessibilidade Digital (WCAG 2.1).

---

## 💻 5. Etapa 5 – Sistema em C Desenvolvido (`loja_eletronicos`)

* **Tecnologia:** C ANSI/ISO com biblioteca Windows Sockets (`winsock2.h`), multithreading (`CreateThread`) e `CRITICAL_SECTION`.
* **Porta do Servidor:** `8080` (Acessível em `http://localhost:8080`).
* **Frontend:** Interface moderna SPA em HTML5, Tailwind CSS, Lucide Icons e design dark/cyberpunk.
* **Arquivos do Código:**
  * `loja_eletronicos/main.c`: Servidor HTTP, structs de dados (`Product`, `CartItem`, `Order`), rotas REST (`/api/products`, `/api/cart`, `/api/checkout`, `/api/stats`) e gravação em `pedidos_loja.txt`.
  * `loja_eletronicos/web/index.html`: Interface visual interativa com busca, filtros de categoria, carrinho, cupons (`TECH10`, `CYBER20`) e checkout com PIX, Cartão e Boleto.
  * `INICIAR_LOJA_ONLINE.bat`: Inicializador em 1 clique que executa o servidor e abre o navegador automaticamente.

---

## 🚀 Como Continuar no Computador de Casa:
1. Abra a pasta do projeto no **Antigravity** de casa.
2. No chat do Antigravity, envie:
   > *"Estou no computador de casa continuando o PIM II da UNIP. Leia o arquivo RESUMO_E_CONTEXTO_PIM_II.md na pasta do projeto e me ajude com o próximo passo."*
3. O agente lerá este arquivo e continuará imediatamente do ponto exato onde paramos!
