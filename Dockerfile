FROM ubuntu:22.04

# ── 换源（必须在首次 apt update 之前完成，否则仍走原源）─────────────
RUN sed -i \
        -e 's@/ports.ubuntu.com/@/mirrors.aliyun.com/@g' \
        -e 's@/archive.ubuntu.com/@/mirrors.aliyun.com/@g' \
        -e 's@/security.ubuntu.com/@/mirrors.aliyun.com/@g' \
        /etc/apt/sources.list


# ── 更新软件包索引（独立层：index 不变时后续 install 层可直接命中缓存）
RUN apt update

# ── 安装构建工具链与 locale ──────────────────────────────────────────
RUN DEBIAN_FRONTEND=noninteractive apt install -y --no-install-recommends \
        tzdata locales \
        cmake gcc g++ make build-essential pkg-config \
        binutils-aarch64-linux-gnu \
    && locale-gen en_US.UTF-8 \
    && update-locale LANG=en_US.UTF-8

# ── 时区与语言环境（纯 ENV / 文件操作，不依赖包管理器）──────────────────
ENV TZ=Asia/Shanghai \
    LANG=en_US.UTF-8 \
    LANGUAGE=en_US:en \
    LC_ALL=en_US.UTF-8

RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime \
    && echo $TZ > /etc/timezone

# ── 安装库、开发工具、Python 与版本控制 ──────────────────────────────
RUN DEBIAN_FRONTEND=noninteractive apt install -y --no-install-recommends \
        libcurl4 libssl-dev \
        wget curl unzip git \
        doxygen lsb-release \
        software-properties-common gnupg \
        python3 python3-pip

# ── 安装网络诊断工具 ────────────────────
RUN DEBIAN_FRONTEND=noninteractive apt install -y --no-install-recommends \
        iproute2 net-tools network-manager arping ethtool

# ── 安装 Conan 2（独立成层，包版本稳定时可复用缓存）──────────────────
RUN pip3 install --no-cache-dir conan

# ── 安装 Clang 18（LLVM 官方脚本）──
RUN wget -qO /tmp/llvm.sh https://apt.llvm.org/llvm.sh \
    && chmod +x /tmp/llvm.sh \
    && /tmp/llvm.sh 18 all \
    && rm /tmp/llvm.sh

# ── 将 Clang 18 注册为系统默认工具链 ────────────────────────────────────
RUN update-alternatives --install /usr/bin/clang          clang          /usr/bin/clang-18          100 \
    && update-alternatives --install /usr/bin/clang++     clang++        /usr/bin/clang++-18        100 \
    && update-alternatives --install /usr/bin/clangd      clangd         /usr/bin/clangd-18         100 \
    && update-alternatives --install /usr/bin/clang-tidy  clang-tidy     /usr/bin/clang-tidy-18     100 \
    && update-alternatives --install /usr/bin/run-clang-tidy run-clang-tidy /usr/bin/run-clang-tidy-18 100 \
    && update-alternatives --install /usr/bin/clang-format clang-format  /usr/bin/clang-format-18   100

# ── 设置编译器环境变量 ──────────────────────────────────────────────────
ENV CC=/usr/bin/clang-18 \
    CXX=/usr/bin/clang++-18

# ── 工作目录 ────────────────────────────────────────────────────────────
WORKDIR /workspace

CMD ["bash"]