FROM alpine as base
LABEL description="Build and Test Webex Http Encoder"

# Build tools
RUN apk add build-base
RUN apk add libressl-dev
RUN apk add gtest-dev
RUN apk add git
RUN apk add cmake

# COPY . /webex_http_encoder
RUN mkdir webex_http_encoder
WORKDIR /webex_http_encoder

FROM base as run
ENV in_args ""
CMD cmake -B build -DBUILD_TESTS=OFF && cmake --build build && build/bin/webex_http_encoder ${in_args}

FROM base as test
CMD cmake -B build -DBUILD_TESTS=ON && cmake --build build \
    && ctest --test-dir build/tests --output-on-failure
