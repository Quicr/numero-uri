cmd 			?= RUN
args			?=

.PHONY: all

all: docker

docker:
	@docker build -t webex_http_encoder --target ${cmd} .
	-@docker run -v ${PWD}:/webex_http_encoder -e in_args="${args}" \
		--rm -it webex_http_encoder

build:
	cmake -B build
	cmake --build build

run: build
	./build/bin/Debug/webex_http_encoder


