Types
=====

Expressions
------------

Expressions such as `{{ VAR }}` are replaced with the corresponding value in the
given dictionary or are left blank.

If `VAR` is not `NULL` or a `string`, the render fails.


Statements
----------

 - `{% if <var> [ == | != | < | > | <= | >= ] <val | none> %}` and {% endif %}`
    - `val` must be inside quotes (e.g. `"foo"`)
 - `{% for <var> in <dict> %}` and `{% endfor %}`


Comments
--------

Anything between `{# ... #}`.
