/*
 * Copyright (C) 2025 Philippe Proulx <eepp.ca>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#ifndef _JOME_UTILS_HPP
#define _JOME_UTILS_HPP

#include <QString>
#include <utility>
#include <fmt/format.h>

namespace jome {

/*
 * Like fmt::format(), but returns a `QString`.
 */
template <typename... ArgTs>
QString qFmtFormat(fmt::format_string<ArgTs...> fmt, ArgTs&&... args)
{
    return QString::fromStdString(fmt::format(fmt, std::forward<ArgTs>(args)...));
}

} // namespace jome

#endif // _JOME_UTILS_HPP
