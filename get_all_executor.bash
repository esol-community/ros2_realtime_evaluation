#!/usr/bin/env bash

set -euo pipefail

for e in single multi events static_single cbg cie realtime_single realtime_multi; do
    EXECUTOR="$e" docker compose run --build --remove-orphans record
    docker compose up --build visualize-to-html

    [ -f "caret_sample/jupyter/record_stress_non_rt.html" ] &&
        mv "caret_sample/jupyter/record_stress_non_rt.html" \
           "caret_sample/jupyter/${e}_record_stress_non_rt.html"

    [ -f "caret_sample/jupyter/record_stress_non_rt.ipynb" ] &&
        mv "caret_sample/jupyter/record_stress_non_rt.ipynb" \
           "caret_sample/jupyter/${e}_record_stress_non_rt.ipynb"
done