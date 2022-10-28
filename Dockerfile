FROM alpine as base
LABEL description="Build and Test Webex Http Encoder"

# Build tools
RUN apk add build-base
RUN apk add libressl-dev
RUN apk add gtest-dev

# COPY . /webex_http_encoder
RUN mkdir webex_http_encoder
WORKDIR /webex_http_encoder

CMD make build && build/bin/webex_http_encoder
