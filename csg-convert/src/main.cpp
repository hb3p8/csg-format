#include <iostream>
#include <unordered_map>
#include <cmath>
#include <list>

#include <json11/json11.hpp>
#include <parser/parser.h>

using namespace std;
using namespace lars;

class CsgVisitor{
  
  double value;
  unordered_map<string,double> variables;
  
public:
  
  double get_value(){
    return value;
  }
  
  double get_value(expression<CsgVisitor> e){
    e.accept(this);
    return get_value();
  }
  
  void visit_number(expression<CsgVisitor> e){
    value = stod(e.string());
  }
  
  void visit_set_variable(expression<CsgVisitor> e){
    variables[e[0].string()] = get_value(e[1]);
  }
  
  void visit_variable(expression<CsgVisitor> e){
    value = variables[e[0].string()];
  }
  
  void visit_left_binary_operator_list (expression<CsgVisitor> e){
    double lhs = get_value(e[0]);
    
    for(auto i:range((e.size()-1)/2)*2+1){
      double rhs = get_value(e[i+1]);
           if(e[i].character()=='+'){ lhs = lhs + rhs; }
      else if(e[i].character()=='-'){ lhs = lhs - rhs; }
      else if(e[i].character()=='*'){ lhs = lhs * rhs; }
      else if(e[i].character()=='/'){ lhs = lhs / rhs; }
      else throw "undefined operator";
    }
    
    value = lhs;
  }
  
  void visit_exponent(expression<CsgVisitor> e){
    if(e.size() == 1) e[0].accept();
    else value = pow(get_value(e[0]), get_value(e[1]));
  }
  
};

int main(int argc, char ** argv){
  parsing_expression_grammar_builder<CsgVisitor> aGrammar;
  using Expression = expression<CsgVisitor>;

  auto propagate = [](Expression e){ for(auto c:e)c.accept(); };
  
  aGrammar["ObjectList"]   << "( Comment | Object | Instruction )+" << propagate;
  aGrammar["Comment"]      << "'#' String" << [ ](Expression e){ std::cout << "Comment: " << e[0].string() << std::endl; };
  aGrammar["Instruction"]  << "Name '(' ArgumentList ')' ' '* '{' ObjectList '}'" << propagate;
  aGrammar["Object"  ]     << "Name '(' ArgumentList ')' ';'" << propagate;
  aGrammar["ArgumentList"] << " Argument ( ',' Argument )* | Argument? " << propagate;
  aGrammar["Argument"]     << "Name '=' ( Number | '\"' String '\"' | Boolean )"     << [ ](Expression e){ std::cout << e[0].string() << " | " << e[1].string() << std::endl; };
  aGrammar["Number"  ]     << "'-'? [0-9]+ ('.' [0-9]+)?"               ;
  aGrammar["Boolean" ]     << "'true' | 'false'"               << [ ](Expression e){};
  aGrammar["Name"    ]     << "[a-zA-Z] [a-zA-Z]*" << [ ](Expression e){ std::cout << "Name found: " << std::endl; };
  aGrammar["String"  ]     << "[0-9a-zA-Z ]*" << [ ](Expression e){ std::cout << "String found: "  << std::endl; };
  
  aGrammar.set_starting_rule("ObjectList");
  
  aGrammar["Whitespace"] << "[ \t\n]";
  
  aGrammar.set_separator_rule("Whitespace");
  
  auto p = aGrammar.get_parser();
  
  CsgVisitor visitor;

  std::list<std::string> aTestStrs;

  aTestStrs.push_back("#version csg-core 0.3");
  aTestStrs.push_back("box();");
  // aTestStrs.push_back("");
  aTestStrs.push_back("sphere(r=5.0);");
  aTestStrs.push_back("sphere(r=true);");
  aTestStrs.push_back("sphere(r=\"three\");");
  aTestStrs.push_back("sphere(r=5.0, r=\"asd\");");
  aTestStrs.push_back("sphere(r=5.0, r=\"asd\", topYa=\"4\");");
  aTestStrs.push_back("union(r=5.0, r=\"asd\") { sphere(); }");
  aTestStrs.push_back("union(r=5.0, r=\"asd\") { sphere(); box(); }");

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
      p.parse(str).accept(&visitor); 
      // cout << str << " = " << visitor.get_value() << endl;
      cout << "Correct" << endl;
    }
    catch (parser<CsgVisitor>::error e){
      cout << "  ";
      for(auto i UNUSED :range(e.begin_position().character-1))cout << " ";
      for(auto i UNUSED :range(e.length()))cout << "~";
      cout << "^\n";
      cout << e.error_message() << " while parsing " << e.rule_name() << endl;
    }
  }

  json11::Json obj = json11::Json::object({
      { "k1", "v1" },
      { "k2", 42.0 },
      { "k3", json11::Json::array({ "a", 123.0, true, false, nullptr }) },
  });

  std::cout << "obj: " << obj.dump() << "\n";
  
  return 0;
}
