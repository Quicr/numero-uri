.PHONY: all

all: docker

docker:
	@docker build -t webex_http_encoder .

	@echo ""
	@echo "--Run Executable--"
	@echo ""
	-@docker run --name webex_http_encoder -v ${PWD}:/src -it webex_http_encoder

	@echo ""
	@echo "--Clean up container--"
	docker rm webex_http_encoder

build:
	@g++ -std=c++17 -o webex_http_encoder main.cc HttpEncoder.cc


