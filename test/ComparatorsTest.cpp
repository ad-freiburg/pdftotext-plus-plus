/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <gtest/gtest.h>

#include <algorithm>  // sort
#include <vector>

#include "../src/PdfDocument.h"
#include "../src/utils/Comparators.h"

// _________________________________________________________________________________________________

const PdfCharacter char01(1, 12.3, 14.6, 45.7, 15.6, 0, 0);
const PdfCharacter char02(1, 10.1, 13.5, 12.6, 16.8, 0, 0);
const PdfCharacter char03(1, 45.2, 10.5, 67.1, 17.8, 0, 0);

const PdfCharacter char11(1, 12.3, 14.6, 45.7, 15.6, 1, 0);
const PdfCharacter char12(1, 10.1, 13.5, 12.6, 16.8, 1, 0);
const PdfCharacter char13(1, 45.2, 10.5, 67.1, 17.8, 1, 0);

const PdfCharacter char21(1, 12.3, 14.6, 45.7, 15.6, 2, 0);
const PdfCharacter char22(1, 10.1, 13.5, 12.6, 16.8, 2, 0);
const PdfCharacter char23(1, 45.2, 10.5, 67.1, 17.8, 2, 0);

const PdfCharacter char31(1, 12.3, 14.6, 45.7, 15.6, 3, 0);
const PdfCharacter char32(1, 10.1, 13.5, 12.6, 16.8, 3, 0);
const PdfCharacter char33(1, 45.2, 10.5, 67.1, 17.8, 3, 0);

// _________________________________________________________________________________________________
TEST(Comparators, LeftXAscComparator) {
  // std::vector<PdfCharacter*> chars0 = { &char01, &char02, &char03 } ;
  // sort(chars0.begin(), chars0.end(), comparators::LeftXAscComparator());
  // ASSERT_EQ(chars0[0], &char02);
  // ASSERT_EQ(chars0[1], &char01);
  // ASSERT_EQ(chars0[2], &char03);
}

// _________________________________________________________________________________________________
TEST(Comparators, RotLeftXAscComparator) {
  // std::vector<PdfCharacter*> chars0 = { &char01, &char02, &char03 };
  // sort(chars0.begin(), chars0.end(), comparators::RotLeftXAscComparator());
  // ASSERT_EQ(chars0[0], &char02);
  // ASSERT_EQ(chars0[1], &char01);
  // ASSERT_EQ(chars0[2], &char03);

  // std::vector<PdfCharacter*> chars1 = { &char11, &char12, &char13 };
  // sort(chars1.begin(), chars1.end(), comparators::RotLeftXAscComparator());
  // ASSERT_EQ(chars1[0], &char13);
  // ASSERT_EQ(chars1[1], &char12);
  // ASSERT_EQ(chars1[2], &char11);

  // std::vector<PdfCharacter*> chars2 = { &char21, &char22, &char23 };
  // sort(chars2.begin(), chars2.end(), comparators::RotLeftXAscComparator());
  // ASSERT_EQ(chars2[0], &char22);
  // ASSERT_EQ(chars2[1], &char21);
  // ASSERT_EQ(chars2[2], &char23);

  // std::vector<PdfCharacter*> chars3 = { &char31, &char32, &char33 };
  // sort(chars3.begin(), chars3.end(), comparators::RotLeftXAscComparator());
  // ASSERT_EQ(chars3[0], &char31);
  // ASSERT_EQ(chars3[1], &char32);
  // ASSERT_EQ(chars3[2], &char33);
}

// _________________________________________________________________________________________________
TEST(Comparators, RotLeftXDescComparator) {
  // std::vector<PdfCharacter*> chars0 = { &char01, &char02, &char03 };
  // sort(chars0.begin(), chars0.end(), comparators::RotLeftXDescComparator());
  // ASSERT_EQ(chars0[0], &char03);
  // ASSERT_EQ(chars0[1], &char01);
  // ASSERT_EQ(chars0[2], &char02);

  // std::vector<PdfCharacter*> chars1 = { &char11, &char12, &char13 };
  // sort(chars1.begin(), chars1.end(), comparators::RotLeftXDescComparator());
  // ASSERT_EQ(chars1[0], &char11);
  // ASSERT_EQ(chars1[1], &char12);
  // ASSERT_EQ(chars1[2], &char13);

  // std::vector<PdfCharacter*> chars2 = { &char21, &char22, &char23 };
  // sort(chars2.begin(), chars2.end(), comparators::RotLeftXDescComparator());
  // ASSERT_EQ(chars2[0], &char23);
  // ASSERT_EQ(chars2[1], &char21);
  // ASSERT_EQ(chars2[2], &char22);

  // std::vector<PdfCharacter*> chars3 = { &char31, &char32, &char33 };
  // sort(chars3.begin(), chars3.end(), comparators::RotLeftXDescComparator());
  // ASSERT_EQ(chars3[0], &char33);
  // ASSERT_EQ(chars3[1], &char32);
  // ASSERT_EQ(chars3[2], &char31);
}

// _________________________________________________________________________________________________
TEST(Comparators, RightXAscComparator) {
  // std::vector<PdfCharacter*> chars0 = { &char01, &char02, &char03 };
  // sort(chars0.begin(), chars0.end(), comparators::RightXAscComparator());
  // ASSERT_EQ(chars0[0], &char02);
  // ASSERT_EQ(chars0[1], &char01);
  // ASSERT_EQ(chars0[2], &char03);
}

// _________________________________________________________________________________________________
TEST(Comparators, RightXDescComparator) {
  // std::vector<PdfCharacter*> chars0 = { &char01, &char02, &char03 };
  // sort(chars0.begin(), chars0.end(), comparators::RightXDescComparator());
  // ASSERT_EQ(chars0[0], &char03);
  // ASSERT_EQ(chars0[1], &char01);
  // ASSERT_EQ(chars0[2], &char02);
}

// _________________________________________________________________________________________________
TEST(Comparators, UpperYAscComparator) {
  // std::vector<PdfCharacter*> chars0 = { &char01, &char02, &char03 };
  // sort(chars0.begin(), chars0.end(), comparators::UpperYAscComparator());
  // ASSERT_EQ(chars0[0], &char03);
  // ASSERT_EQ(chars0[1], &char02);
  // ASSERT_EQ(chars0[2], &char01);
}

// _________________________________________________________________________________________________
TEST(Comparators, RotLowerYAscComparator) {
  // std::vector<PdfCharacter*> chars0 = { &char01, &char02, &char03 };
  // sort(chars0.begin(), chars0.end(), comparators::RotLowerYAscComparator());
  // ASSERT_EQ(chars0[0], &char01);
  // ASSERT_EQ(chars0[1], &char02);
  // ASSERT_EQ(chars0[2], &char03);

  // std::vector<PdfCharacter*> chars1 = { &char11, &char12, &char13 };
  // sort(chars1.begin(), chars1.end(), comparators::RotLowerYAscComparator());
  // ASSERT_EQ(chars1[0], &char12);
  // ASSERT_EQ(chars1[1], &char11);
  // ASSERT_EQ(chars1[2], &char13);

  // std::vector<PdfCharacter*> chars2 = { &char21, &char22, &char23 };
  // sort(chars2.begin(), chars2.end(), comparators::RotLowerYAscComparator());
  // ASSERT_EQ(chars2[0], &char23);
  // ASSERT_EQ(chars2[1], &char22);
  // ASSERT_EQ(chars2[2], &char21);

  // std::vector<PdfCharacter*> chars3 = { &char31, &char32, &char33 };
  // sort(chars3.begin(), chars3.end(), comparators::RotLowerYAscComparator());
  // ASSERT_EQ(chars3[0], &char32);
  // ASSERT_EQ(chars3[1], &char31);
  // ASSERT_EQ(chars3[2], &char33);
}

// _________________________________________________________________________________________________
TEST(Comparators, RotLowerYDescComparator) {
  // std::vector<PdfCharacter*> chars0 = { &char01, &char02, &char03 };
  // sort(chars0.begin(), chars0.end(), comparators::RotLowerYDescComparator());
  // ASSERT_EQ(chars0[0], &char03);
  // ASSERT_EQ(chars0[1], &char02);
  // ASSERT_EQ(chars0[2], &char01);

  // std::vector<PdfCharacter*> chars1 = { &char11, &char12, &char13 };
  // sort(chars1.begin(), chars1.end(), comparators::RotLowerYDescComparator());
  // ASSERT_EQ(chars1[0], &char13);
  // ASSERT_EQ(chars1[1], &char11);
  // ASSERT_EQ(chars1[2], &char12);

  // std::vector<PdfCharacter*> chars2 = { &char21, &char22, &char23 };
  // sort(chars2.begin(), chars2.end(), comparators::RotLowerYDescComparator());
  // ASSERT_EQ(chars2[0], &char21);
  // ASSERT_EQ(chars2[1], &char22);
  // ASSERT_EQ(chars2[2], &char23);

  // std::vector<PdfCharacter*> chars3 = { &char31, &char32, &char33 };
  // sort(chars3.begin(), chars3.end(), comparators::RotLowerYDescComparator());
  // ASSERT_EQ(chars3[0], &char33);
  // ASSERT_EQ(chars3[1], &char31);
  // ASSERT_EQ(chars3[2], &char32);
}
