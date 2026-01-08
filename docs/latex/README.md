# LaTeX Report (Library Management System)

This folder contains a polished LaTeX report for the project.

## Build (recommended: latexmk)

Install TeX:
- **Ubuntu/Debian**: `sudo apt-get update && sudo apt-get install -y texlive-full latexmk`
- **Arch**: `sudo pacman -S texlive-most latexmk`
- **macOS**: install **MacTeX** and ensure `latexmk` is available
- **Windows**: install **MiKTeX** or **TeX Live**

Then build:

```bash
cd docs/latex
latexmk -pdf -interaction=nonstopmode -halt-on-error main.tex
```

Output: `docs/latex/main.pdf`

## Clean

```bash
cd docs/latex
latexmk -C
```


