/// This file is not technically part of the ChaiScript API. It is used solely for generating Doxygen docs
/// regarding the ChaiScript standard runtime library.

/// \brief Items in this namespace exist in the ChaiScript language runtime. They are not part of the C++ API
namespace ChaiScript_Language
{

/// \page LangStandardLibraryRef ChaiScript Language Standard Libary Reference
///
/// ChaiScript, at its core, has some very functional programming-inspired habits. Few places show this off as clearly 
/// as the prelude, itself a name taken as a nod to the popular functional language Haskell. This prelude is available 
/// to all standard ChaiScript applications, and provides a simple foundation for using numbers, strings, and ranges 
/// (the general category of Container cs and their iteration).
///



/// \brief Converts o into a string.
///
/// \code
/// eval> to_string(3).is_type("string") <br>
/// true<br>
/// \endcode
string to_string(Object o);


/// \brief Prints o to the terminal, without a trailing carriage return. Applies conversions to string automatically.
/// \code
/// eval> puts("hi, "); puts("there")
/// hi, thereeval>
/// \endcode
/// \sa to_string
/// \sa print
void puts(Object o);


/// \brief Prints o to the terminal, with a trailing carriage return. Applies conversions to string automatically
/// \code
/// eval> print("hello")
/// hello
/// eval>
/// \endcode
/// \sa to_string
/// \sa puts
void print(Object o);

/// \brief ChaiScript representation of std::string. It is an std::string but only some member are exposed to ChaiScript.
/// 
/// Because the ChaiScript string object is an std::string, it is directly convertable to and from std::string
/// using the chaiscript::boxed_cast and chaiscript::var functions.
///
/// With the exception of string::trim, string::rtrim, string::ltrim, all members are direct passthroughs to the
/// std::string of the same name. 
///
/// \note Object and function notations are equivalent in ChaiScript. This means that
///       \c "bob".find("b") and \c find("bob", "b") are exactly the same. Most examples below follow the
///       second formation of the function calls.
/// \sa \ref keyworddef for extending existing C++ classes in ChaiScript
class string
{
  public:
    /// \brief Finds the first instance of substr.
    /// \code
    /// eval> find("abab", "ab")
    /// 0
    /// \endcode
    int find(string s) const;


    /// \brief Finds the last instance of substr.
    /// \code
    /// eval> rfind("abab", "ab")
    /// 2
    /// \endcode
    int rfind(string s) const;

    /// \brief Finds the first of characters in list in the string.
    ///
    /// \code
    /// eval> find_first_of("abab", "bec")
    /// 1
    /// \endcode
    int find_first_of(string list) const;

    /// \brief Finds the last of characters in list in the string.
    ///
    /// \code
    /// eval> find_last_of("abab", "bec")
    /// 3
    /// \endcode
    int find_last_of(string list) const;

    /// \brief Finds the first non-matching character to list in the str string.
    ///
    /// \code
    /// eval> find_first_not_of("abcd", "fec")
    /// 0
    /// \endcode
    int find_first_not_of(string list) const;

    /// \brief Finds the last non-matching character to list in the list string.
    ///
    /// \code
    /// eval> find_last_not_of("abcd", "fec")
    /// 3
    /// \endcode 
    int find_last_not_of(string list) const;

    /// \brief Removes whitespace from the front of the string, returning a new string
    ///
    /// \note This function is implemented as a ChaiScript function using the def member function notation.
    ///
    /// \code
    /// eval> ltrim("  bob")
    /// bob
    /// \endcode
    ///
    /// \sa \ref keyworddef
    string lstrim() const;

    /// \brief Removes whitespace from the back of the string, returning a new string
    ///
    /// \note This function is implemented as a ChaiScript function using the def member function notation.
    ///
    /// \code
    /// eval> rtrim("bob  ") + "|"
    /// bob|
    /// \endcode
    ///
    /// \sa \ref keyworddef
    string rtrim() const;

    /// \brief Removes whitespace from the front and back of the string, returning a new string
    ///
    /// \note This function is implemented as a ChaiScript function using the def member function notation.
    ///
    /// \code
    /// eval> trim("  bob  ") + "|"
    /// bob|
    /// \endcode
    /// 
    /// Equivalent to rtrim(ltrim("  bob  "));
    ///
    /// \sa \ref keyworddef
    string trim() const;
};

class Function
{
  public:
};

/// \brief Generic concept of a value in ChaiScript. 
///
/// The Object type exists merely as a concept. All objects in ChaiScript support this concept 
/// and have the following methods available to them. All objects are stored internally as chaiscript::Boxed_Value types.
class Object
{
  public:
}

/// \brief Returns the max of a or b. Requires that operator>(a, b) exists
/// Equivalent to 
/// \code
/// return a>b?a:b;
/// \endcode
///
/// Example:
/// \code
/// eval> max(4, 10)
/// 10
/// \endcode
Object max(Object a, Object b);

/// \brief Returns the min of a or b. Requires that operator<(a, b) exists
///
/// Equivalent to 
/// \code
/// return a<b?a:b;
/// \endcode
///
/// Example:
/// \code
/// eval> min(4, 10)
/// 4
/// \endcode
Object min(Object a, Object b);

/// \brief Returns true if x is an even integer. 
/// 
/// Will also work on any non-integer type for which an operator%(x, int) exists
///
/// Example:
/// \code
/// eval> even(4)
/// true
/// \endcode
bool even(Object x);

/// \brief Returns true if x is an odd integer. 
/// 
/// Will also work on any non-integer type for which an operator%(x, int) exists
///
/// Example:
/// \code
/// eval> odd(4)
/// false 
/// \endcode
bool even(Object x);


/// \brief Applies the function f over each element in the Container c.
///
/// Example:
/// \code
/// eval> for_each([1, 2, 3], print)
/// 1
/// 2
/// 3
/// \endcode
void for_each(Container c, Function f) 


/// \brief Applies f over each element in the Container c, joining all the results.
///
/// Example:
/// \code
/// eval> map([1, 2, 3], odd)
/// [true, false, true]
/// \endcode
Container map(Container c, Function f) 


/// \brief Starts with the initial value and applies the function f to it and the first element of the Container c. 
///        The result is then applied to the second element, and so on until the elements are exhausted.
///
/// Example:
/// \code
/// eval> foldl([1, 2, 3, 4], `+`, 0)
/// 10
/// \endcode
Object foldl(Container c, Function f, Object initial) 


/// \brief Returns the sum total of the values in the Container c.
///
/// Example:
/// \code
/// eval> sum([1, 2, 3, 4])
/// 10
/// \endcode
///
/// Equivalent to:
/// \code
/// foldl(c, `+`, 0.0);
/// \endcode
Numeric sum(Container c) 


/// \brief Returns the product of the value in the Container c.
///
/// Example:
/// \code
/// eval> product([1, 2, 3, 4])
/// 24
/// \endcode
///
/// Equivalent to:
/// \code
/// foldl(c, `*`, 1.0);
/// \endcode
Numeric product(Container c) 


/// \brief Takes num elements from the Container c, returning them.
///
/// Example:
/// \code
/// eval> take([1, 2, 3, 4], 2)
/// [1, 2]
/// \endcode
///
/// \returns A container of the same type that was passed in
Object take(Container c, int num) 


/// \brief Takes elements from the Container c that match function f, stopping at the first non-match, returning them as a new Vector.
///
/// Example:
/// \code
/// eval> take_while([1, 2, 3], odd)
/// [1]
/// \endcode
///
/// \returns A container of the same type that was passed in
Object take_while(Container c, Function f) 


/// \brief Drops num elements from the Container c, returning the remainder.
///
/// Example:
/// \code
/// eval> drop([1, 2, 3, 4], 2)
/// [3, 4]
/// \endcode
///
/// \returns A container of the same type that was passed in
Object drop(Container c, int num) 


/// \brief Drops elements from the Container c that match f, stopping at the first non-match, returning the remainder.
///
/// Example:
/// \code
/// eval> drop_while([1, 2, 3], odd)         
/// [2, 3]
/// \endcode
Object drop_while(Container c, Function f) 


/// \brief Similar to foldl, this takes the first two elements as its starting values for f. This assumes Container c has at least 2 elements.
///
/// Example:
/// \code
/// eval> reduce([1, 2, 3, 4], `+`)
/// 10
/// \endcode
Object reduce(Container c, Function f) 


/// \brief Takes elements from Container c that match function f, return them.
///
/// Example:
/// \code
/// eval> filter([1, 2, 3, 4], odd)
/// [1, 3]
/// \endcode
Object filter(Container c, Function f) 


/// \brief Joins the elements of the Container c into a string, delimiting each with the delim string.
///
/// Example:
/// \code
/// eval> join([1, 2, 3], "*") 
/// 1*2*3
/// \endcode
string join(Container c, string delim) 


/// \brief Returns the contents of the Container c in reversed order.
///
/// Example:
/// \code
/// eval> reverse([1, 2, 3, 4, 5, 6, 7])
/// [7, 6, 5, 4, 3, 2, 1]
/// \endcode
Container reverse(Container c) 


/// \brief Generates a new Vector filled with values starting at x and ending with y.
///
/// Works on types supporting operator<=(x, y) and operator++(x)
///
/// Example:
/// \code
/// eval> generate_range(1, 10)
/// [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
/// \endcode
Vector generate_range(Object x, Object y) 


/// \brief Returns a new Container with x and y concatenated.
///
/// Example:
/// \code
/// eval> concat([1, 2, 3], [4, 5, 6])
/// [1, 2, 3, 4, 5, 6]
/// \endcode
Object concat(Container x, Container y) 


/// \brief Returns a new Vector with x and y as its values.
///
/// Example:
/// \code
/// eval> collate(1, 2)
/// [1, 2]
/// \endcode
Vector collate(x, y) 


/// \brief Applies f to elements of x and y, returning a new Vector with the result of each application.
///
/// Example:
/// \code
/// eval> zip_with(`+`, [1, 2, 3], [4, 5, 6])
/// [5, 7, 9]
/// \endcode
Vector zip_with(Function f, Container x, Container y) 


/// \brief Collates elements of x and y, returning a new Vector with the result.
///
/// Example:
/// \code
/// eval> zip([1, 2, 3], [4, 5, 6])
/// [[1, 4], [2, 5], [3, 6]]
/// \endcode
Vector zip(Container x, Container y) 


}

