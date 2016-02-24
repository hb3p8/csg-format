#include <iostream>
#include <unordered_map>
#include <cmath>

#include "parser/parser.h"

using namespace std;
using namespace lars;

int main(int argc, char ** argv){
  parsing_expression_grammar_builder<double> g;
  using expression = expression<double>;
  
  unordered_map<string,double> variables;
  
  g["Expression"] << "(Set | Sum) &'\\0'"              << [ ](expression e){ e.value() = e[0].get_value();                       };
  g["Set"       ] << "Name '=' Sum"                    << [&](expression e){ variables[e[0].string()] = e[1].get_value();        };
  g["Sum"       ] << "Add | Subtract | Product"        << [ ](expression e){ e.value() = e[0].get_value();                       };
  g["Add"       ] << "Sum '+' Product"                 << [ ](expression e){ e.value() = e[0].get_value() + e[1].get_value();    };
  g["Subtract"  ] << "Sum '-' Product"                 << [ ](expression e){ e.value() = e[0].get_value() - e[1].get_value();    };
  g["Product"   ] << "Multiply | Divide | Exponent"    << [ ](expression e){ e.value() = e[0].get_value();                       };
  g["Multiply"  ] << "Product '*' Exponent"            << [ ](expression e){ e.value() = e[0].get_value() * e[1].get_value();    };
  g["Divide"    ] << "Product '/' Exponent"            << [ ](expression e){ e.value() = e[0].get_value() / e[1].get_value();    };
  g["Exponent"  ] << "Power | Atomic"                  << [ ](expression e){ e.value() = e[0].get_value();                       };
  g["Power"     ] << "Atomic '^' Exponent"             << [ ](expression e){ e.value() = pow(e[0].get_value(),e[1].get_value()); };
  g["Atomic"    ] << "Number | Brackets | Variable"    << [ ](expression e){ e.value() = e[0].get_value();                       };
  g["Brackets"  ] << "'(' Sum ')'"                     << [ ](expression e){ e.value() = e[0].get_value();                       };
  g["Number"    ] << "'-'? [0-9]+ ('.' [0-9]+)?"       << [ ](expression e){ e.value() = stod(e.string());                       };
  g["Variable"  ] << "Name"                            << [&](expression e){ e.value() = variables[e[0].string()];               };
  g["Name"      ] << "[a-zA-Z] [a-zA-Z0-9]*"           ;
  
  g.set_starting_rule("Expression");
  
  g["Whitespace"] << "[ \t]";
  g.set_separator_rule("Whitespace");
  
  auto p = g.get_parser();
  
  while (true) {
    string str;
    cout << "> ";
    getline(cin,str);
    if(str == "q" || str == "quit")break;
    try {
      auto e = p.parse(str);
      cout << str << " = " << *e.evaluate() << endl;
    }
    catch (parser<double>::error e){
      cout << "  ";
      for(auto i UNUSED :range(e.begin_position().character-1))cout << " ";
      for(auto i UNUSED :range(e.length()))cout << "~";
      cout << "^\n";
      cout << e.error_message() << " while parsing " << e.rule_name() << endl;
    }
  }
  
  return 0;
}

