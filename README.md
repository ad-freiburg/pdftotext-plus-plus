<br/>
<div align="center">
  <a href="https://github.com/pdftotext-plus-plus/pdftotext-plus-plus">
    <img src="logo.png" alt="logo" width="80" height="80">
  </a>

  <h3 align="center">pdftotext++</h3>

  <p align="center">
    An extension of <a href="https://github.com/pdftotext-plus-plus/pdftotext-plus-plus">Poppler's pdftotext</a> that converts PDF files to more <br/>accurate plain text and provides many other features you always missed.
    <br/>
    <a href="https://pdftotext.cs.uni-freiburg.de"><strong>Explore the docs »</strong></a>
    <br/>
    <br/>
    <a href="https://pdftotext.cs.uni-freiburg.de">View Demo</a>
    ·
    <a href="https://github.com/pdftotext-plus-plus/pdftotext-plus-plus/issues">Report Bug</a>
    ·
    <a href="https://github.com/pdftotext-plus-plus/pdftotext-plus-plus/issues">Request Feature</a>
  </p>
  <br/>
</div>

<!-- =========================================================================================== -->

## What's wrong with the classic *pdftotext* ?

Absolutely nothing.

## What are the main features of *pdftotext++* ?

* Translation of ligatures and characters with diacritic marks.
* Word Boundary Detection.
* Text Block Boundary Detection.
* Semantic Roles Detection.
* Reading Order Detection.
* Paragraph Detection.
* Word dehyphenation.
* Detection of sub- and superscripts.
* Serialization to different formats (plain text, xml, json, etc.)

## How can I use it?

We recommend to use the provided Docker setup. The usage is as follows:

```bash
git clone https://github.com/pdftotext-plus-plus/pdftotext-plus-plus.git
cd pdftotext-plus-plus.git
docker build -t pdftotext-plus-plus .
docker run --rm -v <path-to-pdf>:/file.pdf pdftotext-plus-plus /file.pdf
```