import os
import re
import subprocess
import markdown

EDGE_PATH = r"C:\Program Files (x86)\Microsoft\Edge\Application\msedge.exe"
BASE_DIR = os.path.dirname(os.path.abspath(__file__))

CSS_ESTILO = """
@page {
    size: A4;
    margin: 20mm 15mm 20mm 15mm;
}
body {
    font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Helvetica, Arial, sans-serif;
    font-size: 11pt;
    line-height: 1.6;
    color: #24292f;
    background-color: #ffffff;
}
h1, h2, h3, h4 {
    color: #0969da;
    font-weight: 700;
    margin-top: 1.4em;
    margin-bottom: 0.5em;
    page-break-after: avoid;
}
h1 {
    font-size: 18pt;
    border-bottom: 2px solid #0969da;
    padding-bottom: 0.3em;
    color: #1f2328;
}
h2 {
    font-size: 14pt;
    border-bottom: 1px solid #d0d7de;
    padding-bottom: 0.2em;
}
h3 {
    font-size: 12pt;
}
p, ul, ol {
    margin-bottom: 0.8em;
}
li {
    margin-bottom: 0.3em;
}
table {
    border-collapse: collapse;
    width: 100%;
    margin: 1em 0;
    font-size: 9.5pt;
    page-break-inside: avoid;
}
th, td {
    border: 1px solid #d0d7de;
    padding: 6px 10px;
    text-align: left;
}
th {
    background-color: #f6f8fa;
    font-weight: 600;
}
tr:nth-child(even) {
    background-color: #fbfcfd;
}
code {
    font-family: ui-monospace, SFMono-Regular, "SF Mono", Menlo, Consolas, monospace;
    font-size: 85%;
    background-color: #eff1f3;
    padding: 0.2em 0.4em;
    border-radius: 4px;
}
pre {
    background-color: #f6f8fa;
    border: 1px solid #d0d7de;
    border-radius: 6px;
    padding: 12px;
    overflow-x: auto;
    font-size: 9pt;
    line-height: 1.45;
    page-break-inside: avoid;
}
pre code {
    background-color: transparent;
    padding: 0;
}
blockquote {
    border-left: 4px solid #0969da;
    padding: 0.5em 1em;
    color: #57606a;
    background-color: #f6f8fa;
    margin: 1em 0;
    border-radius: 0 4px 4px 0;
}
.mermaid {
    text-align: center;
    margin: 1.5em 0;
    page-break-inside: avoid;
}
hr {
    border: none;
    border-top: 1px solid #d0d7de;
    margin: 1.5em 0;
}
"""

def converter_md_para_pdf(nome_md, nome_pdf):
    caminho_md = os.path.join(BASE_DIR, nome_md)
    caminho_pdf = os.path.join(BASE_DIR, nome_pdf)
    html_temp = os.path.join(BASE_DIR, nome_md.replace(".md", "_temp.html"))

    with open(caminho_md, "r", encoding="utf-8") as f:
        conteudo_md = f.read()

    def substituir_mermaid(match):
        codigo = match.group(1).strip()
        return f'<div class="mermaid">\n{codigo}\n</div>'

    conteudo_md = re.sub(r'```mermaid\s*\n(.*?)\n```', substituir_mermaid, conteudo_md, flags=re.DOTALL)

    html_corpo = markdown.markdown(
        conteudo_md,
        extensions=['tables', 'fenced_code', 'nl2br']
    )

    html_completo = f"""<!DOCTYPE html>
<html lang="pt-BR">
<head>
    <meta charset="UTF-8">
    <title>{nome_pdf}</title>
    <script src="https://cdn.jsdelivr.net/npm/mermaid@10/dist/mermaid.min.js"></script>
    <script>
        mermaid.initialize({{ startOnLoad: true, theme: 'neutral' }});
    </script>
    <style>
        {CSS_ESTILO}
    </style>
</head>
<body>
    {html_corpo}
</body>
</html>
"""

    with open(html_temp, "w", encoding="utf-8") as f:
        f.write(html_completo)

    cmd = [
        EDGE_PATH,
        "--headless",
        "--disable-gpu",
        "--run-all-compositor-stages-before-draw",
        "--virtual-time-budget=2000",
        f"--print-to-pdf={caminho_pdf}",
        f"file:///{html_temp.replace(os.sep, '/')}"
    ]

    print(f"[*] Gerando PDF: {nome_pdf}...", flush=True)
    subprocess.run(cmd, check=True)

    if os.path.exists(html_temp):
        os.remove(html_temp)

    print(f"[OK] PDF gerado: {nome_pdf} (Tamanho: {os.path.getsize(caminho_pdf)} bytes)", flush=True)

if __name__ == "__main__":
    import sys

    if len(sys.argv) > 2:
        converter_md_para_pdf(sys.argv[1], sys.argv[2])
    else:
        arquivos = [
            ("ETAPA_3_KENAI_PARTE_1_DIRETRIZES.md", "ETAPA_3_KENAI_PARTE_1_DIRETRIZES.pdf"),
            ("ETAPA_3_KENAI_PARTE_2_GABARITO_PRONTO.md", "ETAPA_3_KENAI_PARTE_2_GABARITO_PRONTO.pdf"),
            ("ETAPA_4_SAULLO_PARTE_1_DIRETRIZES.md", "ETAPA_4_SAULLO_PARTE_1_DIRETRIZES.pdf"),
            ("ETAPA_4_SAULLO_PARTE_2_GABARITO_PRONTO.md", "ETAPA_4_SAULLO_PARTE_2_GABARITO_PRONTO.pdf"),
            ("ETAPA_6_VITOR_GODOY_PARTE_1_DIRETRIZES.md", "ETAPA_6_VITOR_GODOY_PARTE_1_DIRETRIZES.pdf"),
            ("ETAPA_6_VITOR_GODOY_PARTE_2_GABARITO_PRONTO.md", "ETAPA_6_VITOR_GODOY_PARTE_2_GABARITO_PRONTO.pdf"),
            ("ETAPA_8_CAUE_PARTE_1_DIRETRIZES.md", "ETAPA_8_CAUE_PARTE_1_DIRETRIZES.pdf"),
            ("ETAPA_8_CAUE_PARTE_2_RELATORIO_PRONTO.md", "ETAPA_8_CAUE_PARTE_2_RELATORIO_PRONTO.pdf"),
        ]

        for md_file, pdf_file in arquivos:
            if os.path.exists(os.path.join(BASE_DIR, md_file)):
                converter_md_para_pdf(md_file, pdf_file)
            else:
                print(f"[!] Arquivo não encontrado: {md_file}", flush=True)
