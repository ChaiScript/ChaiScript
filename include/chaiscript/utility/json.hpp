// From github.com/nbsdx/SimpleJSON. 
// Released under the DWTFYW PL
//


#pragma once

#ifndef SIMPLEJSON_HPP
#define SIMPLEJSON_HPP


#include <cstdint>
#include <cmath>
#include <cctype>
#include <string>
#include <deque>
#include <map>
#include <type_traits>
#include <initializer_list>
#include <ostream>
#include <iostream>
#include "../chaiscript_defines.hpp"

namespace json {

using std::map;
using std::deque;
using std::string;
using std::enable_if;
using std::initializer_list;
using std::is_same;
using std::is_convertible;
using std::is_integral;
using std::is_floating_point;

namespace {
    string json_escape( const string &str ) {
        string output;
        for( unsigned i = 0; i < str.length(); ++i )
            switch( str[i] ) {
                case '\"': output += "\\\""; break;
                case '\\': output += "\\\\"; break;
                case '\b': output += "\\b";  break;
                case '\f': output += "\\f";  break;
                case '\n': output += "\\n";  break;
                case '\r': output += "\\r";  break;
                case '\t': output += "\\t";  break;
                default  : output += str[i]; break;
            }
        return output;
    }

    bool isspace(const char c)
    {
#ifdef CHAISCRIPT_MSVC
      // MSVC warns on these line in some circumstances
#pragma warning(push)
#pragma warning(disable : 6330)
#endif
      return ::isspace(c) != 0;
#ifdef CHAISCRIPT_MSVC
#pragma warning(pop)
#endif


    }
}

class JSON
{
    struct Internal {
        template<typename T>
        auto clone(const std::unique_ptr<T> &ptr) {
          if (ptr != nullptr) {
            return std::make_unique<T>(*ptr);
          } else {
            return std::unique_ptr<T>(nullptr);
          }
        }

        Internal( double d ) : Float( d ){}
        Internal( long   l ) : Int( l ){}
        Internal( bool   b ) : Bool( b ){}
        Internal( string s ) : String(std::make_unique<std::string>(std::move(s))) {}
        Internal()           : Int( 0 ){}

        Internal(const Internal &other)
          : List(clone(other.List)),
            Map(clone(other.Map)),
            String(clone(other.String)),
            Float(other.Float),
            Int(other.Int),
            Bool(other.Bool)
        {
        }

        Internal &operator=(const Internal &other)
        {
          List = clone(other.List);
          Map = clone(other.Map);
          String = clone(other.String);
          Float = other.Float;
          Int = other.Int;
          Bool = other.Bool;
          return *this;
        }

        Internal(Internal &&) = default;
        Internal &operator=(Internal &&) = default;

        std::unique_ptr<deque<JSON>>      List;
        std::unique_ptr<map<string,JSON>> Map;
        std::unique_ptr<string>           String;
        double              Float;
        long                Int;
        bool                Bool;
    };

    Internal internal;

    public:
        enum class Class {
            Null,
            Object,
            Array,
            String,
            Floating,
            Integral,
            Boolean
        };

        template <typename Container>
        class JSONWrapper {
            Container *object = nullptr;

            public:
                JSONWrapper( Container *val ) : object( val ) {}
                JSONWrapper( std::nullptr_t ) {}

                typename Container::iterator begin() { return object ? object->begin() : typename Container::iterator(); }
                typename Container::iterator end() { return object ? object->end() : typename Container::iterator(); }
                typename Container::const_iterator begin() const { return object ? object->begin() : typename Container::iterator(); }
                typename Container::const_iterator end() const { return object ? object->end() : typename Container::iterator(); }
        };

        template <typename Container>
        class JSONConstWrapper {
            const Container *object = nullptr;

            public:
                JSONConstWrapper( const Container *val ) : object( val ) {}
                JSONConstWrapper( std::nullptr_t ) {}

                typename Container::const_iterator begin() const { return object ? object->begin() : typename Container::const_iterator(); }
                typename Container::const_iterator end() const { return object ? object->end() : typename Container::const_iterator(); }
        };

        JSON() : internal(), Type( Class::Null ){}

        explicit JSON(Class type)
            : internal(), Type(Class::Null)
        {
            SetType( type );
        }

        JSON( initializer_list<JSON> list ) 
            : internal(), Type(Class::Null)
        {
            SetType( Class::Object );
            for( auto i = list.begin(), e = list.end(); i != e; ++i, ++i )
                operator[]( i->ToString() ) = *std::next( i );
        }

        template <typename T>
        JSON( T b, typename enable_if<is_same<T,bool>::value>::type* = nullptr ) : internal( b ), Type( Class::Boolean ){}

        template <typename T>
        JSON( T i, typename enable_if<is_integral<T>::value && !is_same<T,bool>::value>::type* = nullptr ) : internal( long(i) ), Type( Class::Integral ){}

        template <typename T>
        JSON( T f, typename enable_if<is_floating_point<T>::value>::type* = nullptr ) : internal( double(f) ), Type( Class::Floating ){}

        template <typename T>
        JSON( T s, typename enable_if<is_convertible<T,string>::value>::type* = nullptr ) : internal( string( s ) ), Type( Class::String ){}

        JSON( std::nullptr_t ) : Type( Class::Null ){}

        static JSON Make( Class type ) {
            return JSON(type);
        }

        static JSON Load( const string & );

        template <typename T>
        void append( T arg ) {
            SetType( Class::Array ); internal.List->emplace_back( arg );
        }

        template <typename T, typename... U>
        void append( T arg, U... args ) {
            append( arg ); append( args... );
        }

        template <typename T>
            typename enable_if<is_same<T,bool>::value, JSON&>::type operator=( T b ) {
                SetType( Class::Boolean ); internal.Bool = b; return *this;
            }

        template <typename T>
            typename enable_if<is_integral<T>::value && !is_same<T,bool>::value, JSON&>::type operator=( T i ) {
                SetType( Class::Integral ); internal.Int = i; return *this;
            }

        template <typename T>
            typename enable_if<is_floating_point<T>::value, JSON&>::type operator=( T f ) {
                SetType( Class::Floating ); internal.Float = f; return *this;
            }

        template <typename T>
            typename enable_if<is_convertible<T,string>::value, JSON&>::type operator=( T s ) {
                SetType( Class::String ); *internal.String = string( s ); return *this;
            }

        JSON& operator[]( const string &key ) {
            SetType( Class::Object ); return internal.Map->operator[]( key );
        }

        JSON& operator[]( const size_t index ) {
            SetType( Class::Array );
            if( index >= internal.List->size() ) internal.List->resize( index + 1 );
            return internal.List->operator[]( index );
        }

        JSON &at( const string &key ) {
            return operator[]( key );
        }

        const JSON &at( const string &key ) const {
            return internal.Map->at( key );
        }

        JSON &at( unsigned index ) {
            return operator[]( index );
        }

        const JSON &at( unsigned index ) const {
            return internal.List->at( index );
        }

        int length() const {
            if( Type == Class::Array )
                return static_cast<int>(internal.List->size());
            else
                return -1;
        }

        bool hasKey( const string &key ) const {
            if( Type == Class::Object )
                return internal.Map->find( key ) != internal.Map->end();
            return false;
        }

        int size() const {
            if( Type == Class::Object )
                return static_cast<int>(internal.Map->size());
            else if( Type == Class::Array )
                return static_cast<int>(internal.List->size());
            else
                return -1;
        }

        Class JSONType() const { return Type; }

        /// Functions for getting primitives from the JSON object.
        bool IsNull() const { return Type == Class::Null; }

        string ToString() const { bool b; return ToString( b ); }
        string ToString( bool &ok ) const {
            ok = (Type == Class::String);
            return ok ? *internal.String : string("");
        }

        double ToFloat() const { bool b; return ToFloat( b ); }
        double ToFloat( bool &ok ) const {
            ok = (Type == Class::Floating);
            return ok ? internal.Float : 0.0;
        }

        long ToInt() const { bool b; return ToInt( b ); }
        long ToInt( bool &ok ) const {
            ok = (Type == Class::Integral);
            return ok ? internal.Int : 0;
        }

        bool ToBool() const { bool b; return ToBool( b ); }
        bool ToBool( bool &ok ) const {
            ok = (Type == Class::Boolean);
            return ok ? internal.Bool : false;
        }

        JSONWrapper<map<string,JSON>> ObjectRange() {
            if( Type == Class::Object )
                return JSONWrapper<map<string,JSON>>( internal.Map.get() );
            return JSONWrapper<map<string,JSON>>( nullptr );
        }

        JSONWrapper<deque<JSON>> ArrayRange() {
            if( Type == Class::Array )
                return JSONWrapper<deque<JSON>>( internal.List.get() );
            return JSONWrapper<deque<JSON>>( nullptr );
        }

        JSONConstWrapper<map<string,JSON>> ObjectRange() const {
            if( Type == Class::Object )
                return JSONConstWrapper<map<string,JSON>>( internal.Map.get() );
            return JSONConstWrapper<map<string,JSON>>( nullptr );
        }


        JSONConstWrapper<deque<JSON>> ArrayRange() const { 
            if( Type == Class::Array )
                return JSONConstWrapper<deque<JSON>>( internal.List.get() );
            return JSONConstWrapper<deque<JSON>>( nullptr );
        }

        string dump( int depth = 1, string tab = "  ") const {
            switch( Type ) {
                case Class::Null:
                    return "null";
                case Class::Object: {
                    string pad = "";
                    for( int i = 0; i < depth; ++i, pad += tab );

                    string s = "{\n";
                    bool skip = true;
                    for( auto &p : *internal.Map ) {
                        if( !skip ) s += ",\n";
                        s += ( pad + "\"" + p.first + "\" : " + p.second.dump( depth + 1, tab ) );
                        skip = false;
                    }
                    s += ( "\n" + pad.erase( 0, 2 ) + "}" ) ;
                    return s;
                }
                case Class::Array: {
                    string s = "[";
                    bool skip = true;
                    for( auto &p : *internal.List ) {
                        if( !skip ) s += ", ";
                        s += p.dump( depth + 1, tab );
                        skip = false;
                    }
                    s += "]";
                    return s;
                }
                case Class::String:
                    return "\"" + json_escape( *internal.String ) + "\"";
                case Class::Floating:
                    return std::to_string( internal.Float );
                case Class::Integral:
                    return std::to_string( internal.Int );
                case Class::Boolean:
                    return internal.Bool ? "true" : "false";
            }

            throw std::runtime_error("Unhandled JSON type");
        }

        friend std::ostream& operator<<( std::ostream&, const JSON & );

    private:
        void SetType( Class type ) {
            if( type == Type )
                return;

            internal.Map.reset();
            internal.List.reset();
            internal.String.reset();

            switch( type ) {
              case Class::Object:    internal.Map    = std::make_unique<std::map<string,JSON>>(); break;
              case Class::Array:     internal.List   = std::make_unique<std::deque<JSON>>();      break;
              case Class::String:    internal.String = std::make_unique<std::string>();           break;
              case Class::Floating:  internal.Float  = 0.0;                    break;
              case Class::Integral:  internal.Int    = 0;                      break;
              case Class::Boolean:   internal.Bool   = false;                  break;
              case Class::Null:      break;
            }

            Type = type;
        }

    private:
        Class Type;
};

inline JSON Array() {
    return JSON::Make( JSON::Class::Array );
}

template <typename... T>
inline JSON Array( T... args ) {
    JSON arr = JSON::Make( JSON::Class::Array );
    arr.append( args... );
    return arr;
}

inline JSON Object() {
    return JSON::Make( JSON::Class::Object );
}

inline std::ostream& operator<<( std::ostream &os, const JSON &json ) {
    os << json.dump();
    return os;
}

namespace {
    JSON parse_next( const string &, size_t & );

    void consume_ws( const string &str, size_t &offset ) {
        while( isspace( str[offset] ) ) ++offset;
    }

    JSON parse_object( const string &str, size_t &offset ) {
        JSON Object = JSON::Make( JSON::Class::Object );

        ++offset;
        consume_ws( str, offset );
        if( str[offset] == '}' ) {
            ++offset; return Object;
        }

        for (;offset<str.size();) {
            JSON Key = parse_next( str, offset );
            consume_ws( str, offset );
            if( str[offset] != ':' ) {
                throw std::runtime_error(std::string("JSON ERROR: Object: Expected colon, found '") + str[offset] + "'\n");
            }
            consume_ws( str, ++offset );
            JSON Value = parse_next( str, offset );
            Object[Key.ToString()] = Value;

            consume_ws( str, offset );
            if( str[offset] == ',' ) {
                ++offset; continue;
            }
            else if( str[offset] == '}' ) {
                ++offset; break;
            }
            else {
                throw std::runtime_error(std::string("JSON ERROR: Object: Expected comma, found '") + str[offset] + "'\n");
            }
        }

        return Object;
    }

    JSON parse_array( const string &str, size_t &offset ) {
        JSON Array = JSON::Make( JSON::Class::Array );
        unsigned index = 0;

        ++offset;
        consume_ws( str, offset );
        if( str[offset] == ']' ) {
            ++offset; return Array;
        }

        for (;offset < str.size();) {
            Array[index++] = parse_next( str, offset );
            consume_ws( str, offset );

            if( str[offset] == ',' ) {
                ++offset; continue;
            }
            else if( str[offset] == ']' ) {
                ++offset; break;
            }
            else {
              throw std::runtime_error(std::string("JSON ERROR: Array: Expected ',' or ']', found '") + str[offset] + "'\n");
            }
        }

        return Array;
    }

    JSON parse_string( const string &str, size_t &offset ) {
        string val;
        for( char c = str[++offset]; c != '\"' ; c = str[++offset] ) {
            if( c == '\\' ) {
                switch( str[ ++offset ] ) {
                case '\"': val += '\"'; break;
                case '\\': val += '\\'; break;
                case '/' : val += '/' ; break;
                case 'b' : val += '\b'; break;
                case 'f' : val += '\f'; break;
                case 'n' : val += '\n'; break;
                case 'r' : val += '\r'; break;
                case 't' : val += '\t'; break;
                case 'u' : {
                    val += "\\u" ;
                    for( unsigned i = 1; i <= 4; ++i ) {
                        c = str[offset+i];
                        if( (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F') )
                            val += c;
                        else {
                            throw std::runtime_error(std::string("JSON ERROR: String: Expected hex character in unicode escape, found '") + c + "'");
                        }
                    }
                    offset += 4;
                } break;
                default  : val += '\\'; break;
                }
            }
            else
                val += c;
        }
        ++offset;
        return JSON(val);
    }

    JSON parse_number( const string &str, size_t &offset ) {
        JSON Number;
        string val, exp_str;
        char c = '\0';
        bool isDouble = false;
        long exp = 0;
        for (; offset < str.size() ;) {
            c = str[offset++];
            if( (c == '-') || (c >= '0' && c <= '9') )
                val += c;
            else if( c == '.' ) {
                val += c; 
                isDouble = true;
            }
            else
                break;
        }
        if( offset < str.size() && (c == 'E' || c == 'e' )) {
            c = str[ offset++ ];
            if( c == '-' ) { exp_str += '-';}
            else if( c == '+' ) { }
            else --offset;

            for (; offset < str.size() ;) {
                c = str[ offset++ ];
                if( c >= '0' && c <= '9' )
                    exp_str += c;
                else if( !isspace( c ) && c != ',' && c != ']' && c != '}' ) {
                    throw std::runtime_error(std::string("JSON ERROR: Number: Expected a number for exponent, found '") + c + "'");
                }
                else
                    break;
            }
            exp = chaiscript::parse_num<long>( exp_str );
        }
        else if( offset < str.size() && (!isspace( c ) && c != ',' && c != ']' && c != '}' )) {
            throw std::runtime_error(std::string("JSON ERROR: Number: unexpected character '") + c + "'");
        }
        --offset;

        if( isDouble )
            Number = chaiscript::parse_num<double>( val ) * std::pow( 10, exp );
        else {
            if( !exp_str.empty() )
                Number = static_cast<double>(chaiscript::parse_num<long>( val )) * std::pow( 10, exp );
            else
                Number = chaiscript::parse_num<long>( val );
        }
        return Number;
    }

    JSON parse_bool( const string &str, size_t &offset ) {
        JSON Bool;
        if( str.substr( offset, 4 ) == "true" ) {
            offset += 4;
            Bool = true;
        } else if( str.substr( offset, 5 ) == "false" ) {
            offset += 5;
            Bool = false;
        } else {
            throw std::runtime_error(std::string("JSON ERROR: Bool: Expected 'true' or 'false', found '") + str.substr( offset, 5 ) + "'");
        }
        return Bool;
    }

    JSON parse_null( const string &str, size_t &offset ) {
        if( str.substr( offset, 4 ) != "null" ) {
            throw std::runtime_error(std::string("JSON ERROR: Null: Expected 'null', found '") + str.substr( offset, 4 ) + "'");
        }
        offset += 4;
        return JSON();
    }

    JSON parse_next( const string &str, size_t &offset ) {
        char value;
        consume_ws( str, offset );
        value = str[offset];
        switch( value ) {
            case '[' : return parse_array( str, offset );
            case '{' : return parse_object( str, offset );
            case '\"': return parse_string( str, offset );
            case 't' :
            case 'f' : return parse_bool( str, offset );
            case 'n' : return parse_null( str, offset );
            default  : if( ( value <= '9' && value >= '0' ) || value == '-' )
                           return parse_number( str, offset );
        }
        throw std::runtime_error(std::string("JSON ERROR: Parse: Unexpected starting character '") + value + "'");
    }
}

inline JSON JSON::Load( const string &str ) {
    size_t offset = 0;
    return parse_next( str, offset );
}

} // End Namespace json


#endif 
