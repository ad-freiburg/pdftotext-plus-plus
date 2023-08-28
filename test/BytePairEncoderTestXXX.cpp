// /**
//  * Copyright 2023, University of Freiburg,
//  * Chair of Algorithms and Data Structures.
//  * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
//  *
//  * Modified under the Poppler project - http://poppler.freedesktop.org
//  */

// #include <codecvt>
// #include <fstream>
// #include <gtest/gtest.h>
// #include <locale>
// #include <string>
// #include <unordered_map>
// #include <unordered_set>
// #include <utility>
// #include <vector>

// #include "./BytePairEncoder.h"


// // _____________________________________________________________________________________________
// void readVocabularyFromFile(const std::string& path, std::unordered_map<std::wstring, int>*
//  res) {
//   std::wifstream vocabFile(path);

//   // Abort if the file can't be read.
//   if (!vocabFile.is_open()) {
//     return;
//   }

//   // Tell the wifstream that the file is encoded in UTF-8 and contains multi-byte characters.
//   auto codecvt = new std::codecvt_utf8<wchar_t, 0x10ffff, std::consume_header>();
//   vocabFile.imbue(std::locale(vocabFile.getloc(), codecvt));

//   // Read the vocabulary line by line, where each line is of form <token>TAB<id>.
//   std::wstring line;
//   while (true) {
//     std::getline(vocabFile, line);

//     // Abort if the end of file was reached.
//     if (vocabFile.eof()) {
//       break;
//     }

//     // Split the line "<token>TAB<id>" into "<token>" and "<id>".
//     size_t posTab = line.find(L"\t");
//     if (posTab == std::string::npos) {
//       continue;
//     }
//     std::wstring token = line.substr(0, posTab);
//     int tokenId = std::stoi(line.substr(posTab + 1));
//     res->insert(std::make_pair(token, tokenId));
//   }

//   delete codecvt;
// }

// // _____________________________________________________________________________________________
// TEST(BytePairEncoderTest, testConstructor) {
//   std::unordered_map<std::wstring, int> vocabulary;
//   vocabulary[L"effi"] = 0;
//   vocabulary[L"cient"] = 1;

//   BytePairEncoder encoder(vocabulary);

//   ASSERT_EQ(5, encoder._vocabulary.size());
//   ASSERT_EQ(0, encoder._vocabulary[L"effi"]);
//   ASSERT_EQ(1, encoder._vocabulary[L"cient"]);
//   ASSERT_EQ(2, encoder._vocabulary[encoder.PADDING_SYMBOL]);
//   ASSERT_EQ(3, encoder._vocabulary[encoder.UNKNOWN_CHAR_SYMBOL]);
//   ASSERT_EQ(4, encoder._vocabulary[encoder.WORD_DELIM_SYMBOL]);
//   ASSERT_EQ(0, encoder._encodings_cache.size());
// }

// // ______________________________________________________________________________________________
// TEST(BytePairEncoderTest, testEncode) {
//   std::unordered_map<std::wstring, int> vocabulary;
//   readVocabularyFromFile("./vocab-bpe.test.txt", &vocabulary);

//   BytePairEncoder encoder(vocabulary);

//   // Test encoding an empty string.
//   std::vector<int> result1;
//   encoder.encode(L"", 5, &result1);
//   ASSERT_EQ(5, result1.size());
//   ASSERT_EQ(281, result1[0]);
//   ASSERT_EQ(281, result1[1]);
//   ASSERT_EQ(281, result1[2]);
//   ASSERT_EQ(281, result1[3]);
//   ASSERT_EQ(281, result1[4]);

//   // Test encoding a string with a single word.
//   std::vector<int> result2;
//   encoder.encode(L"computer", 7, &result2);
//   ASSERT_EQ(7, result2.size());
//   ASSERT_EQ(270, result2[0]);
//   ASSERT_EQ(79, result2[1]);
//   ASSERT_EQ(84, result2[2]);
//   ASSERT_EQ(83, result2[3]);
//   ASSERT_EQ(258, result2[4]);
//   ASSERT_EQ(281, result2[5]);
//   ASSERT_EQ(281, result2[6]);

//   // Test encoding a string with two words.
//   std::vector<int> result3;
//   encoder.encode(L"computer Trash", 10, &result3);
//   ASSERT_EQ(10, result3.size());
//   ASSERT_EQ(270, result3[0]);
//   ASSERT_EQ(79, result3[1]);
//   ASSERT_EQ(84, result3[2]);
//   ASSERT_EQ(83, result3[3]);
//   ASSERT_EQ(258, result3[4]);
//   ASSERT_EQ(51, result3[5]);
//   ASSERT_EQ(81, result3[6]);
//   ASSERT_EQ(64, result3[7]);
//   ASSERT_EQ(82, result3[8]);
//   ASSERT_EQ(71, result3[9]);

//   // Test encoding a string with three words and a different target length.
//   std::vector<int> result4;
//   encoder.encode(L"computer Trash killer", 6, &result4);
//   ASSERT_EQ(6, result4.size());
//   ASSERT_EQ(270, result4[0]);
//   ASSERT_EQ(79, result4[1]);
//   ASSERT_EQ(84, result4[2]);
//   ASSERT_EQ(83, result4[3]);
//   ASSERT_EQ(258, result4[4]);
//   ASSERT_EQ(51, result4[5]);
// }

// // ______________________________________________________________________________________________
// TEST(BytePairEncoderTest, testEncodeWord) {
//   std::unordered_map<std::wstring, int> vocabulary;
//   readVocabularyFromFile("./vocab-bpe.test.txt", &vocabulary);

//   BytePairEncoder encoder(vocabulary);

//   // Test encoding an empty word.
//   std::vector<int> result1;
//   encoder.encodeWord(L"", &result1);
//   ASSERT_EQ(0, result1.size());

//   // Test encoding a lowercase word.
//   std::vector<int> result2;
//   encoder.encodeWord(L"computer", &result2);
//   ASSERT_EQ(5, result2.size());
//   ASSERT_EQ(270, result2[0]);
//   ASSERT_EQ(79, result2[1]);
//   ASSERT_EQ(84, result2[2]);
//   ASSERT_EQ(83, result2[3]);
//   ASSERT_EQ(257, result2[4]);

//   // Test encoding a lowercase word with a word delimiter.
//   std::vector<int> result3;
//   encoder.encodeWord(L"computer✂", &result3);
//   ASSERT_EQ(5, result3.size());
//   ASSERT_EQ(270, result3[0]);
//   ASSERT_EQ(79, result3[1]);
//   ASSERT_EQ(84, result3[2]);
//   ASSERT_EQ(83, result3[3]);
//   ASSERT_EQ(258, result3[4]);

//   // Test encoding an uppercase word.
//   std::vector<int> result4;
//   encoder.encodeWord(L"Trash", &result4);
//   ASSERT_EQ(5, result4.size());
//   ASSERT_EQ(51, result4[0]);
//   ASSERT_EQ(81, result4[1]);
//   ASSERT_EQ(64, result4[2]);
//   ASSERT_EQ(82, result4[3]);
//   ASSERT_EQ(71, result4[4]);

//   // Test encoding another lowercase word.
//   std::vector<int> result5;
//   encoder.encodeWord(L"killer", &result5);
//   ASSERT_EQ(4, result5.size());
//   ASSERT_EQ(74, result5[0]);
//   ASSERT_EQ(72, result5[1]);
//   ASSERT_EQ(256, result5[2]);
//   ASSERT_EQ(257, result5[3]);

//   // Test encoding another uppercase word.
//   std::vector<int> result6;
//   encoder.encodeWord(L"September", &result6);
//   ASSERT_EQ(2, result6.size());
//   ASSERT_EQ(278, result6[0]);
//   ASSERT_EQ(257, result6[1]);

//   // Test encoding another lowercase word.
//   std::vector<int> result7;
//   encoder.encodeWord(L"september", &result7);
//   ASSERT_EQ(8, result7.size());
//   ASSERT_EQ(82, result7[0]);
//   ASSERT_EQ(68, result7[1]);
//   ASSERT_EQ(79, result7[2]);
//   ASSERT_EQ(83, result7[3]);
//   ASSERT_EQ(68, result7[4]);
//   ASSERT_EQ(76, result7[5]);
//   ASSERT_EQ(65, result7[6]);
//   ASSERT_EQ(257, result7[7]);

//   // Test encoding another uppercase word with a word delimiter.
//   std::vector<int> result8;
//   encoder.encodeWord(L"September✂", &result8);
//   ASSERT_EQ(1, result8.size());
//   ASSERT_EQ(279, result8[0]);
// }

// // ______________________________________________________________________________________________
// TEST(BytePairEncoderTest, computeTokenPairPositions) {
//   // ---------
//   // Test computing the token pair positions from an empty list.
//   std::vector<std::wstring> tokens1 = {};
//   std::vector<std::pair<std::wstring, std::unordered_set<size_t>* >* > result1;
//   BytePairEncoder::computeTokenPairPositions(tokens1, &result1);
//   ASSERT_EQ(0, result1.size());

//   // ---------
//   // Test computing the token pair positions from a list with a single, empty token.
//   std::vector<std::wstring> tokens2 = {L""};
//   std::vector<std::pair<std::wstring, std::unordered_set<size_t>* >* > result2;
//   BytePairEncoder::computeTokenPairPositions(tokens2, &result2);
//   ASSERT_EQ(0, result2.size());

//   // ---------
//   // Test computing the token pair positions from a list with one-character tokens.
//   std::vector<std::wstring> tokens3 = {L"f", L"o", L"x", L"i", L"f", L"o", L"x"};
//   std::vector<std::pair<std::wstring, std::unordered_set<size_t>* >* > result3;
//   BytePairEncoder::computeTokenPairPositions(tokens3, &result3);

//   // The expected result is: [("fo", {0, 4}), ("ox", {1, 5}), ("xi", {2}), ("if", {3})].
//   ASSERT_EQ(4, result3.size());

//   // Test the ("fo", {0, 4}) part.
//   std::wstring token = result3[0]->first;
//   std::unordered_set<size_t>* positions = result3[0]->second;
//   ASSERT_EQ(L"fo", token);
//   ASSERT_EQ(2, positions->size());
//   ASSERT_TRUE(positions->count(0));
//   ASSERT_TRUE(positions->count(4));

//   // Test the ("ox", {1, 5}) part.
//   token = result3[1]->first;
//   positions = result3[1]->second;
//   ASSERT_EQ(L"ox", token);
//   ASSERT_EQ(2, positions->size());
//   ASSERT_TRUE(positions->count(1));
//   ASSERT_TRUE(positions->count(5));

//   // Test the ("xi", {2}) part.
//   token = result3[2]->first;
//   positions = result3[2]->second;
//   ASSERT_EQ(L"xi", token);
//   ASSERT_EQ(1, positions->size());
//   ASSERT_TRUE(positions->count(2));

//   // Test the ("if", {3}) part.
//   token = result3[3]->first;
//   positions = result3[3]->second;
//   ASSERT_EQ(L"if", token);
//   ASSERT_EQ(1, positions->size());
//   ASSERT_TRUE(positions->count(3));

//   // ---------
//   // Test computing the token pair positions from a list with various-length tokens.
//   std::vector<std::wstring> tokens4 = {L"fo", L"x", L"if", L"ox", L"i", L"fox"};
//   std::vector<std::pair<std::wstring, std::unordered_set<size_t>* >* > result4;
//   BytePairEncoder::computeTokenPairPositions(tokens4, &result4);

//   // The expected result is: [("fox", {0}), ("xif", {1}), ("ifox", {2, 4}), ("oxi", {3})].
//   ASSERT_EQ(4, result4.size());

//   // Test the ("fox", {0}) part.
//   token = result4[0]->first;
//   positions = result4[0]->second;
//   ASSERT_EQ(L"fox", token);
//   ASSERT_EQ(1, positions->size());
//   ASSERT_TRUE(positions->count(0));

//   // Test the ("xif", {1}) part.
//   token = result4[1]->first;
//   positions = result4[1]->second;
//   ASSERT_EQ(L"xif", token);
//   ASSERT_EQ(1, positions->size());
//   ASSERT_TRUE(positions->count(1));

//   // Test the ("ifox", {2, 4}) part.
//   token = result4[2]->first;
//   positions = result4[2]->second;
//   ASSERT_EQ(L"ifox", token);
//   ASSERT_EQ(2, positions->size());
//   ASSERT_TRUE(positions->count(2));
//   ASSERT_TRUE(positions->count(4));

//   // Test the ("oxi", {3}) part.
//   token = result4[3]->first;
//   positions = result4[3]->second;
//   ASSERT_EQ(L"oxi", token);
//   ASSERT_EQ(1, positions->size());
//   ASSERT_TRUE(positions->count(3));
// }
