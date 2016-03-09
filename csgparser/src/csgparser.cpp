#include <iostream>
#include <fstream>
#include <cmath>
#include <list>

#include <json11/json11.hpp>
#include <parser/parser.h>

#include <csgparser.hpp>

using namespace lars;
using namespace json11;

namespace csg {

class CsgVisitor{  
  
public:

  Json getValue () {

    return m_value;
  }

  Json getValue (expression<CsgVisitor> e) {

    e.accept (this);
    return m_value;
  }

  std::pair<std::string, Json> getArgument (expression<CsgVisitor> e) {

    return std::make_pair (e[0].string(), getValue (e[1]));
  }

  void visitNumber (expression<CsgVisitor> e) {

    m_value = stod (e.string());
  }

  void visitBoolean (expression<CsgVisitor> e) {

    m_value = e.string() == "true";
  }

  void visitString (expression<CsgVisitor> e) {

    m_value = e.string();
  }

  void visitArguments (expression<CsgVisitor> e) {

    Json::object anArgs;

    for (int i = 0; i < e.size(); ++i) {
      auto anArg = getArgument (e[i]);
      anArgs[anArg.first] = anArg.second;
    }

    m_value = anArgs;
  }

  void visitArray (expression<CsgVisitor> e) {

    Json::array anArray;

    for (int i = 0; i < e.size(); ++i) {
      anArray.push_back (getValue (e[i]));
    }

    m_value = anArray;
  }
 
  void visitObject (expression<CsgVisitor> e) {

    Json::object anObject = Json::object({
      { "type", e[0].string() },
      { "properties", getValue (e[1]) },
    });

    m_value = anObject;
  }

  void visitInstruction (expression<CsgVisitor> e) {

    Json::object anInstruction = Json::object({
      { "type", e[0].string() },
      { "properties", getValue (e[1]) },
      { "objects", getValue (e[2]) },
    });

    m_value = anInstruction;
  }

private:

  Json m_value;
  
};

lars::parser<CsgVisitor> createParser() {

  parsing_expression_grammar_builder<CsgVisitor> aGrammar;
  using expression = expression<CsgVisitor>;

  auto aPropagateFn = [](expression e){ for (auto c : e) c.accept(); };

  aGrammar["File"]         << "ObjectList &'\\0'"                                 << aPropagateFn;
  aGrammar["ObjectList"]   << "( Comment | Object | Instruction )+"               << [](expression e){ e.visitor().visitArray (e); };
  aGrammar["Comment"]      << "'#' (!('\n') .)*"                                  << [](expression e){ /*std::cout << "Comment: " << e[0].string() << std::endl;*/ };
  aGrammar["Instruction"]  << "Name '(' ( Matrix | ArgumentList ) ')' InstrBody"  << [](expression e){ e.visitor().visitInstruction (e); };
  aGrammar["InstrBody"]    << "'{' ObjectList '}'"                                << aPropagateFn;
  aGrammar["Object"  ]     << "Name '(' ArgumentList ')' ';'"                     << [](expression e){ e.visitor().visitObject (e); };
  aGrammar["ArgumentList"] << " Argument ( ',' Argument )* | Argument? "          << [](expression e){ e.visitor().visitArguments (e); };
  aGrammar["Argument"]     << "( Name '=' ( Value ) )"                            ;
  aGrammar["Matrix"]       << "Array"                                             << aPropagateFn;
  aGrammar["Array"]        << "'[' ( Value ( ',' Value )* | (Value)? ) ']'"       << [](expression e){ e.visitor().visitArray (e); };
  aGrammar["Value"   ]     << "Number | '\"' String '\"' | Boolean | Array"       << aPropagateFn;
  aGrammar["Number"  ]     << "'-'? [0-9]+ ('.' [0-9]+)?"                         << [](expression e){ e.visitor().visitNumber (e); };
  aGrammar["Boolean" ]     << "'true' | 'false'"                                  << [](expression e){ e.visitor().visitBoolean (e); };
  aGrammar["Name"    ]     << "'$'? [a-zA-Z] [a-zA-Z]*"                                ;
  aGrammar["String"  ]     << "(!('\"') .)*"                                     << [](expression e){ e.visitor().visitString (e); };

  aGrammar.set_starting_rule ("File");

  aGrammar["Whitespace"] << "[ \t\n\r]";

  aGrammar.set_separator_rule ("Whitespace");

  return aGrammar.get_parser();
}
/*
// Valid types for CSG file format.
enum ValidType {
  CsgGroup,
  CsgMatrix,

  // CSG operations
  CsgUnion,
  CsgDifference,
  CsgIntersection,

  // CSG primitives
  CsgCube,
  CsgSphere,
  CsgCylinder,
};

enum Profile {
  CsgCoreProfile,
  OpenScadProfile
};

//! Helper class what ensures that JSON CSG description is valid.
class CsgValidator {

public:

  CsgValidator() {

    m_validNames["group"] = CsgGroup;
    m_validNames["multmatrix"] = CsgMatrix;
    m_validNames["union"] = CsgUnion;
    m_validNames["difference"] = CsgDifference;
    m_validNames["intersection"] = CsgIntersection;
    m_validNames["cube"] = CsgCube;
    m_validNames["sphere"] = CsgSphere;
    m_validNames["cylinder"] = CsgCylinder;
  }

  void validate (const json11::Json& theObject) {

    if (!theObject.is_object()) {
      return;
    }

    std::string aType = theObject["type"];

    auto anIter = m_validNames.find (aType);
    if (anIter == m_validNames.end()) {
      return; // invalid object
    }

    switch (*anIter) {
      case CsgGroup:
      {

        break;
      }
    }
  }

private:

  std::map<std::string, ValidType> m_validNames;

  Profile m_profile;
};*/

json11::Json Parser::parse (const std::string theFilePath) {

  auto aParser = createParser();
  CsgVisitor aVisitor;

  std::ifstream aStream (theFilePath);
  std::stringstream aBuffer;
  aBuffer << aStream.rdbuf();

  std::cout << aBuffer.str() << std::endl;

  try { 
    aParser.parse (aBuffer.str()).accept (&aVisitor); 
  }
  catch (parser<CsgVisitor>::error e) {
    for(auto i UNUSED :range(e.begin_position().character-1))std::cout << " ";
    for(auto i UNUSED :range(e.length()))std::cout << "~";
    std::cout << "^\n";
    std::cout << e.error_message() << " while parsing " << e.rule_name() << std::endl;
    std::cout << e.what() << std::endl;
  }

  std::cout << aVisitor.getValue().dump() << std::endl;

  return aVisitor.getValue();
}

json11::Json Parser::parseJSON (const std::string theFilePath) {
  
  std::ifstream aStream (theFilePath);
  std::stringstream aBuffer;
  aBuffer << aStream.rdbuf();

  std::string anErrors;
  Json aCsg = Json::parse (aBuffer.str(), anErrors);
  std::cout << anErrors << std::endl;

  // TODO: validate
}

// TODO: parser class members
// TODO: replace cout
void writeData (const json11::Json theData, const std::string& theIndent);

void writeProperties (const json11::Json theData) {
  if (!theData["properties"].is_object()) {
    throw std::runtime_error ("Object properties should be represented with a dictionary");
  }

  if (theData["properties"].object_items().empty()) {
    return; // not an error
  }

  auto aProperties = theData["properties"].object_items();

  // comma separated lists are messy
  auto anIter = aProperties.begin();
  std::cout << anIter->first << " = " << anIter->second.dump();
  anIter++;
  for (; anIter != aProperties.end(); ++anIter) {

    std::cout << ", " << anIter->first << " = " << anIter->second.dump();
  }
}

void writeObject (const json11::Json theData, const std::string& theIndent) {

  std::cout << theIndent << theData["type"].string_value() << "(";
  writeProperties (theData);
  std::cout << ");\n";
}

void writeInstruction (const json11::Json theData, const std::string& theIndent) {

  std::cout << theIndent << theData["type"].string_value() << "(";
  writeProperties (theData);
  std::cout << ") {\n";
  for (auto& anObject: theData["objects"].array_items()) {
    writeData (anObject, theIndent + "  ");
  }
  std::cout << theIndent << "}\n";
}

//! Checks if object has at least specified children count
void assertChildrenNum (const json11::Json theData, int theChildrenNum) {
  
  if (theData["objects"].array_items().size() < 2) {
    throw std::runtime_error ("Too few children objects for instruction: " + theData["type"].dump());
  }
}

void writeData (const json11::Json theData, const std::string& theIndent) {

  if (theData.is_array()) {
    for (auto& anObject: theData.array_items()) {
      writeData (anObject, theIndent);
    }
  }

  if (!theData.is_object()) {
    return; // error
  }

  std::string aType = theData["type"].string_value();

  if (aType == "group") {
    // OpenScad compatibility empty group
    if (theData["objects"].array_items().empty()) {
      std::cout << theIndent << "group();\n";
    }
    else {
      writeInstruction (theData, theIndent);
    }
  }
  else if (aType == "multmatrix") {

    // OpenScad compatibility matrix
    if (theData["properties"].is_array()) {

      // TODO: validate matrix
      std::cout << theIndent << theData["type"].string_value() << "(";
      std::cout << theData["properties"].dump();
      std::cout << ") {\n";
      for (auto& anObject: theData["objects"].array_items()) {
        writeData (anObject, theIndent + "  ");
      }
      std::cout << theIndent << "}\n";
    }
    else {
      writeInstruction (theData, theIndent);
    }
  }
  else if (aType == "union") {
    assertChildrenNum (theData, 2);
    writeInstruction (theData, theIndent);
  }
  else if (aType == "difference") {
    assertChildrenNum (theData, 2);
    writeInstruction (theData, theIndent);
  }
  else if (aType == "intersection") {
    assertChildrenNum (theData, 2);
    writeInstruction (theData, theIndent);
  }
  else if (aType == "cube") {
    writeObject (theData, theIndent);
  }
  else if (aType == "sphere") {
    writeObject (theData, theIndent);
  }
  else if (aType == "cylinder") {
    writeObject (theData, theIndent);
  }
  else {
    throw std::runtime_error ("Unknown object type: " + theData["type"].dump());
  }
}

void Parser::write (const json11::Json theData, const std::string theFilePath) {

  writeData (theData, "");
}

void Parser::writeJSON (const json11::Json theData, const std::string theFilePath) {

  // TODO: validate
  
  std::string aCsgJs = theData.dump();

  std::ofstream aFile;
  aFile.open (theFilePath);
  aFile << aCsgJs;
  aFile.close();
}

} // csg