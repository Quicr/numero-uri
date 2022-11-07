.PHONY: all

all: docker

docker:
	@docker build -t webex_http_encoder .
	-@docker run -v ${PWD}:/webex_http_encoder --rm -it webex_http_encoder

build:
	cmake -B build
	cmake --build build

run: build
	./build/bin/Debug/webex_http_encoder


