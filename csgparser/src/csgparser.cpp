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
  
  aGrammar["ObjectList"]   << "( Comment | Object | Instruction )+"               << [](expression e){ e.visitor().visitArray (e); };
  aGrammar["Comment"]      << "'#' String"                                        << [](expression e){ std::cout << "Comment: " << e[0].string() << std::endl; };
  aGrammar["Instruction"]  << "Name '(' ArgumentList ')' ' '* '{' ObjectList '}'" << [](expression e){ e.visitor().visitInstruction (e); };
  aGrammar["Object"  ]     << "Name '(' ArgumentList ')' ';'"                     << [](expression e){ e.visitor().visitObject (e); };
  aGrammar["ArgumentList"] << " Argument ( ',' Argument )* | Argument? "          << [](expression e){ e.visitor().visitArguments (e); };
  aGrammar["Argument"]     << "Name '=' ( Value )"                                ;
  aGrammar["Array"]        << "'[' ( Value ( ',' Value )* | (Value)? ) ']'"       << [](expression e){ e.visitor().visitArray (e); };
  aGrammar["Value"   ]     << "Number | '\"' String '\"' | Boolean | Array"       << aPropagateFn;
  aGrammar["Number"  ]     << "'-'? [0-9]+ ('.' [0-9]+)?"                         << [](expression e){ e.visitor().visitNumber (e); };
  aGrammar["Boolean" ]     << "'true' | 'false'"                                  << [](expression e){ e.visitor().visitBoolean (e); };
  aGrammar["Name"    ]     << "[a-zA-Z] [a-zA-Z]*"                                ;
  aGrammar["String"  ]     << "[0-9a-zA-Z ]*"                                     << [](expression e){ e.visitor().visitString (e); };
  
  aGrammar.set_starting_rule ("ObjectList");
  
  aGrammar["Whitespace"] << "[ \t\n]";
  
  aGrammar.set_separator_rule ("Whitespace");
  
  return aGrammar.get_parser();
}

json11::Json Parser::parse (const std::string theFilePath) {

  auto aParser = createParser();
  CsgVisitor aVisitor;

  std::ifstream aStream (theFilePath);
  std::stringstream aBuffer;
  aBuffer << aStream.rdbuf();

  try { 
    aParser.parse (aBuffer.str()).accept (&aVisitor); 
  }
  catch (parser<CsgVisitor>::error e) {
    std::cout << e.error_message() << " while parsing " << e.rule_name() << std::endl;
  }

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

void Parser::write (const json11::Json theData, const std::string theFilePath) {
  //
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