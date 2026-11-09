# Relatório Técnico: Sistema de E-commerce - CyberHardware (Etapa 7)

**Curso:** CST em Análise e Desenvolvimento de Sistemas  
**Disciplina:** PIM II – Projeto Integrado Multidisciplinar (Direitos Humanos, Inclusão e Segurança)  
**Aluno:** Gabriel  
**Instituição:** Universidade Paulista (UNIP) – Mirassol / SP  
**Ano:** 2026  

---

## Sumário
1. Acessibilidade Digital e Inclusão na Plataforma CyberHardware
2. Privacidade e Proteção de Dados Pessoais (Conformidade LGPD)
3. Segurança da Informação e Confiabilidade Operacional
4. Direitos Humanos, Aspectos Éticos e Impactos Socioambientais
5. Referências Bibliográficas

---

## 1. Acessibilidade Digital e Inclusão na Plataforma CyberHardware

### 1.1. Contextualização e Inclusão Digital
A acessibilidade digital constitui um pilar indispensável no desenvolvimento de sistemas modernos de comércio eletrônico, pois assegura que pessoas com diferentes características físicas, sensoriais, motoras e cognitivas — bem como usuários com distintos níveis de letramento digital — utilizem os serviços disponibilizados de maneira autônoma, segura e eficiente. Na plataforma de e-commerce desenvolvida para a **CyberHardware Comércio e Soluções em Tecnologia Ltda.**, a inclusão digital foi incorporada desde a concepção dos requisitos (Etapa 3) até a construção da interface web e do motor transacional em C (Etapa 5).

### 1.2. Fundamentação Legal e Normativa de Acessibilidade
O desenvolvimento da interface da CyberHardware observa rigorosamente o arcabouço legal brasileiro e as normas técnicas internacionais de usabilidade e acessibilidade:
* **Lei nº 13.146/2015 (Lei Brasileira de Inclusão da Pessoa com Deficiência – LBI):** Estabelece, em seu artigo 63, a obrigatoriedade de acessibilidade nos sítios da internet mantidos por empresas com sede ou representação comercial no País, garantindo igualdade de oportunidades e autonomia (BRASIL, 2015).
* **ABNT NBR 17225:2025:** Define os requisitos técnicos atuais de acessibilidade para conteúdos e aplicações web no Brasil, visando eliminar barreiras que dificultem a navegação, a compreensão e a interação dos usuários (ABNT, 2025).
* **WCAG 2.2 (Web Content Accessibility Guidelines):** Reúne as recomendações do *World Wide Web Consortium* fundamentadas nos quatro princípios fundamentais — **Perceptível, Operável, Compreensível e Robusto (POUR)** (W3C, 2023).
* **ABNT NBR ISO 9241-11:2018:** Fornece as diretrizes de ergonomia da interação humano-sistema e usabilidade, pautadas na eficácia, eficiência e satisfação do usuário no contexto de compra (ABNT, 2018).
* **ABNT NBR 17060:2022:** Estabelece parâmetros de acessibilidade em interfaces móveis e responsivas (ABNT, 2022).

### 1.3. Aplicação Prática da Acessibilidade na Solução Desenvolvida
Em alinhamento direto com o requisito não funcional **RNF03** (definido na Etapa 3), a interface web da CyberHardware (`web/index.html`) implementou mecanismos concretos e verificáveis de acessibilidade:

| Critério Normativo (WCAG 2.2 / NBR 17225) | Implementação Realizada no Sistema CyberHardware | Benefício para o Usuário |
|---|---|---|
| **Navegação por Teclado e Link de Salto** | Inclusão do atalho oculto *"Pular para o conteúdo principal"* (`href="#main-content"`) acionável via tecla `Tab` com foco visível destacado. | Permite que usuários com deficiência motora ou visual acessem o catálogo sem percorrer todo o cabeçalho repetidamente. |
| **Semântica e Tecnologias Assistivas (ARIA)** | Uso de marcos estruturais semânticos (`<header role="banner">`, `<main>`, `<nav aria-label="...">`, `<div role="dialog" aria-modal="true">`). | Garante leitura ordenada e compreensível por leitores de tela (NVDA, JAWS e VoiceOver). |
| **Contraste Visual Sóbrio e Legibilidade** | Adoção de paleta corporativa de alto contraste (texto escuro `#0f172a` sobre superfície clara `#ffffff` e botões em azul sólido `#1e40af`). | Atende aos níveis AA/AAA de contraste cromático, auxiliando pessoas com baixa visão ou daltonismo. |
| **Independência de Cor nos Indicadores** | O status do estoque não depende apenas de cores (verde/amarelo/vermelho), exibindo rótulos textuais explícitos (*"X un. em estoque"* ou *"Sem Estoque"*). | Impede ambiguidades para usuários com dificuldades na percepção de cores ou que utilizam sintetizadores de voz. |
| **Formulários Identificados e Compreensíveis** | Todos os campos do checkout (Nome, CPF, E-mail e Endereço) possuem rótulos `<label for="...">` vinculados e instruções diretas de preenchimento. | Reduz erros operacionais durante a finalização da compra e facilita a compreensão por pessoas com menor experiência digital. |

---

## 2. Privacidade e Proteção de Dados Pessoais (Conformidade LGPD)

### 2.1. Enquadramento na Lei Geral de Proteção de Dados (Lei nº 13.709/2018)
O comércio eletrônico exige a coleta de dados pessoais para viabilizar o faturamento fiscal e a entrega logística das mercadorias. Na plataforma da CyberHardware, o tratamento de informações pessoais obedece estritamente aos princípios estabelecidos no **Art. 6º da LGPD** (BRASIL, 2018), aplicando o conceito de **Privacidade desde a Concepção (*Privacy by Design*)**:

1. **Princípio da Finalidade e Adequação (Art. 6º, I e II):** Os dados solicitados no checkout possuem finalidade específica, explícita e legítima: emissão do comprovante do pedido e despacho físico do hardware adquirido.
2. **Princípio da Necessidade / Minimização de Dados (Art. 6º, III):** Tanto na `struct Cliente` em linguagem C (Etapa 5) quanto na tabela `Cliente` do banco de dados relacional (Etapa 4), a coleta restringe-se ao mínimo indispensável (`nome_completo`, `cpf`, `email` e `endereco`). Não são coletados dados pessoais sensíveis (Art. 5º, II, como origem racial, convicção religiosa ou dados biométricos).
3. **Base Legal para o Tratamento (Art. 7º, V e II):** O processamento fundamenta-se na **execução de contrato de compra e venda** a pedido do titular e no **cumprimento de obrigação legal/fiscal** (emissão de documentos fiscais eletrônicos).
4. **Direitos do Titular (Art. 18 da LGPD):** A arquitetura do banco de dados relacional normalizado (com restrições `UNIQUE` para CPF e e-mail na tabela `Cliente`) permite localizar, atualizar, corrigir ou anonimizar rapidamente os registros mediante solicitação do consumidor.

---

## 3. Segurança da Informação e Confiabilidade Operacional

A segurança da informação na CyberHardware baseia-se na preservação da tríade **CID (Confidencialidade, Integridade e Disponibilidade)**, articulando medidas técnicas entre o código em C (Etapa 5), o banco de dados (Etapa 4) e a infraestrutura de redes (Etapa 6):

### 3.1. Confidencialidade
* **Não Armazenamento de Dados Financeiros Críticos:** No fluxo de pagamento via PIX, Cartão de Crédito ou Boleto, o sistema **não armazena números de cartão de crédito, códigos CVV ou senhas financeiras** em disco (`pedidos_loja.txt`) nem nas tabelas do banco de dados, registrando apenas a modalidade escolhida e o status da transação.
* **Isolamento de Rede e Criptografia em Trânsito:** Conforme projetado na Etapa 6, o tráfego externo ocorre exclusivamente sob protocolo seguro **HTTPS/TLS (porta 443)**, enquanto o servidor de banco de dados permanece isolado na sub-rede privada (**VLAN 30**), inacessível diretamente a partir da Internet.

### 3.2. Integridade
* **Prevenção contra Estouro de Buffer (*Buffer Overflow*):** No código em C (`sistema_loja.c`), a leitura de entradas textuais utiliza a função segura `fgets()` com limite estrito de bytes (`sizeof`) e sanitização de quebras de linha, prevenindo corrupção de memória.
* **Integridade Transacional e Baixa Atômica:** A rotina de fechamento de venda em duas fases no backend em C, aliada às restrições `CHECK (quantidade_estoque >= 0)` e `CHECK (preco_unitario > 0)` no SGBD relacional, impede a gravação de saldos negativos ou preços adulterados.

### 3.3. Disponibilidade e Rastreabilidade
* **Persistência e Auditoria Contínua:** Todas as vendas aprovadas são gravadas imediatamente em armazenamento não volátil (`pedidos_loja.txt` em modo *append* `"a+"`), garantindo rastreabilidade com código único (`PED-XXXXX`) e carimbo de data e hora (`DD/MM/AAAA HH:MM:SS`).
* **Resiliência na Nuvem:** A infraestrutura prevê balanceamento de carga e rotinas diárias de backup automatizado do banco de dados para rápida recuperação em caso de incidentes.

---

## 4. Direitos Humanos, Aspectos Éticos e Impactos Socioambientais

### 4.1. Ética Comercial e Defesa do Consumidor
Em conformidade com a **Declaração Universal dos Direitos Humanos** (ONU, 1948), a **Constituição Federal de 1988** (BRASIL, 1988), o **Código de Defesa do Consumidor – Lei nº 8.078/1990** (BRASIL, 1990) e o **Decreto nº 7.962/2013** (Lei do Comércio Eletrônico), a plataforma CyberHardware adota práticas éticas transparentes:
* **Transparência de Preços e Descontos:** O carrinho de compras e o modal de checkout exibem com clareza o preço unitário, o subtotal bruto, o valor exato abatido por cupons promocionais (`TECH10` ou `CYBER20`), a gratuidade do frete e o valor total final antes da confirmação do pedido, sem taxas ocultas ou interfaces manipulativas (*dark patterns*).
* **Combate à Ruptura Invisível como Dever Ético:** Conforme demonstrado na pesquisa da Etapa 2, vender um produto sem estoque físico gera frustração e lesão ao consumidor. O bloqueio preventivo de itens sem estoque na interface e no código em C assegura respeito à boa-fé objetiva nas relações de consumo.
* **Igualdade e Não Discriminação:** O sistema garante tratamento isonômico a todos os compradores, exibindo os mesmos preços, descontos e disponibilidade técnica independentemente de localização ou perfil do usuário.

### 4.2. Impactos Sociais e Responsabilidade Ambiental (Resíduos Eletrônicos)
Como empresa comercializadora de componentes de informática e periféricos (placas de vídeo, processadores, monitores e SSDs), a CyberHardware reconhece sua responsabilidade perante a **Lei nº 12.305/2010 (Política Nacional de Resíduos Sólidos)** (BRASIL, 2010):
* **Consumo Consciente e Durabilidade:** O catálogo apresenta especificações técnicas claras para que o cliente adquira componentes compatíveis com sua necessidade real, evitando trocas prematuras e desperdício tecnológico.
* **Logística Reversa e Descarte Adequado (*E-Waste*):** Nas comunicações pós-venda e informações institucionais, a empresa orienta os consumidores sobre o descarte ambientalmente correto de hardwares obsoletos e embalagens em pontos de coleta de lixo eletrônico, mitigando impactos ambientais de metais pesados.

---

## 5. Considerações Finais da Etapa 7

A integração dos preceitos de Direitos Humanos, Acessibilidade Digital (ABNT NBR 17225:2025 e WCAG 2.2), Proteção de Dados (LGPD) e Segurança da Informação consolida a maturidade técnica e social do projeto da CyberHardware. Mais do que cumprir exigências formais, a solução demonstra que o desempenho computacional do backend em C e a modelagem rigorosa de dados caminham em harmonia com a inclusão social, a ética comercial, a privacidade do cidadão e a sustentabilidade ambiental.

---

## 6. Referências Bibliográficas (Normas ABNT NBR 6023)

* ASSOCIAÇÃO BRASILEIRA DE NORMAS TÉCNICAS. **ABNT NBR 17060**: Acessibilidade em aplicativos de dispositivos móveis – Requisitos. Rio de Janeiro: ABNT, 2022.
* ASSOCIAÇÃO BRASILEIRA DE NORMAS TÉCNICAS. **ABNT NBR 17225**: Acessibilidade em conteúdo e aplicações web – Requisitos. Rio de Janeiro: ABNT, 2025.
* ASSOCIAÇÃO BRASILEIRA DE NORMAS TÉCNICAS. **ABNT NBR ISO 9241-11**: Ergonomia da interação humano-sistema – Parte 11: Usabilidade: definições e conceitos. Rio de Janeiro: ABNT, 2018.
* BRASIL. **Constituição da República Federativa do Brasil de 1988**. Brasília, DF: Presidência da República, 1988. Disponível em: <https://www.planalto.gov.br/ccivil_03/constituicao/constituicao.htm>. Acesso em: 28 out. 2026.
* BRASIL. **Decreto nº 7.962, de 15 de março de 2013**. Regulamenta a Lei nº 8.078, de 11 de setembro de 1990, para dispor sobre a contratação no comércio eletrônico. Brasília, DF: Presidência da República, 2013. Disponível em: <https://www.planalto.gov.br/ccivil_03/_ato2011-2014/2013/decreto/d7962.htm>. Acesso em: 28 out. 2026.
* BRASIL. **Lei nº 8.078, de 11 de setembro de 1990**. Dispõe sobre a proteção do consumidor e dá outras providências. Brasília, DF: Presidência da República, 1990. Disponível em: <https://www.planalto.gov.br/ccivil_03/leis/l8078compilado.htm>. Acesso em: 28 out. 2026.
* BRASIL. **Lei nº 12.305, de 2 de agosto de 2010**. Institui a Política Nacional de Resíduos Sólidos. Brasília, DF: Presidência da República, 2010. Disponível em: <https://www.planalto.gov.br/ccivil_03/_ato2007-2010/2010/lei/l12305.htm>. Acesso em: 28 out. 2026.
* BRASIL. **Lei nº 13.146, de 6 de julho de 2015**. Institui a Lei Brasileira de Inclusão da Pessoa com Deficiência (Estatuto da Pessoa com Deficiência). Brasília, DF: Presidência da República, 2015. Disponível em: <https://www.planalto.gov.br/ccivil_03/_ato2015-2018/2015/lei/l13146.htm>. Acesso em: 28 out. 2026.
* BRASIL. **Lei nº 13.709, de 14 de agosto de 2018**. Lei Geral de Proteção de Dados Pessoais (LGPD). Brasília, DF: Presidência da República, 2018. Disponível em: <https://www.planalto.gov.br/ccivil_03/_ato2015-2018/2018/lei/l13709.htm>. Acesso em: 28 out. 2026.
* ORGANIZAÇÃO DAS NAÇÕES UNIDAS (ONU). **Declaração Universal dos Direitos Humanos**. Paris: ONU, 1948. Disponível em: <https://www.un.org/en/about-us/universal-declaration-of-human-rights>. Acesso em: 28 out. 2026.
* WORLD WIDE WEB CONSORTIUM (W3C). **Web Content Accessibility Guidelines (WCAG) 2.2**. [S. l.]: W3C, 2023. Disponível em: <https://www.w3.org/TR/WCAG22/>. Acesso em: 28 out. 2026.
