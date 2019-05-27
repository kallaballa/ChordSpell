#include "chord_spell.hpp"
#include <numeric>

char *find_matches(const char * txt, int *bl) {
  int i, j;
  char *res = (char*) malloc(256 * sizeof(char));
  char ctmp = 'a';
  for (i = 0; i < 256; i++) {
    res[i] = 0;
  }
  i = 0, j = 0;
  while (ctmp != 0) {
    ctmp = txt[i];
    if (bl[i] == true) {
      res[j] = ctmp;
      j++;
    }
    i++;
  }
  return res;
}

double jaro_winker_distance(const char *t1, const char *t2) {
  int maxGap, l1, l2, cntMatching, cntTransposition, lenPrefix, i, j;
  char *t1Match, *t2Match;
  int *b1, *b2;
  double distanceJaro;
  if (t1[0] == 0 || t2[0] == 0)
    return 0.0;
  l1 = strlen(t1);
  l2 = strlen(t2);
  maxGap = (std::max(l1, l2) / 2) - 1;
  cntMatching = 0;
  b1 = (int*) malloc((l1 + 2) * sizeof(int));
  b2 = (int*) malloc((l2 + 2) * sizeof(int));
  for (i = 0; i < l1; i++)
    b1[i] = false;
  for (i = 0; i < l2; i++)
    b2[i] = false;

  for (i = 0; i < l1; i++) {
    for (j = std::max(i - maxGap, 0); j <= std::min(i + maxGap, l2); j++) {
      if (t1[i] == t2[j]) {
        b1[i] = true;
        b2[j] = true;
        cntMatching++;
        break;
      }

    }

  }

  if (cntMatching == 0)
    return 0.0;

  t1Match = find_matches(t1, b1); //Génére la liste des caractères communs dans l'ordre de t1
  t2Match = find_matches(t2, b2);

  cntTransposition = 0;
  if (strcmp(t1Match, t2Match) != 0) {
    for (i = 0; i < (int) strlen(t1Match); i++)
      if (t1Match[i] != t2Match[i])
        cntTransposition++; //Calcul le nombre de transpositions
  } else
    cntTransposition = 0;

  free(b1);
  free(b2);
  free(t1Match);
  free(t2Match);

  distanceJaro = (((double) cntMatching / l1) + ((double) cntMatching / l2) + ((cntMatching - cntTransposition / 2.0) / cntMatching)) / 3.0;

  lenPrefix = 0;
  for (i = 0; i < std::min(3, std::min(l1, l2)) + 1; i++) //longueur max : 4
      {
    if (t1[i] == t2[i])
      lenPrefix++;
    else
      break;

  }
  return distanceJaro + (lenPrefix * 0.1 * (1 - distanceJaro));
}

template<typename T>
bool isBlankString(const T& s) {
  bool blank = true;
  for (auto& c : s) {
    if (!isblank(c)) {
      blank = false;
      break;
    }
  }
  return blank;
}

bool isSwapped(const string& a, const string& b) {
  if (a.size() != b.size())
    return false;

  string copy = b;
  for (const char& c : a) {
    if (copy.empty())
      return true;

    size_t i = copy.find(c);
    if (i == string::npos) {
      return false;
    } else {
      copy[i] = 'Z';
    }
  }
  return true;
}

int levenshtein_distance(const std::string &s1, const std::string &s2) {
  // To change the type this function manipulates and returns, change
  // the return type and the types of the two variables below.
  int s1len = s1.size();
  int s2len = s2.size();

  auto column_start = (decltype(s1len)) 1;

  auto column = new decltype(s1len)[s1len + 1];
  std::iota(column + column_start, column + s1len + 1, column_start);

  for (auto x = column_start; x <= s2len; x++) {
    column[0] = x;
    auto last_diagonal = x - column_start;
    for (auto y = column_start; y <= s1len; y++) {
      auto old_diagonal = column[y];
      auto possibilities = { column[y] + 1, column[y - 1] + 1, last_diagonal + (s1[y - 1] == s2[x - 1] ? 0 : 1) };
      column[y] = std::min(possibilities);
      last_diagonal = old_diagonal;
    }
  }
  auto result = column[s1len];
  delete[] column;
  return result;
}

ChordSpell::ChordSpell(const string& dictfilename) {
  string line;
  std::ifstream ifs(dictfilename);
  std::set<string> tmpdict;
  while (std::getline(ifs, line)) {
    if (!isBlankString(line)) {
      tmpdict.insert(line);
    }
  }
  std::copy(tmpdict.begin(), tmpdict.end(), std::back_inserter(dict));
  ifs.close();
}

string ChordSpell::fix(string chord) {
  if (isBlankString(chord)) {
    return "";
  }

  double min_dist = std::numeric_limits<double>().max();
  double min_swap_dist = std::numeric_limits<double>().min();
  double dist;
  double swap_dist;
  std::vector<string> suggestions;
  std::vector<string> swapsuggestions;

  for (auto& p : dict) {
    if (isSwapped(p, chord)) {
      swap_dist = jaro_winker_distance(p.c_str(), chord.c_str());
      if (swap_dist == min_swap_dist) {
        swapsuggestions.push_back(p);
      } else if (swap_dist > min_swap_dist) {
        swapsuggestions.clear();
        swapsuggestions.push_back(p);
        min_swap_dist = swap_dist;
      }
    } else if (tolower(p[0]) == tolower(chord[0])) {
      string first;
      string second;
      if (p.size() < chord.size()) {
        first = p;
        second = chord;
      } else {
        first = chord;
        second = p;
      }

      dist = levenshtein_distance(p, chord);
      if (dist == min_dist) {
        suggestions.push_back(p);
      } else if (dist < min_dist) {
        suggestions.clear();
        suggestions.push_back(p);
        min_dist = dist;
      }
    }
  }

  if (min_swap_dist != std::numeric_limits<double>().min()) {
    return swapsuggestions[0];
  } else if ((chord.size() - min_dist) > 2) {
    double jw = 0;
    double min_jw = std::numeric_limits<double>().min();
    string sug;
    for (const string& s : suggestions) {
      jw = jaro_winker_distance(s.c_str(), chord.c_str());
      if (jw > min_jw) {
        sug = s;
        min_jw = jw;
      }
    }
    if (jw < 0.9)
      return "";
    else
      return sug;
  } else {
    return "";
  }
}
