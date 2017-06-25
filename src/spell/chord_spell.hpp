#ifndef SRC_CHORD_SPELL_HPP_
#define SRC_CHORD_SPELL_HPP_

#include <iostream>
#include <set>
#include <algorithm>
#include <fstream>
#include <string>
#include <sstream>
#include <limits>
#include <queue>

using std::string;
using std::cerr;
using std::endl;
using std::vector;
using std::queue;
using std::ifstream;
using std::ofstream;

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


class ChordSpell {
  std::vector<string> dict;
public:
  ChordSpell(const string& dictfilename);
  string fix(string chord);
};

#endif
