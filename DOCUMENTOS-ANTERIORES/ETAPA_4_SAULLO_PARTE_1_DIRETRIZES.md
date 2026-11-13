# ETAPA 4: BANCO DE DADOS — PARTE 1: DIRETRIZES, ENTIDADES E REGRAS DE NEGÓCIO
**Destinatário:** Saullo (Responsável pela Etapa 4)  
**Projeto:** PIM II – CST em Análise e Desenvolvimento de Sistemas (UNIP 2026/2)  
**Organização:** CyberHardware Comércio e Soluções em Tecnologia Ltda.  
**Objetivo deste Documento:** Fornecer o briefing com as entidades, relacionamentos, cardinalidades e regras do negócio para que você possa pensar, projetar e desenvolver a sua modelagem de banco de dados.

---

## 🎯 1. O Que o Manual do PIM II Exige de Você na Etapa 4
Você é o engenheiro de dados da nossa equipe. O manual da UNIP espera que o seu capítulo entregue:
1. **Modelo Conceitual:** Levantamento das entidades, seus atributos e o Diagrama Entidade-Relacionamento (DER).
2. **Modelo Lógico e Normalização:** Conversão do DER para tabelas com chaves primárias (`PK`), estrangeiras (`FK`) e a aplicação formal das três formas normais (1FN, 2FN e 3FN).
3. **Modelo Físico:** Scripts SQL de criação (`DDL`) e de inserção de dados de teste (`DML`).
4. **Estudo Comparativo NoSQL:** Justificativa da escolha do banco relacional para vendas e proposta de uso de NoSQL (ex: Redis ou MongoDB) como solução complementar.

---

## 🏢 2. Contexto do Sistema CyberHardware
O nosso e-commerce de hardware já possui um backend funcional em C e uma interface web SPA. Para que tudo fique 100% integrado no relatório final, o seu banco de dados precisa refletir a mesma lógica que o sistema já executa:
* O cliente navega no catálogo de peças de informática e periféricos organizados por categorias.
* O cliente adiciona itens ao carrinho e realiza o checkout.
* Cada venda dá baixa automática no estoque dos produtos comprados e gera um pedido com valor bruto, desconto de cupom, valor total faturado e forma de pagamento (PIX, Cartão ou Boleto).

---

## 🗂️ 3. As 5 Entidades do Sistema

Para atender a todos os requisitos do PIM II sem inflar o escopo, o sistema necessita de **5 entidades fundamentais**:

1. **CATEGORIA:** Agrupa os tipos de componentes (ex: *Placas de Vídeo*, *Processadores*, *Periféricos*, *Monitores*, *Armazenamento*).
2. **PRODUTO:** Representa o hardware comercializado, contendo nome comercial, preço unitário, saldo atual de estoque, estoque mínimo de alerta e especificações técnicas.
3. **CLIENTE:** Cadastro do comprador para fins fiscais e de entrega (em conformidade com a LGPD: nome completo, CPF único, e-mail único e endereço de entrega).
4. **PEDIDO:** Representa a transação comercial concluída (cabeçalho da venda com código único, cliente associado, data/hora, subtotal, desconto, valor final, forma de pagamento e status).
5. **ITEM_PEDIDO:** Entidade associativa indispensável para detalhar os itens contidos em cada pedido (produto associado, quantidade comprada, preço unitário congelado na data da venda e subtotal do item).

---

## 🔗 4. Mapeamento de Relacionamentos e Cardinalidades

Ao desenhar o seu Modelo Conceitual (DER), considere as seguintes cardinalidades:

### Relacionamento 1: CATEGORIA ➔ PRODUTO
* **Descrição:** Uma categoria classifica vários produtos, mas cada produto pertence a apenas uma categoria.
* **Cardinalidade:** `1:N` (Um para Muitos).
* **Mapeamento:** A chave primária de `CATEGORIA` (`id_categoria`) migra como chave estrangeira (`FK`) para dentro da tabela `PRODUTO`.

### Relacionamento 2: CLIENTE ➔ PEDIDO
* **Descrição:** Um cliente cadastrado pode realizar nenhum, um ou vários pedidos ao longo do tempo. Por outro lado, um pedido registrado pertence obrigatoriamente a um único cliente.
* **Cardinalidade:** `1:N` (Um para Muitos).
* **Mapeamento:** A chave primária de `CLIENTE` (`id_cliente`) migra como chave estrangeira (`FK`) para dentro da tabela `PEDIDO`.

### Relacionamento 3: PEDIDO ➔ PRODUTO (Relação N:N resolvida por ITEM_PEDIDO)
* **Descrição:** Um pedido pode conter vários produtos diferentes. Um mesmo produto pode estar presente em diversos pedidos distintos de clientes diferentes.
* **Cardinalidade Original:** `N:N` (Muitos para Muitos).
* **Regra de Modelagem Relacional:** Em bancos relacionais, uma relação `N:N` **não pode existir diretamente** no modelo lógico/físico, pois geraria redundância e violaria a 1FN. 
* **Resolução:** Cria-se a entidade associativa **`ITEM_PEDIDO`**, que transforma a relação em:
  * `PEDIDO` ➔ `ITEM_PEDIDO` (1:N)
  * `PRODUTO` ➔ `ITEM_PEDIDO` (1:N)
* **Mapeamento:** `ITEM_PEDIDO` recebe as chaves estrangeiras (`id_pedido` e `id_produto`), além de guardar a quantidade e o preço unitário congelado no momento da compra.

---

## ⚖️ 5. Regras de Negócio e Restrições de Integridade para Considerar

No momento de criar o seu modelo lógico e os scripts, certifique-se de contemplar:
1. **Integridade de Estoque:** A coluna `quantidade_estoque` nunca pode ser negativa (utilizar restrição `CHECK (quantidade_estoque >= 0)`).
2. **Preço Válido:** O preço de venda deve ser maior que zero (`CHECK (preco > 0)`).
3. **Unicidade de Identificação:** O CPF e o E-mail do cliente não podem se repetir no banco (`UNIQUE`).
4. **Código Único de Pedido:** Cada venda precisa de um identificador comercial amigável e único (ex: `PED-00001`, com restrição `UNIQUE`).
5. **Histórico de Preço:** O preço unitário do produto deve ser gravado dentro de `ITEM_PEDIDO`. *Motivo:* se a loja aumentar o preço da placa de vídeo no mês seguinte, os pedidos antigos realizados no mês anterior não podem ter seus valores alterados!

---

## 📋 6. Roteiro do Que Você Deve Produzir para a Sua Entrega

Utilize as diretrizes acima para produzir o seu capítulo:
1. [ ] Montar o texto explicando as entidades e atributos do domínio da CyberHardware.
2. [ ] Desenhar o Diagrama Entidade-Relacionamento (DER) em ferramenta visual (ex: brModelo, Draw.io, Lucidchart ou Astah).
3. [ ] Redigir a fundamentação da Normalização explicando como a 1FN, 2FN e 3FN foram atendidas.
4. [ ] Escrever os scripts SQL (`CREATE TABLE` com PKs, FKs e restrições, e `INSERT` com os dados).
5. [ ] Elaborar a seção comparativa de NoSQL (quando usar SQL ACID vs NoSQL como Redis/MongoDB).

> 💡 **Nota de Apoio:** Se você tiver qualquer dúvida na modelagem ou quiser validar se o seu script e DER estão corretos, consulte o arquivo companheiro:  
> 👉 [**`ETAPA_4_SAULLO_PARTE_2_GABARITO_PRONTO.md`**](file:///c:/Users/vitin/OneDrive/Documentos/_PIM_II_UNIP_/ETAPA_4_SAULLO_PARTE_2_GABARITO_PRONTO.md), que contém a solução de referência completa já testada e validada com o nosso sistema!
