/*
 * Copyright (C) 2025 Philippe Proulx <eepp.ca>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#ifndef _JOME_UTILS_HPP
#define _JOME_UTILS_HPP

#include <functional>
#include <utility>

namespace jome {

/*
 * Partial implementation of INVOKE.
 *
 * As found in
 * <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2017/p0312r1.html>.
 */
template <typename FuncT, typename... ArgTs>
auto call(FuncT func, ArgTs&&...args) -> decltype(std::ref(func)(std::forward<ArgTs>(args)...))
{
    return std::ref(func)(std::forward<ArgTs>(args)...);
}

} // namespace jome

#endif // _JOME_UTILS_HPP
