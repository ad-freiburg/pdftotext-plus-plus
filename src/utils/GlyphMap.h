/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef GLYPHMAP_H_
#define GLYPHMAP_H_

#include <string>
#include <unordered_map>
#include <utility>  // make_pair

/**
 * A mapping of glyph names to their respective Unicode codepoints and string representations.
 * It is needed because for some glyphs, some PDFs only provide the glyph names but not the
 * respective Unicode character (usually due to a missing or broken encoding).
 */
const std::unordered_map<std::string, std::pair<unsigned int, std::string>> glyphMap = {
  { "summationdisplay", std::make_pair(0x2211, "\u2211") },
  { "summationssdisplay", std::make_pair(0x2211, "\u2211") },
  { "angbracketleft", std::make_pair(0x3008, "\u3008") },
  { "angbracketright", std::make_pair(0x3009, "\u3009") },
  { "circlecopyrt", std::make_pair(0x00A9, "\u00A9") },
  { "controlNULL", std::make_pair(0x0000, "\u0000") },
  { "angbracketleftbig", std::make_pair(0x2329, "\u2329") },
  { "angbracketleftBig", std::make_pair(0x2329, "\u2329") },
  { "angbracketleftbigg", std::make_pair(0x2329, "\u2329") },
  { "angbracketleftBigg", std::make_pair(0x2329, "\u2329") },
  { "angbracketrightBig", std::make_pair(0x232A, "\u232A") },
  { "angbracketrightbig", std::make_pair(0x232A, "\u232A") },
  { "angbracketrightBigg", std::make_pair(0x232A, "\u232A") },
  { "angbracketrightbigg", std::make_pair(0x232A, "\u232A") },
  { "arrowhookleft", std::make_pair(0x21AA, "\u21AA") },
  { "arrowhookright", std::make_pair(0x21A9, "\u21A9") },
  { "arrowlefttophalf", std::make_pair(0x21BC, "\u21BC") },
  { "arrowleftbothalf", std::make_pair(0x21BD, "\u21BD") },
  { "arrownortheast", std::make_pair(0x2197, "\u2197") },
  { "arrownorthwest", std::make_pair(0x2196, "\u2196") },
  { "arrowrighttophalf", std::make_pair(0x21C0, "\u21C0") },
  { "arrowrightbothalf", std::make_pair(0x21C1, "\u21C1") },
  { "arrowsoutheast", std::make_pair(0x2198, "\u2198") },
  { "arrowsouthwest", std::make_pair(0x2199, "\u2199") },
  { "backslashbig", std::make_pair(0x2216, "\u2216") },
  { "backslashBig", std::make_pair(0x2216, "\u2216") },
  { "backslashBigg", std::make_pair(0x2216, "\u2216") },
  { "backslashbigg", std::make_pair(0x2216, "\u2216") },
  { "bardbl", std::make_pair(0x2016, "\u2016") },
  { "bracehtipdownleft", std::make_pair(0xFE37, "\uFE37") },
  { "bracehtipdownright", std::make_pair(0xFE37, "\uFE37") },
  { "bracehtipupleft", std::make_pair(0xFE38, "\uFE38") },
  { "bracehtipupright", std::make_pair(0xFE38, "\uFE38") },
  { "braceleftBig", std::make_pair(0x007B, "\u007B") },
  { "braceleftbig", std::make_pair(0x007B, "\u007B") },
  { "braceleftbigg", std::make_pair(0x007B, "\u007B") },
  { "braceleftBigg", std::make_pair(0x007B, "\u007B") },
  { "bracerightBig", std::make_pair(0x007D, "\u007D") },
  { "bracerightbig", std::make_pair(0x007D, "\u007D") },
  { "bracerightbigg", std::make_pair(0x007D, "\u007D") },
  { "bracerightBigg", std::make_pair(0x007D, "\u007D") },
  { "bracketleftbig", std::make_pair(0x005B, "\u005B") },
  { "bracketleftBig", std::make_pair(0x005B, "\u005B") },
  { "bracketleftbigg", std::make_pair(0x005B, "\u005B") },
  { "bracketleftBigg", std::make_pair(0x005B, "\u005B") },
  { "bracketleftmath", std::make_pair(0x005B, "\u005B") },
  { "bracketrightBig", std::make_pair(0x005D, "\u005D") },
  { "bracketrightbig", std::make_pair(0x005D, "\u005D") },
  { "bracketrightbigg", std::make_pair(0x005D, "\u005D") },
  { "bracketrightBigg", std::make_pair(0x005D, "\u005D") },
  { "bracketrightmath", std::make_pair(0x005D, "\u005D") },
  { "ceilingleftbig", std::make_pair(0x2308, "\u2308") },
  { "ceilingleftBig", std::make_pair(0x2308, "\u2308") },
  { "ceilingleftBigg", std::make_pair(0x2308, "\u2308") },
  { "ceilingleftbigg", std::make_pair(0x2308, "\u2308") },
  { "ceilingrightbig", std::make_pair(0x2309, "\u2309") },
  { "ceilingrightBig", std::make_pair(0x2309, "\u2309") },
  { "ceilingrightbigg", std::make_pair(0x2309, "\u2309") },
  { "ceilingrightBigg", std::make_pair(0x2309, "\u2309") },
  { "circledotdisplay", std::make_pair(0x2299, "\u2299") },
  { "circledottext", std::make_pair(0x2299, "\u2299") },
  { "circlemultiplydisplay", std::make_pair(0x2297, "\u2297") },
  { "circlemultiplytext", std::make_pair(0x2297, "\u2297") },
  { "circleplusdisplay", std::make_pair(0x2295, "\u2295") },
  { "circleplustext", std::make_pair(0x2295, "\u2295") },
  { "contintegraldisplay", std::make_pair(0x222E, "\u222E") },
  { "contintegraltext", std::make_pair(0x222E, "\u222E") },
  { "coproductdisplay", std::make_pair(0x2210, "\u2210") },
  { "coproducttext", std::make_pair(0x2210, "\u2210") },
  { "epsilon1", std::make_pair(0x03B5, "\u03B5") },
  { "equalmath", std::make_pair(0x003D, "\u003D") },
  { "floorleftBig", std::make_pair(0x230A, "\u230A") },
  { "floorleftbig", std::make_pair(0x230A, "\u230A") },
  { "floorleftbigg", std::make_pair(0x230A, "\u230A") },
  { "floorleftBigg", std::make_pair(0x230A, "\u230A") },
  { "floorrightbig", std::make_pair(0x230B, "\u230B") },
  { "floorrightBig", std::make_pair(0x230B, "\u230B") },
  { "floorrightBigg", std::make_pair(0x230B, "\u230B") },
  { "floorrightbigg", std::make_pair(0x230B, "\u230B") },
  { "hatwide", std::make_pair(0x005e, "\u005e") },
  { "hatwider", std::make_pair(0x005e, "\u005e") },
  { "hatwidest", std::make_pair(0x005e, "\u005e") },
  { "intercal", std::make_pair(0x1D40, "\u1D40") },
  { "integraldisplay", std::make_pair(0x222B, "\u222B") },
  { "integraltext", std::make_pair(0x222B, "\u222B") },
  { "intersectiondisplay", std::make_pair(0x22C2, "\u22C2") },
  { "intersectiontext", std::make_pair(0x22C2, "\u22C2") },
  { "logicalanddisplay", std::make_pair(0x2227, "\u2227") },
  { "logicalandtext", std::make_pair(0x2227, "\u2227") },
  { "logicalordisplay", std::make_pair(0x2228, "\u2228") },
  { "logicalortext", std::make_pair(0x2228, "\u2228") },
  { "parenleftBig", std::make_pair(0x0028, "\u0028") },
  { "parenleftbig", std::make_pair(0x0028, "\u0028") },
  { "parenleftBigg", std::make_pair(0x0028, "\u0028") },
  { "parenleftbigg", std::make_pair(0x0028, "\u0028") },
  { "parenleftmath", std::make_pair(0x0028, "\u0028") },
  { "parenrightBig", std::make_pair(0x0029, "\u0029") },
  { "parenrightbig", std::make_pair(0x0029, "\u0029") },
  { "parenrightBigg", std::make_pair(0x0029, "\u0029") },
  { "parenrightbigg", std::make_pair(0x0029, "\u0029") },
  { "parenrightmath", std::make_pair(0x0029, "\u0029") },
  { "plusmath", std::make_pair(0x002B, "\u002B") },
  { "prime", std::make_pair(0x2032, "\u2032") },
  { "productdisplay", std::make_pair(0x220F, "\u220F") },
  { "producttext", std::make_pair(0x220F, "\u220F") },
  { "question_sign", std::make_pair(0x003F, "?") },
  { "radicalbig", std::make_pair(0x221A, "\u221A") },
  { "radicalBig", std::make_pair(0x221A, "\u221A") },
  { "radicalBigg", std::make_pair(0x221A, "\u221A") },
  { "radicalbigg", std::make_pair(0x221A, "\u221A") },
  { "radicalbt", std::make_pair(0x221A, "\u221A") },
  { "radicaltp", std::make_pair(0x221A, "\u221A") },
  { "radicalvertex", std::make_pair(0x221A, "\u221A") },
  { "slashbig", std::make_pair(0x002F, "\u002F") },
  { "slashBig", std::make_pair(0x002F, "\u002F") },
  { "slashBigg", std::make_pair(0x002F, "\u002F") },
  { "slashbigg", std::make_pair(0x002F, "\u002F") },
  { "summationdisplay", std::make_pair(0x2211, "\u2211") },
  { "summationtext", std::make_pair(0x2211, "\u2211") },
  { "thumbs_down", std::make_pair(0x1F44E, "\u1F44E") },
  { "thumbs_down_alt", std::make_pair(0x1F44E, "\u1F44E") },
  { "thumbs_up", std::make_pair(0x1F44D, "\u1F44D") },
  { "thumbs_up_alt", std::make_pair(0x1F44D, "\u1F44D") },
  { "tildewide", std::make_pair(0x02DC, "\u02DC") },
  { "tildewider", std::make_pair(0x02DC, "\u02DC") },
  { "tildewidest", std::make_pair(0x02DC, "\u02DC") },
  { "uniondisplay", std::make_pair(0x22C3, "\u22C3") },
  { "unionmultidisplay", std::make_pair(0x228E, "\u228E") },
  { "unionmultitext", std::make_pair(0x228E, "\u228E") },
  { "unionsqdisplay", std::make_pair(0x2294, "\u2294") },
  { "unionsqtext", std::make_pair(0x2294, "\u2294") },
  { "uniontext", std::make_pair(0x22C3, "\u22C3") },
  { "vextenddouble", std::make_pair(0x2225, "\u2225") },
  { "vextendsingle", std::make_pair(0x2223, "\u2223") }
};

#endif  // GLYPHMAP_H_
