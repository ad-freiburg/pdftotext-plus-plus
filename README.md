<br/>
<p align="center">
  <a href="https://github.com/pdftotext-plus-plus/pdftotext-plus-plus">
    <img src="logo-2.png" alt="logo" width="200">
  </a>
</p>

<p align="center">
  <a href="https://github.com/pdftotext-plus-plus/pdftotext-plus-plus/actions/workflows/checkstyle.yml"><img src="https://github.com/pdftotext-plus-plus/pdftotext-plus-plus/actions/workflows/checkstyle.yml/badge.svg"></a>
  <a href="https://github.com/pdftotext-plus-plus/pdftotext-plus-plus/actions/workflows/unit_test.yml"><img src="https://github.com/pdftotext-plus-plus/pdftotext-plus-plus/actions/workflows/unit_test.yml/badge.svg"></a>
</p>

<p align="center">
  <a href="https://pdftotext.cs.uni-freiburg.de">Explore the docs</a>
  &nbsp·&nbsp
  <a href="https://github.com/pdftotext-plus-plus/pdftotext-plus-plus/issues">Report a Bug</a>
  &nbsp·&nbsp
  <a href="https://github.com/pdftotext-plus-plus/pdftotext-plus-plus/issues">Request a Feature</a>
</p>
<br>

A fast and accurate command line tool for extracting text from PDF files. The main features are:
* accurate detection of words, text lines and text blocks
* splitting ligatures into separate characters, for example: *ﬃ* into *f*, *f*, and *i*.
* merging characters with combining diacritical marks to single characters, for example: *`a* to *à*.
* detecting the semantic roles (for example: *title*, *author*, *section heading*, *paragraph*, *footnote*) of text blocks
* detecting the natural reading order of text blocks
* merging hyphenated words
* detecting sub- and superscripts
* customizable output of the extracted text, for example: in plain text format, or in a structured format (JSONL) in which the text is annotated with different layout information (for example: the font, font size, or position of the text in the PDF).

*pdftotext++* is written in C++ and is based on Poppler's <a href="https://github.com/pdftotext-plus-plus/pdftotext-plus-plus">pdftotext</a>.
There are several installation options (e.g., via APT, Docker, or building from source), see the [description below](#installation).

<!-- =========================================================================================== -->

## Quick Usage Guide

Extract the text from *file.pdf* and output it to the console:
```
pdftotext++ file.pdf -
```

Extract the text from *file.pdf* and write it to *output.txt*:
```
pdftotext++ file.pdf output.txt
```

Extract the words from *file.pdf* and output them together with layout information in JSONL format:
```
pdftotext++ --output-words file.pdf -
```

Extract the text from *file.pdf*, output it to the console and create a PDF file *words.pdf* in which a bounding box is drawn around each detected word (this is particularly useful for debugging):
```
pdftotext++ --visualize-words --visualization-path words.pdf file.pdf -
```

Print the full usage information:
```
pdftotext++ --help
```

## Installation

### Apt (Recommended)
Install packages to allow *apt* to use a repository over HTTPS:
```
sudo apt-get update
sudo apt-get install ca-certificates curl gnupg lsb-release
```

Add *pdftotext++*'s official GPG key:
```
sudo mkdir -m 0755 -p /etc/apt/keyrings
curl -fsSL https://pdftotext.cs.uni-freiburg.de/download/apt/gpg | sudo gpg --dearmor -o /etc/apt/keyrings/pdftotext-plus-plus.gpg
```

Add the repository and install *pdftotext++*:
```
echo "deb [arch=$(dpkg --print-architecture) signed-by=/etc/apt/keyrings/pdftotext-plus-plus.gpg] https://pdftotext.cs.uni-freiburg.de/download/apt $(lsb_release -cs) stable" | sudo tee /etc/apt/sources.list.d/pdftotext-plus-plus.list > /dev/null
sudo apt-get update
sudo apt-get install pdftotext++
```
Run *pdftotext++* (type `pdftotext++ --help` to see the full usage information):
```
pdftotext++ [options] <pdf-file> <output-file>
```

### Docker
  ```
  git clone https://github.com/pdftotext-plus-plus/pdftotext-plus-plus.git
  cd pdftotext-plus-plus
  docker build -f Dockerfiles/Dockerfile -t pdftotext-plus-plus .
  docker run --rm -it -v <pdf-file>:/file.pdf --name pdftotext-plus-plus pdftotext-plus-plus [options] /file.pdf <output-file>
  ```

### Build from source (on a Debian system)

  ```bash
  git clone https://github.com/pdftotext-plus-plus/pdftotext-plus-plus.git
  cd pdftotext-plus-plus
  sudo apt-get update
  sudo apt-get install -y build-essential cmake git tar wget
  wget https://github.com/mikefarah/yq/releases/latest/download/yq_linux_amd64 -O /usr/bin/yq
  chmod +x /usr/bin/yq
  make requirements/pre
  make requirements/run
  ldconfig
  make clean compile
  ./build/pdftotext++ [options] <pdf-file> <output-file>
  ```
