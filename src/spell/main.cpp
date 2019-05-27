#include "chord_spell.hpp"
#include <boost/locale.hpp>
#include <fstream>
#include <string>
#include <sstream>
#include <locale>
#include <codecvt>
#include <mutex>
#include <boost/program_options.hpp>
namespace po = boost::program_options;

using std::string;
using std::wstring;
using std::cerr;
using std::endl;
using std::vector;
using std::locale;
using std::wifstream;
using std::wofstream;

bool isBlankString(wstring s) {
  bool blank = true;
  for(auto& c : s) {
    if(!isblank(c)) {
      blank = false;
      break;
    }
  }
  return blank;
}

int main(int argc, char** argv) {
  //command line parsing
  string inputFile;
  string chordDictFile = "chorddict.txt";
  string overrideDictFile = "chordoverridedict.txt";

  po::options_description genericDesc("Options");
  genericDesc.add_options()("chord-dict,c", po::value<string>(&chordDictFile)->default_value(chordDictFile), "The chord dictioary to use")("override-dict,o",
      po::value<string>(&overrideDictFile)->default_value(overrideDictFile), "The override chord dictioary to use")("help,h", "Produce help message");

  po::options_description hidden("Hidden options");
  hidden.add_options()("inputFile", po::value<string>(&inputFile), "inputFile");

  po::options_description cmdline_options;
  cmdline_options.add(genericDesc).add(hidden);

  po::options_description visible;
  visible.add(genericDesc);

  po::positional_options_description p;
  p.add("inputFile", -1);

  po::variables_map vm;
  po::store(po::command_line_parser(argc, argv).options(cmdline_options).positional(p).run(), vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cerr << "Usage: chord_spell [options] <file>..." << std::endl;
    std::cerr << visible;
    return 0;
  }

  locale::global(locale("en_US.UTF-8"));
  std::locale loc = boost::locale::generator().generate("en_US.UTF-8");
  //std::wcin.imbue(loc);
  std::wcout.imbue(loc);

  ChordSpell spellChord(chordDictFile);
  ChordSpell spellOverride(overrideDictFile);
  wstring line;
  std::set<wstring> chordDict;
  std::wifstream wifsChords(chordDictFile);
  if(!wifsChords) {
    std::cerr << "Missing chord dictionary" << std::endl;
    exit(2);
  }

  while (std::getline(wifsChords, line)) {
    if (!isBlankString(line)) {
      chordDict.insert(line);
    }
  }
  wifsChords.close();

  std::set<wstring> overrideDict;
  std::wifstream wifsOverride(overrideDictFile);
  if(!wifsOverride) {
    std::cerr << "Missing override chord dictionary" << std::endl;
    exit(2);
  }
  while (std::getline(wifsOverride, line)) {
    if (!isBlankString(line)) {
      overrideDict.insert(line);
    }
  }
  wifsOverride.close();

  std::wifstream wifsIn(inputFile);
  if(!wifsIn) {
    std::cerr << "Missing input file" << std::endl;
    exit(2);
  }

  long brokenScore = 0;
  long fixScore = 0;
  long correctScore = 0;

  while (std::getline(wifsIn, line)) {
    if (isBlankString(line)) {
      continue;
    }

    bool isOverride = line.find(L'/') != wstring::npos;

    if(isOverride) {
      const auto& result = overrideDict.find(line);
      if(result != overrideDict.end()) {
        std::wcout << line << std::endl;
        ++correctScore;
        continue;
      }
    } else {
      const auto& result = chordDict.find(line);
      if(result != chordDict.end()) {
        std::wcout << line << std::endl;
        ++correctScore;
        continue;
      }
    }

    typedef std::codecvt_utf8<wchar_t> convert_type;
    std::wstring_convert<convert_type, wchar_t> converter;

    //use converter (.to_bytes: wstr->str, .from_bytes: str->wstr)
    std::string brokenChord = converter.to_bytes(line);

    if(isOverride) {
      brokenChord = spellOverride.fix(brokenChord);
    } else {
      brokenChord = spellChord.fix(brokenChord);
    }
    if(!brokenChord.empty()) {
      std::cout << brokenChord << std::endl;
      ++fixScore;
    } else {
      std::wcerr << L"Can't fix: " << line << std::endl;
      ++brokenScore;
    }
  }

  std::wcerr << L"Fixed Chords: " << fixScore << std::endl;
  std::wcerr << L"Unfixable chords: " << brokenScore << std::endl;
  std::wcerr << L"Correct Chords: " << correctScore << std::endl;
}
