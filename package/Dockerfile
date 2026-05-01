ARG BASE_IMAGE=none
FROM ${BASE_IMAGE}

RUN sysctl -w kernel.core_pattern=/corefile/core.%e.%p.%s.%E
COPY . /workspace

CMD [ "bash" ]