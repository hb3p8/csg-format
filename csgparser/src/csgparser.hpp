#include <string>

#include <json11/json11.hpp>

#define CSG_EXPORT

namespace csg {

class Parser {
  
public:

  Parser();

  //! Reads CSG file.
  CSG_EXPORT static json11::Json parse (const std::string theFilePath);

  //! Reads CSGJS file.
  CSG_EXPORT static json11::Json parseJSON (const std::string theFilePath);

  //! Validates and writes CSG file.
  CSG_EXPORT static void write (const json11::Json theData, const std::string theFilePath);

  //! Validates and writes CSGJS file.
  // TODO: implement the actual validation
  CSG_EXPORT static void writeJSON (const json11::Json theData, const std::string theFilePath);
  
};

} // csg