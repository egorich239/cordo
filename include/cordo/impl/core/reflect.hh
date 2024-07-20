#pragma once

#include <algorithm>
#include <type_traits>

#include "cordo/impl/core/cstring.hh"
#include "cordo/impl/core/meta.hh"

namespace cordo_internal_reflect {

constexpr inline struct {
  template <typename T>
  static constexpr auto type_name(::cordo::tag_t<T>) noexcept {
    constexpr auto fn_name = ::cordo::cstring(__PRETTY_FUNCTION__);
    constexpr auto param_idx = fn_name.find('[');
    if constexpr (param_idx == fn_name.size()) {
      return ::cordo::null_t{};
    } else {
      constexpr auto param_part =
          fn_name.substr(::cordo::value_t<param_idx + 1>{});
      constexpr auto T_idx = param_part.find('T');
      if constexpr (T_idx + 5 > param_part.size() ||
                    param_part.value[T_idx + 1] != ' ' ||
                    param_part.value[T_idx + 2] != '=' ||
                    param_part.value[T_idx + 3] != ' ' ||
                    param_part.value[param_part.size() - 1] != ']') {
        return ::cordo::null_t{};
      } else {
        constexpr auto result =
            param_part.substr(::cordo::value_t<T_idx + 4>{},
                              ::cordo::value_t<param_part.size() - 1>{});
        if constexpr (result.size() == 0) {
          return ::cordo::null_t{};
        } else {
          return result;
        }
      }
    }
  }

  template <typename T>
  static constexpr auto type_base_name(::cordo::tag_t<T>) noexcept {
    constexpr auto tn = type_name(::cordo::tag_t<T>{});
    if constexpr (std::is_same_v<decltype(tn), ::cordo::null_t>) {
      return tn;
    } else {
      constexpr bool is_simple =
          std::none_of(std::begin(tn.value), std::end(tn.value), [](char c) {
            return c == '<' || c == '>' || c == ' ' || c == '&' || c == '*';
          });

      constexpr auto last_idx = tn.rfind(':');
      constexpr auto last = tn.substr(::cordo::value_t<last_idx>{});
      constexpr bool is_ident =
          is_simple && last.size() > 0 &&
          (('A' <= last[0] && last[0] <= 'Z') ||
           ('a' <= last[0] && last[0] <= 'z') || (last[0] == '_')) &&
          [last] {
            bool res = true;
            for (::std::size_t i = 1; i < last.size(); ++i) {
              res &= ('A' <= last[i] && last[i] <= 'Z') ||
                     ('a' <= last[i] && last[i] <= 'z') ||
                     ('0' <= last[i] && last[i] <= '9') || (last[i] == '_');
            }
            return res;
          }();
      if constexpr (is_ident) {
        return last;
      } else {
        return ::cordo::null_t{};
      }
    }
  }
} reflect{};

static_assert(reflect.type_base_name(::cordo::tag_t<char>{}) ==
              ::cordo::cstring("char"));
static_assert(reflect.type_base_name(::cordo::tag_t<::cordo::null_t>{}) ==
              ::cordo::cstring("null_t"));
static_assert(std::is_same_v<decltype(reflect.type_base_name(
                                 ::cordo::tag_t<::cordo::value_t<0>>{})),
                             ::cordo::null_t>);

}  // namespace cordo_internal_reflect