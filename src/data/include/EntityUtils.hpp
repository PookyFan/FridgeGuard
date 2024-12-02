#pragma once

namespace FG::data
{
using Id = int;

template<typename T>
using Nullable = std::optional<T>;

template<class T>
concept WithFkEntity = requires { typename T::FkEntity; };

template<class S>
constexpr auto primaryAndForeignKeysCount() { return 1; }

template<WithFkEntity S>
constexpr auto primaryAndForeignKeysCount() { return 2; }
}