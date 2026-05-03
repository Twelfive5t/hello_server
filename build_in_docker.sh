#!/bin/bash
. ./.env

# 如果没有传入参数，使用默认值 'Server_Release'
PRESET="${1:-Server_Release}"

# 检查并拉取 BUILDER_IMAGE
if [ -z "$(docker images -q $BUILDER_IMAGE)" ]; then
  echo "Pulling $BUILDER_IMAGE..."
  docker pull $BUILDER_IMAGE
else
  echo "$BUILDER_IMAGE already exists locally."
fi

# 检查并拉取 RUNTIME_IMAGE
if [ -z "$(docker images -q $RUNTIME_IMAGE)" ]; then
  echo "Pulling $RUNTIME_IMAGE..."
  docker pull $RUNTIME_IMAGE
else
  echo "$RUNTIME_IMAGE already exists locally."
fi

# 在 Docker 中构建项目
docker run -it --privileged --rm \
    -v $(pwd):/workspace \
    -v /usr/bin/docker:/usr/bin/docker \
    -v /var/run/docker.sock:/var/run/docker.sock \
    -v /usr/libexec/docker/cli-plugins:/usr/libexec/docker/cli-plugins \
    -v ~/.conan2:/root/.conan2 \
    $BUILDER_IMAGE \
    bash -c "./build.sh $PRESET"
    # /bin/sh -c "while sleep 1000; do :; done"
