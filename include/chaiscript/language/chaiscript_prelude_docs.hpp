
/// \brief Items in this namespace exist in the ChaiScript language runtime. They are not part of the C++ API
namespace ChaiScript_Language
{

/// \page LangStandardLibraryRef ChaiScript Language Standard Libary Reference
///
/// ChaiScript, at its core, has some very functional programming-inspired habits. Few places show this off as clearly 
/// as the prelude, itself a name taken as a nod to the popular functional language Haskell. This prelude is available 
/// to all standard ChaiScript applications, and provides a simple foundation for using numbers, strings, and ranges 
/// (the general category of containers and their iteration).
///
/// \section LibraryStrings Strings
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

    /// Finds the first of characters in list in the string.
    ///
    /// \code
    /// eval> find_first_of("abab", "bec")
    /// 1
    /// \endcode
    int find_first_of(string list) const;

    /// Finds the last of characters in list in the string.
    ///
    /// \code
    /// eval> find_last_of("abab", "bec")
    /// 3
    /// \endcode
    int find_last_of(string list) const;

    /// Finds the first non-matching character to list in the str string.
    ///
    /// \code
    /// eval> find_first_not_of("abcd", "fec")
    /// 0
    /// \endcode
    int find_first_not_of(string list) const;

    ///find_last_not_of(str, list): Finds the last non-matching character to list in the str string.
    ///
    /// \code
    /// eval> find_last_not_of("abcd", "fec")
    /// 3
    /// \endcode 
    int find_last_not_of(string list) const;

    /// Removes whitespace from the front of the string, returning a new string
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

    /// Removes whitespace from the back of the string, returning a new string
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

    /// Removes whitespace from the front and back of the string, returning a new string
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

///Numbers
///
///max(a, b): Returns the maximum value of a or b.
///
///eval> max(4, 10)
///10
///min(a, b): Returns the minimum value of a or b.
///
///eval> min(4, 10)
///4
///even(x): Returns true if x is even, otherwise returns false.
///
///eval> even(4)
///true
///odd(x): Returns true if x is odd, otherwise returns false.
///
///eval> odd(4)
///false
///Containers
///
///for_each(container, f): Applies the function f over each element in the container.
///
///eval> for_each([1, 2, 3], print)
///1
///2
///3
///map(container, f): Applies f over each element in the container, joining all the results.
///
///eval> map([1, 2, 3], odd)
///[true, false, true]
///foldl(container, f, initial): Starts with the initial value and applies the function f to it and the first element of the container. The result is then applied to the second element, and so on until the elements are exhausted.
///
///eval> foldl([1, 2, 3, 4], `+`, 0)
///10
///sum(container): Returns the sum total of the values in the container.
///
///eval> sum([1, 2, 3, 4])
///10
///product(container): Returns the product of the value in the container.
///
///eval> product([1, 2, 3, 4])
///24
///take(container, num): Takes num elements from the container, returning them.
///
///eval> take([1, 2, 3, 4], 2)
///[1, 2]
///take_while(container, f): Takes elements from the container that match function f, stopping at the first non-match, returning them as a new Vector.
///
///eval> take_while([1, 2, 3], odd)
///[1]
///drop(container, num): Drops num elements from the container, returning the remainder.
///
///eval> drop([1, 2, 3, 4], 2)
///[3, 4]
///drop_while(container, f): Drops elements from the container that match f, stopping at the first non-match, returning the remainder.
///
///eval> drop_while([1, 2, 3], odd)         
///[2, 3]
///reduce(container, f): Similar to foldl, this takes the first two elements as its starting values for f. This assumes container has at least 2 elements.
///
///eval> reduce([1, 2, 3, 4], `+`)
///10
///filter(container, f): Takes elements from container that match function f, return them.
///
///eval> filter([1, 2, 3, 4], odd)
///[1, 3]
///join(container, delim): Joins the elements of the container into a string, delimiting each with the delim string.
///
///eval> join([1, 2, 3], "*") 
///1*2*3
///reverse(container): Returns the contents of the container in reversed order.
///
///eval> reverse([1, 2, 3, 4, 5, 6, 7])
///[7, 6, 5, 4, 3, 2, 1]
///generate_range(x, y): Generates a new Vector filled with values starting at x and ending with y.
///
///eval> generate_range(1, 10)
///[1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
///concat(x, y): Returns a new Vector with x and y concatenated.
///
///eval> concat([1, 2, 3], [4, 5, 6])
///[1, 2, 3, 4, 5, 6]
///collate(x, y): Returns a new Vector with x and y as its values.
///
///eval> collate(1, 2)
///[1, 2]
///zip_with(f, x, y): Applies f to elements of x and y, returning a new Vector with the result of each application.
///
///eval> zip_with(`+`, [1, 2, 3], [4, 5, 6])
///[5, 7, 9]
///zip(x, y): Collates elements of x and y, returning a new Vector with the result.
///
///eval> zip([1, 2, 3], [4, 5, 6])
///[[1, 4], [2, 5], [3, 6]]
}

