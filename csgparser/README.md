# CSG Parser

File *csgparser.cpp* implements a simple reference parser of CSG format.
It depends on two libs:

* [json11](https://github.com/dropbox/json11)
* [lars::parser](https://github.com/TheLartians/Parser)

Which already included (hopefully legally) into the repository.

JSON format was used as intermediate data representation in order to not introduce something completely new.

## csg2json

File *csg2json.cpp* implements a simple CSG to JSON back and forth converter which serves for number of important tasks:
* It is a sample of using csgparser
* It allows validation of CSG files (or at least it should)
* It converts CSG files to JSON so you may stick to JSON both for import and export in your solution
