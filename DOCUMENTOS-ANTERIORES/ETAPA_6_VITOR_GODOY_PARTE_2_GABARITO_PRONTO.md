# UNIVERSIDADE PAULISTA (UNIP)
## CST em Análise e Desenvolvimento de Sistemas — PIM II

**Relatório Técnico — Etapa 6: Redes de Computadores e Sistemas Distribuídos**  
**Projeto:** Infraestrutura em Nuvem, Topologia e Segurança para o CyberHardware E-Commerce  
**Disciplina:** Projeto Integrado Multidisciplinar II (PIM II)  
**Aluno:** Vitor Godoy  
**Campus:** Mirassol / SP — 2026  

---

## 1. Visão Geral da Arquitetura Cliente-Servidor e Alinhamento à Etapa 3 (`PIM.pdf`)

A infraestrutura de redes e sistemas distribuídos do **CyberHardware E-Commerce** materializa a arquitetura lógica em três camadas definida na Etapa 3 (`PIM.pdf`, p. 8–9) — **Apresentação (View)**, **Lógica de Negócio (Controller)** e **Persistência de Dados (Model)** — conectando os clientes Web ao motor de aplicação em Linguagem C (`sistema_loja.c`, Etapa 5) e ao banco de dados relacional (Etapa 4) sob os requisitos não funcionais de **segurança (`RNF01`, `RNF02`)**, **desempenho `<= 2s` (`RNF04`)**, **disponibilidade em nuvem `>= 99,5%` (`RNF05`)** e **backups automatizados (`RNF06`)**.

* **Camada de Apresentação (`web/index.html` + `/img/*.svg`):** entregue em HTTPS (TLS 1.3) por proxy reverso com acessibilidade WCAG 2.1 (`RNF03`).
* **Camada de Aplicação Distribuída (`sistema_loja.exe` via Winsock2/POSIX HTTP/1.1):** processa autenticação com *Bearer Tokens* e hash SHA-256 (`/api/auth/*`), catálogo e gestão administrativa (`/api/produtos`), estimativa regional de frete (`/api/frete`), checkout atômico em 2 fases (`/api/checkout`), telemetria (`/api/status`) e histórico/rastreamento de pedidos (`/api/pedidos`, `/api/pedidos/status`).
* **Integração Externa (Gateway de Pagamento — `UC05`):** conforme o Diagrama de Sequência da Etapa 3 (`PIM.pdf`, p. 11), a aplicação realiza chamadas *outbound* criptografadas (TLS 1.3 porta `443`) ao Gateway de Pagamento para aprovar ou recusar transações via PIX, Cartão ou Boleto antes de persistir a baixa definitiva do estoque.

---

## 2. Mapeamento TCP/IP vs. OSI e Justificativa do Protocolo de Transporte

| Camada TCP/IP | Camada OSI Equivalente | Protocolos e Artefatos na CyberHardware | Responsabilidade no Sistema |
| :--- | :--- | :--- | :--- |
| **Aplicação** | Aplicação, Apresentação e Sessão | HTTPS (TLS 1.3), HTTP/1.1 REST (JSON), DNS, SSH (VPN) | Transporte das APIs (`/api/auth/*`, `/api/checkout`, `/api/pedidos`) e cifragem ponta a ponta (`RNF02`). |
| **Transporte** | Transporte | **TCP** (`SOCK_STREAM`, portas `443`, `8080`, `3306`) | Garante entrega ordenada, *handshake* em 3 vias e controle de fluxo entre cliente, servidor C e SGBD. |
| **Internet / Rede** | Rede | IPv4, Sub-redes VLAN (`192.168.10/20/30.0/24`), VPN `10.8.0.0/24` | Roteamento isolado por segmento e filtragem L3 entre DMZ, Aplicação e Banco de Dados. |
| **Acesso à Rede** | Enlace e Física | Ethernet Gigabit Virtual (VPC Cloud), Switches L2 (802.1Q) | Encapsulamento de quadros e isolamento lógico em zona de disponibilidade múltipla (Multi-AZ). |

### Por que TCP e não UDP?
Na programação de *sockets* em C (`socket(AF_INET, SOCK_STREAM, 0)`), adota-se exclusivamente o **TCP (Transmission Control Protocol)**. Diferentemente do UDP, que é não orientado à conexão e tolera perda ou reordenação de pacotes (adequado a *streaming* ou telemetria bruta), uma plataforma de e-commerce **não pode tolerar pacotes perdidos ou duplicados** durante o envio de credenciais, validação de estoque ou liquidação de checkout com Gateway de Pagamento. O TCP assegura retransmissão automática e ordem exata dos bytes lidos pelo servidor via `recv()` usando o cabeçalho `Content-Length`.

---

## 3. Topologia de Rede em Nuvem e Segmentação Lógica (VLANs)

Para eliminar o risco de exposição direta do backend e do SGBD à internet pública, a arquitetura implementa defesa em profundidade com três sub-redes isoladas (VLANs), **Web Application Firewall (WAF)**, **Elastic Load Balancer (ELB)** e túnel administrativo restrito por **VPN (`10.8.0.0/24`) + MFA**:

![Topologia de Rede Distribuida CyberHardware](diagramas/etapa6_topologia_rede.png)

* **VLAN 10 — DMZ (`192.168.10.0/24`):** recebe o tráfego público HTTPS (`443`) após o WAF/ELB, realiza a terminação do certificado SSL/TLS (`RNF02`), serve os ativos estáticos da SPA e atua como proxy reverso encaminhando chamadas `/api/*` para a porta interna `8080` na VLAN 20.
* **VLAN 20 — Aplicação (`192.168.20.0/24`):** executa o cluster de instâncias do motor em C sob **Auto Scaling** (escalonamento horizontal automático ao atingir **CPU > 70%**). Não possui IP público e mantém canal de saída controlado (`443`) até o **Gateway de Pagamento** externo (`UC05`).
* **VLAN 30 — Dados e Backup (`192.168.30.0/24`):** abriga o SGBD Relacional (`MySQL 8.0`, porta `3306`) sem rota para a internet. Realiza replicação Multi-AZ e **backups diários automatizados** criptografados (AES-256) para armazenamento em objeto (AWS S3 / Azure Blob), cumprindo o `RNF06`.

---

## 4. Política de Firewall (Default Deny) e Segurança LGPD / HTTPS

| Regra | Origem | Destino | Porta / Protocolo | Ação | Finalidade e Conformidade |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **R01** | Internet (`0.0.0.0/0`) | Borda ELB / DMZ (`192.168.10.0/24`) | TCP `443` (HTTPS / TLS 1.3) | **ALLOW** | Acesso público cifrado (`RNF02`), protegendo senhas e dados na LGPD (`RNF01`). |
| **R02** | Internet (`0.0.0.0/0`) | Qualquer VLAN (`192.168.x.x`) | TCP `8080` e `3306` | **DENY** | **Bloqueia acesso externo direto** ao servidor C (`8080`) e ao banco de dados (`3306`). |
| **R03** | VLAN 10 DMZ (`192.168.10.0/24`) | VLAN 20 App (`192.168.20.0/24`) | TCP `8080` (HTTP Interno) | **ALLOW** | Proxy reverso autenticado encaminha requisições REST ao backend C. |
| **R04** | VLAN 20 App (`192.168.20.0/24`) | VLAN 30 Dados (`192.168.30.0/24`) | TCP `3306` (SQL) | **ALLOW** | Permite persistência relacional exclusivamente oriunda da camada de aplicação. |
| **R05** | VLAN 20 App (`192.168.20.0/24`) | Gateway de Pagamento (*Allowlist*) | TCP `443` (Outbound TLS) | **ALLOW** | Validação externa de pagamentos (`PIX`, `CARTAO`, `BOLETO` — `UC05`). |
| **R06** | VPN Admin (`10.8.0.0/24` + MFA) | VLAN 20 / VLAN 30 | TCP `22` (SSH) / `443` | **ALLOW** | Administração segura para gestão de catálogo/estoque (`UC06`/`UC07`). |
| **R99** | Qualquer (`*`) | Qualquer (`*`) | Todos (`ANY`) | **DENY** | **Default Deny** com registro de auditoria (*logs* de tentativas bloqueadas). |

No plano de aplicação (`sistema_loja.c`), a segurança em trânsito (HTTPS/TLS na borda) é complementada pela criptografia irreversível em repouso (**SHA-256 com salt aleatório** para senhas em `usuarios_loja.txt` e tabela `usuario`) e pela omissão deliberada de *hashes* nas respostas JSON da API, permitindo apenas tráfego estritamente necessário para cada sessão autenticada.

---

## 5. Escalabilidade em Nuvem, SLA (`RNF05`) e Latência (`RNF04`)

* **Disponibilidade e Cloud (`RNF05`):** hospedagem em **AWS** ou **Microsoft Azure** distribuída em **múltiplas Zonas de Disponibilidade (Multi-AZ)** com **ELB** e instância *Standby* do banco relacional na VLAN 30. Essa topologia elimina pontos únicos de falha (SPOF) e garante **SLA mínimo de 99,5%** de uptime mensal exigido pelo projeto.
* **Desempenho de API e Pesquisa no Catálogo (`RNF04 <= 2 segundos`):** por ser compilado diretamente em código de máquina (`gcc -O2`) sem camadas pesadas de máquina virtual, o motor REST em C respondeu entre **0,42 ms e 55,12 ms** nos testes locais de integração (incluindo autenticação SHA-256, cálculo de frete, catálogo e checkout com persistência em arquivo). Somando a sobrecarga de TLS 1.3 e latência WAN (~80 a 200 ms no Brasil), o tempo total ponta a ponta mantém-se confortavelmente abaixo de `300 ms`, atendendo com ampla margem o requisito `<= 2 segundos` (`RNF04`).

---

## 6. Referências

ASSOCIAÇÃO BRASILEIRA DE NORMAS TÉCNICAS. **NBR ISO/IEC 27001**: Segurança da informação, segurança cibernética e proteção à privacidade. Rio de Janeiro, 2022.

BRASIL. **Lei nº 13.709, de 14 de agosto de 2018**. Lei Geral de Proteção de Dados Pessoais (LGPD). Brasília, DF: Presidência da República, 2018.

KUROSE, James F.; ROSS, Keith W. **Redes de computadores e a Internet**: uma abordagem top-down. 8. ed. São Paulo: Pearson, 2021.

TANENBAUM, Andrew S.; FEAMSTER, Nick; WETHERALL, David J. **Redes de computadores**. 6. ed. São Paulo: Pearson, 2021.
