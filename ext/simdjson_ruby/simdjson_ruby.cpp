
#include <string>
#include <string_view>

#include "simdjson.h"
#include "simdjson_ruby.hpp"

VALUE rb_mSimdjsonRuby;

using namespace simdjson;

// Convert tape to Ruby's Object
static VALUE make_ruby_object(ParsedJson::Iterator &it) {
    if (it.is_object()) {
        VALUE hash = rb_hash_new();
        if (it.down()) {
            do {
                assert(it.is_string());
                VALUE key = rb_str_new(it.get_string(), it.get_string_length());
                it.next();
                VALUE val = make_ruby_object(it);
                rb_hash_aset(hash, key, val);
            } while (it.next());
            it.up();
        }
        return hash;
    } else if (it.is_array()) {
        VALUE ary = rb_ary_new();
        if (it.down()) {
            VALUE e0 = make_ruby_object(it);
            rb_ary_push(ary, e0);
            while (it.next()) {
                VALUE e = make_ruby_object(it);
                rb_ary_push(ary, e);
            }
            it.up();
        }
        return ary;
    } else if (it.is_string()) {
        return rb_str_new(it.get_string(), it.get_string_length());
    } else if (it.is_integer()) {
        return LONG2NUM(it.get_integer());
    } else if (it.is_double()) {
        return DBL2NUM(it.get_double());
    } else if (it.get_type() == 'n') {  // TODO replace get_type()
        return Qnil;
    } else if (it.get_type() == 't') {  // TODO replace get_type()
        return Qtrue;
    } else if (it.get_type() == 'f') {  // TODO replace get_type()
        return Qfalse;
    }
    // unknown case (bug)
    rb_raise(rb_eException, "[BUG] must not happen");
}

static VALUE rb_simdjson_parse(VALUE self, VALUE arg) {
    const padded_string p{RSTRING_PTR(arg)};
    ParsedJson pj = build_parsed_json(p);
    if (!pj.is_valid()) {
        // TODO raise Exception?
        return Qnil;
    }
    ParsedJson::Iterator it{pj};
    return make_ruby_object(it);
}

extern "C" {

void Init_simdjson_ruby(void) {
    rb_mSimdjsonRuby = rb_define_module("SimdjsonRuby");
    rb_define_module_function(rb_mSimdjsonRuby, "parse", reinterpret_cast<VALUE (*)(...)>(rb_simdjson_parse), 1);
}
}
