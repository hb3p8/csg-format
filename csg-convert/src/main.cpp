#include <iostream>
#include <unordered_map>
#include <cmath>
#include <list>

#include <json11/json11.hpp>
#include <parser/parser.h>

using namespace std;
using namespace lars;
using namespace json11;

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

int main (int argc, char ** argv) {
  parsing_expression_grammar_builder<CsgVisitor> aGrammar;
  using expression = expression<CsgVisitor>;

  auto propagate = [](expression e){ for (auto c : e) c.accept(); };
  
  aGrammar["ObjectList"]   << "( Comment | Object | Instruction )+"               << [](expression e){ e.visitor().visitArray (e); };
  aGrammar["Comment"]      << "'#' String"                                        << [](expression e){ std::cout << "Comment: " << e[0].string() << std::endl; };
  aGrammar["Instruction"]  << "Name '(' ArgumentList ')' ' '* '{' ObjectList '}'" << [](expression e){ e.visitor().visitInstruction (e); };
  aGrammar["Object"  ]     << "Name '(' ArgumentList ')' ';'"                     << [](expression e){ e.visitor().visitObject (e); };
  aGrammar["ArgumentList"] << " Argument ( ',' Argument )* | Argument? "          << [](expression e){ e.visitor().visitArguments (e); };
  aGrammar["Argument"]     << "Name '=' ( Value )"                                ;
  aGrammar["Array"]        << "'[' ( Value ( ',' Value )* | (Value)? ) ']'"       << [](expression e){ e.visitor().visitArray (e); };
  aGrammar["Value"   ]     << "Number | '\"' String '\"' | Boolean | Array"       << propagate;
  aGrammar["Number"  ]     << "'-'? [0-9]+ ('.' [0-9]+)?"                         << [](expression e){ e.visitor().visitNumber (e); };
  aGrammar["Boolean" ]     << "'true' | 'false'"                                  << [](expression e){ e.visitor().visitBoolean (e); };
  aGrammar["Name"    ]     << "[a-zA-Z] [a-zA-Z]*"                                ;
  aGrammar["String"  ]     << "[0-9a-zA-Z ]*"                                     << [](expression e){ e.visitor().visitString (e); };
  
  aGrammar.set_starting_rule("ObjectList");
  
  aGrammar["Whitespace"] << "[ \t\n]";
  
  aGrammar.set_separator_rule("Whitespace");
  
  auto aParser = aGrammar.get_parser();
  
  CsgVisitor visitor;

  std::list<std::string> aTestStrs;

  aTestStrs.push_back("#version csg-core 0.3");
  aTestStrs.push_back("box();");
  aTestStrs.push_back("sphere(r=5.0);");
  aTestStrs.push_back("matrix(m=[1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16]);");
  aTestStrs.push_back("sphere (r=true);");
  aTestStrs.push_back("sphere(r=\"three\");");
  aTestStrs.push_back("sphere(r=5.0, r=\"asd\");");
  aTestStrs.push_back("sphere(r=5.0, r=\"asd\", topYa=\"4\");");
  aTestStrs.push_back("union(r=5.0, r=\"asd\") { sphere(); }");
  aTestStrs.push_back("union(r=5.0, r=\"asd\") { sphere(); box(); }");

  aTestStrs.push_back("sphere (r=true);\n"
                      "sphere(r=\"three\");\n"
                      "sphere(r=5.0, r=\"asd\");");

  while (true) {
    string str;
    cout << "> ";
    if (!aTestStrs.empty()) {
      str = aTestStrs.front();
      aTestStrs.pop_front();
      std::cout << str << std::endl;
    }
    else {
      break;
      // getline(cin,str);
    }
    if(str == "q" || str == "quit")break;
    try { 
      aParser.parse (str).accept (&visitor); 
      cout << Json (visitor.getValue()).dump() << endl;
    }
    catch (parser<CsgVisitor>::error e){
      cout << "  ";
      for(auto i UNUSED :range(e.begin_position().character-1))cout << " ";
      for(auto i UNUSED :range(e.length()))cout << "~";
      cout << "^\n";
      cout << e.error_message() << " while parsing " << e.rule_name() << endl;
    }
  }

  return 0;
}
