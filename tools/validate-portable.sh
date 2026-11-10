#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
build_dir="${repo_root}/build/portable"

cmake -S "${repo_root}" -B "${build_dir}" \
  -DCMS2_BUILD_PLUGIN=OFF \
  -DCMS2_BUILD_TESTS=ON
cmake --build "${build_dir}" --parallel
ctest --test-dir "${build_dir}" --output-on-failure

python3 - "${repo_root}" <<'PY'
import json
import pathlib
import sys
import xml.etree.ElementTree as ET

root = pathlib.Path(sys.argv[1])
with (root / "vcpkg.json").open(encoding="utf-8") as handle:
    json.load(handle)
with (root / "CMakePresets.json").open(encoding="utf-8") as handle:
    json.load(handle)
ET.parse(root / "package/fomod/info.xml")
print("Manifest and FOMOD metadata checks passed.")
PY
