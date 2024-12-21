
#pragma once

#include <cstdlib>
#include <set>
#include "types.h"

#ifndef MSDF_ATLAS_PUBLIC
#define MSDF_ATLAS_PUBLIC
#endif

namespace msdf_atlas {

/// Represents a set of Unicode codepoints (characters)
class Charset {

public:
    /// The set of the 95 printable ASCII characters
    static MSDF_ATLAS_PUBLIC const Charset ASCII;

    /// Adds a codepoint
    void add(unicode_t cp);
    /// Removes a codepoint
    void remove(unicode_t cp);

    size_t size() const;
    bool empty() const;
    std::set<unicode_t>::const_iterator begin() const;
    std::set<unicode_t>::const_iterator end() const;

    /// Load character set from a text file with the correct syntax
    bool load(const char *filename, bool disableCharLiterals = false);

private:
    std::set<unicode_t> codepoints;

};

}

//I'm not sure why, but this part was not compiled into the library, so I had to add it manually.
//TODO: Check why this part was not compiled into the library and fix it.

namespace msdf_atlas
{

    static Charset createAsciiCharset()
    {
        Charset ascii;
        for (unicode_t cp = 0x20; cp < 0x7f; ++cp)
            ascii.add(cp);
        return ascii;
    }

    const Charset Charset::ASCII = createAsciiCharset();

    void Charset::add(unicode_t cp)
    {
        codepoints.insert(cp);
    }

    void Charset::remove(unicode_t cp)
    {
        codepoints.erase(cp);
    }

    size_t Charset::size() const
    {
        return codepoints.size();
    }

    bool Charset::empty() const
    {
        return codepoints.empty();
    }

    std::set<unicode_t>::const_iterator Charset::begin() const
    {
        return codepoints.begin();
    }

    std::set<unicode_t>::const_iterator Charset::end() const
    {
        return codepoints.end();
    }

}
