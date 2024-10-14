{% set categories = [
    'VP', 'UT', 'FT', 'Muon', 'Calo', 'RICH',
    'Core', 'Conditions', 'Utilities',
    'Build',
    ] -%}
{% set used_mrs = [] -%}
{% macro section(labels, mrs=merge_requests, used=used_mrs, indent='', highlight='highlight') -%}
{% for mr in order_by_label(select_mrs(mrs, labels, used), categories) -%}
  {% set mr_labels = categories|select("in", mr.labels)|list -%}
{{indent}}- {% if mr_labels %}{{mr_labels|map('label_ref')|join(' ')}} | {% endif -%}
    {{mr.title|sentence}}, {{mr|mr_ref(project_fullname)}} (@{{mr.author.username}}){% if mr.issue_refs %} [{{mr.issue_refs|join(',')}}]{% endif %}{% if highlight in mr.labels %} :star:{% endif %}
{# {{mr.description|mdindent(2)}} -#}
{% endfor -%}
{% endmacro -%}
{{date}} {{project}} {{version}}
===

This version uses {{project_deps|last}}.

This version is released on the `{{branch}}` branch.
Built relative to {{project}} [{{project_prev_tag}}](/../../tags/{{project_prev_tag}}), with the following changes:

{{ section([[]]) }}
