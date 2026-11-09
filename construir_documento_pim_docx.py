# -*- coding: utf-8 -*-
import os
import docx
from docx.shared import Inches, Pt, RGBColor, Cm
from docx.enum.text import WD_ALIGN_PARAGRAPH
from docx.enum.table import WD_TABLE_ALIGNMENT
from docx.oxml import OxmlElement, parse_xml
from docx.oxml.ns import nsdecls, qn

def create_pim_document():
    doc = docx.Document()

    # 1. Configuração de Margens ABNT (Superior 3cm, Esquerda 3cm, Inferior 2cm, Direita 2cm)
    for section in doc.sections:
        section.top_margin = Cm(3.0)
        section.left_margin = Cm(3.0)
        section.bottom_margin = Cm(2.0)
        section.right_margin = Cm(2.0)
        section.header_distance = Cm(1.5)
        section.footer_distance = Cm(1.5)

    # 2. Configuração de Estilos Básicos
    style_normal = doc.styles['Normal']
    font_normal = style_normal.font
    font_normal.name = 'Arial'
    font_normal.size = Pt(12)
    font_normal.color.rgb = RGBColor(33, 37, 41)

    # Helpers de formatação
    def set_cell_background(cell, fill_hex):
        shading_elm = parse_xml(f'<w:shd {nsdecls("w")} w:fill="{fill_hex}"/>')
        cell._tc.get_or_add_tcPr().append(shading_elm)

    def set_cell_margins(cell, top=100, bottom=100, left=140, right=140):
        tcPr = cell._tc.get_or_add_tcPr()
        tcMar = OxmlElement('w:tcMar')
        for m, val in [('top', top), ('bottom', bottom), ('left', left), ('right', right)]:
            node = OxmlElement(f'w:{m}')
            node.set(qn('w:w'), str(val))
            node.set(qn('w:type'), 'dxa')
            tcMar.append(node)
        tcPr.append(tcMar)

    def p_normal(text, bold_prefix=None, italic=False, space_after=6):
        p = doc.add_paragraph()
        p.alignment = WD_ALIGN_PARAGRAPH.JUSTIFY
        p.paragraph_format.line_spacing = 1.5
        p.paragraph_format.first_line_indent = Cm(1.25)
        p.paragraph_format.space_before = Pt(0)
        p.paragraph_format.space_after = Pt(space_after)
        if bold_prefix:
            r_bold = p.add_run(bold_prefix)
            r_bold.bold = True
            r_bold.font.name = 'Arial'
            r_bold.font.size = Pt(12)
        r = p.add_run(text)
        r.italic = italic
        r.font.name = 'Arial'
        r.font.size = Pt(12)
        return p

    def p_bullet(text, bold_prefix=None):
        p = doc.add_paragraph(style='List Bullet')
        p.alignment = WD_ALIGN_PARAGRAPH.JUSTIFY
        p.paragraph_format.line_spacing = 1.5
        p.paragraph_format.space_before = Pt(0)
        p.paragraph_format.space_after = Pt(3)
        if bold_prefix:
            r_bold = p.add_run(bold_prefix)
            r_bold.bold = True
            r_bold.font.name = 'Arial'
            r_bold.font.size = Pt(12)
        r = p.add_run(text)
        r.font.name = 'Arial'
        r.font.size = Pt(12)
        return p

    def add_h1(text, page_break=True):
        if page_break:
            doc.add_page_break()
        p = doc.add_paragraph()
        p.alignment = WD_ALIGN_PARAGRAPH.LEFT
        p.paragraph_format.line_spacing = 1.5
        p.paragraph_format.space_before = Pt(12)
        p.paragraph_format.space_after = Pt(12)
        r = p.add_run(text.upper())
        r.bold = True
        r.font.name = 'Arial'
        r.font.size = Pt(14)
        r.font.color.rgb = RGBColor(15, 23, 42)
        return p

    def add_h2(text):
        p = doc.add_paragraph()
        p.alignment = WD_ALIGN_PARAGRAPH.LEFT
        p.paragraph_format.line_spacing = 1.5
        p.paragraph_format.space_before = Pt(14)
        p.paragraph_format.space_after = Pt(6)
        r = p.add_run(text)
        r.bold = True
        r.font.name = 'Arial'
        r.font.size = Pt(12)
        r.font.color.rgb = RGBColor(30, 41, 59)
        return p

    def add_h3(text):
        p = doc.add_paragraph()
        p.alignment = WD_ALIGN_PARAGRAPH.LEFT
        p.paragraph_format.line_spacing = 1.5
        p.paragraph_format.space_before = Pt(10)
        p.paragraph_format.space_after = Pt(4)
        r = p.add_run(text)
        r.bold = True
        r.italic = True
        r.font.name = 'Arial'
        r.font.size = Pt(12)
        r.font.color.rgb = RGBColor(51, 65, 85)
        return p

    def p_quote(text, author=None):
        p = doc.add_paragraph()
        p.alignment = WD_ALIGN_PARAGRAPH.JUSTIFY
        p.paragraph_format.line_spacing = 1.0
        p.paragraph_format.left_indent = Cm(4.0)
        p.paragraph_format.space_before = Pt(6)
        p.paragraph_format.space_after = Pt(6)
        r = p.add_run(f'"{text}"')
        r.italic = True
        r.font.name = 'Arial'
        r.font.size = Pt(10)
        if author:
            r_auth = p.add_run(f" ({author})")
            r_auth.font.name = 'Arial'
            r_auth.font.size = Pt(10)
            r_auth.bold = True
        return p

    def p_code(code_text):
        p = doc.add_paragraph()
        p.alignment = WD_ALIGN_PARAGRAPH.LEFT
        p.paragraph_format.line_spacing = 1.15
        p.paragraph_format.left_indent = Cm(1.0)
        p.paragraph_format.space_before = Pt(4)
        p.paragraph_format.space_after = Pt(6)
        r = p.add_run(code_text)
        r.font.name = 'Consolas'
        r.font.size = Pt(9.5)
        r.font.color.rgb = RGBColor(30, 41, 59)
        return p

    def p_table_caption(text):
        p = doc.add_paragraph()
        p.alignment = WD_ALIGN_PARAGRAPH.CENTER
        p.paragraph_format.space_before = Pt(8)
        p.paragraph_format.space_after = Pt(3)
        p.paragraph_format.line_spacing = 1.0
        r = p.add_run(text)
        r.bold = True
        r.font.name = 'Arial'
        r.font.size = Pt(10)
        return p

    def p_table_source(text="Fonte: Autores (2026)."):
        p = doc.add_paragraph()
        p.alignment = WD_ALIGN_PARAGRAPH.CENTER
        p.paragraph_format.space_before = Pt(2)
        p.paragraph_format.space_after = Pt(8)
        p.paragraph_format.line_spacing = 1.0
        r = p.add_run(text)
        r.font.name = 'Arial'
        r.font.size = Pt(9.5)
        return p

    def add_table_styled(headers, data, widths=None):
        table = doc.add_table(rows=len(data) + 1, cols=len(headers))
        table.alignment = WD_TABLE_ALIGNMENT.CENTER
        table.autofit = False

        hdr_cells = table.rows[0].cells
        for i, title in enumerate(headers):
            hdr_cells[i].text = title
            set_cell_background(hdr_cells[i], "1E3A8A")
            set_cell_margins(hdr_cells[i], top=100, bottom=100, left=120, right=120)
            p = hdr_cells[i].paragraphs[0]
            p.alignment = WD_ALIGN_PARAGRAPH.CENTER
            p.paragraph_format.space_before = Pt(2)
            p.paragraph_format.space_after = Pt(2)
            p.paragraph_format.line_spacing = 1.0
            for run in p.runs:
                run.font.name = "Arial"
                run.font.size = Pt(9.5)
                run.font.bold = True
                run.font.color.rgb = RGBColor(255, 255, 255)

        for r_idx, row in enumerate(data):
            row_cells = table.rows[r_idx + 1].cells
            bg_color = "F1F5F9" if r_idx % 2 == 1 else "FFFFFF"
            for c_idx, val in enumerate(row):
                row_cells[c_idx].text = str(val)
                set_cell_background(row_cells[c_idx], bg_color)
                set_cell_margins(row_cells[c_idx], top=80, bottom=80, left=120, right=120)
                p = row_cells[c_idx].paragraphs[0]
                p.paragraph_format.space_before = Pt(2)
                p.paragraph_format.space_after = Pt(2)
                p.paragraph_format.line_spacing = 1.0
                if c_idx == 0 or len(str(val)) < 15:
                    p.alignment = WD_ALIGN_PARAGRAPH.CENTER
                else:
                    p.alignment = WD_ALIGN_PARAGRAPH.LEFT
                for run in p.runs:
                    run.font.name = "Arial"
                    run.font.size = Pt(9)
                    run.font.color.rgb = RGBColor(30, 41, 59)

        if widths:
            for r in table.rows:
                for idx, w in enumerate(widths):
                    r.cells[idx].width = w

        tblPr = table._tbl.tblPr
        tblBorders = parse_xml(
            f'<w:tblBorders {nsdecls("w")}>'
            f'<w:top w:val="single" w:sz="4" w:space="0" w:color="CBD5E1"/>'
            f'<w:bottom w:val="single" w:sz="6" w:space="0" w:color="1E3A8A"/>'
            f'<w:left w:val="none"/>'
            f'<w:right w:val="none"/>'
            f'<w:insideH w:val="single" w:sz="4" w:space="0" w:color="E2E8F0"/>'
            f'<w:insideV w:val="none"/>'
            f'</w:tblBorders>'
        )
        tblPr.append(tblBorders)

        p_after = doc.add_paragraph()
        p_after.paragraph_format.space_before = Pt(0)
        p_after.paragraph_format.space_after = Pt(2)
        p_after.paragraph_format.line_spacing = 1.0

    print("[*] Iniciando a escrita dos Elementos Pré-Textuais...")

    # =========================================================================
    # ELEMENTOS PRÉ-TEXTUAIS: CAPA
    # =========================================================================
    p_inst = doc.add_paragraph()
    p_inst.alignment = WD_ALIGN_PARAGRAPH.CENTER
    p_inst.paragraph_format.space_before = Pt(0)
    p_inst.paragraph_format.space_after = Pt(4)
    r = p_inst.add_run("UNIVERSIDADE PAULISTA – UNIP\nINSTITUTO DE CIÊNCIAS EXATAS E TECNOLOGIA\nCURSO SUPERIOR DE TECNOLOGIA EM ANÁLISE E DESENVOLVIMENTO DE SISTEMAS")
    r.bold = True
    r.font.name = 'Arial'
    r.font.size = Pt(12)

    for _ in range(5):
        p_sp = doc.add_paragraph()
        p_sp.paragraph_format.space_before = Pt(0)
        p_sp.paragraph_format.space_after = Pt(0)

    p_autores = doc.add_paragraph()
    p_autores.alignment = WD_ALIGN_PARAGRAPH.CENTER
    p_autores.paragraph_format.space_after = Pt(0)
    r_aut = p_autores.add_run("CAUÊ\nGABRIEL\nKENAI\nMORGAN\nSAULLO\nVITOR HUGO GODOY AMARAL\nVITTOR SALVADOR DE OLIVEIRA")
    r_aut.bold = True
    r_aut.font.name = 'Arial'
    r_aut.font.size = Pt(12)

    for _ in range(6):
        p_sp = doc.add_paragraph()
        p_sp.paragraph_format.space_before = Pt(0)
        p_sp.paragraph_format.space_after = Pt(0)

    p_titulo = doc.add_paragraph()
    p_titulo.alignment = WD_ALIGN_PARAGRAPH.CENTER
    p_titulo.paragraph_format.space_after = Pt(6)
    r_tit = p_titulo.add_run("SOLUÇÃO COMPUTACIONAL INTEGRADA DE E-COMMERCE DE ALTA PERFORMANCE:\nAUTOMAÇÃO TRANSACIONAL EM C, BANCO DE DADOS RELACIONAL, REDES SEGURAS E CONFORMIDADE LGPD/ACESSIBILIDADE")
    r_tit.bold = True
    r_tit.font.name = 'Arial'
    r_tit.font.size = Pt(14)

    p_sub = doc.add_paragraph()
    p_sub.alignment = WD_ALIGN_PARAGRAPH.CENTER
    p_sub.paragraph_format.space_after = Pt(0)
    r_sub = p_sub.add_run("Projeto Integrado Multidisciplinar II (PIM II) – CyberHardware Ltda.")
    r_sub.italic = True
    r_sub.font.name = 'Arial'
    r_sub.font.size = Pt(12)

    for _ in range(8):
        p_sp = doc.add_paragraph()
        p_sp.paragraph_format.space_before = Pt(0)
        p_sp.paragraph_format.space_after = Pt(0)

    p_local = doc.add_paragraph()
    p_local.alignment = WD_ALIGN_PARAGRAPH.CENTER
    p_local.paragraph_format.space_after = Pt(0)
    r_loc = p_local.add_run("MIRASSOL – SP\n2026")
    r_loc.bold = True
    r_loc.font.name = 'Arial'
    r_loc.font.size = Pt(12)

    # =========================================================================
    # FOLHA DE ROSTO
    # =========================================================================
    doc.add_page_break()

    p_autores2 = doc.add_paragraph()
    p_autores2.alignment = WD_ALIGN_PARAGRAPH.CENTER
    p_autores2.paragraph_format.space_after = Pt(0)
    r_aut2 = p_autores2.add_run("CAUÊ\nGABRIEL\nKENAI\nMORGAN\nSAULLO\nVITOR HUGO GODOY AMARAL\nVITTOR SALVADOR DE OLIVEIRA")
    r_aut2.bold = True
    r_aut2.font.name = 'Arial'
    r_aut2.font.size = Pt(12)

    for _ in range(6):
        p_sp = doc.add_paragraph()
        p_sp.paragraph_format.space_before = Pt(0)
        p_sp.paragraph_format.space_after = Pt(0)

    p_titulo2 = doc.add_paragraph()
    p_titulo2.alignment = WD_ALIGN_PARAGRAPH.CENTER
    p_titulo2.paragraph_format.space_after = Pt(0)
    r_tit2 = p_titulo2.add_run("SOLUÇÃO COMPUTACIONAL INTEGRADA DE E-COMMERCE DE ALTA PERFORMANCE:\nAUTOMAÇÃO TRANSACIONAL EM C, BANCO DE DADOS RELACIONAL, REDES SEGURAS E CONFORMIDADE LGPD/ACESSIBILIDADE")
    r_tit2.bold = True
    r_tit2.font.name = 'Arial'
    r_tit2.font.size = Pt(13)

    for _ in range(4):
        p_sp = doc.add_paragraph()
        p_sp.paragraph_format.space_before = Pt(0)
        p_sp.paragraph_format.space_after = Pt(0)

    # Nota de Submissão ABNT (recuo de 8 cm à esquerda)
    p_nota = doc.add_paragraph()
    p_nota.alignment = WD_ALIGN_PARAGRAPH.JUSTIFY
    p_nota.paragraph_format.line_spacing = 1.0
    p_nota.paragraph_format.left_indent = Cm(8.0)
    p_nota.paragraph_format.space_after = Pt(0)
    r_nota = p_nota.add_run("Projeto Integrado Multidisciplinar II (PIM II) apresentado ao Curso Superior de Tecnologia em Análise e Desenvolvimento de Sistemas da Universidade Paulista – UNIP, como requisito parcial para obtenção do título de Tecnólogo em Análise e Desenvolvimento de Sistemas.\n\nÁreas Integradas: Metodologia Científica, Engenharia de Software, Banco de Dados, Programação Estruturada em C, Redes de Computadores e Direitos Humanos/Segurança.\n\nOrientador: Corpo Docente UNIP.")
    r_nota.font.name = 'Arial'
    r_nota.font.size = Pt(10)

    for _ in range(6):
        p_sp = doc.add_paragraph()
        p_sp.paragraph_format.space_before = Pt(0)
        p_sp.paragraph_format.space_after = Pt(0)

    p_local2 = doc.add_paragraph()
    p_local2.alignment = WD_ALIGN_PARAGRAPH.CENTER
    p_local2.paragraph_format.space_after = Pt(0)
    r_loc2 = p_local2.add_run("MIRASSOL – SP\n2026")
    r_loc2.bold = True
    r_loc2.font.name = 'Arial'
    r_loc2.font.size = Pt(12)

    # =========================================================================
    # RESUMO
    # =========================================================================
    doc.add_page_break()
    p_res_title = doc.add_paragraph()
    p_res_title.alignment = WD_ALIGN_PARAGRAPH.CENTER
    p_res_title.paragraph_format.space_after = Pt(18)
    r_rt = p_res_title.add_run("RESUMO")
    r_rt.bold = True
    r_rt.font.name = 'Arial'
    r_rt.font.size = Pt(14)

    p_normal("O presente Projeto Integrado Multidisciplinar II (PIM II) tem por finalidade planejar, modelar e implementar uma solução computacional integrada de comércio eletrônico para a organização fictícia CyberHardware Comércio e Soluções em Tecnologia Ltda. (nome comercial CyberStore), empresa de pequeno porte especializada no varejo de hardwares de alta performance e periféricos. A operação enfrentava sérios gargalos operacionais decorrentes de controles manuais e descentralizados: ruptura invisível de estoque (venda de itens esgotados com consequentes cancelamentos e estornos financeiros), lentidão severa na finalização de pedidos em períodos promocionais, falta de integridade referencial em banco de dados, vulnerabilidades de privacidade perante a Lei Geral de Proteção de Dados (LGPD - Lei nº 13.709/2018) e ausência de mecanismos de acessibilidade digital para pessoas com deficiência. Para solucionar esses desafios, o projeto articulou as seis disciplinas do semestre sob uma abordagem ágil (Scrum/Kanban). Foi projetado e implementado um motor transacional nativo em Linguagem C estruturada utilizando a biblioteca Windows Sockets (winsock2.h) na porta 8080, provendo algoritmo de baixa atômica de estoque em duas fases que extingue o falso saldo físico. A solução compreende uma interface web Single Page Application (SPA) em conformidade com as diretrizes da WCAG 2.2 e da ABNT NBR 17225:2025, modelagem relacional normalizada na 3ª Forma Normal com cinco entidades fundamentais (Categoria, Produto, Cliente, Pedido e Item_Pedido), topologia de rede corporativa segmentada em 3 VLANs com políticas rígidas de firewall Default Deny e arquitetura de nuvem escalável na AWS. Os resultados confirmam a eficácia da integração multidisciplinar, entregando uma plataforma de alto desempenho transacional (latência inferior a 50 milissegundos), segura, auditável e socialmente responsável.")

    p_kw = doc.add_paragraph()
    p_kw.alignment = WD_ALIGN_PARAGRAPH.JUSTIFY
    p_kw.paragraph_format.line_spacing = 1.5
    p_kw.paragraph_format.space_before = Pt(12)
    r_kw_bold = p_kw.add_run("Palavras-chave: ")
    r_kw_bold.bold = True
    r_kw_bold.font.name = 'Arial'
    r_kw_bold.font.size = Pt(12)
    r_kw = p_kw.add_run("Comércio Eletrônico. Linguagem C Estruturada. Baixa Atômica de Estoque. Banco de Dados Relacional. Segurança e LGPD. Acessibilidade Digital.")
    r_kw.font.name = 'Arial'
    r_kw.font.size = Pt(12)

    # =========================================================================
    # ABSTRACT
    # =========================================================================
    doc.add_page_break()
    p_abs_title = doc.add_paragraph()
    p_abs_title.alignment = WD_ALIGN_PARAGRAPH.CENTER
    p_abs_title.paragraph_format.space_after = Pt(18)
    r_at = p_abs_title.add_run("ABSTRACT")
    r_at.bold = True
    r_at.font.name = 'Arial'
    r_at.font.size = Pt(14)

    p_normal("This Multidisciplinary Integrated Project II (PIM II) aims to plan, design, and implement an integrated computational e-commerce solution for the fictitious organization CyberHardware Comércio e Soluções em Tecnologia Ltda. (commercial brand CyberStore), a small business enterprise specialized in high-performance computer hardware and peripherals. The operation suffered from critical operational bottlenecks resulting from manual and decentralized controls: invisible stockouts (sale of out-of-stock items resulting in order cancellations and chargeback fees), transaction latency during promotional sales peaks, lack of referential data integrity, data privacy vulnerabilities under the Brazilian General Data Protection Law (LGPD - Law 13,709/2018), and lack of digital accessibility mechanisms for persons with disabilities. To solve these challenges, the project orchestrated the semester's six multidisciplinary disciplines under an agile hybrid framework (Scrum/Kanban). A native transactional core was developed in structured ANSI/ISO C using the Windows Sockets API (winsock2.h) on TCP port 8080, featuring a two-phase atomic inventory deduction algorithm that eliminates phantom inventory discrepancies. The solution incorporates a responsive Single Page Application (SPA) web frontend compliant with WCAG 2.2 and ABNT NBR 17225:2025 accessibility guidelines, a normalized relational database in the 3rd Normal Form (3NF) comprising five core entities (Category, Product, Customer, Order, and Order_Item), a corporate network topology segmented into 3 VLANs with Default Deny firewall policies, and a scalable cloud infrastructure on AWS. Results confirm the efficacy of multidisciplinary integration, delivering a high-performance transactional platform (sub-50ms latency), resilient, auditable, and socially responsible.", italic=True)

    p_kw2 = doc.add_paragraph()
    p_kw2.alignment = WD_ALIGN_PARAGRAPH.JUSTIFY
    p_kw2.paragraph_format.line_spacing = 1.5
    p_kw2.paragraph_format.space_before = Pt(12)
    r_kw2_bold = p_kw2.add_run("Keywords: ")
    r_kw2_bold.bold = True
    r_kw2_bold.font.name = 'Arial'
    r_kw2_bold.font.size = Pt(12)
    r_kw2 = p_kw2.add_run("E-commerce. Structured C Programming. Atomic Inventory Deduction. Relational Database. Information Security and LGPD. Digital Accessibility.")
    r_kw2.font.name = 'Arial'
    r_kw2.font.size = Pt(12)

    # =========================================================================
    # LISTA DE ILUSTRAÇÕES
    # =========================================================================
    doc.add_page_break()
    add_h1("LISTA DE ILUSTRAÇÕES", page_break=False)
    ilustracoes = [
        ("Figura 1 – Diagrama de Casos de Uso (UML) da Plataforma CyberStore", "Capítulo 4"),
        ("Figura 2 – Diagrama de Sequência (UML) do Fluxo Crítico de Compra e Exclusão Mútua", "Capítulo 4"),
        ("Figura 3 – Diagrama Entidade-Relacionamento (DER) do Sistema CyberHardware", "Capítulo 5"),
        ("Figura 4 – Arquitetura Modular e Fluxo de Execução do Sistema em C", "Capítulo 6"),
        ("Figura 5 – Fluxograma do Algoritmo de Baixa Atômica de Estoque em Duas Fases", "Capítulo 6"),
        ("Figura 6 – Diagrama de Topologia de Rede Corporativa Segmentada e Nuvem AWS", "Capítulo 7"),
        ("Figura 7 – Arquitetura Integrada Multidisciplinar de Ponta a Ponta", "Capítulo 9"),
    ]
    for fig, cap in ilustracoes:
        p = doc.add_paragraph()
        p.paragraph_format.line_spacing = 1.5
        p.paragraph_format.space_after = Pt(4)
        p.add_run(fig).font.name = 'Arial'
        r_dots = p.add_run(" " + "." * (80 - len(fig)) + " ")
        r_dots.font.name = 'Arial'
        r_dots.font.color.rgb = RGBColor(140, 140, 140)
        p.add_run(cap).bold = True

    # =========================================================================
    # LISTA DE TABELAS
    # =========================================================================
    doc.add_page_break()
    add_h1("LISTA DE TABELAS", page_break=False)
    tabelas = [
        ("Tabela 1 – Os 7 Gargalos Críticos de Conversão e Soluções no PIM II", "Capítulo 3"),
        ("Tabela 2 – Síntese do Mapeamento Bibliográfico e Correlação Multidisciplinar", "Capítulo 3"),
        ("Tabela 3 – Matriz de Requisitos Funcionais do Sistema (RF01 a RF06)", "Capítulo 4"),
        ("Tabela 4 – Matriz de Requisitos Não Funcionais do Sistema (RNF01 a RNF06)", "Capítulo 4"),
        ("Tabela 5 – Product Backlog Ágil Priorizado via MoSCoW e Story Points", "Capítulo 4"),
        ("Tabela 6 – Resumo Técnico das Funções Implementadas em Linguagem C", "Capítulo 6"),
        ("Tabela 7 – Tabela de Endereçamento IP e Segmentação em VLANs", "Capítulo 7"),
        ("Tabela 8 – Políticas de Filtragem de Pacotes do Firewall de Borda", "Capítulo 7"),
        ("Tabela 9 – Critérios Normativos de Acessibilidade Aplicados no E-commerce", "Capítulo 8"),
        ("Tabela 10 – Análise Comparativa: Cenário Anterior vs. Solução Implantada", "Capítulo 9"),
    ]
    for tab, cap in tabelas:
        p = doc.add_paragraph()
        p.paragraph_format.line_spacing = 1.5
        p.paragraph_format.space_after = Pt(4)
        p.add_run(tab).font.name = 'Arial'
        r_dots = p.add_run(" " + "." * (80 - len(tab)) + " ")
        r_dots.font.name = 'Arial'
        r_dots.font.color.rgb = RGBColor(140, 140, 140)
        p.add_run(cap).bold = True

    # =========================================================================
    # LISTA DE SIGLAS
    # =========================================================================
    doc.add_page_break()
    add_h1("LISTA DE ABREVIATURAS E SIGLAS", page_break=False)
    siglas = [
        ("ABIACOM", "Associação Brasileira de Inteligência Artificial e E-commerce"),
        ("ABNT", "Associação Brasileira de Normas Técnicas"),
        ("ACID", "Atomicidade, Consistência, Isolamento e Durabilidade"),
        ("ALB", "Application Load Balancer"),
        ("ANSI", "American National Standards Institute"),
        ("ARIA", "Accessible Rich Internet Applications"),
        ("AWS", "Amazon Web Services"),
        ("CDC", "Código de Defesa do Consumidor (Lei nº 8.078/1990)"),
        ("CID", "Confidencialidade, Integridade e Disponibilidade"),
        ("DDL", "Data Definition Language"),
        ("DML", "Data Manipulation Language"),
        ("DMZ", "Demilitarized Zone (Zona Desmilitarizada)"),
        ("DoD", "Definition of Done (Definição de Concluído)"),
        ("EPP", "Empresa de Pequeno Porte"),
        ("ES", "Estoque de Segurança"),
        ("FK", "Foreign Key (Chave Estrangeira)"),
        ("HTML", "HyperText Markup Language"),
        ("HTTP", "HyperText Transfer Protocol"),
        ("HTTPS", "HyperText Transfer Protocol Secure"),
        ("ISO", "International Organization for Standardization"),
        ("JSON", "JavaScript Object Notation"),
        ("LBI", "Lei Brasileira de Inclusão da Pessoa com Deficiência (Lei nº 13.146/2015)"),
        ("LGPD", "Lei Geral de Proteção de Dados Pessoais (Lei nº 13.709/2018)"),
        ("MFA", "Multi-Factor Authentication (Autenticação Multifator)"),
        ("MoSCoW", "Must have, Should have, Could have, Won't have"),
        ("MVC", "Model-View-Controller"),
        ("NBR", "Norma Brasileira Regulamentadora"),
        ("NVDA", "NonVisual Desktop Access"),
        ("OSI", "Open Systems Interconnection"),
        ("PIM", "Projeto Integrado Multidisciplinar"),
        ("PIX", "Pagamento Instantâneo Brasileiro (Banco Central)"),
        ("PK", "Primary Key (Chave Primária)"),
        ("PP", "Ponto de Pedido"),
        ("REST", "Representational State Transfer"),
        ("SGBD", "Sistema de Gerenciamento de Banco de Dados"),
        ("SPA", "Single Page Application"),
        ("SQL", "Structured Query Language"),
        ("TCP/IP", "Transmission Control Protocol / Internet Protocol"),
        ("TLS", "Transport Layer Security"),
        ("UML", "Unified Modeling Language"),
        ("UNIP", "Universidade Paulista"),
        ("VLAN", "Virtual Local Area Network"),
        ("WCAG", "Web Content Accessibility Guidelines"),
    ]
    for sigla, desc in siglas:
        p = doc.add_paragraph()
        p.paragraph_format.line_spacing = 1.3
        p.paragraph_format.space_after = Pt(2)
        r_s = p.add_run(sigla)
        r_s.bold = True
        r_s.font.name = 'Arial'
        r_d = p.add_run(f" – {desc}")
        r_d.font.name = 'Arial'

    # =========================================================================
    # SUMÁRIO
    # =========================================================================
    doc.add_page_break()
    add_h1("SUMÁRIO", page_break=False)
    sumario_itens = [
        ("1 INTRODUÇÃO", "1"),
        ("  1.1 Contextualização e Justificativa do Trabalho", "1"),
        ("  1.2 Definição do Problema e Pergunta Norteadora", "2"),
        ("  1.3 Objetivos da Pesquisa e do Projeto de Software", "2"),
        ("  1.4 Metodologia Científica e Procedimentos Técnicos", "3"),
        ("  1.5 Estrutura Organizacional da Monografia", "3"),
        ("2 CARACTERIZAÇÃO DA ORGANIZAÇÃO (ETAPA 1)", "4"),
        ("  2.1 Perfil Institucional da CyberHardware Ltda.", "4"),
        ("  2.2 Missão, Visão e Valores Organizacionais", "4"),
        ("  2.3 Portfólio de Hardwares e Segmento de Atuação", "5"),
        ("  2.4 Diagnóstico dos Gargalos Operacionais e Justificativa da Solução", "5"),
        ("3 FUNDAMENTAÇÃO CIENTÍFICA E MODELAGEM DE ESTOQUE (ETAPA 2)", "7"),
        ("  3.1 Panorama Contemporâneo do E-commerce Brasileiro (2024–2026)", "7"),
        ("  3.2 A Ruptura de Estoque: Conceituação e Origem Interna", "8"),
        ("  3.3 Modelagem Matemática: Estoque de Segurança e Ponto de Pedido", "9"),
        ("  3.4 Latência Computacional, Concorrência e os 7 Gargalos Críticos", "10"),
        ("4 ENGENHARIA DE SOFTWARE ÁGIL E MODELAGEM DO SISTEMA (ETAPA 3)", "13"),
        ("  4.1 Framework Ágil Híbrido: Scrum e Kanban", "13"),
        ("  4.2 Levantamento e Especificação Formal de Requisitos (RF e RNF)", "14"),
        ("  4.3 Product Backlog MoSCoW e Estimativas via Planning Poker", "16"),
        ("  4.4 Arquitetura do Software: Padrão MVC e Modelo Cliente-Servidor REST", "17"),
        ("  4.5 Modelagem Visual com UML: Casos de Uso e Diagrama de Sequência", "18"),
        ("5 MODELAGEM E IMPLEMENTAÇÃO DA BASE DE DADOS (ETAPA 4)", "21"),
        ("  5.1 Modelagem Conceitual: Entidades, Atributos e Regras Estruturais", "21"),
        ("  5.2 Diagrama Entidade-Relacionamento (DER)", "22"),
        ("  5.3 Normalização de Dados: Primeira, Segunda e Terceira Formas Normais", "22"),
        ("  5.4 Modelo Físico: Scripts SQL (DDL e Carga DML de Testes)", "24"),
        ("  5.5 Estudo Comparativo: Núcleo Relacional ACID vs. NoSQL Complementar", "26"),
        ("6 PROGRAMAÇÃO ESTRUTURADA E MOTOR TRANSACIONAL EM C (ETAPA 5)", "28"),
        ("  6.1 Visão Geral e Contextualização do Backend Nativo em C", "28"),
        ("  6.2 Modelagem das Estruturas de Dados em Memória (struct)", "29"),
        ("  6.3 Modularização Funcional e Arquitetura de Funções", "30"),
        ("  6.4 Algoritmo Crítico: Baixa Atômica de Estoque em Duas Fases", "31"),
        ("  6.5 Persistência de Dados em Disco (FILE*) e Auditoria", "33"),
        ("  6.6 Servidor HTTP Nativo (winsock2.h) e Interface Web SPA Corporativa", "34"),
        ("7 INFRAESTRUTURA DE REDES, SEGURANÇA E NUVEM (ETAPA 6)", "36"),
        ("  7.1 Arquitetura Cliente-Servidor e Pilha de Protocolos TCP/IP", "36"),
        ("  7.2 Topologia Corporativa e Segmentação em VLANs", "37"),
        ("  7.3 Políticas de Firewall de Borda e Filtragem Default Deny", "38"),
        ("  7.4 Criptografia em Trânsito (HTTPS/TLS) e Blindagem LGPD", "39"),
        ("  7.5 Computação em Nuvem Escalável e Alta Disponibilidade na AWS", "40"),
        ("8 DIREITOS HUMANOS, ACESSIBILIDADE E LGPD (ETAPA 7)", "42"),
        ("  8.1 Acessibilidade Digital e Inclusão Universal (ABNT NBR 17225 / WCAG)", "42"),
        ("  8.2 Privacidade e Proteção de Dados Pessoais (LGPD - Lei nº 13.709/2018)", "44"),
        ("  8.3 Segurança da Informação Preservando a Tríade CID", "45"),
        ("  8.4 Ética Comercial, Código de Defesa do Consumidor e Impactos Ambientais", "46"),
        ("9 INTEGRAÇÃO DA SOLUÇÃO E CONSIDERAÇÕES FINAIS (ETAPA 8)", "48"),
        ("  9.1 Arquitetura Integrada da Solução Multidisciplinar", "48"),
        ("  9.2 Análise Comparativa: Cenário Anterior vs. Solução Implantada", "49"),
        ("  9.3 Benefícios Operacionais, Tecnológicos e Sociais Alcançados", "50"),
        ("  9.4 Limitações do Protótipo e Perspectivas de Evolução Futura", "51"),
        ("  9.5 Considerações Finais e Resposta à Pergunta de Pesquisa", "52"),
        ("REFERÊNCIAS BIBLIOGRÁFICAS", "54"),
        ("APÊNDICE A – ESTRUTURA DO CÓDIGO-FONTE EM C (sistema_loja.c)", "57"),
        ("APÊNDICE B – SCRIPT DE CRIAÇÃO DO BANCO DE DADOS (DDL/DML SQL)", "60"),
    ]
    for item, pag in sumario_itens:
        p = doc.add_paragraph()
        p.paragraph_format.line_spacing = 1.3
        p.paragraph_format.space_after = Pt(2)
        r_item = p.add_run(item)
        r_item.font.name = 'Arial'
        if not item.startswith("  "):
            r_item.bold = True
        r_dots = p.add_run(" " + "." * max(2, (75 - len(item))) + " ")
        r_dots.font.name = 'Arial'
        r_dots.font.color.rgb = RGBColor(160, 160, 160)
        r_p = p.add_run(pag)
        r_p.font.name = 'Arial'
        if not item.startswith("  "):
            r_p.bold = True

    print("[*] Iniciando a escrita dos Capítulos Textuais (1 a 9)...")

    # =========================================================================
    # CAPÍTULO 1: INTRODUÇÃO
    # =========================================================================
    add_h1("1 INTRODUÇÃO")
    add_h2("1.1 Contextualização e Justificativa do Trabalho")
    p_normal("O avanço acelerado da tecnologia da informação e a transformação digital dos hábitos de consumo consolidaram o comércio eletrônico (e-commerce) como o principal canal de distribuição e faturamento para o varejo de bens duráveis e de informática no cenário global e nacional. Especialmente no segmento de componentes eletrônicos, peças de hardware de alto desempenho e periféricos voltados para computação gráfica, esportes eletrônicos (e-Sports) e ambientes de desenvolvimento, os consumidores exigem plataformas com alta responsividade, precisão rigorosa em saldos de estoque e total segurança nas transações financeiras.")
    p_normal("Entretanto, a operação prática de uma loja virtual impõe desafios computacionais complexos. O processamento simultâneo de requisições de compra por dezenas ou centenas de clientes em momentos de promoções concentradas sobrecarrega servidores convencionais, ocasionando lentidão severa, esgotamento de conexões e, criticamente, a chamada ruptura invisível de estoque. Esse fenômeno ocorre quando o sistema confirma e debita um pedido cujo produto físico não mais existe no centro de distribuição, gerando prejuízos financeiros com estornos bancários e quebra irreversível da confiança do cliente.")
    p_normal("A relevância deste projeto reside na proposição de uma arquitetura inovadora para pequenas e médias empresas: demonstrar que é viável atingir altíssimo desempenho transacional e baixíssima latência computacional sem a obrigatoriedade de contratar infraestruturas de nuvem hiperdimensionadas e onerosas. A adoção de um núcleo de processamento transacional compilado em Linguagem C nativa, integrada a uma base de dados relacional normalizada, uma interface moderna acessível e uma rede devidamente segmentada, viabiliza uma operação robusta, escalável, financeiramente sustentável e em total conformidade legal.")

    add_h2("1.2 Definição do Problema e Pergunta Norteadora")
    p_normal("A organização objeto deste estudo empírico — a CyberHardware Comércio e Soluções em Tecnologia Ltda. — operava com controles manuais de inventário e processos descentralizados de venda. Essa fragilidade tecnológica acarretava quatro gargalos severos: cancelamento sistemático de compras por inconsistência de estoque, perda massiva de conversão durante picos de tráfego, vulnerabilidade legal decorrente do armazenamento desprotegido de dados cadastrais e exclusão digital de usuários com necessidades especiais.")
    p_normal("Diante dessa problemática, formulou-se a seguinte pergunta científica norteadora que orientou todas as etapas do presente trabalho:")
    p_quote("Como desenvolver uma solução computacional integrada de e-commerce que concilie alto desempenho transacional e baixa latência no controle de estoque com a eliminação definitiva da ruptura invisível, assegurando integridade transacional e conformidade aos padrões de segurança, privacidade e acessibilidade universal?", "Grupo de Pesquisa PIM II, 2026")

    add_h2("1.3 Objetivos da Pesquisa e do Projeto de Software")
    p_normal("Para responder à pergunta formulada, definiram-se os seguintes objetivos acadêmicos e tecnológicos:")
    p_normal("Projetar, modelar, implementar e validar uma solução computacional integrada para automação e gestão transacional de um comércio eletrônico de hardware e periféricos, garantindo processamento concorrente de alta velocidade, integridade absoluta de inventário e alinhamento com as normas vigentes de redes, segurança e direitos humanos.", bold_prefix="Objetivo Geral: ")
    p_normal("Como desdobramentos operacionais, estabeleceram-se os seguintes:", bold_prefix="Objetivos Específicos: ")
    p_bullet("Mapear o cenário contemporâneo do e-commerce brasileiro (2024–2026) e quantificar as perdas provocadas por ruptura e latência, aplicando formulações matemáticas de Estoque de Segurança (ES) e Ponto de Pedido (PP).", bold_prefix="a) ")
    p_bullet("Especificar formalmente os requisitos funcionais e não funcionais por meio de metodologia ágil (Scrum/Kanban) e modelagem visual em Unified Modeling Language (UML).", bold_prefix="b) ")
    p_bullet("Modelar e normalizar a base de dados relacional até a Terceira Forma Normal (3FN), garantindo propriedades ACID e definindo estratégias complementares de persistência NoSQL.", bold_prefix="c) ")
    p_bullet("Desenvolver um motor transacional de alta velocidade em Linguagem C estruturada (ANSI/ISO) com servidor HTTP nativo sobre Windows Sockets (winsock2.h) e algoritmo de baixa atômica em duas fases.", bold_prefix="d) ")
    p_bullet("Projetar a infraestrutura de redes corporativa com segmentação em 3 VLANs, firewall com filtragem Default Deny e arquitetura elástica de nuvem pública (AWS).", bold_prefix="e) ")
    p_bullet("Auditar a interface web e os fluxos de dados sob os preceitos da Lei Geral de Proteção de Dados (LGPD) e das diretrizes de acessibilidade da WCAG 2.2 e ABNT NBR 17225:2025.", bold_prefix="f) ")

    add_h2("1.4 Metodologia Científica e Procedimentos Técnicos")
    p_normal("A metodologia adotada fundamenta-se nos preceitos de Gil (2019) e Vergara (2016), estruturando-se em quatro eixos classificatórios:")
    p_bullet("Quanto à Natureza: Pesquisa Aplicada, pois objetiva a resolução de problemas reais de engenharia de software e logística operacional vivenciados no comércio varejista.", bold_prefix="• ")
    p_bullet("Quanto à Abordagem: Mista (Quali-Quantitativa). Quantitativa no levantamento de dados estatísticos de latência, conversão setorial e cálculos de estoque; qualitativa na análise de arquiteturas distribuídas, modelagem de dados e requisitos legais.", bold_prefix="• ")
    p_bullet("Quanto aos Objetivos: Exploratória e Descritiva, proporcionando familiaridade com a sincronização de concorrência e descrevendo minuciosamente os componentes do sistema.", bold_prefix="• ")
    p_bullet("Quanto aos Procedimentos: Pesquisa Bibliográfica em bases científicas (SciELO, IEEE, Google Acadêmico) associada ao Estudo de Caso Experimental com desenvolvimento e testes de protótipo de software executável.", bold_prefix="• ")

    add_h2("1.5 Estrutura Organizacional da Monografia")
    p_normal("O documento está estruturado em nove capítulos interligados: o Capítulo 2 contextualiza a organização CyberHardware; o Capítulo 3 traz a fundamentação científica e os cálculos de estoque; o Capítulo 4 formaliza a Engenharia de Software ágil e UML; o Capítulo 5 detalha o banco de dados e normalização; o Capítulo 6 aprofunda o motor transacional em C; o Capítulo 7 aborda redes, firewall e nuvem; o Capítulo 8 estabelece a conformidade com LGPD e Acessibilidade; e o Capítulo 9 conclui com a integração geral da solução e trabalhos futuros.")

    # =========================================================================
    # CAPÍTULO 2: CARACTERIZAÇÃO DA ORGANIZAÇÃO
    # =========================================================================
    add_h1("2 CARACTERIZAÇÃO DA ORGANIZAÇÃO (ETAPA 1)")
    add_h2("2.1 Perfil Institucional da CyberHardware Ltda.")
    p_normal("A organização objeto deste projeto acadêmico é a CyberHardware Comércio e Soluções em Tecnologia Ltda., empresa fictícia constituída sob a forma de sociedade limitada, atuando no mercado sob o nome comercial e fantasia CyberStore ou CyberHardware E-commerce.")
    p_bullet("Razão Social: CyberHardware Comércio e Soluções em Tecnologia Ltda.")
    p_bullet("Nome Fantasia: CyberHardware E-commerce (CyberStore)")
    p_bullet("CNPJ Fictício: 45.892.104/0001-78 | Inscrição Estadual: 112.345.678.910")
    p_bullet("Sede Administrativa e Centro de Distribuição: Avenida Paulista, nº 1842, 10º andar, Conjunto 102 – Bairro Bela Vista, São Paulo – SP, CEP: 01310-200.")
    p_bullet("Porte Empresarial: Empresa de Pequeno Porte (EPP).")
    p_bullet("Ramo de Atuação: Varejo eletrônico especializado em hardware de alta performance, estações de trabalho profissionais, computadores para entusiastas e periféricos gamers de precisão.")

    add_h2("2.2 Missão, Visão e Valores Organizacionais")
    p_normal("A atuação estratégica da CyberHardware pauta-se nos seguintes princípios norteadores:")
    p_normal("Fornecer componentes de informática e periféricos de ponta com excelência no atendimento ao cliente, assegurando integridade transacional instantânea, agilidade na separação logística e preços altamente competitivos para o ecossistema de estudantes, programadores e gamers.", bold_prefix="Missão: ")
    p_normal("Consolidar-se como uma das principais referências nacionais em comércio eletrônico de hardware especializado e soluções de computação de alta eficiência até 2028, destacando-se pela solidez tecnológica e respeito absoluto ao consumidor.", bold_prefix="Visão: ")
    p_normal("Transparência absoluta nas relações comerciais, ética na publicidade de preços, segurança e privacidade de dados pessoais (Privacy by Design), inovação contínua de software e compromisso inegociável com a inclusão digital e acessibilidade universal.", bold_prefix="Valores: ")

    add_h2("2.3 Portfólio de Hardwares e Segmento de Atuação")
    p_normal("A CyberHardware comercializa um catálogo selecionado em cinco categorias principais: Placas de Vídeo (GPUs dedicadas de última geração), Processadores (CPUs para jogos e computação multithread), Armazenamento de Alta Velocidade (SSDs NVMe PCIe 4.0), Monitores Gamer com alta taxa de atualização e Periféricos Ergonômicos (teclados mecânicos switch linear e mouses de alta resolução). O público-alvo caracteriza-se por alto nível de exigência técnica, onde a precisão das especificações e a entrega rápida do item exato constituem fatores decisivos de retenção de clientes.")

    add_h2("2.4 Diagnóstico dos Gargalos Operacionais e Justificativa da Solução")
    p_normal("A análise do diagnóstico operacional conduzida na Etapa 1 identificou quatro vulnerabilidades críticas que inviabilizavam a expansão da empresa:")
    p_bullet("Controle Manual e Ruptura Invisível: A contagem de inventário em planilhas causava descompasso entre o estoque virtual da loja e o estoque real do armazém. O cliente efetuava o pagamento via PIX ou cartão, mas a empresa descobria tardiamente que a peça não existia mais na prateleira, obrigando cancelamentos unilaterais e gerando multas perante os órgãos de defesa do consumidor.", bold_prefix="1. ")
    p_bullet("Lentidão Transacional em Horários de Pico: A plataforma anterior utilizava interpretadores web pesados que travavam em promoções simultâneas, resultando em tempos de carregamento acima de 6 segundos e elevadas taxas de abandono de carrinho.", bold_prefix="2. ")
    p_bullet("Vulnerabilidade Legal (LGPD): Ausência de políticas formais de minimização de dados e armazenamento inadequado de informações financeiras, gerando passivo jurídico de alto risco.", bold_prefix="3. ")
    p_bullet("Barreiras de Acessibilidade Digital: A antiga interface não dispunha de navegação acessível por teclado, alto contraste ou semântica ARIA, violando a Lei Brasileira de Inclusão.", bold_prefix="4. ")
    p_normal("A justificativa deste projeto repousa na reconstrução da infraestrutura corporativa: criação de um motor transacional nativo em Linguagem C (garantindo velocidade máxima e baixo consumo de memória), banco de dados normalizado na 3FN, regras de firewall com segmentação em VLANs e conformidade estrita aos parâmetros legais e normativos vigentes.")

    # =========================================================================
    # CAPÍTULO 3: FUNDAMENTAÇÃO CIENTÍFICA E MODELAGEM DE ESTOQUE
    # =========================================================================
    add_h1("3 FUNDAMENTAÇÃO CIENTÍFICA E MODELAGEM DE ESTOQUE (ETAPA 2)")
    add_h2("3.1 Panorama Contemporâneo do E-commerce Brasileiro (2024–2026)")
    p_normal("O comércio eletrônico no Brasil expande-se a taxas significativamente superiores à média internacional. Conforme consolidado pelo Online Retail Report da consultoria global FTI Consulting (2025), o mercado digital brasileiro cresceu 11,8% em 2024, atingindo R$ 381 bilhões em faturamento bruto (superando a média mundial de 8,4%) e elevando a representatividade das vendas online no varejo total para 9,03%.")
    p_normal("Esse vetor de aceleração confirmou-se no relatório oficial da Associação Brasileira de Inteligência Artificial e E-commerce (ABIACOM, 2026): no primeiro semestre de 2026, o faturamento atingiu R$ 125 bilhões, representando uma expansão expressiva de 24,37% em relação ao mesmo período do ano anterior. Foram contabilizados 242 milhões de pedidos com ticket médio de R$ 517,00.")
    p_normal("Dentre os elementos transformadores desse ecossistema, destacam-se a hegemonia do PIX — que hoje responde por 40% das compras virtuais no País, rivalizando com o cartão de crédito (44%) — e a predominância do comércio móvel (61% dos acessos originados em smartphones). Essas características impõem às plataformas a exigência de checkouts instantâneos com geração dinâmica de códigos de pagamento e latência desprezível.")

    add_h2("3.2 A Ruptura de Estoque: Conceituação e Origem Interna")
    p_normal("Na literatura logística e de cadeias de suprimentos, a ruptura de estoque (Stockout ou Out-of-Stock - OOS) caracteriza-se pela incapacidade da loja em atender à demanda imediata do comprador. Essa ruptura desdobra-se em duas vertentes operacionais:")
    p_bullet("Ruptura Visível: Ocorre quando a interface exibe abertamente o aviso de 'Esgotado'. O cliente não conclui a compra e, embora represente uma venda perdida no momento, não provoca atrito cadastral ou despesas administrativas pós-venda.", bold_prefix="• ")
    p_bullet("Ruptura Invisível (Falso Saldo): Configura-se quando o sistema exibe o item como disponível, permite o preenchimento de dados e conclui a transação financeira, para somente depois a expedição constatar que a prateleira física está vazia. Trata-se do evento mais danoso para o varejo eletrônico: demanda estorno do dinheiro, gera custos com tarifas bancárias e corrói a reputação institucional da marca.", bold_prefix="• ")
    p_normal("Derrubando o mito comum de que rupturas de estoque decorrem prioritariamente de atrasos dos fabricantes, a pesquisa seminal de Gruen, Corsten e Bharadwaj (2002) e o estudo empírico brasileiro de Aguiar e Sampaio (2021), publicado na conceituada revista acadêmica Produção (SciELO), revelam que entre 70% e 75% dos casos de ruptura derivam de falhas internas da própria empresa. O levantamento de Aguiar e Sampaio sobre 113 estabelecimentos constatou uma taxa média anual de ruptura de 11,5% em itens de alto giro, atingindo até 18% em promoções sazonais desprovidas de softwares transacionais atômicos.")

    add_h2("3.3 Modelagem Matemática: Estoque de Segurança e Ponto de Pedido")
    p_normal("Para mitigar matematicamente a ruptura sem comprometer o fluxo de caixa com excesso de inventário parado, o projeto aplicou a modelagem clássica de gestão da cadeia de suprimentos fundamentada nas variáveis de Venda Máxima Diária (VMD), Venda Média Diária (VMD_med), Prazo Máximo de Reposição (PMR) e Prazo Médio de Reposição (PMR_med):")
    p_quote("ES = (VMD × PMR) – (VMD_méd × PMR_méd)\nPP = (VMD_méd × PMR_méd) + ES", "Formulações Matemáticas de Estoque de Segurança e Ponto de Pedido")
    p_normal("Onde o Estoque de Segurança (ES) amortece oscilações inesperadas de consumo e atrasos de transporte, e o Ponto de Pedido (PP) estabelece o momento exato em que a equipe de compras deve emitir nova ordem de aquisição ao distribuidor. No sistema em C desenvolvido na Etapa 5, essa lógica foi diretamente materializada no campo 'estoque_minimo' da struct Produto, emitindo alertas gerenciais automáticos sempre que o saldo físico atinge o limiar calculado.")

    add_h2("3.4 Latência Computacional, Concorrência e os 7 Gargalos Críticos")
    p_normal("Em grandes datas comemorativas do comércio eletrônico, como a Black Friday, a performance da infraestrutura computacional torna-se o gargalo determinante de sobrevivência. Relatório divulgado pela empresa de monitoramento Sofist em conjunto com a CNN Brasil (2024) registrou que, apenas nas primeiras 12 horas de Black Friday, a lentidão gerou perdas estimadas em R$ 8,1 milhões aos e-commerces brasileiros. Páginas com tempo de carregamento superior a 4,1 segundos provocaram evasão imediata de 53% dos compradores.")
    p_normal("A consultoria de engenharia de software eVision (2026) mapeou os 7 Gargalos Críticos de Infraestrutura que afetam os e-commerces contemporâneos, contrapondo-os diretamente às soluções de engenharia implementadas no projeto da CyberHardware:")

    p_table_caption("Tabela 1 – Os 7 Gargalos Críticos de Conversão e Soluções no PIM II")
    gargalos_data = [
        ("1. Performance", "Páginas lentas decorrentes de linguagens interpretadas pesadas.", "Backend nativo em C compilado (-O2) com respostas REST < 50ms."),
        ("2. Busca Ineficiente", "Catálogo lento ou exibindo produtos esgotados no topo.", "Filtros dinâmicos indexados em memória no C e no frontend SPA."),
        ("3. Estoque Inconsistente", "Descompasso entre saldo na tela e saldo real (ruptura invisível).", "Algoritmo de Baixa Atômica em Duas Fases com exclusão mútua."),
        ("4. Ficha Técnica", "Ausência de especificações técnicas para validação de compra.", "Modal com especificações técnicas completas de hardware integradas."),
        ("5. Fricção no Checkout", "Formulários extensos e múltiplas telas que geram desistência.", "Checkout em modal único (One-Step) com validações em tempo real."),
        ("6. Meios de Pagamento", "Falhas na geração de chaves PIX ou lentidão em parcelamento.", "Suporte integrado a PIX (cópia-e-cola), Cartão de Crédito e Boleto."),
        ("7. Falha de Integração", "Lentidão entre frontend, backend e persistência em disco.", "Arquitetura REST desacoplada sobre Sockets TCP nativos (winsock2.h).")
    ]
    add_table_styled(["Gargalo Mapeado", "Manifestação Operacional Negativa", "Solução Implementada no PIM II"], gargalos_data, [Cm(3.5), Cm(6.0), Cm(6.5)])
    p_table_source("Adaptado de eVision Consulting (2026) e Autores (2026).")

    # =========================================================================
    # CAPÍTULO 4: ENGENHARIA DE SOFTWARE ÁGIL E MODELAGEM UML
    # =========================================================================
    add_h1("4 ENGENHARIA DE SOFTWARE ÁGIL E MODELAGEM DO SISTEMA (ETAPA 3)")
    add_h2("4.1 Framework Ágil Híbrido: Scrum e Kanban")
    p_normal("Para compatibilizar a rigidez dos prazos de entrega acadêmicos com a necessidade de validação constante do código-fonte e dos artefatos de dados, a equipe adotou um framework ágil híbrido. Combinou-se a estrutura iterativa e incremental do Scrum — organizado em Sprints semanais time-boxed — com o controle contínuo de fluxo de valor do Kanban.")
    p_normal("A transparência das atividades deu-se por meio de um quadro visual estruturado nas colunas: Backlog do Produto, Em Análise, Em Desenvolvimento, Em Revisão/Testes e Concluído (Done). As cerimônias ágeis integraram o alinhamento das equipes: Sprint Planning semanal para desdobramento de épicos em tarefas técnicas, reuniões diárias (Daily Scrum de 15 minutos) para destravamento de impedimentos entre as frentes de banco, C e redes, e Sprint Reviews com demonstração dos incrementos funcionais (executável rodando e endpoints respondendo).")

    add_h2("4.2 Levantamento e Especificação Formal de Requisitos (RF e RNF)")
    p_normal("O levantamento de requisitos estabeleceu as fronteiras do sistema, garantindo rastreabilidade com as disciplinas subsequentes:")

    p_table_caption("Tabela 3 – Matriz de Requisitos Funcionais do Sistema (RF01 a RF06)")
    rf_data = [
        ("RF01", "Cadastro e Gestão de Clientes", "Capturar dados do cliente (Nome, CPF, E-mail, Telefone e Endereço) no fechamento da compra.", "Must Have"),
        ("RF02", "Catálogo Dinâmico de Produtos", "Exibir produtos organizados por categoria com preço original, promocional, especificações e saldo.", "Must Have"),
        ("RF03", "Controle Atômico de Estoque", "Validar saldo e executar baixa atômica e automática no ato do faturamento, bloqueando compras excedentes.", "Must Have"),
        ("RF04", "Gestão de Carrinho e Cupons", "Adicionar/remover itens, alterar quantidades e aplicar cupons válidos (TECH10 e CYBER20) com recálculo instantâneo.", "Must Have"),
        ("RF05", "Checkout Multi-pagamento", "Concluir pedidos com opções de PIX (QR Code / Copia-e-Cola), Cartão de Crédito e Boleto Bancário.", "Must Have"),
        ("RF06", "Histórico e Telemetria", "Persistir vendas em disco, gerando código único amigável e disponibilizando dados para auditoria.", "Should Have")
    ]
    add_table_styled(["ID", "Nome do Requisito", "Descrição Detalhada da Funcionalidade", "Prioridade"], rf_data, [Cm(1.5), Cm(4.5), Cm(8.0), Cm(2.5)])
    p_table_source()

    p_table_caption("Tabela 4 – Matriz de Requisitos Não Funcionais do Sistema (RNF01 a RNF06)")
    rnf_data = [
        ("RNF01", "Segurança e LGPD", "Conformidade integral com a Lei nº 13.709/2018; não retenção de dados sensíveis de pagamento em texto puro."),
        ("RNF02", "Desempenho e Latência", "Tempo de resposta das requisições REST ao backend estritamente inferior a 100ms em condições típicas."),
        ("RNF03", "Acessibilidade Digital", "Conformidade com a ABNT NBR 17225:2025 e WCAG 2.2 nível AA (navegação por teclado, ARIA, contraste > 4.5:1)."),
        ("RNF04", "Concorrência e Atomicidade", "Exclusão mútua (CRITICAL_SECTION / Mutex) no backend em C para evitar race conditions no decremento de estoque."),
        ("RNF05", "Arquitetura e Portabilidade", "Arquitetura cliente-servidor desacoplada (REST sobre HTTP/1.1 e JSON), compatível com Chrome, Edge e Firefox."),
        ("RNF06", "Persistência Estruturada", "Durabilidade dos registros em arquivos estruturados e esquema relacional normalizado na 3ª Forma Normal.")
    ]
    add_table_styled(["ID", "Categoria", "Descrição Detalhada da Restrição de Qualidade"], rnf_data, [Cm(1.8), Cm(4.2), Cm(10.0)])
    p_table_source()

    add_h2("4.3 Product Backlog MoSCoW e Estimativas via Planning Poker")
    p_normal("A equipe hierarquizou as demandas por meio da técnica MoSCoW (Must, Should, Could, Won't Have) e estimou o esforço utilizando Planning Poker baseado na sequência de Fibonacci simplificada (1, 2, 3, 5 e 8 story points):")

    p_table_caption("Tabela 5 – Product Backlog Ágil Priorizado via MoSCoW e Story Points")
    backlog_data = [
        ("US01", "Catálogo", "Como cliente, quero ver o catálogo por categoria para achar peças rapidamente.", "Must Have", "5 pts", "Sprint 1"),
        ("US02", "Carrinho", "Como cliente, quero gerenciar itens na sacola com recálculo automático.", "Must Have", "5 pts", "Sprint 1"),
        ("US03", "Estoque C", "Como sistema, quero decrementar o estoque atomicamente para barrar furos.", "Must Have", "8 pts", "Sprint 1"),
        ("US04", "Checkout", "Como cliente, quero pagar via PIX, Cartão ou Boleto com segurança.", "Must Have", "8 pts", "Sprint 2"),
        ("US05", "Cupons", "Como cliente, quero inserir cupons (TECH10/CYBER20) e obter desconto.", "Should Have", "3 pts", "Sprint 2"),
        ("US06", "Histórico", "Como cliente/atendente, quero ver comprovantes e histórico de pedidos.", "Should Have", "5 pts", "Sprint 2"),
        ("US07", "Telemetria", "Como gestor, quero ver métricas de requisições, uptime e faturamento.", "Could Have", "5 pts", "Sprint 3")
    ]
    add_table_styled(["ID", "Épico", "User Story (Como... Quero... Para...)", "Prioridade", "Pontos", "Sprint"], backlog_data, [Cm(1.5), Cm(2.2), Cm(7.3), Cm(2.5), Cm(1.5), Cm(1.8)])
    p_table_source()

    add_h2("4.4 Arquitetura do Software: Padrão MVC e Modelo Cliente-Servidor REST")
    p_normal("A plataforma adota uma Arquitetura Cliente-Servidor Desacoplada baseada no protocolo HTTP/1.1 com chamadas REST assíncronas. Logicamente, o sistema distribui suas responsabilidades segundo os preceitos do consagrado padrão arquitetural MVC (Model-View-Controller):")
    p_bullet("Model (Modelo): Reside no motor em C através de estruturas tipadas (structs Produto, Pedido, Cliente) associadas aos arquivos de persistência e às tabelas SQL normalizadas do Saullo. O Model encapsula todo o estado do inventário e as regras de negócio críticas (como a pré-validação atômica e o congelamento do preço de venda).", bold_prefix="• ")
    p_bullet("View (Visão): Constitui a interface gráfica web SPA executada no navegador do cliente (web/index.html). Desenvolvida em HTML5 semântico e estilização sóbria, é encarregada de renderizar o catálogo, gerenciar os estados visuais da sacola e apresentar formulários acessíveis.", bold_prefix="• ")
    p_bullet("Controller (Controlador): Composto pelo despachante de requisições do servidor HTTP em C (winsock2.h). Atua como intermediário, recebendo requisições HTTP da View, extraindo dados em JSON, acionando as rotinas de validação do Model e retornando o status HTTP (200 OK, 400 Bad Request, etc.) com os dados serializados.", bold_prefix="• ")

    add_h2("4.5 Modelagem Visual com UML: Casos de Uso e Diagrama de Sequência")
    p_normal("A modelagem comportamental e estrutural foi materializada segundo as diretrizes formais da Unified Modeling Language (UML):")
    p_normal("Identificaram-se dois atores principais: o Cliente (responsável pelas operações de consulta, gestão de sacola e conclusão de checkout) e o Administrador (responsável pela supervisão operacional, telemetria técnica e auditoria de estoque). Quatro Casos de Uso estruturam o sistema: UC01 (Consultar Catálogo), UC02 (Gerenciar Carrinho com extensão para Aplicação de Cupons e inclusão de Validação de Estoque), UC03 (Efetuar Checkout com inclusão de Baixa Atômica em C e Gravação em Disco) e UC04 (Monitorar Telemetria e Estoque).", bold_prefix="Casos de Uso (UML Use Case): ")
    p_normal("Mapeia a colaboração temporal no instante mais sensível da compra: o cliente clica em 'Confirmar Pagamento' -> a interface dispara POST /api/checkout -> o servidor em C invoca EnterCriticalSection() -> verifica o saldo de todos os itens na memória -> se houver estoque, efetua o decremento, grava no arquivo pedidos_loja.txt e libera a seção crítica via LeaveCriticalSection() -> responde HTTP 200 ao navegador -> a interface limpa a sacola e exibe o recibo amigável.", bold_prefix="Diagrama de Sequência: ")

    # =========================================================================
    # CAPÍTULO 5: BANCO DE DADOS RELACIONAL E NORMALIZAÇÃO
    # =========================================================================
    add_h1("5 MODELAGEM E IMPLEMENTAÇÃO DA BASE DE DADOS (ETAPA 4)")
    add_h2("5.1 Modelagem Conceitual: Entidades, Atributos e Regras Estruturais")
    p_normal("O projeto de banco de dados para a CyberHardware foi concebido para assegurar integridade referencial absoluta, eliminação de redundâncias, histórico imutável de transações financeiras e estrita conformidade com a LGPD. A modelagem conceitual estruturou o domínio em cinco entidades:")
    p_bullet("CATEGORIA: Classifica os hardwares comercializados. Atributos: id_categoria (PK) e nome_categoria.", bold_prefix="1. ")
    p_bullet("PRODUTO: Armazena as peças físicas do catálogo. Atributos: id_produto (PK), id_categoria (FK), nome_comercial, preco_unitario, quantidade_estoque, estoque_minimo e especificacoes.", bold_prefix="2. ")
    p_bullet("CLIENTE: Gerencia os dados cadastrais mínimos indispensáveis. Atributos: id_cliente (PK), nome_completo, cpf (Unique), email (Unique) e endereco.", bold_prefix="3. ")
    p_bullet("PEDIDO: Cabeçalho da transação comercial. Atributos: id_pedido (PK), id_cliente (FK), codigo_unico (Unique), data_hora, subtotal, desconto, valor_final, forma_pagamento e status.", bold_prefix="4. ")
    p_bullet("ITEM_PEDIDO: Entidade associativa que resolve a cardinalidade N:N entre Pedido e Produto. Atributos: id_pedido (PK, FK), id_produto (PK, FK), quantidade, preco_unitario e subtotal.", bold_prefix="5. ")

    add_h2("5.2 Diagrama Entidade-Relacionamento (DER)")
    p_normal("O Diagrama Entidade-Relacionamento estabelece as seguintes cardinalidades estritas: uma Categoria classifica um ou muitos Produtos (1:N); um Cliente realiza zero ou muitos Pedidos (1:N); um Pedido contém obrigatoriamente um ou muitos Itens_Pedido (1:N); e um Produto compõe zero ou muitos Itens_Pedido (1:N).")

    add_h2("5.3 Normalização de Dados: Primeira, Segunda e Terceira Formas Normais")
    p_normal("Para prevenir anomalias de inserção, exclusão e alteração de dados, a estrutura foi normalizada até a Terceira Forma Normal (3FN):")
    p_bullet("Primeira Forma Normal (1FN): Exige atributos atômicos e ausência de grupos repetidos. A relação original Muitos-para-Muitos (N:N) entre Pedido e Produto violava a 1FN, pois um pedido reúne múltiplos hardwares e o mesmo hardware é vendido repetidas vezes. A criação da tabela associativa Item_Pedido quebrou a relação em duas de Um-para-Muitos (1:N), garantindo valores atômicos por tupla.", bold_prefix="• ")
    p_bullet("Segunda Forma Normal (2FN): Exige a 1FN e determina que todos os atributos não-chave dependam funcionalmente da totalidade da chave primária (especialmente em chaves compostas). Na tabela Item_Pedido, cuja chave primária é composta por (id_pedido, id_produto), os campos quantidade, subtotal e preco_unitario dependem obrigatoriamente da junção dos dois identificadores. Atributos que dependem unicamente do produto (como nome_comercial ou quantidade_estoque) permanecem isolados na tabela Produto.", bold_prefix="• ")
    p_bullet("Terceira Forma Normal (3FN): Exige a 2FN e elimina qualquer dependência transitiva (atributos não-chave dependendo de outros atributos não-chave). Dados cadastrais de clientes residem unicamente na tabela Cliente, migrando apenas o id_cliente como chave estrangeira para Pedido. Adicionalmente, como regra de negócio crítica, o preco_unitario é congelado dentro de Item_Pedido no momento da compra. Essa decisão técnica impede que atualizações cadastrais futuras nos preços do catálogo alterem indevidamente os valores faturados em pedidos do passado, garantindo imutabilidade contábil.", bold_prefix="• ")

    add_h2("5.4 Modelo Físico: Scripts SQL (DDL e Carga DML de Testes)")
    p_normal("A implementação física em MySQL adota tipos de dados rigorosamente dimensionados e restrições de integridade que barram inconsistências diretamente no motor do banco:")
    p_code("""CREATE TABLE Categoria (
    id_categoria INT PRIMARY KEY AUTO_INCREMENT,
    nome_categoria VARCHAR(100) NOT NULL
);

CREATE TABLE Produto (
    id_produto INT PRIMARY KEY AUTO_INCREMENT,
    id_categoria INT NOT NULL,
    nome_comercial VARCHAR(150) NOT NULL,
    preco_unitario DECIMAL(10,2) NOT NULL CHECK (preco_unitario > 0),
    quantidade_estoque INT NOT NULL CHECK (quantidade_estoque >= 0),
    estoque_minimo INT NOT NULL,
    especificacoes TEXT,
    FOREIGN KEY (id_categoria) REFERENCES Categoria(id_categoria)
);

CREATE TABLE Cliente (
    id_cliente INT PRIMARY KEY AUTO_INCREMENT,
    nome_completo VARCHAR(150) NOT NULL,
    cpf VARCHAR(14) NOT NULL UNIQUE,
    email VARCHAR(100) NOT NULL UNIQUE,
    endereco VARCHAR(255) NOT NULL
);

CREATE TABLE Pedido (
    id_pedido INT PRIMARY KEY AUTO_INCREMENT,
    id_cliente INT NOT NULL,
    codigo_unico VARCHAR(20) NOT NULL UNIQUE,
    data_hora DATETIME NOT NULL,
    subtotal DECIMAL(10,2) NOT NULL,
    desconto DECIMAL(10,2) DEFAULT 0,
    valor_final DECIMAL(10,2) NOT NULL,
    forma_pagamento VARCHAR(50) NOT NULL,
    status VARCHAR(50) NOT NULL,
    FOREIGN KEY (id_cliente) REFERENCES Cliente(id_cliente)
);

CREATE TABLE Item_Pedido (
    id_pedido INT NOT NULL,
    id_produto INT NOT NULL,
    quantidade INT NOT NULL CHECK (quantidade > 0),
    preco_unitario DECIMAL(10,2) NOT NULL CHECK (preco_unitario > 0),
    subtotal DECIMAL(10,2) NOT NULL,
    PRIMARY KEY (id_pedido, id_produto),
    FOREIGN KEY (id_pedido) REFERENCES Pedido(id_pedido),
    FOREIGN KEY (id_produto) REFERENCES Produto(id_produto)
);""")

    add_h2("5.5 Estudo Comparativo: Núcleo Relacional ACID vs. NoSQL Complementar")
    p_normal("O projeto comparou criteriosamente os modelos de banco de dados frente aos requisitos da CyberHardware. Concluiu-se que o SGBD relacional (SQL) é mandatório para o núcleo de faturamento e inventário em função das propriedades ACID:")
    p_bullet("Atomicidade: Garante que uma compra de múltiplos hardwares seja gravada integralmente ou cancelada por completo se ocorrer qualquer interrupção, impedindo pagamentos sem baixa de estoque.", bold_prefix="• ")
    p_bullet("Consistência: Assegura que nenhuma regra estrutural (como saldo negativo de estoque ou CPF duplicado) seja violada.", bold_prefix="• ")
    p_bullet("Isolamento: Execuções concorrentes de pedidos simultâneos não interferem no saldo intermediário uma da outra.", bold_prefix="• ")
    p_bullet("Durabilidade: Após confirmada a transação, os dados permanecem salvos em mídia estável mesmo em caso de pane elétrica.", bold_prefix="• ")
    p_normal("De forma complementar e moderna, propõe-se uma arquitetura híbrida de suporte: o Redis (NoSQL chave-valor em memória RAM) atuando no armazenamento temporário de sessões ativas e carrinhos de compras dos clientes (aliviando o banco relacional de operações de altíssima frequência), e o MongoDB (orientado a documentos JSON) para indexar o catálogo dinâmico com especificações técnicas altamente heterogêneas.")

    # =========================================================================
    # CAPÍTULO 6: PROGRAMAÇÃO ESTRUTURADA EM C
    # =========================================================================
    add_h1("6 PROGRAMAÇÃO ESTRUTURADA E MOTOR TRANSACIONAL EM C (ETAPA 5)")
    add_h2("6.1 Visão Geral e Contextualização do Backend Nativo em C")
    p_normal("O núcleo transacional do comércio eletrônico foi desenvolvido em Linguagem C estruturada (padrão ANSI/ISO C99), consubstanciado no arquivo sistema_loja.c com 1.021 linhas de código. O propósito central da escolha do C foi demonstrar a superioridade de desempenho de uma linguagem compilada de baixo nível sobre frameworks interpretados convencionais, proporcionando tempo de resposta na ordem de dezenas de microssegundos e consumo mínimo de memória RAM (inferior a 25 MB sob carga).")
    p_normal("O software opera em duas frentes totalmente integradas: uma interface de console interativa com menu ABNT para gestão interna administrativa e um servidor HTTP multithread nativo via Windows Sockets (winsock2.h) que responde na porta 8080, provendo API REST para a interface web corporativa.")

    add_h2("6.2 Modelagem das Estruturas de Dados em Memória (struct)")
    p_normal("Refletindo com exatidão as entidades do banco de dados relacional, o domínio de dados foi modelado através de quatro estruturas compostas (typedef struct):")
    p_code("""typedef struct {
    int id;                     // Identificador único (PK)
    char nome[80];              // Descrição comercial do hardware
    char categoria[40];         // Categoria associada
    double preco;               // Valor de venda unitário
    double preco_antigo;        // Preço promocional (de/por)
    int estoque;                // Saldo real disponível
    int estoque_minimo;         // Ponto de pedido / alerta de reposição
    char specs[80];             // Especificações resumidas
    char badge[30];             // Classificação comercial
} Produto;

typedef struct {
    int id_produto;             // Referência ao produto (FK)
    int quantidade;             // Unidades selecionadas
    double preco_unitario;      // Preço congelado no ato
    double subtotal;            // quantidade * preco_unitario
} ItemCarrinho;

typedef struct {
    char nome[80];              // Nome do comprador
    char cpf[16];               // CPF único (LGPD)
    char email[80];             // E-mail para contato
    char endereco[120];         // Endereço de entrega
} Cliente;

typedef struct {
    int id_pedido;              // ID numérico interno
    char codigo_pedido[32];     // Código amigável (PED-XXXXX)
    Cliente cliente;            // Registro do cliente associado
    ItemCarrinho itens[10];     // Vetor de itens da cesta
    int total_itens;            // Quantidade de itens distintos
    double valor_bruto;         // Subtotal bruto
    double desconto;            // Desconto de cupom aplicado
    double valor_total;         // Valor final líquido
    char forma_pagamento[30];   // PIX, Cartão ou Boleto
    char data_hora[30];         // Carimbo temporal da venda
} Pedido;""")

    add_h2("6.3 Modularização Funcional e Arquitetura de Funções")
    p_normal("O código-fonte obedece rigorosamente aos princípios de modularização da Engenharia de Software, segregando responsabilidades em funções independentes e especializadas:")

    p_table_caption("Tabela 6 – Resumo Técnico das Funções Implementadas em Linguagem C")
    funcoes_data = [
        ("carregar_produtos_arquivo()", "Lê produtos_estoque.txt na inicialização; cria base default se ausente.", "I/O Disco"),
        ("salvar_produtos_arquivo()", "Grava imediatamente em disco qualquer alteração de saldo de estoque.", "I/O Disco"),
        ("buscar_produto_por_id()", "Busca sequencial otimizada; retorna o índice no vetor ou -1.", "Pesquisa"),
        ("aplicar_cupom_desconto()", "Valida códigos (TECH10 = 10%, CYBER20 = 20%) via strcmp.", "Regra Negócio"),
        ("menu_realizar_venda()", "Conduz o fluxo completo no terminal aplicando baixa atômica e ponteiros.", "Transacional"),
        ("gravar_pedido_arquivo()", "Anexa recibo em pedidos_loja.txt no modo append (a+).", "Auditoria"),
        ("relatorio_gerencial()", "Exibe faturamento e lista hardwares com estoque <= estoque_minimo.", "Gerencial"),
        ("iniciar_servidor_web()", "Inicializa socket TCP 8080 e atende requisições REST (GET/POST).", "Rede / REST")
    ]
    add_table_styled(["Assinatura da Função em C", "Responsabilidade Técnica Operacional", "Módulo"], funcoes_data, [Cm(4.5), Cm(8.5), Cm(3.0)])
    p_table_source()

    add_h2("6.4 Algoritmo Crítico: Baixa Atômica de Estoque em Duas Fases")
    p_normal("A maior inovação algorítmica do projeto reside no mecanismo de Baixa Atômica em Duas Fases, desenvolvido para garantir as propriedades de Atomicidade e Consistência:")
    p_bullet("Fase 1 – Pré-validação Preventiva: Antes de tocar em qualquer saldo de estoque, o algoritmo percorre 100% dos itens da cesta de compras, checando se catalogo[idx].estoque >= item.quantidade. Se um único produto não tiver unidades suficientes, a transação inteira é imediatamente cancelada sem qualquer alteração na memória, emitindo aviso de erro amigável ao usuário.", bold_prefix="1. ")
    p_bullet("Fase 2 – Decremento e Persistência Concorrente: Somente após a confirmação de que todos os produtos solicitados estão disponíveis, o sistema executa o abatimento físico (catalogo[idx].estoque -= quantidade), grava a nova base no arquivo de estoque, anexa o comprovante em pedidos_loja.txt e conclui o faturamento.", bold_prefix="2. ")
    p_normal("No servidor web, esse algoritmo é encapsulado por seções críticas de sincronização (CRITICAL_SECTION), garantindo que requisições concorrentes de múltiplos clientes não gerem condições de corrida (race conditions) nem saldos negativos.")

    add_h2("6.5 Persistência de Dados em Disco (FILE*) e Auditoria")
    p_normal("A persistência no sistema em C realiza-se através da biblioteca padrão <stdio.h> em dois arquivos estruturados:")
    p_bullet("produtos_estoque.txt: Base de catálogo formatada em delimitadores ponto-e-vírgula (ID;NOME;CATEGORIA;PRECO;PRECO_ANTIGO;ESTOQUE;ESTOQUE_MINIMO;SPECS;BADGE), garantindo que modificações de inventário sobrevivam à reinicialização do software.", bold_prefix="• ")
    p_bullet("pedidos_loja.txt: Livro-razão contábil aberto em modo 'a+' (append), preservando o histórico integral de vendas com código único, dados do comprador mascarados, itens e valor faturado para auditoria contínua.", bold_prefix="• ")

    add_h2("6.6 Servidor HTTP Nativo (winsock2.h) e Interface Web SPA Corporativa")
    p_normal("Na opção [6] do console, o executável inicializa a biblioteca ws2_32.lib, vincula um socket TCP na porta 8080 (bind / listen) e atende clientes web:")
    p_bullet("GET /: Entrega o arquivo web/index.html, interface Single Page Application construída com paleta institucional sóbria (tons slate e azul marinho, alto contraste e sem poluição visual).", bold_prefix="• ")
    p_bullet("GET /api/produtos: Serializa as structs Produto da memória do C em uma resposta padronizada JSON contendo o catálogo atualizado em tempo real.", bold_prefix="• ")
    p_bullet("POST /api/checkout: Recebe o payload JSON com dados cadastrais e lista de itens, processa a baixa atômica em C e retorna o código amigável do pedido faturado.", bold_prefix="• ")

    # =========================================================================
    # CAPÍTULO 7: REDES DE COMPUTADORES, SEGURANÇA E NUVEM
    # =========================================================================
    add_h1("7 INFRAESTRUTURA DE REDES, SEGURANÇA E NUVEM (ETAPA 6)")
    add_h2("7.1 Arquitetura Cliente-Servidor e Pilha de Protocolos TCP/IP")
    p_normal("A comunicação corporativa e comercial da CyberHardware fundamenta-se na Arquitetura Cliente-Servidor Desacoplada, operando sobre a pilha de protocolos TCP/IP distribuída em quatro camadas essenciais:")
    p_bullet("Camada de Aplicação: Utiliza o protocolo HTTP/1.1 para tráfego web e chamadas REST (GET e POST) com payloads estruturados em formato JSON.", bold_prefix="1. ")
    p_bullet("Camada de Transporte: Emprega o protocolo orientado à conexão TCP (Transmission Control Protocol) instanciado nativamente via socket(AF_INET, SOCK_STREAM, 0). O TCP é imperativo por garantir entrega ordenada de pacotes, confirmação de recebimento (Three-Way Handshake: SYN -> SYN-ACK -> ACK) e controle de congestionamento em transações financeiras.", bold_prefix="2. ")
    p_bullet("Camada de Internet: Roteamento baseado no protocolo IPv4 com atribuição de endereços e máscaras de sub-rede padronizadas.", bold_prefix="3. ")
    p_bullet("Camada de Acesso à Rede (Física e Enlace): Padrão Ethernet IEEE 802.3 operando sobre cabeamento estruturado Categoria 6 (Cat6) na sede da Avenida Paulista, com taxas de transmissão de 1 Gbps a 10 Gbps.", bold_prefix="4. ")

    add_h2("7.2 Topologia Corporativa e Segmentação em VLANs")
    p_normal("Para isolar o tráfego público da Internet dos dados críticos corporativos e do servidor de banco de dados, a topologia de rede foi segmentada em Redes Locais Virtuais (VLANs IEEE 802.1Q) através de um Switch Core L3 e Firewall de Borda:")

    p_table_caption("Tabela 7 – Tabela de Endereçamento IP e Segmentação em VLANs")
    vlan_data = [
        ("VLAN 10", "DMZ / Web", "192.168.10.0/24", "192.168.10.1", "Servidores HTTP/REST em C e balanceador de carga Nginx."),
        ("VLAN 20", "Corporativa Sede", "192.168.20.0/24", "192.168.20.1", "Estações de trabalho, SAC e terminais administrativos da sede."),
        ("VLAN 30", "Dados Isolada", "192.168.30.0/24", "192.168.30.1", "Servidor SGBD MySQL (Porta 3306) e servidores de backup."),
        ("VPN Admin", "Gerência Remota", "10.8.0.0/24", "10.8.0.1", "Acesso remoto criptografado para desenvolvedores com MFA.")
    ]
    add_table_styled(["Segmento", "Nome Operacional", "Faixa de Rede (CIDR)", "Gateway", "Finalidade Operacional"], vlan_data, [Cm(2.0), Cm(3.0), Cm(3.2), Cm(2.5), Cm(5.3)])
    p_table_source()

    add_h2("7.3 Políticas de Firewall de Borda e Filtragem Default Deny")
    p_normal("O perímetro corporativo é protegido por um firewall UTM que implementa a política de segurança 'Default Deny' (todo o tráfego que não estiver formalmente permitido é bloqueado por padrão):")

    p_table_caption("Tabela 8 – Políticas de Filtragem de Pacotes do Firewall de Borda")
    firewall_data = [
        ("01", "Internet", "Load Balancer (DMZ)", "443 (HTTPS)", "TCP", "ALLOW", "Acesso público criptografado à loja virtual."),
        ("02", "Internet", "Load Balancer (DMZ)", "80 (HTTP)", "TCP", "REDIRECT", "Redirecionamento automático de HTTP para HTTPS (443)."),
        ("03", "Internet", "Servidores Web C", "8080 (HTTP)", "TCP", "DENY", "Impede acesso direto da Internet ao backend C nativo."),
        ("04", "Servidores C (DMZ)", "SGBD (VLAN 30)", "3306 (MySQL)", "TCP", "ALLOW", "Permite que o backend consulte e persista pedidos."),
        ("05", "Internet", "SGBD (VLAN 30)", "Qualquer", "Qualquer", "DENY", "Isolamento total da base de dados contra invasões externas."),
        ("06", "Rede Admin (VLAN 20)", "DMZ e Dados", "22 (SSH)", "TCP", "ALLOW", "Administração restrita com autenticação por chave e MFA.")
    ]
    add_table_styled(["Regra", "Origem", "Destino", "Porta/Serviço", "Prot.", "Ação", "Justificativa Técnica"], firewall_data, [Cm(1.2), Cm(2.5), Cm(3.2), Cm(2.2), Cm(1.2), Cm(1.8), Cm(3.9)])
    p_table_source()

    add_h2("7.4 Criptografia em Trânsito (HTTPS/TLS) e Blindagem LGPD")
    p_normal("Em conformidade direta com a Lei Geral de Proteção de Dados, o tráfego externo entre os navegadores dos compradores e a loja virtual trafega obrigatoriamente cifrado via protocolo HTTPS com encriptação TLS 1.3 (Transport Layer Security). O certificado digital de alta segurança impede ataques de interceptação de pacotes do tipo Man-in-the-Middle (MitM), blindando nomes, CPFs e pedidos em trânsito.")

    add_h2("7.5 Computação em Nuvem Escalável e Alta Disponibilidade na AWS")
    p_normal("Para absorver picos de tráfego promocionais (como campanhas de hardware com cupom CYBER20), a infraestrutura projeta-se na nuvem da Amazon Web Services (AWS):")
    p_bullet("Application Load Balancer (ALB): Recebe as requisições HTTPS públicas na porta 443, efetua a terminação SSL/TLS com certificado do AWS Certificate Manager e distribui a carga de forma equitativa entre os nós do servidor em C.", bold_prefix="• ")
    p_bullet("Auto Scaling Group: Monitora a utilização de processamento das instâncias Amazon EC2 (t3.medium). Ao detectar consumo sustentado superior a 70% de CPU por mais de 3 minutos, novas instâncias são instanciadas automaticamente, reduzindo-as quando a carga normaliza.", bold_prefix="• ")
    p_bullet("Amazon RDS Multi-AZ: Banco de dados relacional gerenciado com réplica síncrona em outra Zona de Disponibilidade, assegurando failover automático em segundos e recuperação de desastres (Disaster Recovery).", bold_prefix="• ")

    # =========================================================================
    # CAPÍTULO 8: DIREITOS HUMANOS, ACESSIBILIDADE E LGPD
    # =========================================================================
    add_h1("8 DIREITOS HUMANOS, ACESSIBILIDADE E LGPD (ETAPA 7)")
    add_h2("8.1 Acessibilidade Digital e Inclusão Universal (ABNT NBR 17225 / WCAG)")
    p_normal("A acessibilidade no comércio eletrônico constitui um imperativo ético e um dever legal indispensável. A plataforma web da CyberHardware foi construída em estrita observância à Lei Brasileira de Inclusão da Pessoa com Deficiência (Lei nº 13.146/2015, Art. 63), à ABNT NBR 17225:2025, à ABNT NBR ISO 9241-11:2018 e às diretrizes internacionais da WCAG 2.2 fundamentadas nos quatro princípios POUR (Perceptível, Operável, Compreensível e Robusto):")

    p_table_caption("Tabela 9 – Critérios Normativos de Acessibilidade Aplicados no E-commerce")
    acess_data = [
        ("Navegação por Teclado e Skip Link", "Atalho oculto 'Pular para o conteúdo principal' (#main-content) via tecla Tab com foco visível.", "Permite acesso imediato ao catálogo sem passar repetidamente pelo menu."),
        ("Tecnologias Assistivas e ARIA", "Marcos semânticos (header, main, nav, dialog, aria-modal, aria-label) e tags descritivas.", "Garante leitura clara e ordenada por leitores de tela (NVDA, JAWS e VoiceOver)."),
        ("Contraste Cromático Sóbrio", "Paleta institucional (texto #0F172A sobre fundo #FFFFFF e botões #1E40AF) com contraste > 7:1.", "Atende aos níveis rigorosos AA/AAA, auxiliando usuários com baixa visão ou daltonismo."),
        ("Independência de Cor em Alertas", "Status de estoque com texto explícito ('X un. em estoque' ou 'Esgotado') além da cor.", "Impede ambiguidade para daltônicos ou pessoas com monitores monocromáticos."),
        ("Formulários Rastreáveis", "Todos os campos do checkout com labels explicitamente vinculados via atributo 'for'.", "Reduz erros operacionais e facilita preenchimento por pessoas com baixo letramento digital.")
    ]
    add_table_styled(["Critério Normativo Aplicado", "Implementação Realizada no Sistema", "Benefício Direto para o Cidadão"], acess_data, [Cm(4.0), Cm(6.5), Cm(5.5)])
    p_table_source()

    add_h2("8.2 Privacidade e Proteção de Dados Pessoais (LGPD - Lei nº 13.709/2018)")
    p_normal("A coleta e o tratamento de dados pessoais no e-commerce da CyberHardware operam sob o conceito de Privacidade desde a Concepção (Privacy by Design), em estrita conformidade com o Artigo 6º da LGPD:")
    p_bullet("Princípio da Minimização de Dados (Art. 6º, III): Tanto nas structs em C quanto no banco relacional, a plataforma coleta estritamente o mínimo necessário para faturamento e remessa postal (Nome, CPF, E-mail e Endereço). Não são coletados ou tratados quaisquer dados pessoais sensíveis (como etnia, religião ou biometria).", bold_prefix="• ")
    p_bullet("Não Armazenamento de Dados Financeiros Críticos: O sistema não salva números de cartões de crédito, códigos de segurança (CVV) ou senhas bancárias em arquivos ou tabelas. A aprovação ocorre via tokens efêmeros de transação.", bold_prefix="• ")
    p_bullet("Bases Legais Claras (Art. 7º, II e V): O tratamento fundamenta-se na execução de contrato de compra e venda a pedido do titular e no cumprimento de obrigação legal e fiscal (emissão de documentos fiscais eletrônicos).", bold_prefix="• ")

    add_h2("8.3 Segurança da Informação Preservando a Tríade CID")
    p_normal("As defesas do sistema alinham-se aos três pilares da Segurança da Informação:")
    p_bullet("Confidencialidade: Assegurada pelo isolamento da base de dados na VLAN 30, criptografia TLS 1.3 no tráfego externo e mascaramento de dados cadastrais.", bold_prefix="• ")
    p_bullet("Integridade: Garantida pela prevenção contra estouro de buffer (uso de fgets com limites sizeof no código em C) e restrições CHECK de validação no banco de dados.", bold_prefix="• ")
    p_bullet("Disponibilidade: Assegurada pela persistência contínua em modo append (pedidos_loja.txt), réplicas Multi-AZ no banco e rotinas automatizadas de backup diário.", bold_prefix="• ")

    add_h2("8.4 Ética Comercial, Código de Defesa do Consumidor e Impactos Ambientais")
    p_normal("A atuação comercial pauta-se pelo Código de Defesa do Consumidor (Lei nº 8.078/1990) e pelo Decreto do Comércio Eletrônico (Decreto nº 7.962/2013). A interface web rejeita veementemente interfaces manipulativas (dark patterns): preços, cupons de desconto, frete e parcelamentos são exibidos de forma clara e transparente antes de qualquer cobrança.")
    p_normal("Sob a perspectiva de sustentabilidade ambiental, alinhando-se à Política Nacional de Resíduos Sólidos (Lei nº 12.305/2010), a CyberHardware disponibiliza em suas páginas orientações aos clientes sobre logística reversa e descarte ambientalmente adequado de lixo eletrônico (e-waste), minimizando o descarte incorreto de circuitos integrados e metais pesados no meio ambiente.")

    # =========================================================================
    # CAPÍTULO 9: INTEGRAÇÃO DA SOLUÇÃO E CONSIDERAÇÕES FINAIS
    # =========================================================================
    add_h1("9 INTEGRAÇÃO DA SOLUÇÃO E CONSIDERAÇÕES FINAIS (ETAPA 8)")
    add_h2("9.1 Arquitetura Integrada da Solução Multidisciplinar")
    p_normal("O desenvolvimento do Projeto Integrado Multidisciplinar II consolidou uma solução computacional única e harmônica, cumprindo com excelência o propósito pedagógico da UNIP ao articular organicamente as seis disciplinas do semestre:")
    p_normal("O cliente acessa a loja por conexão segura HTTPS (Etapa 6 e 7) através de uma interface web corporativa de alto contraste com navegação acessível por teclado (Etapas 3 e 7). Os componentes de hardware do catálogo são consultados via rotas REST estruturadas (Etapa 3 e 5). Ao fechar a compra, a requisição é interceptada em milissegundos pelo backend em C (Etapa 5), que aplica o algoritmo de Baixa Atômica em Duas Fases protegido por exclusão mútua transacional, eliminando a ruptura de estoque fundamentada na pesquisa científica (Etapa 2). A persistência dos dados e a emissão do pedido respeitam as tabelas normalizadas na 3FN (Etapa 4), o princípio da minimização da LGPD (Etapa 7) e a topologia isolada em sub-redes corporativas (Etapa 6).")

    add_h2("9.2 Análise Comparativa: Cenário Anterior vs. Solução Implantada")
    p_normal("O quadro comparativo a seguir sintetiza a evolução proporcionada pelo sistema em relação aos gargalos diagnosticados na Etapa 1:")

    p_table_caption("Tabela 10 – Análise Comparativa: Cenário Anterior vs. Solução Implantada")
    comp_data = [
        ("Controle de Estoque", "Manual em planilhas; vendas em duplicidade e ruptura invisível.", "Baixa atômica preventiva em 2 fases no C; bloqueio instantâneo."),
        ("Performance Web", "Lentidão acentuada em picos; tempo de resposta > 4 segundos.", "Backend em C compilado (-O2) com respostas REST < 50ms e Auto Scaling."),
        ("Persistência de Dados", "Dados descentralizados, redundantes e sujeitos a anomalias.", "Banco relacional na 3FN com chaves FK, CHECK e preço congelado."),
        ("Segurança de Redes", "Rede plana sem segmentação; dados expostos a ataques de interceptação.", "Rede segmentada em 3 VLANs, Firewall Default Deny e HTTPS/TLS 1.3."),
        ("Conformidade Legal", "Coleta excessiva de dados cadastrais; risco de penalidades na LGPD.", "Privacy by Design, minimização de dados e não retenção de cartões."),
        ("Acessibilidade Digital", "Interface inacessível para pessoas com deficiência ou navegação por voz.", "Conformidade com ABNT NBR 17225:2025 e WCAG 2.2 (tags ARIA, alto contraste).")
    ]
    add_table_styled(["Dimensão Técnica", "Cenário Anterior (Pré-Solução)", "Cenário Atual (Pós-Implantação no PIM II)"], comp_data, [Cm(3.5), Cm(6.0), Cm(6.5)])
    p_table_source()

    add_h2("9.3 Benefícios Operacionais, Tecnológicos e Sociais Alcançados")
    p_normal("A implementação da plataforma trouxe ganhos imediatos e mensuráveis para a CyberHardware: a eliminação das vendas em duplicidade zerou os custos operacionais com estornos bancários e preservou a fidelidade dos clientes; o relatório gerencial em C com alerta de estoque mínimo antecipa as ordens aos fornecedores; a eficiência computacional do C reduziu drasticamente a despesa mensal com servidores em nuvem; e a acessibilidade digital expandiu o alcance comercial para milhões de cidadãos com deficiência.")

    add_h2("9.4 Limitações do Protótipo e Perspectivas de Evolução Futura")
    p_normal("Como todo produto tecnológico em ciclo ágil iterativo, o protótipo funcional atual atingiu integralmente o escopo do Produto Mínimo Viável (MVP), deixando perspectivas de expansão para ciclos futuros: integração direta do código C com drivers de conexão nativa ao MySQL (libmysqlclient) e Redis (hiredis), automação com webhooks de gateways bancários para confirmação instantânea de PIX e módulo de emissão automatizada de Notas Fiscais Eletrônicas (NF-e) perante a SEFAZ.")

    add_h2("9.5 Considerações Finais e Resposta à Pergunta de Pesquisa")
    p_normal("Ao retomar a pergunta de pesquisa estabelecida na Etapa 2 — 'Como desenvolver uma solução computacional integrada de e-commerce que concilie alto desempenho transacional e baixa latência no controle de estoque com a eliminação definitiva da ruptura invisível, assegurando integridade transacional e conformidade aos padrões de segurança, privacidade e acessibilidade universal?' —, conclui-se que o objetivo geral e os objetivos específicos foram plenamente alcançados.")
    p_normal("A experiência interdisciplinar de concepção e desenvolvimento do PIM II demonstrou que a engenharia de software contemporânea exige a união indissociável entre rigor metodológico, modelagem de dados sólida, domínio de linguagens de programação de alto desempenho, arquitetura segura de redes e respeito estrito aos Direitos Humanos e à legislação vigente. A equipe entrega à UNIP e ao mercado um trabalho acadêmico maduro, completo e de altíssimo valor técnico.")

    # =========================================================================
    # ELEMENTOS PÓS-TEXTUAIS: REFERÊNCIAS BIBLIOGRÁFICAS
    # =========================================================================
    add_h1("REFERÊNCIAS BIBLIOGRÁFICAS")
    referencias = [
        "ABIACOM – Associação Brasileira de Inteligência Artificial e E-commerce; CNDL. Relatório de Desempenho do Comércio Eletrônico Brasileiro: 1º Semestre de 2026. São Paulo: ABIACOM, 2026.",
        "AGUIAR, L. C.; SAMPAIO, M. Análise da Ruptura de Estoque no Varejo: Um Estudo Empírico em Hipermercados Brasileiros. Revista Produção, São Paulo, v. 31, e20200085, 2021. Disponível em: <https://www.scielo.br>. Acesso em: 01 nov. 2026.",
        "ASSOCIAÇÃO BRASILEIRA DE NORMAS TÉCNICAS. ABNT NBR 14724: Informação e documentação – Trabalhos acadêmicos – Apresentação. Rio de Janeiro: ABNT, 2011.",
        "ASSOCIAÇÃO BRASILEIRA DE NORMAS TÉCNICAS. ABNT NBR 17060: Acessibilidade em aplicativos de dispositivos móveis – Requisitos. Rio de Janeiro: ABNT, 2022.",
        "ASSOCIAÇÃO BRASILEIRA DE NORMAS TÉCNICAS. ABNT NBR 17225: Acessibilidade em conteúdo e aplicações web – Requisitos. Rio de Janeiro: ABNT, 2025.",
        "ASSOCIAÇÃO BRASILEIRA DE NORMAS TÉCNICAS. ABNT NBR 6023: Informação e documentação – Referências – Elaboração. Rio de Janeiro: ABNT, 2018.",
        "ASSOCIAÇÃO BRASILEIRA DE NORMAS TÉCNICAS. ABNT NBR ISO 9241-11: Ergonomia da interação humano-sistema – Parte 11: Usabilidade: definições e conceitos. Rio de Janeiro: ABNT, 2018.",
        "BRASIL. Constituição da República Federativa do Brasil de 1988. Brasília, DF: Presidência da República, 1988.",
        "BRASIL. Decreto nº 7.962, de 15 de março de 2013. Regulamenta a Lei nº 8.078, de 11 de setembro de 1990, para dispor sobre a contratação no comércio eletrônico. Brasília, DF: Presidência da República, 2013.",
        "BRASIL. Lei nº 8.078, de 11 de setembro de 1990. Dispõe sobre a proteção do consumidor e dá outras providências. Brasília, DF: Presidência da República, 1990.",
        "BRASIL. Lei nº 12.305, de 2 de agosto de 2010. Institui a Política Nacional de Resíduos Sólidos. Brasília, DF: Presidência da República, 2010.",
        "BRASIL. Lei nº 13.146, de 6 de julho de 2015. Institui a Lei Brasileira de Inclusão da Pessoa com Deficiência (Estatuto da Pessoa com Deficiência). Brasília, DF: Presidência da República, 2015.",
        "BRASIL. Lei nº 13.709, de 14 de agosto de 2018. Lei Geral de Proteção de Dados Pessoais (LGPD). Brasília, DF: Presidência da República, 2018.",
        "COMER, Douglas E. Redes de Computadores e Internet: tópicos fundamentais. 6. ed. Porto Alegre: Bookman, 2016.",
        "DEITEL, Paul; DEITEL, Harvey. C: como programar. 6. ed. São Paulo: Pearson Prentice Hall, 2011.",
        "ELMASRI, Ramez; NAVATHE, Shamkant B. Sistemas de Banco de Dados. 7. ed. São Paulo: Pearson Education do Brasil, 2019.",
        "EVISION CONSULTING. Os 7 Gargalos Críticos de Infraestrutura e Conversão no E-commerce Moderno. São Paulo: eVision Report, 2026.",
        "FTI CONSULTING. Online Retail Report: Desempenho e Tendências do Varejo Digital na América Latina. Washington: FTI, 2025.",
        "GIL, Antonio Carlos. Como Elaborar Projetos de Pesquisa. 6. ed. São Paulo: Atlas, 2019.",
        "GOOGLE CLOUD. Diagnóstico de Resiliência Tecnológica e Jornada do Cliente no Varejo Brasileiro. São Paulo: Google Cloud Brasil, 2025.",
        "GRUEN, T. W.; CORSTEN, D. S.; BHARADWAJ, S. Retail Out-of-Stocks: A Worldwide Examination of Extent, Causes, and Consumer Responses. Washington: Grocery Manufacturers of America, 2002.",
        "KERNIGHAN, Brian W.; RITCHIE, Dennis M. C: a linguagem de programação padrão ANSI. Rio de Janeiro: Campus, 1990.",
        "KUROSE, James F.; ROSS, Keith W. Redes de Computadores e a Internet: uma abordagem top-down. 7. ed. São Paulo: Pearson Education do Brasil, 2021.",
        "LARMAN, Craig. Utilizando UML e Padrões: uma introdução à análise e ao projeto orientados a objetos e ao desenvolvimento iterativo. 3. ed. Porto Alegre: Bookman, 2007.",
        "MARTIN, Robert C. Clean Code: A Handbook of Agile Software Craftsmanship. Upper Saddle River: Prentice Hall, 2008.",
        "ORGANIZAÇÃO DAS NAÇÕES UNIDAS (ONU). Declaração Universal dos Direitos Humanos. Paris: ONU, 1948.",
        "PRESSMAN, Roger S.; MAXIM, Bruce R. Engenharia de Software: uma abordagem profissional. 9. ed. Porto Alegre: AMGH, 2021.",
        "SCHWABER, Ken; SUTHERLAND, Jeff. O Guia do Scrum: as regras do jogo. Scrum.org, 2020. Disponível em: <https://scrumguides.org/>. Acesso em: 08 nov. 2026.",
        "SOFIST; CNN BRASIL. Monitoramento de Performance e Estudo de Latência em Plataformas de E-commerce na Black Friday. São Paulo: CNN Brasil / Sofist, 2024.",
        "SOMMERVILLE, Ian. Engenharia de Software. 10. ed. São Paulo: Pearson Education do Brasil, 2019.",
        "VERGARA, Sylvia Constant. Projetos e Relatórios de Pesquisa em Administração. 16. ed. São Paulo: Atlas, 2016.",
        "WORLD WIDE WEB CONSORTIUM (W3C). Web Content Accessibility Guidelines (WCAG) 2.2. [S. l.]: W3C, 2023. Disponível em: <https://www.w3.org/TR/WCAG22/>. Acesso em: 28 out. 2026."
    ]
    for ref in referencias:
        p = doc.add_paragraph()
        p.alignment = WD_ALIGN_PARAGRAPH.JUSTIFY
        p.paragraph_format.line_spacing = 1.0
        p.paragraph_format.space_before = Pt(4)
        p.paragraph_format.space_after = Pt(8)
        p.paragraph_format.first_line_indent = Cm(0)
        r = p.add_run(ref)
        r.font.name = 'Arial'
        r.font.size = Pt(11)

    # =========================================================================
    # APÊNDICES
    # =========================================================================
    add_h1("APÊNDICE A – ESTRUTURA DO CÓDIGO-FONTE EM C (sistema_loja.c)")
    p_normal("Abaixo são apresentados trechos estruturantes do código em C compilado (sistema_loja.c), evidenciando a inicialização de sockets TCP com winsock2.h, o despacho de rotas HTTP REST e a baixa atômica de estoque:")
    p_code("""// Trecho: Inicialização do Socket TCP e Loop de Conexões HTTP (Porta 8080)
WSADATA wsa;
SOCKET server_socket, client_socket;
struct sockaddr_in server_addr, client_addr;
int client_len = sizeof(client_addr);

WSAStartup(MAKEWORD(2,2), &wsa);
server_socket = socket(AF_INET, SOCK_STREAM, 0);

server_addr.sin_family = AF_INET;
server_addr.sin_addr.s_addr = INADDR_ANY;
server_addr.sin_port = htons(PORTA_SERVIDOR); // 8080

bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr));
listen(server_socket, 10);

printf("[*] Servidor HTTP CyberHardware rodando em http://localhost:8080\\n");

while (1) {
    client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
    if (client_socket != INVALID_SOCKET) {
        tratar_requisicao_http(client_socket, catalogo, &total_produtos, &total_pedidos, &faturamento);
        closesocket(client_socket);
    }
}
WSACleanup();""")

    add_h1("APÊNDICE B – SCRIPT DE CRIAÇÃO DO BANCO DE DADOS (DDL/DML SQL)")
    p_normal("Abaixo são apresentados os scripts DDL de criação e DML de população do banco de dados relacional MySQL na 3ª Forma Normal:")
    p_code("""-- Criação da Base de Dados Oficial da CyberHardware
CREATE DATABASE IF NOT EXISTS cyberhardware_db CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
USE cyberhardware_db;

-- Inserção de Dados Iniciais de Teste (DML Integrado ao Catálogo em C)
INSERT INTO Categoria (nome_categoria) VALUES 
('Placas de Vídeo'), ('Processadores'), ('Periféricos'), ('Monitores'), ('Armazenamento');

INSERT INTO Produto (id_categoria, nome_comercial, preco_unitario, quantidade_estoque, estoque_minimo, especificacoes) VALUES 
(1, 'Placa de Vídeo RTX 4080 Super 16GB', 7299.90, 5, 2, '16GB GDDR6X | 256-bit | Tri-Fan'),
(2, 'Processador AMD Ryzen 7 7800X3D', 2899.00, 8, 3, '8 Cores / 16 Threads | 104MB Cache'),
(3, 'Teclado Mecânico RGB Switch Red', 349.90, 15, 5, 'ABNT2 | Anti-Ghosting | Switch Linear'),
(3, 'Mouse Gamer Sem Fio 26000 DPI', 299.90, 12, 4, 'Sensor Óptico 26K | Peso 58g'),
(4, 'Monitor Gamer 27\" IPS 165Hz 1ms', 1499.00, 6, 2, '27 pol | Painel IPS | 165Hz'),
(5, 'SSD NVMe M.2 2TB Gen4 7000MB/s', 899.90, 10, 3, 'PCIe 4.0 | Leitura 7000MB/s');

INSERT INTO Cliente (nome_completo, cpf, email, endereco) VALUES 
('Carlos Almeida', '123.456.789-00', 'carlos.almeida@email.com', 'Rua das Orquídeas, 50'),
('Mariana Souza', '987.654.321-11', 'mariana.souza@email.com', 'Av. Brasil, 1500');""")

    nome_arquivo = "PIM_II_UNIP_DOCUMENTO_FINAL_ESBOCO.docx"
    doc.save(nome_arquivo)
    print(f"[OK] Documento gerado com sucesso: {nome_arquivo} (Tamanho: {os.path.getsize(nome_arquivo)} bytes)")

if __name__ == "__main__":
    create_pim_document()
