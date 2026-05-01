# 从 product 文件夹生成产品镜像

. ./.env

cp package/Dockerfile products/bin$SERVER_VERSION/Dockerfile

cd products/bin$SERVER_VERSION

docker build . -t  $PRODUCT_IMAGE --build-arg BASE_IMAGE=$RUNTIME_IMAGE
