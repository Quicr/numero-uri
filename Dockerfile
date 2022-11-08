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
CMD cmake -B build && cmake --build build && build/bin/webex_http_encoder

FROM base as test
CMD cmake -B build && cmake --build build && ctest --test-dir build/tests
