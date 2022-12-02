FROM alpine as base
LABEL description="Build and Test Webex Http Encoder"

# Build tools
RUN apk add build-base
RUN apk add curl-dev
# RUN apk add libressl-dev
RUN apk add gtest-dev
RUN apk add git
RUN apk add cmake

# COPY . /numero_uri
RUN mkdir numero_uri
WORKDIR /numero_uri

FROM base as run
ENV in_args ""
CMD cmake -B build -DBUILD_TESTS=OFF && cmake --build build && build/bin/numero_uri ${in_args}

FROM base as test
CMD cmake -B build -DBUILD_TESTS=ON && cmake --build build \
    && ctest --test-dir build/tests --output-on-failure
