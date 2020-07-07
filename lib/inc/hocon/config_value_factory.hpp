# pragma once

#include "types.hpp"
#include "export.h"
#include <string>

namespace hocon {
    class LIBCPP_HOCON_EXPORT config_value_factory {
         public:
         /**
          * Creates a {@link config_value} from a plain value, which may be
          * a <code>bool</code>, <code>long</code>, <code>string</code>,
          * <code>unordered_map</code>, <code>vector</code> or <code>nullptr</code>. An
          * <code>unordered_map</code> must be a <code>unordered_map</code> from string to more values
          * that can be supplied to <code>from_any_ref()</code>. An <code>unordered_map</code>
          * will become a {@link config_object} and a <code>vector</code> will become a
          * {@link config_list}.
          *
          * <p>
          * In a <code>unordered_map</code> passed to <code>from_any_ref()</code>, the map's keys
          * are plain keys, not path expressions. So if your <code>unordered_map</code> has a
          * key "foo.bar" then you will get one object with a key called "foo.bar",
          * rather than an object with a key "foo" containing another object with a
          * key "bar".
          *
          * <p>
          * The origin_description will be used to set the origin() field on the
          * config_value. It should normally be the name of the file the values came
          * from, or something short describing the value such as "default settings".
          * The origin_description is prefixed to error messages so users can tell
          * where problematic values are coming from.
          *
          * <p>
          * Supplying the result of config_value.unwrapped() to this function is
          * guaranteed to work and should give you back a config_value that matches
          * the one you unwrapped. The re-wrapped config_value will lose some
          * information that was present in the original such as its origin, but it
          * will have matching values.
          *
          * @param unwrapped_value object
          *         object to convert to config_value
          * @param string origin_description
          *         name of origin file or brief description of what the value is
          * @return shared_value a new value
          * */
       static shared_value from_any_ref(unwrapped_value value, std::string origin_description = "");
    };
}  // namespace hocon
