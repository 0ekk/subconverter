#!/bin/bash
# Kept as the one-shot entry point; CI builds the two halves separately so that
# buildx can cache the dependency layer.
set -xe

cd "$(dirname "$0")/.."
bash scripts/build.alpine.deps.sh
bash scripts/build.alpine.build.sh
