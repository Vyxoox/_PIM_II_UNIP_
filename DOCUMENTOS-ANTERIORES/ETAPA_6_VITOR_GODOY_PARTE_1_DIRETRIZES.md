# ETAPA 6: REDES E SISTEMAS DISTRIBUÍDOS — PARTE 1: DIRETRIZES E REQUISITOS DE INFRAESTRUTURA
**Destinatário:** Vitor Godoy (Responsável pela Etapa 6)  
**Projeto:** PIM II – CST em Análise e Desenvolvimento de Sistemas (UNIP 2026/2)  
**Organização:** CyberHardware Comércio e Soluções em Tecnologia Ltda.  
**Objetivo deste Documento:** Apresentar a arquitetura cliente-servidor real que criamos e detalhar os requisitos de infraestrutura, topologia e segurança para você projetar e redigir o seu capítulo.

---

## 🎯 1. O Que o Manual do PIM II Exige de Você na Etapa 6
Você é o engenheiro de redes e infraestrutura da nossa equipe. O manual da UNIP espera que o seu capítulo entregue:
1. **Arquitetura da Aplicação:** Identificação e justificativa do modelo computacional adotado (Arquitetura Cliente-Servidor Desacoplada com REST sobre HTTP).
2. **Pilha de Protocolos (Modelo OSI / TCP-IP):** Caracterização dos protocolos que atuam em cada camada para permitir que a loja online funcione.
3. **Topologia e Segmentação de Rede:** Desenho da rede da empresa (Sede física na Av. Paulista + Ambiente de Nuvem do e-commerce) com segmentação por VLANs.
4. **Segurança e Políticas de Firewall:** Definição das regras de tráfego (portas abertas e fechadas) e proteção de dados em trânsito (criptografia HTTPS/TLS exigida pela LGPD).
5. **Computação em Nuvem (Cloud Computing):** Proposta de hospedagem escalável (IaaS/PaaS) com balanceamento de carga e alta disponibilidade.

---

## 🌐 2. Como o Nosso Sistema Foi Construído (O que Já Está Funcionando)
Para facilitar seu trabalho e dar total embasamento técnico ao seu capítulo, o nosso backend foi implementado em **Linguagem C nativa utilizando a biblioteca Windows Sockets 2 (`winsock2.h`)**:
* **Servidor Socket TCP:** Escuta conexões na porta de desenvolvimento **`8080`**.
* **Protocolo de Aplicação:** HTTP/1.1 nativo.
* **Cliente:** Navegador web (Chrome, Edge, Firefox) executando uma Single Page Application (SPA) acessível em `http://localhost:8080`.
* **Comunicação REST:** O cliente consome dados via `GET /api/produtos` e envia compras via `POST /api/checkout` em payloads JSON.

---

## 🏢 3. Requisitos de Topologia e Segmentação da Rede da Empresa

Para o relatório, a *CyberHardware* precisa de um desenho de rede profissional dividido em **3 sub-redes (VLANs)** para garantir que uma invasão na loja não comprometa o banco de dados nem os computadores do escritório:

### 1. VLAN 10 – DMZ (Zona Desmilitarizada / Servidores Públicos):
* **Finalidade:** Hospeda o balanceador de carga / proxy reverso e os servidores web em C.
* **Acesso:** Recebe tráfego público da Internet exclusivamente na porta segura `443` (HTTPS).
* **Faixa sugerida:** `192.168.10.0/24`.

### 2. VLAN 20 – Rede Administrativa Interna (Escritório Sede):
* **Finalidade:** Estações de trabalho dos departamentos corporativos (Diretoria, Vendas, T.I., SAC e Financeiro) localizados na sede (Av. Paulista, 10º andar).
* **Acesso:** Acesso à Internet via NAT/Firewall. Acesso aos servidores de produção apenas via VPN criptografada com autenticação de dois fatores (MFA).
* **Faixa sugerida:** `192.168.20.0/24`.

### 3. VLAN 30 – Sub-rede Privada de Banco de Dados:
* **Finalidade:** Hospeda o servidor do SGBD relacional e os backups.
* **Acesso:** **Totalmente isolada da Internet**. Aceita conexões unicamente originadas dos servidores Web C da DMZ (porta `3306`).
* **Faixa sugerida:** `192.168.30.0/24`.

---

## 🔒 4. Requisitos de Segurança que Você Deve Considerar

Ao projetar o seu capítulo, certifique-se de fundamentar:
1. **Transporte Confiável (TCP vs UDP):** Por que o e-commerce utiliza TCP e não UDP? *(Resposta: necessidade de integridade estrita, sequenciamento de pacotes e retransmissão em caso de perda, sem o que compras seriam corrompidas).*
2. **Criptografia em Trânsito (TLS/SSL):** O tráfego na porta 80 (HTTP puro) deve ser automaticamente redirecionado para a porta 443 (HTTPS), garantindo que dados pessoais de clientes (CPF, e-mail e endereços) viajem protegidos contra interceptação (*Man-in-the-Middle*).
3. **Regras de Firewall (Princípio do Menor Privilégio):** Bloquear todo o tráfego por padrão (*Default Deny*) e abrir estritamente as portas necessárias.

---

## 📋 5. Roteiro do Que Você Deve Produzir para a Sua Entrega

Utilize as diretrizes acima para produzir o seu capítulo:
1. [ ] Explicar a arquitetura cliente-servidor HTTP/REST que implementamos em C.
2. [ ] Mapear as 4 camadas da pilha TCP/IP aplicadas à loja online.
3. [ ] Desenhar o Diagrama de Topologia de Rede (em Packet Tracer, Draw.io ou Visio) com a Internet, Firewall, DMZ, VLAN Administrativa e VLAN de Banco de Dados.
4. [ ] Montar a tabela de regras de Firewall (origem, destino, porta, protocolo e ação).
5. [ ] Descrever a infraestrutura em Nuvem (AWS/Azure) com Balanceador de Carga (ELB) e Auto Scaling para suportar picos de tráfego.

> 💡 **Nota de Apoio:** Se você quiser ver a topologia já desenhada, as tabelas de firewall e a fundamentação técnica pronta para usar como base, consulte o arquivo companheiro:  
> 👉 [**`ETAPA_6_VITOR_GODOY_PARTE_2_GABARITO_PRONTO.md`**](file:///c:/Users/vitin/OneDrive/Documentos/_PIM_II_UNIP_/ETAPA_6_VITOR_GODOY_PARTE_2_GABARITO_PRONTO.md)!
