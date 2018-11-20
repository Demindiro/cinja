Types
=====

Substitution
------------

Expressions such as `{{ VAR }}` are replaced with the corresponding value in the
given dictionary or are left blank.

If `VAR` is not `NULL` or a `string`, the render fails.


Expressions
-----------

 - `{% if <var> [ == | != | < | > | <= | >= ] <val | none> %}` and {% endif %}`
    - `val` must be inside quotes (e.g. `"foo"`)


Comments
--------

Anything between `{# ... #}`.


Trimming whitespace
-------------------

Whitespace can be trimmed by adding a `-` right before `.}` or after `{.`.

e.g. `{{ FOO -}} bar {{- BAZ }}` is equivalent to `{{ FOO }}bar{{ BAZ }}`.
