# 从 product 文件夹生成产品镜像

. ./.env

cp .env products/bin$SERVER_VERSION/.env
cp start_service.sh products/bin$SERVER_VERSION/start_service.sh
cp stop_service.sh products/bin$SERVER_VERSION/stop_service.sh
cp package/Dockerfile products/bin$SERVER_VERSION/Dockerfile
cp docker-compose.yaml products/bin$SERVER_VERSION/docker-compose.yaml
rm -rf products/bin$SERVER_VERSION/config
cp -r config products/bin$SERVER_VERSION/

cd products/bin$SERVER_VERSION

docker build . -t  $PRODUCT_IMAGE --build-arg BASE_IMAGE=$RUNTIME_IMAGE
