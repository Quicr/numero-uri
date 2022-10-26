FROM alpine as base
LABEL description="Build and Test Webex Http Encoder"

# Build tools
RUN apk add build-base
RUN apk add libressl-dev

COPY . /webex_http_encoder
WORKDIR /webex_http_encoder/
RUN g++ -o webex_http_encoder main.cc HttpEncoder.cc
# TODO make it run tests instead?
CMD ["./webex_http_encoder"]