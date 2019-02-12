#ifndef BITFIELD_BITFIELD_HPP
#define BITFIELD_BITFIELD_HPP

#include <cstdint>

namespace Bitfield{

template<typename TFieldType, uint8_t UIndex, uint8_t VWidth>
struct Bitfield{
    TFieldType bits;
    enum {mask=((1u << VWidth) - 1)};
    
    template<typename Other>
    Bitfield& operator=(Other other){
        bits = (bits & ~(mask << UIndex)) | ((other & mask) << UIndex);
        return *this;
    }

    operator unsigned(){
        return static_cast<unsigned>((bits >> UIndex) & mask);
    }

    Bitfield& operator ++ () const {
        bits = (bits & ~(mask << UIndex)) | (((bits & mask) + (1 << UIndex)) & mask);
        return *this;
    }

    Bitfield operator ++ (int) const {
        bits = (bits & ~(mask << UIndex)) | (((bits & mask) + (1 << UIndex)) & mask);
        return *this;
    }

    template<typename Other>
    Bitfield& operator |= (const Other &other){
        bits = bits | ((other << UIndex) & mask);
        return *this;
    }

    template<typename Other>
    Bitfield& operator &= (const Other &other){
        bits = bits & ((other << UIndex) & mask);
        return *this;
    }

    operator bool () const {
        return (bits & mask);
    }
} __attribute__((packed));

} // namespace Bitfield

#endif // BIFIELD_BITFIELD_HPP