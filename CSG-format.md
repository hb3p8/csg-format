# CSG File Format

Constructive solid geometry (CSG) is a technique used in solid modeling. Constructive solid geometry allows a modeler to create a complex surface or object by using Boolean operators to combine objects.

CSG file format is a lightweight, text-based, language-independent CSG data interchange format. It was derived from .scad format (https://en.wikibooks.org/wiki/OpenSCAD_User_Manual/The_OpenSCAD_Language). CSG format defines a small set of formatting rules for the portable representation of structured CSG data.

## Grammar

### Introduction

CSG format can represent three primitive types (strings, numbers, booleans) and three structured types (instructions, arrays and objects).

A string is a sequence of zero or more Unicode characters.
`union`

An array is an ordered sequence of zero or more values.
`[1, 2, 3]`

Instructions consist of instruction name, optional arguments and scope:

    union() {
      ...
    }

Objects consist of object name and optional arguments:
`cube(size = [15, 15, 15]);`

There are following structural characters:
* begin-comment "#" number sign
* begin-arguments "(" left bracket
* begin-array "[" left square bracket
* begin-scope "{" left curly bracket
* end-arguments ")" right bracket
* end-array "]" right square bracket
* end-scope "}" right curly bracket
* value-separator "," comma
* object-separator ";" semicolon
* name-separator "=" equals sign

Insignificant whitespace is allowed before or after any of the structural characters.

### Values

A value **must** be an object, instruction, array, number, name, or string, or one of the following two literal names:
* false
* true

The literal names **must** be lowercase. No other literal names are allowed.

### Strings

The representation of strings is similar to conventions used in the C family of programming languages. A string begins and ends with quotation marks. All Unicode characters may be placed within the quotation marks.

### Comments

Comments consist of number sign followed by an arbitrary string. Only single line comments are allowed.

    comment = begin-comment *( string )

### Names

Names are the strings used to identify objects, instructions and arguments. It should not be enclosed by quotation marks. None of structural characters or escaped characters allowed in *names*.

### Objects

An object structure is represented as a string (object name) followed by a pair of brackets surrounding zero or more name/value pairs (or arguments). A name of argument is a string. A single equals sign comes after each name, separating the name from the value. A single comma separates a value from a following name. The names within an object **should** be unique. Each object **should** be followed by a semicolon.

    object = name begin-arguments [ argument *( value-separator argument ) ] end-arguments object-separator

    argument = name name-separator value

### Instructions

An instruction structure is represented as a string (instruction name) followed by a pair of brackets surrounding zero or more name/value pairs (or arguments). A name of argument is a string. A single equals sign comes after each name, separating the name from the value. A single comma separates a value from a following name. The names within an object **should** be unique. Each object **should** be followed by a pair of curly brackets (scope) surrounding one or more objects.

    instruction = name begin-arguments [ argument *( value-separator argument ) ] end-arguments begin-scope
      object
      *( object )
    end-scope

### Arrays

An array structure is represented as square brackets surrounding zero or more values (or elements). Elements are separated by commas.

    array = begin-array [ value *( value-separator value ) ] end-array

### Numbers

The representation of numbers is similar to that used in most programming languages. A number contains an integer component that may be prefixed with an optional minus sign, which may be followed by a fraction part and/or an exponent part.

Octal and hex forms are not allowed. Leading zeros are not allowed.

A fraction part is a decimal point followed by one or more digits.

An exponent part begins with the letter E in upper or lowercase, which may be followed by a plus or minus sign. The E and optional sign are followed by one or more digits. 

Numeric values that cannot be represented as sequences of digits (such as Infinity and NaN) are not permitted.

# CSG Elements

This section describes standard CSG primitives and operations.

### Vectors

Vector in 3d space should be represented as array consisting of 3 elements:
`[0.5, 0.6, 0.3]`

### Matrices

4x4 matrix in 3d space should be represented as array consisting of 16 elements:

`[1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 15.0, 20.0, 5.0, 1]`

## Objects

In CSG file format objects represent CSG primitives.

### Cube

Creates a cube in the first octant. When center is true, the cube is centered on the origin.

**Parameters:**

    vector: size
        dimensions of the cube represented with vector
        default: [1, 1, 1]

    boolean: center
        false, cube is placed in 1st (positive) octant, one corner at (0,0,0)
        true, cube is centered at (0,0,0)
        default: true

**Example:**

    cube(size=[0.5, 2.0, 3.0], center=true);

### Sphere

Creates a sphere at the origin of the coordinate system.

**Parameters:**

    real: r
        radius of sphere
        default: 1.0

**Example:**

    sphere(r=0.5);

### Cylinder

Creates a cylinder centered about the z axis. When center is true, it is also centered vertically along the z axis.

**Parameters:**

    real: h
        height of the cylinder
        default: 1.0

    real: r
        radius of cylinder
        default: 1.0

    boolean: center
        false, cylinder is placed above XY plane
        true, cylinder is centered at (0,0,0)
        default: true

**Example:**

    cylinder(h=3.0, r=0.5, center=true);

### Cone

Creates a cone centered about the z axis. When center is true, it is also centered vertically along the z axis.

**Parameters:**

    real: h
        height of the cone
        default: 1.0

    real: r1
        bottom radius of cone
        default: 1.0

    real: r2
        top radius of cone
        default: 1.0

    boolean: center
        false, cone is placed above XY plane
        true, cone is centered at (0,0,0)
        default: true

**Example:**

    cone(h=3.0, r1=1.5, r2=0.0, center=true);

## Instructions

In CSG file format instructions represent operations on CSG primitives.

### Multmatrix

Multiplies the geometry of all child elements with the given 4x4 transformation matrix.

**Parameters:**

    matrix: m
        homogeneous transformation matrix
        default: 1.0

**Example:**

    multmatrix(m=[1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 15.0, 20.0, 5.0, 1]) {
      cube(); 
    }

### Union

Creates a union of all its child nodes. This is the sum of all children (logical **or**).

**Example:**

    union() {
      cube(); 
      sphere();
    }

### Difference

Subtracts the 2nd (and all further) child nodes from the first one (logical **and not**).

**Example:**

    difference() {
      cube(); 
      sphere();
    }

### Intersection

Creates the intersection of all child nodes. This keeps the overlapping portion (logical **and**).
Only the area which is common or shared by **all** children is retained.

**Example:**

    intersection() {
      cube(); 
      sphere();
    }

### SMin

Creates a union with smooth transitions of all its child nodes. For distance field based approaches could be implemented as polynomial smooth min or similar. Otherwise could fallback to a union operation.

**Parameters:**

    real: k
        smoothnes coefficient in range [0, 1] (smin should behave as simple union with k=0.0)
        default: 0.5

**Example:**

    smin() {
      cube(); 
      sphere();
    }

### Group

Groups are used for CSG element aggregation. No implicit CSG operations should be performed on group children.

**Parameters:**

    string: name
        associated name
        default: ""

**Example:**

    group(name="widget") {
      union() {
        cube(); 
        sphere();
      }
    }

## Versioning

Version of CSG file should be defined with version tag in the begining of the file, for example:

    #openscad 1.0
    
    union() {
      cube(); 
      sphere();
    }

