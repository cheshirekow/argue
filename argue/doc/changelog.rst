=========
Changelog
=========

v 0.1.0

Initial release! This is mostly a proof of concept initial implementation. It
lacks several features I would consider required but works pretty well to start
using it.

Features:
* argparse-like syntax
* type-safe implementations for ``store``, ``store_const``, ``help``, and
  ``version`` actions
* support for scalar (nargs=<default>, nargs='?') or
  container (nargs=<n>, nargs='+', nargs='*') assignment
* provides different exception types and error messages for exceptional
  conditions so they can be distinguised between input errors (user errors),
  library usage errors (your bugs), or library errors (my bugs).
* support for custom actions
* output formatting for usage, version, and full help text

