# ETAPA 8: INTEGRAÇÃO E CONCLUSÃO — PARTE 1: DIRETRIZES E ROTEIRO DE EXECUÇÃO
**Destinatário:** Cauê (Responsável pela Etapa 8)  
**Projeto:** PIM II – CST em Análise e Desenvolvimento de Sistemas (UNIP 2026/2)  
**Organização:** CyberHardware Comércio e Soluções em Tecnologia Ltda.  
**Objetivo deste Documento:** Apresentar um roteiro claro do que compõe a Etapa 8 e o resumo do que cada integrante fez nas Etapas 1 a 7, para que você possa redigir a integração e a conclusão geral do trabalho.

---

## 🎯 1. O Que o Manual do PIM II Exige na Etapa 8
A **Etapa 8** é o fechamento de ouro do relatório do PIM II. Os professores avaliadores olham para esta seção para verificar se o trabalho foi realmente integrado ou se foi apenas uma "colcha de retalhos" onde cada aluno escreveu sua parte isolada.

Na sua etapa, você precisa abordar **4 pontos essenciais**:
1. **Integração Multidisciplinar da Solução:** Explicar como as etapas de todos os integrantes (Pesquisa, Engenharia de Software, Banco de Dados, Programação em C, Redes e LGPD/Acessibilidade) trabalham juntas dentro do sistema da **CyberHardware**.
2. **Comparativo Antes vs. Depois (Resolução dos Problemas da Etapa 1):** Mostrar como cada gargalo que a empresa tinha no início foi resolvido pelo sistema.
3. **Benefícios Alcançados para o Negócio:** Destacar os ganhos operacionais, financeiros, técnicos e sociais.
4. **Limitações do Protótipo e Trabalhos Futuros + Considerações Finais:** Apontar quais seriam os próximos passos de evolução do sistema e concluir o trabalho respondendo ao objetivo geral do projeto.

---

## 🧩 2. Resumo do Que Cada Integrante Fez (Para Você Conectar no Texto)

Para facilitar sua escrita, aqui está o mapa exato de como as peças do nosso projeto se encaixam:

* **Etapa 1 (Todos – Caracterização da CyberHardware):**  
  Identificamos que a loja sofria com *ruptura invisível de estoque* (vender produto esgotado por controle manual), *lentidão em horários de pico*, *vulnerabilidade de dados perante a LGPD* e *falta de acessibilidade web*.
* **Etapa 2 (Morgan – Pesquisa Científica e Inovação):**  
  Trouxe dados reais (ABIACOM, SciELO, Sofist/CNN) provando que 70% a 75% das rupturas nascem de falhas internas de sincronização do sistema, que o PIX representa 40% das vendas e que a lentidão gera milhões em prejuízo, além de definir a fórmula matemática do **Ponto de Pedido (Estoque Mínimo)**.
* **Etapa 3 (Kenai – Engenharia de Software Ágil):**  
  Transformou essas dores em **Requisitos Funcionais (RF01–RF06)** e **Não Funcionais (RNF01–RNF06)**, organizou o desenvolvimento em 3 Sprints no Scrum (Product Backlog MoSCoW) e modelou os **4 Casos de Uso UML** (Consultar Catálogo, Gerenciar Carrinho, Efetuar Checkout e Monitorar Estoque).
* **Etapa 4 (Saullo – Modelagem de Banco de Dados):**  
  Criou a estrutura de dados normalizada até a **3ª Forma Normal (3FN)** com **5 tabelas** (`Categoria`, `Produto`, `Cliente`, `Pedido` e `Item_Pedido`), garantindo integridade ACID para vendas e propondo **Redis/MongoDB** como suporte NoSQL.
* **Etapa 5 (Vittor Salvador – Programação Estruturada em C):**  
  Desenvolveu o motor do sistema em **C nativo (`sistema_loja.c`)** com as `structs` espelhando o banco do Saullo, implementou o algoritmo de **Baixa Atômica de Estoque em 2 Fases** (que impede vendas sem saldo), o alerta de estoque mínimo e o **Servidor Web HTTP via Sockets (`winsock2.h`)** na porta `8080` conectado à interface web sóbria (`web/index.html`).
* **Etapa 6 (Vitor Godoy – Redes e Sistemas Distribuídos):**  
  Estruturou a comunicação cliente-servidor TCP/IP, dividiu a rede da empresa em **3 VLANs seguras** (VLAN 10 DMZ para o servidor Web C, VLAN 20 Administrativa e VLAN 30 isolada para o Banco de Dados), definiu as regras de Firewall, HTTPS (porta 443) e escalabilidade em nuvem AWS.
* **Etapa 7 (Gabriel – Direitos Humanos, Inclusão, LGPD e Segurança):**  
  Validou a acessibilidade da interface segundo a **ABNT NBR 17225:2025 e WCAG 2.2** (navegação por teclado, alto contraste, leitores de tela), enquadrou a coleta enxuta de dados nos princípios da **LGPD (Lei 13.709/2018)** e abordou a ética comercial e o descarte sustentável de lixo eletrônico (**Lei 12.305/2010**).

---

## 📋 3. Checklist do Que Você Deve Entregar

1. [ ] Texto explicando o fluxo integrado (desde o clique do cliente na tela acessível, passando pela rede/firewall, processando a baixa atômica no C e garantindo integridade no banco/arquivos).
2. [ ] Quadro comparativo **"Cenário Anterior vs. Cenário Pós-Implantação"**.
3. [ ] Lista de melhorias futuras (ex: conectar o código C diretamente ao MySQL via biblioteca `libmysqlclient`, emitir Nota Fiscal Eletrônica automática e criar app mobile).
4. [ ] Conclusão geral reafirmando o sucesso acadêmico e prático do projeto.

> 💡 **Dica de Ouro:** Para você não precisar começar do zero com o prazo apertado (a entrega vai até `14/11`), já deixei uma versão completa do seu capítulo redigida e formatada no arquivo companheiro:  
> 👉 [**`ETAPA_8_CAUE_PARTE_2_RELATORIO_PRONTO.pdf`**](file:///c:/Users/vitin/OneDrive/Documentos/_PIM_II_UNIP_/ETAPA_8_CAUE_PARTE_2_RELATORIO_PRONTO.pdf). Você pode revisar, ajustar com suas palavras ou usar diretamente no relatório!
