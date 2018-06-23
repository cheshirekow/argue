==========
googletest
==========

Cloned from https://github.com/google/googletest.git on 01/04/2018. At the time
of the clone, the ``master`` branch was at
``be6ee26a9b5b814c3e173c6e5e97c385fbdc1fb0``.

I copied the following files/directories from ``googletest`` in the repo clone
to the ``third_party`` directory under ``googletest``:

* ``include``
* ``src``
* ``LICENSE``

I ignored the rest of the cruft.

===
re2
===

Cloned from https://github.com/google/re2.git on 01/11/2018. At the time of
the clone the ``master`` branch was at::

    commit 7cf8b88e8f70f97fd4926b56aa87e7f53b2717e0
    Author: Paul Wankadia <junyer@google.com>
    Date:   Thu Jan 11 15:33:02 2018 +1100

        Tweak the BUILD file formatting.

        Change-Id: I11b1c4b2bda836a9ea3afeb0cdf7aa9372eeaa13
        Reviewed-on: https://code-review.googlesource.com/21991
        Reviewed-by: Paul Wankadia <junyer@google.com>

I coped the following files/directories over from ``re2`` in the repo clone
to the ``third_party`` directory under ``re2``:

* ``re2``
* ``util``
* ``LICENSE``

=========
cmake-ast
=========

Cloned from https://github.com/polysquare/cmake-ast.git on 01/21/2018. At the
time of the clone the ``master`` branch was at::

    commit 4cb9ca55d84989b748a68d6f0c46cdc72a25da81
    Merge: b71ee96 a95b7fa
    Author: Sam Spilsbury <smspillaz@gmail.com>
    Date:   Thu Nov 10 20:52:13 2016 +0800

        Merge pull request #26 from smspillaz/update-repo-api-key-10092016

        travis: Update secure environment variables on advice from travis-ci

I copied the following files/directories over from the repo clone into the
``third_party`` directory under ``cmake-ast``:

* ``cmake_ast``
* ``LICENSE.md``

=====
clang
=====

This is not the full clone of clang. Just the ctypes python bindings from
``clang/bindings/python/clang``. Cloned from
https://github.com/llvm-mirror/clang.git on 01/25/2018. At the time of the clone
master was at::

    commit a58d0437d1e17d53d7bffea598d77789dd6d28b6
    Author: Amara Emerson <aemerson@apple.com>
    Date:   Fri Jan 26 00:27:22 2018 +0000

        [Driver] Add an -fexperimental-isel driver option to enable/disable GlobalISel.

        Differential Revision: https://reviews.llvm.org/D42276

        git-svn-id: https://llvm.org/svn/llvm-project/cfe/trunk@323485 91177308-0d34-0410-b5e6-96231b3b80d8

