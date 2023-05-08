<br/>
<p align="center">
  <a href="https://github.com/ad-freiburg/pdftotext-plus-plus">
    <img src="logo.png" alt="logo" width="200">
  </a>
</p>

<p align="center">
  <a href="https://github.com/ad-freiburg/pdftotext-plus-plus/actions/workflows/checkstyle.yml">
    <img src="https://github.com/ad-freiburg/pdftotext-plus-plus/actions/workflows/checkstyle.yml/badge.svg">
  </a>
  <a href="https://github.com/ad-freiburg/pdftotext-plus-plus/actions/workflows/unit_test.yml">
    <img src="https://github.com/ad-freiburg/pdftotext-plus-plus/actions/workflows/unit_test.yml/badge.svg">
  </a>
</p>

<p align="center">
  <a href="https://pdftotext.cs.uni-freiburg.de">Explore the docs</a>
  &nbsp·&nbsp
  <a href="https://github.com/ad-freiburg/pdftotext-plus-plus/issues">Report a bug</a>
  &nbsp·&nbsp
  <a href="https://github.com/ad-freiburg/pdftotext-plus-plus/issues">Request a feature</a>
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
* customizable output of the extracted text, for example: in plain text format, or in a structured format (JSONL) in which the text is annotated with layout information (for example: the font, the font size, or the position).

*pdftotext++* is written in C++ and is based on <a href="https://github.com/freedesktop/poppler">Poppler's</a> <a href="https://github.com/freedesktop/poppler/blob/master/utils/pdftotext.cc">pdftotext</a>.
There are several installation options (for example, via Apt, Docker, or building from source), see the [description below](#installation).

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

Extract the text from *file.pdf*, output it to the console, and create a PDF file *words.pdf* in which a bounding box is drawn around each detected word (this is particularly useful for debugging purposes):
```
pdftotext++ --visualize-words --visualization-path words.pdf file.pdf -
```

Print the full usage information:
```
pdftotext++ --help
```

## Installation

### Apt (Recommended)
(1) Install required packages (for example, to allow *Apt* to use a repository over HTTPS):
```
sudo apt-get update
sudo apt-get install -y ca-certificates curl gnupg lsb-release
```

(2) Add *pdftotext++*'s official GPG key:
```
sudo mkdir -m 0755 -p /etc/apt/keyrings
curl -fsSL https://pdftotext.cs.uni-freiburg.de/download/apt/gpg | sudo gpg --dearmor -o /etc/apt/keyrings/pdftotext-plus-plus.gpg
```

(3) Add the repository and install *pdftotext++*:
```
echo "deb [arch=$(dpkg --print-architecture) signed-by=/etc/apt/keyrings/pdftotext-plus-plus.gpg] https://pdftotext.cs.uni-freiburg.de/download/apt $(lsb_release -cs) stable" | sudo tee /etc/apt/sources.list.d/pdftotext-plus-plus.list > /dev/null
sudo apt-get update
sudo apt-get install -y pdftotext++
```

(4) Run *pdftotext++* (type `pdftotext++ --help` to see the full usage information):
```
pdftotext++ [options] <pdf-file> <output-file>
```

### Docker

(1) Clone the project:
```
git clone git@github.com:ad-freiburg/pdftotext-plus-plus.git
cd pdftotext-plus-plus
```

(2) Build a Docker image:
```
docker build -f Dockerfiles/Dockerfile -t pdftotext-plus-plus .
```

(3) Create and run a Docker container:
```
docker run --rm -it -v <pdf-file>:/file.pdf --name pdftotext-plus-plus pdftotext-plus-plus [options] /file.pdf <output-file>
```

### Binary package

(1) Download the [latest binary](https://github.com/ad-freiburg/pdftotext-plus-plus/releases/latest) (for other versions, see the [release page](https://github.com/ad-freiburg/pdftotext-plus-plus/releases)).
```
wget https://github.com/ad-freiburg/pdftotext-plus-plus/releases/latest
```

(2) Install the binary and its dependencies (in the first of the following two commands, change the path to that of the binary you have just downloaded).
```
dpkg -i ./pdftotext-plus-plus_1.0.0-0focal_amd64.deb
sudo apt-get -f install
```

(3) Run *pdftotext++* (type `pdftotext++ --help` to see the full usage information):
```
pdftotext++ [options] <pdf-file> <output-file>
```

### Build from source

(1) Clone the project and run the install script:
```
git clone git@github.com:ad-freiburg/pdftotext-plus-plus.git
cd pdftotext-plus-plus
sudo make install
```

(2) Run *pdftotext++* (type `pdftotext++ --help` to see the full usage information):
```
pdftotext++ [options] <pdf-file> <output-file>
```