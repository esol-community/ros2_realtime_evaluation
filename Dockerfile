FROM ros:jazzy

ENV USER=user

# ----------------------------
# install package (root)
# ----------------------------
ARG NODE_VERSION=22.23.2

RUN --mount=type=cache,target=/var/cache/apt \
    --mount=type=cache,target=/var/lib/apt/lists \
    apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -y \
    libopencv-dev \
    stress-ng \
    python3-pip \
    curl \
    wget \
    libasound2t64 \
    build-essential \
    git \
    python3-colcon-common-extensions \
    python3-colcon-mixin \
    python3-rosdep \
    python3-vcstool \
    ninja-build \
    ccache \
    cmake \
    libglu1-mesa-dev \
    libgl1-mesa-dev \
    libxrandr-dev \
    libxinerama-dev \
    libxcursor-dev \
    libxi-dev \
    unzip tar \
    uthash-dev \
    lttng-tools \
    tree \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /home/$USER
RUN set -eux; \
ARCH="$(dpkg --print-architecture)"; \
   case "$ARCH" in \
      arm64) NODE_ARCH="arm64" ;; \
      amd64) NODE_ARCH="x64" ;; \
      *) echo "Unsupported architecture: $ARCH"; exit 1 ;; \
   esac; \
   curl -fsSLO "https://nodejs.org/dist/v${NODE_VERSION}/node-v${NODE_VERSION}-linux-${NODE_ARCH}.tar.xz"; \
   tar -xJf "node-v${NODE_VERSION}-linux-${NODE_ARCH}.tar.xz" \
      -C /usr/local \
      --strip-components=1 \
      --no-same-owner; \
   rm "node-v${NODE_VERSION}-linux-${NODE_ARCH}.tar.xz"; \
   node --version; \
   npm --version

# ----------------------------
# user
# ----------------------------
RUN useradd -m -s /bin/bash $USER && \
    usermod -aG sudo $USER && \
    echo '%sudo ALL=(ALL) NOPASSWD:ALL' >> /etc/sudoers

RUN chown -R $USER:$USER /home/$USER
USER $USER

# ----------------------------
# env
# ----------------------------
ENV WORKDIR_CARET=/home/$USER/ros2_caret_ws
ENV WORKDIR_SAMPLE=/home/$USER/ros2_ws
ENV CARET_VERSION=v0.7.2
ENV PATH=/home/$USER/.local/bin:$PATH

# ----------------------------
# puppeteer
# ----------------------------
ARG PUPPETEER_BROWSERS_VERSION=3.2.1
ARG CHROME_VERSION=153.0.8010.5
WORKDIR /home/$USER
RUN --mount=type=cache,target=/home/$USER/.npm,uid=1001,gid=1001 \
    npx --yes "@puppeteer/browsers@${PUPPETEER_BROWSERS_VERSION}" \
    install "chrome@${CHROME_VERSION}"; \
    npx --yes "@puppeteer/browsers@${PUPPETEER_BROWSERS_VERSION}" \
    install "chromedriver@${CHROME_VERSION}"; \
    mkdir -p ~/.local/bin && \
    ln -s $PWD/chrome/*/chrome-linux64/chrome ~/.local/bin/chrome && \
    ln -s $PWD/chromedriver/*/chromedriver-linux64/chromedriver ~/.local/bin/chromedriver

# ----------------------------
# CARET
# ----------------------------
RUN git clone https://github.com/tier4/caret.git $WORKDIR_CARET -b $CARET_VERSION
WORKDIR $WORKDIR_CARET

RUN --mount=type=cache,target=/var/cache/apt \
    --mount=type=cache,target=/var/lib/apt/lists \
    --mount=type=cache,target=/home/$USER/.cache/pip \
    export PIP_BREAK_SYSTEM_PACKAGES=1 && \
    . /opt/ros/$ROS_DISTRO/setup.sh && \
    rosdep update && \
    mkdir -p src && \
    vcs import src < caret_jazzy.repos && \
    ./setup_caret.sh -c -d $ROS_DISTRO && \
    colcon build --symlink-install --cmake-args -DCMAKE_BUILD_TYPE=Release

RUN --mount=type=cache,target=/home/$USER/.cache/pip \
    PIP_BREAK_SYSTEM_PACKAGES=1 \
    pip install -U jupyterlab numpy"<2" pandas"~=2.1.1" scipy numexpr bottleneck rotop nbconvert selenium ipykernel

# ----------------------------
# sample workspace
# ----------------------------
RUN mkdir -p $WORKDIR_SAMPLE/src
WORKDIR $WORKDIR_SAMPLE/src

RUN --mount=type=bind,source=./ros2_ws/src,target=/home/$USER/src_origin \
    cp -ar /home/$USER/src_origin/* $WORKDIR_SAMPLE/src/

RUN --mount=type=cache,target=/var/cache/apt \
    --mount=type=cache,target=/var/lib/apt/lists \
    --mount=type=cache,target=/home/$USER/.cache/pip \
    export PIP_BREAK_SYSTEM_PACKAGES=1 && \
    . /opt/ros/$ROS_DISTRO/setup.sh && \
    # vcs import < sample_node/jazzy_in_house.repos
    vcs import < sample_node/jazzy_github.repos

# ----------------------------
# build
# ----------------------------
WORKDIR $WORKDIR_SAMPLE

RUN --mount=type=cache,target=/var/cache/apt \
    --mount=type=cache,target=/var/lib/apt/lists \
    --mount=type=cache,target=/home/$USER/.cache/pip \
    . /opt/ros/${ROS_DISTRO}/setup.sh && \
    . ${WORKDIR_CARET}/install/local_setup.sh && \
    rosdep update && \
    rosdep install --from-paths src --ignore-src --rosdistro ${ROS_DISTRO} -r -y

ARG APP_BUILD_FLAGS

RUN --mount=type=cache,target=/home/${USER}/.ccache \
    export CCACHE_DIR=/home/${USER}/.ccache PATH="/usr/lib/ccache/bin/:$PATH" && \
    . /opt/ros/${ROS_DISTRO}/setup.sh && \
    . ${WORKDIR_CARET}/install/local_setup.sh && \
    colcon build --symlink-install \
      --cmake-args -GNinja -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF --event-handlers console_direct+\
      $APP_BUILD_FLAGS


# ----------------------------
# entrypoint
# ----------------------------
COPY ./ros_entrypoint.sh /
ENTRYPOINT ["/ros_entrypoint.sh"]
CMD ["bash"]