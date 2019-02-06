#pragma once

#include <cstdint>

template<typename T, uint8_t index, uint8_t width>
struct bitfield{
    T bits;
    enum {mask=((1u << width) - 1)};
    template<typename T2>
    bitfield& operator=(T2 value){
        bits = (bits & ~(mask << index)) | ((value & mask) << index);
        return *this;
    }

    operator unsigned(){
        return static_cast<unsigned>((bits >> index) & mask);
    }

    bitfield& operator ++ () const {
        bits = (bits & ~(mask << index)) | (((bits & mask) + (1 << index)) & mask);
        return *this;
    }

    bitfield operator ++ (int) const {
        bits = (bits & ~(mask << index)) | (((bits & mask) + (1 << index)) & mask);
        return *this;
    }

    template<typename TT>
    bitfield& operator |= (const TT &other){
        bits = bits | ((other << index) & mask);
        return *this;
    }

    template<typename TT>
    bitfield& operator &= (const TT &other){
        bits = bits & ((other << index) & mask);
        return *this;
    }

    operator bool () const {
        return (bits & mask);
    }
} __attribute__((packed));