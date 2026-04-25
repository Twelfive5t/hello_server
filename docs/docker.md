
docker配置参考https://github.com/Twelfive5t/EnvironmentDocument/blob/main/Linux/Configure/Docker.md

docker build . -f Dockerfile -t server_builder:latest

docker tag server_builder:latest twelfive5t/server_builder:latest

docker push twelfive5t/server_builder:latest

docker build . -f Dockerfile.runtime -t server_minimal_runtime:latest

docker tag server_minimal_runtime:latest twelfive5t/server_minimal_runtime:latest

docker push twelfive5t/server_minimal_runtime:latest

docker build . -f Dockerfile.devcontainer -t server_devcontainer:latest