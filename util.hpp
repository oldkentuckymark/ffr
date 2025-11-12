#pragma once

#include <initializer_list>

namespace ffr
{
namespace util
{

template<class T, auto SIZE>
class array
{
public:

    constexpr array() = default;

    constexpr array(std::initializer_list<T> list)
    {
        decltype(SIZE) k = 0;
        for(auto i = list.begin(); i != list.end(); ++i)
        {
            data_[k] = *i;
            ++k;
        }
    }

    constexpr auto operator [] (decltype(SIZE) const i) -> T&
    {
        return data_[i];
    }

    constexpr auto operator [] (decltype(SIZE) const i) const -> T const &
    {
        return data_[i];
    }

    constexpr auto size() const -> decltype(SIZE)
    {
        return SIZE;
    }

    constexpr auto clear() -> void
    {
        for(auto i : (*this))
        {
            i = {};
        }
    }

    constexpr auto data() const -> T const*
    {
        return data_;
    }

    constexpr auto data() -> T*
    {
        return data_;
    }

    template<decltype(SIZE) const NUM_CHUNKS, decltype(SIZE) chunk_num>
    constexpr auto chunk() const -> array<T, SIZE/NUM_CHUNKS>
    {
        auto const cs = SIZE/NUM_CHUNKS;
        array<T, cs> r;
        decltype(SIZE) k = 0;
        for(auto i = cs * chunk_num; i < (cs*chunk_num) + cs; ++i)
        {
            r[k] = data_[i];
        }
        return r;
    }



    constexpr auto reverse() -> array<T, SIZE>
    {
        array<T,SIZE> r;
        auto k = SIZE - 1;
        for(decltype(SIZE) i = 0; i < SIZE ; ++i)
        {
            r[k] = data_[i];
            --k;
        }
        return r;
    }

    constexpr auto apply(auto FUNC) -> array<T, SIZE>
    {
        for(auto& i : (*this))
        {
            i = FUNC(i);
        }
    }

    friend struct Iterator;
    struct Iterator
    {

        constexpr explicit Iterator(array<T, SIZE> & o, decltype(SIZE) const i) :
            obj(o),
            idx(i)
        {

        }

        constexpr auto operator*() const -> T& { return obj[idx]; }

        constexpr auto operator++() -> Iterator&
        {
            idx = idx + 1;;
            return *this;
        }
        constexpr auto operator++(int) -> Iterator { Iterator tmp = *this; ++(*this); return tmp; }
        constexpr friend auto operator== (const Iterator& a, const Iterator& b) -> bool { return &a.obj == &b.obj && a.idx == b.idx; };
        constexpr friend auto operator!= (const Iterator& a, const Iterator& b) -> bool { return !(&a.obj == &b.obj && a.idx == b.idx); };

    private:
        array<T, SIZE> & obj;
        decltype(SIZE) idx;
    };

    constexpr auto begin() -> Iterator { return Iterator(*this,0); }
    constexpr auto end() -> Iterator   { return Iterator(*this, SIZE); }

private:
    T data_[SIZE] = {};

};

template<class T, auto SIZE, auto FUNC>
constexpr auto makeTable() -> ffr::util::array<T, SIZE>
{
    ffr::util::array<T,SIZE> r{};
    for(std::size_t i = 0; i < SIZE; ++i)
    {
        r[i] = FUNC(i);
    }
    return r;
}

template<class T>
constexpr auto createCube(T const xRadius,
                          T const yRadius,
                          T const zRadius) -> ffr::util::array<T, 108>
{
    ffr::util::array<T, 108> r =
    {
        // Front face (+Z)
        -xRadius, -yRadius,  zRadius,
        xRadius, -yRadius,  zRadius,
        xRadius,  yRadius,  zRadius,
        -xRadius, -yRadius,  zRadius,
        xRadius,  yRadius,  zRadius,
        -xRadius,  yRadius,  zRadius,

        // Back face (-Z)
        xRadius, -yRadius, -zRadius,
        -xRadius, -yRadius, -zRadius,
        -xRadius,  yRadius, -zRadius,
        xRadius, -yRadius, -zRadius,
        -xRadius,  yRadius, -zRadius,
        xRadius,  yRadius, -zRadius,

        // Left face (-X)
        -xRadius, -yRadius, -zRadius,
        -xRadius, -yRadius,  zRadius,
        -xRadius,  yRadius,  zRadius,
        -xRadius, -yRadius, -zRadius,
        -xRadius,  yRadius,  zRadius,
        -xRadius,  yRadius, -zRadius,

        // Right face (+X)
        xRadius, -yRadius,  zRadius,
        xRadius, -yRadius, -zRadius,
        xRadius,  yRadius, -zRadius,
        xRadius, -yRadius,  zRadius,
        xRadius,  yRadius, -zRadius,
        xRadius,  yRadius,  zRadius,

        // Top face (+Y)
        -xRadius,  yRadius,  zRadius,
        xRadius,  yRadius,  zRadius,
        xRadius,  yRadius, -zRadius,
        -xRadius,  yRadius,  zRadius,
        xRadius,  yRadius, -zRadius,
        -xRadius,  yRadius, -zRadius,

        // Bottom face (-Y)
        -xRadius, -yRadius, -zRadius,
        xRadius, -yRadius, -zRadius,
        xRadius, -yRadius,  zRadius,
        -xRadius, -yRadius, -zRadius,
        xRadius, -yRadius,  zRadius,
        -xRadius, -yRadius, zRadius
    };


    return r;
}

}
}
