/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <cppflow/cppflow.h>

#include <codecvt>  // std::codecvt_utf8
#include <fstream>  // std::wifstream
#include <limits>  // std::numeric_limits
#include <string>
#include <utility>  // std::make_pair
#include <vector>
#include <iostream>
#include <iomanip>

#include "./BytePairEncoder.h"
#include "./SemanticRolesPredictor.h"
#include "./Globals.h"

using std::numeric_limits;
using std::string;
using std::vector;
using std::wstring;

// _________________________________________________________________________________________________
SemanticRolesPredictor::SemanticRolesPredictor() = default;

// _________________________________________________________________________________________________
SemanticRolesPredictor::~SemanticRolesPredictor() {
  if (_modelOk) {
    delete _model;
  }
}

// _________________________________________________________________________________________________
void SemanticRolesPredictor::readModel() {
//   // Read the model.
//   SessionOptions sessionOptions;
//   RunOptions runOptions;
//   auto status = LoadSavedModel(sessionOptions, runOptions, _modelDirPath, { "serve" }, &_bundle);
//   if (!status.ok()) {
//     throw std::invalid_argument("Could not load model \"" + _modelDirPath + "\"");
//   }

  _model = new cppflow::model(globals->semanticRolesDetectionModelsDir);
  _modelOk = true;

  // -----------
  // Read the BPE vocabulary.

  // TODO(korzen): Parameterize the file name.
  string bpeVocabFilePath = globals->semanticRolesDetectionModelsDir + "/bpe-vocab.tsv";
  std::wifstream bpeVocabFile(bpeVocabFilePath);

  // Abort if the file can't be read.
  if (!bpeVocabFile.is_open()) {
    throw std::invalid_argument("Could not load vocab file \"" + bpeVocabFilePath + "\"");
  }

  // Tell the wifstream that the file is encoded in UTF-8 and contains multi-byte characters.
  auto codecvt = new std::codecvt_utf8<wchar_t, 0x10ffff, std::consume_header>();
  bpeVocabFile.imbue(std::locale(bpeVocabFile.getloc(), codecvt));

  // Read the vocabulary line by line, where each line is of form <token>TAB<id>.
  wstring wline;
  while (true) {
    std::getline(bpeVocabFile, wline);

    // Abort if the end of file was reached.
    if (bpeVocabFile.eof()) { break; }

    // Split the line "<token>TAB<id>" into "<token>" and "<id>".
    size_t posTab = wline.find(L"\t");
    if (posTab == string::npos) { continue; }
    wstring token = wline.substr(0, posTab);
    int tokenId = std::stoi(wline.substr(posTab + 1));
    _bpeVocab.insert(std::make_pair(token, tokenId));
  }

  // -----------
  // Read the roles vocabulary.

  // TODO(korzen): Parameterize the file name.
  string rolesVocabFilePath = globals->semanticRolesDetectionModelsDir + "/roles-vocab.tsv";
  std::ifstream vocabFile(rolesVocabFilePath);
  if (!vocabFile.is_open()) {
    throw std::invalid_argument("Could not load vocab file \"" + rolesVocabFilePath + "\"");
  }
  string line;
  while (true) {
    std::getline(vocabFile, line);
    if (vocabFile.eof()) {
      break;
    }
    size_t posTab = line.find("\t");
    if (posTab == string::npos) { continue; }

    string roleStr = line.substr(0, posTab);
    int roleId = std::stoi(line.substr(posTab + 1));
    _rolesVocab[roleId] = roleStr;
  }

  bpeVocabFile.close();
  // No need to delete codecvt.
}

// _________________________________________________________________________________________________
void SemanticRolesPredictor::predict(const PdfDocument* doc) {
  assert(doc);

  if (!_modelOk) {
    readModel();
  }

  cppflow::tensor layoutTensor = createLayoutInputTensor(doc);
  cppflow::tensor wordsTensor = createWordsInputTensor(doc);

  cppflow::tensor output = (*_model)({
    { "serving_default_layout_features_input:0", layoutTensor},
    { "serving_default_words_input:0", wordsTensor}
  }, { "StatefulPartitionedCall:0" })[0];

  std::vector<int64_t> shape = output.shape().get_data<int64_t>();
  assert(shape.size() == 2);
  int64_t yDim = shape[1];

  std::vector<float> outputData = output.get_data<float>();

  // For each block, determine the role with the highest probability.
  int blockIndex = 0;
  for (auto* page : doc->pages) {
    for (auto* block : page->blocks) {
      // For the block, find the role with the highest probability.
      float maxProb = 0;
      int64_t maxSemanticRoleNum = 0;
      for (int64_t roleNum = 0; roleNum < yDim; roleNum++) {
        if (outputData[blockIndex * yDim + roleNum] > maxProb) {
          maxProb = outputData[blockIndex * yDim + roleNum];
          maxSemanticRoleNum = roleNum;
        }
      }
      block->role = SemanticRole(maxSemanticRoleNum);
      blockIndex++;
    }
  }
}

// _________________________________________________________________________________________________
cppflow::tensor SemanticRolesPredictor::createLayoutInputTensor(const PdfDocument* doc) {
  assert(doc);

  // Iterate through the text blocks of the document to count the total number of text blocks and
  // some document-wide statistics (e.g., the largest font size).
  float minFontSize = numeric_limits<float>::max();
  float maxFontSize = numeric_limits<float>::min();
  int numBlocks = 0;
  for (const auto* page : doc->pages) {
    for (const auto* block : page->blocks) {
      if (block->fontSize > maxFontSize) {
        maxFontSize = block->fontSize;
      }

      if (block->fontSize < minFontSize) {
        minFontSize = block->fontSize;
      }

      numBlocks++;
    }
  }

  // Create the tensor.
  int xDim = numBlocks;
  int yDim = 15;
  std::vector<float> layoutTensorValues;
  layoutTensorValues.reserve(xDim * yDim);

  for (const auto* page : doc->pages) {
    for (const auto* block : page->blocks) {
      PdfFontInfo* fontInfo = doc->fontInfos.at(block->fontName);

      // Convert the block text to a wstring, to correctly handle umlauts etc.
      wstring wBlockText = stringConverter.from_bytes(block->text);

      // -----
      // Encode the page number.

      float pageNumEncoded = 0.0f;
      if (doc->pages.size() > 1) {
        // Normalize the 1-based page numbers.
        pageNumEncoded = static_cast<float>(block->pos->pageNum - 1) /
                         static_cast<float>(doc->pages.size() - 1);
      }
      layoutTensorValues.push_back(pageNumEncoded);

      // -----
      // Encode the x/y-coordinates. TODO: Reverse the coordinates.

      float leftX = block->pos->leftX;
      float leftXencoded = page->getWidth() > 0 ? leftX / page->getWidth() : 0.0;
      layoutTensorValues.push_back(leftXencoded);

      // The model expects the origin to be in the page's lower left.
      float upperY = page->getHeight() - block->pos->lowerY;
      float upperYencoded = page->getHeight() > 0 ? upperY / page->getHeight() : 0.0;
      layoutTensorValues.push_back(upperYencoded);

      float rightX = block->pos->rightX;
      float rightXencoded = page->getWidth() > 0 ? rightX / page->getWidth() : 0.0;
      layoutTensorValues.push_back(rightXencoded);

      // The model expects the origin to be in the page's lower left.
      float lowerY = page->getHeight() - block->pos->upperY;
      float lowerYencoded = page->getHeight() > 0 ? lowerY / page->getHeight() : 0.0;
      layoutTensorValues.push_back(lowerYencoded);

      // -----
      // Encode the font size.
      float fsEncoded = 0.0f;
      // Use the whole interval [0, 1], that is: translate the min font size to 0 and the max
      // font size to 1. For example, if the min font size in a document is 8 and the max font
      // size is 10, then translate font size 10 to: (10 - 8) / (12 - 8) = 0.5
      if (minFontSize < maxFontSize) {
        fsEncoded = (block->fontSize - minFontSize) / (maxFontSize - minFontSize);
      }
      layoutTensorValues.push_back(fsEncoded);

      // -----
      // Encode the boldness. TODO: The trained model only accepts a flag, indicating whether or
      // not the block is bold. Consider to train the model with the font weight instead.

      float isBoldEncoded = fontInfo->weight > 500 ? 1.0f : 0.0f;
      layoutTensorValues.push_back(isBoldEncoded);

      // -----
      // Encode the italicness.

      float isItalicEncoded = fontInfo->isItalic ? 1.0f : 0.0f;
      layoutTensorValues.push_back(isItalicEncoded);

      // -----
      // Encode whether or not the block contains an "@".

      float containsAtEncoded = wBlockText.find(L"@") != string::npos ? 1.0f : 0.0f;
      layoutTensorValues.push_back(containsAtEncoded);

      // -----
      // Encode whether or not the block starts with a digit.

      float isFirstCharDigitEnc = wBlockText.size() > 0 && iswdigit(wBlockText[0]) ? 1.0f : 0.0f;
      layoutTensorValues.push_back(isFirstCharDigitEnc);

      // Compute some statistics about the text.
      int numDigits = 0;
      int numUppercasedChars = 0;
      int numNonAsciiChars = 0;
      int numPunctuationChars = 0;
      int numNonWhitespaceChars = 0;
      for (size_t i = 0; i < wBlockText.size(); i++) {
        if (iswdigit(wBlockText[i])) {
          numDigits += 1;
        }

        if (iswupper(wBlockText[i])) {
          numUppercasedChars += 1;
        }

        if (static_cast<unsigned char>(wBlockText[i]) > 127) {
          numNonAsciiChars += 1;
        }

        if (iswpunct(wBlockText[i])) {
          numPunctuationChars += 1;
        }

        if (!iswspace(wBlockText[i])) {
          numNonWhitespaceChars += 1;
        }
      }

      // -----
      // Encode the percentage of digits in the text block.

      float percDigitsEncoded = 0.0f;
      if (numNonWhitespaceChars > 0) {
        percDigitsEncoded = static_cast<float>(numDigits) /
                            static_cast<float>(numNonWhitespaceChars);
      }
      layoutTensorValues.push_back(percDigitsEncoded);

      // -----
      // Encode the percentage of non-ascii characters.

      float percNonAsciiCharsEncoded = 0.0f;
      if (numNonWhitespaceChars > 0) {
        percNonAsciiCharsEncoded = static_cast<float>(numNonAsciiChars) /
                                   static_cast<float>(numNonWhitespaceChars);
      }
      layoutTensorValues.push_back(percNonAsciiCharsEncoded);

      // -----
      // Encode the percentage of punctuation characters.

      float percPunctuationCharsEncoded = 0.0f;
      if (numNonWhitespaceChars > 0) {
        percPunctuationCharsEncoded = static_cast<float>(numPunctuationChars) /
                                      static_cast<float>(numNonWhitespaceChars);
      }
      layoutTensorValues.push_back(percPunctuationCharsEncoded);

      // -----
      // Encode the percentage of words with an uppercased first character.

      int numUppercasedWords = 0;
      int numWords = 0;
      for (const auto* line : block->lines) {
        for (const auto* word : line->words) {
          // Convert the block text to a wstring, to correctly handle umlauts etc.
          wstring wWordText = stringConverter.from_bytes(word->text);

          if (wWordText.length() > 0 && isupper(wWordText[0])) {
            numUppercasedWords += 1;
          }
          numWords++;
        }
      }
      float percUppercasedWordsEncoded = 0.0f;
      if (numWords > 0) {
        percUppercasedWordsEncoded = static_cast<float>(numUppercasedWords) /
                                     static_cast<float>(numWords);
      }
      layoutTensorValues.push_back(percUppercasedWordsEncoded);

      // -----
      // Encode the percentage of uppercased characters.

      float percUppercasedCharsEncoded = 0.0f;
      if (numNonWhitespaceChars > 0) {
        percUppercasedCharsEncoded = static_cast<float>(numUppercasedChars) /
                                     static_cast<float>(numNonWhitespaceChars);
      }
      layoutTensorValues.push_back(percUppercasedCharsEncoded);
    }
  }

  cppflow::tensor layoutTensor = cppflow::tensor(layoutTensorValues, { xDim, yDim });
  return layoutTensor;
}

// _________________________________________________________________________________________________
cppflow::tensor SemanticRolesPredictor::createWordsInputTensor(const PdfDocument* doc) {
  assert(doc);

  // Iterate through the text blocks of the document to count the total number of text blocks.
  int numBlocks = 0;
  for (const auto* page : doc->pages) {
    numBlocks += page->blocks.size();
  }

  BytePairEncoder encoder(&_bpeVocab);

  int xDim = numBlocks;
  int yDim = 100;
  std::vector<int> wordsTensorValues;
  wordsTensorValues.reserve(xDim * yDim);

  for (const auto* page : doc->pages) {
    for (const auto* block : page->blocks) {
      // Convert the block text to a wstring, to correctly handle umlauts etc.
      wstring wBlockText = stringConverter.from_bytes(block->text);

      // Encode the text of each block using byte pair encoding.
      vector<int> encoding;
      encoder.encode(wBlockText, yDim, &encoding);
      for (int k = 0; k < yDim; k++) {
        wordsTensorValues.push_back(encoding[k]);
      }
    }
  }

  cppflow::tensor wordsTensor = cppflow::tensor(wordsTensorValues, { xDim, yDim });
  return wordsTensor;
}
