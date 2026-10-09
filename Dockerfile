FROM qwen-project-engineer:hardened-v2

# Project-specific build dependencies persist in the managed image created by
# project_bootstrap. libcurl headers are already supplied by the hardened-v2
# engineering base; GoogleTest is needed by this project's current test suite.
RUN apt-get update \
    && apt-get install -y --no-install-recommends libgtest-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /project
