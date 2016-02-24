#include <iostream>
#include <unordered_map>
#include <cmath>

#include "parser/parser.h"

using namespace std;
using namespace lars;

class math_visitor{
  
  double value;
  unordered_map<string,double> variables;
  
public:
  
  double get_value(){
    return value;
  }
  
  double get_value(expression<math_visitor> e){
    e.accept(this);
    return get_value();
  }
  
  void visit_number(expression<math_visitor> e){
    value = stod(e.string());
  }
  
  void visit_set_variable(expression<math_visitor> e){
    variables[e[0].string()] = get_value(e[1]);
  }
  
  void visit_variable(expression<math_visitor> e){
    value = variables[e[0].string()];
  }
  
  void visit_left_binary_operator_list (expression<math_visitor> e){
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
  
  void visit_exponent(expression<math_visitor> e){
    if(e.size() == 1) e[0].accept();
    else value = pow(get_value(e[0]), get_value(e[1]));
  }
  
};

int main(int argc, char ** argv){
  parsing_expression_grammar_builder<math_visitor> g;
  using expression = expression<math_visitor>;
  
  // g["Expression"] << "(Set | Sum) &'\\0'"                      ;
  g["Object"  ] << "Name '(' Argument (',' Argument)* ')' ';'"   << [ ](expression e){std::cout << e[0].string() << std::endl; };
  g["Argument"    ] << "Number | '\"' String '\"'"            ;
  g["Number"    ] << "'-'? [0-9]+ ('.' [0-9]+)?"               ;
  g["Name"      ] << "[a-zA-Z] [a-zA-Z]*"                      ;
  g["String"    ] << "[.]*"                                    ;
  
  g.set_starting_rule("Expression");
  
  g["Whitespace"] << "[ \t]";
  
  g.set_separator_rule("Whitespace");
  
  auto p = g.get_parser();
  
  math_visitor visitor;

  while (true) {
    string str;
    cout << "> ";
    getline(cin,str);
    if(str == "q" || str == "quit")break;
    try { 
      p.parse(str).accept(&visitor); 
      cout << str << " = " << visitor.get_value() << endl;
    }
    catch (parser<math_visitor>::error e){
      cout << "  ";
      for(auto i UNUSED :range(e.begin_position().character-1))cout << " ";
      for(auto i UNUSED :range(e.length()))cout << "~";
      cout << "^\n";
      cout << e.error_message() << " while parsing " << e.rule_name() << endl;
    }
  }
  
  return 0;
}
