#include <iostream>

#include <csgparser.hpp>

using namespace json11;

void printHelp() {

  std::cout << "Usage: csg2json <input_file> <output_file>\n"
               "  csg2json converts CSG files to CSGJS and vice versa.\n"
               "  Example:\n"
               "    csg2json input.csg output.csgjs\n";
}

int main (int argc, char ** argv) {

  if (argc != 3) {
    printHelp();
    return 0;
  }

  Json aData = csg::Parser::parse (argv[1]);

  csg::Parser::writeJSON (aData, argv[2]);

  csg::Parser::write (aData, "");
 
  return 0;
}
