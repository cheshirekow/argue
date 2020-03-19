package(default_visibility = ["//visibility:public"])

cc_library(
  name = "argue",
  srcs = [
    "action.cc",
    "exception.cc",
    "glog.cc",
    "kwargs.cc",
    "parse.cc",
    "parser.cc",
  ],
  hdrs = [
    "action.h",
    "action.tcc",
    "argue.h",
    "exception.h",
    "glog.h",
    "keywords.h",
    "keywords.tcc",
    "kwargs.h",
    "kwargs.tcc",
    "parse.h",
    "parse.tcc",
    "parser.h",
    "parser.tcc",
    "storage_model.h",
    "storage_model.tcc",
    "util.h",
  ],
  deps = [
    "//tangent/json",
    "//tangent/util",
    "@system//:fmt",
    "@system//:glog",
  ],
)

cc_binary(
  name = "argue-can-complete",
  srcs = ["can_complete.cc"],
  deps = [
    ":argue",
    "@system//:libelf",
    "@system//:libloki",
  ],
)
