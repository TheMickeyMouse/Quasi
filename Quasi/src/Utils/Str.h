#pragma once
#include "Continuous.h"

namespace Quasi {
    namespace Text {
        struct StringWriter;
    }

    namespace Iter {
        struct LinesIter;
    }

    struct Str;
    struct StrMut;
    struct String;
    struct CStr;

    template <class... Ts> struct Tuple;

    /// Provides an interface to manipulate and test individual characters @p char .
    namespace Chr {
        /// Converts a numeric value @p digit in the range 0-9
        /// into a character corresponding to that digit ('0'-'9').
        /// @warning Returns bad values for @code digit >= 10@endcode. For checked calls, use @p TryFromDigit .
        char FromDigit(u32 digit);
        /// Converts a hexidecimal value @p digit in the range 0-15
        /// into a character corresponding to that hexidecimal digit ('0'-'9'; 'A'-'F').
        /// @warning Returns bad values for @code digit >= 16@endcode. For checked calls, use @p TryFromHexDigit .
        char FromHexDigit(u32 hex);
        /// Converts an ASCII character @p c in the range '0'-'9'
        /// into the numeric value corresponding to that character (0-9).
        /// @warning Returns bad values for @p c @b not in the valid range. For checked calls, use @p TryToDigit .
        u32 ToDigit(char c);
        /// Converts an ASCII character @p c in the range '0'-'9'; 'A'-'F'; 'a-f'
        /// into the hexidecimal value corresponding to that character (0-16).
        /// @warning Returns bad values for @p c @b not in the valid range. For checked calls, use @p TryToHexDigit .
        u32 ToHexDigit(char c);

        /// Converts a numeric value @p digit in the range 0-9
        /// into a character corresponding to that digit ('0'-'9').
        /// Returns null for values not in the valid range.
        Option<char> TryFromDigit(u32 digit);
        /// Converts a hexidecimal value @p digit in the range 0-15
        /// into a character corresponding to that hexidecimal digit ('0'-'9'; 'A'-'F').
        /// Returns null for values not in the valid range.
        Option<char> TryFromHexDigit(u32 digit);
        /// Converts a numeric value @p digit into the corresponding character under base @p radix .
        /// Returns null for values not in the valid range.
        /// @b Example: @code
        /// TryFromDigitRadix(0, 2);   // Some('0'), binary
        /// TryFromDigitRadix(6, 8);   // Some('6'), octal
        /// TryFromDigitRadix(12, 10); // None,      decimal but outside range
        /// TryFromDigitRadix(12, 16); // Some('B'), hexidecimal
        /// TryFromDigitRadix(24, 36); // Some('M'), base-36, max.
        /// @endcode
        Option<char> TryFromDigitRadix(u32 digit, u32 radix);

        /// Converts an ASCII character @p c in the range '0'-'9'
        /// into the numeric value corresponding to that character (0-9).
        /// Returns null for characters not in the valid range.
        Option<u32> TryToDigit(char c);
        /// Converts an ASCII character @p c in the range '0'-'9'; 'A'-'F'; 'a-f'
        /// into the hexidecimal value corresponding to that character (0-16).
        /// Returns null for characters not in the valid range.
        Option<u32> TryToHexDigit(char c);
        /// Converts a character @p digit into the corresponding numeric value under base @p radix .
        /// Returns null for characters not in the valid range.
        /// @b Example: @code
        /// TryToDigitRadix('0', 2);  // Some(0),  binary
        /// TryToDigitRadix('6', 8);  // Some(6),  octal
        /// TryToDigitRadix('B', 10); // None,     decimal but outside range
        /// TryToDigitRadix('B', 16); // Some(12), hexidecimal
        /// TryToDigitRadix('M', 36); // Some(24), base-36, max.
        /// @endcode
        Option<u32> TryToDigitRadix(char c, u32 radix);

        /// Checks if @p c is a numeric digit character, i.e. '0'-'9'.
        bool IsDigit(char c);
        /// Checks if @p c is a hexidecimal digit character, i.e. '0'-'9', 'A'-'F', or 'a-f'.
        bool IsHexDigit(char c);
        /// Checks if @p c is a valid digit character in base @p radix . Specifically:
        /// - if @code radix <= 10@endcode, returns @p true if @p c is in the range '0'-'N' where @code N = radix - 1@endcode;
        /// - else returns @p true if @p c is in any of '0'-'9', 'A'-'$', or 'a'-'$', where @code $ = 'A' + radix - 11@endcode.
        bool IsDigitRadix(char c, u32 radix);

        /// Returns the corresponding escape code for @p c (ex: returns 'n' for the newline character @p '\n'),
        /// and null if the character doesn't need escaping or is not one of:
        /// @p '\'', @p '\"', @p '\\', @p '\a', @p '\b', @p '\f', @p '\n', @p '\r', @p '\t', or @p '\v'.
        Option<char> EscapeRepr(char c);
        /// Writes the escape sequence for the character @p c into the string buffer @p out,
        /// and returns the number of characters written.
        ///
        /// Prefers shortened escape codes like @p '\n' or @p '\t',
        /// but if outside the standard control character range,
        /// will use @p '\xNN' where N is a hexidecimal digit.
        ///
        /// Writes the character directly if no escpaing is required, i.e. the character is printable.
        /// @note Never writes more than 4 characters.
        usize WriteEscape(char c, char* out);
        /// The inverse of @p EscapeRepr, which turns an escape code @p c into its actual character
        /// (ex: returns @p '\n' for the character 'n').
        /// Returns null if no corresponding escape code exists.
        Option<char> UnescapeRepr(char c);

        /// Converts the character @p c into its lowercase variant
        /// if @p c is an uppercase alphabetic character, does nothing otherwise.
        char ToLower(char c);
        /// Converts the character @p c into its uppercase variant
        /// if @p c is a lowercase alphabetic character, does nothing otherwise.
        char ToUpper(char c);
        /// Checks if @p c is a lowercase alphabetic character, i.e. in the range 'a'-'z'.
        bool IsLower(char c);
        /// Checks if @p c is an uppercase alphabetic character, i.e. in the range 'A'-'Z'.
        bool IsUpper(char c);

        /// Checks if @p c is a 'blank' character, aka space @code ' '@endcode (@p 0x20) or tab @p '\\t' (@p 0x09).
        bool IsBlank(char c);
        /// Checks if @p c is a 'whitespace' character. Whitespace characters are specified as either:
        /// - Space @code ' '@endcode (@p 0x20)
        /// - Form feed @p '\\f' (@p 0x0c)
        /// - Newline @p '\\n' (@p 0x0a)
        /// - Carriage return @p '\\r' (@p 0x0d)
        /// - Tab @p '\\t' (@p 0x09)
        /// - or Vertical tab @p '\\v' (@p 0x0b)
        bool IsWhitespace(char c);
        /// Checks if @p c is an alphabetic character, i.e. one of 'A'-'Z' or 'a'-'z'.
        bool IsAlpha(char c);
        /// Checks if @p c is an alphanumeric character, i.e. one of '0'-'9', 'A'-'Z', or 'a'-'z'.
        bool IsAlphaNum(char c);
        /// Checks if @p c is a punctuation character, i.e. one of @code !\"#$%&'()*+,-./:;<=>?\@[\\]^_`{|}~@endcode.
        bool IsPunct(char c);
        /// Checks if @p c is a 'printable' character, i.e. either alphanumeric, punctuation or the space character.
        /// Identical to the range @p 0x1f~0x7f .
        bool IsPrintable(char c);
        /// Checks if @p c is a control character, i.e. those in the range @p 0x00~0x1f or @p 0x7f .
        bool IsControl(char c);
        /// Checks if @p c is a 'graphical' character, i.e. either alphanumeric or punctuation, but @b not the space character.
        bool IsGraphic(char c);

        struct Set {
            u64 bitmask[4] {};
            Set(Span<const char> chars);
            bool operator()(char c) const;
            bool operator()(Str str) const;
        };
    }

    /// The base class for continuous string types. The string type that implements
    /// this base class should be convertible to a @p Str, the most generic string holder.
    ///
    /// To create and establish a string type, one should write the following: @code
    /// struct MyString : StringHolder<MyChar, MyString> { ... }
    /// @endcode
    /// and implement the following methods:
    /// @code
    /// MyChar* DataImpl() { ... }
    /// const MyChar* DataImpl() const { ... }
    /// usize LengthImpl() const { ... }
    /// @endcode
    /// @tparam Char The character type. <b>Const-ness is significant;</b>
    /// and should only be @p char or @code const char@endcode.
    /// @tparam Super The string class that shall implement @p StringHolder .
    template <class Char, class Super>
    struct StringHolder : IContinuous<Char, Super> {
        friend IContinuous<Char, Super>;
        using StrChar = IfElse<SameAs<Char, char>, StrMut, Str>;
    private:
        Super& super() { return *static_cast<Super*>(this); }
        const Super& super() const { return *static_cast<const Super*>(this); }
    public:
        /// Hashes the string using all of its characters.
        /// Useful for using a string as a key into a @p HashMap .
        Hashing::Hash GetHashCode() const;

        // Utf8CharsIter Utf8Chars() const;
        // SplitWhitespaceIter SplitWhitespace() const;

        /// Returns an iterator which yields the individual string chunks
        /// after being split by separator @p sep.
        /// @b Example: @code
        /// Str text = "It was here somewhere..."
        /// for (const Str chunk : text.Split("er")) {
        ///     Text::PrintLn("{}", chunk);
        /// }
        /// // prints:
        /// // It was h
        /// // e somewh
        /// // e...
        /// @endcode
        Iter::SplitIter<Str> Split(Str sep) const;
        /// Returns an iterator which yields the individual lines in a string.
        /// Uses the new line character as the separator (@p '\n').
        Iter::LinesIter Lines() const;
        /// Counts the number of lines in the string.
        /// Identical to @code CountChars('\n') + 1@endcode.
        usize CountLines() const;
        usize CountChars(Char c) const;

        /// Counts the number of UTF-8 codepoints within the string.
        usize Utf8Length() const;

        /// Converts the string into a string slice @p Str .
        Str AsStr() const;
        /// Converts the string into a mutable string slice @p StrMut .
        StrMut AsStrMut() const requires IsMut<Char>;
        /// Converts the string into a string slice @p Str .
        operator Str() const;
        /// Converts the string into a mutable string slice @p StrMut .
        explicit operator StrMut() const requires IsMut<Char>;

        /// Slices the string into a view of the characters in the index range @code [start..len]@endcode.
        /// @warning UB if @p start is greater than the length.
        StrChar Substr(usize start) const;
        /// Slices the string into a view of the characters in the index range @code [start..start+count]@endcode.
        /// @warning UB if @code start + count@endcode is greater than the length.
        StrChar Substr(usize start, usize count) const;

        /// Returns the first character in the string.
        /// @warning UB if the string is empty.
        Char& First() const;
        /// Returns the last character in the string.
        /// @warning UB if the string is empty.
        Char& Last() const;

        /// Returns the first @p num characters in the form of a string slice.
        /// @warning UB if @p num is greater than the length.
        StrChar First(usize num) const;
        /// Returns the characters after @p len in the form of a string slice.
        /// @warning UB if @p len is greater than the length.
        StrChar Skip(usize len) const;
        /// Returns all the characters @em except the first one in the form of a string slice.
        /// @warning UB if the string is empty.
        StrChar Tail() const;
        /// Returns the last @p num characters in the form of a string slice.
        /// @warning UB if @p num is greater than the length.
        StrChar Last(usize num) const;
        /// Returns all the characters @em except the last @p len characters in the form of a string slice.
        /// @warning UB if @p len is greater than the length.
        StrChar Trunc(usize len) const;
        /// Returns all the characters @em except the last one in the form of a string slice.
        /// @warning UB if the string is empty.
        StrChar Init() const;


        /// Returns the first character and trailing string slice in the form of a tuple.
        ///
        /// @b Example: @code
        /// const auto [first, tail] = Str("Hello!").SplitFirst(); // first = 'H', tail = "ello!"
        /// @endcode
        /// @warning UB if the string is empty.
        Tuple<Char&, StrChar> SplitFirst() const;
        /// Returns the initial string slice and the last character in the form of a tuple.
        ///
        /// @b Example: @code
        /// const auto [init, last] = Str("Hello!").SplitLast(); // init = "Hello", last = '!'
        /// @endcode
        /// @warning UB if the string is empty.
        Tuple<StrChar, Char&> SplitLast() const;
        /// Returns the string slices @em before and @em after (inclusive) the index @p i in the form of a tuple,
        /// i.e. the string slice at @code [0..i]@endcode and @code [i..len]@endcode.
        ///
        /// @b Example: @code
        /// const auto [left, right] = Str("Hello!").CutAt(3); // init = "Hel", last = "lo!"
        /// @endcode
        /// @warning UB if @p i is greater than the length.
        Tuple<StrChar, StrChar> CutAt(usize i) const;
        /// Returns the string slices @em before and @em after the index @p i in the form of a tuple
        /// and discards the character at index @p i,
        /// i.e. the string slice at @code [0..i]@endcode and @code [i+1..len]@endcode.
        ///
        /// @b Example: @code
        /// const auto [left, right] = Str("Hello!").SplitAt(3); // init = "Hel", last = "o!"
        /// @endcode
        /// @warning UB if @p i is greater or equal than the length.
        Tuple<StrChar, StrChar> SplitAt(usize i) const;
        /// Returns the string slices @em before and @em after (non-inclusive) the index @p i in the form of a tuple
        /// as well as the character at index @p i.
        /// i.e. the left string slice at @code [0..i]@endcode, the character at @p i
        /// and the right string slice @code [i+1..len]@endcode.
        ///
        /// @b Example: @code
        /// const auto [left, mid, right] = Str("Hello!").PartitionAt(3); // init = "Hel", mid = 'l', last = "o!"
        /// @endcode
        /// @warning UB if @p i is greater or equal than the length.
        Tuple<StrChar, Char&, StrChar> PartitionAt(usize i) const;

        /// Splits the string at the @em first instance of a character
        /// that passes the predicate @p pred in the form of a tuple.
        /// Returns the full string and the empty string if no character passed the predicate.
        ///
        /// @b Example: @code
        /// const auto [left, right] = Str("What's up man").SplitOnceOn(Chr::IsBlank); // left = "What's", right = "up man"
        /// const auto [full, empty] = Str("What's up man").SplitOnceOn(Chr::IsDigit); // full = "What's up man", empty = ""
        /// @endcode
        Tuple<StrChar, StrChar> SplitOnceOn(Predicate<Char> auto&& pred) const;
        /// Splits the string at the @em last instance of a character
        /// that passes the predicate @p pred in the form of a tuple.
        /// Returns the empty string and the full string if no character passed the predicate.
        ///
        /// @b Example: @code
        /// const auto [left, right] = Str("What's up man").SplitOnceOn(Chr::IsBlank); // left = "What's up", right = "man"
        /// const auto [empty, full] = Str("What's up man").SplitOnceOn(Chr::IsDigit); // empty = "", full = "What's up man"
        /// @endcode
        Tuple<StrChar, StrChar> RevSplitOnceOn(Predicate<Char> auto&& pred) const;
        /// Splits the string at the @em first occurence of character @p c in the form of a tuple.
        /// Returns the full string and the empty string if the character wasn't found.
        ///
        /// @b Example: @code
        /// const auto [left, right] = Str("What's up man").SplitOnce(' '); // left = "What's", right = "up man"
        /// const auto [full, empty] = Str("What's up man").SplitOnce('?'); // full = "What's up man", empty = ""
        /// @endcode
        Tuple<StrChar, StrChar> SplitOnce(Char c) const;
        /// Splits the string at the @em first occurence of string @p sep in the form of a tuple.
        /// Returns the full string and the empty string if the string wasn't found.
        ///
        /// @b Example: @code
        /// const auto [left, right] = Str("What's up man").SplitOnce("up"); // left = "What's ", right = " man"
        /// const auto [full, empty] = Str("What's up man").SplitOnce(":)"); // full = "What's up man", empty = ""
        /// @endcode
        Tuple<StrChar, StrChar> SplitOnce(Str sep) const;

        /// Checks if the string slice @p other references the same string as this,
        /// i.e. shares the same character address.
        bool RefEquals(Str other) const;
        /// Checks if the string slice @p other references a substring of this,
        /// i.e. lives somewhere in the string buffer.
        ///
        /// @b Example: @code
        /// const char* s = "abcdefg", * s2 = "bcd";
        /// Str sub = Str(s).Substr(1, 3);  // sub = "bcd"
        /// sub == Str(s2);                 // true, contents same
        /// Str(s).ContainsBuffer(sub);     // true, sub originated from s
        /// Str(s).ContainsBuffer(Str(s2)); // false, same string diff address
        /// @endcode
        bool ContainsBuffer(Str buf) const;
        /// Checks if the string slice @p other overlaps with the string memory chunk of this,
        /// i.e. shares memory in the string buffer.
        ///
        /// @b Example: @code
        /// const char* s = "abcdefg", * s2 = "cdef";
        /// Str x = Str(s).Substr(1, 3), y = Str(s).Substr(2, 4); // x = "bcd", y = "cdef"
        /// y == Str(s2);                 // true, contents same
        /// x.ContainsBuffer(y);          // true, x and y share the memory chunk "cd"
        /// x.ContainsBuffer(Str(s2));    // false, shares string but diff address
        /// @endcode
        bool OverlapsBuffer(Str buf) const;

        /// Checks for string equality.
        bool Equals(Str other) const;
        /// Checks for string equality, ignoring upper/lowercase.
        bool EqualsIgnoreCase(Str other) const;
        /// Checks for string equality. Same as @p Equals .
        bool operator==(Str other) const;
        /// Checks for string equality. Same as @p Equals .
        bool operator==(const String& other) const;
        /// Checks for string equality. Same as @p Equals .
        bool operator==(const Char* other) const;
        /// Checks for string equality. Same as @p Equals .
        template <usize N> bool operator==(const Char other[N]) const { return Equals(other); }

        /// Compares two strings against each other. Provides a total ordering.
        /// @warning The comparison results of strings is not guarenteed to be the same across platforms.
        /// @note If @p a is longer than @p b, then @code a.Cmp(b)@endcode is @b guarenteed to be greater.
        Comparison Cmp(Str other) const;
        /// Identical to @p Cmp .
        Comparison operator<=>(Str other) const;

        /// Reverses the contents of the string. (requires the string to be mutable).
        void Reverse() requires IsMut<Char>;

        /// Finds and returns the index of the beginning of the first instance of @p str in the string.
        OptionUsize Search(Str str) const;
        /// Finds and returns the index of the beginning of the last instance of @p str in the string.
        OptionUsize RevSearch(Str str) const;
        /// Checks if @p str is contained/can be found in the string.
        bool Contains(Str str) const;
        /// Checks if @p c is contained/can be found in the string.
        bool Contains(char c) const;

        /// Finds the first occurence of one of the characters in the character sequence @p anyc,
        /// while also returning what character was found.
        ///
        /// @b Example: @code
        /// Str("how's life").FindOneOf("aeiou"); // (1, 'o')
        /// Str("physical")  .FindOneOf("aeiou"); // (4, 'i')
        /// Str("rhythms")   .FindOneOf("aeiou"); // (None, None)
        /// @endcode
        Tuple<OptionUsize, OptRef<const Char>> FindOneOf(Str anyc) const;
        /// Finds the last occurence of one of the characters in the character sequence @p anyc,
        /// while also returning what character was found.
        ///
        /// @b Example: @code
        /// Str("how's life").FindOneOf("aeiou"); // (9, 'e')
        /// Str("physical")  .FindOneOf("aeiou"); // (6, 'a')
        /// Str("rhythms")   .FindOneOf("aeiou"); // (None, None)
        /// @endcode
        Tuple<OptionUsize, OptRef<const Char>> RevFindOneOf(Str anyc) const;

        /// Finds the first occurence of one of the strings in the string sequence @p anystr,
        /// while also returning what string was found.
        ///
        /// @b Example: @code
        /// Str("this is great") .FindOneOf({ "very", "great" }); // (8, "great")
        /// Str("not bad")       .FindOneOf({ "very", "great" }); // (None, None)
        /// Str("very very cool").FindOneOf({ "very", "great" }); // (0, "very")
        /// @endcode
        Tuple<OptionUsize, OptRef<const Str>> FindOneOf(Span<const Str> anystr) const;
        /// Finds the last occurence of one of the strings in the string sequence @p anystr,
        /// while also returning what string was found.
        ///
        /// @b Example: @code
        /// Str("this is great") .FindOneOf({ "very", "great" }); // (8, "great")
        /// Str("not bad")       .FindOneOf({ "very", "great" }); // (None, None)
        /// Str("very very cool").FindOneOf({ "very", "great" }); // (5, "very")
        /// @endcode
        Tuple<OptionUsize, OptRef<const Str>> RevFindOneOf(Span<const Str> anystr) const;

        /// Checks if the string starts with a character @p prefix .
        bool StartsWith(Char prefix) const;
        /// Checks if the string ends with a character @p suffix .
        bool EndsWith(Char suffix) const;
        /// Checks if the string starts with a string @p prefix .
        bool StartsWith(Str prefix) const;
        /// Checks if the string ends with a string @p suffix .
        bool EndsWith(Str suffix) const;

        /// Trims the starting and ending characters as long as they match the predicate @p pred .
        ///
        /// @b Example: @code
        /// Str("abc1234de").TrimIf(Chr::IsAlpha); // "1234"
        /// @endcode
        StrChar TrimIf(Predicate<Char> auto&& pred) const;
        /// Trims the starting characters as long as they match the predicate @p pred .
        ///
        /// @b Example: @code
        /// Str("abc1234de").TrimStartIf(Chr::IsAlpha); // "1234de"
        /// @endcode
        StrChar TrimStartIf(Predicate<Char> auto&& pred) const;
        /// Trims the ending characters as long as they match the predicate @p pred .
        ///
        /// @b Example: @code
        /// Str("abc1234de").TrimEndIf(Chr::IsAlpha); // "abc1234"
        /// @endcode
        StrChar TrimEndIf(Predicate<Char> auto&& pred) const;
        /// Trims all the starting and ending characters that are equal to @p c .
        /// By default, trims spaces.
        ///
        /// @b Example: @code
        /// Str("___hi__").Trim('_'); // "hi"
        /// @endcode
        StrChar Trim(Char c = ' ') const;
        /// Trims all the starting characters that are equal to @p c .
        /// By default, trims spaces.
        ///
        /// @b Example: @code
        /// Str("___hi__").Trim('_'); // "hi__"
        /// @endcode
        StrChar TrimStart(Char c = ' ') const;
        /// Trims all the ending characters that are equal to @p c .
        /// By default, trims spaces.
        ///
        /// @b Example: @code
        /// Str("___hi__").Trim('_'); // "___hi"
        /// @endcode
        StrChar TrimEnd(Char c = ' ') const;

        /// Returns a string slice where the character prefix is removed.
        StrChar RemovePrefix(Char prefix) const;
        /// Returns a string slice where the character suffix is removed.
        StrChar RemoveSuffix(Char suffix) const;
        /// Returns a string slice where the prefix string is removed.
        StrChar RemovePrefix(Str prefix) const;
        /// Returns a string slice where the suffix string is removed.
        StrChar RemoveSuffix(Str suffix) const;
        /// Returns a string slice where the @em one of character prefixes chosen from a list is removed,
        /// as well as what prefix was removed.
        ///
        /// @b Example: @code
        /// Str("ions")  .RemovePrefixOneOf("aeiou"); // ("ons",    'o')
        /// Str("yellow").RemovePrefixOneOf("aeiou"); // ("yellow", None)
        /// @endcode
        Tuple<StrChar, OptRef<const Char>> RemovePrefixOneOf(Str prefix) const;
        /// Returns a string slice where the @em one of character suffixes chosen from a list is removed,
        /// as well as what suffix was removed.
        ///
        /// @b Example: @code
        /// Str("helloo").RemoveSuffixOneOf("aeiou"); // ("hello", 'o')
        /// Str("heyoh") .RemoveSuffixOneOf("aeiou"); // ("heyoh", None)
        /// @endcode
        Tuple<StrChar, OptRef<const Char>> RemoveSuffixOneOf(Str suffix) const;
        /// Returns a string slice where the @em one of string prefixes chosen from a list is removed,
        /// as well as what prefix was removed.
        ///
        /// @b Example: @code
        /// Str("the dog")   .RemovePrefixOneOf({ "the ", "a ", "an " }); // ("dog",        "the ")
        /// Str("an animal") .RemovePrefixOneOf({ "the ", "a ", "an " }); // ("animal",     "an ")
        /// Str("one person").RemovePrefixOneOf({ "the ", "a ", "an " }); // ("one person", None)
        /// @endcode
        Tuple<StrChar, OptRef<const Str>> RemovePrefixOneOf(Span<const Str> prefix) const;
        /// Returns a string slice where the @em one of string suffixes chosen from a list is removed,
        /// as well as what suffix was removed.
        ///
        /// @b Example: @code
        /// Str("dogs")  .RemoveSuffixOneOf({ "es", "s" }); // ("dog",  "s")
        /// Str("fishes").RemoveSuffixOneOf({ "es", "s" }); // ("fish", "es")
        /// Str("mice")  .RemoveSuffixOneOf({ "es", "s" }); // ("mice", None)
        /// @endcode
        Tuple<StrChar, OptRef<const Str>> RemoveSuffixOneOf(Span<const Str> suffix) const;

        // SplitIfIter           SplitIf(Fn<bool, char> pred)
        // SplitIfInclIter       SplitIfIncl(Fn<bool, char> pred)
        // RevSplitIter          RevSplit(const char sep)
        // RevSplitIfIter        RevSplitIf(Fn<bool, char> pred)
        // SplitAtmostIfIter     SplitIfAtmost(usize maxLen, Fn<bool, char, char> pred)
        // RevSplitAtmostIfIter  RevSplitIfAtmost(usize maxLen, Fn<bool, char, char> pred)
        // SplitTermIter         SplitTerms()
        // blah blah
        // MatchIter             Matches(...)
        // MatchIndiciesIter     MatchesIndices(...)

        // EscapeIter Escaped();
        // EscapeUtf8Iter EscapedUtf8();

        /// Constructs and allocates a new memory-owning string,
        /// whose contents are copied from this string.
        String ToString() const;
        /// Constructs and allocates a new memory-owning string,
        /// whose contents is the @em reverse of that from this string.
        String Reversed() const;
        /// Constructs and allocates a new memory-owning string,
        /// whose contents is the uppercase version of this string.
        ///
        /// @b Example: @code
        /// String greet = Str("What's up?").ToUpper(); // greet = "WHAT'S UP?"
        /// @endcode
        String ToUpper() const;
        /// Constructs and allocates a new memory-owning string,
        /// whose contents is the lowercase version of this string.
        ///
        /// @b Example: @code
        /// String greet = Str("What's up?").ToLower(); // greet = "what's up?"
        /// @endcode
        String ToLower() const;
        /// Modifies the string @em in-place, uppercasing all characters in the string.
        void MakeUpper() requires IsMut<Char>;
        /// Modifies the string @em in-place, lowercasing all characters in the string.
        void MakeLower() requires IsMut<Char>;

        /// Constructs and allocates a new memory-owning string,
        // whose contents is a version of string repeated @p n times.
        String Repeat(usize n) const;

        /// Replaces all instances of the character @p from to the string @p to .
        ///
        /// @b Example: @code
        /// String s = Str("I came; I saw; I conquered").Replace('I', "You")
        /// // s = "You came; You saw; You conquered"
        /// @endcode
        String Replace(Char from, Str to) const;
        /// Replaces all instances of the string @p from to the string @p to .
        ///
        /// @b Example: @code
        /// String s = Str("Hover over the button").Replace("over", "eat")
        /// // s = "Heat eat the button"
        /// @endcode
        String Replace(Str from, Str to) const;

        /// Runs the predicate @p pred on increasingly shortened substrings,
        /// and if a length is reported, replaces that many characters with its returning string value.
        ///
        /// @b Example: @code
        /// String s = Str("ABBAB").ReplaceIf([] (Str x, OptionUsize& len) {
        ///     Str result;
        ///     if (x.StartsWith('A'))  { len = 1; result = "foo "; }
        ///     if (x.StartsWith("BB")) { len = 2; result = "bar "; }
        ///     Text::PrintLn("input = {}, result = {}", x, result);
        ///     return result;
        /// });
        /// Text::PrintLn("s = {}")
        /// // prints:
        /// // input = ABBAB, result = "foo "
        /// // input = BBAB, result = "bar "
        /// // input = AB, result = "foo "
        /// // input = B, result = ""
        /// // input = , result = ""
        /// // s = "foo bar foo B"
        /// @endcode
        String ReplaceIf(Fn<Tuple<Str>, Str, OptionUsize&> auto&& pred) const;
    };

    struct Str : StringHolder<const char, Str> {
        friend IContinuous;
        friend StringHolder;
        using StringHolder::operator==;
        using StringHolder::operator<=>;
    private:
        const char* data = nullptr;
        usize size = 0;
        constexpr Str(const char* data, usize size) : data(data), size(size) {}
    protected:
        const char* DataImpl() const { return data; }
        usize LengthImpl() const { return size; }
    public:
        /// Empty string.
        constexpr Str() = default;
        /// Empty string.
        constexpr Str(Nullptr) : Str() {}
        /// Constructs a string slice to the null-terminated string @p zstr .
        /// Computes @p strlen and has time complexity O(n).
        constexpr Str(const char* zstr) : data(zstr) { while (data[size]) ++size; }

        /// Returns the empty string.
        static constexpr Str Empty() { return nullptr; }
        /// Creates a string slice with character buffer @p data and size @p size .
        static constexpr Str Slice(const char* data, usize size) { return { data, size }; }

        /// Advances the string slice by @p num, essentially skipping the first @p n characters in place.
        ///
        /// @b Example: @code
        /// Str s = "abcdefg";
        /// s.Advance(3); // s is now "defg"
        /// @endcode
        /// @warning UB if @p num is greater than the length.
        Str& Advance(usize num);
        /// Shortens the length of the string slice by @p amount, essentially truncating the string slice in place.
        ///
        /// @b Example: @code
        /// Str s = "abcdefg";
        /// s.Shorten(3); // s is now "abcd"
        /// @endcode
        /// /// @warning UB if @p amount is greater than the length.
        Str& Shorten(usize amount);
        /// Returns the string slice of the first @p num characters, while also advancing the string slice.
        ///
        /// @b Example: @code
        /// Str s = "abcdefg";
        /// s.TakeFirst(3); // returns "abc", s is now "defg"
        /// @endcode
        /// @warning UB if @p amount is greater than the length.
        Str TakeFirst(usize num);
        /// Returns the string slice of the last @p num characters, while also shortening the string slice.
        ///
        /// @b Example: @code
        /// Str s = "abcdefg";
        /// s.TakeLast(3); // returns "efg", s is now "abcd"
        /// @endcode
        /// @warning UB if @p amount is greater than the length.
        Str TakeLast(usize num);
        /// Returns the first character, while also advancing the string slice by 1.
        ///
        /// @b Example: @code
        /// Str s = "abcdefg";
        /// s.TakeFirst(); // returns 'a', s is now "bcdefg"
        /// @endcode
        /// @warning UB if the string is empty.
        const char& TakeFirst();
        /// Returns the last character, while also shrinking the string slice by 1.
        ///
        /// @b Example: @code
        /// Str s = "abcdefg";
        /// s.TakeLast(); // returns 'g', s is now "abcdef"
        /// @endcode
        /// @warning UB if the string is empty.
        const char& TakeLast();
        /// Returns the string after @p i, while also shrinking the string slice to @p i .
        ///
        /// @b Example: @code
        /// Str s = "abcdefg";
        /// s.TakeAfter(3); // returns "defg", s is now "abc"
        /// @endcode
        /// @warning UB if @p i is greater than the length.
        Str TakeAfter(usize i);

        /// Creates and allocates an owning-string which contains the escaped string
        /// by replacing non-printable characters with their escape sequence.
        ///
        /// @b Example: @code
        /// String s = Str("hello,\nworld!").Escape() // s = "hello,\\nworld!"
        /// @endcode
        String Escape() const;
        /// Writes the escaped string to @p output by replacing non-printable characters with their escape sequence.
        /// More generic version of @p Escape .
        usize WriteEscape(Text::StringWriter output) const;
        /// The inverse of @p Escape ; tries and replaces the escape sequences in the string
        /// with their ASCII variant, returning null if an invalid or unfinished escape sequence was encountered.
        /// @b Example: @code
        /// String s  = Str("hello,\\nworld!") // s = "hello,\nworld!"
        /// String s2 = Str("hello,\\qworld!") // s2 = None (\q doesnt exist)
        /// @endcode
        Option<String> Unescape() const;

        /// Converts the string to a mutable version.
        /// This function isn't unsafe but breaks const-correctness.
        StrMut AsMut();
    };

    struct StrMut : StringHolder<char, StrMut> {
        friend IContinuous;
        friend StringHolder;
        using StringHolder::operator==;
        using StringHolder::operator<=>;
    private:
        char* data = nullptr;
        usize size = 0;
        StrMut(char* data, usize size) : data(data), size(size) {}
    protected:
        const char* DataImpl() const { return data; }
        char* DataImpl() { return data; }
        usize LengthImpl() const { return size; }
    public:
        /// Empty string.
        StrMut() = default;
        /// Empty string.
        StrMut(Nullptr) : StrMut() {}

        /// Returns the empty string.
        static StrMut Empty() { return nullptr; }
        /// Creates a mutable string slice with character buffer @p data and size @p size .
        static StrMut Slice(char* data, usize size) { return { data, size }; }

        /// Advances the string slice by @p num, essentially skipping the first @p n characters in place.
        ///
        /// @b Example: @code
        /// Str s = "abcdefg";
        /// s.Advance(3); // s is now "defg"
        /// @endcode
        /// @warning UB if @p num is greater than the length.
        StrMut& Advance(usize num);
        /// Shortens the length of the string slice by @p amount, essentially truncating the string slice in place.
        ///
        /// @b Example: @code
        /// Str s = "abcdefg";
        /// s.Shorten(3); // s is now "abcd"
        /// @endcode
        /// /// @warning UB if @p amount is greater than the length.
        StrMut& Shorten(usize amount);
        /// Returns the string slice of the first @p num characters, while also advancing the string slice.
        ///
        /// @b Example: @code
        /// Str s = "abcdefg";
        /// s.TakeFirst(3); // returns "abc", s is now "defg"
        /// @endcode
        /// @warning UB if @p amount is greater than the length.
        StrMut TakeFirst(usize num);
        /// Returns the string slice of the last @p num characters, while also shortening the string slice.
        ///
        /// @b Example: @code
        /// Str s = "abcdefg";
        /// s.TakeLast(3); // returns "efg", s is now "abcd"
        /// @endcode
        /// @warning UB if @p amount is greater than the length.
        StrMut TakeLast(usize num);
        /// Returns the first character, while also advancing the string slice by 1.
        ///
        /// @b Example: @code
        /// Str s = "abcdefg";
        /// s.TakeFirst(); // returns 'a', s is now "bcdefg"
        /// @endcode
        /// @warning UB if the string is empty.
        char& TakeFirst();
        /// Returns the last character, while also shrinking the string slice by 1.
        ///
        /// @b Example: @code
        /// Str s = "abcdefg";
        /// s.TakeLast(); // returns 'g', s is now "abcdef"
        /// @endcode
        /// @warning UB if the string is empty.
        char& TakeLast();
        /// Returns the string after @p i, while also shrinking the string slice to @p i .
        ///
        /// @b Example: @code
        /// Str s = "abcdefg";
        /// s.TakeAfter(3); // returns "defg", s is now "abc"
        /// @endcode
        /// @warning UB if @p i is greater than the length.
        StrMut TakeAfter(usize i);

        Str AsConst() const { return AsStr(); }
    };

    constexpr Str operator ""_str(const char* data, usize size) {
        return Str::Slice(data, size);
    }

#define strdef template <class Char, class Super>
#define strcls StringHolder<Char, Super>
    strdef auto strcls::SplitOnceOn(Predicate<Char> auto&& pred) const -> Tuple<StrChar, StrChar> {
        const OptionUsize i = FindIf(pred);
        return SplitAt(i.UnwrapOr(super().LengthImpl()));
    }
    strdef auto strcls::RevSplitOnceOn(Predicate<Char> auto&& pred) const -> Tuple<StrChar, StrChar> {
        const OptionUsize i = this->RevFindIf(pred);
        return SplitAt(i.UnwrapOr(0));
    }
    strdef auto strcls::TrimIf(Predicate<Char> auto&& pred) const -> StrChar {
        return TrimStartIf(pred).TrimEndIf(pred);
    }
    strdef auto strcls::TrimStartIf(Predicate<Char> auto&& pred) const -> StrChar {
        usize beg = 0;
        while (beg < this->Length() && pred(this->Get(beg))) ++beg;
        return Skip(beg);
    }
    strdef auto strcls::TrimEndIf(Predicate<Char> auto&& pred) const -> StrChar {
        usize end = this->Length();
        while (end --> 0 && pred(this->Get(end))) {}
        return First(end + 1);
    }
#undef strdef
#undef strcls

    template <class T, class Super>
    Str    IContinuous<T, Super>::AsStr() const requires (sizeof(T) == sizeof(char)) { return Str::Slice((const char*)Data(), Length()); }
    template <class T, class Super>
    StrMut IContinuous<T, Super>::AsStrMut() requires (sizeof(T) == sizeof(char)) && IsMut<T> { return StrMut::Slice((char*)Data(), Length()); }
} // Quasi
