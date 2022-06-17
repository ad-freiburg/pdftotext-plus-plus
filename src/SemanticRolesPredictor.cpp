/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <codecvt>
#include <fstream>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "tensorflow/cc/client/client_session.h"

#include "./BytePairEncoder.h"
#include "./SemanticRolesPredictor.h"

using tensorflow::SessionOptions;
using tensorflow::RunOptions;
using tensorflow::Tensor;
using tensorflow::TensorShape;


// _________________________________________________________________________________________________
SemanticRolesPredictor::SemanticRolesPredictor() = default;

// _________________________________________________________________________________________________
SemanticRolesPredictor::~SemanticRolesPredictor() = default;

// _________________________________________________________________________________________________
void SemanticRolesPredictor::readModel() {
  // Disable the annoying log output of Tensorflow.
  char flag[] = "TF_CPP_MIN_LOG_LEVEL=3";
  putenv(flag);

  // Read the model.
  SessionOptions sessionOptions;
  RunOptions runOptions;
  auto status = LoadSavedModel(sessionOptions, runOptions, _modelDirPath, { "serve" }, &_bundle);
  if (!status.ok()) {
    throw std::invalid_argument("Could not load model \"" + _modelDirPath + "\"");
  }

  // -----------
  // Read the BPE vocabulary.

  std::wifstream bpeVocabFile(_bpeVocabFilePath);

  // Abort if the file can't be read.
  if (!bpeVocabFile.is_open()) {
    throw std::invalid_argument("Could not load vocab file \"" + _bpeVocabFilePath + "\"");
  }

  // Tell the wifstream that the file is encoded in UTF-8 and contains multi-byte characters.
  auto codecvt = new std::codecvt_utf8<wchar_t, 0x10ffff, std::consume_header>();
  bpeVocabFile.imbue(std::locale(bpeVocabFile.getloc(), codecvt));

  // Read the vocabulary line by line, where each line is of form <token>TAB<id>.
  std::wstring wline;
  while (true) {
    std::getline(bpeVocabFile, wline);

    // Abort if the end of file was reached.
    if (bpeVocabFile.eof()) { break; }

    // Split the line "<token>TAB<id>" into "<token>" and "<id>".
    size_t posTab = wline.find(L"\t");
    if (posTab == std::string::npos) { continue; }
    std::wstring token = wline.substr(0, posTab);
    int tokenId = std::stoi(wline.substr(posTab + 1));
    _bpeVocab.insert(std::make_pair(token, tokenId));
  }

  // -----------
  // Read the roles vocabulary.

  std::ifstream vocabFile(_rolesVocabFilePath);
  if (!vocabFile.is_open()) {
    throw std::invalid_argument("Could not load vocab file \"" + _rolesVocabFilePath + "\"");
  }
  std::string line;
  while (true) {
    std::getline(vocabFile, line);
    if (vocabFile.eof()) {
      break;
    }
    size_t posTab = line.find("\t");
    if (posTab == std::string::npos) { continue; }

    std::string roleName = line.substr(0, posTab);
    int roleId = std::stoi(line.substr(posTab + 1));
    _rolesVocab[roleId] = roleName;
  }

  bpeVocabFile.close();
  // No need to delete codecvt.
}

// _________________________________________________________________________________________________
void SemanticRolesPredictor::predict(PdfDocument* doc) {
  if (!_modelOk) {
    readModel();
  }

  tensorflow::Tensor layoutTensor = createLayoutInputTensor(doc);
  tensorflow::Tensor wordsTensor = createWordsInputTensor(doc);

  // Use the model to predict the semantic roles.
  std::vector<tensorflow::Tensor> outputs;
  tensorflow::Status run_status = _bundle.GetSession()->Run({
      { "serving_default_layout_features_input:0", layoutTensor },
      { "serving_default_words_input:0", wordsTensor }
  }, { "StatefulPartitionedCall:0" }, {}, &outputs);

  // For each block, determine the role with the highest probability.
  auto predictions = outputs[0].tensor<float, 2>();
  int blockIndex = 0;
  for (auto* page : doc->pages) {
    for (auto* block : page->blocks) {
      // For the block, find the role with the highest probability.
      float maxProb = 0;
      std::string maxSemanticRole;
      for (size_t roleNum = 0; roleNum < _rolesVocab.size(); roleNum++) {
        if (predictions(blockIndex, roleNum) > maxProb) {
          maxProb = predictions(blockIndex, roleNum);
          maxSemanticRole = _rolesVocab[roleNum];
        }
      }
      block->role = maxSemanticRole;

      // std::stringstream lv;
      // lv << "[";
      // for (int i = 0; i < 15; i++) {
      //   lv << layoutTensor.tensor<float, 2>()(blockIndex, i) << " ";
      // }
      // lv << "]";

      // std::cout << "Layout-Vector: " << lv.str() << std::endl;

      // std::stringstream wv;
      // wv << "[";
      // for (int i = 0; i < 100; i++) {
      //   wv << wordsTensor.tensor<int, 2>()(blockIndex, i) << " ";
      // }
      // wv << "]";

      // std::cout << "Words-Vector: " << wv.str() << std::endl;

      blockIndex++;
    }
  }
}

// _________________________________________________________________________________________________
tensorflow::Tensor SemanticRolesPredictor::createLayoutInputTensor(const PdfDocument* doc) {
  // Iterate through the text blocks of the document to count the total number of text blocks and
  // some document-wide statistics (e.g., the largest font size).
  float minFontSize = std::numeric_limits<float>::max();
  float maxFontSize = std::numeric_limits<float>::min();
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
  tensorflow::Tensor layoutTensor(tensorflow::DT_FLOAT, TensorShape({ numBlocks, 15 }));

  int blockIndex = 0;
  for (const auto* page : doc->pages) {
    for (const auto* block : page->blocks) {
      PdfFontInfo* fontInfo = doc->fontInfos.at(block->fontName);

      // Convert the block text to a wstring, to correctly handle umlauts etc.
      std::wstring wBlockText = stringConverter.from_bytes(block->text);

      // -----
      // Encode the page number.

      float pageNumEncoded = 0.0f;
      if (doc->pages.size() > 1) {
        // Normalize the 1-based page numbers.
        pageNumEncoded = static_cast<float>(block->position->pageNum - 1) /
                         static_cast<float>(doc->pages.size() - 1);
      }
      layoutTensor.tensor<float, 2>()(blockIndex, 0) = pageNumEncoded;

      // -----
      // Encode the x/y-coordinates. TODO: Reverse the coordinates.

      float leftX = block->position->leftX;
      float leftXencoded = page->getWidth() > 0 ? leftX / page->getWidth() : 0.0;
      layoutTensor.tensor<float, 2>()(blockIndex, 1) = leftXencoded;

      // The model expects the origin to be in the page's lower left.
      float upperY = page->getHeight() - block->position->lowerY;
      float upperYencoded = page->getHeight() > 0 ? upperY / page->getHeight() : 0.0;
      layoutTensor.tensor<float, 2>()(blockIndex, 2) = upperYencoded;

      float rightX = block->position->rightX;
      float rightXencoded = page->getWidth() > 0 ? rightX / page->getWidth() : 0.0;
      layoutTensor.tensor<float, 2>()(blockIndex, 3) = rightXencoded;

      // The model expects the origin to be in the page's lower left.
      float lowerY = page->getHeight() - block->position->upperY;
      float lowerYencoded = page->getHeight() > 0 ? lowerY / page->getHeight() : 0.0;
      layoutTensor.tensor<float, 2>()(blockIndex, 4) = lowerYencoded;

      // -----
      // Encode the font size.
      float fsEncoded = 0.0f;
      // Use the whole interval [0, 1], that is: translate the min font size to 0 and the max
      // font size to 1. For example, if the min font size in a document is 8 and the max font
      // size is 10, then translate font size 10 to: (10 - 8) / (12 - 8) = 0.5
      if (minFontSize < maxFontSize) {
        fsEncoded = (block->fontSize - minFontSize) / (maxFontSize - minFontSize);
      }
      layoutTensor.tensor<float, 2>()(blockIndex, 5) = fsEncoded;

      // -----
      // Encode the boldness. TODO: The trained model only accepts a flag, indicating whether or
      // not the block is bold. Consider to train the model with the font weight instead.

      bool isBoldEncoded = fontInfo->weight > 500 ? 1.0f : 0.0f;
      layoutTensor.tensor<float, 2>()(blockIndex, 6) = isBoldEncoded;

      // -----
      // Encode the italicness.

      bool isItalicEncoded = fontInfo->isItalic ? 1.0f : 0.0f;
      layoutTensor.tensor<float, 2>()(blockIndex, 7) = isItalicEncoded;

      // -----
      // Encode whether or not the block contains an "@".

      bool containsAtEncoded = wBlockText.find(L"@") != std::string::npos ? 1.0f : 0.0f;
      layoutTensor.tensor<float, 2>()(blockIndex, 8) = containsAtEncoded;

      // -----
      // Encode whether or not the block starts with a digit.

      bool isFirstCharDigitEnc = wBlockText.size() > 0 && iswdigit(wBlockText[0]) ? 1.0f : 0.0f;
      layoutTensor.tensor<float, 2>()(blockIndex, 9) = isFirstCharDigitEnc;

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
      layoutTensor.tensor<float, 2>()(blockIndex, 10) = percDigitsEncoded;

      // -----
      // Encode the percentage of non-ascii characters.

      float percNonAsciiCharsEncoded = 0.0f;
      if (numNonWhitespaceChars > 0) {
        percNonAsciiCharsEncoded = static_cast<float>(numNonAsciiChars) /
                                   static_cast<float>(numNonWhitespaceChars);
      }
      layoutTensor.tensor<float, 2>()(blockIndex, 11) = percNonAsciiCharsEncoded;

      // -----
      // Encode the percentage of punctuation characters.

      float percPunctuationCharsEncoded = 0.0f;
      if (numNonWhitespaceChars > 0) {
        percPunctuationCharsEncoded = static_cast<float>(numPunctuationChars) /
                                      static_cast<float>(numNonWhitespaceChars);
      }
      layoutTensor.tensor<float, 2>()(blockIndex, 12) = percPunctuationCharsEncoded;

      // -----
      // Encode the percentage of words with an uppercased first character.

      int numUppercasedWords = 0;
      int numWords = 0;
      for (const auto* line : block->lines) {
        for (const auto* word : line->words) {
          // Convert the block text to a wstring, to correctly handle umlauts etc.
          std::wstring wWordText = stringConverter.from_bytes(word->text);

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
      layoutTensor.tensor<float, 2>()(blockIndex, 13) = percUppercasedWordsEncoded;

      // -----
      // Encode the percentage of uppercased characters.

      float percUppercasedCharsEncoded = 0.0f;
      if (numNonWhitespaceChars > 0) {
        percUppercasedCharsEncoded = static_cast<float>(numUppercasedChars) /
                                     static_cast<float>(numNonWhitespaceChars);
      }
      layoutTensor.tensor<float, 2>()(blockIndex, 14) = percUppercasedCharsEncoded;

      // std::stringstream ss;
      // ss << "[";
      // for (int i = 0; i < 15; i++) {
      //   ss << layoutTensor.tensor<float, 2>()(blockIndex, i) << " ";
      // }
      // ss << "]";

      // // LOG(DEBUG) << "Computed layout input tensor: " << ss.str() << std::endl;
      // std::cout << "Block: " << block->text << std::endl;
      // std::string str = ss.str();
      // std::string wstr(str.begin(), str.end());
      // std::cout << "Layout features: " << wstr << std::endl;

      blockIndex++;
    }
  }

  return layoutTensor;
}

// _________________________________________________________________________________________________
tensorflow::Tensor SemanticRolesPredictor::createWordsInputTensor(const PdfDocument* doc) {
  // Iterate through the text blocks of the document to count the total number of text blocks.
  int numBlocks = 0;
  for (const auto* page : doc->pages) {
    numBlocks += page->blocks.size();
  }

  int length = 100;
  tensorflow::Tensor wordsTensor(tensorflow::DT_INT32, TensorShape({ numBlocks, length }));

  BytePairEncoder encoder(&_bpeVocab);

  int blockIndex = 0;
  for (const auto* page : doc->pages) {
    for (const auto* block : page->blocks) {
      // Convert the block text to a wstring, to correctly handle umlauts etc.
      std::wstring wBlockText = stringConverter.from_bytes(block->text);

      // Encode the text of each block using byte pair encoding.
      std::vector<int> encoding;
      encoder.encode(wBlockText, length, &encoding);
      for (size_t k = 0; k < encoding.size(); k++) {
        wordsTensor.tensor<int, 2>()(blockIndex, k) = encoding[k];
      }

      // std::stringstream ss;
      // ss << "[";
      // for (int i = 0; i < 100; i++) {
      //   ss << wordsTensor.tensor<int, 2>()(blockIndex, i) << " ";
      // }
      // ss << "]";
      // std::cout << "Block: " << block->text << std::endl;
      // std::string str = ss.str();
      // std::string wstr(str.begin(), str.end());
      // std::cout << "Word features: " << wstr << std::endl;

      blockIndex++;
    }
  }

  return wordsTensor;
}
